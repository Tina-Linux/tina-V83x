#ifndef __IPU_TYPES_DEFINED__
#define __IPU_TYPES_DEFINED__
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef signed char __s8;
typedef unsigned char __u8;

typedef  short __s16;
typedef unsigned short __u16;

typedef int __s32;
typedef unsigned int __u32;

typedef long long __s64;
typedef unsigned long long __u64;

#ifndef _STDINT
typedef		__s8		int8_t;
typedef		__s16		int16_t;
typedef		__s32		int32_t;
typedef		__s64		int64_t;
typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;
typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
#endif

//typedef  unsigned char     bool;
#ifndef __cplusplus
typedef unsigned char bool;
//static const bool false = 0;
//static const bool true = 1;
#endif

#ifdef __ARM__
#ifndef NULL
	#define NULL 0
#endif
#define false 0
#define true 1
#endif

#endif /* !(__BIT_TYPES_DEFINED__) */

