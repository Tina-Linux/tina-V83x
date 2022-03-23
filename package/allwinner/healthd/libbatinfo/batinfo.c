#define TAG "batinfo"
#include <tina_log.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "batinfo/batinfo.h"

#include <pthread.h>

static struct BatteryProperties last_props;

int mapSysfsString(const char* str, struct sysfsStringEnumMap map[]){
	for(int i = 0; map[i].s; i++)
		if(!strcmp(str, map[i].s))
			return map[i].val;

	return -1;
}

void *memrchr(const void *s, int c, size_t n);
static int readFromFile(const char * file, char * buf, size_t size)
{
	int fd;
	char *cp = NULL;
	char err_buf[80];
	ssize_t count;
	if(file == NULL)
		return -1;
	fd = open(file,O_RDONLY,0);
	if(fd == -1)
		return -1;

	count = read(fd,buf,size);
	if(count > 0)
		cp = (char *)memrchr(buf, '\n',count);

	if(cp)
	    *cp = '\0';
	else
	    buf[0] = '\0';
	close(fd);

	return count;
}

static bool getBooleanField(const char *file) {
    const int SIZE = 16;
    char buf[SIZE];

    bool value = false;
    if (readFromFile(file, buf, SIZE) > 0) {
        if (buf[0] != '0') {
            value = true;
        }
    }

    return value;
}

static int getIntField(const char *file) {
    const int SIZE = 128;
    char buf[SIZE];

    int value = -0xFF;
    if (readFromFile(file, buf, SIZE) > 0) {
        value = strtol(buf, NULL, 0);
    }
    return value;
}

bool batinfo_get_usb_present(void)
{
	return getBooleanField("/sys/class/power_supply/usb/present");
}

bool batinfo_get_ac_present(void)
{
	return getBooleanField("/sys/class/power_supply/ac/present");
}

bool batinfo_get_bat_present(void)
{
	return getBooleanField("/sys/class/power_supply/battery/present");
}

int batinfo_get_bat_status()
{
	int ret = -1;
	char buf[128];
	struct sysfsStringEnumMap batteryStatusMap[] = {
		{"Unknown", BATTERY_STATUS_UNKNOWN},
		{"Charging", BATTERY_STATUS_CHARGING},
		{"Discharging", BATTERY_STATUS_DISCHARGING},
		{"Not charging", BATTERY_STATUS_NOT_CHARGING},
		{"Full", BATTERY_STATUS_FULL},
		{NULL, 0},
	};
	memset(buf, 0, sizeof(buf));
	if(readFromFile("/sys/class/power_supply/battery/status", buf, sizeof(buf)) > 0) {
		ret = mapSysfsString(buf, batteryStatusMap);
		if(ret < 0) {
			TLOGE("Unknow battery status '%s'\n", buf);
			ret = BATTERY_STATUS_UNKNOWN;
		}
	}
	return ret;
}

int batinfo_get_bat_health(void)
{
	int ret = -1;
	char buf[128];
	struct sysfsStringEnumMap batteryHealthMap[] = {
		{ "Unknown", BATTERY_HEALTH_UNKNOWN },
        { "Good", BATTERY_HEALTH_GOOD },
        { "Overheat", BATTERY_HEALTH_OVERHEAT },
        { "Dead", BATTERY_HEALTH_DEAD },
        { "Over voltage", BATTERY_HEALTH_OVER_VOLTAGE },
        { "Unspecified failure", BATTERY_HEALTH_UNSPECIFIED_FAILURE },
        { "Cold", BATTERY_HEALTH_COLD },
        { NULL, 0 },
	};
	memset(buf, 0, sizeof(buf));
	if(readFromFile("/sys/class/power_supply/battery/health", buf, sizeof(buf)) > 0) {
		ret = mapSysfsString(buf, batteryHealthMap);
		if(ret < 0) {
			TLOGE("Unknow battery health'%s'\n", buf);
			ret = BATTERY_HEALTH_UNKNOWN;
		}
	}
	return ret;
}

