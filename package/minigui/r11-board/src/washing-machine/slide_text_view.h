#ifndef SLIDE_TEXT_VIEW_H_
#define SLIDE_TEXT_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include <string.h>
#include <unistd.h>

#define SLIDE_TEXT_VIEW "slideTextView"
#define LIST_MAX_SIZE 8
#define PAGE_ONE 0
#define PAGE_TOW 1
#define SLIDE_TEXT_TIMER_ID 12

typedef struct {
	BITMAP bmpBg;
	PLOGFONT logfontSelect;
	char *listData1[LIST_MAX_SIZE];
	char *listData2[LIST_MAX_SIZE];
	int listSize1;
	int listSize2;
	int selectIndex1;
	int selectIndex2;
	int currentPage;
	HWND notifHwnd;
} SlideTextView;

void SlideTextViewSwitch(HWND hwnd);
BOOL RegisterSlideTextView(void);
void UnregisterSlideTextView(void);
BOOL ChangeSlideTextPage(HWND hwnd);

#endif /* SLIDE_TEXT_VIEW_H_ */
