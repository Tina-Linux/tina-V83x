/*
 ** multimedia-test.c 2017-09-09 14:00:18
 **
 ** multimedia-test: Multimedia Test control UI
 **
 **
 ** Copyright (C) 2017 ~ 2020 Allwinnertech.
 **
 ** License: GPL
 */

#include "multimedia_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/ioctl.h>

#include "resource.h"
#include "multimedia_res_cn.h"
#include "volume.h"
#include "fun_button_view.h"

#define MULTIMEDIA_TITLE "Multimedia Test"

extern int ActivityTPhoto(HWND hosting);
extern int ActivityTAudio(HWND hosting);
extern int ActivityTVideo(HWND hosting);
extern int ActivityTVideoTow(HWND hosting);
extern int ActivityFormatMsg(HWND hosting, int mode);

WinDataType *g_win_data = NULL;

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		if (id == 0) {
			if (sdcard_is_mount_correct() == 1) {
				ActivityFormatMsg(GetParent(hwnd), 0);
			} else {
				sm_warn("the tf card is not mount\n");
			}
		} else if (id == 1) {
			if (udisk_is_mount_correct() == 1) {
				ActivityFormatMsg(GetParent(hwnd), 1);
			} else {
				sm_warn("the u disk is not mount\n");
			}
		}
	}
}

static void creatHeadControl(HWND hwnd) {
	smRect rect;
	getResRect(ID_HEAD_BAR_SD, &rect);
	setCurrentIconValue(ID_HEAD_BAR_SD, 0);
	getResBmp(ID_HEAD_BAR_SD, BMPTYPE_BASE, &g_win_data->s_Head_sd_n);
	setCurrentIconValue(ID_HEAD_BAR_SD, 1);
	getResBmp(ID_HEAD_BAR_SD, BMPTYPE_BASE, &g_win_data->s_Head_sd_p);

	g_win_data->headBarSdHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_NOTIFY,
	WS_EX_TRANSPARENT, 0, rect.x, rect.y, rect.w, rect.h, hwnd,
			(DWORD) &g_win_data->s_Head_sd_n);
	SetNotificationCallback(g_win_data->headBarSdHwnd, my_notify_proc);

	getResRect(ID_HEAD_BAR_UD, &rect);
	setCurrentIconValue(ID_HEAD_BAR_UD, 0);
	getResBmp(ID_HEAD_BAR_UD, BMPTYPE_BASE, &g_win_data->s_Head_ud_n);
	setCurrentIconValue(ID_HEAD_BAR_UD, 1);
	getResBmp(ID_HEAD_BAR_UD, BMPTYPE_BASE, &g_win_data->s_Head_ud_p);

	g_win_data->headBarUdHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_NOTIFY,
	WS_EX_TRANSPARENT, 1, rect.x, rect.y, rect.w, rect.h, hwnd,
			(DWORD) &g_win_data->s_Head_ud_p);
	SetNotificationCallback(g_win_data->headBarUdHwnd, my_notify_proc);

	if (sdcard_is_mount_correct() == 1) {
		SendNotifyMessage(g_win_data->headBarSdHwnd, STM_SETIMAGE,
				(DWORD) &g_win_data->s_Head_sd_n, 0);
	} else {
		SendNotifyMessage(g_win_data->headBarSdHwnd, STM_SETIMAGE,
				(DWORD) &g_win_data->s_Head_sd_p, 0);
	}

	if (udisk_is_mount_correct() == 1) {
		SendNotifyMessage(g_win_data->headBarUdHwnd, STM_SETIMAGE,
				(DWORD) &g_win_data->s_Head_ud_n, 0);
	} else {
		SendNotifyMessage(g_win_data->headBarUdHwnd, STM_SETIMAGE,
				(DWORD) &g_win_data->s_Head_ud_p, 0);
	}
}

/* The program did not create the thread */
static void *MainThreadProc(void *arg) {
	WinDataType *mwd = (WinDataType *) arg;
	mwd->bThreadRun = TRUE;
	while (1) {
		if (!mwd->bThreadRun) {
			sm_debug("main thread end\n");
			pthread_exit((void*) 1);
			sleep(1);
			sm_error("mainthread end\n");
		}
		usleep(1000 * 1000);		/* Delay 1000ms */

		if (sdcard_is_mount_correct() == 1) {
			SendNotifyMessage(g_win_data->headBarSdHwnd, STM_SETIMAGE,
					(DWORD) &g_win_data->s_Head_sd_n, 0);
		} else {
			SendNotifyMessage(g_win_data->headBarSdHwnd, STM_SETIMAGE,
					(DWORD) &g_win_data->s_Head_sd_p, 0);
		}
	}
	sm_error("main thread end fail\n");
	return (void*) 0;
}

