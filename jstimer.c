#include "jsi.h"
#include <string.h>

static size_t timer_id = 1;
static size_t get_timer_id() 
{
	return timer_id++;
}

static void free_timer_ctx(timer_ctx *ctx)
{
	event_free(ctx->ev);
	ctx->J = NULL;
	ctx->base = NULL;
	ctx->ev = NULL;
	free(ctx->func);
	for(size_t i=0; i<ctx->argc; i++)
	{
		free(ctx->argv+i);
	}
	free(ctx);
}

static timer_ctx * new_timer_ctx(js_State *J,
	struct event_base *base, 
	struct event *ev,
	js_Loop *loop)
{
	timer_ctx *ctx = malloc(sizeof(timer_ctx));
	ctx->J = J;
	ctx->base = base;
	ctx->ev = ev;
	ctx->id = get_timer_id();
	ctx->next = loop->timer_list;
	loop->timer_list = ctx;
	return ctx;
}

static void timer_cb(int fd, short int flags, void *userdata)
{
	timer_ctx *ctx = (timer_ctx *)userdata;

	execute_jobs(ctx->J);

	js_State *J = ctx->J;
	js_pushvalue(J, *ctx->func);
	js_pushundefined(J);
	for(size_t i = 0; i<ctx->argc; i++)
	{
		js_pushvalue(J, ctx->argv[i]);
	}
	if(js_pcall(J, ctx->argc))
	{
		fprintf(stderr, "定时器任务运行报错, id: %ld\n", ctx->id);
	}
	if(!ctx->is_interval)
	{
		event_del(ctx->ev);
	}
}

static void register_timer(js_State *J, short is_interval)
{
	long millis;
	struct timeval tv = { 0, 4000 };
	js_Loop *loop;
	struct event_base *base;
	timer_ctx *ctx;
	int n = js_gettop(J) - 1; // 回调函数参数个数, 减去this, func, milliseconds

	if (!js_iscallable(J, 1))
		js_typeerror(J, "callback is not a function");

	if(n > 1)
	{
		millis = (long)js_tonumber(J, 2);
		tv.tv_sec = millis / 1000;
		tv.tv_usec = (millis % 1000) * 1000;
	}

	loop = js_getcontext(J);
	base = loop->base;
	ctx = new_timer_ctx(J, base, NULL, loop);
	ctx->argc = (n - 2) < 0 ? 0 : (n - 2); // func, millis, arg1, arg2
	// 后面timout后需要还原func和argvs
	if(ctx->argc > 0)
	{
		ctx->argv = malloc(sizeof(js_Value) * (ctx->argc));
		for(size_t i = 0; i < ctx->argc; i++)
		{
			memcpy(ctx->argv+i, js_tovalue(J, i+3), sizeof(js_Value));
		}
	}
	ctx->func = malloc(sizeof(js_Value));
	memcpy(ctx->func, js_tovalue(J, 1), sizeof(js_Value));

	short flags = EV_TIMEOUT;
	ctx->is_interval = 0;
	if(is_interval)	
	{
		flags |= EV_PERSIST;
		ctx->is_interval = 1;
	}
	ctx->ev = event_new(base, -1, flags, timer_cb, ctx);

	timer_ctx *tmp = ctx->next;
	int count = 0;
	ctx->time = millis;
	while(tmp)
	{
		if(tmp->time == millis)
		{
			count++;
		}
		tmp = tmp->next;	
	}
	tv.tv_usec += count;

	event_add(ctx->ev, &tv);

	// return timer id
	js_newnumber(J, ctx->id);
}

static void jsB_setTimeout(js_State *J)
{
	register_timer(J, 0);
}

static void jsB_setInterval(js_State *J)
{

	register_timer(J, 1);
}

static void jsB_clearTimeout(js_State *J)
{
	js_Loop *loop;
	timer_ctx *ctx;

	loop = (js_Loop *)js_getcontext(J);
	size_t id = (size_t)js_tonumber(J, 1);
	if(id < 1 || loop->timer_list == NULL)
	{
		js_syntaxerror(J, "id不能小于1或者全局链表没有timer注册");
	}
	ctx = loop->timer_list;
	if(ctx->id == id) // 第一个就匹配了
	{
		loop->timer_list = loop->timer_list->next;
		free_timer_ctx(ctx);
		return;
	}
	else
	{
		while(ctx->next != NULL && ctx->next->id != id)
		{
			ctx = ctx->next;
		}
	}
	if(ctx->next == NULL)	
		js_syntaxerror(J, "找不到id: %ld\n", id);
	timer_ctx *buf = ctx->next;
	ctx->next = ctx->next->next;
	free_timer_ctx(buf);
}

void jsB_inittimer(js_State *J)
{
	js_newcfunction(J, jsB_setTimeout, "setTimeout", 1);
	js_setglobal(J, "setTimeout");

	js_newcfunction(J, jsB_clearTimeout, "clearTimeout", 1);
	js_setglobal(J, "clearTimeout");

	js_newcfunction(J, jsB_setInterval, "setInterval", 1);
	js_setglobal(J, "setInterval");

	js_newcfunction(J, jsB_clearTimeout, "clearInterval", 1);
	js_setglobal(J, "clearInterval");
}