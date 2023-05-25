#include <stdio.h>
#include <event2/event.h>

struct event_base *base;

void micro_cb(evutil_socket_t fd, short flags, void *userdata)
{
	printf("%s\n", (char *)userdata);
}

void macro_cb(evutil_socket_t fd, short flags, void *userdata)
{
	printf("%s\n", (char *)userdata);
	// struct event *ev;
	// struct timeval tv;
	// ev = evtimer_new(base, micro_cb, userdata);
	// tv.tv_sec = 0;
	// tv.tv_usec = 10000;
	// event_add(ev, &tv);	
}

void timeout_cb(evutil_socket_t fd, short flags, void *userdata)
{
	printf("timeonce cb...\n");
	base = (struct event_base *)userdata;
	struct event *m1, *m2, *e1, *e2;
	struct timeval tv;
	m1 = evtimer_new(base, macro_cb, "m1");
	m2 = evtimer_new(base, macro_cb, "m2");
	e1 = evtimer_new(base, micro_cb, "e1");
	e2 = evtimer_new(base, micro_cb, "e2");
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	event_priority_set(e1, 0);
	event_add(e1, &tv);
	// tv.tv_sec = 0;
	// tv.tv_usec = 10000;
	// event_add(e2, &tv);

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	// event_add(m1, &tv);

	// for(size_t i=0;i<10000000;i++){}
	event_add(m1, &tv);
	// for(size_t i=0;i<10000000;i++){}

	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	event_priority_set(e2, 1);
	event_add(e2, &tv);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	event_add(m2, &tv);
}

int main (int argc, char *argv[]) {
	struct event_base *base;
	// struct timeval tv;

	base = event_base_new();
	// event_base_priority_init(base, 5);

	// tv.tv_sec = 0;
	// tv.tv_usec = 1;
	// event_base_once(base, -1, EV_TIMEOUT, timeout_cb, base, &tv);
	struct event *m1, *m2, *e1, *e2;
	struct timeval tv;
	m1 = evtimer_new(base, macro_cb, "m1");
	m2 = evtimer_new(base, macro_cb, "m2");
	e1 = evtimer_new(base, micro_cb, "e1");
	e2 = evtimer_new(base, micro_cb, "e2");
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	// event_priority_set(e1, 0);
	event_add(e1, &tv);
	// tv.tv_sec = 0;
	// tv.tv_usec = 10000;
	// event_add(e2, &tv);

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	// event_add(m1, &tv);

	// for(size_t i=0;i<10000000;i++){}
	event_add(m1, &tv);
	// for(size_t i=0;i<10000000;i++){}

	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	// event_priority_set(e2, 1);
	event_add(e2, &tv);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	event_add(m2, &tv);
	event_base_dispatch(base);

	event_base_free(base);
	libevent_global_shutdown();
	return 0;
}