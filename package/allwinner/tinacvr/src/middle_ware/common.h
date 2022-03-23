#ifndef  _COMMON_LIB_H_
#define   _COMMON_LIB_H_

#include <time.h>

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
