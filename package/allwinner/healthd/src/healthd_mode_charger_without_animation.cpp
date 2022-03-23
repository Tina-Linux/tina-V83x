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

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)

#define BATTERY_UNKNOWN_TIME    (2 * MSEC_PER_SEC)
#define POWER_ON_KEY_TIME       (2 * MSEC_PER_SEC)
#define UNPLUGGED_SHUTDOWN_TIME (10 * MSEC_PER_SEC)
#define AUTOSUSPEND_TIMEOUT 	(30 * MSEC_PER_SEC)

#define BATTERY_FULL_THRESH     95

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define test_bit(bit, array) \
    ((array)[(bit)/BITS_PER_LONG] & (1 << ((bit) % BITS_PER_LONG)))


struct key_state {
	bool pending;
	bool down;
	int64_t timestamp;
};

struct charger {
	bool have_battery_state;
	bool charger_connected;
	bool can_enter_standby;
	int64_t next_charger_transition;
	int64_t next_key_check;
	int64_t next_pwr_check;

	struct key_state keys[KEY_MAX + 1];
};

static struct charger charger_state;
static struct healthd_config *healthd_config;
static struct BatteryProperties *batt_prop;

/* current time in milliseconds */
static int64_t curr_time_ms(void)
{
	struct timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	return tm.tv_sec * MSEC_PER_SEC + (tm.tv_nsec / NSEC_PER_MSEC);
}

static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";

static int autosuspend_enable(void)
{
	const int SIZE = 256;
	char file[256];
	int ret, fd;

	sprintf(file, "/sys/power/%s", "state");
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

static int autosuspend_disable(void)
{
	const int SIZE = 256;
	char file[256];
	int ret, fd;

	sprintf(file, "/sys/power/%s", "state");
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

static int request_suspend(bool enable)
{
	DLOG("request_suspend enable=%d\n", enable);
	if (enable)
		return autosuspend_enable();
	else
		return autosuspend_disable();
}

typedef int (*input_callback)(int fd, uint32_t epevents, void *data);
typedef int (*input_set_key_callback)(int code, int value, void *data);

struct fd_info {
	int fd;
	input_callback cb;
	void *data;
};

#define MAX_DEVICES 16
#define MAX_MISC_FDS 16
static int epollfd;
static struct epoll_event polledevents[MAX_DEVICES + MAX_MISC_FDS];
static struct fd_info ev_fdinfo[MAX_DEVICES + MAX_MISC_FDS];
static unsigned int ev_count;
static unsigned int ev_dev_count;
static unsigned int ev_misc_count;
static void charger_event_handler(uint32_tt /*epevents */ )
{
	int ret,n;
	int npolledevents;

	npolledevents = epoll_wait(epollfd, polledevents, ev_count, -1);
	if (npolledevents < 0)
		return ;
	for (n = 0; n < npolledevents; n++) {
		struct fd_info *fdi = (struct fd_info *)(polledevents[n].data.ptr);
		input_callback cb = fdi->cb;
		if (cb)
			cb(fdi->fd, polledevents[n].events, fdi->data);
	}
	return ;
}

int input_init(input_callback input_cb, void *data)
{
	DIR *dir;
	struct dirent *de;
	int fd;
	struct epoll_event ev;
	bool epollctlfail = false;

	epollfd = epoll_create(MAX_DEVICES + MAX_MISC_FDS);
	if (epollfd == -1)
		return -1;

	dir = opendir("/dev/input");
	if (dir != 0) {
		while ((de = readdir(dir))) {
			unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];
			if (strncmp(de->d_name, "event", 5))
				continue;
			fd = openat(dirfd(dir), de->d_name, O_RDONLY);
			if (fd < 0)
				continue;

			/* read the evbits of the input device */
			if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
				close(fd);
				continue;
			}

			/* TODO: add ability to specify event masks. For now, just assume
			 * that only EV_KEY and EV_REL event types are ever needed. */
			if (!test_bit(EV_KEY, ev_bits) && !test_bit(EV_REL, ev_bits)) {
				close(fd);
				continue;
			}

			ev.events = EPOLLIN | EPOLLWAKEUP;
			ev.data.ptr = (void *)&ev_fdinfo[ev_count];
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)) {
				close(fd);
				epollctlfail = true;
				continue;
			}

			ev_fdinfo[ev_count].fd = fd;
			ev_fdinfo[ev_count].cb = input_cb;
			ev_fdinfo[ev_count].data = data;
			ev_count++;
			ev_dev_count++;
			if (ev_dev_count == MAX_DEVICES)
				break;
		}
	}
	return epollfd;
}
int input_sync_key_state(input_set_key_callback set_key_cb, void *data)
{
    unsigned long key_bits[BITS_TO_LONGS(KEY_MAX)];
    unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];
    unsigned i;
    int ret;

    for (i = 0; i < ev_dev_count; i++) {
        int code;

        memset(key_bits, 0, sizeof(key_bits));
        memset(ev_bits, 0, sizeof(ev_bits));

        ret = ioctl(ev_fdinfo[i].fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits);
        if (ret < 0 || !test_bit(EV_KEY, ev_bits))
            continue;

        ret = ioctl(ev_fdinfo[i].fd, EVIOCGKEY(sizeof(key_bits)), key_bits);
        if (ret < 0)
            continue;

        for (code = 0; code <= KEY_MAX; code++) {
            if (test_bit(code, key_bits))
                set_key_cb(code, 1, data);
        }
    }

    return 0;
}

