/**
 * Project Name: smart-music-player
 * File Name: control_button_view.h
 * Date: 2019-08-21 09:50
 * Author: anruliu
 * Description: Custom control, Contains three buttons
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_CONTROL_BUTTON_VIEW_H_
#define SRC_WIDGET_CONTROL_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define CONTROL_BUTTON_VIEW "controlButtonView"

typedef struct {
    ButtonData buttonData[3];
    Uint32 flag; /* 0 is music paly, 1 is bluetooth music, 2 is music aux */
} ControlButotnData;

HWND ControlButotnViewInit(HWND hosting, Uint32 flag);
BOOL RegisterControlButotnView(void);
void UnregisterControlButotnView(void);

#endif /* SRC_WIDGET_CONTROL_BUTTON_VIEW_H_ */
