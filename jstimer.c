#include "jsi.h"

static void free_timer_ctx(timer_ctx *ctx)
{
	struct list_head *el, *el1;

	list_del(&ctx->link);
	event_free(ctx->ev);
	ctx->ev = NULL;
	ctx->J = NULL;
	ctx->base = NULL;
	list_for_each_safe(el, el1, &ctx->argv_list)	
	{
		list_del(el);
		timer_argv *v = list_entry(el, timer_argv, link);
		free(v->v);
		free(v);
	}
	free(ctx->func);
	free(ctx);
}

void js_freetimer(js_Loop *loop)
{
	struct list_head *el, *el1;
	list_for_each_safe(el, el1, &loop->timer_list)
	{
		timer_ctx *ctx = list_entry(el, timer_ctx, link);
		free_timer_ctx(ctx);
	}
}

static timer_ctx * new_timer_ctx(
	js_State *J,
	struct event_base *base, 
	struct event *ev,
	js_Loop *loop)
{
	timer_ctx *ctx = malloc(sizeof(timer_ctx));
	ctx->J = J;
	ctx->base = base;
	ctx->ev = ev;
	ctx->id = ++loop->timer_id;
	init_list_head(&ctx->argv_list);	
	list_add_tail(&ctx->link, &loop->timer_list);
	return ctx;
}

static void timer_cb(int fd, short int flags, void *userdata)
{
	struct list_head *el, *el1;
	timer_ctx *ctx;

	ctx = (timer_ctx *)userdata;

	execute_jobs(ctx->J);

	js_State *J = ctx->J;
	js_pushvalue(J, *ctx->func);
	js_pushundefined(J);

	list_for_each_safe(el, el1, &ctx->argv_list)
	{
		timer_argv *v = list_entry(el, timer_argv, link);
		js_pushvalue(J, *(v->v));
	}

	if(js_pcall(J, ctx->argc))
	{
		fprintf(stderr, "定时器任务运行报错, id: %ld\n", ctx->id);
	}
	if(!ctx->is_interval)
	{
		event_del(ctx->ev);
		free_timer_ctx(ctx);
	}
}

static void register_timer(js_State *J, short is_interval)
{
	struct list_head *el, *el1;
	int count;
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

	for(size_t i = 0; i < ctx->argc; i++)
	{
		timer_argv *v = malloc(sizeof(timer_argv));
		v->v = malloc(sizeof(js_Value));
		memcpy(v->v, js_tovalue(J, i+3), sizeof(js_Value));
		list_add_tail(&v->link, &ctx->argv_list);
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

	ctx->time = millis;

	count = 0;
	list_for_each_safe(el, el1, &loop->timer_list)
	{
		ctx = list_entry(el, timer_ctx, link);
		if(ctx->time == millis) 
		{
			count++;
		}
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
	struct list_head *el, *el1;

	loop = (js_Loop *)js_getcontext(J);
	size_t id = (size_t)js_tonumber(J, 1);

	list_for_each_safe(el, el1, &loop->timer_list)
	{
		ctx = list_entry(el, timer_ctx, link);
		if(ctx->id == id) 
		{
			free_timer_ctx(ctx);
			return;
		}
	}
	js_syntaxerror(J, "id不能小于1或者全局链表没有timer注册");
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