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
#include "minui/minui.h"
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

#define BATTERY_FULL_THRESH     95

#define LAST_KMSG_PATH          "/proc/last_kmsg"
#define LAST_KMSG_PSTORE_PATH   "/sys/fs/pstore/console-ramoops"
#define LAST_KMSG_MAX_SZ        (32 * 1024)

#define WAKEALARM_PATH        "/sys/class/rtc/rtc0/wakealarm"
#define ALARM_IN_BOOTING_PATH        "/sys/module/rtc_sunxi/parameters/alarm_in_booting"

typedef GRSurface* gr_surface;

struct key_state {
	bool pending;
	bool down;
	int64_t timestamp;
};

struct frame {
	int disp_time;
	int min_capacity;
	bool level_only;

	gr_surface surface;
};

struct animation {
	bool run;

	struct frame *frames;
	int cur_frame;
	int num_frames;

	int cur_cycle;
	int num_cycles;

	/* current capacity being animated */
	int capacity;
};

struct charger {
	bool have_battery_state;
	bool charger_connected;
	int64_t next_screen_transition;
	int64_t next_key_check;
	int64_t next_pwr_check;

	struct key_state keys[KEY_MAX + 1];

	struct animation *batt_anim;
	gr_surface surf_unknown;
};

static struct frame batt_anim_frames[] = {
	{
	 .disp_time = 750,
	 .min_capacity = 0,
	 .level_only = false,
	 .surface = NULL,
	 },
	{
	 .disp_time = 750,
	 .min_capacity = 20,
	 .level_only = false,
	 .surface = NULL,
	 },
	{
	 .disp_time = 750,
	 .min_capacity = 40,
	 .level_only = false,
	 .surface = NULL,
	 },
	{
	 .disp_time = 750,
	 .min_capacity = 60,
	 .level_only = false,
	 .surface = NULL,
	 },
	{
	 .disp_time = 750,
	 .min_capacity = 80,
	 .level_only = true,
	 .surface = NULL,
	 },
	{
	 .disp_time = 750,
	 .min_capacity = BATTERY_FULL_THRESH,
	 .level_only = false,
	 .surface = NULL,
	 },
};

static struct animation battery_animation = {
	.run = false,
	.frames = batt_anim_frames,
	.cur_frame = 0,
	.num_frames = ARRAY_SIZE(batt_anim_frames),
	.cur_cycle = 0,
	.num_cycles = 3,
	.capacity = 0,
};

static struct charger charger_state;
static struct healthd_config *healthd_config;
static struct BatteryProperties *batt_prop;
static int char_width;
static int char_height;
static bool minui_inited;

static int alarm_fd = 0;
static pthread_t tid_alarm;

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
	int ret,fd;

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
	int ret,fd;

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

static void clear_screen(void)
{
	gr_color(0, 0, 0, 255);
	gr_clear();
}

static int draw_text(const char *str, int x, int y)
{
	int str_len_px = gr_measure(gr_sys_font(), str);

	if (x < 0)
		x = (gr_fb_width() - str_len_px) / 2;
	if (y < 0)
		y = (gr_fb_height() - char_height) / 2;
	gr_text(gr_sys_font(), x, y, str, 0);

	return y + char_height;
}

static void android_green(void)
{
	gr_color(0xa4, 0xc6, 0x39, 255);
}

static int draw_surface_centered(struct charger * /*charger */ , gr_surface surface)
{
	int w;
	int h;
	int x;
	int y;
	w = gr_get_width(surface);
	h = gr_get_height(surface);
	x = (gr_fb_width() - w) / 2;
	y = (gr_fb_height() - h) / 2;

	DLOG("drawing surface %dx%d+%d+%d\n", w, h, x, y);
	gr_blit(surface, 0, 0, w, h, x, y);
	return y + h;
}

static void draw_unknown(struct charger *charger)
{
	int y;
	if (charger->surf_unknown) {
		draw_surface_centered(charger, charger->surf_unknown);
	} else {
		android_green();
		y = draw_text("Charging!", -1, -1);
		draw_text("?\?/100", -1, y + 25);
	}
}

