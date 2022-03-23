/**
 * Project Name: smart-music-player
 * File Name: desk_button_view.h
 * Date: 2019-08-17 11:47
 * Author: anruliu
 * Description: Custom control
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_DESK_BUTTON_VIEW_H_
#define SRC_WIDGET_DESK_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define DESK_BUTTON_VIEW "deskButtonView"

typedef struct {
    ButtonData buttonData[6];
    PLOGFONT desLogfont;
    Uint32 flag; /* 0 is smart music player window, 1 is scene window */
} DeskButotnData;

HWND DeskButotnViewInit(HWND hosting, Uint32 flag);
BOOL RegisterDeskButotnView(void);
void UnregisterDeskButotnView(void);

#endif /* SRC_WIDGET_DESK_BUTTON_VIEW_H_ */
