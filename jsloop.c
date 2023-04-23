#include "jsi.h"

void js_freeloop(js_Loop *loop)
{
	timer_ctx *tmp;
	while(loop->timer_list)
	{
		tmp = loop->timer_list;
		loop->timer_list = loop->timer_list->next;
		// free argv and func
		for(size_t i = 0; i < tmp->argc; i++)
		{
			free(tmp->argv+i);
		}
		free(tmp->func);
		free(tmp);
	}
	js_freexhr(loop);

	curl_global_cleanup();
	event_base_free(loop->base);
	libevent_global_shutdown();
}

js_Loop *js_newloop(js_State *J)
{
	js_Loop *loop = malloc(sizeof(js_Loop));
	loop->J = J;
	loop->base = event_base_new();
	// event_base_priority_init(loop->base, 3);
	loop->timer_list = NULL;
	loop->micro_list = NULL;
	curl_global_init(CURL_GLOBAL_ALL);

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
void js_runloop(js_Loop *loop, char *filename)
{
	loop->filename = filename;
	struct timeval tv = { 0, 0 };
	event_base_once(loop->base, -1, EV_TIMEOUT, once_cb, loop, &tv);
	// js_dofile(loop->J, loop->filename);
	event_base_dispatch(loop->base);
}