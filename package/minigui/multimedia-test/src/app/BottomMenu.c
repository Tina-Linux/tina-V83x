#include "BottomMenu.h"
#include "player_int.h"
#include "jpegdecode.h"
#include "RightMenu.h"
#include "HeadWindow.h"
#include "button_view.h"
#include <videoOutPort.h>
#include "multimedia_test.h"
#include "seekbar.h"
extern WinDataType *g_win_data;
static SeektoBar *seektobar0;
dispOutPort *hdl = NULL;
int voice = 0;
extern RightMenuDataType *RightMenuWnd;
extern HbarWinDataType *headbar;
BottomMenuDataType *bottom = NULL;
RecordWinDataType *rcd = NULL;
char *MEDIA_PARTH_A = "/mnt/SDCARD/DCIMA/";
CvrRectType rect;
int file_index = -1;
JpegDecoder* jpegdecoder;
ImgFrame* imgFrame;
extern int next_number;

int pathFindFilename(char *path[64]) {
	char *filename = (char *) malloc(64);
	int index = -1;
	index = SendMessage(RightMenuWnd->ListBox0, LB_GETCURSEL, 0, 0L);
	SendMessage(RightMenuWnd->ListBox0, LB_GETTEXT, index, (LPARAM) filename);
	int i = 0;
	char *file_name[64];
	char *p[64];
	for (i = 0; i < RightMenuWnd->request_number; ++i) {
		file_name[i] = strrchr(path[i], '/');
		p[i] = file_name[i] + 1;
	}
	for (i = 0; i < RightMenuWnd->request_number; ++i) {
		if (0 == strcmp(p[i], filename)) {
			file_index = i;
			return 0;
		} else {

			printf("p != filename\n");
		}
	}

	return 0;
}

int InitDisplayRect(int rectx, int recty, int rectw, int recth) {
	s32 ret = 0;
	CvrRectType rect;
	rect.x = rectx;
	rect.y = recty;
	rect.w = rectw;
	rect.h = recth;
	ret = tplayer_setdisplayrect(rect.x, rect.y, rect.w, rect.h);
	if (ret != 0) {
		sm_error("player set display rect fail\n");
		return -1;
	}
	return 0;
}

int startPlayer() {
	int val = 0;
	s32 ret = 0;
	char path[64];
	tplayer_init(0);
	tplayer_setvolume(bottom->voice_number * 10);
	voice = tplayer_getvolume();
	printf("voice nymber = %d\n\n\n", voice);
	InitDisplayRect(headbar->Displayrect.x, headbar->Displayrect.y,
			headbar->Displayrect.w, headbar->Displayrect.h);
	disp_rectsz rect;
	file_index = SendMessage(RightMenuWnd->ListBox0, LB_GETCURSEL, 0, 0L);
	ret = tplayer_play_url(RightMenuWnd->filepath_list[file_index]);
	if (ret != 0) {
		sm_error("player play url fail\n");
		return -1;
	}
	seektobar0->seektobarlen = 1;
	tplayer_play();
	tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
	tplayer_getduration(&rcd->rpData.videoTotalTime);
	sm_debug("videoCurTime=%d, videoTotalTime=%d\n", rcd->rpData.videoCurTime,
			rcd->rpData.videoTotalTime);
	bottom->isPlaying = FALSE;
	bottom->tplayerOpen = TRUE;
	return 0;
}

void pausePlayer() {
	int *msec;
	tplayer_pause();
	tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
	bottom->isPlaying = TRUE;
	bottom->tplayerOpen = TRUE;
}

void stopPlayer() {
	tplayer_stop();
	bottom->isPlaying = TRUE;
	bottom->tplayerOpen = TRUE;
}

void playerseekto(int videoCurTime) {
	tplayer_seekto(videoCurTime);
	tplayer_play();
	bottom->isPlaying = FALSE;
	bottom->tplayerOpen = TRUE;
}

