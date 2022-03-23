#ifndef __TEMP_SETTING_WINDOW_H__
#define __TEMP_SETTING_WINDOW_H__

#include <sys/time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "smart_resource.h"

typedef struct
{
	int set_temp_flag;
	int set_temp_num;
	HWND parentHwnd;
	HWND tempSetHwnd;
	smRect tempSetSize;

	BITMAP bgBmp;
	BITMAP label_degree_icon;
	BITMAP label_up_icon;
	BITMAP label_down_icon;

	HWND label_temp_num;
	HWND label_temp_deg;
	HWND labelTempDown;
	HWND labelTempUp;
}tempSetWinData;

tempSetWinData* GetTempSetWinData(void);
HWND GetTempSetWin(void);
HWND TempSetWinInit(HWND hParent);
int TempSetWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif
