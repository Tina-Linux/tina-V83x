#ifndef __CVR_HABR_WINDOW_H__
#define __CVR_HABR_WINDOW_H__

#include "CvrInclude.h"
#include "CvrResource.h"

#include <sys/time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

typedef struct
{
	HWND ParentHwnd;
	HWND HbarHwnd;
	CvrRectType hbarSize;

	HWND audioHwnd;
	BITMAP audioBmp;

	HWND tfcardHwnd;
	BITMAP tfcardBmp;

	HWND batHwnd;
	BITMAP batBmp;

	HWND uvcHwnd;
	BITMAP uvcBmp;

	HWND lockHwnd;
	BITMAP lockBmp;

	HWND label1Hwnd;
	HWND label2Hwnd;
	HWND label3Hwnd;
}HbarWinDataType;

HbarWinDataType* GetHbarWinData(void);
HWND GetHbarWinHwnd(void);
HWND HbarWinInit(HWND hParent);
int HbarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif