#ifndef  _COMMON_LIB_H_
#define  _COMMON_LIB_H_

#include <time.h>

/* #define SM_DEBUG */
#define SM_WARN
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

typedef signed char s8;
typedef unsigned char u8;
typedef short int s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long int s64;
typedef unsigned long long int u64;
typedef unsigned char Bool;

typedef struct {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
} CvrRectType;

int sdcard_is_exist(void);
int sdcard_is_mount_correct(void);
int udisk_is_mount_correct(void);
int get_disk_total(int mode);
int get_disk_free(int mode);
int get_disk_bsize(int mode);
int get_mount_path(char *mountPath);
int format_is_correct(void);
int format_disk(int mode);
int play_wav_music(const char * partname);
int rear_det_init(void);
int rear_det_exit(void);
int get_local_time(struct tm *u_time);
int set_local_time(struct tm *u_time);
int back_car_det(void);
int power_off(void);
int keytone_play(void);
int keytone_init(const char * partname);
int keytone_exit(void);

#endif

