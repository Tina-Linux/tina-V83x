#ifndef __STATBAR_WINDOW_H__
#define __STATBAR_WINDOW_H__

#include <sys/time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "smart_resource.h"

typedef struct
{
	HWND parentHwnd;
	HWND statBarHwnd;
	smRect statBarSize;
	int mode_idx;

	BITMAP bgBmp;
	BITMAP headbgBmp;

	BITMAP s_Head_unlock;
	BITMAP s_Head_shape;
	BITMAP s_Head_preopen;

	HWND dateHwnd;
	HWND timeHwnd;

	HWND backHwnd;
	BITMAP backBmp;

	HWND titleHwnd;
	BITMAP titleBmp;

	HWND labelLock;
    BITMAP s_Head_lock;

	HWND iconHwnd[8];
	BITMAP iconItem[8];
	HWND fontHwnd[8];
	BITMAP fontItem[8];
}stBarWinData;

stBarWinData* GetStBarWinData(void);
HWND GetStBarWin(void);
HWND StBarWinInit(HWND hParent);
int StBarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif

