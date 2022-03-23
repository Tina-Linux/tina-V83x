#include "CvrRecordWindow.h"
#include "CvrMainWindow.h"
#include "TipLabel.h"
#include <pthread.h>
#include "MessageBox.h"
#include "ProgressBar.h"

/********************************************************************************/
/*数据定义与声明															    */
/********************************************************************************/
#define RECORD_TIMER_ID1	100
#define DISK_FULL_LEVEL (200 * 1024) //预留200M
RecordWinDataType *rcd=NULL;


/********************************************************************************/
/*视频回放场景接口																*/
/********************************************************************************/
void initProgressBar(s32 ms);
void updateProgressBar(s32 ms);
s32 CreateProgressBar(HWND hWnd);
s32 DestroyProgressBar(HWND hWnd);
s32 VideoPlayInit(RecordWinDataType *rcd);
s32 VideoPlayUninit(RecordWinDataType *rcd);


/********************************************************************************/
/*回放预览场景接口																*/
/********************************************************************************/
static void deleteDialogCallback(HWND hDlg, void* data);
void deleteFileDialog(HWND mHwnd);
s32 GetPlayBackNoFileInfo(RecordWinDataType *rcd);
s32 GetPlayBackFileInfo(RecordWinDataType *rcd);
s32 UpdatePlayBackFileTypeIcon(void);
s32 PlayBackThumbInit(RecordWinDataType *rcd);
s32 PlayBackThumbUninit(RecordWinDataType *rcd);

/********************************************************************************/
/*拍照场景接口																    */
/********************************************************************************/
int TakeImage(void);
s32 PhotoDiskCheck(void);


/********************************************************************************/
/*录像场景接口																    */
/********************************************************************************/
int GetRecordTime(void);
s32 RecordDiskCheck(void);
__record_state_e GetRecordSta(void);
void RecordStart(void);
void RecordStop(void);
void RecorderMute(bool onoff);
s32 HomeDvInit(HWND hWnd, RecordWinDataType *rcd);
s32 HomeDvExit(HWND hWnd, RecordWinDataType *rcd);
s32 RecordSuspend(HWND hWnd, RecordWinDataType *rcd);
s32 RecordResume(HWND hWnd, RecordWinDataType *rcd);
s32 RecordLowSuspend(HWND hWnd, RecordWinDataType *rcd);
s32 RecordLowResume(HWND hWnd, RecordWinDataType *rcd);


/********************************************************************************/
/*定时器、线程处理接口														    */
/********************************************************************************/
bool RecordTimerID1Proc(HWND hWnd, int id, DWORD tm);
void *RecordThreadProc(void *arg);


/********************************************************************************/
/*按键处理接口																    */
/********************************************************************************/
s32 RecordKeyOkProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress);
s32 RecordKeyModeProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress);
s32 RecordKeyLeftProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress);
s32 RecordKeyRightProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress);
s32 RecordkeyProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress);


/********************************************************************************/
/*窗口相关接口																    */
/********************************************************************************/
s32 CreateRecordWidget(HWND hWnd);
int RecordWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
RecordWinDataType* GetRecordWinData(void);
HWND GetRecordWinHwnd(void);
HWND RecordWinInit(HWND hParent);


/********************************************************************************/
/*视频回放场景接口																*/
/********************************************************************************/
void initProgressBar(s32 ms)
{
	PGBTime_t PGBTime;

	PGBTime.min = ms / 1000 / 60;
	PGBTime.sec = ms / 1000 % 60;

	SendMessage(rcd->rpData.pgbHwnd, PGBM_SETTIME_RANGE, 0, (LPARAM)&PGBTime);
}

void updateProgressBar(s32 ms)
{
	PGBTime_t curDuration;

	curDuration.min = ms / 1000 / 60;
	curDuration.sec = ms / 1000 % 60;

	SendMessage(rcd->rpData.pgbHwnd, PGBM_SETCURTIME, (WPARAM)&curDuration, 0);
}

s32 CreateProgressBar(HWND hWnd)
{
	HWND retWnd;
	CvrRectType rect;
	ProgressBarData_t PGBData;

	RegisterPGBControls();

	getResRect(ID_PLAYBACK_PGB, &rect);
	PGBData.bgcWidget = getResColor(ID_PLAYBACK_PGB, COLOR_BGC);
	PGBData.fgcWidget = getResColor(ID_PLAYBACK_PGB, COLOR_FGC);
	retWnd = CreateWindowEx(CTRL_CDRPROGRESSBAR, NULL,
		WS_VISIBLE, WS_EX_NONE, ID_PLAYBACK_PGB,
		rect.x, rect.y, rect.w, rect.h,
		hWnd, (DWORD)&PGBData);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create playback progress bar failed\n");
		return -1;
	}
	rcd->rpData.pgbHwnd = retWnd;

	initProgressBar(rcd->rpData.videoTotalTime);

	return 0;
}

s32 DestroyProgressBar(HWND hWnd)
{
	SendMessage(rcd->rpData.pgbHwnd, MSG_CLOSE, 0, 0);
	UnRegisterPGBControls();
}

s32 VideoPlayInit(RecordWinDataType *rcd)
{
	s32 ret = 0;
	char path[64];
	cvr_debug("enter video play\n");

	ret = tplayer_init(0);
	if(ret != 0)
	{
		cvr_error("player init fail\n");
		return -1;
	}

	ret = tplayer_setdisplayrect(0, 0, 1280, 320);
	if(ret != 0)
	{
		cvr_error("player set display rect fail\n");
		return -1;
	}

	ret = tplayer_play_url(rcd->pbData.fileInfo.curFilePath);
	if(ret != 0)
	{
		cvr_error("player play url fail\n");
		return -1;
	}

	tplayer_play();
	tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
	tplayer_getduration(&rcd->rpData.videoTotalTime);
	rcd->rpData.bReplaying = 1;
	cvr_debug("videoCurTime=%d, videoTotalTime=%d\n", rcd->rpData.videoCurTime, rcd->rpData.videoTotalTime);

	ShowWindow(GetHbarWinHwnd(), SW_HIDE);
	SendNotifyMessage(GetHbarWinHwnd(), MSG_WIN_SUSPEND, 0, 0);
	CreateProgressBar(rcd->RecordHwnd);

	return 0;
}

s32 VideoPlayUninit(RecordWinDataType *rcd)
{
	RECT rect;
	cvr_debug("exit video play\n");

	unloadBitMap(&rcd->rpData.pauseBmp);
	GetClientRect(rcd->rpData.pauseHwnd, &rect);
	InvalidateRect(rcd->rpData.pauseHwnd, &rect, true);
	ShowWindow(rcd->rpData.pauseHwnd, SW_HIDE);

	SendNotifyMessage(GetHbarWinHwnd(), MSG_WIN_RESUME, 0, 0);
	ShowWindow(GetHbarWinHwnd(), SW_SHOW);
	DestroyProgressBar(rcd->RecordHwnd);

	tplayer_pause();
	tplayer_stop();
	tplayer_exit();
	rcd->rpData.bReplaying = 0;

	return 0;
}

/********************************************************************************/
/*回放预览场景接口																*/
/********************************************************************************/
static void deleteDialogCallback(HWND hDlg, void* data)
{
	media_del_cur_file();
	de_show();
	show_current_file();
	GetPlayBackFileInfo(rcd);

	if(rcd->pbData.fileInfo.totalItem == 0)
	{
	//rcd->pbData.fileInfo.fileType = MEDIA_F_TYPE_PICTURE;
	rcd->pbData.fileInfo.curItem = 0;
	rcd->pbData.fileInfo.totalItem = 0;
	memcpy(rcd->pbData.fileInfo.curFileName, "", sizeof(rcd->pbData.fileInfo.curFileName));
	memcpy(rcd->pbData.fileInfo.curFilePath, "", sizeof(rcd->pbData.fileInfo.curFilePath));
	}

	SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);

	UpdatePlayBackFileTypeIcon();

	CloseMessageBox();
}

void deleteFileDialog(HWND mHwnd)
{
	int retval;
	const char* ptr;
	MessageBox_t messageBoxData;

	memset(&messageBoxData, 0, sizeof(messageBoxData));

	messageBoxData.dwStyle = MB_OKCANCEL | MB_HAVE_TITLE | MB_HAVE_TEXT;
	messageBoxData.flag_end_key = 0;

	ptr = getLabel(LANG_LABEL_DELETEFILE_TITLE);
	if(ptr == NULL) {
		cvr_error("get deletefile title failed\n");
		return;
	}
	messageBoxData.title = ptr;

	ptr = getLabel(LANG_LABEL_DELETEFILE_TEXT);
	if(ptr == NULL) {
		cvr_error("get deletefile text failed\n");
		return;
	}
	messageBoxData.text = ptr;

	ptr = getLabel(LANG_LABEL_SUBMENU_OK);
	if(ptr == NULL) {
		cvr_error("get ok string failed\n");
		return;
	}
	messageBoxData.buttonStr[0] = ptr;

	ptr = getLabel(LANG_LABEL_SUBMENU_CANCEL);
	if(ptr == NULL) {
		cvr_error("get ok string failed\n");
		return;
	}
	messageBoxData.buttonStr[1] = ptr;

	messageBoxData.pLogFont = getLogFont();

	getResRect(ID_MENU_LIST_MB, &messageBoxData.rect);
	messageBoxData.fgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_FGC);
	//messageBoxData.bgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_BGC);
	messageBoxData.bgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_BGC1);
	messageBoxData.linecTitle = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_TITLE);
	messageBoxData.linecItem = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_ITEM);
	messageBoxData.confirmCallback = deleteDialogCallback;
	messageBoxData.confirmData = (void*)0;
	retval = showMessageBox(mHwnd, &messageBoxData);
}

s32 GetPlayBackNoFileInfo(RecordWinDataType *rcd)
{
	rcd->pbData.fileInfo.fileType = MEDIA_F_TYPE_VIDEO;
	rcd->pbData.fileInfo.curItem = 0;
	rcd->pbData.fileInfo.totalItem = 0;
	memcpy(rcd->pbData.fileInfo.curFileName, "", sizeof(rcd->pbData.fileInfo.curFileName));
	memcpy(rcd->pbData.fileInfo.curFilePath, "", sizeof(rcd->pbData.fileInfo.curFilePath));

	return 0;
}

s32 GetPlayBackFileInfo(RecordWinDataType *rcd)
{
	rcd->pbData.fileInfo.fileType = media_get_current_file_type();
	rcd->pbData.fileInfo.curItem = media_get_cur_index();
	rcd->pbData.fileInfo.totalItem = media_get_total_num();
	media_get_current_file_name(rcd->pbData.fileInfo.curFileName);
	media_get_current_file_path(rcd->pbData.fileInfo.curFilePath);

	return 0;
}

s32 UpdatePlayBackFileTypeIcon(void)
{
	CvrRectType rect;

	if(rcd->pbData.fileInfo.fileType == MEDIA_F_TYPE_VIDEO)
	{
		unloadBitMap(&rcd->pbData.startBmp);
		GetClientRect(rcd->pbData.startHwnd, &rect);
		InvalidateRect(rcd->pbData.startHwnd, &rect, true);

		getResBmp(ID_PLAYBACKPREVIEW_ICON, BMPTYPE_BASE, &rcd->pbData.startBmp);
		SendMessage(rcd->pbData.startHwnd, STM_SETIMAGE, (WPARAM)&rcd->pbData.startBmp, 0);
		ShowWindow(rcd->pbData.startHwnd, SW_SHOW);
	}
	else
	{
		unloadBitMap(&rcd->pbData.startBmp);
		GetClientRect(rcd->pbData.startHwnd, &rect);
		InvalidateRect(rcd->pbData.startHwnd, &rect, true);
		ShowWindow(rcd->pbData.startHwnd, SW_HIDE);
	}

	return 0;
}

s32 PlayBackThumbInit(RecordWinDataType *rcd)
{
	s32 ret;
	CvrRectType screen_rect;

	cvr_debug("enter playback preview\n");
	getResRect(ID_SCREEN,&screen_rect);
	rcd->pbData.displaySize.x = screen_rect.x;
	rcd->pbData.displaySize.y = screen_rect.y;
	rcd->pbData.displaySize.width = screen_rect.w;
	rcd->pbData.displaySize.height = screen_rect.h;

	media_play_init(rcd->pbData.displaySize, SHOW_PIC_WIN_BESTSHOW);
	ret = create_media_list();
	if(ret != 0)
	{
		cvr_error("create media list fail\n");
	}

	if(format_is_correct()!=1 || media_get_total_num()<=0)
	{
		GetPlayBackNoFileInfo(rcd);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 2, 0);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);
	}
	else
	{
		show_current_file();
		GetPlayBackFileInfo(rcd);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 2, 0);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);
		UpdatePlayBackFileTypeIcon();
	}

	return 0;
}

