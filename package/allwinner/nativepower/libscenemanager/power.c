/*
 * Copyright (C) 2008 The Android Open Source Project
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
#include "scene_manager.h"
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <ctype.h>

#define WAKELOCK_PATH	"/sys/power/wake_lock"
#define WAKEUNLOCK_PATH	"/sys/power/wake_unlock"

int acquire_wake_lock(const char *id)
{
	int fd, ret = 0;
	ssize_t len;

	fd = open(WAKELOCK_PATH, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "fatal error opening %s\n", WAKELOCK_PATH);
		return -1;
	}

	len = strlen(id);

	if (write(fd, id, len) != len)
		ret = 0;
	close(fd);
	return ret;
}

int release_wake_lock(const char *id)
{
	ssize_t len;
	int ret = 0;
	int fd;

	fd = open(WAKEUNLOCK_PATH, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "fatal error opening %s\n", WAKELOCK_PATH);
		return -1;
	}

	len = strlen(id);
	if (write(fd, id, len) != len)
		ret = -1;
	close(fd);
	return ret;
}

int get_wake_lock_count()
{
	char buf[2048];
	char *d = " ";
	char *p;
	int count = 0;
	int fd;

	fd = open(WAKELOCK_PATH, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "fatal error opening %s\n", WAKELOCK_PATH);
		return -1;
	}

	memset(buf, 0, 1024);
	lseek(fd, 0L, SEEK_SET);
	if (read(fd, buf, 2048) < 0) {
		return -1;
	}
	p = strtok(buf, d);

	while (p) {
		if (isspace(*p))
			break;
		count++;
		p = strtok(NULL, d);
	}
	close(fd);
	return count;
}
