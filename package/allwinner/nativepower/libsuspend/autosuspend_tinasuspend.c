/*
 * Copyright (C) 2012 The Android Open Source Project
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
#define LOG_TAG "libsuspend"
#include <tina_log.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "autosuspend_ops.h"

#define EARLYSUSPEND_SYS_POWER_STATE "/sys/power/state"

static int sPowerStatefd;
static const char *pwr_state_mem = "mem";
static const char *pwr_state_on = "on";
static pthread_t tinasuspend_thread;
static pthread_mutex_t tinasuspend_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t tinasuspend_cond = PTHREAD_COND_INITIALIZER;
static bool wait_for_tinasuspend;
static enum {
	EARLYSUSPEND_ON,
	EARLYSUSPEND_MEM,
} tinasuspend_state = EARLYSUSPEND_ON;
extern void (*wakeup_func) (bool success);

#ifndef __unused
#define __unused        __attribute__((__unused__))
#endif

int tinasuspend_set_state(unsigned int state)	//0-on, 1-mem
{
	if (state != EARLYSUSPEND_ON && state != EARLYSUSPEND_MEM)
		return -1;
	if (wait_for_tinasuspend) {
		pthread_mutex_lock(&tinasuspend_mutex);
		tinasuspend_state = state;
		pthread_cond_signal(&tinasuspend_cond);
		pthread_mutex_unlock(&tinasuspend_mutex);
	}
	return 0;
}

int tinasuspend_wait_resume(void)
{
	if (wait_for_tinasuspend) {
		pthread_mutex_lock(&tinasuspend_mutex);
		while (tinasuspend_state == EARLYSUSPEND_MEM)
			pthread_cond_wait(&tinasuspend_cond, &tinasuspend_mutex);
		pthread_mutex_unlock(&tinasuspend_mutex);
	}
	return 0;
}

static void *tinasuspend_thread_func(void __unused * arg)
{
	int ret;
	bool success;
	while (1) {

		pthread_mutex_lock(&tinasuspend_mutex);
		while (tinasuspend_state == EARLYSUSPEND_ON) {
			pthread_cond_wait(&tinasuspend_cond, &tinasuspend_mutex);
		}
		pthread_mutex_unlock(&tinasuspend_mutex);
		TLOGI("prepare standy!\n");
		/*state change callback[wake->suspend] */

		TLOGV("%s: write %s to %s\n", __func__, pwr_state_mem, SYS_POWER_STATE);
		ret = TEMP_FAILURE_RETRY(write(sPowerStatefd, pwr_state_mem, strlen(pwr_state_mem)));
		if (ret < 0) {
			success = false;
		}
		TLOGI("sleep 2s, wait for enter standby\n", ret);
		sleep(2);
		pthread_mutex_lock(&tinasuspend_mutex);
		tinasuspend_state = EARLYSUSPEND_ON;
		pthread_cond_signal(&tinasuspend_cond);
		pthread_mutex_unlock(&tinasuspend_mutex);
		ret = TEMP_FAILURE_RETRY(write(sPowerStatefd, pwr_state_on, strlen(pwr_state_on)));
		/*state change callback[suspend->wake] */
		TLOGI("resume, perform wakeup function!\n");
		void (*func) (bool success) = wakeup_func;
		if (func != NULL) {
			(*func) (success);
		}
	}
}

static int autosuspend_tinasuspend_enable(void)
{
	char buf[80];
	int ret;

	TLOGV("autosuspend_tinasuspend_enable\n");
	ret = write(sPowerStatefd, pwr_state_on, strlen(pwr_state_on));
	if (ret < 0) {
		strerror_r(errno, buf, sizeof(buf));
		TLOGE("Error writing to %s: %s\n", EARLYSUSPEND_SYS_POWER_STATE, buf);
	}
	if (wait_for_tinasuspend) {
		pthread_mutex_lock(&tinasuspend_mutex);
		while (tinasuspend_state != EARLYSUSPEND_ON) {
			pthread_cond_wait(&tinasuspend_cond, &tinasuspend_mutex);
		}
		pthread_mutex_unlock(&tinasuspend_mutex);
	}

	TLOGV("autosuspend_tinasuspend_enable done\n");

	return 0;

err:
	return ret;
}

static int autosuspend_tinasuspend_disable(void)
{
	char buf[80];
	int ret;

	TLOGV("autosuspend_tinasuspend_disable\n");

	ret = TEMP_FAILURE_RETRY(write(sPowerStatefd, pwr_state_on, strlen(pwr_state_on)));
	if (ret < 0) {
		strerror_r(errno, buf, sizeof(buf));
		TLOGE("Error writing to %s: %s\n", EARLYSUSPEND_SYS_POWER_STATE, buf);
	}

	if (wait_for_tinasuspend) {
		pthread_mutex_lock(&tinasuspend_mutex);
		while (tinasuspend_state != EARLYSUSPEND_ON) {
			pthread_cond_wait(&tinasuspend_cond, &tinasuspend_mutex);
		}
		pthread_mutex_unlock(&tinasuspend_mutex);
	}

	TLOGV("autosuspend_tinasuspend_disable done\n");

	return 0;

err:
	return ret;
}

struct autosuspend_ops autosuspend_tinasuspend_ops = {
	.enable = autosuspend_tinasuspend_enable,
	.disable = autosuspend_tinasuspend_disable,
};

struct autosuspend_ops *autosuspend_tinasuspend_init(void)
{
	char buf[80];
	int ret;

	sPowerStatefd = TEMP_FAILURE_RETRY(open(EARLYSUSPEND_SYS_POWER_STATE, O_RDWR));
	if (sPowerStatefd < 0) {
		strerror_r(errno, buf, sizeof(buf));
		TLOGW("Error opening %s: %s\n", EARLYSUSPEND_SYS_POWER_STATE, buf);
		return NULL;
	}

	ret = TEMP_FAILURE_RETRY(write(sPowerStatefd, "on", 2));
	if (ret < 0) {
		strerror_r(errno, buf, sizeof(buf));
		TLOGW("Error writing 'on' to %s: %s\n", EARLYSUSPEND_SYS_POWER_STATE, buf);
		goto err_write;
	}

	TLOGI("Selected tina suspend\n");
	ret = pthread_create(&tinasuspend_thread, NULL, tinasuspend_thread_func, NULL);
	if (ret) {
		strerror_r(errno, buf, sizeof(buf));
		TLOGE("Error creating thread: %s\n", buf);
		goto err_write;
	}

	wait_for_tinasuspend = true;

	return &autosuspend_tinasuspend_ops;

err_write:
	close(sPowerStatefd);
	return NULL;
}
