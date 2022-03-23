#ifndef __HEADBAR_WINDOW_H__
#define __HEADBAR_WINDOW_H__

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
	HWND headBarHwnd;
	smRect headBarSize;

	BITMAP headbgBmp;

	BITMAP s_Head_unlock;
	BITMAP s_Head_shape;
	BITMAP s_Head_preopen;

	HWND dateHwnd;
	HWND timeHwnd;

	HWND labelLock;
    BITMAP s_Head_lock;

	HWND iconHwnd[8];
	BITMAP iconItem[8];
	HWND fontHwnd[8];
	BITMAP fontItem[8];
}headBarWinData;

headBarWinData* GetHeadBarWinData(void);
HWND GetHeadBarWin(void);
HWND HeadBarWinInit(HWND hParent);
int HeadBarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif

