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

	jsB_initcurl(loop);

	js_setcontext(J, loop);
	return loop;
}