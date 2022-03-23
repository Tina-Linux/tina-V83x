#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define APP_DEBUG
#define APP_WARN
#define APP_ERROR

// fprintf(stdout, "dbg line[%4d] %s() ", __LINE__, __FUNCTION__);
#ifdef APP_DEBUG
#define app_debug(fmt, ...) \
	do {fprintf(stdout, "dbg L[%-4d] F[%-24s] ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define app_debug(fmt, ...) \
	do {;} while (0)
#endif

#ifdef APP_WARN
#define app_warn(fmt, ...) \
	do {fprintf(stdout, "wrn L[%-4d] F[%-24s] ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define app_warn(fmt, ...) \
	do {;} while (0)
#endif

#ifdef APP_ERROR
#define app_error(fmt, ...) \
	do {fprintf(stdout, "err L[%-4d] F[%-24s] ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define app_error(fmt, ...) \
	do {;} while (0)
#endif

typedef signed char			  s8;
typedef unsigned char		  u8;
typedef short int			  s16;
typedef unsigned short		  u16;
typedef int					  s32;
typedef unsigned int		  u32;

#ifndef CVR_TRUE
#define	CVR_TRUE	1
#endif

#ifndef CVR_FALSE
#define CVR_FALSE	0
#endif

#ifndef CVR_OK
#define	CVR_OK		0
#endif

#ifndef CVR_FAIL
#define CVR_FAIL	-1
#endif

#endif
