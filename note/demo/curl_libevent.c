#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <event2/event.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MSG_OUT stdout

enum {
    XHR_RSTATE_UNSENT = 0,
    XHR_RSTATE_OPENED,
    XHR_RSTATE_HEADERS_RECEIVED,
    XHR_RSTATE_LOADING,
    XHR_RSTATE_DONE,
};

typedef struct
{
    struct event_base *base;
    CURLM *curl_handle;
    struct event *timeout;

    struct event *fifo_event;
    FILE *input;

    int still_running;
    int stopped;
} global_t;

typedef struct curl_socket_s
{
    struct event *event;
    curl_socket_t sockfd;
} curl_socket_ctx;

typedef void (*curl_done_cb)(CURLMsg *message, void *arg);
typedef char content_t;
typedef struct
{
    char *url; // 使用strdup，需要free，见destroy_req_ctx
    content_t *hbuf; // header content buffer
    size_t hlen;
    content_t *bbuf; // body content buffer
    size_t blen;
    curl_done_cb done_cb;
    CURL *handle; // curl easy handle
    unsigned short ready_state; // xhr ready_state
} req_ctx_t;

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data);
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data);
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
static void curl_perform(int fd, short event, void *arg);

static curl_socket_ctx *create_socket_ctx(curl_socket_t sockfd, global_t *g)
{
    curl_socket_ctx *context;
    context = (curl_socket_ctx *)malloc(sizeof(*context));
    context->sockfd = sockfd;
    context->event = event_new(g->base, sockfd, 0, curl_perform, context);
    return context;
}

static void destroy_socket_ctx(curl_socket_ctx *context)
{
    event_del(context->event);
    event_free(context->event);
    free(context);
}

static void request_done_cb(CURLMsg *message, void *arg)
{
    req_ctx_t *ctx = (req_ctx_t *)arg;
    printf("***************************** %s header(%ld) *********************************\n",ctx->url, ctx->hlen);
    printf("%s\n", ctx->hbuf);
    printf("***************************** %s body(%ld) *********************************\n", ctx->url, ctx->blen);
    printf("%s\n", ctx->bbuf);
}

static void destroy_req_ctx(req_ctx_t *ctx)
{
    free(ctx->bbuf);
    free(ctx->hbuf);
    free(ctx->url);
    ctx->done_cb = NULL;
    ctx->handle = NULL;
    free(ctx);
}

static void add_download(const char *url, global_t *g)
{
    req_ctx_t *ctx = malloc(sizeof(req_ctx_t));
    ctx->hbuf = malloc(sizeof(content_t) * 1024);
    memset(ctx->hbuf, 0, 1024);
    ctx->bbuf = malloc(sizeof(content_t) * 1024);
    memset(ctx->bbuf, 0, 1024);
    ctx->hlen = ctx->blen = 0;
    ctx->done_cb = request_done_cb;
    ctx->url = strdup(url);

    CURL *handle = curl_easy_init();
    ctx->handle = handle;
    ctx->ready_state = XHR_RSTATE_UNSENT;

    curl_easy_setopt(handle, CURLOPT_URL, ctx->url);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, ctx); // 主要写header数据的buf
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, ctx);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, ctx); // 提供request callback，在check_multi_info中获取private data，然后调用callback

    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, progress_cb);
    curl_easy_setopt(handle, CURLOPT_XFERINFODATA, ctx);

    curl_multi_add_handle(g->curl_handle, handle);
}

static void fifo_cb(int fd, short event, void *arg)
{
    char s[1024];
    long int rv = 0;
    int n = 0;
    global_t *g = (global_t *)arg;
    (void)fd;
    (void)event;
    
    do {
        s[0]='\0';
        rv = fscanf(g->input, "%1023s%n", s, &n);
        s[n]='\0';
        if(n && s[0]) 
        {
            if(!strcmp(s, "stop")) {
                g->stopped = 1;
                if(g->still_running == 0)
                    event_base_loopbreak(g->base);
            }
            else
                add_download(s, g);
        }
        else
        break;
    } while(rv != EOF);
}

