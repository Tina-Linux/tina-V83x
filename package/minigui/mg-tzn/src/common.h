#ifndef  _COMMON_LIB_H_
#define   _COMMON_LIB_H_

#include <time.h>

#define SM_DEBUG
#define SM_ERROR

#ifdef SM_DEBUG
#define sm_debug(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define sm_debug(fmt, ...) \
	do {;} while (0)
#endif

#ifdef SM_WARN
#define sm_warn(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define sm_warn(fmt, ...) \
	do {;} while (0)
#endif

#ifdef SM_ERROR
#define sm_error(fmt, ...) \
	do {fprintf(stdout, "dbg line[%-4d] %-24s ", __LINE__, __FILE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define sm_error(fmt, ...) \
	do {;} while (0)
#endif

#define	SM_TRUE 	1
#define SM_FALSE 	0

#define	SM_OK 		0
#define SM_FAIL 	-1

int play_wav_music(const char * partname);
int get_local_time(struct tm *u_time);
int set_local_time(struct tm *u_time);
int power_off(void);

#endif

