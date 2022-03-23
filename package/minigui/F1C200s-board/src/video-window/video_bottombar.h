#ifndef VIDEO_BOTTOMBAR_H_
#define VIDEO_BOTTOMBAR_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define VIDEO_BOTTOM_BAR "VideoBottomBar"

#define ID_BUTTON_PRE 1
#define ID_BUTTON_NEXT 2
#define ID_BUTTON_START 3

typedef struct {
	ButtonView *preButton;
	ButtonView *nextButton;
	ButtonView *beginButton;
} VideoBarView;

BOOL RegisterVideoBottomBar(void);
void UnregisterVideoBottomBar(void);
HWND VideoBottomBarViewInit(HWND hParent, VideoBarView *bottomBarData);
int GetVideoBottomBarDistance();

#endif /* BOTTOMBAR_VIEW_H_ */
