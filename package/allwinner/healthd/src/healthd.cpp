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

#include "healthd.h"
#include "BatteryMonitor.h"

#include <libgen.h>
#include <linux/input.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <dirent.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/time.h>
#include <signal.h>

using namespace softwinner;

// Periodic chores intervals in seconds
#define DEFAULT_PERIODIC_CHORES_INTERVAL_FAST (60)
#define DEFAULT_PERIODIC_CHORES_INTERVAL_SLOW (60*3)

#define POWER_SUPPLY_SUBSYSTEM "power_supply"

static struct healthd_config healthd_config = {
	.periodic_chores_interval_fast = DEFAULT_PERIODIC_CHORES_INTERVAL_FAST,
	.periodic_chores_interval_slow = DEFAULT_PERIODIC_CHORES_INTERVAL_SLOW,
	.batteryPresent = -1,
	.batteryCapacity = -1,
	.batteryVoltage = -1,
	.batteryTemperature = -1,
	.batteryCurrentNow = -1,
	.batteryStatus = -1,
	.batteryHealth = -1,
	.screen_on = NULL,

};

static int wakealarm_wake_interval = DEFAULT_PERIODIC_CHORES_INTERVAL_FAST;

static int eventct;
static int epollfd;

#define MAX_EPOLL_EVENTS 40
static int uevent_fd;
static int wakealarm_fd;

static int is_charger = 0;
static int awake_poll_interval = -1;

struct healthd_mode_ops *healthd_mode_ops;

/* Tina mode */
extern void healthd_mode_tina_init(struct healthd_config *config);
extern int healthd_mode_tina_preparetowait(void);
extern void healthd_mode_tina_battery_update(struct BatteryProperties *props);

/* Charger mode */
extern void healthd_mode_charger_init(struct healthd_config *config);
extern int healthd_mode_charger_preparetowait(void);
extern void healthd_mode_charger_heartbeat(void);
extern void healthd_mode_charger_battery_update(struct BatteryProperties *props);

/* NOPs for modes that need no special action */
static void healthd_mode_nop_init(struct healthd_config *config);
static int healthd_mode_nop_preparetowait(void);
static void healthd_mode_nop_heartbeat(void);
static void healthd_mode_nop_battery_update(struct BatteryProperties *props);

static struct healthd_mode_ops tina_ops = {
	.init = healthd_mode_tina_init,
	.preparetowait = healthd_mode_tina_preparetowait,
	.heartbeat = healthd_mode_nop_heartbeat,
	.battery_update = healthd_mode_tina_battery_update,
};

static struct healthd_mode_ops charger_ops = {
#ifdef SHUTDOWN_CHARGER
	.init = healthd_mode_charger_init,
	.preparetowait = healthd_mode_charger_preparetowait,
	.heartbeat = healthd_mode_charger_heartbeat,
	.battery_update = healthd_mode_charger_battery_update,
#else
	.init = NULL,
	.preparetowait = NULL,
	.heartbeat = NULL,
	.battery_update = NULL,
#endif
};

static struct healthd_mode_ops recovery_ops = {
	.init = healthd_mode_nop_init,
	.preparetowait = healthd_mode_nop_preparetowait,
	.heartbeat = healthd_mode_nop_heartbeat,
	.battery_update = healthd_mode_nop_battery_update,
};

static void healthd_mode_nop_init(struct healthd_config * /*config */ )
{
}

static int healthd_mode_nop_preparetowait(void)
{
	return -1;
}

static void healthd_mode_nop_heartbeat(void)
{
}

static void healthd_mode_nop_battery_update(struct BatteryProperties * /*props */ )
{
}

static BatteryMonitor *gBatteryMonitor;

#define PROP_NAME_MAX 256

int healthd_register_event(int fd, void (*handler) (uint32_tt))
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	if (is_charger)
		ev.events |= EPOLLWAKEUP;
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		TLOGE("epoll_ctl failed; errno=%d\n", errno);
		return -1;
	}

	eventct++;
	return 0;
}

