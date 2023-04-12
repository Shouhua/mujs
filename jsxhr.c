#include "jsi.h"

#define CONTENT_LEN 1024*1024

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data);
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data);
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
static void curl_perform(int fd, short event, void *arg);

char* strdup (const char* s)
{
  size_t slen = strlen(s);
  char* result = malloc(slen + 1);
  if(result == NULL)
  {
    return NULL;
  }

  memcpy(result, s, slen+1);
  return result;
}

static int start_timeout(CURLM *multi, long timeout_ms, void *userp)
{
    js_Loop *loop = (js_Loop *)userp;
    if (timeout_ms < 0)
    {
        evtimer_del(loop->curlm_timeout);
    }
    else
    {
        // if (timeout_ms == 0)
        //     timeout_ms = 1; /* 0 means directly call socket_action, but we will do it
                            //    in a bit */
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        evtimer_del(loop->curlm_timeout);
        evtimer_add(loop->curlm_timeout, &tv);
    }
    return 0;
}

static curl_socket_ctx *create_socket_ctx(curl_socket_t sockfd, js_Loop *loop)
{
    curl_socket_ctx *context;
    context = (curl_socket_ctx *)malloc(sizeof(*context));
    context->sockfd = sockfd;
    context->event = event_new(loop->base, sockfd, 0, curl_perform, context);
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
    js_Loop *loop = (js_Loop *)userp;
    curl_socket_ctx *socket_ctx;
    int events = 0;

    switch (action)
    {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
        socket_ctx = socketp ? (curl_socket_ctx *)socketp : create_socket_ctx(s, loop);

        curl_multi_assign(loop->multi_handle, s, (void *)socket_ctx);

        if (action != CURL_POLL_IN)
            events |= EV_WRITE;
        if (action != CURL_POLL_OUT)
            events |= EV_READ;

        events |= EV_PERSIST;

        event_del(socket_ctx->event);
        event_assign(socket_ctx->event, loop->base, s, events,
                     curl_perform, loop);
        event_add(socket_ctx->event, NULL);

        break;
    case CURL_POLL_REMOVE:
        if (socketp)
        {
            event_del(((curl_socket_ctx *)socketp)->event);
            destroy_socket_ctx((curl_socket_ctx *)socketp);
            curl_multi_assign(loop->multi_handle, s, NULL);
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
    free(ctx);
}

static void check_multi_info(js_Loop *loop)
{
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    req_ctx *ctx;
    char *done_url;

    while ((message = curl_multi_info_read(loop->multi_handle, &pending)))
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
                ctx->done_cb(message->data.result, ctx);
            }

            curl_multi_remove_handle(loop->multi_handle, easy_handle);
            // curl_easy_cleanup(easy_handle);
            curl_easy_reset(easy_handle);
            destroy_req_ctx(ctx);
            loop->easy_handle = NULL;
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

    js_Loop *loop = (js_Loop *)arg;

    curl_multi_socket_action(loop->multi_handle, fd, flags,
                             &still_running);

    check_multi_info(loop);
    if(still_running <= 0)
    {
        if(evtimer_pending(loop->curlm_timeout, NULL))
        {
            evtimer_del(loop->curlm_timeout);
        }
    }
}

static void maybe_emit_event(req_ctx *ctx, short event)
{
    if(ctx->events[event])
    {
        js_pushobject(ctx->J, ctx->events[event]);
        js_pushundefined(ctx->J);
        if(js_pcall(ctx->J, 0))
        {
            fprintf(stderr, "%s: %d\n", "运行事件出现问题", event);
            js_pop(ctx->J, 1);
            return;
        }
    }
}

static void curlm_timeout(int fd,
	short flags,
	void *userdata)
{
    js_Loop *loop = (js_Loop *)userdata;
    int running_handles;
    curl_multi_socket_action(loop->multi_handle, CURL_SOCKET_TIMEOUT, 0,
                             &running_handles);
    check_multi_info(loop);
}

static size_t header_cb(char *ptr, size_t size, size_t nmemb, void *data) 
{
    const char status_line[] = "HTTP/";
    const char empty_line[] = "\r\n";
    size_t real_len = size * nmemb;
    req_ctx *ctx = (req_ctx *)data;
    if(strncmp(status_line, ptr, sizeof(status_line) - 1) == 0) // 第一行HTTP/2.0 OK
    {
        if(ctx->hlen == 0)
        {
            maybe_emit_event(ctx, XHR_EVENT_LOAD_START);
        }
        else
        {
            free(ctx->hbuf);
            ctx->hbuf = realloc(ctx->hbuf, CONTENT_LEN);
        }
    }
    else if(strncmp(empty_line, ptr, sizeof(empty_line) - 1) == 0) // header结束了
    {
        long code = -1;
        curl_easy_getinfo(ctx->loop->easy_handle, CURLINFO_RESPONSE_CODE, &code);
        if(code > -1 && code / 100 != 3) // redirect
        {
            ctx->ready_state = XHR_RSTATE_HEADERS_RECEIVED;
            maybe_emit_event(ctx, XHR_EVENT_READY_STATE_CHANGED);
        }
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
                *temp = tolower(*temp);
            }
        }
    }
    if((ctx->hlen + real_len) > CONTENT_LEN)
    {
        ctx->hbuf = realloc(ctx->hbuf, ctx->hlen + real_len);
    }
    memcpy(ctx->hbuf+ctx->hlen, (content_t *)ptr, real_len);
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
        maybe_emit_event(ctx, XHR_EVENT_READY_STATE_CHANGED);
    }
    if((ctx->blen + real_len) > CONTENT_LEN)
    {
        ctx->bbuf = realloc(ctx->bbuf, ctx->blen + real_len);
    }
    memcpy(ctx->bbuf+ctx->blen, (content_t *)ptr, real_len);
    ctx->blen += real_len;
    return real_len;
}
static int progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) 
{
    req_ctx *ctx = (req_ctx *)clientp;
    if(ctx->ready_state == XHR_RSTATE_LOADING)
    {
        // curl_off_t cl = -1;
        // curl_easy_getinfo(ctx->handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
        /*
        // js define property event.lengthComputable = cl 
        // js define property event.loaded = dlnow 
        // js define property event.total = dltotal 
        // emit XHR_EVENT_PROGRESS
        */
        // if(cl > 0) {
        //     printf("%s 可以获取下载内容总长度：%ld\n", ctx->url, cl);
        // } 
        // else 
        // {
        //     printf("%s 不能获取下载内容总长度\n", ctx->url);
        // }
        // printf("loaded: %ld, total: %ld\n", dlnow, dltotal);
    }
    return 0;
}