int batinfo_get_bat_capacity(void)
{
	return getIntField("/sys/class/power_supply/battery/capacity");
}

int batinfo_get_bat_capacity_alert_level1()
{
	return getIntField("/sys/class/power_supply/battery/capacity_alert_max");
}

int batinfo_get_bat_capacity_alert_level2()
{
	return getIntField("/sys/class/power_supply/battery/capacity_alert_min");
}

int batinfo_get_bat_voltage(void)
{
	return getIntField("/sys/class/power_supply/battery/voltage_now");
}

int batinfo_get_bat_current(void)
{
	return getIntField("/sys/class/power_supply/battery/current_now");
}

int batinfo_get_bat_temp(void)
{
	return getIntField("/sys/class/power_supply/battery/temp");
}

int batinfo_get_ic_temp(void)
{
	return getIntField("/sys/class/power_supply/battery/temp_ambient");
}

int batinfo_get_all(struct BatteryProperties *props)
{
	if(props == NULL)
		return -1;
	props->chargerAcOnline = batinfo_get_ac_present();
	props->chargerUsbOnline = batinfo_get_usb_present();
	props->batteryPresent = batinfo_get_bat_present();
	props->batteryStatus = batinfo_get_bat_status();
	props->batteryHealth = batinfo_get_bat_health();
	props->batteryLevel = batinfo_get_bat_capacity();
	props->batteryVoltage = batinfo_get_bat_voltage();
	props->batteryCurrentNow = batinfo_get_bat_current();
	props->batteryTemperature = batinfo_get_bat_temp();
	return 0;
}
#ifdef BROADCAST_INFO
DBusConnection *batinfo_dbus_bus_get(void)
{
	DBusConnection *conn;
	DBusError err;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	dbus_bus_request_name(conn, "healthd.dbus.service", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if(dbus_error_is_set(&err)) {
		printf("request name error(%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}
	return conn;
}

static void get_arg_from_msg(DBusMessage *msg, DBusConnection *conn, int type, void *data)
{
	DBusMessageIter args;
	DBusError err;

	dbus_error_init(&err);
	if(!dbus_message_iter_init(msg, &args)) {
		printf("dbus_message_iter_init error(%s)\n", err.message);
		goto err;
	}
	if(type != dbus_message_iter_get_arg_type(&args)) {
		printf("data type error\n");
		goto err;
	}
	dbus_message_iter_get_basic(&args, data);
err:
	dbus_error_free(&err);
}

int batinfo_get_change(DBusConnection *conn, DBusMessage *msg, struct BatteryProperties *props, unsigned int *change)
{
	if(!conn || !msg || !props || !change)
		return -1;
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "ac_present")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_BOOLEAN, &props->chargerAcOnline);
		*change |= BATINFO_AC_PRESENT;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "usb_present")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_BOOLEAN, &props->chargerUsbOnline);
		*change |= BATINFO_USB_PRESENT;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "battery_present")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_BOOLEAN, &props->batteryPresent);
		*change |= BATINFO_BAT_PRESENT;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "status")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryStatus);
		*change |= BATINFO_BAT_STATUS;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "health")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryHealth);
		*change |= BATINFO_BAT_HEALTH;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "capacity")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryLevel);
		*change |= BATINFO_BAT_CAPACITY;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "vol_now")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryVoltage);
		*change |= BATINFO_BAT_VOLTAGE;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "current_now")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryCurrentNow);
		*change |= BATINFO_BAT_CURRENT;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "temp")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_UINT32, &props->batteryTemperature);
		*change |= BATINFO_BAT_TEMP;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "bat_low_capacity")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_BOOLEAN, &props->batteryLowCapWarn);
		*change |= BATINFO_BAT_LOW_CAPACITY_WARNING;
	}
	if(dbus_message_is_signal(msg, "healthd.signal.interface", "ic_over_temp")) {
		get_arg_from_msg(msg, conn, DBUS_TYPE_BOOLEAN, &props->batteryOverTempWarn);
		*change |= BATINFO_IC_OVER_TEMP_WARNING;
	}
	return 0;
}
#endif
