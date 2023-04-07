#include "jsi.h"
#include <string.h>

static size_t timer_id = 1;
static size_t get_timer_id() 
{
	return timer_id++;
}

static timer_ctx * new_timer_ctx(js_State *J,
	struct event_base *base, 
	struct event *ev)
{
	timer_ctx *ctx = malloc(sizeof(timer_ctx));
	ctx->J = J;
	ctx->base = base;
	ctx->ev = ev;
	ctx->id = get_timer_id();
	js_Loop *loop = (js_Loop *)js_getcontext(J);
	ctx->timer_next = loop->timer_list;
	loop->timer_list = ctx;
	return ctx;
}

static void timer_cb(int fd, short int flags, void *userdata)
{
	timer_ctx *ctx = (timer_ctx *)userdata;
	js_State *J = ctx->J;
	js_pushvalue(J, *ctx->func);
	js_pushundefined(J);
	for(size_t i = 0; i<ctx->argc; i++)
	{
		js_pushvalue(J, ctx->argv[i]);
	}
	js_call(J, ctx->argc);
}

// setTimeout(function(){}, milliseconds, args)
static void jsB_setTimeout(js_State *J)
{
	long millis;
	struct timeval *tv;
	js_Loop *loop;
	struct event_base *base;
	timer_ctx *ctx;
	int n; // 回调函数参数个数

	if (!js_iscallable(J, 1))
		js_typeerror(J, "callback is not a function");

	millis = (long)js_tonumber(J, 2);
	tv = malloc(sizeof(struct timeval));
	tv->tv_sec = millis / 1000;
	tv->tv_usec = (millis % 1000) * 1000;

	loop = (js_Loop *)js_getcontext(J);
	base = loop->base;
	ctx = new_timer_ctx(J, base, NULL);
	n = js_getlength(J, 1);
	ctx->argc = n < 0 ? 0 : n; // func, millis, arg1, arg2
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
	
	ctx->ev = event_new(base, 0, EV_TIMEOUT, timer_cb, ctx);
	event_add(ctx->ev, tv);

	// return timer id
	js_newnumber(J, ctx->id);
}

static void jsB_setInterval(js_State *J)
{
	long millis;
	struct timeval *tv;
	js_Loop *loop;
	struct event_base *base;
	timer_ctx *ctx;
	int n; // 回调函数参数个数

	if (!js_iscallable(J, 1))
		js_typeerror(J, "callback is not a function");

	millis = (long)js_tonumber(J, 2);
	tv = malloc(sizeof(struct timeval));
	tv->tv_sec = millis / 1000;
	tv->tv_usec = (millis % 1000) * 1000;

	loop = (js_Loop *)js_getcontext(J);
	base = loop->base;
	ctx = new_timer_ctx(J, base, NULL);
	n = js_getlength(J, 1);
	ctx->argc = n < 0 ? 0 : n; // func, millis, arg1, arg2
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
	
	ctx->ev = event_new(base, 0, EV_TIMEOUT | EV_PERSIST, timer_cb, ctx);
	event_add(ctx->ev, tv);

	// return timer id
	js_newnumber(J, ctx->id);
}

static void free_timer(timer_ctx *ctx)
{
	event_del(ctx->ev);
	ctx->J = NULL;
	ctx->base = NULL;
	ctx->ev = NULL;
	free(ctx->func);
	free(ctx->argv);
	free(ctx);
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
		loop->timer_list = loop->timer_list->timer_next;
		free_timer(ctx);
		return;
	}
	else
	{
		while(ctx->timer_next != NULL && ctx->timer_next->id != id)
		{
			ctx = ctx->timer_next;
		}
	}
	if(ctx->timer_next == NULL)	
		js_syntaxerror(J, "找不到id: %ld\n", id);
	timer_ctx *buf = ctx->timer_next;
	ctx->timer_next = ctx->timer_next->timer_next;
	free_timer(buf);
}

void jsB_inittimer(js_State *J)
{
	js_newcfunction(J, jsB_setTimeout, "setTimeout", 1);
	js_setglobal(J, "setTimeout");

	js_newcfunction(J, jsB_clearTimeout, "clearTimeout", 1);
	js_setglobal(J, "clearTimeout");

	js_newcfunction(J, jsB_setInterval, "setInterval", 1);
	js_setglobal(J, "setInterval");
}