s32 PlayBackThumbUninit(RecordWinDataType *rcd)
{
	CvrRectType rect;

	cvr_debug("exit playback preview\n");
	if(rcd->pbData.fileInfo.totalItem > 0)
	{
		unloadBitMap(&rcd->pbData.startBmp);
		GetClientRect(rcd->pbData.startHwnd, &rect);
		InvalidateRect(rcd->pbData.startHwnd, &rect, true);
		ShowWindow(rcd->pbData.startHwnd, SW_HIDE);

		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 0, (LPARAM)&rcd->pbData.fileInfo);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 0, 0);
		de_show();
	}
	else
	{
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 0, (LPARAM)&rcd->pbData.fileInfo);
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 0, 0);
	}

	destroy_media_list();
	media_play_exit();
	return 0;
}


/********************************************************************************/
/*拍照场景接口																    */
/********************************************************************************/
s32 PhotoDiskCheck(void)
{
	s32 ret  = 0;
	s32 size = 0;

	ret = format_is_correct();
	if(ret == -1)
	{
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_NO_TFCARD, 1, 3000);
		return  -1;
	}
	else if(ret == 0 || !rcd->data.bRecordListExit)
	{
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_TFCARD_FORMAT, 1, 3000);
		return  -1;
	}

	size = get_disk_free() - (300*1024);		// 预留300MB get_disk_free以KB为单位
	if(size <= 0)
	{
		cvr_warn("tfcard is full:%d\n", size);
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_TFCARD_FULL1, 1, 3000);
		return  -1;
	}

	return 0;
}

int TakeImage(void)
{
	s32 ret = -1;

	if(PhotoDiskCheck() == -1)
	{
		return -1;
	}

	SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 3, 0);
	if(rcd->rcData.sensor_mode == FRONT_SENSOR_MODE)
	{
		ret = recorder_take_picture(0);
	}
	else if(rcd->rcData.sensor_mode == BACK_SENSOR_MODE)
	{
		ret = recorder_take_picture(1);
	}
	else if(rcd->rcData.sensor_mode == FRONT_AND_BACK_SENSOR_MODE)
	{
		ret = recorder_take_picture(0);
		ret = recorder_take_picture(1);
	}

	return ret;
}


/********************************************************************************/
/*录像场景接口																    */
/********************************************************************************/
int GetRecordTime(void)
{
	if(recorder_get_rec_state(0) == RECORD_START){
		return recorder_get_cur_rectime(0);
	}else if(recorder_get_rec_state(1) == RECORD_START){
		return recorder_get_cur_rectime(1);
	}else{
		return -1;
	}
}

s32 RecordDiskCheck(void)
{
	s32 ret  = 0;
	s32 size = 0;

	ret = format_is_correct();
	if(ret == -1)
	{
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_NO_TFCARD, 1, 3000);
		return  -1;
	}
	else if(ret == 0 || !rcd->data.bRecordListExit)
	{
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_TFCARD_FORMAT, 1, 3000);
		return  -1;
	}
	size = get_disk_free() - recorder_reserve_size();
	ret = recorder_ish_deleted_file();
	if((size <= 0) &&(ret != 1))
	{
		printf("full. size = %d ret = %d \n", size, ret);
		CloseTipLabel();
		ShowTipLabel(rcd->RecordHwnd, LABEL_TFCARD_FULL, 1, 3000);
		return  -1;
	}

	return 0;
}

__record_state_e GetRecordSta(void)
{
	__record_state_e sta = RECORD_UNINIT;

	if(rcd->rcData.sensor_mode == FRONT_SENSOR_MODE)
	{
		sta = recorder_get_rec_state(0);
	}
	else if(rcd->rcData.sensor_mode == BACK_SENSOR_MODE)
	{
		sta = recorder_get_rec_state(1);
	}
	else if(rcd->rcData.sensor_mode == FRONT_AND_BACK_SENSOR_MODE)
	{
		sta = recorder_get_rec_state(0);
	}

	return sta;
}

void RecordStart(void)
{
	s32 ret = -1;

	if(RecordDiskCheck() == -1)
	{
		return -1;
	}

	if(rcd->rcData.sensor_mode == FRONT_SENSOR_MODE)
	{
		ret = recorder_start_recording(0);
	}
	else if(rcd->rcData.sensor_mode == BACK_SENSOR_MODE)
	{
		ret = recorder_start_recording(1);
	}
	else if(rcd->rcData.sensor_mode == FRONT_AND_BACK_SENSOR_MODE)
	{
		ret = recorder_start_recording(0);
		recorder_start_recording(1);
	}

	if(ret == 0)
	{
		rcd->rcData.rec_status = 1;
	}

}

void RecordStop(void)
{
	s32 ret = -1;

	if(rcd->rcData.sensor_mode == FRONT_SENSOR_MODE)
	{
		ret = recorder_stop_recording(0);
	}
	else if(rcd->rcData.sensor_mode == BACK_SENSOR_MODE)
	{
		ret = recorder_stop_recording(1);
	}
	else if(rcd->rcData.sensor_mode == FRONT_AND_BACK_SENSOR_MODE)
	{
		ret = recorder_stop_recording(0);
		recorder_stop_recording(1);
	}

	if(ret == 0)
	{
		rcd->rcData.rec_status = 0;
	}
}

void RecorderMute(bool onoff)
{
	if(rcd->rcData.sensor_mode == FRONT_SENSOR_MODE)
	{
		recorder_mute_en(0,onoff);
	}
	else if(rcd->rcData.sensor_mode == BACK_SENSOR_MODE)
	{
		recorder_mute_en(1,onoff);
	}
	else if(rcd->rcData.sensor_mode == FRONT_AND_BACK_SENSOR_MODE)
	{
		recorder_mute_en(0,onoff);
		recorder_mute_en(1,onoff);
	}
}

s32 HomeDvInit(HWND hWnd, RecordWinDataType *rcd)
{
	s32 ret = 0;
	rec_media_info_t info;
	CvrRectType   showRect;
	CvrRectType   pipRect;

	getResRect(ID_PREVIEW_SHOW, &showRect);
	getResRect(ID_PREVIEW_PIP,  &pipRect);

	info.pip_rect.x = pipRect.x;
	info.pip_rect.y = pipRect.y;
	info.pip_rect.width  = pipRect.w;
	info.pip_rect.height = pipRect.h;

	info.show_rect.x = showRect.x;
	info.show_rect.y = showRect.y;
	info.show_rect.width  = showRect.w;
	info.show_rect.height = showRect.h;

	info.mute_en[0] = 0;
	info.source_size[0].width = 1920;
	info.source_size[0].height = 1080;
	info.pre_mode[0] = PREVIEW_HOST;
	info.cycle_rec_time[0]   = rcd->usrData.video_time;
	info.cam_quality_mode[0] = CAMERA_QUALITY_800;
	info.rec_quality_mode[0] = RECORD_QUALITY_1920_1080;
	info.source_frate[0] = 30;
	info.time_water_en[0] = rcd->usrData.time_water_en;

	info.mute_en[1] = 0;
	info.source_size[1].width = 1280;
	info.source_size[1].height = 720;
	info.pre_mode[1] = PREVIEW_PIP;
	info.cycle_rec_time[1] =  rcd->usrData.video_time;
	info.cam_quality_mode[1] = CAMERA_QUALITY_800;
	info.rec_quality_mode[1] = RECORD_QUALITY_1280_720;
	info.source_frate[1] = 25;
	info.time_water_en[1] = rcd->usrData.time_water_en;

	if(format_is_correct() == 1)
	{
		if(!rcd->data.bRecordListExit)
		{
			cvr_debug("create rec list\n");
			create_rec_list();
			rcd->data.bRecordListExit = CVR_TRUE;
		}
	}
	else
	{
		cvr_warn("the tf card is not correct\n");
	}

	recorder_init_info(&info);
	recorder_init(0);
    recorder_init(1);

	recorder_start_preview(0);
	recorder_start_preview(1);

	// debug
	rcd->rcData.preview_mode = HOST_FRONT_PP_BACK;
	recorder_preview_en(0,0);
	recorder_preview_en(1,0);
	recorder_set_preview_mode(0,PREVIEW_HOST);
	recorder_set_preview_mode(1,PREVIEW_PIP);
	recorder_preview_en(0,1);
	recorder_preview_en(1,1);

	RecorderMute(rcd->rcData.voice_en);
	setResBoolValue(ID_MENU_LIST_SILENTMODE, rcd->rcData.voice_en);

	rcd->rcData.preview_mode  = HOST_FRONT_PP_BACK;
	rcd->rcData.sensor_mode   = FRONT_AND_BACK_SENSOR_MODE;
	rcd->rcData.rec_status    = 0;

	/*
	ret = pthread_create(&rcd->rcdThreadID, NULL, RecordThreadProc, (void *)rcd);
	if (ret == -1)
	{
		cvr_debug("create thread fail\n");
		return -1;
	}
	cvr_debug("rcd->rcdThreadID=%d\n", rcd->rcdThreadID);
	*/

	return 0;
}

s32 HomeDvExit(HWND hWnd, RecordWinDataType *rcd)
{
	recorder_exit(0);
	recorder_exit(1);
	recorder_exit_info();

	if(format_is_correct() == 1)
	{
		if(rcd->data.bRecordListExit)
		{
			cvr_warn("destroy rec list\n");
			destroy_rec_list();
			rcd->data.bRecordListExit = CVR_FALSE;
		}
	}

	/*
	rcd->bRcdThreadRun = CVR_FALSE;
	pthread_join(rcd->rcdThreadID, NULL);
	rcd->rcdThreadID = 0;
	*/

	return 0;
}


s32 RecordLowSuspend(HWND hWnd, RecordWinDataType *rcd)
{
	cvr_debug("record low suspend\n");

	if(rcd->rcData.preview_mode == HOST_FRONT)
	{
		recorder_preview_en(0,0);
	}
	else if(rcd->rcData.preview_mode == HOST_BACK)
	{
		recorder_preview_en(1,0);
	}
	else if(rcd->rcData.preview_mode == HOST_FRONT_PP_BACK)
	{
		recorder_preview_en(0,0);
		recorder_preview_en(1,0);
	}
	else if(rcd->rcData.preview_mode == HOST_BACK_PP_FRONT)
	{
		recorder_preview_en(0,0);
		recorder_preview_en(1,0);
	}

	if(format_is_correct() == 1)
	{
		if(rcd->data.bRecordListExit)
		{
			cvr_debug("destroy rec list\n");
			destroy_rec_list();
			rcd->data.bRecordListExit = CVR_FALSE;
		}
	}

	return 0;
}

s32 RecordLowResume(HWND hWnd, RecordWinDataType *rcd)
{
	s32 ret = 0;
	rec_media_part_info_t info;

	cvr_debug("record low resume\n");
	GetUserData(&rcd->usrData);

	if(format_is_correct() == 1)
	{
		if(!rcd->data.bRecordListExit)
		{
			cvr_debug("create rec list\n");
			create_rec_list();
			rcd->data.bRecordListExit = CVR_TRUE;
		}
	}
	else
	{
		cvr_warn("the tf card is not correct\n");
	}

	if(rcd->usrData.voice_en)
	{
		rcd->rcData.voice_en = 1;
	}
	else
	{
		rcd->rcData.voice_en = 0;
	}

	info.mute_en[0] = rcd->rcData.voice_en;
	info.cycle_rec_time[0]   = rcd->usrData.video_time;
	info.time_water_en[0] = rcd->usrData.time_water_en;
	info.mute_en[1] = rcd->rcData.voice_en;
	info.cycle_rec_time[1]   = rcd->usrData.video_time;
	info.time_water_en[1] = rcd->usrData.time_water_en;
	ret = recorder_part_info_set(&info);
	if(ret != 0)
	{
		cvr_error("record part info set fail\n");

		return -1;
	}

	RecorderMute(rcd->rcData.voice_en);
	setResBoolValue(ID_MENU_LIST_SILENTMODE, rcd->rcData.voice_en);

	// 不是录像预览和拍照预览需要重新设置一下起点，否则回放场景拔插usb, 在到录像预览
	// 拔插usb会出错
	if(rcd->data.curScene != RECORD && rcd->data.curScene != IMAGE)
	{
		rcd->data.oldScene = RECORD;
		rcd->data.curScene = RECORD;
	}
	if(rcd->data.curScene == RECORD)
	{
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 0, 0);
	}
	else if(rcd->data.curScene == IMAGE)
	{
		SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 1, 0);
	}

	// 设置碰撞时gsensor灵敏度
	if(rcd->usrData.gsensor_level == 0)
	{
		g_sensor_threshold_set(0);		// 高
	}
	else if(rcd->usrData.gsensor_level == 1)
	{
		g_sensor_threshold_set(1);		// 中
	}
	else if(rcd->usrData.gsensor_level == 2)
	{
		g_sensor_threshold_set(2);		// 低
	}

	// 设置音量大小
	if(rcd->usrData.voice_volume == 0)
	{
		volume_set_volume(30);		// 高
	}
	else if(rcd->usrData.voice_volume == 1)
	{
		volume_set_volume(24);		// 中
	}
	else if(rcd->usrData.voice_volume == 2)
	{
		volume_set_volume(6);		// 低
	}

	if(rcd->rcData.preview_mode == HOST_FRONT)
	{
		recorder_preview_en(0,1);
	}
	else if(rcd->rcData.preview_mode == HOST_BACK)
	{
		recorder_preview_en(1,1);
	}
	else if(rcd->rcData.preview_mode == HOST_FRONT_PP_BACK)
	{
		recorder_preview_en(0,1);
		recorder_preview_en(1,1);
	}
	else if(rcd->rcData.preview_mode == HOST_BACK_PP_FRONT)
	{
		recorder_preview_en(0,1);
		recorder_preview_en(1,1);
	}

	return 0;

}

