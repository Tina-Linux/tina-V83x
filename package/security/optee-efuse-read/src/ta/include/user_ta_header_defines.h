#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#include "efuse_read_demo_ta.h"
#define TA_UUID API_DEMO_UUID

/*
 * This is important to have TA_FLAG_SINGLE_INSTANCE && !TA_FLAG_MULTI_SESSION
 * as it is used by the ytest
 */
#define TA_FLAGS		(TA_FLAG_USER_MODE | TA_FLAG_EXEC_DDR | \
				TA_FLAG_MULTI_SESSION)
#define TA_STACK_SIZE		(2 * 1024)
#define TA_DATA_SIZE		(32 * 1024)

#endif