static const char *fifo = "curl_libevent.fifo";
static int init_fifo(global_t *g)
{
    struct stat st;
    curl_socket_t sockfd;
    
    fprintf(MSG_OUT, "Creating named pipe \"%s\"\n", fifo);
    if(lstat(fifo, &st) == 0) {
        // if((st.st_mode & S_IFMT) == S_IFREG) {
        if(S_ISREG(st.st_mode)) {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }
    unlink(fifo);
    if(mkfifo(fifo, 0600) == -1) {
        perror("mkfifo");
        exit(1);
    }
    sockfd = open(fifo, O_RDWR | O_NONBLOCK, 0);
    if(sockfd == -1) {
        perror("open");
        exit(1);
    }
    g->input = fdopen(sockfd, "r");
    
    fprintf(MSG_OUT, "Now, pipe some URL's into > %s\n", fifo);
    g->fifo_event = event_new(g->base, sockfd, EV_READ | EV_PERSIST,
                fifo_cb, g);
    // event_assign(g->fifo_event, g->base, sockfd, EV_READ | EV_PERSIST,
                // fifo_cb, g);
    event_add(g->fifo_event, NULL);
    return (0);
}

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data) 
{
    const char status_line[] = "HTTP/";
    const char empty_line[] = "\r\n";
    size_t real_len = size * nmemb;
    req_ctx_t *ctx = (req_ctx_t *)data;
    if(strncmp(status_line, ptr, sizeof(status_line) - 1) == 0) // 第一行HTTP/2.0 OK
    {
    }
    else if(strncmp(empty_line, ptr, sizeof(empty_line) - 1) == 0) // header结束了
    {
        ctx->ready_state = XHR_RSTATE_HEADERS_RECEIVED;
        *(ctx->hbuf+ctx->hlen-2) = '\0';
        return real_len;
    }
    else
    {
        char *colon = memchr(ptr, ':', real_len);
        if(colon)
        {
            for(char *temp = ptr; temp != colon; temp++)
            {
                *temp = toupper(*temp);
            }
        }
    }
    if((ctx->hlen + real_len) < 1024)
    {
        memcpy(ctx->hbuf+ctx->hlen, (content_t *)ptr, real_len);
    }
    ctx->hlen += real_len;
    return real_len;
}

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    req_ctx_t *ctx = (req_ctx_t *)data;
    size_t real_len = size * nmemb;
    if(ctx->ready_state == XHR_RSTATE_HEADERS_RECEIVED)
    {
        ctx->ready_state = XHR_RSTATE_LOADING;
    }
    if((ctx->blen + real_len) < 1024)
    {
        memcpy(ctx->bbuf+ctx->blen, (content_t *)ptr, real_len);
        // *(ctx->bbuf+real_len) = '\0';
    }
    ctx->blen += real_len;
    return real_len;
}
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) 
{
    req_ctx_t *ctx = (req_ctx_t *)clientp;
    if(ctx->ready_state == XHR_RSTATE_LOADING)
    {
        curl_off_t cl = -1;
        curl_easy_getinfo(ctx->handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
        if(cl > 0) {
            printf("%s 可以获取下载内容总长度：%ld\n", ctx->url, cl);
        } 
        else 
        {
            printf("%s 不能获取下载内容总长度\n", ctx->url);
        }
        printf("loaded: %ld, total: %ld\n", dlnow, dltotal);
    }
    return 0;
}

static void check_multi_info(global_t *g)
{
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    req_ctx_t *ctx;
    char *done_url;

    while ((message = curl_multi_info_read(g->curl_handle, &pending)))
    {
        switch (message->msg)
        {
        case CURLMSG_DONE:
            /* Do not use message data after calling curl_multi_remove_handle() and
               curl_easy_cleanup(). As per curl_multi_info_read() docs:
               "WARNING: The data the returned pointer points to will not survive
               calling curl_multi_cleanup, curl_multi_remove_handle or
               curl_easy_cleanup." */
            easy_handle = message->easy_handle;

            curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &ctx);
            if(ctx == NULL)
            {
                fprintf(stderr, "%s request context is NULL", done_url);
            } 
            else 
            {
                ctx->done_cb(message, ctx);
            }

            curl_multi_remove_handle(g->curl_handle, easy_handle);
            curl_easy_cleanup(easy_handle);
            destroy_req_ctx(ctx);
            break;

        default:
            fprintf(stderr, "CURLMSG default\n");
            break;
        }
    }
    if(g->still_running == 0 && g->stopped)
    {
        event_base_loopbreak(g->base);
    }
}

