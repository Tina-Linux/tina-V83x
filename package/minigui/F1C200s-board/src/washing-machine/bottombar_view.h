#ifndef BOTTOMBAR_VIEW_H_
#define BOTTOMBAR_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "double_text_view.h"
#include "slide_text_view.h"
#include "button_view.h"

#define BOTTOM_BAR_VIEW "bottomBarView"
#define ID_SLIDE_TEXT_VIEW_ONE 1
#define ID_SLIDE_TEXT_VIEW_TOW 2
#define ID_SLIDE_TEXT_VIEW_THR 3
#define ID_DOUBLE_TEXT_VIEW_ONE 4
#define ID_DOUBLE_TEXT_VIEW_TOW 5
#define ID_DOUBLE_TEXT_VIEW_THR 6
#define ID_BUTTON_SWITCH 7
#define ID_BUTTON_START 8

typedef struct {
	DoubleTextView *doubleTextData1;
	DoubleTextView *doubleTextData2;
	DoubleTextView *doubleTextData3;
	SlideTextView *slideTextData1;
	SlideTextView *slideTextData2;
	SlideTextView *slideTextData3;
	ButtonView *nextButton;
	ButtonView *beginButton;
} BottomBarView;

BOOL RegisterBottomBarView(void);
void UnregisterBottomBarView(void);
HWND BottomBarViewInit(HWND hParent, BottomBarView *bottomBarData);
int GetBottomBarDistance();

#endif /* BOTTOMBAR_VIEW_H_ */
