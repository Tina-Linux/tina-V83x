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
#include <stdbool.h>
#include <stdio.h>

#include <suspend/autosuspend.h>

#include "autosuspend_ops.h"

static struct autosuspend_ops *autosuspend_ops;
static bool autosuspend_enabled;
static bool autosuspend_inited;

void (*wakeup_func) (bool success) = NULL;

static int autosuspend_init(void)
{
	if (autosuspend_inited) {
		return 0;
	}
#ifdef USE_TINA_SUSPEND
	autosuspend_ops = autosuspend_tinasuspend_init();
	if (autosuspend_ops) {
		goto out;
	}
#else
	autosuspend_ops = autosuspend_earlysuspend_init();
	if (autosuspend_ops) {
		goto out;
	}
#endif
#if 0
/* Remove autosleep so userspace can manager suspend/resume and keep stats */
	autosuspend_ops = autosuspend_autosleep_init();
	if (autosuspend_ops) {
		goto out;
	}
#endif

	autosuspend_ops = autosuspend_wakeup_count_init();
	if (autosuspend_ops) {
		goto out;
	}

	if (!autosuspend_ops) {
		TLOGE("failed to initialize autosuspend\n");
		return -1;
	}

out:
	autosuspend_inited = true;

	TLOGV("autosuspend initialized\n");
	return 0;
}

int autosuspend_enable(void)
{
	int ret;

	ret = autosuspend_init();
	if (ret) {
		return ret;
	}

	TLOGV("autosuspend_enable\n");

	if (autosuspend_enabled) {
		return 0;
	}
	ret = autosuspend_ops->enable();
	if (ret) {
		return ret;
	}

	autosuspend_enabled = true;
	return 0;
}

int autosuspend_disable(void)
{
	int ret;

	ret = autosuspend_init();
	if (ret) {
		return ret;
	}

	TLOGV("autosuspend_disable\n");

	if (!autosuspend_enabled) {
		return 0;
	}

	ret = autosuspend_ops->disable();
	if (ret) {
		return ret;
	}

	autosuspend_enabled = false;
	return 0;
}

void set_wakeup_callback(void (*func) (bool success))
{
	if (wakeup_func != NULL) {
		TLOGE("Duplicate wakeup callback applied, keeping original");
		return;
	}
	wakeup_func = func;
}
