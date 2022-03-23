/**
 * Project Name: smart-music-player
 * File Name: headbar_view.h
 * Date: 2019-08-19 19:50
 * Author: anruliu
 * Description: Custom control
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_HEADBAR_VIEW_H_
#define SRC_WIDGET_HEADBAR_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "button_view.h"

#define HEADBAR_VIEW "headBarView"

typedef struct {
    PLOGFONT titleLogfont;
    ButtonData buttonData;
    Uint32 titleIndex; /* Title text index 0~7*/
    Uint32 flag; /* 0 is normal, 1 is music playing */
    BITMAP musicPlayingBmp[4];
} HeadbarData;

HWND HeadbarViewInit(HWND hosting, Uint32 flag, Uint32 index);
BOOL RegisterHeadbarView(void);
void UnregisterHeadbarView(void);

#endif /* SRC_WIDGET_HEADBAR_VIEW_H_ */