static void draw_battery(struct charger *charger)
{
	struct animation *batt_anim = charger->batt_anim;
	struct frame *frame = &batt_anim->frames[batt_anim->cur_frame];

	if (batt_anim->num_frames != 0) {
		draw_surface_centered(charger, frame->surface);
		DLOG("drawing frame #%d min_cap=%d time=%d\n",
		     batt_anim->cur_frame, frame->min_capacity, frame->disp_time);
	}
}

#define STR_LEN 64
static void draw_capacity(struct charger *charger)
{
	char cap_str[STR_LEN];
	int x, y;
	int str_len_px;

	snprintf(cap_str, (STR_LEN - 1), "%d%%", charger->batt_anim->capacity);
	str_len_px = gr_measure(gr_sys_font(), cap_str);
	x = (gr_fb_width() - str_len_px) / 2;
	y = (gr_fb_height() + char_height) / 2;
	android_green();
	gr_text(gr_sys_font(), x, y, cap_str, 0);
}

static void redraw_screen(struct charger *charger)
{
	struct animation *batt_anim = charger->batt_anim;

	clear_screen();

	/* try to display *something* */
	if (batt_anim->capacity < 0 || batt_anim->num_frames == 0) {
		draw_unknown(charger);
	} else {
		draw_battery(charger);
		draw_capacity(charger);
	}
	gr_flip();
}

static void kick_animation(struct animation *anim)
{
	anim->run = true;
}

static void reset_animation(struct animation *anim)
{
	anim->cur_cycle = 0;
	anim->cur_frame = 0;
	anim->run = false;
}

static void update_screen_state(struct charger *charger, int64_t now)
{
	struct animation *batt_anim = charger->batt_anim;
	int cur_frame;
	int disp_time;

	if (!batt_anim->run || now < charger->next_screen_transition)
		return;

	if (!minui_inited) {
		if (healthd_config && healthd_config->screen_on) {
			if (!healthd_config->screen_on(batt_prop)) {
				DLOG("[%" PRId64 "] leave screen off\n", now);
				batt_anim->run = false;
				charger->next_screen_transition = -1;
				if (charger->charger_connected)
					request_suspend(true);
				return;
			}
		}

		gr_init();
		gr_font_size(gr_sys_font(), &char_width, &char_height);

#ifndef CHARGER_DISABLE_INIT_BLANK
		gr_fb_blank(true);
#endif
		minui_inited = true;
	}

	/* animation is over, blank screen and leave */
	if (batt_anim->cur_cycle == batt_anim->num_cycles) {
		reset_animation(batt_anim);
		charger->next_screen_transition = -1;
		gr_fb_blank(true);
		DLOG("[%" PRId64 "] animation done\n", now);

		if (charger->charger_connected) {
			DLOG("[%" PRId64 "] request_suspend(true)---update_screen_state\n", now);
			request_suspend(true);
		}
		return;
	}

	disp_time = batt_anim->frames[batt_anim->cur_frame].disp_time;

	if (batt_anim->cur_frame == 0) {
		int ret;

		DLOG("[%" PRId64 "] animation starting\n", now);
		if (batt_prop && batt_prop->batteryLevel >= 0 && batt_anim->num_frames != 0) {
			int i;

			for (i = 1; i < batt_anim->num_frames; i++) {
				if (batt_prop->batteryLevel < batt_anim->frames[i].min_capacity)
					break;
			}
			batt_anim->cur_frame = i - 1;

			disp_time = batt_anim->frames[batt_anim->cur_frame].disp_time * 2;
		}
		if (batt_prop)
			batt_anim->capacity = batt_prop->batteryLevel;
	}
	/* unblank the screen  on first cycle */
	if (batt_anim->cur_cycle == 0)
		gr_fb_blank(false);

	/* draw the new frame (@ cur_frame) */
	redraw_screen(charger);

	/* if we don't have anim frames, we only have one image, so just bump
	 * the cycle counter and exit
	 */
	if (batt_anim->num_frames == 0 || batt_anim->capacity < 0) {
		DLOG("[%" PRId64 "] animation missing or unknown battery status\n", now);
		charger->next_screen_transition = now + BATTERY_UNKNOWN_TIME;
		batt_anim->cur_cycle++;
		return;
	}

	/* schedule next screen transition */
	charger->next_screen_transition = now + disp_time;

	/* advance frame cntr to the next valid frame only if we are charging
	 * if necessary, advance cycle cntr, and reset frame cntr
	 */
	if (charger->charger_connected) {
		batt_anim->cur_frame++;

		/* if the frame is used for level-only, that is only show it when it's
		 * the current level, skip it during the animation.
		 */
		while (batt_anim->cur_frame < batt_anim->num_frames &&
		       batt_anim->frames[batt_anim->cur_frame].level_only)
			batt_anim->cur_frame++;
		if (batt_anim->cur_frame >= batt_anim->num_frames) {
			batt_anim->cur_cycle++;
			batt_anim->cur_frame = 0;

		}
	} else {

		batt_anim->cur_frame = 0;
		batt_anim->cur_cycle++;
	}
}