s32 RecordSuspend(HWND hWnd, RecordWinDataType *rcd)
{
	s32 ret = 0;

	cvr_debug("record suspend\n");
	ret = KillTimer(hWnd, RECORD_TIMER_ID1);

	if(rcd->data.curScene==RECORD || rcd->data.curScene==IMAGE)
	{
		if(GetRecordSta()==RECORD_START)
		{
			RecordStop();
			if(rcd->rcData.rec_status == 0)
			{
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 1, 0);
			}
		}

		HomeDvExit(hWnd, rcd);
	}
	else if(rcd->data.curScene==PLAYBACK)
	{
		PlayBackThumbUninit(rcd);
		noreorder_media_list();
	}
	else if(rcd->data.curScene==REPLAY)
	{
		VideoPlayUninit(rcd);
	}

	return 0;
}

s32 RecordResume(HWND hWnd, RecordWinDataType *rcd)
{
	s32 ret = 0;
	static bool bStart = 0;
	cvr_debug("record resume\n");

	GetUserData(&rcd->usrData);

	if(rcd->usrData.voice_en)
	{
		rcd->rcData.voice_en = 1;
	}
	else
	{
		rcd->rcData.voice_en = 0;
	}

	if(!bStart)
    {
		rcd->data.oldScene = RECORD;
		rcd->data.curScene = RECORD;
    }

	// 不是录像预览和拍照预览需要重新设置一下起点，否则回放场景拔插usb, 在到录像预览
	// 拔插usb会出错
	if(rcd->data.curScene != RECORD && rcd->data.curScene != IMAGE)
	{
		rcd->data.oldScene = RECORD;
		rcd->data.curScene = RECORD;
	}
	if(rcd->data.curScene == RECORD)
	{
		SendMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 0, 0);
	}
	else if(rcd->data.curScene == IMAGE)
	{
		SendMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 1, 0);
	}

	HomeDvInit(hWnd, rcd);
	ret = SetTimerEx(hWnd, RECORD_TIMER_ID1, 10, RecordTimerID1Proc);		// 设置定时器间隔为500ms

	// 开机录像功能
	if(!bStart)
	{
		bStart = 1;

		if(rcd->usrData.pwr_record_en)
		{
			if(GetRecordSta() == RECORD_STOP)
			{
				RecordStart();
				if(rcd->rcData.rec_status == 1)
				{
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 0, 0);
				}
			}
		}
	}

	// 设置碰撞时gsensor灵敏度
	if(rcd->usrData.gsensor_level == 0)
	{
		g_sensor_threshold_set(0);		// 高
	}
	else if(rcd->usrData.gsensor_level == 1)
	{
		g_sensor_threshold_set(1);		// 中
	}
	else if(rcd->usrData.gsensor_level == 2)
	{
		g_sensor_threshold_set(2);		// 低
	}

	// 设置音量大小
	if(rcd->usrData.voice_volume == 0)
	{
		volume_set_volume(30);		// 高
	}
	else if(rcd->usrData.voice_volume == 1)
	{
		volume_set_volume(24);		// 中
	}
	else if(rcd->usrData.voice_volume == 2)
	{
		volume_set_volume(6);		// 低
	}

	return 0;
}


/********************************************************************************/
/*定时器、线程处理接口														    */
/********************************************************************************/
bool RecordTimerID1Proc(HWND hWnd, int id, DWORD tm)	// 不要返回CVR_FALSE，否则会删除定时器
{
	static s32 recordCount = 0;
	static s32 takePictureCnt = 0;
	RecordWinDataType *rcd = NULL;

	rcd = (RecordWinDataType*)GetWindowAdditionalData(hWnd);

	if(rcd->ptData.bTakePicture)
	{
		takePictureCnt = (takePictureCnt>=10) ? (10) : (takePictureCnt+1);
		if(takePictureCnt>=10)
		{
			SendMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 4, 0);
			rcd->ptData.bTakePicture = 0;
		}
	}
	else
	{
		takePictureCnt = 0;
	}

	if(rcd->data.curScene == RECORD)
	{
		if(GetRecordSta() == RECORD_START)
		{
			// 当前时间小于录像的时间，说明开始下一个视频的录制
			if(GetRecordTime() < recordCount)
			{
				if(rcd->rcData.bLock)
				{
					rcd->rcData.bLock = CVR_FALSE;
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LOCK, 0, 0);
				}
			}
			recordCount = GetRecordTime();
		}
		else
		{
			if(rcd->rcData.bLock)
			{
				rcd->rcData.bLock = CVR_FALSE;
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LOCK, 0, 0);
			}
		}

		return CVR_TRUE;
	}

	if(rcd->data.curScene == REPLAY)
	{
		if(rcd->rpData.bReplaying)
		{
			tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
			updateProgressBar(rcd->rpData.videoCurTime);
			if(rcd->rpData.videoCurTime >= rcd->rpData.videoTotalTime || tplayer_getcompletestate()==1)
			{
				rcd->data.oldScene = REPLAY;
				rcd->data.curScene = PLAYBACK;
				VideoPlayUninit(rcd);
				reorder_media_list();
				PlayBackThumbInit(rcd);
			}
		}

		return CVR_TRUE;
	}

	return CVR_TRUE;
}

void *RecordThreadProc(void *arg)
{
	RecordWinDataType *rcd = (RecordWinDataType *)arg;

	rcd->bRcdThreadRun = CVR_TRUE;
	cvr_debug("record thread begin\n");
	while (1)
	{
		if(!rcd->bRcdThreadRun)
		{
			cvr_debug("record thread end\n");
			pthread_exit((void*)1);
			sleep(1);
			cvr_error("record thread end\n");
		}

		//cvr_debug("record thread run\n");
		sleep(1);
	}

	cvr_error("record thread end fail\n");
	return (void*)0;
}


