#include <stdio.h>
#include <event2/event.h>

void timeout_cb(int fd, short flags, void *userdata)
{
	(void)fd;
	(void)flags;
	struct event *ev = (struct event *)userdata;
	printf("timout priority: %d\n", event_get_priority(ev));
}

int main()
{
	struct event_base *base;
	struct event *micro, *macro;
	struct timeval tv;
	base = event_base_new();

	event_base_priority_init(base, 3);
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	macro = event_new(base, -1, EV_TIMEOUT, timeout_cb, event_self_cbarg());
	micro = event_new(base, -1, EV_TIMEOUT, timeout_cb, event_self_cbarg());
	// event_priority_set(macro, 1);
	event_priority_set(micro, 2);
	evtimer_add(macro, &tv);
	tv.tv_sec = 0;
	tv.tv_usec = 500;
	evtimer_add(micro, &tv);

	event_base_dispatch(base);

	event_base_free(base);
	libevent_global_shutdown();
	return 0;
}