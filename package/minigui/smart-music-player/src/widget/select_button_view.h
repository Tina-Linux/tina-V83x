/**
 * Project Name: smart-music-player
 * File Name: select_button_view.h
 * Date: 2019-08-22 15:43
 * Author: anruliu
 * Description: Select button, including horizontal and vertical
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_SELECT_BUTTON_VIEW_H_
#define SRC_WIDGET_SELECT_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define SELECT_BUTTON_VIEW "selectButtonView"
#define SELECT_BUTTON_HORIZONTAL 0
#define SELECT_BUTTON_VERTICAL 1

typedef struct {
    PLOGFONT textLogfont;
    BITMAP bgBmp;
    ButtonData buttonData[2];
    Uint32 displayMode; /* 0 is horizontal, 1 is vertical */
    char* listText[12];
    Uint32 listSize; /* Up to 12 */
    Uint32 curIndex;
} SelectButtonData;

BOOL RegisterSelectButtonView(void);
void UnregisterSelectButtonView(void);

#endif /* SRC_WIDGET_SELECT_BUTTON_VIEW_H_ */
