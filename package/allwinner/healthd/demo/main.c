#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <dbus/dbus.h>
#include "batinfo/batinfo.h"

#define TAGS	"HEALTHD-DEMO"
#define debug(fmt, ...)		printf("[%s] LINE:%d "fmt"", TAGS, __LINE__, ##__VA_ARGS__)

#define MAX_WATCHES 100
static DBusConnection *conn;
static struct pollfd pollfds[MAX_WATCHES];
static DBusWatch *watches[MAX_WATCHES];
static int max_i;
static char *progname;

static dbus_bool_t add_watch(DBusWatch *watch, void *data)
{
	short cond = POLLHUP | POLLERR;
	int fd;
	unsigned int flags;

	debug("[%s] add watch %p\n", progname, (void *)watch);
	fd = dbus_watch_get_unix_fd(watch);
	flags = dbus_watch_get_flags(watch);

	if(flags & DBUS_WATCH_READABLE)
		cond |= POLLIN;
	if(flags & DBUS_WATCH_WRITABLE)
		cond |= POLLOUT;
	++max_i;
	pollfds[max_i].fd = fd;
	pollfds[max_i].events = cond;
	watches[max_i] = watch;

	return 1;
}

static void remove_watch(DBusWatch *watch, void *data)
{
	int i, found = 0;

	debug("[%s] remove watch %p/n", progname, (void*)watch);
	for(i=0; i<=max_i;i++) {
		if(watches[i] == watch) {
			found = 1;
			break;
		}
	}
	if(!found) {
		debug("watch %p not found!\n", (void *)watch);
		return ;
	}
	memset(&pollfds[i], 0, sizeof(pollfds[i]));
	watches[i] = NULL;
	if(i == max_i && max_i > 0)
		--max_i;
}

static void get_arg_from_msg(DBusMessage *msg, DBusConnection *conn, int type, void *data)
{
	DBusMessageIter args;
	DBusError err;

	dbus_error_init(&err);
	if(!dbus_message_iter_init(msg, &args)) {
		debug("dbus_message_iter_init error(%s)\n", err.message);
		goto err;
	}
	if(type != dbus_message_iter_get_arg_type(&args)) {
		debug("data type error\n");
		goto err;
	}
	dbus_message_iter_get_basic(&args, data);
err:
	dbus_error_free(&err);
}

static DBusHandlerResult
filter_func(DBusConnection *conn, DBusMessage *msg, void *data)
{
	struct BatteryProperties props;
	unsigned int change=0;
/*
	debug("[%s] if:%s, member:%s, path:%s\n", progname,
				dbus_message_get_interface(msg),
				dbus_message_get_member(msg),
				dbus_message_get_path(msg));
*/
	if(!batinfo_get_change(conn, msg, &props, &change)) {
		if(change&BATINFO_AC_PRESENT)
			debug("ac_present change: %u\n",props.chargerAcOnline);
		if(change&BATINFO_USB_PRESENT)
			debug("usb_present change: %u\n", props.chargerUsbOnline);
		if(change&BATINFO_BAT_PRESENT)
			debug("battery_present change: %u\n", props.batteryPresent);
		if(change&BATINFO_BAT_STATUS)
			debug("status change: %u\n", props.batteryStatus);
		if(change&BATINFO_BAT_HEALTH)
			debug("health change: %u\n", props.batteryHealth);
		if(change&BATINFO_BAT_CAPACITY)
			debug("capacity change: %u\n", props.batteryLevel);
		if(change&BATINFO_BAT_VOLTAGE)
			debug("vol_now change: %u\n", props.batteryVoltage);
		if(change&BATINFO_BAT_CURRENT)
			debug("current_now change: %u\n", props.batteryCurrentNow);
		if(change&BATINFO_BAT_TEMP)
			debug("temp change: %u\n", props.batteryTemperature);
		if(change&BATINFO_BAT_LOW_CAPACITY_WARNING)
			debug("ic_over_temp change: %u\n", props.batteryLowCapWarn);
		if(change&BATINFO_IC_OVER_TEMP_WARNING)
			debug("ic_over_temp change: %u\n", props.batteryOverTempWarn);
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}

static void fd_handler(short events, DBusWatch *watch)
{
	unsigned int flags = 0;

	if(events & POLLIN)
		flags |= DBUS_WATCH_READABLE;
	if(events & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;
	if(events & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;
	if(events & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	while(!dbus_watch_handle(watch, flags)) {
		debug("dbus_watch_handle needs more memory\n");
		sleep(1);
	}
	dbus_connection_ref(conn);
	while(dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS);

	dbus_connection_unref(conn);

}

int main(int argc, char *argv[])
{
	DBusError err;

	progname = argv[0];
	dbus_error_init(&err);
	conn = batinfo_dbus_bus_get();

	if(!dbus_connection_set_watch_functions(conn, add_watch, remove_watch, NULL, NULL, NULL)) {
		debug("dbus_connection_set_watch_functions failed\n");
		dbus_error_free(&err);
		return 1;
	}
	if(!dbus_connection_add_filter(conn, filter_func, NULL, NULL)) {
		debug("dbus_connection_add_filter failed!\n");
		dbus_error_free(&err);
		return 1;
	}
	//add all 'signal' event
	//dbus_bus_add_match(conn, "type='signal'", NULL);
	//add all 'method' event
	//dbus_bus_add_match(conn, "type='method_call'", NULL);

	dbus_bus_add_match(conn, "type='signal', interface='healthd.signal.interface'", NULL);

	while(1) {
		struct pollfd fds[MAX_WATCHES];
		DBusWatch *watch[MAX_WATCHES];
		int nfds, i;

		for(nfds=0,i=0; i<= max_i; ++i) {
			if(pollfds[i].fd == 0 ||
				!dbus_watch_get_enabled(watches[i]))
				continue;
			fds[nfds].fd = pollfds[i].fd;
			fds[nfds].events = pollfds[i].events;
			fds[nfds].revents = 0;
			watch[nfds] = watches[i];
			++nfds;
		}
		if(poll(fds, nfds, -1) <= 0) {
			perror("poll");
			break;
		}
		for(i=0;i < nfds; i++) {
			if(fds[i].revents) {
				fd_handler(fds[i].revents, watch[i]);
			}
		}

	}
	return 0;

}
