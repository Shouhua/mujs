#include <stdio.h>
#include <event2/event.h>
#include <event2/watch.h>

void timeout_cb(int fd, short flags, void *userdata)
{
	(void)fd;
	(void)flags;
	// struct event *ev = (struct event *)userdata;
	// printf("timout priority: %d\n", event_get_priority(ev));
	printf("%s\n", (char *)userdata);
}

static void on_prepare(struct evwatch *watcher, const struct evwatch_prepare_cb_info *info, void *userdata)
{
	printf("on_prepare\n");
}
static void on_check(struct evwatch *watcher, const struct evwatch_check_cb_info *info, void *userdata)
{
	printf("on_check\n");
}

int main()
{
	event_enable_debug_mode();
	event_enable_debug_logging(EVENT_DBG_ALL);
	struct event_base *base;
	struct event *micro, *macro;
	struct timeval tv;
	base = event_base_new();

	evwatch_prepare_new(base, &on_prepare, NULL);
	evwatch_check_new(base, &on_check, NULL);

	// event_base_priority_init(base, 3);
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	macro = event_new(base, -1, EV_TIMEOUT, timeout_cb, "t1");
	micro = event_new(base, -1, EV_TIMEOUT, timeout_cb, "t2");
	// event_priority_set(macro, 1);
	// event_priority_set(micro, 2);
	evtimer_add(macro, &tv);
	// tv.tv_sec = 0;
	// tv.tv_usec = 500;
	evtimer_add(micro, &tv);

	event_base_dispatch(base);

	event_base_free(base);
	libevent_global_shutdown();
	return 0;
}