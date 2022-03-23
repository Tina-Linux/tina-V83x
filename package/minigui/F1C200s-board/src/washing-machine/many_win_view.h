#ifndef MANY_WIN_VIEW_H_
#define MANY_WIN_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define MANY_WIN_VIEW "manyWinView"
#define SLIDE_MODE_APPEAR 0
#define SLIDE_MODE_SWITCH 1

typedef struct {
	BITMAP bmpFun[6];
	char *nameFun[6];
	HWND funBtn[6];
} ManyWinView;

BOOL RegisterManyWinView(void);
void UnregisterManyWinView(void);
void ManyWinViewInit(HWND hParent, ManyWinView *manyWinData1,
		ManyWinView *manyWinData2, ManyWinView *manyWinData3, BITMAP bmpPointN,
		BITMAP bmpPointP);
void ManyWinAppear();
void OnePageSwitch();
BOOL GetSlideStatus();
int GetCurrentPage();

#endif /* MANY_WIN_VIEW_H_ */
