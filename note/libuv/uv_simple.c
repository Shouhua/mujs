#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;

void cb(uv_timer_t *handle)
{
	(void)handle;
	printf("hello again...\n");
	uv_close((uv_handle_t *)handle, NULL);
}

void timer_cb(uv_timer_t* handle)
{
	(void)handle;
	printf("timer_cb\n");

	uv_timer_t t1;
	uv_timer_init(loop, &t1);
	uv_timer_start(&t1, cb, 500, 0);

	uv_timer_t t2;
	uv_timer_init(loop, &t2);
	uv_timer_start(&t2, cb, 500, 0);
	uv_close((uv_handle_t *)handle, NULL);
}

int main()
{
	loop = uv_default_loop();

	uv_timer_t t1;
	uv_timer_init(loop, &t1);
	uv_timer_start(&t1, timer_cb, 500, 0);
	uv_run(loop, UV_RUN_DEFAULT);
	uv_loop_close(loop);
	return 0;
}