extern int seekto_lengthX;
static void seektobar_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		playerseekto(
				seekto_lengthX * rcd->rpData.videoTotalTime
						/ bottom->startMenuSize.w);
	}
}

static void voice_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		if (bottom->voiceMax == FALSE && bottom->isPlaying == FALSE) {
			if (bottom->voice_number <= 4) {
				setCurrentIconValue(ID_BOTTOMMENU_BTN1, bottom->voice_number);
				getResBmp(ID_BOTTOMMENU_BTN1, BMPTYPE_BASE, &bottom->volumeBmp);
				SendMessage(hwnd, STM_SETIMAGE, (DWORD) &bottom->volumeBmp,
						bottom->voice_number);
				tplayer_setvolume(bottom->voice_number * 10);
				voice = tplayer_getvolume();
				printf("set voice number=%d\n", voice);
				if (bottom->voice_number == 4) {
					bottom->voiceMax = TRUE;
				} else if (bottom->voice_number < 4) {
					bottom->voice_number++;
				}
			}
		} else if (bottom->voiceMax == TRUE && bottom->isPlaying == FALSE) {
			if (bottom->voice_number >= 0) {
				bottom->voice_number--;
				setCurrentIconValue(ID_BOTTOMMENU_BTN1, bottom->voice_number);
				getResBmp(ID_BOTTOMMENU_BTN1, BMPTYPE_BASE, &bottom->volumeBmp);
				SendMessage(hwnd, STM_SETIMAGE, (DWORD) &bottom->volumeBmp,
						bottom->voice_number);
				tplayer_setvolume(bottom->voice_number * 10);
				voice = tplayer_getvolume();
				printf("set voice number=%d\n", voice);
				if (bottom->voice_number == 0) {
					bottom->voice_number++;
					bottom->voiceMax = FALSE;
				}
			}
		} else if (bottom->isPlaying == TRUE) {
			printf("music no play, please play music then set volume\n");
		}
	}
}

void startMusic() {
	int curindex = -1;
	tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
	int index = file_index;
	/*int index = SendMessage (hwnd, LB_GETCURSEL, 0, 0L);*/
	if (RightMenuWnd->filepath_list[0] != NULL) {

		pathFindFilename(RightMenuWnd->filepath_list);
	} else {
		printf("please choose url\n");
		return;
	}

	if (headbar->isRightWndOpen == TRUE) {
		ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		headbar->isRightWndOpen = FALSE;
	}
	if (file_index >= 0) {
		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleStartBmp);
		SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleStartBmp, 0);
		InvalidateRect(bottom->musicMiddleHwnd, NULL, TRUE);
		if (rcd->rpData.videoCurTime <= 0) {
			startPlayer();
		} else {
			curindex = SendMessage(RightMenuWnd->ListBox0, LB_GETCURSEL, 0, 0L);
			if (curindex == index) {
				playerseekto(rcd->rpData.videoCurTime);
			} else {
				file_index = curindex;
				tplayer_exit();
				startPlayer();
			}
		}
	} else if (file_index == -1) {
		printf("please choose music file url:\n");
	}

}
void pauseMusic() {
	setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
	getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE, &bottom->musicMiddleBmp);
	SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
			(DWORD) &bottom->musicMiddleBmp, 0);
	InvalidateRect(bottom->musicMiddleHwnd, NULL, TRUE);
	pausePlayer();
}
static void music_start_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED && bottom->isPlaying == TRUE
			&& bottom->isfirstFile == FALSE && bottom->islastFile == FALSE) {
		startMusic();
	} else if (nc == STN_CLICKED && bottom->isPlaying == FALSE) {
		pauseMusic();
	}
}
void startMusicLeft() {
	stopPlayer();
	tplayer_exit();
	seektobar0->seektobarlen = 1;
	int index = file_index;
	if (RightMenuWnd->paginal_number > 1) {
		if (file_index > 0) {
			bottom->islastFile = FALSE;
			index--;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			startPlayer();
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);
			file_index = index;
		} else if (file_index == 0) {
			index = FILE_MAX - 1;
			SendMessage(RightMenuWnd->prevbutton, BM_CLICK, 0, 0);
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			startPlayer();
			file_index = index;
		}
	} else if (RightMenuWnd->paginal_number == 1 && file_index > 0) {
		bottom->islastFile = FALSE;
		index--;
		SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
		startPlayer();
		ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		headbar->isRightWndOpen = FALSE;
		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleStartBmp);
		SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleStartBmp, 0);
		file_index = index;
	} else if (RightMenuWnd->paginal_number == 1 && file_index <= 0) {
		printf("this is first one\n");
		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleBmp);
		SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleBmp, 0);
		seektobar0->seektobarlen = 1;
		bottom->isfirstFile = TRUE;
		bottom->onceflash = TRUE;
	}
}
static void musicleft_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		startMusicLeft();
	}
}
void startMusicRight() {
	stopPlayer();
	tplayer_exit();
	seektobar0->seektobarlen = 1;
	int index = file_index;
	if (RightMenuWnd->paginal_number >= 1
			&& RightMenuWnd->paginal_number < next_number + 1) {
		if (file_index >= 0 && file_index < FILE_MAX - 1) {
			bottom->isfirstFile = FALSE;
			index++;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			startPlayer();
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);
			file_index = index;
		} else if (file_index == FILE_MAX - 1) {
			index = 0;
			SendMessage(RightMenuWnd->nextbutton, BM_CLICK, 0, 0);
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			startPlayer();
			file_index = index;
		}
	} else if (RightMenuWnd->paginal_number == next_number + 1) {
		if (file_index < RightMenuWnd->paginal_residual_number - 1) {
			index++;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			startPlayer();
			headbar->isRightWndOpen = FALSE;
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);
			file_index = index;
		} else if (file_index == RightMenuWnd->paginal_residual_number - 1) {
			printf("this is last one\n");
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleBmp);
			SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleBmp, 0);
			seektobar0->seektobarlen = 1;
			bottom->islastFile = TRUE;
			bottom->onceflash = TRUE;
		}
	}
}
static void musicright_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		startMusicRight();
	}
}

