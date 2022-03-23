/**
 * Project Name: smart-music-player
 * File Name: switch_button_view.h
 * Date: 2019-08-23 10:06
 * Author: anruliu
 * Description: Switch button custom control
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_SWITCH_BUTTON_VIEW_H_
#define SRC_WIDGET_SWITCH_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define SWITCH_BUTTON_VIEW "switchButtonView"

typedef struct {
    BITMAP bgBmp;
    BITMAP onBmp;
    BITMAP offBmp;
    Uint32 buttonStatus; /* 0 is off, 1 is on */
    int slidingDistance; /* Button sliding distance */
} SwitchButtonData;

BOOL RegisterSwitchButtonView(void);
void UnregisterSwitchButtonView(void);

#endif /* SRC_WIDGET_SWITCH_BUTTON_VIEW_H_ */
