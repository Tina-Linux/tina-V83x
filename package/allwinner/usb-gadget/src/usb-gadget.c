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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/netlink.h>

#include "usb-gadget.h"
/*
 * function: enalbe gadget mass storage.
 * 1 - enable mass storage function
 * 0 - disable mass storage function
*/
static int tina_gadget_mass_storage_enable(int enable)
{

	if (enable) {
		system("echo 0 > "
		       "/sys/devices/virtual/android_usb/android0/enable");
		system("echo mass_storage,adb > "
		       "/sys/devices/virtual/android_usb/android0/functions");
		system("echo 1 > "
		       "/sys/devices/virtual/android_usb/android0/enable");
		return 1;
	} else {
		system("echo adb > "
		       "/sys/devices/virtual/android_usb/android0/functions");
		system("echo 0 > "
		       "/sys/devices/virtual/android_usb/android0/enable");
		return 0;
	}
}

/*
 * function: get usb connect state
 * return :
 *  1  - connect to pc
 *  0  - connect to charger
 * -1  - error state
*/
int tina_gadget_get_connect_state(void)
{
	char str[16];
	int state = 0;
	int fd = open(LRADC_CHARGING_PATH, O_RDONLY);
	if (fd < 0) {
		printf("unable to open usb connect state path.\n",
		       strerror(errno));
		close(fd);
		return 0;
	}
	if (read(fd, str, sizeof(str)) < 0) {
		printf("unable to read state..\n", strerror(errno));
		close(fd);
		return 0;
	}
	if (str[0] == '2' || str[0] == '3') {
		state = 1;
	} else if (str[0] == '1') {
		state = 0;
	} else {
		state = -1;
	}

	close(fd);

	return state;
}

/*
 * function: set mmc mass storaget enable
*/
void tina_gadget_is_mmc_storage_enable(int enable)
{
	int fd = open(MMC_STORAGE_PATH, O_RDONLY);
	if (fd < 0) {
		printf("unable to find sdcard path.\n", strerror(errno));
		close(fd);
		return;
	}

	if (enable) {
		tina_gadget_mass_storage_enable(1);
		system("echo /dev/mmcblk0 > "
		       "/sys/devices/virtual/android_usb/android0/"
		       "f_mass_storage/lun/file");
	} else {
		tina_gadget_mass_storage_enable(0);
		system("echo \"\" > "
		       "/sys/devices/virtual/android_usb/android0/"
		       "f_mass_storage/lun/file");
	}
}

/*
 * function: create an endpoint to recv kobject uevent
 * return :
 * >0  - socket descriptor
 * -1  - error state
*/
int tina_gadget_open_socket(void)
{
	struct sockaddr_nl nls;
	int nlbufsize = 64 * 1024;
	int s;

	memset(&nls,0,sizeof(struct sockaddr_nl));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	if ((s = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT)) == -1) {
		fprintf(stderr, "Failed to open hotplug socket: %s\n", strerror(errno));
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &nlbufsize, sizeof(nlbufsize))) {
		fprintf(stderr, "Failed to resize receive buffer: %s\n", strerror(errno));
		close(s);
		return -1;
	}

	if (bind(s, (void *)&nls, sizeof(struct sockaddr_nl))) {
		fprintf(stderr, "Failed to bind hotplug socket: %s\n", strerror(errno));
		close(s);
		return -1;
	}

	fcntl(s, F_SETFL, O_NONBLOCK);

	return s;
}

#define UEVENT_MSG_LEN 1024
/*
 * function: recv kobject uevent and detect eject msg
 * return :
 *  0  - detect eject msg
 * -1  - error state
*/
int tina_gadget_ums_is_eject(int socket)
{
	char buf[UEVENT_MSG_LEN];
	int len,i=0;

	len = recv(socket, buf, sizeof(buf), MSG_DONTWAIT);

	if (len < 1)
		return -1;
	if (len >= UEVENT_MSG_LEN)
		return -1;
	buf[len] = '\0';

#if 0
{
	int l=0;
	printf("recv uevent:\n");
	while(l < len) {
		printf("%s\n", &buf[l]);
		l += strlen(&buf[l]) + 1;
	}
	printf("=========================\n");
}
#endif

	while(i < len)  {
		int l = strlen(buf + i)+1;
		if (strstr(&buf[i], "USB_MASS_STORAGE=EJECTED"))
			return 0;
		i += l;
	}

	return -1;
}
