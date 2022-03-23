/*
* btmanager - bt_log.c
*
* Copyright (c) 2018 Allwinner Technology. All Rights Reserved.
*
* Author         laumy    liumingyuan@allwinnertech.com
* verision       0.01
* Date           2018.3.26
*
* History:
*    1. date
*    2. Author
*    3. modification
*/

#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>

#include "bt_log.h"

#define CONFIG_DEBUG_FILE 1
#define CONFIG_DEBUG_SYSLOG 1

static int btmg_debug_syslog = 0;
//#ifdef DEBUG_LOG_LEVEL
//int btmg_debug_level = DEBUG_LOG_LEVEL;
//#else
int btmg_debug_level = MSG_WARNING;
//#endif
int btmg_debug_timestap = 0;

static FILE *out_file = NULL;

static int sys_get_time(struct sys_time *t)
{
	int res;
	struct timeval tv;
	res = gettimeofday(&tv, NULL);
	t->sec = tv.tv_sec;
	t->usec = tv.tv_usec;
	return res;
}

int btmg_set_debug_level(int level)
{
	if (level >= MSG_NONE) {
		btmg_debug_level = level;
		return 0;
	} else {
		printf("Illegal log level value!\n");
		return -1;
	}
}

int btmg_get_debug_level()
{
	return btmg_debug_level;
}

void btmg_debug_open_syslog(void)
{
	openlog("bt manager", LOG_PID | LOG_NDELAY, LOG_DAEMON);
	btmg_debug_syslog++;
}

void btmg_debug_close_syslog(void)
{
	if (btmg_debug_syslog)
		closelog();
}
static int syslog_priority(int level)
{
	switch (level) {
	case MSG_MSGDUMP:
	case MSG_DEBUG:
		return LOG_DEBUG;
	case MSG_INFO:
		return LOG_NOTICE;
	case MSG_WARNING:
		return LOG_WARNING;
	case MSG_ERROR:
		return LOG_ERR;
	}
	return LOG_INFO;
}

void btmg_debug_print_timestap(void)
{
#ifndef CONFIG_ANDROID_LOG
	struct sys_time tv;

	if (!btmg_debug_timestap)
		return;

	sys_get_time(&tv);
#ifdef CONFIG_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%ld.%06u: ", (long) tv.sec,
			(unsigned int) tv.usec);
	} else
#endif /* CONFIG_DEBUG_FILE */
	printf("%ld.%06u: ", (long) tv.sec, (unsigned int) tv.usec);
#endif /* CONFIG_ANDROID_LOG */
}
void btmg_print(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level <= btmg_debug_level) {
#ifdef CONFIG_DEBUG_SYSLOG
		if (btmg_debug_syslog) {
			vsyslog(syslog_priority(level), fmt, ap);
		} else {
#endif /* CONFIG_DEBUG_SYSLOG */
		btmg_debug_print_timestap();
#ifdef CONFIG_DEBUG_FILE
		if (out_file) {
			vfprintf(out_file, fmt, ap);
//			fprintf(out_file, "\n");
		} else {
#endif /* CONFIG_DEBUG_FILE */
		vprintf(fmt, ap);
//		printf("\n");
#ifdef CONFIG_DEBUG_FILE
		}
#endif /* CONFIG_DEBUG_FILE */
#ifdef CONFIG_DEBUG_SYSLOG
		}
#endif /* CONFIG_DEBUG_SYSLOG */
	}
	va_end(ap);
}

#ifdef CONFIG_DEBUG_FILE
static char *last_path = NULL;
#endif /* CONFIG_DEBUG_FILE */

int btmg_debug_open_file(const char *path)
{
#ifdef CONFIG_DEBUG_FILE
	if (!path)
		return 0;

	if (last_path == NULL || strcmp(last_path, path) != 0) {
		/* Save our path to enable re-open */
		free(last_path);
		last_path = strdup(path);
	}

	out_file = fopen(path, "a");
	if (out_file == NULL) {
		btmg_printf(MSG_ERROR, "btmg_debug_open_file: Failed to open "
			   "output file, using standard output");
		return -1;
	}
#else /* CONFIG_DEBUG_FILE */
	(void)path;
#endif /* CONFIG_DEBUG_FILE */
	return 0;
}


void btmg_debug_close_file(void)
{
#ifdef CONFIG_DEBUG_FILE
	if (!out_file)
		return;
	fclose(out_file);
	out_file = NULL;
	free(last_path);
	last_path = NULL;
#endif /* CONFIG_DEBUG_FILE */
}
