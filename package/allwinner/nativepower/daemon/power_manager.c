/*
 * Copyright (C) 2016 The AllWinner Project
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "power_manager.h"
#include "scene_manager.h"
#include "suspend/autosuspend.h"

#define AWAKE_TIMEOUT_LIMIT	10

static pthread_mutex_t mLock = PTHREAD_MUTEX_INITIALIZER;
static time_t mLastUserActivityTime = -1;
static time_t mKeepAwakeTime = -1;

extern int set_sleep_state(unsigned int state);

int releaseWakeLock(const char *id)
{
	return release_wake_lock(id);
}

int acquireWakeLock(const char *id)
{
	return acquire_wake_lock(id);
}

int goToSleep(long event_time_ms, int reason, int flags)
{
/*
*		EARLYSUSPEND_ON  = 0
*		EARLYSUSPEND_MEM = 1
*/
	if (get_wake_lock_count() <= 1) {
		set_sleep_state(1);
		releaseWakeLock(NATIVE_POWER_DISPLAY_LOCK);
#ifdef USE_TINA_SUSPEND
		tinasuspend_set_state(1);
		tinasuspend_wait_resume();
#endif
		return 0;
	}

	return -1;
}

int shutdown()
{
	int pid = fork();
	int ret = -1;

	if (pid == 0) {
		execlp("/sbin/poweroff", "poweroff", NULL);
	}

	return 0;
}

int reboot()
{
	int pid = fork();
	int ret = -1;

	if (pid == 0) {
		execlp("/sbin/reboot", "reboot", NULL);
	}

	return 0;
}

int userActivity()
{
	pthread_mutex_lock(&mLock);
	mLastUserActivityTime = time(NULL);
	pthread_mutex_unlock(&mLock);

	return 0;
}

int isActivityTimeValid()
{
	time_t timenow;
	time_t awakeTimeout = -1;
	char cAwakeTimeout[32];

	memset(cAwakeTimeout, 0, sizeof(cAwakeTimeout));
	np_scene_property_get("awake_timeout", cAwakeTimeout, sizeof(cAwakeTimeout));
	awakeTimeout = atoll(cAwakeTimeout);
	if (mKeepAwakeTime != awakeTimeout && awakeTimeout > AWAKE_TIMEOUT_LIMIT) {
		pthread_mutex_lock(&mLock);
		mKeepAwakeTime = awakeTimeout;
		pthread_mutex_unlock(&mLock);
	}

	if (mKeepAwakeTime < 0) {
		return 1;
	}

	timenow = time(NULL);
	if (timenow < mLastUserActivityTime || mLastUserActivityTime <= 0) {
		mLastUserActivityTime = timenow;
	}

	if ((timenow - mLastUserActivityTime) < mKeepAwakeTime) {
		return 1;
	} else {
		mLastUserActivityTime = 0;
		return 0;
	}
}

int invalidateActivityTime()
{
	mLastUserActivityTime = -1;

	return 0;
}

int init_awake_timeout()
{
	char cAwakeTimeout[32];
	time_t awakeTimeout = -1;
	int ret = -1;

	memset(cAwakeTimeout, 0, sizeof(cAwakeTimeout));
	ret = np_scene_property_get("awake_timeout", cAwakeTimeout, sizeof(cAwakeTimeout));

	awakeTimeout = atoll(cAwakeTimeout);

	if (awakeTimeout > 0) {
		pthread_mutex_lock(&mLock);
		mKeepAwakeTime = awakeTimeout;
		pthread_mutex_unlock(&mLock);
		ret = 0;
	}

	return ret;
}

int set_awake_timeout(long timeout_s)
{
	char cAwakeTimeout[32];
	int ret = -1;

	if (timeout_s < AWAKE_TIMEOUT_LIMIT)
		return -1;
	pthread_mutex_lock(&mLock);
	mKeepAwakeTime = (time_t) timeout_s;
	pthread_mutex_unlock(&mLock);
	sprintf(cAwakeTimeout, "%ld", timeout_s);
	ret = np_scene_property_set("awake_timeout", cAwakeTimeout);
	return ret;
}
