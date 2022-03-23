#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "awcpptrace.hpp"

int atrace_marker_fd = ATRACE_FD_UNINIT;

#define TRACE_MARKER_PATH "/sys/kernel/debug/tracing/trace_marker"

int aw_trace_init(void)
{
	atrace_marker_fd = open(TRACE_MARKER_PATH, O_WRONLY);
	if (-1==atrace_marker_fd) {
		printf("Failed to open <%s>\n", TRACE_MARKER_PATH);
		return -1;
	} else {
		printf("Success to open <%s>\n", TRACE_MARKER_PATH);
	}

	return 0;
}