static void wakealarm_set_interval(int interval)
{
	struct itimerspec itval;

	if (wakealarm_fd == -1)
		return;

	wakealarm_wake_interval = interval;

	if (interval == -1)
		interval = 0;

	itval.it_interval.tv_sec = interval;
	itval.it_interval.tv_nsec = 0;
	itval.it_value.tv_sec = interval;
	itval.it_value.tv_nsec = 0;

	if (timerfd_settime(wakealarm_fd, 0, &itval, NULL) == -1)
		TLOGE("wakealarm_set_interval: timerfd_settime failed\n");
}

static void import_kernel_nv(char *name, int for_emulator)
{
	char *value = strchr(name, '=');
	int name_len = strlen(name);

	if (value == 0)
		return;
	*value++ = 0;
	if (name_len == 0)
		return;

	if (!strncmp(name, "boot.mode", 9) && name_len > 9) {
		const char *boot_prop_name = name + 10;
		DLOG("boot_prop_name=%s\n", boot_prop_name);

		if (!strcmp(boot_prop_name, "charger")) {
			is_charger = 1;
		}
	}
}

void import_kernel_cmdline(int in_qemu, void (*import_kernel_nv) (char *name, int in_qemu))
{
	char cmdline[1024];
	char *ptr;
	int fd;

	fd = open("/proc/cmdline", O_RDONLY);
	if (fd >= 0) {
		int n = read(fd, cmdline, 1023);
		if (n < 0)
			n = 0;
		/* get rid of trailing newline, it happens */
		if (n > 0 && cmdline[n - 1] == '\n')
			n--;
		cmdline[n] = 0;
		close(fd);
	} else {
		cmdline[0] = 0;
	}
	ptr = cmdline;
	while (ptr && *ptr) {
		char *x = strchr(ptr, ' ');
		if (x != 0)
			*x++ = 0;
		import_kernel_nv(ptr, in_qemu);
		ptr = x;
	}
}

static void process_kernel_cmdline(void)
{
	chmod("/proc/cmdline", 0440);
	import_kernel_cmdline(0, import_kernel_nv);
}

int uevent_open_socket(int buf_sz, bool passcred)
{

	struct sockaddr_nl addr;
	int on = passcred;
	int s;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;
	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (s < 0)
		return -1;

	setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
	setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(s);
		return -1;
	}
	return s;
}

ssize_t uevent_kernel_multicast_uid_recv(int socket, void *buffer, size_t length, uid_t * user)
{
	struct iovec iov = { buffer, length };
	struct sockaddr_nl addr;
	char control[CMSG_SPACE(sizeof(struct ucred))];
	struct msghdr hdr = {
		&addr,
		sizeof(addr),
		&iov,
		1,
	};
	hdr.msg_control = (void *)control;
	hdr.msg_controllen = sizeof(control);
	*user = -1;
	ssize_t n = recvmsg(socket, &hdr, 0);
	if (n <= 0) {
		return n;
	}

	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&hdr);
	if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
		bzero(buffer, length);
		errno = EIO;
		return -1;
	}

	struct ucred *cred = (struct ucred *)CMSG_DATA(cmsg);
	*user = cred->uid;
	if (cred->uid != 0) {
		bzero(buffer, length);
		errno = EIO;
		return -1;
	}
	if (addr.nl_groups == 0 || addr.nl_pid != 0) {
		bzero(buffer, length);
		errno = EIO;
		return -1;
	}
	return n;
}

ssize_t uevent_kernel_multicast_recv(int socket, void *buffer, size_t length)
{
	uid_t user = -1;
	return uevent_kernel_multicast_uid_recv(socket, buffer, length, &user);
}

void healthd_battery_update(void)
{
	int new_wake_interval = gBatteryMonitor->update()?
	    healthd_config.periodic_chores_interval_fast : healthd_config.periodic_chores_interval_slow;

	if (new_wake_interval != wakealarm_wake_interval)
		wakealarm_set_interval(new_wake_interval);
	if (healthd_config.periodic_chores_interval_fast == -1)
		awake_poll_interval = -1;
	else
		awake_poll_interval =
		    new_wake_interval == healthd_config.periodic_chores_interval_fast ?
		    -1 : healthd_config.periodic_chores_interval_fast * 1000;
}

static void periodic_chores()
{
	healthd_battery_update();
}

