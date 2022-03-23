#ifndef __SMART_CTRL_WINDOW_H__
#define __SMART_CTRL_WINDOW_H__

#include "resource.h"

typedef int (*SmWinProc)(HWND, int, WPARAM, LPARAM);

typedef struct {
	HWND ParentHwnd;

	pthread_t threadID;
	int bThreadRun;
} WinDataType;
#endif