int jpeg_dispClose() {
/*	DispSetEnable(hdl, 0);*/
	DispFreeVideoMem(hdl);
	DispDeinit(hdl);
	DestroyVideoOutport(hdl);
	JpegDecoderDestory(jpegdecoder);
	bottom->isPlaying = TRUE;
	hdl = NULL;
	jpegdecoder = NULL;
	imgFrame = NULL;
	return 0;
}

int jpeg_disp() {
	VoutRect *rect;
	videoParam videoparam;
	rect = (VoutRect *) malloc(sizeof(VoutRect));
	hdl = CreateVideoOutport(0);
	DispInit(hdl, 1, ROTATION_ANGLE_0, rect);
	int width = DispGetScreenWidth(hdl);
	int height = DispGetScreenHeight(hdl);
	rect->x = 0;
	rect->y = 0;
	rect->width = width;
	rect->height = height;
	DispSetEnable(hdl, 1);
	DispSetRoute(hdl, VIDEO_SRC_FROM_CAM);
	DispSetRect(hdl, rect);

	videoparam.srcInfo.w = imgFrame->mDisplayWidth;
	videoparam.srcInfo.h = imgFrame->mDisplayHeight;
	printf("%d,%d\n", imgFrame->mDisplayWidth, imgFrame->mDisplayHeight);
	DispAllocateVideoMem(hdl, &videoparam);
	DispSetZorder(hdl, VIDEO_ZORDER_MIDDLE);

	videoparam.srcInfo.w = imgFrame->mDisplayWidth;
	videoparam.srcInfo.h = imgFrame->mDisplayHeight;
	videoparam.srcInfo.crop_x = 0;
	videoparam.srcInfo.crop_y = 0;
	videoparam.srcInfo.crop_w = imgFrame->mDisplayWidth;
	videoparam.srcInfo.crop_h = imgFrame->mDisplayHeight;
	videoparam.srcInfo.format = VIDEO_PIXEL_FORMAT_YV12;
	videoparam.srcInfo.color_space = 0;
	int writedata = DispWriteData(hdl, imgFrame->mYuvData, imgFrame->mYuvSize,
			&videoparam);

	bottom->isPlaying = FALSE;
	return 0;
}

