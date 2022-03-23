#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include "dbus_client.h"

DBusConnection *dbus_client_open()
{
	DBusError err;
	DBusConnection *conn;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "dbus bus get error(%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}
	dbus_error_free(&err);
	return conn;
}

void dbus_client_close(DBusConnection * conn)
{

}

int dbus_client_invoke(DBusConnection * conn, const NativePower_Function func_id, const char *data)
{
	DBusMessage *msg;
	DBusMessageIter arg;
	dbus_uint32_t value;
	DBusPendingCall *pending = NULL;
	DBusError err;
	int ret = 0;

	if (!conn)
		return -1;
/*
	msg = dbus_message_new_signal("/nativepower/service/method",
                                    "nativepower.signal.interface",
                                    "invoke");
*/
/*
	dbus_bus_request_name(conn, "nativepower.dbus.client", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if(dbus_error_is_set(&err)) {
		fprintf(stderr, "connection error(%s)\n", err.message);
        dbus_error_free(&err);
		return -1;
    }
*/
	msg = dbus_message_new_method_call("nativepower.dbus.server", "/nativepower/service/method", "nativepower.method.interface", "method");
	if (!msg) {
		fprintf(stderr, "dbus_message_new_method error\n");
		return -1;
	}
	dbus_message_iter_init_append(msg, &arg);
	if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_UINT32, (void *)&func_id)) {
		fprintf(stderr, "out of memeory\n");
		dbus_message_unref(msg);
		return -1;
	}
	if (data != NULL) {
		if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_STRING, &data)) {
			fprintf(stderr, "out of memeory\n");
			dbus_message_unref(msg);
			return -1;
		}
	}
#if 0
	if (!dbus_connection_send(conn, msg, NULL)) {
		fprintf(stderr, "out of memory\n");
		dbus_message_unref(msg);
		return -1;
	}
#else
	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
		fprintf(stderr, "out of memory\n");
		dbus_message_unref(msg);
		return -1;
	}
	if (NULL == pending) {
		fprintf(stderr, "pending call null\n");
		dbus_message_unref(msg);
		return -1;
	}
#endif
	dbus_error_init(&err);
	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	msg = NULL;

	dbus_pending_call_block(pending);
	msg = dbus_pending_call_steal_reply(pending);
	if (msg == NULL) {
		fprintf(stderr, "reply null\n");
		return -1;
	}
	dbus_pending_call_unref(pending);
	if (dbus_error_is_set(&err))
		fprintf(stderr, "pending return (%s)", err.message);
	if (!dbus_message_iter_init(msg, &arg))
		fprintf(stderr, "message has no arg\n");
	else if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arg)) {
		char *err_msg;
		ret = -1;
		dbus_message_iter_get_basic(&arg, &err_msg);
		fprintf(stderr, "pending return (%s)\n", err_msg);
	} else if (DBUS_TYPE_UINT32 == dbus_message_iter_get_arg_type(&arg))
		dbus_message_iter_get_basic(&arg, &ret);
	else
		fprintf(stderr, "invalid type!\n");

	dbus_error_free(&err);
	dbus_message_unref(msg);
	return ret;

}

int dbus_client_send_signal(DBusConnection * conn, const NativePower_Function func_id, const char *data)
{
	DBusMessage *msg;
	DBusMessageIter arg;
	dbus_uint32_t value;

	if (!conn)
		return -1;

	msg = dbus_message_new_signal("/nativepower/service/signal", "nativepower.signal.interface", "invoke");
	if (!msg) {
		fprintf(stderr, "dbus_message_new_signal error\n");
		return -1;
	}
	dbus_message_iter_init_append(msg, &arg);
	if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_UINT32, (void *)&func_id)) {
		fprintf(stderr, "out of memeory\n");
		dbus_message_unref(msg);
		return -1;
	}
	if (data != NULL) {
		if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_STRING, &data)) {
			fprintf(stderr, "out of memeory\n");
			dbus_message_unref(msg);
			return -1;
		}
	}

	if (!dbus_connection_send(conn, msg, NULL)) {
		fprintf(stderr, "out of memory\n");
		dbus_message_unref(msg);
		return -1;
	}
	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	return 0;
}
