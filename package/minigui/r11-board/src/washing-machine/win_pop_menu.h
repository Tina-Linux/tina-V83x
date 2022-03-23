#ifndef SLIDE_MENU_WIN_H_
#define SLIDE_MENU_WIN_H_

#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define LX 416
#define TY 0
#define RX 800
#define BY 480
#define WIDTH 384
//static BITMAP bmpData[12];
HWND button;
HWND GetPopMenuHwnd();
int WinPopMenuWin(HWND hosting);

#endif /* SLIDE_MENU_WIN_H_ */