int jpeg_decoder(void) {
	jpegdecoder = JpegDecoderCreate();
	if (NULL == jpegdecoder) {
		printf("create jpegdecoder failed\n");
		return -1;
	}
	JpegDecodeScaleDownRatio scaleRatio;
	scaleRatio = JPEG_DECODE_SCALE_DOWN_1;
	JpegDecodeOutputDataType outputDataTpe;
	outputDataTpe = JpegDecodeOutputDataYV12;
	JpegDecoderSetDataSource(jpegdecoder,
			RightMenuWnd->filepath_list[file_index], scaleRatio, outputDataTpe);

	imgFrame = JpegDecoderGetFrame(jpegdecoder);

	if (imgFrame == NULL) {
		printf("JpegDecoderGetFrame fail\n");
		JpegDecoderDestory(jpegdecoder);
		return -1;
	} else {
		printf(
				"JpegDecoderGetFrame successfully,imgFrame->mWidth = %d,imgFrame->mHeight = %d,imgFrame->mYuvData = %p,imgFrame->mYuvSize = %d\n",
				imgFrame->mWidth, imgFrame->mHeight, imgFrame->mYuvData,
				imgFrame->mYuvSize);
		printf("imgFrame->mRGB565Data = %p,imgFrame->mRGB565Size = %d\n",
				imgFrame->mRGB565Data, imgFrame->mRGB565Size);
		printf("imgFrame->mDisplayWidth= %d, imgFrame->mDisplayHeight=%d\n",
				imgFrame->mDisplayWidth, imgFrame->mDisplayHeight);
	}
	return 0;
}

void startPicture() {
	int index = file_index;

	pathFindFilename(RightMenuWnd->filepath_list);
	if (headbar->isRightWndOpen == TRUE) {
		ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		headbar->isRightWndOpen = FALSE;
	}
	if (file_index >= 0) {
		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleStartBmp);
		SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleStartBmp, 0);
		if (hdl != NULL) {
			DispFreeVideoMem(hdl);
			DispDeinit(hdl);
			DestroyVideoOutport(hdl);
			hdl = NULL;
		}

		jpeg_decoder();
		jpeg_disp();
	} else {
		printf("please choose picture\n");
	}

}
void pausePicture() {
	setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
	getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE, &bottom->musicMiddleBmp);
	SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
			(DWORD) &bottom->musicMiddleBmp, 0);
	bottom->isPlaying = TRUE;
}
static void picture_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED && bottom->isPlaying == TRUE) {
		startPicture();
	} else if (nc == STN_CLICKED && bottom->isPlaying == FALSE) {
		pausePicture();
	}
}
void startPictureLeft() {
	if (hdl != NULL) {
		DispFreeVideoMem(hdl);
		DispDeinit(hdl);
		DestroyVideoOutport(hdl);
		hdl = NULL;
	}
	int index = file_index;
	if (RightMenuWnd->paginal_number > 1) {
		if (file_index > 0) {
			index--;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			bottom->isPlaying = FALSE;
			file_index = index;

			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;

			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);

			jpeg_decoder();
			jpeg_disp();

		} else if (file_index == 0) {
			index = FILE_MAX - 1;
			SendMessage(RightMenuWnd->prevbutton, BM_CLICK, 0, 0);
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			jpeg_decoder();
			jpeg_disp();
			file_index = index;
		}
	} else if (RightMenuWnd->paginal_number == 1 && file_index > 0) {
		index--;
		SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
		bottom->isPlaying = FALSE;
		file_index = index;

		ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		headbar->isRightWndOpen = FALSE;

		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleStartBmp);
		SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleStartBmp, 0);

		jpeg_decoder();
		jpeg_disp();
	} else if (RightMenuWnd->paginal_number == 1 && file_index <= 0) {
		setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
		getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
				&bottom->musicMiddleBmp);
		SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
				(DWORD) &bottom->musicMiddleBmp, 0);

	}

}
static void pictureleft_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		startPictureLeft();
	}
}