static int set_key_callback(charger* charger, int code, int value) {
    int64_t now = curr_time_ms();
    bool down = !!value;

    if (code > KEY_MAX) return -1;

    /* ignore events that don't modify our state */
    if (charger->keys[code].down == down) return 0;

    /* only record the down even timestamp, as the amount
     * of time the key spent not being pressed is not useful */
    if (down) charger->keys[code].timestamp = now;
    charger->keys[code].down = down;
    charger->keys[code].pending = true;
    if (down) {
        DLOG("[%" PRId64 "] key[%d] down\n", now, code);
    } else {
        int64_t duration = now - charger->keys[code].timestamp;
        int64_t secs = duration / 1000;
        int64_t msecs = duration - secs * 1000;
        DLOG("[%" PRId64 "] key[%d] up (was down for %" PRId64 ".%" PRId64 "sec)\n", now, code,
             secs, msecs);
    }

    return 0;
}

static void update_input_state(struct charger *charger, struct input_event *ev)
{
	if (ev->type != EV_KEY)
		return;
	DLOG("update_input_state\n");
	set_key_callback(charger, ev->code, ev->value);
}

static void set_next_key_check(struct charger *charger, struct key_state *key, int64_t timeout)
{
	int64_t then = key->timestamp + timeout;
	DLOG("set_next_key_check, %lld\n", charger->next_key_check);
	if (charger->next_key_check == -1 || then < charger->next_key_check)
		charger->next_key_check = then;
}

