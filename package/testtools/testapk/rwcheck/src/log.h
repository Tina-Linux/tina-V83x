#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>

#define INFO(fmt, ...) \
	do { \
		fprintf(stdout, fmt, ## __VA_ARGS__); \
	} while (0)

#define ERROR(fmt, ...) \
	do { \
		fprintf(stderr, fmt, ## __VA_ARGS__); \
	} while (0)

#define ERRNO(fmt, ...) \
    do { \
	    fprintf(stderr, "%s (%d): %s: " fmt, __func__, __LINE__, \
			    strerror(errno), ## __VA_ARGS__); \
    } while (0)

#define DEBUG_LOG 0
#if DEBUG_LOG
#define DEBUG(fmt, ...) \
	do { \
        fprintf(stderr, "\n%s (%d) ", __func__, __LINE__); \
        fprintf(stderr, fmt, ## __VA_ARGS__); \
    } while (0)
#else
#define DEBUG(fmt, ...)
#endif

#endif
