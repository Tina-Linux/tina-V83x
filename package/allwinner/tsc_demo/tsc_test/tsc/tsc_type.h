#ifndef TSC_type_H
#define TSC_type_H

#if 0
#ifndef int8_t
	#define   int8_t (signed char);
#endif

#ifndef uint8_t
	#define  uint8_t (unsigned char) ;
#endif

#ifndef int32_t
	#define  int32_t long;
#endif

#ifndef uint32_t
	#define  uint32_t (unsigned int);
#endif

#ifndef NULL
	#define NULL 0
#endif
#else

#include <stdint.h>

#endif


#endif// TSC_type_H
