#include "jsi.h"

void js_freeloop(js_Loop *loop)
{
	js_freetimer(loop);
	js_freexhr(loop);
	js_freetask(loop);

	event_base_free(loop->base);
	libevent_global_shutdown();
}

js_Loop *js_newloop(js_State *J, char *filename)
{
	js_Loop *loop = malloc(sizeof(js_Loop));
	loop->J = J;
	loop->base = event_base_new();
	loop->filename = filename;

	loop->timer_id = 0;
	init_list_head(&loop->timer_list);

	jsB_initcurl(loop);

	jsB_initjob(loop);

	js_setcontext(J, loop);
	return loop;
}

void once_cb(int fd, short flags, void *userdata)
{
	js_Loop *loop = (js_Loop *)userdata;
	js_dofile(loop->J, loop->filename);
}
void js_runloop(js_Loop *loop)
{
	struct timeval tv = { 0, 0 };
	event_base_once(loop->base, -1, EV_TIMEOUT, once_cb, loop, &tv);
	event_base_dispatch(loop->base);
}