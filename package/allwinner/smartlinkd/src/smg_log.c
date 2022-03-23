/*
 *  Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
 *
 *  Author: laumy <liumingyuan@allwinnertech.com>
 *
 *  date: 2019-3-15
 *
 */
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "smg_log.h"
#include <sys/time.h>

#define CONFIG_DEBUG_FILE 1
#define CONFIG_DEBUG_SYSLOG 1

static int smg_debug_syslog = 0;

int smg_debug_level = SMG_INFO;
int smg_debug_timestap = 0;

static FILE *out_file = NULL;

struct sm_sys_time {
	long sec;
	long usec;
};

extern int get_process_state(const char *process_nae,int length);
void smg_set_debug_level(int level)
{
	smg_debug_level = level;
}
int smg_get_debug_level()
{
	return smg_debug_level;
}
static int _sys_get_time(struct sm_sys_time *t)
{
	int res;
	struct timeval tv;
	res = gettimeofday(&tv, NULL);
	t->sec = tv.tv_sec;
	t->usec = tv.tv_usec;
	return res;
}
void smg_debug_open_syslog(void)
{
	openlog("wifi_manager", LOG_PID | LOG_NDELAY, LOG_DAEMON);
	smg_debug_syslog++;
}
void smg_debug_close_syslog(void)
{
	if (smg_debug_syslog)
		closelog();
}
static int syslog_priority(int level)
{
	switch (level) {
	case SMG_MSGDUMP:
	case SMG_DEBUG:
		return LOG_DEBUG;
	case SMG_INFO:
		return LOG_NOTICE;
	case SMG_WARNING:
		return LOG_WARNING;
	case SMG_ERROR:
		return LOG_ERR;
	}
	return LOG_INFO;
}

void smg_debug_print_timestap(void)
{
#ifndef CONFIG_ANDROID_LOG
	struct sm_sys_time tv;

	if (!smg_debug_timestap)
		return;

	_sys_get_time(&tv);
#ifdef CONFIG_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%ld.%06u: ", (long) tv.sec,
			(unsigned int) tv.usec);
	} else
#endif /* CONFIG_DEBUG_FILE */
	printf("%ld.%06u: ", (long) tv.sec, (unsigned int) tv.usec);
#endif /* CONFIG_ANDROID_LOG */
}
void smg_print(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level <= smg_debug_level) {
#ifdef CONFIG_DEBUG_SYSLOG
		if (smg_debug_syslog) {
			vsyslog(syslog_priority(level), fmt, ap);
		} else {
#endif /* CONFIG_DEBUG_SYSLOG */
		smg_debug_print_timestap();
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

int smg_debug_open_file(const char *path)
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
		smg_printf(SMG_ERROR, "smg_debug_open_file: Failed to open "
			   "output file, using standard output");
		return -1;
	}
#else /* CONFIG_DEBUG_FILE */
	(void)path;
#endif /* CONFIG_DEBUG_FILE */
	return 0;
}


void smg_debug_close_file(void)
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
