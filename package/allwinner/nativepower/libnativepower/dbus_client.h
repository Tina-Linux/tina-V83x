#ifndef _UBUS_CLIENT_H_
#define _UBUS_CLIENT_H_
#include <dbus/dbus.h>

typedef enum {
	NATIVEPOWER_DEAMON_GOTOSLEEP,
	NATIVEPOWER_DEAMON_ACQUIRE_WAKELOCK,
	NATIVEPOWER_DEAMON_RELEASE_WAKELOCK,
	NATIVEPOWER_DEAMON_SHUTDOWN,
	NATIVEPOWER_DEAMON_REBOOT,
	NATIVEPOWER_DEAMON_USERACTIVITY,
	NATIVEPOWER_DEAMON_SET_AWAKE_TIMEOUT,
	NATIVEPOWER_DEAMON_SET_SCENE,
	NATIVEPOWER_DEAMON_MAX,
} NativePower_Function;

DBusConnection *dbus_client_open();
void dbus_client_close();
int dbus_client_send_signal(DBusConnection * conn, const NativePower_Function func_id, const char *msg);
int dbus_client_invoke(DBusConnection * conn, const NativePower_Function func_id, const char *dataconst);

#endif