static s32 CreateMainThread(HWND hWnd, WinDataType *mwd) {
	s32 ret = 0;

	ret = pthread_create(&mwd->threadID, NULL, MainThreadProc, (void *) mwd);
	if (ret == -1) {
		sm_debug("create thread fail\n");
		return -1;
	}
	sm_debug("g_win_data->threadID=%d\n", mwd->threadID);

	return 0;
}

static s32 CloseMainThread(HWND hWnd, WinDataType *mwd) {
	mwd->bThreadRun = FALSE;
	pthread_join(mwd->threadID, NULL);
	mwd->threadID = 0;
	return 0;
}

static void openWin(HWND hWnd, int index) {
	switch (index) {
	case 0:
		ShowWindow(hWnd, SW_HIDE);
		ActivityTPhoto(hWnd);
		break;
	case 1:
		ShowWindow(hWnd, SW_HIDE);
		ActivityTAudio(hWnd);
		break;
	case 2:
		ShowWindow(hWnd, SW_HIDE);
		ActivityTVideo(hWnd);
		break;
	case 3:
		ShowWindow(hWnd, SW_HIDE);
		ActivityTVideoTow(hWnd);
		break;
	case 4:
		ShowWindow(hWnd, SW_HIDE);
		HbarWinInit(hWnd, ID_PICTURE);
		BottomMenuInit(hWnd, ID_PICTURE);
		RightResourceInit();
		break;
	case 5:
		ShowWindow(hWnd, SW_HIDE);
		HbarWinInit(hWnd, ID_MUSIC);
		BottomMenuInit(hWnd, ID_MUSIC);
		RightResourceInit();
		break;
	case 6:
		ShowWindow(hWnd, SW_HIDE);
		HbarWinInit(hWnd, ID_PLAYER);
		BottomMenuInit(hWnd, ID_PLAYER);
		RightResourceInit();
		break;
	case 7:
		if (sdcard_is_mount_correct() == 1) {
			ActivityFormatMsg(hWnd, 0);
		} else {
			sm_warn("the tf card is not mount\n");
		}
		break;
	case 8:
		if (udisk_is_mount_correct() == 1) {
			ActivityFormatMsg(hWnd, 1);
		} else {
			sm_warn("the u disk is not mount\n");
		}
		break;
	}
}

