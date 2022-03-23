/*
** smart_ctrl.c 2017-06-02 09:55:18
**
** smart_ctrl.c: Smart home control UI for embedded system
**
**
** Copyright (C) 2017 ~ 2020 Allwinnertech.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/ioctl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "sys_parm.h"
#include "ap_uart.h"
#include "dispatch_mgr.h"
#include "smart_ctrl.h"
#include "headbar_win.h"
#include "statbar_win.h"
#include "mainpge_win.h"
#include "temp_setting_win.h"
#include "smart_thread.h"
#include "volume.h"

WinDataType *g_win_data = NULL;
static char* g_pMainTitle = "Smart Home" ;
int DispHdle;

void WinWidgetInit(HWND hWnd, WinDataType *mwd)
{
	mwd->headBarHwnd = HeadBarWinInit(hWnd);
	mwd->statBarHwnd = StBarWinInit(hWnd);
	mwd->pagectrlHwnd = MainPgeWinInit(hWnd);
	mwd->tempctrlHwnd = TempSetWinInit(hWnd);
}

static int MainWindowProc (HWND hWnd, int nMessage, WPARAM wParam, LPARAM lParam)
{

	PBITMAP btn_bmp;
	gal_pixel pixel;
	DATE_TIME dateTime;
	WinDataType *mwd;
	int items;
	mwd = (WinDataType*)GetWindowAdditionalData(hWnd);

    if((nMessage != MSG_IDLE) &&(nMessage != MSG_TIMER)) {
    	/* DBG("hwnd=%d,nMessage=%d,wparam=%d,lparam=%d,activewin=%d.\n",
			hWnd, nMessage, wParam, lParam, GetActiveWindow());*/
    }
    switch (nMessage)
    {
        case MSG_CREATE:
            {
				sys_param_pwr_on_restore();
				WinWidgetInit(hWnd, mwd);
				ShowWindow(g_win_data->headBarHwnd, SW_SHOW);
				ShowWindow(g_win_data->pagectrlHwnd, SW_SHOW);
				//CreatMainButtonWindow(hWnd);
				//CreatSetTemperatureWindow(hWnd);
                SetTimer(hWnd, TIMER_SEC, 6000);
                break;
            }
        case MSG_TIMER:
            {
                //RECT text_rc = {TIME_POS_X, TIME_POS_Y,
                //    TIME_POS_X + 80, TIME_POS_Y + 40};
                sys_param_date_time_get(&dateTime);
                if (wParam == TIMER_SEC) {
                    dateTime.m_time.m_minute++;
                    if (dateTime.m_time.m_minute == 60) {
                        dateTime.m_time.m_minute = 0;
                        dateTime.m_time.m_hour++;
                    }
                    if (dateTime.m_time.m_hour == 24) {
                        dateTime.m_time.m_hour = 0;
                        dateTime.m_time.m_minute = 0;
						dateTime.m_date.m_day++;
                    }
                }
				sys_param_date_time_set(&dateTime);
				//InvalidateRect (hWnd, &text_rc, FALSE);
                break;
            }
		case MSG_SHOW_STATBAR:
		{
			 items = LOWORD(lParam);

			switch(items){
				case ID_MAINPAGE_BTN_ITEM1:
				case ID_MAINPAGE_BTN_ITEM2:
				case ID_MAINPAGE_BTN_ITEM3:
				{
					SendMessage (g_win_data->statBarHwnd, MSG_SHOW_ITEMS, wParam, items);
					ShowWindow(g_win_data->headBarHwnd, SW_HIDE);
					ShowWindow(g_win_data->tempctrlHwnd, SW_HIDE);
					ShowWindow(g_win_data->pagectrlHwnd, SW_HIDE);
					ShowWindow(g_win_data->statBarHwnd, SW_SHOW);
					UpdateWindow (hWnd, TRUE);
					break;
				}
			}
			break;
		}
		case MSG_SET_LOCKICON:
		{
			SendMessage (g_win_data->headBarHwnd, MSG_SET_LOCKICON, wParam, 0);
			break;
		}
		case MSG_CHANGE_ICONS:
		{
			items = LOWORD(lParam);
			SendMessage (g_win_data->pagectrlHwnd, MSG_CHANGE_ICONS, wParam, items);
			ShowWindow(g_win_data->statBarHwnd, SW_HIDE);
			ShowWindow(g_win_data->headBarHwnd, SW_SHOW);
			ShowWindow(g_win_data->pagectrlHwnd, SW_SHOW);
			ShowWindow(g_win_data->tempctrlHwnd, SW_SHOW);
			break;
		}
		case MSG_ITEMS_RETURN:
		{
			ShowWindow(g_win_data->statBarHwnd, SW_HIDE);
			ShowWindow(g_win_data->headBarHwnd, SW_SHOW);
			ShowWindow(g_win_data->pagectrlHwnd, SW_SHOW);
			ShowWindow(g_win_data->tempctrlHwnd, SW_SHOW);
			break;
		}
        case MSG_PAINT:
            {
                HDC hdc;

                hdc = BeginPaint (hWnd);
                FillBoxWithBitmap (hdc, 0, 0, 800, 480, &g_win_data->s_Main_bg);
				//FillBoxWithBitmap (hdc, 0, 0, 800, 60, &g_win_data->s_Head_bg);
				//OnModeRotation(hWnd, hdc);
                EndPaint (hWnd, hdc);
                return 0;
            }

        case MSG_COMMAND:
            break;

	    case MSG_ERASEBKGND:
			{
				break;
			}
        case MSG_CLOSE:
            {
                KillTimer (hWnd, TIMER_SEC);
                DestroyAllControls (hWnd);
                DestroyMainWindow (hWnd);
				if(g_win_data)
					free(g_win_data);
                break;
            }
	case MSG_KEYDOWN:
	    {
			break;
	    }
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

int SetLcdBackLightPwm(void)
{
	int retval;
	unsigned long args[4]={0};

	DispHdle = open("/dev/disp", O_RDWR);
	if (DispHdle < 0){
		printf("fail to open %s", "/dev/disp");
	}

	args[0] = 0;
	args[1] = 128;
	retval = ioctl(DispHdle, 0x102, args);
	if (retval < 0){
		return -1;
	}

	return 0;
}

void HardwareInit(void)
{
	int ret;
	pthread_t msg_thread;

	msg_thread = createMainThread();
	volume_set_volume(40);
	ret = play_wav_music(VOICE_POWER_OFF);
	if (ret != 0){
		printf("play %s fail\n", VOICE_POWER_ON);
	}

	SetLcdBackLightPwm();
}

void SystemInit(void)
{
	HardwareInit();
	/*dispatcher_mgr_init();
    register_all_pic();*/
	ResourceInit();
}

void SystemUninit(void)
{
	ResourceUninit();
}

int RegisterExWindows(void)
{
	int count;

	const char *windowClass[] =
	{
		WINDOW_HEADBAR,
		WINDOW_STATBAR,
		WINDOW_MAINPAGE,
		WINDOW_SETTEMP,
	};

	WNDCLASS WndClass =
	{
		NULL,
		0,
		WS_CHILD | WS_VISIBLE,
		WS_EX_USEPARENTFONT | WS_EX_TROUNDCNS | WS_EX_BROUNDCNS,
		0,
		COLOR_lightgray,
		NULL,
		0
	};

	SmWinProc windowProc[] =
	{
		HeadBarWinProc,
		StBarWinProc,
		MainPgeWinProc,
		TempSetWinProc,
	};

	for(count=0; count<SelfCtrlWinCnt; count++)
	{
		WndClass.spClassName = windowClass[count];
		WndClass.WinProc = windowProc[count];
		WndClass.iBkColor = COLOR_lightgray;

		if(RegisterWindowClass(&WndClass) == FALSE)
		{
			sm_warn("register %s failed\n", windowClass[count]);
			return -1;
		}
	}

	return 0;
}

void UnRegisterExWindows(void)
{
	UnregisterWindowClass(WINDOW_STATBAR);
	UnregisterWindowClass(WINDOW_HEADBAR);
	UnregisterWindowClass(WINDOW_MAINPAGE);
	UnregisterWindowClass(WINDOW_SETTEMP);
}

int MiniGUIMain (int argc, const char* argv[])
{
	MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
	smRect         rect;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "smart_ctrl" , 0 , 0);
#endif
	SystemInit();

	g_win_data = (WinDataType*)malloc(sizeof(WinDataType));
	if (NULL == g_win_data)
	{
		printf("malloc window data error\n");
		return -1;
	}
	memset((void *)g_win_data, 0, sizeof(WinDataType));

	/*register window ctrl*/
	if (RegisterExWindows() < 0)
	{
		sm_error("register windows failed\n");
		return -1;
	}

    SetDefaultWindowElementRenderer ("classic");
    getResRect(ID_SCREEN, &rect);
	getResBmp(ID_SCREEN_BKG, BMPTYPE_BASE, &g_win_data->s_Main_bg);
	getResBmp(ID_HEAD_BKG, BMPTYPE_BASE, &g_win_data->s_Head_bg);
    CreateInfo.dwStyle      = WS_NONE;
    CreateInfo.dwExStyle    = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS;
    CreateInfo.spCaption    = g_pMainTitle;
    CreateInfo.hMenu        = 0;
    CreateInfo.hCursor      = 0; /*GetSystemCursor (0);*/
    CreateInfo.hIcon        = 0;
    CreateInfo.MainWindowProc = MainWindowProc;
    CreateInfo.lx           = rect.x;
    CreateInfo.ty           = rect.y;
    CreateInfo.rx           = rect.w;
    CreateInfo.by           = rect.h;
    CreateInfo.iBkColor     = PIXEL_lightwhite;
    CreateInfo.hHosting     = HWND_DESKTOP;
	CreateInfo.dwAddData = (DWORD)g_win_data;
    g_win_data->ParentHwnd = CreateMainWindow (&CreateInfo);
    if (g_win_data->ParentHwnd == HWND_INVALID)
        return -1;

    ShowWindow(g_win_data->ParentHwnd, SW_SHOWNORMAL);

    while(GetMessage (&Msg, g_win_data->ParentHwnd)){
        DispatchMessage (&Msg);
    }

    MainWindowThreadCleanup (g_win_data->ParentHwnd);
	UnRegisterExWindows();
   	SystemUninit();

    return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

