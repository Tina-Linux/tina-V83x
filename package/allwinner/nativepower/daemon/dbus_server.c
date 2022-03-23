#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <dbus/dbus.h>
#include <tina_log.h>

#include "dbus_server.h"
#include "scene_manager.h"
#include "power_manager.h"
#define MAX_WATCHES 100

static DBusConnection *conn;
static struct pollfd pollfds[MAX_WATCHES];
static DBusWatch *watches[MAX_WATCHES];
static int max_i;

static dbus_bool_t add_watch(DBusWatch * watch, void *data)
{
	short cond = POLLHUP | POLLERR;
	int fd;
	unsigned int flags;

	TLOGI("add watch %p\n", (void *)watch);
	fd = dbus_watch_get_unix_fd(watch);
	flags = dbus_watch_get_flags(watch);

	if (flags & DBUS_WATCH_READABLE)
		cond |= POLLIN;
	if (flags & DBUS_WATCH_WRITABLE)
		cond |= POLLOUT;
	++max_i;
	pollfds[max_i].fd = fd;
	pollfds[max_i].events = cond;
	watches[max_i] = watch;

	return 1;
}

static void remove_watch(DBusWatch * watch, void *data)
{
	int i, found = 0;

	TLOGI("remove watch %p\n", (void *)watch);
	for (i = 0; i <= max_i; i++) {
		if (watches[i] == watch) {
			found = 1;
			break;
		}
	}
	if (!found) {
		TLOGE("watch %p not found!\n", (void *)watch);
		return;
	}
	memset(&pollfds[i], 0, sizeof(pollfds[i]));
	watches[i] = NULL;
	if (i == max_i && max_i > 0)
		--max_i;
}

static DBusHandlerResult filter_func(DBusConnection * conn,
					DBusMessage * msg,
					void *data)
{
	DBusMessageIter args;
	DBusMessage *reply;
	DBusError err;
	int ret = -1, serial = 0;
	unsigned int func_id;
	char *scene, *id, *interface;

	dbus_error_init(&err);
#if 0
	printf("[%s] if:%s, member:%s, path:%s\n", "TEST",
					dbus_message_get_interface(msg),
					dbus_message_get_member(msg),
					dbus_message_get_path(msg));
#endif
/*
	dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &func_id,
					DBUS_TYPE_STRING, &id,
					DBUS_TYPE_STRING, &scene);
*/
	interface = dbus_message_get_interface(msg);
	if (strcmp(interface, "nativepower.method.interface"))
		goto err;
	dbus_message_iter_init(msg, &args);
	dbus_message_iter_get_basic(&args, &func_id);
	if (!dbus_message_iter_next(&args))
		TLOGI("no more args\n");
	switch (func_id) {
	case NATIVEPOWER_DEAMON_GOTOSLEEP:
		ret = goToSleep(100, 1, 0);
		break;
	case NATIVEPOWER_DEAMON_ACQUIRE_WAKELOCK:
		if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
			dbus_message_iter_get_basic(&args, &id);
			dbus_message_iter_next(&args);
			ret = acquireWakeLock(id);
		}
		break;
	case NATIVEPOWER_DEAMON_RELEASE_WAKELOCK:
		if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
			dbus_message_iter_get_basic(&args, &id);
			dbus_message_iter_next(&args);
			ret = releaseWakeLock(id);
		}
		break;
	case NATIVEPOWER_DEAMON_SHUTDOWN:
		ret = shutdown();
		break;
	case NATIVEPOWER_DEAMON_REBOOT:
		ret = reboot();
		break;
	case NATIVEPOWER_DEAMON_USERACTIVITY:
		ret = userActivity();
		break;
	case NATIVEPOWER_DEAMON_SET_AWAKE_TIMEOUT:
		if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
			dbus_message_iter_get_basic(&args, &id);
			dbus_message_iter_next(&args);
			ret = set_awake_timeout(atol(id));
		}
		break;
	case NATIVEPOWER_DEAMON_SET_SCENE:
		if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
			dbus_message_iter_get_basic(&args, &scene);
			ret = np_scene_change(scene);
		}
		break;
	default:
		TLOGE("invalid func id:%u\n", func_id);
		goto err;
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &ret)) {
		dbus_message_unref(reply);
		TLOGE("out of memory\n");
		goto err;
	}
	if (!dbus_connection_send(conn, reply, &serial)) {
		dbus_message_unref(reply);
		TLOGE("out of memory\n");
		goto err;
	}
	dbus_connection_flush(conn);
	dbus_message_unref(reply);
err:
	dbus_error_free(&err);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static void fd_handler(short events, DBusWatch * watch)
{
	unsigned int flags = 0;

	if (events & POLLIN)
		flags |= DBUS_WATCH_READABLE;
	if (events & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;
	if (events & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;
	if (events & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	while (!dbus_watch_handle(watch, flags)) {
		TLOGE("dbus_watch_handle needs more memory\n");
		sleep(1);
	}
	dbus_connection_ref(conn);
	while (dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS) ;

	dbus_connection_unref(conn);

}

void *dbus_server_thread_func(void *arg)
{
	DBusError err;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	dbus_bus_request_name(conn, "nativepower.dbus.server",
					DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		TLOGE("request name error(%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}

	if (!dbus_connection_set_watch_functions(conn, add_watch,
						remove_watch, NULL,
						NULL, NULL)) {
		TLOGE("dbus_connection_set_watch_functions failed\n");
		dbus_error_free(&err);
		return NULL;
	}
	if (!dbus_connection_add_filter(conn, filter_func, NULL, NULL)) {
		TLOGE("dbus_connection_add_filter failed!\n");
		dbus_error_free(&err);
		return NULL;
	}

	dbus_bus_add_match(conn, "type='method_call', interface='nativepower.method.interface'", NULL);

	while (1) {
		struct pollfd fds[MAX_WATCHES];
		DBusWatch *watch[MAX_WATCHES];
		int nfds, i;

		for (nfds = 0, i = 0; i <= max_i; ++i) {
			if (pollfds[i].fd == 0 || !dbus_watch_get_enabled(watches[i]))
				continue;
			fds[nfds].fd = pollfds[i].fd;
			fds[nfds].events = pollfds[i].events;
			fds[nfds].revents = 0;
			watch[nfds] = watches[i];
			++nfds;
		}
		if (poll(fds, nfds, -1) <= 0) {
			perror("poll");
			break;
		}
		for (i = 0; i < nfds; i++) {
			if (fds[i].revents) {
				fd_handler(fds[i].revents, watch[i]);
			}
		}

	}
	return NULL;
}