static int set_key_callback(int code, int value, void *data)
{
	struct charger *charger = (struct charger *)data;
	int64_t now = curr_time_ms();
	int down = ! !value;

	if (code > KEY_MAX)
		return -1;

	/* ignore events that don't modify our state */
	if (charger->keys[code].down == down)
		return 0;

	/* only record the down even timestamp, as the amount
	 * of time the key spent not being pressed is not useful */
	if (down)
		charger->keys[code].timestamp = now;
	charger->keys[code].down = down;
	charger->keys[code].pending = true;
	if (down) {
		DLOG("[%" PRId64 "] key[%d] down\n", now, code);
	} else {
		int64_t duration = now - charger->keys[code].timestamp;
		int64_t secs = duration / 1000;
		int64_t msecs = duration - secs * 1000;
		DLOG("[%" PRId64 "] key[%d] up (was down for %" PRId64 ".%" PRId64 "sec)\n", now, code, secs, msecs);
	}

	return 0;
}

static void update_input_state(struct charger *charger, struct input_event *ev)
{
	if (ev->type != EV_KEY)
		return;
	set_key_callback(ev->code, ev->value, charger);
}

static void set_next_key_check(struct charger *charger, struct key_state *key, int64_t timeout)
{
	int64_t then = key->timestamp + timeout;
	if (charger->next_key_check == -1 || then < charger->next_key_check)
		charger->next_key_check = then;
}

static void process_key(struct charger *charger, int code, int64_t now)
{
	struct key_state *key = &charger->keys[code];
	int64_t reboot_timeout = key->timestamp + POWER_ON_KEY_TIME;

	DLOG("timestamp=%lld, now=%lld, reboot_timeout=%lld\n",
	     key->timestamp, now, reboot_timeout);
	if (code == KEY_POWER) {
		if (key->down) {
			if (now >= reboot_timeout) {
				DLOG("[%" PRId64 "] rebooting\n", now);
				reboot(RB_AUTOBOOT);
			} else {
				/* if the key is pressed but timeout hasn't expired,
				 * make sure we wake up at the right-ish time to check
				 */
				DLOG("power_key pressed but timeout hasn't expired\n");
				//request_suspend(false);
				set_next_key_check(charger, key, (POWER_ON_KEY_TIME));
			}
		} else {
			/* if the power key got released, force screen state cycle */
			if (key->pending) {
				if (!charger->can_enter_standby) {
					DLOG("release key, but can_enter_standby is false!\n");
					charger->can_enter_standby = true;
					request_suspend(false);
				} else {
					charger->next_charger_transition = -1;
					if (charger->charger_connected) {
						charger->can_enter_standby = false;
						request_suspend(true);
						charger->next_charger_transition = now + AUTOSUSPEND_TIMEOUT;
					}
				}
			}
		}
	}

	key->pending = false;
}

static void handle_input_state(struct charger *charger, int64_t now)
{
	process_key(charger, KEY_POWER, now);
	if (charger->next_key_check != -1 && now > charger->next_key_check)
		charger->next_key_check = -1;
}

