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
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <sys/epoll.h>
#include "healthd.h"
#include "BatteryMonitor.h"

#define POWER_SUPPLY_SUBSYSTEM "power_supply"
#define POWER_SUPPLY_SYSFS_PATH "/sys/class/" POWER_SUPPLY_SUBSYSTEM

#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)

#define BATTERY_UNKNOWN_TIME    (2 * MSEC_PER_SEC)
#define POWER_ON_KEY_TIME       (2 * MSEC_PER_SEC)
#define UNPLUGGED_SHUTDOWN_TIME (3 * MSEC_PER_SEC)
#define WAKEUP_TIME (2 * NSEC_PER_MSEC * MSEC_PER_SEC)

#define WAKEUP_TIME_MORE (60 * NSEC_PER_MSEC * MSEC_PER_SEC)

namespace softwinner {

int LastAcOnline = 0;
int LastUsbOnline = 0;
int LastPowerKey = 0;
int AcOnline = 0;
int UsbOnline = 0;
int PowerKey = 0;

static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";

static int acquire_wake_lock_timeout(long long timeout)
{
	int fd, ret;
	char str[64];

	fd = open("/sys/power/wake_lock", O_WRONLY, 0);
	if (fd < 0)
		 return -1;

	 sprintf(str, "charge %lld", timeout);
	 ret = write(fd, str, strlen(str));
	 close(fd);
	 return ret;
}

int BatteryMonitor::set_power_state_mem(void)
{
	const int SIZE = 256;
	char file[256];
	int ret, fd;

	sprintf(file, "sys/power/%s", "state");
	fd = open(file, O_RDWR, 0);
	if (fd == -1) {
		TLOGE("Could not open '%s'\n", file);
		return -1;
	}
	ret = write(fd, pwr_state_mem, strlen(pwr_state_mem));
	if (ret < 0) {
		TLOGE("set_power_state_mem err\n");
	}
	close(fd);
	return 0;
}

int BatteryMonitor::set_power_state_on(void)
{
	const int SIZE = 256;
	char file[256];
	int ret, fd;

	sprintf(file, "sys/power/%s", "state");
	fd = open(file, O_RDWR, 0);
	if (fd == -1) {
		TLOGE("Could not open '%s'\n", file);
		return -1;
	}
	ret = write(fd, pwr_state_on, strlen(pwr_state_on));
	if (ret < 0) {
		TLOGE("set_power_state_on err\n");
	}
	close(fd);
	return 0;
}

bool BatteryMonitor::update(void)
{
	const int SIZE = 256;
	char buf[SIZE];
	char file[256];
	int ret;

	props.chargerAcOnline = false;
	props.chargerUsbOnline = false;
	props.batteryStatus = BATTERY_STATUS_UNKNOWN;
	props.batteryHealth = BATTERY_HEALTH_UNKNOWN;

	/* present */
	mHealthdConfig->batteryPresent = batinfo_get_bat_present();
	props.batteryPresent = mHealthdConfig->batteryPresent;
	TLOGI("update-mHealthdConfig->batteryPresent=%d\n", mHealthdConfig->batteryPresent);

	/* capacity */
	mHealthdConfig->batteryCapacity = batinfo_get_bat_capacity();
	if (mHealthdConfig->batteryCapacity < 0)
		props.batteryLevel = 0;
	else
		props.batteryLevel = mHealthdConfig->batteryCapacity;
	TLOGI("update-mHealthdConfig->batteryCapacity=%d\n", mHealthdConfig->batteryCapacity);

	/* voltage_now */
	mHealthdConfig->batteryVoltage = batinfo_get_bat_voltage();
	if (mHealthdConfig->batteryVoltage < 0)
		props.batteryVoltage = 0;
	else
		props.batteryVoltage = mHealthdConfig->batteryVoltage;
	TLOGI("update-mHealthdConfig->batteryVoltage=%d\n", mHealthdConfig->batteryVoltage);

	/* temperture */
	mHealthdConfig->batteryTemperature = batinfo_get_bat_temp();
	if (mHealthdConfig->batteryTemperature == -0xFF)
		mHealthdConfig->batteryTemperature = 0;
	else
		props.batteryTemperature = mHealthdConfig->batteryTemperature;
	TLOGI("update-mHealthdConfig->batteryTemperature=%d\n", mHealthdConfig->batteryTemperature);

	/* current */
	mHealthdConfig->batteryCurrentNow = batinfo_get_bat_current();
	if (mHealthdConfig->batteryCurrentNow < 0)
		props.batteryCurrentNow = 0;
	else
		props.batteryCurrentNow = mHealthdConfig->batteryCurrentNow;
	TLOGI("update-mHealthdConfig->batteryCurrentNow=%d\n", mHealthdConfig->batteryCurrentNow);

	/* status */
	ret = batinfo_get_bat_status();
	if (ret > 0) {
		mHealthdConfig->batteryStatus = ret;
		props.batteryStatus = mHealthdConfig->batteryStatus;
	}
	TLOGI("update-mHealthdConfig->batteryStatus=%d\n", mHealthdConfig->batteryStatus);

	/* health */
	ret = batinfo_get_bat_health();
	if (ret > 0) {
		mHealthdConfig->batteryHealth = ret;
		props.batteryHealth = mHealthdConfig->batteryHealth;
	}
	TLOGI("update-mHealthdConfig->batteryHealth=%d\n", mHealthdConfig->batteryHealth);

	/* ac online */
	AcOnline = batinfo_get_ac_present();
	props.chargerAcOnline = AcOnline;
	TLOGI("update-ac online=%d\n", AcOnline);

	/* usb online */
	UsbOnline = batinfo_get_usb_present();
	props.chargerUsbOnline = UsbOnline;
	TLOGI("update-usb online=%d\n", UsbOnline);

	healthd_mode_ops->battery_update(&props);

	TLOGD("healthd:l-%d c-%d u-%d A-%d v-%d h-%d p-%d\n", mHealthdConfig->batteryCapacity,
	      mHealthdConfig->batteryCurrentNow, UsbOnline, AcOnline, mHealthdConfig->batteryVoltage,
	      mHealthdConfig->batteryHealth, mHealthdConfig->batteryPresent);

	LastAcOnline = AcOnline;
	LastUsbOnline = UsbOnline;
	return AcOnline | UsbOnline;
}

void BatteryMonitor::init(struct healthd_config *hc, int *charger)
{
	DIR *dir = opendir(POWER_SUPPLY_SYSFS_PATH);
	char buf[256];
	char file[256];
	int ret;
	int batteryStatus;
	const int SIZE = 128;
	mHealthdConfig = hc;
	is_charge_bm = *charger;

	if (dir == NULL) {
		TLOGE("Could not open %s\n", POWER_SUPPLY_SYSFS_PATH);
	} else {
		struct dirent *entry;
		while ((entry = readdir(dir))) {
			const char *name = entry->d_name;

			if (!strcmp(name, ".") || !strcmp(name, ".."))
				continue;

			if (!strcmp(name, "usb")) {
				LastUsbOnline = batinfo_get_usb_present();
				TLOGI("Init usb_online=%d\n", LastUsbOnline);
			} else if (!strcmp(name, "ac")) {
				LastAcOnline = batinfo_get_ac_present();
				TLOGI("Init ac_online=%d\n", LastAcOnline);
			} else if (!strcmp(name, "battery")) {
				mBatteryDevicePresent = true;

				/* status */
				ret = batinfo_get_bat_status();
				if (ret > 0)
					mHealthdConfig->batteryStatus = ret;
				TLOGI("Init batteryStatus=%d\n", mHealthdConfig->batteryStatus);

				/* health */
				ret = batinfo_get_bat_health();
				if (ret > 0)
					mHealthdConfig->batteryHealth = ret;
				TLOGI("Init batteryHealth=%d\n", mHealthdConfig->batteryHealth);

				/* present */
				mHealthdConfig->batteryPresent = batinfo_get_bat_present();
				TLOGI("Init batteryPresen=%d\n", mHealthdConfig->batteryPresent);

				/* capacity */
				mHealthdConfig->batteryCapacity = batinfo_get_bat_capacity();
				TLOGI("Init batteryCapacity=%d\n", mHealthdConfig->batteryCapacity);

				/* voltage_now */
				mHealthdConfig->batteryVoltage = batinfo_get_bat_voltage();
				TLOGI("Init batteryVoltage=%d\n", mHealthdConfig->batteryVoltage);

				/* temp */
				mHealthdConfig->batteryTemperature = batinfo_get_bat_temp();
				TLOGI("Init batteryTemperature=%d\n", mHealthdConfig->batteryTemperature);

				/* current_now */
				mHealthdConfig->batteryCurrentNow = batinfo_get_bat_current();
				TLOGI("Init batteryCurrentNow=%d\n", mHealthdConfig->batteryCurrentNow);
			}
		}

	}
	DLOG("Battery property initial\n");
	DLOG("usb_present:%d, ac_present:%d, bat_present:%d\n",
		LastUsbOnline, LastAcOnline, mBatteryDevicePresent);
	DLOG("status=%d,health=%d,present=%d,cap=%d,vol=%d,cur=%d,temp=%d\n",
		mHealthdConfig->batteryStatus, mHealthdConfig->batteryHealth,
		mHealthdConfig->batteryPresent, mHealthdConfig->batteryCapacity,
		mHealthdConfig->batteryVoltage, mHealthdConfig->batteryTemperature,
		mHealthdConfig->batteryCurrentNow);
	closedir(dir);

	if (!mBatteryDevicePresent) {
		TLOGE("No battery devices found\n");
		hc->periodic_chores_interval_fast = -1;
		hc->periodic_chores_interval_slow = -1;
	}

}

}
