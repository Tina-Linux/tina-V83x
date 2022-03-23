/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define TAG "healthd"
#include <tina_log.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/reboot.h>

#include <pthread.h>
#include <sys/time.h>

#include "healthd.h"

#include "uci.h"

#ifdef BROADCAST_INFO
#ifdef __cplusplus
extern "C" {
#endif
#include <dbus/dbus.h>
#ifdef __cplusplus
}
#endif
static DBusConnection *gConn;
#endif
/* get uci config APIs */
static bool get_healthd_config(const char *option, int *value)
{
	bool ret = false;
	struct uci_context *ctx;
	struct uci_element *e;
	struct uci_ptr ptr;
	char path[128];

	snprintf(path, sizeof(path), "healthd.@healthd[0].%s", option);
	ctx = uci_alloc_context();
	if (!ctx) {
		uci_perror(ctx, TAG);
		return false;
	}

	if (uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK) {
		uci_perror(ctx, TAG);
		goto out;
	}

	e = ptr.last;
	if (e->type == UCI_TYPE_OPTION && ptr.o->type == UCI_TYPE_STRING) {
		*value = atoi(ptr.o->v.string) * 10;
		ret = true;
	}

out:
	uci_free_context(ctx);
	return ret;
}

enum {
	LED_GREED = 2,
	LED_BLUE,
};

static void led_flash(unsigned int led, const char *trigger)
{
	int fd;
	size_t size;
	char path[128];
	char old_trigger[32];

	if (led != LED_GREED && led != LED_BLUE || trigger == NULL)
		return;
	snprintf(path, sizeof(path), "/sys/class/leds/led%u/trigger", led);
	fd = open(path, O_RDWR);
	if (fd < 0) {
		TLOGV("open %s error.\n", path);
		return;
	}
	memset(old_trigger, 0, sizeof(old_trigger));
	if (read(fd, old_trigger, sizeof(old_trigger)) < 0) {
		TLOGE("read %s error.\n", path);
		goto exit;
	}
	if (!strcmp(old_trigger, trigger))
		goto exit;
	size = strlen(trigger);
	if (size != write(fd, trigger, size))
		TLOGE("set %s trigger:%s error.\n", path, trigger);
exit:
	close(fd);
	return;
}

#if 1
#define DUMP_INFO(prop)
#else
#define DUMP_INFO(prop)		dump_battery_info(prop)
static void dump_battery_info(struct BatteryProperties *prop)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "echo chargerAcOnline=%d >> /root/shutdown_log", prop->chargerAcOnline);
	system(buf);
	snprintf(buf, sizeof(buf), "echo chargerUsbOnline=%d >> /root/shutdown_log", prop->chargerUsbOnline);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryPresent=%d >> /root/shutdown_log", prop->batteryPresent);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryStatus=%d >> /root/shutdown_log", prop->batteryStatus);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryHealth=%d >> /root/shutdown_log", prop->batteryHealth);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryLevel=%d >> /root/shutdown_log", prop->batteryLevel);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryVoltage=%d >> /root/shutdown_log", prop->batteryVoltage);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryCurrentNow=%d >> /root/shutdown_log", prop->batteryCurrentNow);
	system(buf);
	snprintf(buf, sizeof(buf), "echo batteryTemperature=%d >> /root/shutdown_log", prop->batteryTemperature);
	system(buf);
}
#endif
#ifdef BROADCAST_INFO
static int dbus_open()
{
	DBusError err;

	dbus_error_init(&err);
	gConn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		TLOGE("dbus bus get error(%s)\n", err.message);
		dbus_error_free(&err);
		return -1;
	}
	dbus_error_free(&err);
	return 0;
}

static int dbus_service_send_signal(const char *member, int type, void *data)
{
	DBusMessage *msg;
	DBusMessageIter arg;
	dbus_uint32_t value;

	if (!gConn && dbus_open() != 0)
		return -1;

	if (type == DBUS_TYPE_UINT32)
		value = *(unsigned int *)data;
	else if (type == DBUS_TYPE_BOOLEAN)
		value = *(unsigned char *)data;
	DLOG("send signal:member=%s, type=%d, data=%u\n", member, type, value);
	msg = dbus_message_new_signal("/healthd/service/signal", "healthd.signal.interface", member);
	if (!msg) {
		TLOGE("dbus_message_new_signal error\n");
		return -1;
	}
	dbus_message_iter_init_append(msg, &arg);
	if (!dbus_message_iter_append_basic(&arg, type, &value)) {
		TLOGE("out of memeory\n");
		dbus_message_unref(msg);
		return -1;
	}
	if (!dbus_connection_send(gConn, msg, NULL)) {
		TLOGE("out of memory\n");
		dbus_message_unref(msg);
		return -1;
	}
	dbus_connection_flush(gConn);
	dbus_message_unref(msg);
	return 0;
}
#endif

static void perform_shutdown(struct BatteryProperties *props)
{
	DUMP_INFO(props);
	reboot(RB_POWER_OFF);
}

