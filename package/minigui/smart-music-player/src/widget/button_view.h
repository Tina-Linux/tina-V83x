#ifndef SRC_WIDGET_BUTTON_VIEW_H_
#define SRC_WIDGET_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define BUTTON_VIEW "buttonView"
#define SELECT_STATUS_YES 1
#define SELECT_STATUS_NO 0
#define SWITCH_MODE_NORMAL 0
#define SWITCH_MODE_STATIC 1
#define SWITCH_MODE_FORBID 2
#define WE_FGC_TEXT_COLOR 0

typedef struct {
    HWND buttonWnd;
    BITMAP bmpSelect;
    BITMAP bmpNormal;
    int switchMode;
    int selectStatus;
    int doubleClickDisable;
} ButtonData;

BOOL RegisterButtonView(void);
void UnregisterButtonView(void);

#endif /* SRC_WIDGET_BUTTON_VIEW_H_ */
