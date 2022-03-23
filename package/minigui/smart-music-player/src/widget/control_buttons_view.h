/**
 * Project Name: smart-music-player
 * File Name: control_buttons_view.h
 * Date: 2019-08-21 09:50
 * Author: anruliu
 * Description: Custom control, Contains Four buttons
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_CONTROL_BUTTONS_VIEW_H_
#define SRC_WIDGET_CONTROL_BUTTONS_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define CONTROL_BUTTONS_VIEW "controlButtonsView"

typedef struct {
    ButtonData buttonData[3];
    BITMAP playModeBmp[4];
} ControlButotnsData;

HWND ControlButotnsViewInit(HWND hosting);
BOOL RegisterControlButotnsView(void);
void UnregisterControlButotnsView(void);

#endif /* SRC_WIDGET_CONTROL_BUTTONS_VIEW_H_ */