/********************************************************************************/
/*按键处理接口																    */
/********************************************************************************/
s32 RecordKeyOkProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress)
{
	CvrRectType rect;

	if(rcd->data.curScene == RECORD)
	{
		if(isLongPress == 0)
		{
			if(GetRecordSta() == RECORD_STOP)
			{
			    RecordStart();
				if(rcd->rcData.rec_status == 1)
				{
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 0, 0);
				}
			}
			else if(GetRecordSta() == RECORD_START)
			{
				RecordStop();
				if(rcd->rcData.rec_status == 0)
				{
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 1, 0);
				}
			}
		}
	}
	else if(rcd->data.curScene == IMAGE)
	{
		if(isLongPress == 0)
		{
			if(sdcard_is_mount_correct())
			{
				if(format_is_correct())
				{
					rcd->ptData.bTakePicture = 1;
					TakeImage();
				}
				else
				{
					rcd->ptData.bTakePicture = 0;
					CloseTipLabel();
					ShowTipLabel(rcd->RecordHwnd, LABEL_TFCARD_FORMAT, 1, 3000);
				}
			}
			else
			{
				rcd->ptData.bTakePicture = 0;
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 4, 0);
				CloseTipLabel();
				ShowTipLabel(rcd->RecordHwnd, LABEL_NO_TFCARD, 1, 3000);
			}
		}

	}
	else if(rcd->data.curScene == PLAYBACK)
	{
		if(isLongPress == 0)
		{
			if(rcd->pbData.fileInfo.totalItem <= 0)
			{
				CloseTipLabel();
				ShowTipLabel(rcd->RecordHwnd, LABEL_FILELIST_EMPTY, 1, 3000);
				return 0;
			}

			if(rcd->pbData.fileInfo.fileType==MEDIA_F_TYPE_VIDEO)
			{
				int ret ;
				rcd->data.oldScene = PLAYBACK;
				rcd->data.curScene = REPLAY;
				PlayBackThumbUninit(rcd);
				noreorder_media_list();
				ret = VideoPlayInit(rcd);
				if (-1 == ret)
				{
					rcd->data.oldScene = REPLAY;
					rcd->data.curScene = PLAYBACK;
					VideoPlayUninit(rcd);
					reorder_media_list();
					PlayBackThumbInit(rcd);
				}
			}
		}
	}
	else if(rcd->data.curScene == REPLAY)
	{
		if(isLongPress == 0)
		{
			if(rcd->rpData.bReplaying)
			{
				// 停止播放
				rcd->rpData.bReplaying = 0;
				tplayer_pause();

				getResBmp(ID_PLAYBACK_ICON, BMPTYPE_BASE, &rcd->rpData.pauseBmp);
				SendMessage(rcd->rpData.pauseHwnd, STM_SETIMAGE, (WPARAM)&rcd->rpData.pauseBmp, 0);
				ShowWindow(rcd->rpData.pauseHwnd, SW_SHOW);
			}
			else
			{
				rcd->rpData.bReplaying = 1;
				tplayer_play();

				unloadBitMap(&rcd->rpData.pauseBmp);
				GetClientRect(rcd->rpData.pauseHwnd, &rect);
				InvalidateRect(rcd->rpData.pauseHwnd, &rect, true);
				ShowWindow(rcd->rpData.pauseHwnd, SW_HIDE);
			}
		}
	}

	return 0;
}

s32 RecordKeyModeProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress)
{
	if(rcd->data.curScene == RECORD)
	{
		if(isLongPress == 0)
		{
			if(GetRecordSta() == RECORD_STOP)
			{
				// 切换到拍照场景
				rcd->data.oldScene = RECORD;
				rcd->data.curScene = IMAGE;
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 1, 0);
			}
			else if(GetRecordSta() == RECORD_START)
			{
				//一键拍照
				//rcd->ptData.bTakePicture = 1;
				//TakeImage();
				;
			}
		}
	}
	else if(rcd->data.curScene == IMAGE)
	{
		if(isLongPress == 0)
		{
			rcd->data.oldScene = IMAGE;
			rcd->data.curScene = PLAYBACK;
			HomeDvExit(rcd->RecordHwnd, rcd);
			PlayBackThumbInit(rcd);
		}
	}
	else if(rcd->data.curScene == PLAYBACK)
	{
		if(isLongPress == 1)
		{
			if(rcd->pbData.fileInfo.totalItem <= 0)
			{
				CloseTipLabel();
				ShowTipLabel(rcd->RecordHwnd, LABEL_FILELIST_EMPTY, 1, 3000);
				return 0;
			}
			else
			{
				deleteFileDialog(rcd->RecordHwnd);
				return 0;
			}
		}
		else
		{
			rcd->data.oldScene = PLAYBACK;
			rcd->data.curScene = RECORD;
			PlayBackThumbUninit(rcd);
			noreorder_media_list();
			HomeDvInit(rcd->RecordHwnd, rcd);
		}
	}
	else if(rcd->data.curScene == REPLAY)
	{
		if(isLongPress == 0)
		{
			rcd->data.oldScene = REPLAY;
			rcd->data.curScene = PLAYBACK;
			VideoPlayUninit(rcd);
			reorder_media_list();
			PlayBackThumbInit(rcd);
		}
	}

	return 0;
}

s32 RecordKeyLeftProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress)
{
	CvrRectType rect;
	if((rcd->data.curScene == RECORD)||(rcd->data.curScene == IMAGE))
	{
		if(isLongPress == 0)
		{
			if(rcd->rcData.preview_mode == HOST_FRONT)
			{
				rcd->rcData.preview_mode = HOST_BACK;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_PIP);
				recorder_set_preview_mode(1,PREVIEW_HOST);
				recorder_preview_en(1,1);
			}
			else if(rcd->rcData.preview_mode == HOST_BACK)
			{
				rcd->rcData.preview_mode = HOST_FRONT_PP_BACK;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_HOST);
				recorder_set_preview_mode(1,PREVIEW_PIP);
				recorder_preview_en(0,1);
				recorder_preview_en(1,1);
			}
			else if(rcd->rcData.preview_mode == HOST_FRONT_PP_BACK)
			{
				rcd->rcData.preview_mode = HOST_BACK_PP_FRONT;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_PIP);
				recorder_set_preview_mode(1,PREVIEW_HOST);
				recorder_preview_en(0,1);
				recorder_preview_en(1,1);
			}
			else if(rcd->rcData.preview_mode == HOST_BACK_PP_FRONT)
			{
				rcd->rcData.preview_mode = HOST_FRONT;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_HOST);
				recorder_set_preview_mode(1,PREVIEW_PIP);
				recorder_preview_en(0,1);
			}
		}
		else
		{
			{
				static s32 nCarBackStus = 0;

				if(nCarBackStus == 0)
				{
					nCarBackStus = 1;
				}
				else
				{
					nCarBackStus = 0;
				}

				SendNotifyMessage(rcd->RecordHwnd, MSG_CARBACK_DET, nCarBackStus, 0);
			}
		}
	}
	else if(rcd->data.curScene == PLAYBACK)
	{
		if(isLongPress == 0)
		{
			if(rcd->pbData.fileInfo.totalItem > 1)
			{
				de_show();
				show_pre_file();

				GetPlayBackFileInfo(rcd);
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);
				UpdatePlayBackFileTypeIcon();
			}
		}
	}
	else if(rcd->data.curScene == REPLAY)
	{
		if(isLongPress == 0)
		{
			tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
			cvr_debug("rcd->rpData.videoCurTime=%d\n", rcd->rpData.videoCurTime);

			rcd->rpData.videoCurTime = rcd->rpData.videoCurTime - 2000;
			if(rcd->rpData.videoCurTime > 0)
			{
				tplayer_seekto(rcd->rpData.videoCurTime);
				updateProgressBar(rcd->rpData.videoCurTime);
			}
			else
			{
				tplayer_seekto(0);
				updateProgressBar(0);
			}
		}
	}

	return 0;
}

