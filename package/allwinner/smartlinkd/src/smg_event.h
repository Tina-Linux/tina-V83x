#ifndef __SMG_EVENT_H
#define __SMG_EVENT_H

#include <stdbool.h>

#if __cplusplus
extern "C" {
#endif


#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
   (__extension__                                                              \
     ({ long int __result;                                                     \
        do __result = (long int) (expression);                                 \
        while (__result == -1L && errno == EINTR);                             \
        __result; }))
#endif

struct sm_event {
	int fd[2];
	bool enable;
};

int sm_event_init();

void sm_event_deinit();

int sm_event_wait(int timeout_ms);

int sm_event_notice(int type);

int clear_sm_event();


#if __cplusplus
};  // extern "C"
#endif

#endif /*__WIFI_EVENT_H*/
