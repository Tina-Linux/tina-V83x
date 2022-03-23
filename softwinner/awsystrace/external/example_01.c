#include "stdio.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * about atrace_begin and atrace_end
 * please refer to android source-code:
 * android/system/core/include/cutils/trace.h
 */

#define ATRACE_MESSAGE_LENGTH (128)

int atrace_marker_fd;

/**
 *  * Trace the beginning of a context.  name is used to identify the context.
 *   * This is often used to time function execution.
 *    */
#define ATRACE_BEGIN(name) atrace_begin(ATRACE_TAG, name)
static inline void atrace_begin(unsigned long tag, const char* name)
{
	char buf[ATRACE_MESSAGE_LENGTH];
	size_t len;

	len = snprintf(buf, ATRACE_MESSAGE_LENGTH, "B|%d|%s", getpid(), name);
	write(atrace_marker_fd, buf, len);   
}

/**
 *  * Trace the end of a context.
 *   * This should match up (and occur after) a corresponding ATRACE_BEGIN.
 *    */
#define ATRACE_END() atrace_end(ATRACE_TAG)
static inline void atrace_end(unsigned long tag)
{
	char c = 'E';
	write(atrace_marker_fd, &c, 1);
}


int main(void)
{
	printf("cq test\n");

	atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
	if (-1==atrace_marker_fd) {
		printf("open atrace-marker-file Failed!\n");
		return -1;
	} else {
		printf("open atrace-marker-file Ok\n");
	}

	atrace_begin(1, "AWSYSTRACE-TEST");
	printf("AWSYSTRACE-TEST\n");
	sleep(1);
	atrace_end(1);

	return 0;
}