s32 RecordKeyRightProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress)
{
	CvrRectType rect;

	// 静音开关
	if(rcd->data.curScene == RECORD)
	{
		if(isLongPress == 1)
		{
			//一键加锁
			if(GetRecordSta()==RECORD_START && !rcd->rcData.bLock)
			{
				rcd->rcData.bLock = CVR_TRUE;
				collide_save_file(0);
				collide_save_file(1);
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LOCK, 1, 0);
			}
		}
		else
		{
			if(rcd->rcData.voice_en == 1)
			{
				rcd->rcData.voice_en = 0;
			}
			else
			{
				rcd->rcData.voice_en = 1;
			}
			RecorderMute(rcd->rcData.voice_en);
			setResBoolValue(ID_MENU_LIST_SILENTMODE, rcd->rcData.voice_en);
		}

	}
	else if(rcd->data.curScene == PLAYBACK)
	{
		if(isLongPress == 0)
		{
			if(rcd->pbData.fileInfo.totalItem > 1)
			{
				de_show();
				show_next_file();

				GetPlayBackFileInfo(rcd);
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);
				UpdatePlayBackFileTypeIcon();
			}
		}
	}
	else if(rcd->data.curScene == REPLAY)
	{
		if(isLongPress == 0)
		{
			tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
			cvr_debug("rcd->rpData.videoCurTime=%d\n", rcd->rpData.videoCurTime);

			rcd->rpData.videoCurTime = rcd->rpData.videoCurTime + 2000;

			if(rcd->rpData.videoCurTime >= rcd->rpData.videoTotalTime)
			{
				tplayer_seekto(rcd->rpData.videoTotalTime-100);
				updateProgressBar(rcd->rpData.videoTotalTime-100);
			}
			else
			{
				tplayer_seekto(rcd->rpData.videoCurTime);
				updateProgressBar(rcd->rpData.videoCurTime);
			}
		}
	}

	return 0;
}

s32 RecordkeyProc(RecordWinDataType *rcd, s32 keyCode, s32 isLongPress)
{
	CvrRectType rect;
	cvr_debug("keyCode=%x, isLongPress=%d\n", keyCode, isLongPress);

	switch(keyCode)
	{
		case CVR_KEY_OK:
		{
			RecordKeyOkProc(rcd, keyCode, isLongPress);
			break;
		}

		case CVR_KEY_MODE:
		{
			if(isLongPress == 1)
			{
				if((rcd->data.curScene==RECORD) && (GetRecordSta()!=RECORD_START))
				{
					return WINDOW_MENU_ID;
				}
				else if(rcd->data.curScene == IMAGE)
				{
					return WINDOW_MENU_ID;
				}
			}

			RecordKeyModeProc(rcd, keyCode, isLongPress);
			break;
		}

		case CVR_KEY_LEFT:
		{
			RecordKeyLeftProc(rcd, keyCode, isLongPress);
			break;
		}

		case CVR_KEY_RIGHT:
		{
			RecordKeyRightProc(rcd, keyCode, isLongPress);
			break;
		}

	    case CVR_KEY_MENU:
		{
	        break;
	    }

		default:
		{
			break;
		}
	}

	return WINDOW_RECORD_ID;
}


/********************************************************************************/
/*窗口相关接口																    */
/********************************************************************************/
s32 CreateRecordWidget(HWND hWnd)
{
	HWND retWnd;
	CvrRectType rect;
	getResRect(ID_PLAYBACK_ICON,&rect);
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_PLAYBACK_ICON,
			rect.x,rect.y,rect.w,rect.h,
			hWnd, (DWORD)NULL);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create audio Hwnd failed\n");
		return -1;
	}
	rcd->rpData.pauseHwnd  = retWnd;

	getResRect(ID_PLAYBACKPREVIEW_ICON,&rect);
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_PLAYBACKPREVIEW_ICON,
			rect.x,rect.y,rect.w,rect.h,
			hWnd, (DWORD)NULL);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create audio Hwnd failed\n");
		return -1;
	}
	rcd->pbData.startHwnd  = retWnd;

	rect.x = 0;
	rect.y = 0;
	rect.w = 1280;
	rect.h = 320;
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_CARBACK,
			rect.x,rect.y,rect.w,rect.h,
			hWnd, (DWORD)NULL);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create carback Hwnd failed\n");
		return -1;
	}
	rcd->data.carBackHwnd  = retWnd;

	return 0;
}

