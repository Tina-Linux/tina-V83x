#ifndef __CVR_MAIN_WINDOW_H__
#define __CVR_MAIN_WINDOW_H__

#include "CvrInclude.h"
#include "CvrResource.h"
#include "middle_ware.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

// statement
typedef int (*CvrWinProc) (HWND, int, WPARAM, LPARAM);

// window name
#define WINDOW_RECORD			"CvrRecordWin"
#define WINDOW_MENU				"CvrMenuWin"
#define WINDOW_USB				"CvrUsbWin"
#define WINDOW_HBAR				"CvrHbarWin"

typedef enum
{
	SHORT_PRESS = 0,
	LONG_PRESS = 1,
}KeyType;

typedef struct
{
	HWND MainWinHwnd;

	HWND RecordWinHwnd;
	HWND HbarWinHwnd;
	HWND MenuWinHwnd;
	HWND UsbWinHwnd;

	WindowIDType curWindowID;
	WindowIDType oldWindowID;
	CvrRectType mainSize;

	bool isKeyUp;
	s32 downKey;

	power_type_t powerStaus;
	power_level_t powerLevel;
	bool bTfcardExist;

	pthread_t threadID;
	bool bThreadRun;
	bool bLcdOn;

	s32 auto_lcd_off_timer;		// 自动关屏时间
	s32 auto_lcd_off_cnt;		// 自动关屏计时
}MainWinDataType;

// interface
HWND GetMainWinHwnd(void);
MainWinDataType* GetMainWinData(void);
void ChangeWindow(MainWinDataType *mainWinData, WindowIDType oldWinID, WindowIDType newWinID);

#endif