#ifndef BATINFO_H
#define BATINFO_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef BROADCAST_INFO
#include <dbus/dbus.h>
#endif

enum {
	BATTERY_STATUS_UNKNOWN = 1,
	BATTERY_STATUS_CHARGING = 2,
	BATTERY_STATUS_DISCHARGING = 3,
	BATTERY_STATUS_NOT_CHARGING = 4,
	BATTERY_STATUS_FULL = 5,
};

enum {
	BATTERY_HEALTH_UNKNOWN = 1,
	BATTERY_HEALTH_GOOD = 2,
	BATTERY_HEALTH_OVERHEAT = 3,
	BATTERY_HEALTH_DEAD = 4,
	BATTERY_HEALTH_OVER_VOLTAGE = 5,
	BATTERY_HEALTH_UNSPECIFIED_FAILURE = 6,
	BATTERY_HEALTH_COLD = 7,
};

typedef enum {
	BATINFO_AC_PRESENT = 0x01,
	BATINFO_USB_PRESENT = 0x02,
	BATINFO_BAT_PRESENT = 0x04,
	BATINFO_BAT_STATUS = 0x08,
	BATINFO_BAT_HEALTH = 0x10,
	BATINFO_BAT_CAPACITY = 0x20,
	BATINFO_BAT_VOLTAGE = 0x40,
	BATINFO_BAT_CURRENT = 0x80,
	BATINFO_BAT_TEMP = 0x100,
	BATINFO_BAT_LOW_CAPACITY_WARNING = 0x200,
	BATINFO_IC_OVER_TEMP_WARNING = 0x400,
	__BATINFO_MAX,
} BATINFO_ITEM;

struct BatteryProperties {
	bool chargerAcOnline;
	bool chargerUsbOnline;
	bool batteryPresent;
	int batteryStatus;
	int batteryHealth;
	int batteryLevel;
	int batteryVoltage;
	int batteryCurrentNow;
	int batteryTemperature;
	bool batteryLowCapWarn;
	bool batteryOverTempWarn;
};

struct sysfsStringEnumMap {
	const char *s;
	int val;
};

int mapSysfsString(const char *str, struct sysfsStringEnumMap map[]);

bool batinfo_get_usb_present(void);
bool batinfo_get_ac_present(void);
bool batinfo_get_bat_present(void);
int batinfo_get_bat_status();
int batinfo_get_bat_health();
int batinfo_get_bat_capacity();
int batinfo_get_bat_capacity_alert_level1();
int batinfo_get_bat_capacity_alert_level2();
int batinfo_get_bat_voltage();
int batinfo_get_bat_current();
int batinfo_get_bat_temp();
int batinfo_get_ic_temp();
int batinfo_get_all(struct BatteryProperties *props);

#ifdef BROADCAST_INFO
DBusConnection *batinfo_dbus_bus_get(void);
int batinfo_get_change(DBusConnection *conn,
			DBusMessage *msg,
			struct BatteryProperties *props,
			unsigned int *change);
#endif

#ifdef __cplusplus
}
#endif
#endif 	/* BATINFO_H */
