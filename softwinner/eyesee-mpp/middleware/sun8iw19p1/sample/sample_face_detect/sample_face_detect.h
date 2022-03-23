#ifndef __SAMPLE_FACE_DETECT__
#define __SAMPLE_FACE_DETECT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <utils/plat_log.h>

#include "mpp_sys.h"
#include "mpp_vi.h"
#include "mpp_eve.h"
#include "mpp_vo.h"
#include "mpp_venc.h"
#include "mpp_osd.h"

#define TEST_FRAME_NUM      	(30000)  // 测试帧数

#define VI_CAPTURE_WIDTH		(1280)	// 必须16对齐，否则截图花屏
#define VI_CAPTURE_HIGHT		(720)

#define EVE_CALC_WIDTH			(640)
#define EVE_CALC_HIGHT			(360)

#define VO_LCD_DISPLAY_WIDTH	(240)   // adapt to actual size of lcd screen
#define VO_LCD_DISPLAY_HIGHT	(320)

#define VO_HDMI_DISPLAY_WIDTH	(1920)
#define VO_HDMI_DISPLAY_HIGHT	(1080)

#define TRANS_VO_TO_LCD     // 输出到LCD，不定义则输出到HDMI
//#define SAVE_H264_FILE      // 保存H264文件

typedef enum MPP_STATETYPE
{
    MPP_StateIdle = 0X70000000,
    MPP_StateFilled,
    MPP_StateMax = 0X7FFFFFFF
} MPP_STATETYPE;

typedef struct _Picture_Params
{
	MPP_STATETYPE              ePicState;
	VIDEO_FRAME_INFO_S         stVideoFrame;
	pthread_mutex_t            lockPicture;
    AW_AI_EVE_EVENT_RESULT_S*  pEveResult;
}Picture_Params;

#endif