#define UEVENT_MSG_LEN 2048
static void uevent_event(uint32_tt)
{
	char msg[UEVENT_MSG_LEN + 2];
	char *cp;
	int n;

	n = uevent_kernel_multicast_recv(uevent_fd, msg, UEVENT_MSG_LEN);
	if (n <= 0)
		return;
	if (n >= UEVENT_MSG_LEN)	/* overflow -- discard */
		return;
	msg[n] = '\0';
	msg[n + 1] = '\0';
	cp = msg;

	while (*cp) {
		if (!strcmp(cp, "SUBSYSTEM=" POWER_SUPPLY_SUBSYSTEM)) {
			healthd_battery_update();
			break;
		}
		while (*cp++) ;
	}
}

static void uevent_init(void)
{
	uevent_fd = uevent_open_socket(64 * 1024, true);

	if (uevent_fd < 0) {
		TLOGE("uevent_init: uevent_open_socket failed\n");
		return;
	}
	fcntl(uevent_fd, F_SETFL, O_NONBLOCK);
	if (healthd_register_event(uevent_fd, uevent_event))
		TLOGE("register for uevent events failed\n");

}

static const char *pwr_state_mem = "mem";
static int set_power_state_mem(void)
{
	const int SIZE = 256;
	char file[256];
	int ret;
	sprintf(file, "sys/power/%s", "state");
	int fd = open(file, O_RDWR, 0);
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

static void wakealarm_event(uint32_tt /*epevents */ )
{
	unsigned long long wakeups;

	DLOG("enter\n");
	if (read(wakealarm_fd, &wakeups, sizeof(wakeups)) == -1) {
		TLOGE("wakealarm_event: read wakealarm fd failed\n");
		return;
	}
	periodic_chores();
}

static void wakealarm_init(void)
{
	/* change CLOCK_BOOTTIME_ALARM to CLOCK_MONOTONIC, avoid wakeup frequently! */
	wakealarm_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (wakealarm_fd == -1) {
		TLOGE("wakealarm_init: timerfd_create failed\n");
		return;
	}

	if (healthd_register_event(wakealarm_fd, wakealarm_event))
		TLOGE("Registration of wakealarm event failed\n");

	wakealarm_set_interval(healthd_config.periodic_chores_interval_fast);
}

static void healthd_mainloop(void)
{
	while (1) {
		struct epoll_event events[eventct];
		int nevents;
		int timeout = awake_poll_interval;
		int mode_timeout;
		mode_timeout = healthd_mode_ops->preparetowait();
		DLOG("mode_timeout=%d\n", mode_timeout);
		if (timeout < 0 || (mode_timeout > 0 && mode_timeout < timeout))
			timeout = mode_timeout;
		nevents = epoll_wait(epollfd, events, eventct, timeout);
		if (nevents == -1) {
			if (errno == EINTR)
				continue;
			TLOGE("healthd_mainloop: epoll_wait failed\n");
			break;
		}

		for (int n = 0; n < nevents; ++n) {
			if (events[n].data.ptr)
				(*(void (*)(int))events[n].data.ptr) (events[n].events);
		}
		if (!nevents) {
			periodic_chores();
		}
		healthd_mode_ops->heartbeat();

	}

	return;

}

static int healthd_init()
{
	epollfd = epoll_create(MAX_EPOLL_EVENTS);
	if (epollfd == -1) {
		TLOGE("epoll_create failed; errno=%d\n", errno);
		return -1;
	}

	healthd_mode_ops->init(&healthd_config);
	wakealarm_init();
	uevent_init();
	gBatteryMonitor = new BatteryMonitor();
	gBatteryMonitor->init(&healthd_config, &is_charger);
	return 0;
}

int main(int argc, char **argv)
{
	int ret;

	/* tina_ops used to broadcast battery info */
	healthd_mode_ops = &tina_ops;
#if 1
	if(charger_ops.init) {
		process_kernel_cmdline();
		if (is_charger)
			healthd_mode_ops = &charger_ops;
	}
#else
	healthd_mode_ops = &charger_ops;
#endif
	ret = healthd_init();
	if (ret) {
		TLOGE("Initialization failed, exiting\n");
		exit(2);
	}

	healthd_mainloop();
	TLOGE("Main loop terminated, exiting\n");
	return 0;
}
