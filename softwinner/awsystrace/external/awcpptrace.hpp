#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "awctrace.h"

/**
 * Macro for AW	ATRACE_CALL
 * Can use for function-trace
 * please refer to android source-code:
 * system/core/include/utils/Trace.h
 */
#define ATRACE_NAME(name) ScopedTrace ___tracer(ATRACE_TAG, name)
#define ATRACE_CALL() ATRACE_NAME(__FUNCTION__)

class ScopedTrace {
public:
	inline ScopedTrace(unsigned long tag, const char* name)
		: mTag(tag) {
		atrace_begin(mTag,name);
	}

	inline ~ScopedTrace() {
		atrace_end(mTag);
	}

private:
	unsigned long mTag;
};

