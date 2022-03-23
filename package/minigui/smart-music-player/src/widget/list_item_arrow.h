/**
 * Project Name: smart-music-player
 * File Name: list_item_arrow.h
 * Date: 2019-08-27 16:48
 * Author: anruliu
 * Description: List item containing arrow
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_LIST_ITEM_ARROW_H_
#define SRC_WIDGET_LIST_ITEM_ARROW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define LIST_ITEM_ARROW "listItemArrow"

typedef struct {
    BITMAP arrowBmp;
    PLOGFONT desLogfont;
    char* desText;
    int clickDisable;
} ListItemArrowData;

BOOL RegisterListItemArrow(void);
void UnregisterListItemArrow(void);

#endif /* SRC_WIDGET_LIST_ITEM_ARROW_H_ */