static void request_done_cb(CURLcode result, void *arg)
{
    req_ctx *ctx = (req_ctx *)arg;
    const char *done_url = NULL;
    curl_easy_getinfo(ctx->loop->easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
    if(done_url)
    {
        free(ctx->url);
        ctx->url = strdup(done_url);
    }
    ctx->ready_state = XHR_RSTATE_DONE;
    maybe_emit_event(ctx, XHR_EVENT_READY_STATE_CHANGED);

    if(result == CURLE_OPERATION_TIMEDOUT)
        maybe_emit_event(ctx, XHR_EVENT_TIMEOUT);
    
    maybe_emit_event(ctx, XHR_EVENT_LOAD_END);
    if(result != CURLE_OPERATION_TIMEDOUT)
    {
        if(result != CURLE_OK)
        {
            fprintf(stderr, "下载出现问题：%d\n", result);
            maybe_emit_event(ctx, XHR_EVENT_ERROR);
        }
        else
            maybe_emit_event(ctx, XHR_EVENT_LOAD);
    }
}

static req_ctx * register_request(const char *method, 
	const char *url,
	int async,
	js_State *J)
{
	js_Loop *loop = js_getcontext(J);
    req_ctx *ctx = malloc(sizeof(req_ctx));

    ctx->J = J;
    ctx->loop = loop;

    ctx->hlen = ctx->blen = 0;
    ctx->hbuf = malloc(sizeof(content_t) * CONTENT_LEN);
    memset(ctx->hbuf, 0, CONTENT_LEN);
    ctx->bbuf = malloc(sizeof(content_t) * CONTENT_LEN);
    memset(ctx->bbuf, 0, CONTENT_LEN);
    ctx->done_cb = request_done_cb;
    ctx->url = strdup(url);
	ctx->method = strdup(method);
	ctx->async = async;
    ctx->sent = 0;
    ctx->timeout = 0;
    ctx->response_type = XHR_RTYPE_DEFAULT; // default means text

    ctx->ready_state = XHR_RSTATE_UNSENT;
    for(int i = 0; i<XHR_EVENT_MAX; i++)
    {
        ctx->events[i] = NULL;
    }

    curl_easy_setopt(loop->easy_handle, CURLOPT_URL, ctx->url);
    curl_easy_setopt(loop->easy_handle, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(loop->easy_handle, CURLOPT_HEADERDATA, ctx); // 主要写header数据的buf
    curl_easy_setopt(loop->easy_handle, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(loop->easy_handle, CURLOPT_WRITEDATA, ctx);
    curl_easy_setopt(loop->easy_handle, CURLOPT_PRIVATE, ctx); // 提供request callback，在check_multi_info中获取private data，然后调用callback

    curl_easy_setopt(loop->easy_handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(loop->easy_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(loop->easy_handle, CURLOPT_XFERINFOFUNCTION, progress_cb);
    curl_easy_setopt(loop->easy_handle, CURLOPT_XFERINFODATA, ctx);

    if(async) // 只在异步情况下使用timeout
    {
        curl_easy_setopt(loop->easy_handle, CURLOPT_TIMEOUT_MS, 0L); // 设置默认值
        curl_easy_setopt(loop->easy_handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L); // 设置连接超时默认值, curl默认值300s
    }

	ctx->ready_state = XHR_RSTATE_OPENED;
	return ctx;
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
    int n = js_gettop(J) - 1;
	js_Object *self = js_toobject(J, 0); // this
	int async;	
	const char *method = js_tostring(J, 1);
	const char *url = js_tostring(J, 2);
	if(n == 3)
	{
		async = js_toboolean(J, 3);
	}
	req_ctx * ctx = register_request(method, url, async, J);
    self->u.c.data = ctx;
}

static void Xp_onprogress_setter(js_State *J)
{
    js_Object *self = js_toobject(J, 0);
    js_Object *func = js_toobject(J, 1);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    ctx->events[XHR_EVENT_PROGRESS] = func;
}

static void Xp_ontimeout_setter(js_State *J)
{
    js_Object *self = js_toobject(J, 0);
    js_Object *func = js_toobject(J, 1);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    ctx->events[XHR_EVENT_TIMEOUT] = func;
}

static void Xp_onload_setter(js_State *J)
{
    js_Object *self = js_toobject(J, 0);
    js_Object *func = js_toobject(J, 1);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    ctx->events[XHR_EVENT_LOAD] = func;
}

static void Xp_send(js_State *J)
{
    js_Loop *loop = js_getcontext(J);
	js_Object *self = js_toobject(J, 0); // this
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    if(ctx->async)
    {
        curl_multi_add_handle(loop->multi_handle, loop->easy_handle);
    }
    else
    {
        CURLcode result = curl_easy_perform(loop->easy_handle);
        request_done_cb(result, ctx);
    }
    ctx->sent = 1;
}

static void Xp_response_type_getter(js_State *J)
{
    char *str;
	js_Object *self = js_toobject(J, 0); // this
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    switch(ctx->response_type)
    {
        case XHR_RTYPE_DEFAULT:
            str = "";
            break;
        case XHR_RTYPE_TEXT:
            str = "text";
            break;
        case XHR_RTYPE_ARRAY_BUFFER:
            str = "arraybuffer";
            break;
        case XHR_RTYPE_JSON:
            str = "json";
            break;
        default:
            js_typeerror(J, "response type码不对：%d\n", ctx->response_type);
    }
    js_pushstring(J, str);
}
static void Xp_response_type_setter(js_State *J)
{
    static const char array_buffer[] = "arraybuffer";
    static const char json[] = "json";
    static const char text[] = "text";

	js_Object *self = js_toobject(J, 0); // this
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    const char *v = js_tostring(J, -1);
    if(v)
    {
        if(strncmp(array_buffer, v, sizeof(array_buffer)) - 1 == 0)
        {
            ctx->response_type = XHR_RTYPE_ARRAY_BUFFER;
        }        
        else if (strncmp(json, v, sizeof(json) - 1) == 0)
            ctx->response_type = XHR_RTYPE_JSON;
        else if (strncmp(text, v, sizeof(text) - 1) == 0)
            ctx->response_type = XHR_RTYPE_TEXT;
        else if (strlen(v) == 0)
            ctx->response_type = XHR_RTYPE_DEFAULT;
    }
    js_pushundefined(J);
}

static void Xp_response_getter(js_State *J)
{
	js_Object *self = js_toobject(J, 0); // this
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    if(ctx->blen > 0)
    {
        switch(ctx->response_type)
        {
            case XHR_RTYPE_DEFAULT:
            case XHR_RTYPE_TEXT:
                js_pushstring(J, ctx->bbuf);
                break;
            case XHR_RTYPE_JSON:
                js_getglobal(J, "JSON");
                js_getproperty(J, -1, "parse");
                js_rot2(J);
                js_pushstring(J, ctx->bbuf);
                if (js_pcall(J, 1)) {
                    fprintf(stderr, "解析json出问题了");
                    js_pop(J, 1);
                    return;
                }
                break;
            case XHR_RTYPE_ARRAY_BUFFER:
                // TODO
            default:
                js_typeerror(J, "response type不符合： %d\n", ctx->response_type);
        }
    }
}

static void Xp_timeout_setter(js_State *J)
{
    js_Object *self = js_toobject(J, 0);
    long timeout = js_tonumber(J, -1);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    if(!ctx->async)
    {
        fprintf(stderr, "timeout设置不能用于sync模式下");
    }
    if(!ctx->sent)
    {
        curl_easy_setopt(ctx->loop->easy_handle, CURLOPT_TIMEOUT_MS, &timeout);
        ctx->timeout = timeout;
    }
    else
    {
        fprintf(stderr, "timeout设置要在send之前设置");
    }
}

static void Xp_timeout_getter(js_State *J)
{
    js_Object *self = js_toobject(J, 0);
    req_ctx *ctx = (req_ctx *)self->u.c.data;
    js_pushnumber(J, ctx->timeout);
}

void jsB_initxhr(js_State *J)
{
	js_pushobject(J, J->Xhr_prototype);
	{
		jsB_propf(J, "XMLHttpRequest.prototype.open", Xp_open, 2); // 2代表最小参数个数
		jsB_propf(J, "XMLHttpRequest.prototype.send", Xp_send, 0);

        js_newcfunction(J, Xp_response_getter, "response", 0);
        js_pushnull(J);
        js_defaccessor(J, -3, "response", JS_READONLY);

        js_newcfunction(J, Xp_response_type_getter, "response_type_getter", 0);
        js_newcfunction(J, Xp_response_type_setter, "response_type_seter", 1);
        js_defaccessor(J, -3, "responseType", JS_DONTENUM);

        js_pushnull(J);
        js_newcfunction(J, Xp_onload_setter, "onload_setter", 1);
        js_defaccessor(J, -3, "onload", JS_DONTCONF | JS_DONTENUM);
        
        js_pushnull(J);
        js_newcfunction(J, Xp_ontimeout_setter, "ontimeout_setter", 1);
        js_defaccessor(J, -3, "ontimeout", JS_DONTCONF | JS_DONTENUM);

        js_pushnull(J);
        js_newcfunction(J, Xp_onprogress_setter, "onprogress_setter", 1);
        js_defaccessor(J, -3, "onprogress", JS_DONTCONF | JS_DONTENUM);

        js_newcfunction(J, Xp_timeout_getter, "timeout_getter", 0);
        js_newcfunction(J, Xp_timeout_setter, "timeout_setter", 1);
        js_defaccessor(J, -3, "timeout", JS_DONTCONF | JS_DONTENUM);
	}
	js_newcconstructor(J, jsB_new_XMLHttpRequest, jsB_new_XMLHttpRequest, "XMLHttpRequest", 0);
    js_defglobal(J, "XMLHttpRequest", JS_DONTENUM);
}

void jsB_initcurl(js_Loop *loop)
{
    if(!loop->multi_handle)
    {
        loop->multi_handle = curl_multi_init();
        loop->curlm_timeout = evtimer_new(loop->base, curlm_timeout, loop);

        curl_multi_setopt(loop->multi_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
        curl_multi_setopt(loop->multi_handle, CURLMOPT_SOCKETDATA, loop);
        curl_multi_setopt(loop->multi_handle, CURLMOPT_TIMERFUNCTION, start_timeout);
        curl_multi_setopt(loop->multi_handle, CURLMOPT_TIMERDATA, loop);
    }
    if(!loop->easy_handle)
    {
        loop->easy_handle = curl_easy_init();
    }
}

void js_freexhr(js_Loop *loop)
{
    if(loop->multi_handle)
        curl_multi_cleanup(loop->multi_handle);
    if(loop->easy_handle)
        curl_easy_cleanup(loop->easy_handle);
}