static void process_key(struct charger *charger, int code, int64_t now)
{
	struct key_state *key = &charger->keys[code];
	int64_t reboot_timeout = key->timestamp + POWER_ON_KEY_TIME;
	int64_t display_timeout = key->timestamp + (POWER_ON_KEY_TIME / 4);

	DLOG("timestamp=%lld, now=%lld, reboot_timeout=%lld, display_timeout=%lld\n",
	       key->timestamp, now, reboot_timeout, display_timeout);
	if (code == KEY_POWER) {
		if (key->down) {
			if (now >= reboot_timeout) {
				reset_animation(charger->batt_anim);
				clear_screen();
				gr_flip();
				DLOG("[%" PRId64 "] rebooting\n", now);
				reboot(RB_AUTOBOOT);
			} else if (now >= display_timeout) {
				kick_animation(charger->batt_anim);
				DLOG("[%" PRId64 "] batt_anim\n", now);
				request_suspend(false);
				set_next_key_check(charger, key, (POWER_ON_KEY_TIME * 3 / 4));
			} else {
				/* if the key is pressed but timeout hasn't expired,
				 * make sure we wake up at the right-ish time to check
				 */
				DLOG("power_key pressed but timeout hasn't expired\n");
				//request_suspend(false);
				set_next_key_check(charger, key, (POWER_ON_KEY_TIME / 4));
			}
		} else {
			/* if the power key got released, force screen state cycle */
			if (key->pending) {
				if (!charger->batt_anim->run) {
					DLOG("re-run animation!\n");
					kick_animation(charger->batt_anim);
					request_suspend(false);
				} else {
					reset_animation(charger->batt_anim);
					charger->next_screen_transition = -1;
					clear_screen();
					gr_flip();
					if (charger->charger_connected) {
						DLOG("enter super standby!\n");
						request_suspend(true);
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
	DLOG("handle_input_state\n");
	if (charger->next_key_check != -1 && now > charger->next_key_check)
		charger->next_key_check = -1;
}

static void handle_power_supply_state(struct charger *charger, int64_t now)
{
	DLOG("handle_power_supply_state charger->charger_connected=%d\n", charger->charger_connected);

	if (!charger->have_battery_state)
		return;
	if (!charger->charger_connected) {
		DLOG("handle_power_supply_state  request_suspend(false)\n");
		kick_animation(charger->batt_anim);
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
			kick_animation(charger->batt_anim);
		}

		charger->next_pwr_check = -1;
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

	/* do screen update last in case any of the above want to start
	 * screen transitions (animations, etc)
	 */
	update_screen_state(charger, now);
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
		charger->next_screen_transition = curr_time_ms() - 1;
		reset_animation(charger->batt_anim);
		kick_animation(charger->batt_anim);
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

	if (charger->next_screen_transition != -1)
		next_event = charger->next_screen_transition;
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

static int input_callback(charger* charger, int fd, unsigned int epevents) {
    struct input_event ev;
    int ret;
    DLOG("input_callback\n");

    ret = ev_get_input(fd, epevents, &ev);
    if (ret) return -1;
    update_input_state(charger, &ev);
    return 0;
}

static void charger_event_handler(uint32_tt /*epevents */ )
{
	int ret;

	ret = ev_wait(-1);
	if (!ret)
		ev_dispatch();
}

void healthd_mode_charger_init(struct healthd_config *config)
{
	int ret;
	struct charger *charger = &charger_state;
	int i;
	int epollfd;
	gr_surface *scale_frames;
	int scale_count;

	printf("--------------- STARTING CHARGER MODE ---------------\n");
	printf("\n");
	/* input event init */
	ret = ev_init(std::bind(&input_callback, charger, std::placeholders::_1, std::placeholders::_2));
	if (!ret) {
		/* input event add into epollfd */
		epollfd = ev_get_epollfd();
		healthd_register_event(epollfd, charger_event_handler);
	}

	int scale_fps;
	/* battery_fail png init */
	ret = res_create_display_surface("charger/battery_fail", &charger->surf_unknown);
	if (ret < 0) {
		TLOGE("Cannot load battery_fail image\n");
		charger->surf_unknown = NULL;
	}
	/* battery animation property */
	charger->batt_anim = &battery_animation;

	/* battery_scale png init */
	ret = res_create_multi_display_surface("charger/battery_scale", &scale_count, &scale_fps, &scale_frames);
	if (ret < 0) {
		TLOGE("Cannot load battery_scale image\n");
		charger->batt_anim->num_frames = 0;
		charger->batt_anim->num_cycles = 1;
	} else if (scale_count != charger->batt_anim->num_frames) {
		TLOGE("battery_scale image has unexpected frame count (%d, expected %d)\n",
		      scale_count, charger->batt_anim->num_frames);
		charger->batt_anim->num_frames = 0;
		charger->batt_anim->num_cycles = 1;
	} else {
		/* scale frames setting */
		for (i = 0; i < charger->batt_anim->num_frames; i++) {
			charger->batt_anim->frames[i].surface = scale_frames[i];
		}
	}

	/* key event init */
	ev_sync_key_state(
		std::bind(&set_key_callback, charger, std::placeholders::_1, std::placeholders::_2));

	charger->next_screen_transition = -1;
	charger->next_key_check = 1000;
	charger->next_pwr_check = -1;
	healthd_config = config;
}
