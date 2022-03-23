#ifndef SEEKTO_BAR_H_
#define SEEKTO_BAR_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "common.h"

#define SEEKTO_BAR "seektoBar"

typedef struct {
	HWND seektobarWnd;
	CvrRectType seektobarSize;
	BITMAP bmpSelect;
	/* Represents the length of seekbar */
	int seektobarlen;
	BOOL isDown;
} SeektoBar;

BOOL RegisterButtonView(void);
void UnregisterButtonView(void);

#endif /* SEEKTO_BAR_H_ */
