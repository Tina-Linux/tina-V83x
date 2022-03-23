#ifndef __CVR_INCLUDE_H__
#define __CVR_INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CVR_DEBUG
#define CVR_WARN
#define CVR_ERROR

// fprintf(stdout, "dbg line[%4d] %s() ", __LINE__, __FUNCTION__);
#ifdef CVR_DEBUG
#define cvr_debug(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define cvr_debug(fmt, ...) \
	do {;} while (0)
#endif

#ifdef CVR_WARN
#define cvr_warn(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define cvr_warn(fmt, ...) \
	do {;} while (0)
#endif

#ifdef CVR_ERROR
#define cvr_error(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define cvr_error(fmt, ...) \
	do {;} while (0)
#endif

typedef signed char			  s8;
typedef unsigned char		  u8;
typedef short int			  s16;
typedef unsigned short		  u16;
typedef int					  s32;
typedef unsigned int		  u32;
typedef long long int			s64;
typedef unsigned long long int	u64;
typedef unsigned char		  bool;

#define	CVR_TRUE	1
#define CVR_FALSE	0

#define	CVR_OK		0
#define CVR_FAIL	-1

typedef struct
{
	s32 x;
	s32 y;
	s32 w;
	s32 h;
}CvrRectType;

#endif