static int RecordCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	RecordWinDataType *rcd = NULL;
	rcd = (RecordWinDataType*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
		case MSG_GSENSOR_HIT:
		{
			cvr_debug("gsensor hit\n");
			if(GetRecordSta()==RECORD_START && !rcd->rcData.bLock)
			{
				rcd->rcData.bLock = CVR_TRUE;
				collide_save_file(0);
				collide_save_file(1);
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LOCK, 1, 0);
			}
			break;
		}

		case MSG_TF_CARD_PLUGIN:
		{
			cvr_debug("record tf in....\n");
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 1, 0);

			if(rcd->data.curScene==RECORD || rcd->data.curScene==IMAGE)
			{
				if(format_is_correct() == 1)
				{
					if(!rcd->data.bRecordListExit)
					{
						cvr_debug("create rec list\n");
						create_rec_list();
						rcd->data.bRecordListExit = CVR_TRUE;
					}
					else
					{
						// 为了使链表与tf里的文件同步
						destroy_rec_list();
						create_rec_list();
						rcd->data.bRecordListExit = CVR_TRUE;
					}
				}
			}
			else if(rcd->data.curScene == PLAYBACK)
			{
				PlayBackThumbUninit(rcd);
				noreorder_media_list();
				PlayBackThumbInit(rcd);
			}

			break;
		}

		case MSG_TF_CARD_PLUGOUT:
		{
			cvr_debug("record tf out....\n");
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 0, 0);

			if(rcd->data.curScene==RECORD || rcd->data.curScene==IMAGE)
			{
				if(GetRecordSta() == RECORD_START)
				{
					RecordStop();
					if(rcd->rcData.rec_status == 0)
					{
						SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_RECINFO, 1, 0);
					}
				}
				//if(format_is_correct() == 1)
				{
					if(rcd->data.bRecordListExit)
					{
						destroy_rec_list();
						rcd->data.bRecordListExit = CVR_FALSE;
					}
				}
			}
			else if(rcd->data.curScene == PLAYBACK)
			{
				if(rcd->pbData.fileInfo.totalItem > 0)
				{
					GetPlayBackNoFileInfo(rcd);
					unloadBitMap(&rcd->pbData.startBmp);
					GetClientRect(rcd->pbData.startHwnd, &rect);
					InvalidateRect(rcd->pbData.startHwnd, &rect, true);
					ShowWindow(rcd->pbData.startHwnd, SW_HIDE);

					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_LABLE3, 1, (LPARAM)&rcd->pbData.fileInfo);
					de_show();
				}
			}
			else if(rcd->data.curScene == REPLAY)
			{
				rcd->data.oldScene = REPLAY;
				rcd->data.curScene = PLAYBACK;
				VideoPlayUninit(rcd);
				PlayBackThumbInit(rcd);
			}
			break;
		}

		case MSG_CARBACK_DET:
		{
			int nIndex;
			int err_code;

			if((rcd->data.curScene != RECORD) && (rcd->data.curScene != IMAGE))
			{
				break;
			}

			nIndex = (int)(wParam);
			if(nIndex == 1)
			{
				rcd->data.carBackStus = 1;
				rcd->rcData.preview_mode = HOST_BACK;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_PIP);
				recorder_set_preview_mode(1,PREVIEW_HOST);
				recorder_preview_en(1,1);

				unloadBitMap(&rcd->data.carBackBmp);
				GetClientRect(rcd->data.carBackHwnd, &rect);
				InvalidateRect(rcd->data.carBackHwnd, &rect, true);

				err_code = LoadBitmapFromFile(HDC_SCREEN, &rcd->data.carBackBmp, "/usr/res/others/back_car.png");
				if(err_code != ERR_BMP_OK) {
					cvr_debug("load %s bitmap failed\n", "/usr/res/others/back_car.png");
				}
				SendMessage(rcd->data.carBackHwnd, STM_SETIMAGE, (WPARAM)&rcd->data.carBackBmp, 0);
				ShowWindow(rcd->data.carBackHwnd, SW_SHOW);
			}
			else
			{
				rcd->data.carBackStus = 0;
				rcd->rcData.preview_mode = HOST_FRONT_PP_BACK;
				recorder_preview_en(0,0);
				recorder_preview_en(1,0);
				recorder_set_preview_mode(0,PREVIEW_HOST);
				recorder_set_preview_mode(1,PREVIEW_PIP);
				recorder_preview_en(0,1);
				recorder_preview_en(1,1);

				unloadBitMap(&rcd->data.carBackBmp);
				GetClientRect(rcd->data.carBackHwnd, &rect);
				InvalidateRect(rcd->data.carBackHwnd, &rect, true);
				ShowWindow(rcd->data.carBackHwnd, SW_HIDE);
			}

			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}

int RecordWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	RecordWinDataType *rcd = NULL;
	rcd = (RecordWinDataType*)GetWindowAdditionalData(hWnd);

	//cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_FONTCHANGED:
		{
			HWND hChild;
			PLOGFONT pLogFont;

			pLogFont = GetWindowFont(hWnd);
			if(!pLogFont)
			{
				cvr_error("invalid logFont\n");
				break;
			}

			hChild = GetNextChild(hWnd, 0);
			while( hChild && (hChild != HWND_INVALID))
			{
				SetWindowFont(hChild, pLogFont);
				hChild = GetNextChild(hWnd, hChild);
			}
		}
			break;

		case MSG_CREATE:
			CreateRecordWidget(hWnd);
			break;

		case MSG_CVR_KEY:
			return RecordkeyProc(rcd, wParam, lParam);
			break;

		case MSG_TIMER:
			break;

		case MSG_DESTROY:
			break;

		case MSG_WIN_RESUME:
		{
			s32 nIndex;

			nIndex = (s32)(wParam);
			if(nIndex == 0)
			{
				RecordResume(hWnd, rcd);
			}
			else if(nIndex == 1)
			{
				RecordLowResume(hWnd, rcd);
			}
		}
			break;

		case MSG_WIN_SUSPEND:
		{
			s32 nIndex;

			nIndex = (s32)(wParam);
			if(nIndex == 0)
			{
				RecordSuspend(hWnd, rcd);
			}
			else if(nIndex == 1)
			{
				RecordLowSuspend(hWnd, rcd);
			}
			else if(nIndex == 3)
			{
				cvr_debug("record suspend 3\n");
				KillTimer(hWnd, RECORD_TIMER_ID1);
				HomeDvExit(hWnd, rcd);
			}
		}
			break;

		default:
		{
			RecordCallBack(hWnd, message, wParam, lParam);
		}
			break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

RecordWinDataType* GetRecordWinData(void)
{
	if(rcd != NULL)
	{
		return rcd;
	}

	return NULL;
}

HWND GetRecordWinHwnd(void)
{
	if(rcd!=NULL && rcd->RecordHwnd!=0)
	{
		return rcd->RecordHwnd;
	}

	return 0;
}

HWND RecordWinInit(HWND hParent)
{
	HWND mHwnd;
	CvrRectType rect;
	CvrRectType scnRect;
	CvrRectType hbarRect;

	rcd = (RecordWinDataType*)malloc(sizeof(RecordWinDataType));
	if(NULL == rcd)
	{
		cvr_error("malloc record window data error\n");
		return -1;
	}
	memset((void *)rcd, 0, sizeof(RecordWinDataType));

	getResRect(ID_SCREEN,&scnRect);
	getResRect(ID_STATUSBAR,&hbarRect);

	rcd->RecordSize.x = scnRect.x;
	rcd->RecordSize.y = scnRect.y + hbarRect.h;
	rcd->RecordSize.w = scnRect.w;
	rcd->RecordSize.h = scnRect.h - hbarRect.h;

	rcd->data.bPowerOn = 0;


	rect = rcd->RecordSize;
	mHwnd = CreateWindowEx(WINDOW_RECORD, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_RECORD_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)rcd);

	if(mHwnd == HWND_INVALID)
	{
		cvr_error("create status bar failed\n");
		return;
	}
	setHwnd(WINDOW_RECORD_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);

	rcd->ParentHwnd = hParent;
	rcd->RecordHwnd = mHwnd;

	return mHwnd;
}
