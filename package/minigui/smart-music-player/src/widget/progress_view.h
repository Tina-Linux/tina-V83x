/**
 * Project Name: smart-music-player
 * File Name: progress_view.h
 * Date: 2019-08-21 17:35
 * Author: anruliu
 * Description: Music playback progress bar custom control, including time display
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_PROGRESS_VIEW_H_
#define SRC_WIDGET_PROGRESS_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define PROGRESS_VIEW "progressView"

typedef struct {
    int currentProgress;
} ProgressData;

HWND ProgressViewInit(HWND hosting);
BOOL RegisterProgressView(void);
void UnregisterProgressView(void);

#endif /* SRC_WIDGET_PROGRESS_VIEW_H_ */
