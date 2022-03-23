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

#ifndef _HEALTHD_H_
#define _HEALTHD_H_

#include <sys/types.h>
#include <string.h>
#include "batinfo/batinfo.h"

#if 0
#define DLOG(fmt, arg...) 		printf("[%s:%u] "fmt"", __FUNCTION__, __LINE__, ##arg)
#else
#define DLOG(fmt, arg...)
#endif

struct healthd_config {
	int periodic_chores_interval_fast;
	int periodic_chores_interval_slow;
	int batteryPresent;
	int batteryCapacity;
	int batteryVoltage;
	int batteryTemperature;
	int batteryCurrentNow;
	int batteryStatus;
	int batteryHealth;
	 bool(*screen_on) (BatteryProperties *props);
};

typedef unsigned int __u32;
typedef unsigned long uint32_tt;

int healthd_register_event(int fd, void (*handler) (uint32_tt));
void healthd_battery_update();

struct healthd_mode_ops {
	void (*init) (struct healthd_config *config);
	int (*preparetowait) (void);
	void (*heartbeat) (void);
	void (*battery_update) (struct BatteryProperties *props);
};

extern struct healthd_mode_ops *healthd_mode_ops;

// Charger mode

void healthd_mode_charger_init(struct healthd_config *config);
int healthd_mode_charger_preparetowait(void);
void healthd_mode_charger_heartbeat(void);
void healthd_mode_charger_battery_update(struct BatteryProperties *props);

#endif /* _HEALTHD_H_ */
