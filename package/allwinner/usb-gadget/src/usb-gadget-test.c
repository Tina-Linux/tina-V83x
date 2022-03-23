#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <linux/input.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <getopt.h>

#include "usb-gadget.h"

#define PROGRAM_NAME "usb-gadget-test"
#define VERSION "Revision 1.0"

static void display_version(void)
{
	printf("%s " VERSION "\n"
			"\n"
			"This is USB gadget demo!\n",
			PROGRAM_NAME);

	exit(0);
}

static void display_help()
{
	printf("Usage: %s [OPTIONS] [ARGUMENTS]\n"
			"USB gadget test,contain ums enable/disable and eject detect\n"
			"\n"
			"-h         --help               display this help and exit\n"
			"-v         --version            output version information and exit\n"
			"-e         --enable             Enable usb mass storage\n"
			"-d         --disable            Disable usb mass storage\n"
			"-l         --listen             listen uevent and parse eject msg\n",
			PROGRAM_NAME);
}

static void ums_control(int onoff)
{
	int state = tina_gadget_get_connect_state();
	if (state == 1) {
		printf("usb is connect to pc.\n");
		if (onoff)
			tina_gadget_is_mmc_storage_enable(1);
		else
			tina_gadget_is_mmc_storage_enable(0);
	}
}

static int gSocket;
static int gLoop = 1;
static void event_handler(int arg)
{
	if (!tina_gadget_ums_is_eject(gSocket)) {
		printf("Recv ums ject msg!\n");
		gLoop = 0;
	}
}

#define MAX_EPOLL_EVENTS 40
static int event_cnt;
static int ums_event_register(int epollfd, void (*handler)(int))
{
	struct epoll_event ev;
	gSocket = tina_gadget_open_socket();
	if (gSocket < 0) {
		printf("open socket error\n");
		return -1;
	}

	ev.events = EPOLLIN;
	ev.data.ptr = handler;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, gSocket, &ev) == -1) {
		printf("epoll_ctl failed\n");
		close(gSocket);
		return -1;
	}
	event_cnt++;
	return 0;
}
#define s2ms(n)  (n*1000)
static void ums_listen_eject(void)
{
	int epollfd;
	int timeout = s2ms(20), event_num;
	struct epoll_event events[event_cnt+1];

	epollfd = epoll_create(MAX_EPOLL_EVENTS);
	if (epollfd == -1) {
		printf("epoll_create failed\n");
		exit(1);
	}

	ums_event_register(epollfd, event_handler);

	while(gLoop) {
		int n;
		event_num = epoll_wait(epollfd, events, event_cnt, timeout);
		if (event_num == -1)
			break;
		for (n=0; n < event_num; n++) {
			if (events[n].data.ptr)
				(*(void (*)(int))events[n].data.ptr) (events[n].events);
		}
	}
}

int main(int argc, char* argv[])
{
	int option_index = 0;
	const char *short_options = "hvedl";
	const struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'v', },
		{ "enable", no_argument, 0, 'e', },
		{ "disable", no_argument, 0, 'd', },
		{ "listen", no_argument, 0, 'l', },
		{ NULL, 0, 0, 0 }
	};

	int c = getopt_long(argc, argv, short_options,
			long_options, &option_index);

	switch (c) {
		case 'h':
			display_help();
			break;
		case 'v':
			display_version();
			break;
		case 'e':
			ums_control(1);
			break;
		case 'd':
			ums_control(0);
			break;
		case 'l':
			ums_listen_eject();
			break;
		default:
			display_help();
			break;
	}

	return 0;
}
