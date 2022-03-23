/*
 * time_title_view.h
 *
 * Display time control
 * 11:52
 * 11/12 Wednesday
 *
 *  Created on: 2019/8/16
 *      Author: anruliu
 */

#ifndef SRC_WIDGET_TIME_TITLE_VIEW_H_
#define SRC_WIDGET_TIME_TITLE_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define TIME_TITLE_VIEW "timeTitleView"

typedef struct {
    PLOGFONT timeLogfont;
    PLOGFONT timeDataLogfont;
    PLOGFONT volumeLogfont;
    BITMAP volumeBmp;
    Uint32 flag; /* 0 is smart music player window, 1 is screen savers window */
} TimeTitleData;

HWND TimeTitleViewInit(HWND hosting, Uint32 flag);
BOOL RegisterTimeTitleView(void);
void UnregisterTimeTitleView(void);

#endif /* SRC_WIDGET_TIME_TITLE_VIEW_H_ */
