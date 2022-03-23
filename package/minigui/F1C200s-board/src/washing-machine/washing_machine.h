#ifndef __SMART_CTRL_WINDOW_H__
#define __SMART_CTRL_WINDOW_H__

#include "resource.h"

#define _DEBUG_
#ifdef  _DEBUG_
#define DBG(fmt, ...)  do { printf("%s line %d, "fmt, __func__, \
	__LINE__, ##__VA_ARGS__); } while(0)
#else
#define DBG(fmt, ...)  do {  } while(0)
#endif

typedef int (*SmWinProc) (HWND, int, WPARAM, LPARAM);

typedef struct
{
	HWND ParentHwnd;
	HWND fastMenuHwnd;
	HWND slideHwnd;
	HWND popHwnd;

	BITMAP s_Head_lock;
	BITMAP s_Head_unlock;
	BITMAP s_Head_shape;
	BITMAP s_Head_preopen;

	BITMAP s_Head_bg;
	BITMAP s_Main_bg;
	int pop_menu_show;

	pthread_t threadID;
	int bThreadRun;
}WinDataType;
#endif
