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

void execute_jobs(js_State *J)
{
	js_Loop *loop = js_getcontext(J);
	micro_task *task, *tmp;
	task = loop->micro_list;
	while(task)
	{
		js_pushobject(J, task->fn);
		js_pushundefined(J);
		if(js_pcall(J, 0))
		{
			fprintf(stderr, "%s\n", js_trystring(J, -1, "Error"));
			js_pop(J, 1);
		}
		tmp = task;
		task = task->next;
		free(tmp->fn);
		free(tmp);
	}
	loop->micro_list_tail = NULL;
	loop->micro_list = NULL;
	// evtimer_del(loop->micro_event);
	// loop->is_micro_event_added = 0;
}

void micro_cb(int fd, short flags, void *userdata)
{
	js_State *J = (js_State *)userdata;
	execute_jobs(J);
}

static void jsB_queueMicrotask(js_State *J)
{
	js_Loop *loop;
	int top;
	micro_task *task, *task_tmp;
	loop = js_getcontext(J);
	top = js_gettop(J);
	if(top < 2)
	{
		js_typeerror(J, "需要一个函数作为参数");
	}
	if(!js_iscallable(J, -1))
	{
		js_typeerror(J, "第一个参数不是函数");
	}
	js_Object *fn = js_toobject(J, -1);
	task = malloc(sizeof(micro_task));
	task->fn = malloc(sizeof(js_Object));
	memcpy(task->fn, fn, sizeof(js_Object));
	task->next = NULL;
	if(!loop->micro_list)
	{
		loop->micro_list = task;
		loop->micro_list_tail = task;
		// 设置尾巴
	}
	else
	{
		task_tmp = loop->micro_list_tail;
		task_tmp->next = task;
		loop->micro_list_tail = task;
		// while(task_tmp && task_tmp->next)
		// {
		// 	task_tmp = task_tmp->next;
		// }
		// task_tmp->next = task;	
	}
	// if(!loop->is_micro_event_added)
	// {
	// 	struct timeval tv;
	// 	tv.tv_sec = 0;
	// 	tv.tv_usec = 0;
	// 	// event_priority_set(loop->micro_event, 0);
	// 	evtimer_add(loop->micro_event, &tv);
	// 	loop->is_micro_event_added = 1;
	// }
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	evtimer_del(loop->micro_event);
	evtimer_add(loop->micro_event, &tv);
	// loop->is_micro_event_added = 1;

	js_pushundefined(J);
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

	loop->micro_event = event_new(loop->base, -1, EV_TIMEOUT | EV_PERSIST, micro_cb, J);

	js_newcfunction(J, jsB_queueMicrotask, "queueMicrotask", 0);
	js_setglobal(J, "queueMicrotask");

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
	event_base_dispatch(loop->base);
}