void startPictureRight() {
	if (hdl != NULL) {
		DispFreeVideoMem(hdl);
		DispDeinit(hdl);
		DestroyVideoOutport(hdl);
		hdl = NULL;
	}
	int index = file_index;
	if (RightMenuWnd->paginal_number >= 1
			&& RightMenuWnd->paginal_number < next_number + 1) {
		if (file_index >= 0 && file_index < FILE_MAX - 1) {
			index++;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			file_index = index;
			jpeg_decoder();
			jpeg_disp();

			bottom->isPlaying = FALSE;
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;

			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);

		} else if (file_index == FILE_MAX - 1) {
			index = 0;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			SendMessage(RightMenuWnd->nextbutton, BM_CLICK, 0, 0);
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			jpeg_decoder();
			jpeg_disp();
			file_index = index;
		}
	} else if (RightMenuWnd->paginal_number == next_number + 1) {
		if (file_index < RightMenuWnd->paginal_residual_number - 1) {
			index++;
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, index, 0);
			file_index = index;
			jpeg_decoder();
			jpeg_disp();

			bottom->isPlaying = FALSE;
			headbar->isRightWndOpen = FALSE;

			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 1);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleStartBmp);
			SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleStartBmp, 0);

		} else {
			printf("this is last one\n");
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleBmp);
			SendMessage(bottom->pictureMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleBmp, 0);

		}
	}
}
static void pictureright_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		startPictureRight();
	}
}

static SeektoBar* SeektoBarInit(SeektoBar *seektobar, int id) {
	seektobar = (SeektoBar*) malloc(sizeof(SeektoBar));
	if (NULL == seektobar) {
		printf("malloc ButtonView data error\n");
	}
	memset((void *) seektobar, 0, sizeof(SeektoBar));
	seektobar->seektobarlen = 1;
	seektobar->isDown = FALSE;
	setCurrentIconValue(ID_SEEKTOBAR, 1);
	getResBmp(ID_SEEKTOBAR, BMPTYPE_BASE, &seektobar->bmpSelect);
	return seektobar;
}

static ButtonView* MenuButtonDataInit(ButtonView *buttonData, int id);

static void CreateBottomMenuBtn(HWND hWnd) {
	getResRect(ID_SEEKTOBAR, &bottom->startMenuSize);
	setCurrentIconValue(ID_BOTTOMMENU_BTN1, bottom->voice_number);
	getResRect(ID_BOTTOMMENU_BTN1, &bottom->volumeMenuSize);
	getResBmp(ID_BOTTOMMENU_BTN1, BMPTYPE_BASE, &bottom->volumeBmp);

	printf("bottom button1:[%d][%d][%d][%d]\n", bottom->volumeMenuSize.x,
			bottom->volumeMenuSize.y, bottom->volumeMenuSize.w,
			bottom->volumeMenuSize.h);
	seektobar0 = SeektoBarInit(seektobar0, 0);
	bottom->startMenuHwnd = CreateWindowEx(SEEKTO_BAR, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_SEEKTOBAR, bottom->startMenuSize.x,
			bottom->startMenuSize.y, bottom->startMenuSize.w,
			bottom->startMenuSize.h, hWnd, (DWORD) seektobar0);
	SetNotificationCallback(bottom->startMenuHwnd, seektobar_notif_proc);

	bottom->volumeMenuHwnd = CreateWindowEx(CTRL_STATIC, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOMMENU_BTN1, bottom->volumeMenuSize.x,
			bottom->volumeMenuSize.y, bottom->volumeMenuSize.w,
			bottom->volumeMenuSize.h, hWnd, (DWORD) &bottom->volumeBmp);
	SetNotificationCallback(bottom->volumeMenuHwnd, voice_notif_proc);
}

