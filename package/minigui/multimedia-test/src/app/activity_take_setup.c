#include "resource.h"
#include "recorder_int.h"

static HWND hMainWnd = HWND_INVALID;
static int freedSta = -1;			/* -1 Forced exit, 0 click OK */
static int selectSetUpId = 0;		/* Selected tab ID */
static HWND videoStaticBox;			/* Video frame of the static box handle */
static HWND videoSingleBox[16];		/* Video radio button handle */
static HWND audioSingleBox[17];		/* Record audio radio button handle */
static HWND photoSingleBox[8];		/* Photo radio button handle */
static HWND videoTowSingleBox[6];	/* Dual shot radio button handle */

/**
 * Set camera parameters selected state
 */
static int setPhotoSingleBoxCheck() {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (dv_core->mTcaptureSize[0].width) {
	case 640:
		SendMessage(photoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 800:
		SendMessage(photoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 1280:
		SendMessage(photoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 1600:
		SendMessage(photoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 2048:
		SendMessage(photoSingleBox[4], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 2560:
		SendMessage(photoSingleBox[5], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 3264:
		SendMessage(photoSingleBox[6], BM_SETCHECK, BST_CHECKED, 0);
		break;
	default:
		SendMessage(photoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	return 0;
}

/**
 * Set camera parameters
 */
static int setPhotoInfo(int singleBoxId) {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (singleBoxId) {
	case ID_PHOTO_SIZE_640x480:
		dv_core->mTcaptureSize[0].width = 640;
		dv_core->mTcaptureSize[0].height = 480;
		break;
	case ID_PHOTO_SIZE_800x600:
		dv_core->mTcaptureSize[0].width = 800;
		dv_core->mTcaptureSize[0].height = 600;
		break;
	case ID_PHOTO_SIZE_1280x960:
		dv_core->mTcaptureSize[0].width = 1280;
		dv_core->mTcaptureSize[0].height = 960;
		break;
	case ID_PHOTO_SIZE_1600x1200:
		dv_core->mTcaptureSize[0].width = 1600;
		dv_core->mTcaptureSize[0].height = 1200;
		break;
	case ID_PHOTO_SIZE_2048x1536:
		dv_core->mTcaptureSize[0].width = 2048;
		dv_core->mTcaptureSize[0].height = 1536;
		break;
	case ID_PHOTO_SIZE_2560x1920:
		dv_core->mTcaptureSize[0].width = 2560;
		dv_core->mTcaptureSize[0].height = 1920;
		break;
	case ID_PHOTO_SIZE_3264x2448:
		dv_core->mTcaptureSize[0].width = 3264;
		dv_core->mTcaptureSize[0].height = 2448;
		break;
	case ID_PHOTO_FORMAT_JPG:
		dv_core->mTcaptureFormat[0] = T_CAPTURE_JPG;
		break;
	}
	return 0;
}

/**
 * Set recording parameters selected state
 */
static int setAudioSingleBoxCheck() {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (dv_core->mToutputFormat[0]) {
	case T_OUTPUT_AAC:
		SendMessage(audioSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case T_OUTPUT_MP3:
		SendMessage(audioSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTaudioEncodeFormat[0]) {
	case T_AUDIO_PCM:
		SendMessage(audioSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case T_AUDIO_AAC:
		SendMessage(audioSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case T_AUDIO_MP3:
		SendMessage(audioSingleBox[4], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case T_AUDIO_LPCM:
		SendMessage(audioSingleBox[5], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTmaxRecordTimeMs[0]) {
	case 60 * 1000:
		SendMessage(audioSingleBox[6], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 2 * 60 * 1000:
		SendMessage(audioSingleBox[7], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 3 * 60 * 1000:
		SendMessage(audioSingleBox[8], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 5 * 60 * 1000:
		SendMessage(audioSingleBox[9], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTencoderBitRate[0]) {
	case 1 * 1000 * 1000:
		SendMessage(audioSingleBox[10], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 3 * 1000 * 1000:
		SendMessage(audioSingleBox[11], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 6 * 1000 * 1000:
		SendMessage(audioSingleBox[12], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case 8 * 1000 * 1000:
		SendMessage(audioSingleBox[13], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTmicFormat[0]) {
	case T_MIC_PCM:
		SendMessage(audioSingleBox[14], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTmicSampleRate[0]) {
	case 8 * 1000:
		SendMessage(audioSingleBox[15], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	switch (dv_core->mTmicChannels[0]) {
	case 2:
		SendMessage(audioSingleBox[16], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	return 0;
}

/**
 * Set recording parameters
 */
static int setAudioInfo(int singleBoxId) {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (singleBoxId) {
	case ID_AUDIO_OPF_AAC:
		dv_core->mToutputFormat[0] = T_OUTPUT_AAC;
		break;
	case ID_AUDIO_OPF_MP3:
		dv_core->mToutputFormat[0] = T_OUTPUT_MP3;
		break;
	case ID_AUDIO_AEF_PCM:
		dv_core->mTaudioEncodeFormat[0] = T_AUDIO_PCM;
		break;
	case ID_AUDIO_AEF_AAC:
		dv_core->mTaudioEncodeFormat[0] = T_AUDIO_AAC;
		break;
	case ID_AUDIO_AEF_MP3:
		dv_core->mTaudioEncodeFormat[0] = T_AUDIO_MP3;
		break;
	case ID_AUDIO_AEF_LPCM:
		dv_core->mTaudioEncodeFormat[0] = T_AUDIO_LPCM;
		break;
	case ID_AUDIO_MRTMS_1:
		dv_core->mTmaxRecordTimeMs[0] = 60 * 1000;
		break;
	case ID_AUDIO_MRTMS_2:
		dv_core->mTmaxRecordTimeMs[0] = 2 * 60 * 1000;
		break;
	case ID_AUDIO_MRTMS_3:
		dv_core->mTmaxRecordTimeMs[0] = 3 * 60 * 1000;
		break;
	case ID_AUDIO_MRTMS_5:
		dv_core->mTmaxRecordTimeMs[0] = 5 * 60 * 1000;
		break;
	case ID_AUDIO_EBR_1:
		dv_core->mTencoderBitRate[0] = 1 * 1000 * 1000;
		break;
	case ID_AUDIO_EBR_3:
		dv_core->mTencoderBitRate[0] = 3 * 1000 * 1000;
		break;
	case ID_AUDIO_EBR_6:
		dv_core->mTencoderBitRate[0] = 6 * 1000 * 1000;
		break;
	case ID_AUDIO_EBR_8:
		dv_core->mTencoderBitRate[0] = 8 * 1000 * 1000;
		break;
	case ID_AUDIO_MIPF:
		dv_core->mTmicFormat[0] = T_MIC_PCM;
		break;
	case ID_AUDIO_MSR:
		dv_core->mTmicSampleRate[0] = 8 * 1000;
		break;
	case ID_AUDIO_MC:
		dv_core->mTmicChannels[0] = 2;
		break;
	}
	return 0;
}

/**
 * Set the video radio button to check the status
 */
static int setVideoSingleBoxCheck(int setUpId) {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
	switch (setUpId) {
	case ID_VIDEO_OPF:
		switch (dv_core->mToutputFormat[0]) {
		case T_OUTPUT_TS:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_OUTPUT_MOV:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_VEF:
		switch (dv_core->mTvideoEncodeFormat[0]) {
		case T_VIDEO_H264:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_VIDEO_MJPEG:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_AEF:
		switch (dv_core->mTaudioEncodeFormat[0]) {
		case T_AUDIO_PCM:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_AUDIO_AAC:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_AUDIO_MP3:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_AUDIO_LPCM:
			SendMessage(videoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_MRTMS:
		switch (dv_core->mTmaxRecordTimeMs[0]) {
		case 60 * 1000:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 2 * 60 * 1000:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 3 * 60 * 1000:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 5 * 60 * 1000:
			SendMessage(videoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_EBR:
		switch (dv_core->mTencoderBitRate[0]) {
		case 1 * 1000 * 1000:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 3 * 1000 * 1000:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 6 * 1000 * 1000:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 8 * 1000 * 1000:
			SendMessage(videoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_EFR:
		switch (dv_core->mTencodeFramerate[0]) {
		case 25:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 30:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_VES:
		switch (dv_core->mTvideoEncodeSize[0].width) {
		case 640:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 1280:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 1920:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_CCS:
		switch (dv_core->mTcameraCaptureSize[0].width) {
		case 640:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 1280:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 1920:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		default:
			SendMessage(videoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_MIPF:
		switch (dv_core->mTmicFormat[0]) {
		case T_MIC_PCM:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_MSR:
		switch (dv_core->mTmicSampleRate[0]) {
		case 8 * 1000:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_MC:
		switch (dv_core->mTmicChannels[0]) {
		case 2:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_AM:
		switch (dv_core->mTaudioMute[0]) {
		case 1:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case 0:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_PROUT:
		switch (dv_core->mTRoute[0]) {
		case T_ROUTE_ISP:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_ROUTE_VE:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_ROUTE_CAMERA:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_PROTATE:
		switch (dv_core->mTrotateDegree[0]) {
		case T_ROTATION_ANGLE_0:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_ROTATION_ANGLE_90:
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_ROTATION_ANGLE_180:
			SendMessage(videoSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
			break;
		case T_ROTATION_ANGLE_270:
			SendMessage(videoSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_VESDR:
		switch (dv_core->mTscaleDownRatio[0]) {
		case 2:
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
		break;
	case ID_VIDEO_CEWM:
		if (dv_core->mTcameraEnableWM[0] == 1) {
			SendMessage(videoSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
		} else if (dv_core->mTcameraEnableWM[0] == 0) {
			SendMessage(videoSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
		}
		break;
	case ID_VIDEO_CIPF:
		SendMessage(videoSingleBox[dv_core->mTcameraFormat[0]], BM_SETCHECK,
		BST_CHECKED, 0);
		break;
	}
	return 0;
}

/**
 * Display the video check box based on the incoming ID
 *
 * @setUpId		ID of the option button
 */
static int showVideoSingleBox(int setUpId) {
	char *staticBoxName[17] = { "OutputFormat", "VideoEncoderFormat",
			"AudioEncoderFormat", "MaxRecordTimeMs", "EncoderBitRate",
			"EncodeFrameRate", "VideoEncodeSize", "CameraCaptureSize",
			"MICInputFormat", "MICSampleRate", "MICChannels", "AudioMute",
			"PreviewRoute", "PreviewRotate", "VEScaleDownRatio",
			"CameraEnableWM", "CameraInputFormat" };
	int textCount[17] = { 2, 2, 4, 4, 4, 2, 3, 4, 1, 1, 1, 2, 3, 4, 1, 2, 16 };
	int textIndex[17] = { 0, 2, 4, 8, 12, 16, 18, 21, 25, 26, 27, 28, 30, 33,
			27, 28, 40 };
	char *textName[56] = { "TS", "MOV", "H264", "MJPEG", "PCM", "AAC", "MP3",
			"LPCM", "1 min", "2 min", "3 min", "5 min", "1 Mbps", "3 Mbps",
			"6 Mbps", "8 Mbps", "25 fps", "30 fps", "640x480", "1280x720",
			"1920x1080", "640x480", "1280x720", "1920x1080", "Customize", "PCM",
			"8000", "2", "YES", "NO", "ISP", "VE", "CAMERA", "0 deg", "90 deg",
			"180 deg", "270 deg", "2", "YES", "NO", "YUV420SP", "YVU420SP",
			"YUV420P", "YVU420P", "YUV422SP", "YVU422SP", "YUV422P", "YVU422P",
			"YUYV422", "UYVY422", "YVYU422", "VYUY422", "ARGB", "RGBA", "ABGR",
			"BGRA" };
	SetWindowCaption(videoStaticBox, staticBoxName[setUpId]);
	UpdateWindow(videoStaticBox, FALSE);
	int itemCount = textCount[setUpId];
	for (int i = 0; i < itemCount; i++) {
		SetWindowCaption(videoSingleBox[i], textName[textIndex[setUpId] + i]);
		ShowWindow(videoSingleBox[i], SW_SHOWNORMAL);
	}
	setVideoSingleBoxCheck(setUpId);
	return 0;
}

/**
 * Set recording parameters
 */
static int setVideoInfo(int singleBoxId) {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}

	switch (selectSetUpId) {
	case ID_VIDEO_OPF:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mToutputFormat[0] = T_OUTPUT_TS;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mToutputFormat[0] = T_OUTPUT_MOV;
			break;
		}
		break;
	case ID_VIDEO_VEF:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTvideoEncodeFormat[0] = T_VIDEO_H264;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTvideoEncodeFormat[0] = T_OUTPUT_MOV;
			break;
		}
		break;
	case ID_VIDEO_AEF:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTaudioEncodeFormat[0] = T_AUDIO_PCM;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTaudioEncodeFormat[0] = T_AUDIO_AAC;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTaudioEncodeFormat[0] = T_AUDIO_MP3;
			break;
		case ID_SINGLE_BOX4:
			dv_core->mTaudioEncodeFormat[0] = T_AUDIO_LPCM;
			break;
		}
		break;
	case ID_VIDEO_MRTMS:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTmaxRecordTimeMs[0] = 60 * 1000;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTmaxRecordTimeMs[0] = 2 * 60 * 1000;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTmaxRecordTimeMs[0] = 3 * 60 * 1000;
			break;
		case ID_SINGLE_BOX4:
			dv_core->mTmaxRecordTimeMs[0] = 5 * 60 * 1000;
			break;
		}
		break;
	case ID_VIDEO_EBR:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTencoderBitRate[0] = 1 * 1000 * 1000;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTencoderBitRate[0] = 3 * 1000 * 1000;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTencoderBitRate[0] = 6 * 1000 * 1000;
			break;
		case ID_SINGLE_BOX4:
			dv_core->mTencoderBitRate[0] = 8 * 1000 * 1000;
			break;
		}
		break;
	case ID_VIDEO_EFR:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTencodeFramerate[0] = 25;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTencodeFramerate[0] = 30;
			break;
		}
		break;
	case ID_VIDEO_VES:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTvideoEncodeSize[0].width = 640;
			dv_core->mTvideoEncodeSize[0].height = 480;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTvideoEncodeSize[0].width = 1280;
			dv_core->mTvideoEncodeSize[0].height = 720;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTvideoEncodeSize[0].width = 1920;
			dv_core->mTvideoEncodeSize[0].height = 1080;
			break;
		}
		break;
	case ID_VIDEO_CCS:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTcameraCaptureSize[0].width = 640;
			dv_core->mTcameraCaptureSize[0].height = 480;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTcameraCaptureSize[0].width = 1280;
			dv_core->mTcameraCaptureSize[0].height = 720;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTcameraCaptureSize[0].width = 1920;
			dv_core->mTcameraCaptureSize[0].height = 1080;
			break;
		case ID_SINGLE_BOX4: {
			smRect rect;
			getResRect(ID_FRONT_CAMRA_SIZE, &rect);
			dv_core->mTcameraCaptureSize[0].width = rect.w;
			dv_core->mTcameraCaptureSize[0].height = rect.h;
			break;
		}
		}
		break;
	case ID_VIDEO_MIPF:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTmicFormat[0] = T_MIC_PCM;
			break;
		}
		break;
	case ID_VIDEO_MSR:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTmicSampleRate[0] = 8 * 1000;
			break;
		}
		break;
	case ID_VIDEO_MC:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTmicChannels[0] = 2;
			break;
		}
		break;
	case ID_VIDEO_AM:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTaudioMute[0] = 1;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTaudioMute[0] = 0;
			break;
		}
		break;
	case ID_VIDEO_PROUT:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTRoute[0] = T_ROUTE_ISP;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTRoute[0] = T_ROUTE_VE;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTRoute[0] = T_ROUTE_CAMERA;
			break;
		}
		break;
	case ID_VIDEO_PROTATE:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTrotateDegree[0] = T_ROTATION_ANGLE_0;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTrotateDegree[0] = T_ROTATION_ANGLE_90;
			break;
		case ID_SINGLE_BOX3:
			dv_core->mTrotateDegree[0] = T_ROTATION_ANGLE_180;
			break;
		case ID_SINGLE_BOX4:
			dv_core->mTrotateDegree[0] = T_ROTATION_ANGLE_270;
			break;
		}
		break;
	case ID_VIDEO_VESDR:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTscaleDownRatio[0] = 2;
			break;
		}
		break;
	case ID_VIDEO_CEWM:
		switch (singleBoxId) {
		case ID_SINGLE_BOX1:
			dv_core->mTcameraEnableWM[0] = 1;
			break;
		case ID_SINGLE_BOX2:
			dv_core->mTcameraEnableWM[0] = 0;
			break;
		}
		break;
	case ID_VIDEO_CIPF:
		dv_core->mTcameraFormat[0] = singleBoxId - 17;
		break;
	}
	return 0;
}

/**
 * Set dual shooting parameters selected state
 */
static int setVideoTowSingleBoxCheck() {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (dv_core->pre_mode) {
	case PREVIEW_FRONT_UP:
		SendMessage(videoTowSingleBox[0], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case PREVIEW_FRONT_DOWN:
		SendMessage(videoTowSingleBox[1], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case PREVIEW_FRONT_LEFT:
		SendMessage(videoTowSingleBox[2], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case PREVIEW_FRONT_RIGHT:
		SendMessage(videoTowSingleBox[3], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case PREVIEW_FRONT_HOST:
		SendMessage(videoTowSingleBox[4], BM_SETCHECK, BST_CHECKED, 0);
		break;
	case PREVIEW_FRONT_PIP:
		SendMessage(videoTowSingleBox[5], BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	return 0;
}

/**
 * Set double shooting parameters
 */
static int setVideoTowInfo(int singleBoxId) {
	__dv_core_t *dv_core = get_dv_core_t();
	if (!dv_core) {
		sm_error("dv_core is NULL!\n");
		return -1;
	}
	switch (singleBoxId) {
	case ID_VIDEO_TOW_UP:
		dv_core->pre_mode = PREVIEW_FRONT_UP;
		dv_core->mTpreviewRect[0].x = 0;
		dv_core->mTpreviewRect[0].y = 0;
		dv_core->mTpreviewRect[0].width = g_rcScr.right;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->mTpreviewRect[1].x = 0;
		dv_core->mTpreviewRect[1].y = g_rcScr.bottom / 2;
		dv_core->mTpreviewRect[1].width = g_rcScr.right;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_MIDDLE;
		break;
	case ID_VIDEO_TOW_DOWN:
		dv_core->pre_mode = PREVIEW_FRONT_DOWN;
		dv_core->mTpreviewRect[0].x = 0;
		dv_core->mTpreviewRect[0].y = g_rcScr.bottom / 2;
		dv_core->mTpreviewRect[0].width = g_rcScr.right;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->mTpreviewRect[1].x = 0;
		dv_core->mTpreviewRect[1].y = 0;
		dv_core->mTpreviewRect[1].width = g_rcScr.right;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_MIDDLE;
		break;
	case ID_VIDEO_TOW_LEFT:
		dv_core->pre_mode = PREVIEW_FRONT_LEFT;
		dv_core->mTpreviewRect[0].x = 0;
		dv_core->mTpreviewRect[0].y = 0;
		dv_core->mTpreviewRect[0].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->mTpreviewRect[1].x = g_rcScr.right / 2;
		dv_core->mTpreviewRect[1].y = 0;
		dv_core->mTpreviewRect[1].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_MIDDLE;
		break;
	case ID_VIDEO_TOW_RIGHT:
		dv_core->pre_mode = PREVIEW_FRONT_RIGHT;
		dv_core->mTpreviewRect[0].x = g_rcScr.right / 2;
		dv_core->mTpreviewRect[0].y = 0;
		dv_core->mTpreviewRect[0].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->mTpreviewRect[1].x = 0;
		dv_core->mTpreviewRect[1].y = 0;
		dv_core->mTpreviewRect[1].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_MIDDLE;
		break;
	case ID_VIDEO_TOW_HOST:
		dv_core->pre_mode = PREVIEW_FRONT_HOST;
		dv_core->mTpreviewRect[0].x = 0;
		dv_core->mTpreviewRect[0].y = 0;
		dv_core->mTpreviewRect[0].width = g_rcScr.right;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_BOTTOM;
		dv_core->mTpreviewRect[1].x = g_rcScr.right / 2;
		dv_core->mTpreviewRect[1].y = 0;
		dv_core->mTpreviewRect[1].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_MIDDLE;
		break;
	case ID_VIDEO_TOW_PIP:
		dv_core->pre_mode = PREVIEW_FRONT_PIP;
		dv_core->mTpreviewRect[0].x = g_rcScr.right / 2;
		dv_core->mTpreviewRect[0].y = 0;
		dv_core->mTpreviewRect[0].width = g_rcScr.right / 2;
		dv_core->mTpreviewRect[0].height = g_rcScr.bottom / 2;
		dv_core->mTZorder[0] = T_PREVIEW_ZORDER_MIDDLE;
		dv_core->mTpreviewRect[1].x = 0;
		dv_core->mTpreviewRect[1].y = 0;
		dv_core->mTpreviewRect[1].width = g_rcScr.right;
		dv_core->mTpreviewRect[1].height = g_rcScr.bottom;
		dv_core->mTZorder[1] = T_PREVIEW_ZORDER_BOTTOM;
		break;
	}
	return 0;
}

static void my_photo_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == BN_CLICKED) {
		if (id >= ID_PHOTO_SIZE_640x480 && id < ID_PHOTO_END) {
			setPhotoInfo(id);
		} else if (id == ID_BUTTON_OK) {
			freedSta = 0;
			PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
		}
	}
}

static void my_audio_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == BN_CLICKED) {
		if (id >= ID_AUDIO_OPF_AAC && id < ID_AUDIO_END) {
			setAudioInfo(id);
		} else if (id == ID_BUTTON_OK) {
			freedSta = 0;
			PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
		}
	}
}

static void my_video_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == BN_CLICKED) {
		if (id >= ID_VIDEO_OPF && id <= ID_VIDEO_CIPF) {
			for (int i = 0; i < 16; i++)
				ShowWindow(videoSingleBox[i], SW_HIDE);
			showVideoSingleBox(id);
			selectSetUpId = id;
		} else if (id >= ID_SINGLE_BOX1 && id <= ID_SINGLE_BOX16) {
			setVideoInfo(id);
		} else if (id == ID_BUTTON_OK) {
			freedSta = 0;
			PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
		}
	}
}

static void my_video_tow_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == BN_CLICKED) {
		if (id >= ID_VIDEO_TOW_UP && id < ID_VIDEO_TOW_END) {
			setVideoTowInfo(id);
		} else if (id == ID_BUTTON_OK) {
			freedSta = 0;
			PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
		}
	}
}

/* Create a control to set the camera data */
static void CreatePhotoControl(HWND hwnd) {
	int i = 0;
	RECT rect;
	PLOGFONT curFont;
	HWND btnHwnd, staticHwnd;
	GetClientRect(hwnd, &rect);
	if (rect.right <= 800) {
		curFont = getLogFont(ID_FONT_TIMES_18);
	} else {
		curFont = getLogFont(ID_FONT_TIMES_28);
	}
	int width = rect.right / 2;
	int hight = rect.bottom / 10;

	staticHwnd = CreateWindowEx(CTRL_STATIC, "Size",
	WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0, 0, 0,
			rect.right / 2, rect.bottom - hight, hwnd, 0);
	SetWindowFont(staticHwnd, curFont);

	char *size[7] = { "640x480", "800x600", "1280x960", "1600x1200",
			"2048x1536", "2560x1920", "3264x2448" };

	DWORD dwStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | BS_CHECKED
			| WS_GROUP;
	for (i = 0; i < 7; i++) {
		photoSingleBox[i] = CreateWindowEx(CTRL_BUTTON, size[i], dwStyle,
		WS_EX_TRANSPARENT, ID_PHOTO_SIZE_640x480 + i, 20, hight * i + 30,
				width - 40, hight, hwnd, 0);
		SetNotificationCallback(photoSingleBox[i], my_photo_proc);
		SetWindowFont(photoSingleBox[i], curFont);

		if (i == 0)
			dwStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE;
	}

	staticHwnd = CreateWindowEx(CTRL_STATIC, "Format",
	WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0, rect.right / 2,
			0, rect.right / 2, rect.bottom - hight, hwnd, 0);
	SetWindowFont(staticHwnd, curFont);

	photoSingleBox[7] = CreateWindowEx(CTRL_BUTTON, "JPG",
	WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | WS_GROUP | BS_CHECKED,
	WS_EX_TRANSPARENT, ID_PHOTO_FORMAT_JPG, rect.right / 2 + 20, 30, width - 40,
			hight, hwnd, 0);
	SetNotificationCallback(photoSingleBox[7], my_photo_proc);
	SetWindowFont(photoSingleBox[7], curFont);

	setPhotoSingleBoxCheck();

	HWND okBtn = CreateWindowEx(CTRL_BUTTON, "OK",
	WS_CHILD | WS_VISIBLE,
	WS_EX_TRANSPARENT, ID_BUTTON_OK, 0, rect.bottom - hight, rect.right, hight,
			hwnd, 0);
	SetNotificationCallback(okBtn, my_photo_proc);
	SetWindowFont(okBtn, curFont);
}

/* Create a control to set recording data */
static void CreateAudioControl(HWND hwnd) {
	RECT rect;
	PLOGFONT curFont;
	HWND staticHwnd;
	GetClientRect(hwnd, &rect);
	if (rect.right <= 800) {
		curFont = getLogFont(ID_FONT_TIMES_18);
	} else {
		curFont = getLogFont(ID_FONT_TIMES_28);
	}
	int btnWidth = rect.right / 8;
	int btnHight = rect.bottom / 10;
	int staWidth = rect.right / 2;
	int staHight = (rect.bottom - btnHight) / 4;
	char *boxName[7] = { "OutputFormat", "AudioEncoderFormat",
			"MaxRecordTimeMs", "EncoderBitRate", "MICInputFormat",
			"MICSampleRate", "MICChannels" };
	int i = 0, iHight = 0, iWidth = 0;
	for (i = 0; i < 7; i++) {
		if (i != 0 && i % 2 == 0) {
			iHight++;
			iWidth = 0;
		}
		if (i < 6) {
			staticHwnd = CreateWindowEx(CTRL_STATIC, boxName[i],
			WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0,
					iWidth * staWidth, iHight * staHight, staWidth, staHight,
					hwnd, 0);
			iWidth++;
		} else if (i == 6) {
			staticHwnd = CreateWindowEx(CTRL_STATIC, boxName[i],
			WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0, iWidth,
					3 * staHight, rect.right, staHight, hwnd, 0);
		}
		SetWindowFont(staticHwnd, curFont);
	}

	char *btnName[17] = { "AAC", "MP3", "PCM", "AAC", "MP3", "LPCM", "1 min",
			"2 min", "3 min", "5 min", "1 Mbps", "3 Mbps", "6 Mbps", "8 Mbps",
			"PCM", "8000", "2" };
	iHight = 0;
	iWidth = 0;
	int staWidthW = 0;
	DWORD dwStyle;
	for (i = 0; i < 17; i++) {
		if (i == 0 || i == 2 || i == 6 || i == 10 || i == 14 || i == 15
				|| i == 16) {
			dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP
					| BS_CHECKED;
		} else {
			dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
		}
		audioSingleBox[i] = CreateWindowEx(CTRL_BUTTON, btnName[i], dwStyle,
		WS_EX_TRANSPARENT, ID_AUDIO_OPF_AAC + i,
				10 + staWidthW + iWidth * btnWidth, 30 + iHight, btnWidth,
				btnHight, hwnd, 0);
		SetNotificationCallback(audioSingleBox[i], my_audio_proc);
		SetWindowFont(audioSingleBox[i], curFont);

		iWidth++;
		if (i == 5 || i == 13 || i == 15) {
			iHight += staHight;
			staWidthW = 0;
			iWidth = 0;
		}
		if (i == 1 || i == 9 || i == 14) {
			iWidth = 0;
			staWidthW = staWidth;
		}

	}

	setAudioSingleBoxCheck();

	HWND okBtn = CreateWindowEx(CTRL_BUTTON, "OK",
	WS_CHILD | WS_VISIBLE,
	WS_EX_TRANSPARENT, ID_BUTTON_OK, 0, rect.bottom - btnHight, rect.right,
			btnHight, hwnd, 0);
	SetNotificationCallback(okBtn, my_audio_proc);
	SetWindowFont(okBtn, curFont);
}

/* Create a control to set recording data */
static void CreateVedioControl(HWND hwnd) {
	RECT rect;
	PLOGFONT curFont;
	HWND buttonHwnd;
	GetClientRect(hwnd, &rect);
	if (rect.right <= 800) {
		curFont = getLogFont(ID_FONT_TIMES_18);
	} else {
		curFont = getLogFont(ID_FONT_TIMES_28);
	}
	int width = rect.right / 5;
	int hight = rect.bottom / 10;
	char *boxName[17] = { "OutputFormat", "VideoEncoderFormat",
			"AudioEncoderFormat", "MaxRecordTimeMs", "EncoderBitRate",
			"EncodeFrameRate", "VideoEncodeSize", "CameraCaptureSize",
			"MICInputFormat", "MICSampleRate", "MICChannels", "AudioMute",
			"PreviewRoute", "PreviewRotate", "VEScaleDownRatio",
			"CameraEnableWM", "CameraInputFormat" };
	int i = 0, iHight = 0, iWidth = 0;
	for (i = 0; i < 17; i++) {
		if (i < 15) {
			if (i != 0 && i % 5 == 0) {
				iHight++;
				iWidth = 0;
			}
			buttonHwnd = CreateWindowEx(CTRL_BUTTON, boxName[i],
			WS_CHILD | WS_VISIBLE,
			WS_EX_TRANSPARENT, ID_VIDEO_OPF + i, iWidth * width, iHight * hight,
					width, hight, hwnd, 0);
			iWidth++;
		} else if (i < 17) {
			buttonHwnd = CreateWindowEx(CTRL_BUTTON, boxName[i],
			WS_CHILD | WS_VISIBLE,
			WS_EX_TRANSPARENT, ID_VIDEO_OPF + i, (i - 15) * (rect.right / 2),
					3 * hight, rect.right / 2, hight, hwnd, 0);
		}
		SetNotificationCallback(buttonHwnd, my_video_proc);
		SetWindowFont(buttonHwnd, curFont);
	}

	videoStaticBox = CreateWindowEx(CTRL_STATIC, boxName[0],
	WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0, 0, 4 * hight,
			rect.right, 5 * hight, hwnd, 0);
	SetWindowFont(videoStaticBox, curFont);

	iHight += 2;
	iWidth = 0;
	DWORD dwStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | BS_CHECKED;
	for (i = 0; i < 16; i++) {
		if (i != 0 && i % 5 == 0) {
			iHight++;
			iWidth = 0;
		}
		videoSingleBox[i] = CreateWindowEx(CTRL_BUTTON, "", dwStyle,
		WS_EX_TRANSPARENT, ID_SINGLE_BOX1 + i, 20 + iWidth * width,
				30 + iHight * hight, width, hight, hwnd, 0);
		SetNotificationCallback(videoSingleBox[i], my_video_proc);
		SetWindowFont(videoSingleBox[i], curFont);
		iWidth++;
		if (i == 0)
			dwStyle = WS_CHILD | BS_AUTORADIOBUTTON;
	}

	showVideoSingleBox(0);

	HWND okBtn = CreateWindowEx(CTRL_BUTTON, "OK",
	WS_CHILD | WS_VISIBLE,
	WS_EX_TRANSPARENT, ID_BUTTON_OK, 0, rect.bottom - hight, rect.right, hight,
			hwnd, 0);
	SetNotificationCallback(okBtn, my_video_proc);
	SetWindowFont(okBtn, curFont);
}

/* Create a control that sets double shot data */
static void CreateTowVideoControl(HWND hwnd) {
	int i = 0;
	RECT rect;
	PLOGFONT curFont;
	HWND staticHwnd;
	GetClientRect(hwnd, &rect);
	if (rect.right <= 800) {
		curFont = getLogFont(ID_FONT_TIMES_18);
	} else {
		curFont = getLogFont(ID_FONT_TIMES_28);
	}
	int width = rect.right / 2;
	int hight = rect.bottom / 10;

	staticHwnd = CreateWindowEx(CTRL_STATIC, "Mode",
	WS_CHILD | SS_GROUPBOX | WS_VISIBLE, WS_EX_TRANSPARENT, 0, 0, 0, rect.right,
			rect.bottom - hight, hwnd, 0);
	SetWindowFont(staticHwnd, curFont);

	char *size[6] = { "Up and down", "Down and up", "Left and right",
			"Right and left", "Front and back", "Back and front" };

	DWORD dwStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | BS_CHECKED
			| WS_GROUP;
	for (i = 0; i < 6; i++) {
		videoTowSingleBox[i] = CreateWindowEx(CTRL_BUTTON, size[i], dwStyle,
		WS_EX_TRANSPARENT, ID_VIDEO_TOW_UP + i, 20, hight * i + 30, width,
				hight, hwnd, 0);
		SetNotificationCallback(videoTowSingleBox[i], my_video_tow_proc);
		SetWindowFont(videoTowSingleBox[i], curFont);

		if (i == 0)
			dwStyle = WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE;
	}

	setVideoTowSingleBoxCheck();

	HWND okBtn = CreateWindowEx(CTRL_BUTTON, "OK",
	WS_CHILD | WS_VISIBLE,
	WS_EX_TRANSPARENT, ID_BUTTON_OK, 0, rect.bottom - hight, rect.right, hight,
			hwnd, 0);
	SetNotificationCallback(okBtn, my_video_tow_proc);
	SetWindowFont(okBtn, curFont);
}

static int ActivityTSetUpProc(HWND hWnd, int nMessage, WPARAM wParam,
		LPARAM lParam) {

	int mode = GetWindowAdditionalData(hWnd);

	switch (nMessage) {
	case MSG_CREATE: {
		freedSta = -1;
		if (mode == 0) {
			CreatePhotoControl(hWnd);
		} else if (mode == 1) {
			CreateAudioControl(hWnd);
		} else if (mode == 2) {
			CreateVedioControl(hWnd);
		} else if (mode == 3) {
			CreateTowVideoControl(hWnd);
		}
		break;
	}
	case MSG_KEYUP: {
		switch (wParam) {
		case CVR_KEY_RIGHT:
			sm_debug("CVR_KEY_RIGHT\n");
			break;
		case CVR_KEY_LEFT:
			sm_debug("CVR_KEY_LEFT\n");
			break;
		case CVR_KEY_OK:
			sm_debug("CVR_KEY_OK\n");
			PostMessage(hWnd, MSG_CLOSE, 0, 0);
			break;
		}
		break;
	}
	case MSG_DESTROY:
		if (freedSta == -1)
			recorder_exit_info();
		PostMessage(GetHosting(hWnd), MSG_TAKE_SETUP_CLOSE, mode, 0);
		DestroyAllControls(hWnd);
		hMainWnd = HWND_INVALID;
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		MainWindowThreadCleanup(hWnd);
		return 0;
	}
	return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

/**
 * Parameter setting interface
 *
 * @hosting Attached to the window
 * @mode	0 camera settings
 * 			1 recording settings
 * 			2 video settings
 * 			3 dual camera settings
 */
int ActivityTSetUp(HWND hosting, int mode) {
	if (hMainWnd != HWND_INVALID)
		return -1;
	if (mode < 0 || mode > 3)
		return -1;

	MSG Msg;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TOPMOST;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityTSetUpProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = PIXEL_lightgray;
	CreateInfo.dwAddData = mode;
	CreateInfo.hHosting = hosting;

	hMainWnd = CreateMainWindow(&CreateInfo);

	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
