#include "jsi.h"

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data);
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data);
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
static void curl_perform(int fd, short event, void *arg);

static int start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    js_Loop *global = (js_Loop *)userp;
    if (timeout_ms < 0)
    {
        evtimer_del(global->curlm_timeout);
    }
    else
    {
        if (timeout_ms == 0)
            timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                               in a bit */
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        evtimer_del(global->curlm_timeout);
        evtimer_add(global->curlm_timeout, &tv);
    }
    return 0;
}

static curl_socket_ctx *create_socket_ctx(curl_socket_t sockfd, js_Loop *g)
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

static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp,
                         void *socketp)
{
    js_Loop *global = (js_Loop *)userp;
    curl_socket_ctx *socket_ctx;
    int events = 0;

    switch (action)
    {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
        socket_ctx = socketp ? (curl_socket_ctx *)socketp : create_socket_ctx(s, global);

        curl_multi_assign(global->curlm_handle, s, (void *)socket_ctx);

        if (action != CURL_POLL_IN)
            events |= EV_WRITE;
        if (action != CURL_POLL_OUT)
            events |= EV_READ;

        events |= EV_PERSIST;

        event_del(socket_ctx->event);
        event_assign(socket_ctx->event, global->base, s, events,
                     curl_perform, global);
        event_add(socket_ctx->event, NULL);

        break;
    case CURL_POLL_REMOVE:
        if (socketp)
        {
            event_del(((curl_socket_ctx *)socketp)->event);
            destroy_socket_ctx((curl_socket_ctx *)socketp);
            curl_multi_assign(global->curlm_handle, s, NULL);
        }
        break;
    default:
        abort();
    }

    return 0;
}

static void destroy_req_ctx(req_ctx *ctx)
{
    free(ctx->bbuf);
    free(ctx->hbuf);
    free(ctx->url);
	free(ctx->method);
    ctx->done_cb = NULL;
    ctx->handle = NULL;
    free(ctx);
}

static void check_multi_info(js_Loop *g)
{
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    req_ctx *ctx;
    char *done_url;

    while ((message = curl_multi_info_read(g->curlm_handle, &pending)))
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

            curl_multi_remove_handle(g->curlm_handle, easy_handle);
            curl_easy_cleanup(easy_handle);
            destroy_req_ctx(ctx);
            break;

        default:
            fprintf(stderr, "CURLMSG default\n");
            break;
        }
    }
}

static void curl_perform(int fd, short event, void *arg)
{
	int still_running;
    int flags = 0;

    if (event & EV_READ)
        flags |= CURL_CSELECT_IN;
    if (event & EV_WRITE)
        flags |= CURL_CSELECT_OUT;

    js_Loop *global = (js_Loop *)arg;

    curl_multi_socket_action(global->curlm_handle, fd, flags,
                             &still_running);

    check_multi_info(global);
    if(still_running <= 0)
    {
        if(evtimer_pending(global->curlm_timeout, NULL))
        {
            evtimer_del(global->curlm_timeout);
        }
    }
}