static void handle_power_supply_state(struct charger *charger, int64_t now)
{
	if (!charger->have_battery_state)
		return;
	if (!charger->charger_connected) {
		request_suspend(false);
		if (charger->next_pwr_check == -1) {
			charger->next_pwr_check = now + UNPLUGGED_SHUTDOWN_TIME;
			DLOG("[%" PRId64 "] device unplugged: shutting down in %" PRId64 " (@ %" PRId64 ")\n",
			     now, (int64_t) UNPLUGGED_SHUTDOWN_TIME, charger->next_pwr_check);
		} else if (now >= charger->next_pwr_check) {
			DLOG("[%" PRId64 "] shutting down\n", now);
			reboot(RB_POWER_OFF);
		} else {
			/* otherwise we already have a shutdown timer scheduled */
		}
	} else {
		/* online supply present, reset shutdown timer if set */
		if (charger->next_pwr_check != -1) {
			DLOG("[%" PRId64 "] device plugged in: shutdown cancelled\n", now);
		}

		charger->next_pwr_check = -1;
	}
}

void handle_autosuspend_state(struct charger *charger, int64_t now)
{
	if (now < charger->next_charger_transition)
		return;
	if (charger->can_enter_standby &&
		charger->charger_connected &&
		now > charger->next_charger_transition) {
		DLOG("[%" PRId64 "] request_suspend(true)---\n", now);
		charger->can_enter_standby = false;
		request_suspend(true);
		charger->next_charger_transition = now + AUTOSUSPEND_TIMEOUT;
	}
}

void healthd_mode_charger_heartbeat()
{
	struct charger *charger = &charger_state;
	int64_t now = curr_time_ms();
	int ret;

	DLOG("healthd_mode_charger_heartbeat\n");
	handle_input_state(charger, now);
	handle_power_supply_state(charger, now);

	handle_autosuspend_state(charger, now);

}

void healthd_mode_charger_battery_update(struct BatteryProperties *props)
{
	struct charger *charger = &charger_state;

	charger->charger_connected = props->chargerAcOnline || props->chargerUsbOnline;
	DLOG("healthd_mode_charger_battery_update charger_connected=%d\n", charger->charger_connected);
	DLOG("chargerAcOnline=%d\n", props->chargerAcOnline);
	DLOG("chargerUsbOnline=%d\n", props->chargerUsbOnline);
	if (!charger->have_battery_state) {
		charger->have_battery_state = true;
		charger->can_enter_standby = true;
		charger->next_charger_transition = curr_time_ms() + AUTOSUSPEND_TIMEOUT;
	}
	batt_prop = props;
}

int healthd_mode_charger_preparetowait(void)
{
	struct charger *charger = &charger_state;
	int64_t now = curr_time_ms();
	int64_t next_event = INT64_MAX;
	int64_t timeout;
	struct input_event ev;
	int ret;

	DLOG("[%" PRId64 "] next key: %" PRId64 " next pwr: %" PRId64 "\n", now,
	     charger->next_key_check, charger->next_pwr_check);

	if (charger->next_charger_transition != -1)
		next_event = charger->next_charger_transition;
	if (charger->next_key_check != -1 && charger->next_key_check < next_event)
		next_event = charger->next_key_check;
	if (charger->next_pwr_check != -1 && charger->next_pwr_check < next_event)
		next_event = charger->next_pwr_check;

	if (next_event != -1 && next_event != INT64_MAX)
		timeout = max(0, next_event - now);
	else
		timeout = -1;

	return (int)timeout;
}

static int input_callback_func(int fd, unsigned int epevents, void *data)
{
	struct charger *charger = (struct charger *)data;
	struct input_event ev;
	int r;

	if (epevents & EPOLLIN) {
		r = read(fd, &ev, sizeof(ev));
		if (r == sizeof(ev))
			update_input_state(charger, &ev);
		else
			return -1;
	}
}

void healthd_mode_charger_init(struct healthd_config *config)
{
	struct charger *charger = &charger_state;
	int i;

	printf("--------------- STARTING CHARGER MODE ---------------\n");
	printf("\n");

	/* input event init */
	epollfd = input_init(input_callback_func, charger);
	if (epollfd > 0) {
		/* input event add into epollfd */
		healthd_register_event(epollfd, charger_event_handler);
	}

	/* key event init */
	input_sync_key_state(set_key_callback, charger);

	healthd_config = config;
}