static ButtonView* MenuButtonDataInit(ButtonView *buttonData, int id) {
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		printf("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));
	if (id == ID_BOTTOM_MUSIC_LEFT) {
		getResBmp(ID_BOTTOM_MUSIC_LEFT, BMPTYPE_BASE, &buttonData->bmpNormal);
	} else if (id == ID_BOTTOM_MUSIC_RIGHT) {
		getResBmp(ID_BOTTOM_MUSIC_RIGHT, BMPTYPE_BASE, &buttonData->bmpNormal);
	}
	return buttonData;
}

static void CreateBottomMusic(HWND hWnd) {
	static ButtonView * buttonleft;
	static ButtonView *buttonright;

	getResRect(ID_BOTTOM_MUSIC_LEFT, &bottom->musicLeftSize);
	buttonleft = MenuButtonDataInit(buttonleft, ID_BOTTOM_MUSIC_LEFT);
/*	getResBmp(ID_BOTTOM_MUSIC_LEFT, BMPTYPE_BASE, &buttonData->bmpSelect);*/
	printf("ID MUSIC LEFT:[%d][%d][%d][%d]\n", bottom->musicLeftSize.x,
			bottom->musicLeftSize.y, bottom->musicLeftSize.w,
			bottom->musicLeftSize.h);
	bottom->musicLeftHwnd = CreateWindowEx(BUTTON_VIEW, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_MUSIC_LEFT, bottom->musicLeftSize.x,
			bottom->musicLeftSize.y, bottom->musicLeftSize.w,
			bottom->musicLeftSize.h, hWnd, (DWORD) buttonleft);
	SetNotificationCallback(bottom->musicLeftHwnd, musicleft_notif_proc);

	setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
	getResRect(ID_BOTTOM_MUSIC_MIDDLE, &bottom->musicMiddleSize);
	getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE, &bottom->musicMiddleBmp);
	printf("ID MUSIC MIDDLE:[%d][%d][%d][%d]\n", bottom->musicMiddleSize.x,
			bottom->musicMiddleSize.y, bottom->musicMiddleSize.w,
			bottom->musicMiddleSize.h);
	bottom->musicMiddleHwnd = CreateWindowEx(CTRL_STATIC, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_MUSIC_MIDDLE, bottom->musicMiddleSize.x,
			bottom->musicMiddleSize.y, bottom->musicMiddleSize.w,
			bottom->musicMiddleSize.h, hWnd, (DWORD) &bottom->musicMiddleBmp);
	SetNotificationCallback(bottom->musicMiddleHwnd, music_start_notif_proc);

	getResRect(ID_BOTTOM_MUSIC_RIGHT, &bottom->musicRightSize);
	buttonright = MenuButtonDataInit(buttonright, ID_BOTTOM_MUSIC_RIGHT);
	/*getResBmp(ID_BOTTOM_MUSIC_RIGHT, BMPTYPE_BASE, &buttonData->bmpSelect);*/
	printf("ID MUSIC RIGHT:[%d][%d][%d][%d]\n", bottom->musicRightSize.x,
			bottom->musicRightSize.y, bottom->musicRightSize.w,
			bottom->musicRightSize.h);
	bottom->musicRightHwnd = CreateWindowEx(BUTTON_VIEW, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_MUSIC_RIGHT, bottom->musicRightSize.x,
			bottom->musicRightSize.y, bottom->musicRightSize.w,
			bottom->musicRightSize.h, hWnd, (DWORD) buttonright);
	SetNotificationCallback(bottom->musicRightHwnd, musicright_notif_proc);
}

