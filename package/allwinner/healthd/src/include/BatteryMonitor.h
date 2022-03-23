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

#ifndef HEALTHD_BATTERYMONITOR_H
#define HEALTHD_BATTERYMONITOR_H
#include <stdlib.h>
#include <stdio.h>

#include "healthd.h"

namespace softwinner {

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

class BatteryMonitor {
public:

	void init(struct healthd_config *hc, int *charger);
	bool update(void);

private:
	struct healthd_config *mHealthdConfig;
	int is_charge_bm;
	bool mBatteryDevicePresent;
	struct BatteryProperties props;
	int getBatteryStatus(const char *status);
	int getBatteryHealth(const char *status);
	int getvalue(char *buf);
	int readFromFile(const char *file, char *buf, size_t size);
	bool getBooleanField(const char *file);
	int getIntField(const char *file);
	int set_power_state_on(void);
	int set_power_state_mem(void);
};

}
#endif	/* HEALTHD_BATTERY_MONTIOR_H */
