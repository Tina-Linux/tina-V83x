#ifndef _RECORDER_INT_H_
#define _RECORDER_INT_H_

#include "Trecorder.h"
#include "common.h"

#define CHECK_NULL_POINTER(e)                                            \
		do {														\
			if (!(e))												\
			{														\
				printf("check (%s) failed.", #e);		   \
				return -1;											\
			}														\
		} while (0)

/* #define PARTH_A  "/mnt/SDCARD/DCIMA" */
/* #define PARTH_B  "/mnt/SDCARD/DCIMB" */
#define SENSOR_NUM 2	/* The number of cameras */

#define FILE_NAME_PREFIX  "AW_"
typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} R_SIZE;

typedef enum tag_PREVIEW_MODE_E {
	/* The main camera is shown above, */
	/* the sub camera is shown below */
	PREVIEW_FRONT_UP,
	/* The main camera is shown below, */
	/* the secondary camera is shown above */
	PREVIEW_FRONT_DOWN,
	/* The main camera is shown on the left */
	/* and the secondary camera is shown on the right */
	PREVIEW_FRONT_LEFT,
	/* The main camera is shown on the right */
	/* and the secondary camera is shown on the left */
	PREVIEW_FRONT_RIGHT,
	/* The main camera shows full screen, */
	/* the sub camera is displayed in the upper right corner */
	PREVIEW_FRONT_HOST,
	/* The main camera is displayed in the upper right corner, */
	/* the secondary camera shows full screen */
	PREVIEW_FRONT_PIP,
} __preview_mode_e;

typedef enum tag_RECORD_STATE {
	RECORD_UNINIT, RECORD_STOP, RECORD_START, RECORD_HALT,
} __record_state_e;

typedef enum tag_CAMERA_QUALITY {
	CAMERA_QUALITY_100,
	CAMERA_QUALITY_200,
	CAMERA_QUALITY_300,
	CAMERA_QUALITY_500,
	CAMERA_QUALITY_800,
} __camera_quality_e;

typedef enum tag_RECORD_QUALITY {
	RECORD_QUALITY_640_480, RECORD_QUALITY_1280_720, RECORD_QUALITY_1920_1080,
} __record_quality_e;

typedef enum tag_CYCLE_REC_TIME_E {
	CYCLE_REC_TIME_1_MIM,
	CYCLE_REC_TIME_2_MIM,
	CYCLE_REC_TIME_3_MIM,
	CYCLE_REC_TIME_5_MIM,
	CYCLE_REC_TIME_
} __cycle_rec_time_e;

typedef enum __RECORD_VID_WIN_RATIO_MODE {
	RECORD_VID_WIN_BESTSHOW = 0x00,
	RECORD_VID_WIN_ORIGINAL,
	RECORD_VID_WIN_FULLSCN,
	RECORD_VID_WIN_CUTEDGE,
	RECORD_VID_WIN_NOTCARE,
	RECORD_VID_WIN_ORIG_CUTEDGE_FULLSCN,
	RECORD_VID_WIN_UNKNOWN
} record_vid_win_ratio_mode_t;

typedef struct {
	TrecorderHandle* mTrecorder[SENSOR_NUM];			/* Camera handle */
	TcameraIndex mTcameraIndex[SENSOR_NUM];				/* Camera location */
	TmicIndex mTmicIndex[SENSOR_NUM];					/* Microphone position */
	TdispIndex mTdispIndex[SENSOR_NUM];				/* Preview the display layer */
	/* Time watermark switch, 1 shows watermark, 0 does not show watermark */
	unsigned int mTcameraEnableWM[SENSOR_NUM];
	ToutputFormat mToutputFormat[SENSOR_NUM];			/* Packaging format */
	TvideoEncodeFormat mTvideoEncodeFormat[SENSOR_NUM];	/* Video encoding format */
	TaudioEncodeFormat mTaudioEncodeFormat[SENSOR_NUM];	/* Audio encoding format */
	unsigned int mTmaxRecordTimeMs[SENSOR_NUM];	/* The largest video recording time, in ms */
	unsigned int mTencoderBitRate[SENSOR_NUM];			/* Bit rate */
	unsigned int mTencodeFramerate[SENSOR_NUM];			/* Frame rate */
	R_SIZE mTvideoEncodeSize[SENSOR_NUM];				/* Video resolution */
	TcameraFormat mTcameraFormat[SENSOR_NUM];			/* Camera input format */
	R_SIZE mTcameraCaptureSize[SENSOR_NUM];	/* Video source (camera) resolution */
	TmicFormat mTmicFormat[SENSOR_NUM];					/* Audio input format */
	unsigned int mTmicSampleRate[SENSOR_NUM];			/* Audio sampling rate */
	unsigned int mTmicChannels[SENSOR_NUM];				/* Audio channel */
	unsigned int mTaudioMute[SENSOR_NUM];		 /* Sound switch, 1 mute, 0 mute */
	TRoute mTRoute[SENSOR_NUM];							/* Preview the route */
	TrotateDegree mTrotateDegree[SENSOR_NUM];		/* Preview the rotation angle */
	unsigned int mTscaleDownRatio[SENSOR_NUM];			/* Reduce the ratio */
	TdispRect mTpreviewRect[SENSOR_NUM];				/* Preview display area */
	TZorder mTZorder[SENSOR_NUM];						/* Display level */
	pthread_mutex_t mutex0[SENSOR_NUM];      	/* Protect collision variables */
	pthread_mutex_t mutex1[SENSOR_NUM];      /* Protect the video recorder status */
	__record_state_e recordSta[SENSOR_NUM];				/* Video status */
	__preview_mode_e pre_mode;							/* Display mode */
	TcaptureFormat mTcaptureFormat[SENSOR_NUM];			/* Photo format */
	R_SIZE mTcaptureSize[SENSOR_NUM];					/* Photo resolution */
} __dv_core_t;

int recorder_init(int index, int justAudio);
int recorder_exit(int index, int justAudio);
int recorder_start_recording(int index);
int recorder_stop_recording(int index);
int recorder_take_picture(int index);
int recorder_mute_en(int index, Bool onoff);
int recorder_get_rec_state(int index);
int recorder_get_cur_rectime(int index);
int create_rec_list(void);
int destroy_rec_list(void);
int recorder_init_info(R_SIZE frontCamera, R_SIZE backCamera,
		R_SIZE frontRreviewRect, R_SIZE backRreviewRect);
int recorder_exit_info(void);
int recorder_reserve_size(void);
int recorder_ish_deleted_file(void);
int recorder_preview_en(int index, Bool onoff);
int recorder_start_preview(int index);
int collide_save_file(int index);
__dv_core_t *get_dv_core_t(void);

#endif
