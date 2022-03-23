/*
 * tina usb gadget interface  to mount mass storage.
 *
 * Copyright (c) 2017  Allwinner Technology Co., Ltd.
 *
 * Author: huangshr <huangshr@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __TINA_USB_GADGET__
#define __TINA_USB_GADGET__

enum LRADC_CHARGE_TYPE {
	LRADC_CHARGE_AC = 1,
	LRADC_CHARGE_USB_20,
	LRADC_CHARGE_USB_30,
};

#define MMC_STORAGE_PATH    "/dev/mmcblk0"
#define LRADC_CHARGING_PATH "/sys/devices/platform/lradc_battery/charging"
#define GADGET_STATE_PATH   "/sys/devices/virtual/android_usb/android0/state"
#define GADGET_ENABLE_PATH  "/sys/devices/virtual/android_usb/android0/enable"
#define GADGET_UMS_LUNFILE  \
	"/sys/devices/virtual/android_usb/android0/f_mass_storage/lun/file"

int tina_gadget_get_connect_state(void);
void tina_gadget_is_mmc_storage_enable(int enable);
int tina_gadget_ums_is_eject(int socket);
int tina_gadget_open_socket(void);

#endif
