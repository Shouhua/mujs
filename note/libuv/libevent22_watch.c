/*
尝试libevent2.2新添加功能prepare和check，放在watch.h
1. 由于没有现成版本安装包，本地编译，按照官方文档就好
2. 设置libevent2.2的libevent.so，ubuntu新建文件
    sudo echo 'libenevt安装目录/lib' > /etc/ld.so.conf.d/libevent22.conf
    sudo ldconfig -v #加载新添加的动态库到cache中
3. 编译次脚本文件
    gcc libevent22_watch.c -Wall -pedantic -Wextra -o libevent22_watch -levent -I/libent_installed_dir/include -L/libent_installed_dir/lib
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/watch.h>

static struct event_base *base;

static void
on_prepare(evutil_socket_t sig, short flags, void *arg)
{
    printf("on_prepare\n");
}

static void
on_check(evutil_socket_t sig, short flags, void *arg)
{
    printf("on_check\n");
}

static void
on_timeout(evutil_socket_t sig, short flags, void *arg)
{
    printf("on_timeout\n");
}

static void
on_sigint(evutil_socket_t sig, short flags, void *arg)
{
    event_base_loopbreak(base);
}

int
main()
{
	struct timeval one_second = { 1, 0 };
	struct event *timeout_event, *sigint_event;

	base = event_base_new();

	/* add prepare and check watchers; no need to hang on to their pointers,
	 * since they will be freed for us in event_base_free. */
	evwatch_prepare_new(base, &on_prepare, NULL);
	evwatch_check_new(base, &on_check, NULL);

	/* set a persistent one second timeout */
	timeout_event = event_new(base, -1, EV_PERSIST, &on_timeout, NULL);
	if (!timeout_event)
		return EXIT_FAILURE;
	event_add(timeout_event, &one_second);

	/* set a handler for interrupt, so we can quit cleanly */
	sigint_event = evsignal_new(base, SIGINT, &on_sigint, NULL);
	if (!sigint_event)
		return EXIT_FAILURE;
	event_add(sigint_event, NULL);

	/* run the event loop until interrupted */
	event_base_dispatch(base);

	/* clean up */
	event_free(timeout_event);
	event_free(sigint_event);
	event_base_free(base);
	return 0;
}