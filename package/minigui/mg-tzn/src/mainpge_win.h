#ifndef __MAINPAGE_WINDOW_H__
#define __MAINPAGE_WINDOW_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "smart_resource.h"

typedef struct
{
	HWND parentHwnd;
	HWND mainpgeHwnd;
	smRect mainpgeSize;
	HWND leftHwnd;
	BITMAP iconleft;
	HWND rightHwnd;
	BITMAP iconright;
	int first_page;
	int curIconIdx[4];
	int curFontIdx[4];
	HWND iconHwnd[4];
	BITMAP iconItem[4];
	HWND fontHwnd[4];
	BITMAP fontItem[4];
}maiPageWinData;

maiPageWinData* GetMainPgeWinData(void);
HWND GetMainPgeWin(void);
HWND MainPgeWinInit(HWND hParent);
int MainPgeWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif

