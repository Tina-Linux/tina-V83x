/*
 * dtv_base.h
 *
 *  Created on: 2014-2-19
 *      Author: common
 */

#ifndef DTV_BASE_H_
#define DTV_BASE_H_
//#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "tsc_type.h"

long long getNowUs();
void hexdump(void * buf, unsigned int size);
uint32_t caculateCRC32(uint8_t *data, uint32_t len);

#define DTV_ASSERT(expr) \
	do {\
		if(!(expr)/*&& DTV_NDEBUG*/) {\
			ALOGE("%s:%d, assert \"%s\" failed", __FILE__, __LINE__, #expr); \
			abort();\
		}\
	} while(0)

#define DTV_TRESPASS() \
	do {\
		ALOGE("%s:%d, should not be here", __FILE__, __LINE__); \
		abort();\
	} while(0)

#ifdef __cplusplus
}
#endif

#endif /* DTV_BASE_H_ */
