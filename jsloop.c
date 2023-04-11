#include "jsi.h"

void js_freeloop(js_Loop *loop)
{
	curl_global_cleanup();
	event_base_free(loop->base);
	libevent_global_shutdown();
}

void js_runloop(js_Loop *loop)
{
	event_base_dispatch(loop->base);
}

js_Loop *js_newloop(js_State *J)
{
	js_Loop *loop = malloc(sizeof(js_Loop));
	loop->base = event_base_new();
	loop->timer_list = NULL;
	curl_global_init(CURL_GLOBAL_ALL);

	js_setcontext(J, loop);
	return loop;
}