static void CreateBottomPicture(HWND hwnd) {
	getResRect(ID_BOTTOM_MUSIC_LEFT, &bottom->musicLeftSize);
	getResBmp(ID_BOTTOM_MUSIC_LEFT, BMPTYPE_BASE, &bottom->musicLeftBmp);
	printf("ID MUSIC LEFT:[%d][%d][%d][%d]\n", bottom->musicLeftSize.x,
			bottom->musicLeftSize.y, bottom->musicLeftSize.w,
			bottom->musicLeftSize.h);
	bottom->pictureLeftHwnd = CreateWindowEx(CTRL_STATIC, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_PICTURE_LEFT, bottom->musicLeftSize.x,
			bottom->musicLeftSize.y, bottom->musicLeftSize.w,
			bottom->musicLeftSize.h, hwnd, (DWORD) &bottom->musicLeftBmp);
	SetNotificationCallback(bottom->pictureLeftHwnd, pictureleft_notif_proc);

	setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
	getResRect(ID_BOTTOM_MUSIC_MIDDLE, &bottom->musicMiddleSize);
	getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE, &bottom->musicMiddleBmp);
	printf("ID MUSIC MIDDLE:[%d][%d][%d][%d]\n", bottom->musicMiddleSize.x,
			bottom->musicMiddleSize.y, bottom->musicMiddleSize.w,
			bottom->musicMiddleSize.h);
	bottom->pictureMiddleHwnd = CreateWindowEx(CTRL_STATIC, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_PICTURE_MIDDLE, bottom->musicMiddleSize.x,
			bottom->musicMiddleSize.y, bottom->musicMiddleSize.w,
			bottom->musicMiddleSize.h, hwnd, (DWORD) &bottom->musicMiddleBmp);
	SetNotificationCallback(bottom->pictureMiddleHwnd, picture_notif_proc);

	getResRect(ID_BOTTOM_MUSIC_RIGHT, &bottom->musicRightSize);
	getResBmp(ID_BOTTOM_MUSIC_RIGHT, BMPTYPE_BASE, &bottom->musicRightBmp);
	printf("ID MUSIC RIGHT:[%d][%d][%d][%d]\n", bottom->musicRightSize.x,
			bottom->musicRightSize.y, bottom->musicRightSize.w,
			bottom->musicRightSize.h);
	bottom->pictureRightHwnd = CreateWindowEx(CTRL_STATIC, " ",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP,
	WS_EX_TRANSPARENT, ID_BOTTOM_PICTURE_RIGHT, bottom->musicRightSize.x,
			bottom->musicRightSize.y, bottom->musicRightSize.w,
			bottom->musicRightSize.h, hwnd, (DWORD) &bottom->musicRightBmp);
	SetNotificationCallback(bottom->pictureRightHwnd, pictureright_notif_proc);
}

