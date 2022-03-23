#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "awtracetag.h"

/**
 * about atrace_begin and atrace_end
 * please refer to android source-code:
 * android/system/core/include/cutils/trace.h
 */

#define ATRACE_FD_UNINIT      (-1)
#define ATRACE_MESSAGE_LENGTH (128)

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern int atrace_marker_fd;
extern int aw_trace_init(void);

/**
 * Trace the beginning of a context.  name is used to identify the context.
 * This is often used to time function execution.
 */
#define ATRACE_BEGIN(name) atrace_begin(ATRACE_TAG, name)
static inline void atrace_begin(unsigned long tag, const char* name)
{
	char buf[ATRACE_MESSAGE_LENGTH];
	size_t len;

	if (ATRACE_FD_UNINIT == atrace_marker_fd)
	      aw_trace_init();

	len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "B|%d|%s", getpid(), name);
	write(atrace_marker_fd, buf, len);   
}

/**
 * Trace the end of a context.
 * This should match up (and occur after) a corresponding ATRACE_BEGIN.
 */
#define ATRACE_END() atrace_end(ATRACE_TAG)
static inline void atrace_end(unsigned long tag)
{
	if (ATRACE_FD_UNINIT == atrace_marker_fd)
	      aw_trace_init();

	char c = 'E';
	write(atrace_marker_fd, &c, 1);
}

/**
 * Traces an integer counter value.  name is used to identify the counter.
 * This can be used to track how a value changes over time.
 */
#define ATRACE_INT(name, value) atrace_int(ATRACE_TAG, name, value)
static inline void atrace_int(unsigned long tag, const char* name, int value)
{
	if (ATRACE_FD_UNINIT == atrace_marker_fd)
		aw_trace_init();

	char buf[ATRACE_MESSAGE_LENGTH];
	int len = 0;

	len = snprintf(buf, sizeof(buf), "C|%d| %s |%d", getpid(), name, value);
	write(atrace_marker_fd, buf, len);
}

#ifdef __cplusplus
}
#endif  /* __cplusplus */

