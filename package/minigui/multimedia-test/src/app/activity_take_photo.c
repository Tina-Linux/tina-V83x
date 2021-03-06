#include "resource.h"
#include "recorder_int.h"
#include <bottombar_view.h>

static HWND hMainWnd = HWND_INVALID;
/* Camera initialization status, 0 success, 1 failed */
static s32 recInitSta = -1;
/* -1 Forced exit, 0 click to return, 1 click to set the time */
static int freedSta = -1;
static BOOL bRecordListExit = FALSE;
/* Without the touch screen and mouse, mark the currently selected button */
static int curIndex = 0;

/* Initialize the camera, only use the front camera */
static s32 DvInit(void) {
	smRect rect;
	getResRect(ID_FRONT_CAMRA_SIZE, &rect);
	R_SIZE frontCamera, backCamera, frontRreviewRect, backRreviewRect;
	frontCamera.x = rect.x;
	frontCamera.y = rect.y;
	frontCamera.width = rect.w;
	frontCamera.height = rect.h;
	getResRect(ID_BACK_CAMRA_SIZE, &rect);
	backCamera.x = rect.x;
	backCamera.y = rect.y;
	backCamera.width = rect.w;
	backCamera.height = rect.h;
	frontRreviewRect.x = 0;
	frontRreviewRect.y = 0;
	frontRreviewRect.width = g_rcScr.right;
	frontRreviewRect.height = g_rcScr.bottom;
	backRreviewRect.x = 0;
	backRreviewRect.y = 0;
	backRreviewRect.width = g_rcScr.right;
	backRreviewRect.height = g_rcScr.bottom;

	recInitSta = recorder_init_info(frontCamera, backCamera, frontRreviewRect,
			frontRreviewRect);
	recInitSta = recorder_init(0, 0);
	if (recInitSta == -1) {
		sm_warn("recorder init error\n");
		return recInitSta;
	}
	recorder_start_preview(0);
	int ret;
	ret = format_is_correct();
	if (ret == -1) {
		sm_warn("SD card or U disk does not exist\n");
	} else if (ret == 0) {
		sm_warn("SD card or U disk format is not correct, please format\n");
	} else if (ret == 1) {
		if (!bRecordListExit) {
			sm_debug("create rec list\n");
			create_rec_list();
			bRecordListExit = TRUE;
		}
	}
	return 0;
}
/* Log out of the camera */
static s32 DvExit(void) {
	recorder_exit(0, 0);
	int ret;
	ret = format_is_correct();
	if (ret == 1 && bRecordListExit) {
		sm_warn("destroy rec list\n");
		destroy_rec_list();
		bRecordListExit = FALSE;
	}
	return 0;
}
/* Check the SD card and capacity */
static s32 PhotoDiskCheck(void) {
	s32 ret = -1;
	s32 size = 0;

	ret = format_is_correct();
	if (ret == -1) {
		sm_warn("SD card or U disk does not exist\n");
		return -1;
	} else if (ret == 0) {
		sm_warn("SD card or U disk format is not correct, please format\n");
		return -1;
	} else if (ret == 1) {
		if (!bRecordListExit) {
			sm_warn("recordList is not exit\n");
			return -1;
		}
	}

	/* Reserved 300MB get_disk_free in KB */
	size = get_disk_free(2) - (300 * 1024);
	if (size <= 0) {
		sm_warn("tfcard is full:%d\n", size);
		return -1;
	}

	return 0;
}
/* Take a photo */
static int TakeImage(void) {
	s32 ret = -1;
	if (PhotoDiskCheck() == -1)
		return -1;

	ret = recorder_take_picture(0);
	return ret;
}

static void openWin(HWND hWnd, int index) {
	switch (index) {
	case 0:
		freedSta = 0;
		PostMessage(hWnd, MSG_CLOSE, 0, 0);
		PostMessage(GetHosting(hWnd), MSG_TAKE_CAMERA_CLOSE, 0, 0);
		break;
	case 1:
		if (recInitSta == -1) {
			sm_warn("recorder init error,\n");
		} else {
			TakeImage();
		}
		break;
	case 2:
		freedSta = 1;
		PostMessage(hWnd, MSG_CLOSE, 0, 0);
		PostMessage(GetHosting(hWnd), MSG_TAKE_CAMERA_SETUP, 0, 0);
		break;
	}
}

#define __ID_TIMER_SLIDER 100
static int ActivityTPhotoProc(HWND hWnd, int nMessage, WPARAM wParam,
		LPARAM lParam) {

	switch (nMessage) {
	case MSG_CREATE: {
		freedSta = -1;
		DvInit();
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
		break;
	}
	case MSG_TIMER: {
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		BottomBarViewInit(hWnd, 0);
		break;
	}
	case MSG_ERASEBKGND: {
		HDC hdc = (HDC) wParam;
		BOOL fGetDC = FALSE;
		if (hdc == 0) {
			hdc = GetClientDC(hWnd);
			fGetDC = TRUE;
		}
		SetMemDCColorKey(hdc, MEMDC_FLAG_SRCCOLORKEY, PIXEL_transparent);
		if (fGetDC)
			ReleaseDC(hdc);
		return 0;
	}
	case MSG_HBAR_CLICK: {
		openWin(hWnd, wParam);
		break;
	}
	case MSG_KEYUP: {
		switch (wParam) {
		case CVR_KEY_RIGHT:
			if (curIndex == 2)
				curIndex = 0;
			else
				curIndex++;
			sm_debug("CVR_KEY_RIGHT %d\n", curIndex);
			break;
		case CVR_KEY_LEFT:
			if (curIndex == 0)
				curIndex = 2;
			else
				curIndex--;
			sm_debug("CVR_KEY_LEFT %d\n", curIndex);
			break;
		case CVR_KEY_OK:
			sm_debug("CVR_KEY_OK %d\n", curIndex);
			openWin(hWnd, curIndex);
			break;
		}
		break;
	}
	case MSG_DESTROY:
		if (freedSta == -1 || freedSta == 0) {
			DvExit();
			recorder_exit_info();
			freedSta = -1;
		} else if (freedSta == 1) {
			DvExit();
			freedSta = -1;
		}
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

int ActivityTPhoto(HWND hosting) {

	if (hMainWnd != HWND_INVALID)
		return -1;

	MSG Msg;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TRANSPARENT;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityTPhotoProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = PIXEL_transparent;
	CreateInfo.dwAddData = 0;
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