static void curl_perform(int fd, short event, void *arg)
{
    int flags = 0;

    if (event & EV_READ)
        flags |= CURL_CSELECT_IN;
    if (event & EV_WRITE)
        flags |= CURL_CSELECT_OUT;

    global_t *global = (global_t *)arg;

    curl_multi_socket_action(global->curl_handle, fd, flags,
                             &global->still_running);

    check_multi_info(global);
    if(global->still_running <= 0)
    {
        if(evtimer_pending(global->timeout, NULL))
        {
            evtimer_del(global->timeout);
        }
    }
}

static void on_timeout(evutil_socket_t fd, short events, void *arg)
{
    global_t *global = (global_t *)arg;
    int running_handles;
    curl_multi_socket_action(global->curl_handle, CURL_SOCKET_TIMEOUT, 0,
                             &running_handles);
    check_multi_info(global);
}

static int start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    global_t *global = (global_t *)userp;
    // printf("curl超时回调: %ldms\n", timeout_ms);
    if (timeout_ms < 0)
    {
        evtimer_del(global->timeout);
    }
    else
    {
        if (timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                               in a bit */
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        evtimer_del(global->timeout);
        evtimer_add(global->timeout, &tv);
    }
    return 0;
}

static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp,
                         void *socketp)
{
    global_t *global = (global_t *)userp;
    curl_socket_ctx *socket_ctx;
    int events = 0;

    switch (action)
    {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
        socket_ctx = socketp ? (curl_socket_ctx *)socketp : create_socket_ctx(s, global);

        curl_multi_assign(global->curl_handle, s, (void *)socket_ctx);

        if (action != CURL_POLL_IN)
            events |= EV_WRITE;
        if (action != CURL_POLL_OUT)
            events |= EV_READ;

        events |= EV_PERSIST;

        event_del(socket_ctx->event);
        event_assign(socket_ctx->event, global->base, socket_ctx->sockfd, events,
                     curl_perform, global);
        event_add(socket_ctx->event, NULL);

        break;
    case CURL_POLL_REMOVE:
        if (socketp)
        {
            event_del(((curl_socket_ctx *)socketp)->event);
            destroy_socket_ctx((curl_socket_ctx *)socketp);
            curl_multi_assign(global->curl_handle, s, NULL);
        }
        break;
    default:
        abort();
    }

    return 0;
}

static void clean_fifo(global_t *g)
{
    event_del(g->fifo_event);
    fclose(g->input);
    unlink(fifo);
}

int main(int argc, char **argv)
{
    if (curl_global_init(CURL_GLOBAL_ALL))
    {
        fprintf(stderr, "Could not init curl\n");
        return 1;
    }
    global_t *g = malloc(sizeof(global_t));

    g->stopped = 0;
    g->still_running = 0;
    g->base = event_base_new();
    g->curl_handle = curl_multi_init();
    g->timeout = evtimer_new(g->base, on_timeout, g);
    init_fifo(g);

    curl_multi_setopt(g->curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
    curl_multi_setopt(g->curl_handle, CURLMOPT_SOCKETDATA, g);
    curl_multi_setopt(g->curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);
    curl_multi_setopt(g->curl_handle, CURLMOPT_TIMERDATA, g);

    event_base_dispatch(g->base);

    clean_fifo(g);
    curl_multi_cleanup(g->curl_handle);
    event_free(g->timeout);
    event_base_free(g->base);

    libevent_global_shutdown();
    curl_global_cleanup();

    free(g);
    return 0;
}