void healthd_mode_tina_battery_update(struct BatteryProperties *props)
{
	int low_capacity_warning, low_capacity_shutdown;
	int bat_temp_low, bat_temp_over;
	int ic_temp, ic_high_temp_warning, ic_high_temp_shutdown;
	static BatteryProperties last_prop;

	DLOG("before tina ops update:ac-%u,usb-%u,bat-%u,bat_status-%u,"
		"bat_health-%u,bat_cap-%u,bat_vol-%u,bat_temp-%u\n",
		props->chargerAcOnline, props->chargerUsbOnline,
		props->batteryPresent, props->batteryStatus,
		props->batteryHealth, props->batteryLevel,
		props->batteryVoltage, props->batteryTemperature);

	props->batteryLowCapWarn = false;
	if (props->batteryPresent &&
		!props->chargerAcOnline &&
		!props->chargerUsbOnline) {
		/*low capacity handle */
		low_capacity_shutdown = batinfo_get_bat_capacity_alert_level2();
		if (low_capacity_shutdown >= 0 &&
			props->batteryLevel <= low_capacity_shutdown) {
			/*low capacity shutdown */
			perform_shutdown(props);
		}
		low_capacity_warning = batinfo_get_bat_capacity_alert_level1();
		if (low_capacity_warning >= 0 &&
			props->batteryLevel <= low_capacity_warning) {
			/*low capacity warning */
			props->batteryLowCapWarn = true;
		}
	}
	led_flash(LED_GREED, props->batteryLowCapWarn ? "timer" : "none");

	if (props->batteryPresent) {
		/*bat temp low & over handle */
		if (props->batteryTemperature != -0xFF &&
		    	get_healthd_config("bat_temp_low", &bat_temp_low) &&
			props->batteryTemperature <= bat_temp_low) {
			perform_shutdown(props);
		}
		if (props->batteryTemperature != -0xFF &&
		    	get_healthd_config("bat_temp_over", &bat_temp_over) &&
			props->batteryTemperature >= bat_temp_over) {
			perform_shutdown(props);
		}
	}

	/*high ic temp handle */
	props->batteryOverTempWarn = false;
	ic_temp = batinfo_get_ic_temp();
	if (ic_temp >= 0) {
		if (get_healthd_config("ic_high_temp_shutdown", &ic_high_temp_shutdown)
		    && ic_temp >= ic_high_temp_shutdown) {
			/*high ic temp shutdown */
			perform_shutdown(props);
		}
		if (get_healthd_config("ic_high_temp_warning", &ic_high_temp_warning)
		    && ic_temp >= ic_high_temp_warning) {
			/*high ic temp warning */
			props->batteryOverTempWarn = true;
		}
	}
	led_flash(LED_BLUE, props->batteryOverTempWarn ? "timer" : "none");

#ifdef BROADCAST_INFO
	if (last_prop.chargerAcOnline != props->chargerAcOnline)
		dbus_service_send_signal("ac_present", DBUS_TYPE_BOOLEAN, &props->chargerAcOnline);
	if (last_prop.chargerUsbOnline != props->chargerUsbOnline)
		dbus_service_send_signal("usb_present", DBUS_TYPE_BOOLEAN, &props->chargerUsbOnline);
	if (last_prop.batteryPresent != props->batteryPresent)
		dbus_service_send_signal("battery_present", DBUS_TYPE_BOOLEAN, &props->batteryPresent);
	if (last_prop.batteryStatus != props->batteryStatus)
		dbus_service_send_signal("status", DBUS_TYPE_UINT32, &props->batteryStatus);
	if (last_prop.batteryHealth != props->batteryHealth)
		dbus_service_send_signal("health", DBUS_TYPE_UINT32, &props->batteryHealth);
	if (last_prop.batteryLevel != props->batteryLevel)
		dbus_service_send_signal("capacity", DBUS_TYPE_UINT32, &props->batteryLevel);
	if (last_prop.batteryVoltage != props->batteryVoltage)
		dbus_service_send_signal("vol_now", DBUS_TYPE_UINT32, &props->batteryVoltage);
	if (last_prop.batteryCurrentNow != props->batteryCurrentNow)
		dbus_service_send_signal("current_now", DBUS_TYPE_UINT32, &props->batteryCurrentNow);
	if (last_prop.batteryTemperature != props->batteryTemperature)
		dbus_service_send_signal("temp", DBUS_TYPE_UINT32, &props->batteryTemperature);
	if (last_prop.batteryLowCapWarn != props->batteryLowCapWarn)
		dbus_service_send_signal("bat_low_capacity", DBUS_TYPE_BOOLEAN, &props->batteryLowCapWarn);
	if (last_prop.batteryOverTempWarn != props->batteryOverTempWarn)
		dbus_service_send_signal("ic_over_temp", DBUS_TYPE_BOOLEAN, &props->batteryOverTempWarn);
#endif
	last_prop = *props;
}

int healthd_mode_tina_preparetowait(void)
{
	DLOG("healthd_mode_tina_preparetowait\n");
	return -1;
}

void healthd_mode_tina_init(struct healthd_config *config)
{
	DLOG("healthd_mode_tina_init\n");
#ifdef BROADCAST_INFO
	dbus_open();
#endif
}
