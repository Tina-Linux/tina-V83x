#ifndef _RECORDER_INT_H_
#define _RECORDER_INT_H_

#include "Trecorder.h"

#define CHECK_NULL_POINTER(e)                                            \
		do {														\
			if (!(e))												\
			{														\
				printf("check (%s) failed.", #e);		   \
				return -1;											\
			}														\
		} while (0)


#define PARTH_A  "/mnt/SDCARD/DCIMA"
#define PARTH_B  "/mnt/SDCARD/DCIMB"
#define SENSOR_NUM 2

#define FILE_NAME_PREFIX  "AW_"
typedef struct {
	unsigned int width;
	unsigned int height;
}R_SIZE;

/*
   0: 画中画开，前摄像头主显
   1: 画中画开，后摄像头主显
   2:  画中画关，显示前摄像头
   3: 画中画关, 显示后摄像头
*/
typedef enum tag_PREVIEW_MODE_E{
	PREVIEW_HOST,
	PREVIEW_PIP,
	PREVIEW_
}__preview_mode_e;

typedef enum tag_RECORD_STATE{
	RECORD_UNINIT,
    RECORD_STOP,
	RECORD_START,
	RECORD_HALT,
}__record_state_e;

typedef enum tag_CAMERA_QUALITY{
    CAMERA_QUALITY_100,
    CAMERA_QUALITY_200,
    CAMERA_QUALITY_300,
    CAMERA_QUALITY_500,
    CAMERA_QUALITY_800,
}__camera_quality_e;

typedef enum tag_RECORD_QUALITY{
    RECORD_QUALITY_640_480,
    RECORD_QUALITY_1280_720,
    RECORD_QUALITY_1920_1080,
}__record_quality_e;

typedef enum tag_CYCLE_REC_TIME_E{
    CYCLE_REC_TIME_1_MIM,
    CYCLE_REC_TIME_2_MIM,
    CYCLE_REC_TIME_3_MIM,
    CYCLE_REC_TIME_5_MIM,
    CYCLE_REC_TIME_
}__cycle_rec_time_e;

typedef enum __RECORD_VID_WIN_RATIO_MODE
{
    RECORD_VID_WIN_BESTSHOW = 0x00,        /* 以图片本身的比例缩放至满窗口显示，图片不变形 */
    RECORD_VID_WIN_ORIGINAL,               /* 以图片原始大小在窗口内显示，不能溢出窗口     */
    RECORD_VID_WIN_FULLSCN,                /* 以窗口的比例缩放图片至满窗口显示，可能会变形 */
    RECORD_VID_WIN_CUTEDGE,                /* 裁边模式，在srcFrame区域再裁掉上下黑边，裁边后，以bestshow模式显示         */
    RECORD_VID_WIN_NOTCARE,                /* 不关心图片显示比例，以当前设置的比例         */
    RECORD_VID_WIN_ORIG_CUTEDGE_FULLSCN,    /* 以图片本身的比例缩放至满窗口显示，图片不变,图片超出部分裁剪掉     */
    RECORD_VID_WIN_UNKNOWN
}record_vid_win_ratio_mode_t;

typedef struct tag_DV_CORE{
	//内部赋值
	TrecorderHandle*       mTrecorder[SENSOR_NUM];			//摄像头句柄
	__record_state_e	   record_sta[SENSOR_NUM];			// 录像的状态
	R_SIZE				   cam_size[SENSOR_NUM];			//拍照分辨率
	R_SIZE                   rec_size[SENSOR_NUM];			//录像分辨率
	unsigned int           rec_time_ms[SENSOR_NUM];			//录像最大文件时间,单位ms
	pthread_mutex_t mutex0[SENSOR_NUM];      //保护碰撞变量
	pthread_mutex_t mutex1[SENSOR_NUM];      //保护录像器状态
	//外部赋值
    TdispRect              pip_rect;					// 画中画区域
    TdispRect              show_rect;					// 主显示区域
    __camera_quality_e     cam_quality_mode[SENSOR_NUM];	//拍照质量
	__record_quality_e     rec_quality_mode[SENSOR_NUM];	//录像质量
	R_SIZE				   source_size[SENSOR_NUM];			//视频源分辨率
	unsigned int           source_frate[SENSOR_NUM];		//视频源帧率
	__cycle_rec_time_e     cycle_rec_time[SENSOR_NUM];      //循环录像的时间
	unsigned int           frame_rate[SENSOR_NUM];			//帧率
	unsigned int		   video_bps[SENSOR_NUM];			//码率
	unsigned int           mute_en[SENSOR_NUM];		    // 1 静音 0 不静音
	__preview_mode_e       pre_mode[SENSOR_NUM];
	int                    time_water_en[SENSOR_NUM];	// 时间水印开关
}__dv_core_t;

typedef struct REC_MEDIA_INFO_T{
	TdispRect              pip_rect;				// 画中画区域
    TdispRect              show_rect;					// 主显示区域
    record_vid_win_ratio_mode_t ratio_mode;					//显示模式
    __camera_quality_e     cam_quality_mode[SENSOR_NUM];	//拍照质量
	__record_quality_e     rec_quality_mode[SENSOR_NUM];	//录像质量
	R_SIZE				   source_size[SENSOR_NUM];			//视频源分辨率
	unsigned int		   source_frate[SENSOR_NUM];		//视频源帧率
	__cycle_rec_time_e     cycle_rec_time[SENSOR_NUM];      //循环录像的时间
	unsigned int           mute_en[SENSOR_NUM];			// 1 静音 0 不静音
	__preview_mode_e       pre_mode[SENSOR_NUM];
	int                    time_water_en[SENSOR_NUM];	// 时间水印开关
}rec_media_info_t;

typedef struct
{
	__cycle_rec_time_e     cycle_rec_time[SENSOR_NUM];      //循环录像的时间
	unsigned int           mute_en[SENSOR_NUM];			// 1 静音 0 不静音
	int                    time_water_en[SENSOR_NUM];	// 时间水印开关
}rec_media_part_info_t;

int recorder_init(int index);
int recorder_exit(int index);
int recorder_start_recording(int index);
int recorder_stop_recording(int index);
int recorder_take_picture(int index);
int recorder_mute_en(int index, bool onoff);
int recorder_get_rec_state(int index);
int recorder_get_cur_rectime(int index);
int create_rec_list(void);
int destroy_rec_list(void);
int recorder_init_info(rec_media_info_t *info);
int recorder_exit_info(void);
int recorder_part_info_set(rec_media_part_info_t *info);
int recorder_reserve_size(void);
int recorder_ish_deleted_file(void);
int recorder_preview_en(int index, bool onoff);
int recorder_start_preview(int index);
int recorder_set_preview_mode(int index, __preview_mode_e pre_mode);
int collide_save_file(int index);

#endif
