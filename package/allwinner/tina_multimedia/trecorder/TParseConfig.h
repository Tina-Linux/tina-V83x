#ifndef __CAMERA_CONFIG_H__
#define __CAMERA_CONFIG_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAMERA_KEY_CONFIG_PATH	"/etc/recorder.cfg"


#define kNUMBER_OF_CAMERA					"number_of_camera"

/* camera config */
#define kCAMERA_ID							"camera_id"
#define kCAMERA_TYPE						"camera_type"

#define kVIDEO_ENABLE						"video_enable"
#define kVIDEO_WIDTH						"video_width"
#define kVIDEO_HEIGHT						"video_height"
#define kVIDEO_FRAMERATE					"video_framerate"
#define kVIDEO_ROTATION						"video_rotation"
#define kVIDEO_FORMAT						"video_format"
#define kVIDEO_USE_WM						"video_use_wm"
#define kVIDEO_WM_POS_X						"video_wm_pos_x"
#define kVIDEO_WM_POS_Y						"video_wm_pos_y"
#define kVIDEO_SCALE_DOWN_ENABLE			"video_scale_down_enable"
#define kVIDEO_SUB_WIDTH					"video_sub_width"
#define kVIDEO_SUB_HEIGHT					"video_sub_height"
#define kVIDEO_BUF_NUM						"video_buf_num"

#define kAUDIO_ENABLE						"audio_enable"
#define kAUDIO_FORMAT						"audio_format"
#define kAUDIO_CHANNELS						"audio_channels"
#define kAUDIO_SAMPLERATE					"audio_samplerate"
#define kAUDIO_SAMPLEBITS					"audio_samplebits"

#define kDISPLAY_ENABLE						"display_enable"
#define kDISPLAY_RECT_X						"display_rect_x"
#define kDISPLAY_RECT_Y						"display_rect_y"
#define kDISPLAY_RECT_WIDTH					"display_rect_width"
#define kDISPLAY_RECT_HEIGHT				"display_rect_height"
#define kDISPLAY_ZORDER						"display_zorder"
#define kDISPLAY_ROTATION					"display_rotation"
#define kDISPLAY_SRC_WIDTH					"display_src_width"
#define kDISPLAY_SRC_HEIGHT				"display_src_height"

#define kENCODER_ENABLE						"encoder_enable"
#define kENCODER_VOUTPUT_ENABLE				"encoder_voutput_enable"
#define kENCODER_VOUTPUT_TYPE				"encoder_voutput_type"
#define kENCODER_VOUTPUT_WIDTH				"encoder_voutput_width"
#define kENCODER_VOUTPUT_HEIGHT				"encoder_voutput_height"
#define kENCODER_VOUTPUT_FRAMERATE			"encoder_voutput_framerate"
#define kENCODER_VOUTPUT_BITRATE			"encoder_voutput_bitrate"
#define kENCODER_SCALE_DOWN_RATIO			"encoder_scale_down_ratio"

#define kENCODER_AOUTPUT_ENABLE				"encoder_aoutput_enable"
#define kENCODER_AOUTPUT_TYPE				"encoder_aoutput_type"
#define kENCODER_AOUTPUT_CHANNELS			"encoder_aoutput_channels"
#define kENCODER_AOUTPUT_SAMPLERATE			"encoder_aoutput_samplerate"
#define kENCODER_AOUTPUT_SAMPLERBITS		"encoder_aoutput_samplerbits"
#define kENCODER_AOUTPUT_BITRATE			"encoder_aoutput_bitrate"

#define kMUXER_ENABLE						"muxer_enable"
#define kMUXER_OUTPUT_TYPE					"muxer_output_type"


int readKey(int cameraId, char *key, char *value);
int setKey(int cameraId, char *key, char *value);


#endif
