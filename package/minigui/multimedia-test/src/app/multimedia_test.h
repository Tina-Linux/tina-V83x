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

typedef int (*SmWinProc)(HWND, int, WPARAM, LPARAM);

typedef struct {
	HWND ParentHwnd;
	HWND fastMenuHwnd;
	HWND headBarSdHwnd;
	HWND headBarUdHwnd;

	BITMAP s_Head_sd_p;
	BITMAP s_Head_sd_n;
	BITMAP s_Head_ud_p;
	BITMAP s_Head_ud_n;

	pthread_t threadID;
	int bThreadRun;
	int curIndex;
} WinDataType;
#endif
