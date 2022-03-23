#include "awcpptrace.hpp"

void atrace_call_test(void)
{
	ATRACE_CALL();
	sleep(1);

	return;
}

int main(void)
{
	printf("Testing ATRACE_CALL, atrace_begin, atrace_end\n");

	if (0 != aw_trace_init()) {
		printf("Error: Failed to open trace_mark node, exit(1) here");
		exit(1);
	}

	while(1) {
		printf("Testing atrace_begin and atrace_end\n");
		ATRACE_INT("ATRACE_INIT", 1);
		atrace_begin(ATRACE_BEGIN_END_TEST, "ATRACE_BEGIN_END_TEST");
		sleep(1);
		atrace_end(1);
		ATRACE_INT("ATRACE_INIT", 2);
		sleep(1);
		ATRACE_INT("ATRACE_INIT", 3);
		printf("Testing ATRACE_CALL\n");
		atrace_call_test();
		sleep(1);
	}

	return 0;
}
