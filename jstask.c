#include "jsi.h"

void free_task(micro_task *task)
{
	list_del(&task->link);
	free(task->fn);
	free(task);
}

void js_freetask(js_Loop *loop)
{
	struct list_head *el, *el1;
	list_for_each_safe(el, el1, &loop->micro_list)
	{
		micro_task *task = list_entry(el, micro_task, link);
		free_task(task);
	}
}

void execute_jobs(js_State *J)
{
	struct list_head *el, *el1;
	micro_task *task;
	js_Loop *loop;

	loop = js_getcontext(J);
	list_for_each_safe(el, el1, &loop->micro_list)
	{
		task = list_entry(el, micro_task, link);
		js_pushobject(J, task->fn);
		js_pushundefined(J);
		if(js_pcall(J, 0))
		{
			fprintf(stderr, "%s\n", js_trystring(J, -1, "Error"));
			js_pop(J, 1);
		}
		free_task(task);
	}
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
	micro_task *task;
	struct timeval tv;

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
	list_add_tail(&task->link, &loop->micro_list);

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	evtimer_del(loop->micro_event);
	evtimer_add(loop->micro_event, &tv);

	js_pushundefined(J);
}

void jsB_inittask(js_State *J)
{
	js_newcfunction(J, jsB_queueMicrotask, "queueMicrotask", 0);
	js_setglobal(J, "queueMicrotask");
}

void jsB_initjob(js_Loop *loop)
{
	loop->micro_event = event_new(loop->base, -1, EV_TIMEOUT | EV_PERSIST, micro_cb, loop->J);
	init_list_head(&loop->micro_list);
}