static void curlm_timeout(struct event_base *base,
	int fd,
	short flags,
	void *userdata)
{
    js_Loop *global = (js_Loop *)userdata;
    int running_handles;
    curl_multi_socket_action(global->curlm_handle, CURL_SOCKET_TIMEOUT, 0,
                             &running_handles);
    check_multi_info(global);
}

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data) 
{
    const char status_line[] = "HTTP/";
    const char empty_line[] = "\r\n";
    size_t real_len = size * nmemb;
    req_ctx *ctx = (req_ctx *)data;
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
    req_ctx *ctx = (req_ctx *)data;
    size_t real_len = size * nmemb;
    if(ctx->ready_state == XHR_RSTATE_HEADERS_RECEIVED)
    {
        ctx->ready_state = XHR_RSTATE_LOADING;
    }
    if((ctx->blen + real_len) < 1024)
    {
        memcpy(ctx->bbuf+ctx->blen, (content_t *)ptr, real_len);
    }
    ctx->blen += real_len;
    return real_len;
}
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) 
{
    req_ctx *ctx = (req_ctx *)clientp;
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

static void request_done_cb(CURLMsg *message, void *arg)
{
    req_ctx *ctx = (req_ctx *)arg;
    printf("***************************** %s header(%ld) *********************************\n",ctx->url, ctx->hlen);
    printf("%s\n", ctx->hbuf);
    printf("***************************** %s body(%ld) *********************************\n", ctx->url, ctx->blen);
    printf("%s\n", ctx->bbuf);
}

static req_ctx * register_request(const char *method, 
	const char *url,
	int async,
	js_State *J)
{
	js_Loop *g = (js_Loop *)js_getcontext(J);
    req_ctx *ctx = malloc(sizeof(req_ctx));
    ctx->hbuf = malloc(sizeof(content_t) * 1024);
    memset(ctx->hbuf, 0, 1024);
    ctx->bbuf = malloc(sizeof(content_t) * 1024);
    memset(ctx->bbuf, 0, 1024);
    ctx->hlen = ctx->blen = 0;
    ctx->done_cb = request_done_cb;
    ctx->url = strdup(url);
	ctx->method = strdup(method);
	ctx->async = async;

    CURL *handle = curl_easy_init();
    ctx->handle = handle;
    ctx->ready_state = XHR_RSTATE_UNSENT;
    ctx->curlm_handle = g->curlm_handle;
	for(int i = 0; i<XHR_EVENT_MAX; i++)
	{
		ctx->events[i] = NULL;
	}

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

	ctx->ready_state = XHR_RSTATE_OPENED;
	return ctx;
}

static void Xp_toString(js_State *J)
{
	// TODO
	// print '[object XMLHttpRequest]'
}

static void jsB_new_XMLHttpRequest(js_State *J)
{
	js_Object *obj;
	obj = jsV_newobject(J, JS_CXHR, J->Xhr_prototype);
	js_pushobject(J, obj);
}

static void Xp_open(js_State *J)
{
	// TODO xhr.open(method, url, isAsync, user, passwd)
	// js_Object *o1 = js_toobject(J, 1); // argv1
	js_Object *self = js_toobject(J, 0); // this
	int async;	
	int n = js_getlength(J, 0);
	char *method = js_tostring(J, 1);
	char *url = js_tostring(J, 2);
	if(n = 3) 
	{
		async = js_toboolean(J, 3);
	}
	req_ctx * ctx = register_request(method, url, async, J);
    self->u.c.data = ctx;
}

static void Xp_onload(js_State *J)
{
    // 暂时使用方法 
    js_Object *self = js_toobject(J, 0);
    js_Object *func = js_toobject(J, 1);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    ctx->events[XHR_EVENT_LOAD] = func;
}

static void Xp_send(js_State *J)
{
	js_Object *self = js_toobject(J, 0); // this
    req_ctx *ctx = (req_ctx *)self->u.c.data;
	// 如何获取当前curl easy handle
	// if not async use curl easy handle
	// if async curl multi handle add easy handle
	curl_multi_add_handle(ctx->curlm_handle, ctx->handle);
}

void jsB_initxhr(js_State *J)
{
	js_Loop *loop = (js_Loop *)js_getcontext(J);
	loop->curlm_handle = curl_multi_init();
	loop->curlm_timeout = evtimer_new(loop->base, curlm_timeout, loop);

	curl_multi_setopt(loop->curlm_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
    curl_multi_setopt(loop->curlm_handle, CURLMOPT_SOCKETDATA, loop);
    curl_multi_setopt(loop->curlm_handle, CURLMOPT_TIMERFUNCTION, start_timeout);
    curl_multi_setopt(loop->curlm_handle, CURLMOPT_TIMERDATA, loop);

	js_pushobject(J, J->Xhr_prototype);
	{
		jsB_propf(J, "XMLHttpRequest.prototype.toString", Xp_toString, 0);
		jsB_propf(J, "XMLHttpRequest.prototype.open", Xp_open, 2); /* 1 */
		jsB_propf(J, "XMLHttpRequest.prototype.onload", Xp_onload, 0); /* 1 */
		jsB_propf(J, "XMLHttpRequest.prototype.send", Xp_send, 0);
	}
	js_newcconstructor(J, jsB_new_XMLHttpRequest, jsB_new_XMLHttpRequest, "XMLHttpRequest", 0);

}