#define __ID_TIMER_SLIDER 100
static int MainWindowProc(HWND hWnd, int nMessage, WPARAM wParam, LPARAM lParam) {

	WinDataType *mwd;
	mwd = (WinDataType*) GetWindowAdditionalData(hWnd);

	switch (nMessage) {
	case MSG_CREATE: {
		creatHeadControl(hWnd);
		/* CreateMainThread(hWnd, mwd); */
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
		break;
	}
	case MSG_TIMER: {
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		FunButtonViewInit(hWnd);
		break;
	}
	case MSG_FUN_BUTTON_CLICK:
		openWin(hWnd, wParam);
		break;
	case MSG_TAKE_CAMERA_CLOSE:
		ShowWindow(hWnd, SW_SHOWNORMAL);
		break;
		/**
		 * Click the Settings button in the recording interface,
		 * Close the recording interface, the main interface receives the message,
		 * The main interface to open the setting interface,
		 * Close the setting interface, and then open the recording interface from the main interface,
		 * Mainly in order to minigui control can be displayed on the camera data normal
		 **/
	case MSG_TAKE_CAMERA_SETUP:
		/* wParam 0 pictures, 1 recording, 2 videos, 3 double shots */
		ActivityTSetUp(hWnd, wParam);
		break;
	case MSG_TAKE_SETUP_CLOSE:
		switch (wParam) {
		case 0:
			ActivityTPhoto(hWnd);
			break;
		case 1:
			ActivityTAudio(hWnd);
			break;
		case 2:
			ActivityTVideo(hWnd);
			break;
		case 3:
			ActivityTVideoTow(hWnd);
			break;
		}
		break;
	case MSG_KEYUP: {
		switch (wParam) {
		case CVR_KEY_RIGHT:
			if (g_win_data->curIndex == 8)
				g_win_data->curIndex = 0;
			else
				g_win_data->curIndex++;
			sm_debug("CVR_KEY_RIGHT %d\n", g_win_data->curIndex);
			break;
		case CVR_KEY_LEFT:
			if (g_win_data->curIndex == 0)
				g_win_data->curIndex = 8;
			else
				g_win_data->curIndex--;
			sm_debug("CVR_KEY_LEFT %d\n", g_win_data->curIndex);
			break;
		case CVR_KEY_OK:
			sm_debug("CVR_KEY_OK %d\n", g_win_data->curIndex);
			openWin(hWnd, g_win_data->curIndex);
			break;
		}
		break;
	}
	case MSG_DESTROY:
		/* CloseMainThread(hWnd, mwd); */
		unloadBitMap(&g_win_data->s_Head_sd_n);
		unloadBitMap(&g_win_data->s_Head_sd_p);
		unloadBitMap(&g_win_data->s_Head_ud_n);
		unloadBitMap(&g_win_data->s_Head_ud_p);
		if (g_win_data)
			free(g_win_data);
		DestroyAllControls(hWnd);
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		PostQuitMessage(hWnd);
		return 0;
	}
	return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

void HardwareInit(void) {
	int ret;
	volume_init();
	volume_set_volume(24);
	ret = play_wav_music(VOICE_POWER_OFF);
	if (ret != 0) {
		printf("play %s fail\n", VOICE_POWER_ON);
	}
}

void SystemInit(void) {
	HardwareInit();
	ResourceInit();

	int ret = getPlatform();
	if (ret != -1) {
		if (ret == ALLWINNERTECH_R40) {
			system("amixer cset numid=32 1");
			system("amixer cset numid=25 1");
			system("/usr/res/SetAudioPass/r40_multimedia-test.sh");
		}else if(ret == ALLWINNERTECH_R16){
			system("/usr/res/SetAudioPass/r16_multimedia-test.sh");
		}else if(ret == ALLWINNERTECH_R18){
			system("/usr/res/SetAudioPass/r18_multimedia-test.sh");
		}else if(ret == ALLWINNERTECH_F35){
			system("/usr/res/SetAudioPass/MR100_multimedia-test.sh");
		}
	}else {
		sm_warn("get platform failure\n");
	}
}

void SystemUninit(void) {
	ResourceUninit();
	system("ubus call boot-play boot_complete '{\"stop\":true}'");
}

BOOL RegisterExWindows(void) {
	BOOL ret;
	ret = RegisterBottomBarView();
	ret = RegisterButtonView();
	ret = RegisterFunButtonView();
	ret = RegisterSeektoBarView();
	return ret;
}

void UnRegisterExWindows(void) {
	UnregisterButtonView();
	UnregisterBottomBarView();
	UnregisterFunButtonView();
	UnregisterSeektoBarView();
}

int MiniGUIMain(int argc, const char* argv[])
{
	MSG Msg;
	MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
	JoinLayer(NAME_DEF_LAYER , MULTIMEDIA_TITLE , 0 , 0);
#endif

	SystemInit();

	g_win_data = (WinDataType*) malloc(sizeof(WinDataType));
	if (NULL == g_win_data) {
		sm_error("malloc window data error\n");
		return -1;
	}
	memset((void *) g_win_data, 0, sizeof(WinDataType));
	g_win_data->curIndex = 0;

	if (!RegisterExWindows()) {
		sm_error("register windows failed\n");
		return -1;
	}

	SetDefaultWindowElementRenderer("classic");
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = MULTIMEDIA_TITLE;
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0); /*GetSystemCursor (0);*/
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWindowProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = PIXEL_lightwhite;
	CreateInfo.hHosting = HWND_DESKTOP;
	CreateInfo.dwAddData = (DWORD) g_win_data;
	g_win_data->ParentHwnd = CreateMainWindow(&CreateInfo);
	if (g_win_data->ParentHwnd == HWND_INVALID)
		return -1;

	ShowWindow(g_win_data->ParentHwnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, g_win_data->ParentHwnd)) {
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup(g_win_data->ParentHwnd);
	UnRegisterExWindows();
	SystemUninit();

	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

