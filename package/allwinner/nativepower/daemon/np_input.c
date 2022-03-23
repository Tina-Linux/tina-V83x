/*
 * Copyright (C) 2016 The AllWinner Project
 */
#include <pthread.h>
#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "power_manager.h"

#define DEV_INPUT_KEY_PATH "/dev/input/event*"
#define MSEC_PER_SEC            (1000LL)
#define NSEC_PER_MSEC           (1000000LL)

static pthread_t thread_key_power;
static pthread_mutex_t sleep_lock = PTHREAD_MUTEX_INITIALIZER;
static int has_sleep = 0;

int set_sleep_state(unsigned int state)
{
/*
 *	steta:
 *	0-awake, 1-sleep
 */
	if (state != 0 && state != 1)
		return -1;
	pthread_mutex_lock(&sleep_lock);
	has_sleep = state;
	pthread_mutex_unlock(&sleep_lock);
	return 0;
}

static int open_input(int *fd_array, int *num)
{
	char *filename = NULL;
	glob_t globbuf;
	unsigned i;
	int success = 0;
	int fd_num = 0;

	if (!fd_array || !num)
		return -1;
	/* get all the matching event filenames */
	glob(DEV_INPUT_KEY_PATH, 0, NULL, &globbuf);

	/* for each event file */
	if (globbuf.gl_pathc > *num)
		return -1;
	for (i = 0; i < globbuf.gl_pathc; ++i) {
		filename = globbuf.gl_pathv[i];

		/* open this input layer device file */
		fd_array[fd_num] = open(filename, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (fd_array[fd_num] >= 0) {
			fd_num++;
			success = 1;
		}
	}
	*num = fd_num;
	globfree(&globbuf);
	if (!success)
		return -1;

	return 0;
}

static uint64_t curr_time_ms(void)
{
	struct timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	return tm.tv_sec * MSEC_PER_SEC + (tm.tv_nsec / NSEC_PER_MSEC);
}

/*=============================================================
 * Function: scanPowerKey
 * Descriptions:
 *   the thread to scan power
 *============================================================*/
static void *scanPowerKey(void *arg)
{
	uint64_t powerkey_down_ms;
	struct epoll_event ev;
	int epollfd, i, input_event_count = 128;
	int input_event_fd[input_event_count];
	struct epoll_event events[input_event_count];
	struct input_event key_event;
	int nevents, timeout = -1;

	epollfd = epoll_create(40);
	if (epollfd == -1) {
		printf("epoll create failed.\n");
		return NULL;
	}
	open_input(input_event_fd, &input_event_count);
	for (i = 0; i < input_event_count; i++) {
		ev.data.fd = input_event_fd[i];
		ev.events = EPOLLIN | EPOLLWAKEUP;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, input_event_fd[i], &ev)) {
			printf("epoll_ctl failed.\n");
			return NULL;
		}
	}

	while (1) {
		memset(&events, 0, sizeof(events));
		nevents = epoll_wait(epollfd, events, input_event_count, timeout);
		if (nevents == -1) {
			if (errno == EINTR)
				continue;
			printf("epoll_wait failed.\n");
			break;
		}

		for (i = 0; i < nevents; i++) {
			if (read(events[i].data.fd, &key_event, sizeof(struct input_event)) == sizeof(struct input_event)) {
				if (key_event.type == EV_KEY && key_event.code == KEY_POWER) {
					/* printf("---------KEY_POWER,value=%d---------\n", key_event.value); */
					if (key_event.value == 1) {
						if (has_sleep) {
							set_sleep_state(0);
							powerkey_down_ms = 0;
						} else
							powerkey_down_ms = curr_time_ms();
					} else if (key_event.value == 0 && powerkey_down_ms != 0) {
						uint64_t now = 0;
						now = curr_time_ms();
						if (now <= powerkey_down_ms)
							powerkey_down_ms = 0;
						else if (now - powerkey_down_ms > 2000UL) {
							system("poweroff");
						} else if (powerkey_down_ms != 0) {
							/* printf("get suspend key event!\n"); */
							if (goToSleep(key_event.time.tv_sec, 0, 0) == 0)
								set_sleep_state(1);
						}
					}
				}
			}
		}
	}
}

int np_input_init()
{
	int res = 0;

	res = pthread_create(&thread_key_power, NULL, scanPowerKey, NULL);
	if (res < 0) {
		printf("\tcreate thread of scanPowerKey failed\n");
		return -1;
	}

	res = pthread_detach(thread_key_power);
	if (res < 0) {
		printf("\tset scanPowerKey detached failed\n");
		return -1;
	}

	return res;
}
