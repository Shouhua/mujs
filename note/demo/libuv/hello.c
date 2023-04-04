#include <stdio.h>
#include <uv.h>

unsigned int counter = 0;
void
timer_callback(uv_timer_t *handle) {
	counter++;
	printf("%d\t\n", counter);
	// fflush(stdout);
	if(counter > 4) {
		uv_timer_stop(handle);
	}
}

int main() {
	uv_loop_t *loop = uv_default_loop();
	uv_timer_t timer_handler;
	uv_timer_init(loop, &timer_handler);
	uv_timer_start(&timer_handler, timer_callback, 1000, 2000);

	uv_run(loop, UV_RUN_DEFAULT);

	uv_timer_stop(&timer_handler);
	uv_loop_close(loop);
	return 0;
}