#ifndef BUTTON_VIEW_H_
#define BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "win_pop_menu.h"
#define BUTTON_VIEW "buttonView"
#define SELECT_STATUS_YES 1
#define SELECT_STATUS_NO 0
#define SWITCH_MODE_NORMAL 0
#define SWITCH_MODE_STATIC 1
#define SWITCH_MODE_FORBID 2
#define WE_FGC_TEXT_COLOR 0

typedef struct {
	BITMAP bmpSelect;
	BITMAP bmpNormal;
	char *textSelect;
	int switchMode;
	int selectStatus;
	int textX;
	int textY;
} ButtonView;

BOOL RegisterButtonView(void);
void UnregisterButtonView(void);

#endif /* BUTTON_VIEW_H_ */
