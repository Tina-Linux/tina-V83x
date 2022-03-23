#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#define TA_UUID { 0x12345678, 0x4321, 0x8765, \
	{0x9b,0x74,0xf3,0xfc,0x35,0x7c,0x7c,0x61} }

/*
 * This is important to have TA_FLAG_SINGLE_INSTANCE && !TA_FLAG_MULTI_SESSION
 * as it is used by the ytest
 */
#define TA_FLAGS		(TA_FLAG_USER_MODE | TA_FLAG_EXEC_DDR | \
				TA_FLAG_MULTI_SESSION)
#define TA_STACK_SIZE		(2 * 1024)
#define TA_DATA_SIZE		(32 * 1024)

#endif
