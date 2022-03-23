#ifndef  _COMMON_LIB_H_
#define   _COMMON_LIB_H_

#include <time.h>

#define ALLWINNERTECH_R6

/*#define SM_DEBUG*/
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

#define	SM_TRUE		1
#define SM_FALSE	0

#define	SM_OK		0
#define SM_FAIL		-1
#define  TINAL_OK     0
#define  TINAL_FAILD  -1

int sdcard_is_exist(void);
int sdcard_is_mount_correct(void);
int get_disk_total(void);
int get_disk_free(void);
int get_disk_bsize(void);
int format_is_correct(void);
int format_disk(void);
int sdcard_fs_flex(void);
int play_wav_music(const char * partname);
int rear_det_init(void);
int rear_det_exit(void);
int get_local_time(struct tm *u_time);
int set_local_time(struct tm *u_time);
int back_car_det(void);
int usb_storage_adcard_on(void);
int usb_storage_adcard_off(void);
int back_car_det(void);
int power_off(void);
int keytone_play(void);
int keytone_init(const char * partname);
int keytone_exit(void);

#endif