static int BottomMenuProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {
	int id = 0;
	HDC hdc;
	int index = 0;
	switch (message) {
	case MSG_CREATE:
		switch (bottom->addID) {
		case ID_PLAYER:
			CreateBottomMusic(hWnd);
			CreateBottomMenuBtn(hWnd);
			break;
		case ID_MUSIC:
			CreateBottomMusic(hWnd);
			CreateBottomMenuBtn(hWnd);
			break;
		case ID_PICTURE:
			CreateBottomPicture(hWnd);
			break;
		}
		return 0;
	case MSG_KEYUP:
		switch (wParam) {

		case CVR_KEY_MODE:
			sm_debug("CVR_KEY_MODE\n");
			RightMenuInit(headbar->HbarHwnd, ID_LIST, headbar->addID);
			SendMessage(RightMenuWnd->ListBox0, LB_SETCURSEL, 0, 0);
			file_index = 0;
			break;

		case CVR_KEY_POWER:
			sm_debug("CVR_KEY_POWER\n");
			head_back(headbar->backHwnd);
			g_win_data->curIndex = 0;

			break;
		}
		return 0;
	case MSG_USRPLAYEXIT:
		if (headbar->isforloop == 1) {
			tplayer_seekto(0);
			tplayer_play();
		} else {
			setCurrentIconValue(ID_BOTTOM_MUSIC_MIDDLE, 0);
			getResBmp(ID_BOTTOM_MUSIC_MIDDLE, BMPTYPE_BASE,
					&bottom->musicMiddleBmp);
			SendMessage(bottom->musicMiddleHwnd, STM_SETIMAGE,
					(DWORD) &bottom->musicMiddleBmp, 0);
			bottom->isPlaying = TRUE;
			rcd->rpData.videoCurTime = 0;
		}

		return 0;
	case MSG_ERASEBKGND:
		if (bottom->addID != ID_PICTURE) {
			break;
		}
		hdc = BeginPaint(hWnd);
		SetMemDCColorKey(hdc, MEMDC_FLAG_SRCCOLORKEY, PIXEL_transparent);
		EndPaint(hWnd, hdc);
		return 0;
		case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		break;
		case MSG_DESTROY:
		if (NULL != bottom) {
			unloadBitMap(&bottom->volumeBmp);
			unloadBitMap(&bottom->musicLeftBmp);
			unloadBitMap(&bottom->musicMiddleBmp);
			unloadBitMap(&bottom->musicRightBmp);
			unloadBitMap(&bottom->musicMiddleStartBmp);
			if (bottom->tplayerOpen == TRUE) {
				tplayer_exit();
			}
			if (jpegdecoder != NULL) {
				JpegDecoderDestory(jpegdecoder);
				jpegdecoder = NULL;
			}
			if (hdl != NULL) {
				DispDeinit(hdl);
				DispFreeVideoMem(hdl);
				DestroyVideoOutport(hdl);
				hdl = NULL;
			}
			KillTimer(hWnd, _ID_SEEKBAR_TIMER);
			free(bottom);
			bottom = NULL;
		}
		return 0;
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void InitAllResource(int id) {
	if (id == ID_PICTURE) {
		bottom->tplayerOpen = FALSE;
		bottom->isPlaying = TRUE;
		bottom->addID = id;
	} else {
		s32 ret = 0;
		rcd = (RecordWinDataType *) malloc(sizeof(RecordWinDataType));
		/*	ret = tplayer_init(0);*/
		/*display_lcd_onoff(1);*/
		bottom->voice_number = 1;
		/*tplayer_setvolume(bottom->voice_number*10);*/
		bottom->tplayerOpen = FALSE;
		bottom->isPlaying = TRUE;
		bottom->voiceMax = FALSE;
		bottom->speed = 0;
		bottom->addID = id;
		bottom->media_number = 0;
		bottom->isfirstFile = FALSE;
		bottom->islastFile = FALSE;
		bottom->onceflash = FALSE;
	}
}

int BottomMenuInit(HWND hosting, int id) {
	MSG Msg;
	MAINWINCREATE CreateInfo;
	if (NULL == bottom) {
		bottom = (BottomMenuDataType*) malloc(sizeof(BottomMenuDataType));
		if (NULL == bottom) {
			sm_error("malloc usb window data error\n");
			return -1;
		}
		memset((void *) bottom, 0, sizeof(BottomMenuDataType));
	}
	InitAllResource(id);
	getResRect(ID_BOTTOM_MENU, &bottom->mainSize);
	printf("bottom menu:[%d][%d][%d][%d]\n", bottom->mainSize.x,
			bottom->mainSize.y, bottom->mainSize.w, bottom->mainSize.h);

	CreateInfo.dwStyle = WS_NONE;
/*        CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;*/
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = BottomMenuProc;
	CreateInfo.lx = bottom->mainSize.x;
	CreateInfo.ty = bottom->mainSize.y;
	CreateInfo.rx = bottom->mainSize.w;
	CreateInfo.by = bottom->mainSize.h;
	CreateInfo.iBkColor = PIXEL_transparent;

/*	CreateInfo.iBkColor = COLOR_black;*/
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	bottom->BottomMenuHwnd = CreateMainWindow(&CreateInfo);
	bottom->BottomFatherHwnd = hosting;
	if (bottom->BottomMenuHwnd == HWND_INVALID)
		return -1;

	ShowWindow(bottom->BottomMenuHwnd, SW_SHOWNORMAL);
	MainWindowThreadCleanup(bottom->BottomMenuHwnd);
	return 0;
}
