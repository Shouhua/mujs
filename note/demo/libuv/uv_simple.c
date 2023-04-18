#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;

void cb(uv_timer_t *handle)
{
	printf("hello again...\n");
}

void timer_cb(uv_timer_t* handle)
{
	printf("timer_cb\n");

	uv_timer_t t1;
	uv_timer_init(loop, &t1);
	uv_timer_start(&t1, cb, 500, 0);

	uv_timer_t t2;
	uv_timer_init(loop, &t2);
	// uv_timer_start(&t2, cb, 500, 0);
	// char *msg = handle->data;
	// printf("msg: %d\n", *msg);
	// uv_timer_stop(handle);
	// uv_unref((uv_handle_t *)handle);
}

int main()
{
	loop = uv_default_loop();

	uv_timer_t t1;
	uv_timer_init(loop, &t1);
	uv_timer_start(&t1, timer_cb, 500, 0);

	return uv_run(loop, UV_RUN_DEFAULT);
}

// void once_cb(uv_timer_t *handle)
// {
// 	printf("timeonce cb...\n");
// 	// uv_timer_t m1;
// 	// uv_timer_init(loop, &m1);
// 	// m1.data = "m1";
// 	// uv_timer_start(&m1, timer_cb, 1000, 0);
// 	uv_timer_t m1, m2, e1, e2;
// 	uv_timer_init(loop, &m1);
// 	uv_timer_init(loop, &m2);
// 	uv_timer_init(loop, &e1);
// 	uv_timer_init(loop, &e2);
// 	m1.data = "m1";
// 	m2.data = "m2";
// 	e1.data = "e1";
// 	e2.data = "e2";
// 	uv_timer_start(&e1, timer_cb, 1000, 0);
// 	uv_timer_start(&m1, timer_cb, 0, 0);
// 	uv_timer_start(&e2, timer_cb, 1000, 0);
// 	uv_timer_start(&m2, timer_cb, 0, 0);
// }

// int main() {
//     loop = uv_default_loop();

// 	// uv_timer_t once;
// 	// uv_timer_init(loop, &once);
// 	// uv_timer_start(&once, once_cb, 0, 0);

// 		uv_timer_t m1, m2, e1, e2;
// 	uv_timer_init(loop, &m1);
// 	uv_timer_init(loop, &m2);
// 	uv_timer_init(loop, &e1);
// 	uv_timer_init(loop, &e2);
// 	m1.data = "m1";
// 	m2.data = "m2";
// 	e1.data = "e1";
// 	e2.data = "e2";
// 	uv_timer_start(&e1, timer_cb, 1000, 0);
// 	uv_timer_start(&m1, timer_cb, 0, 0);
// 	uv_timer_start(&e2, timer_cb, 1000, 0);
// 	uv_timer_start(&m2, timer_cb, 0, 0);

//     uv_run(loop, UV_RUN_DEFAULT);

//     uv_loop_close(loop);
//     return 0;
// }