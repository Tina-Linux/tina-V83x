/**
 * Project Name: smart-music-player
 * File Name: list_item_select.h
 * Date: 2019-08-26 16:31
 * Author: anruliu
 * Description: List item containing select button
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_LIST_ITEM_SELECT_H_
#define SRC_WIDGET_LIST_ITEM_SELECT_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "select_button_view.h"

#define LIST_ITEM_SELECT "listItemSelect"

typedef struct {
    SelectButtonData *selectButtonData;
    PLOGFONT desLogfont;
    char* desText;
} ListItemSelectData;

BOOL RegisterListItemSelect(void);
void UnregisterListItemSelect(void);

#endif /* SRC_WIDGET_LIST_ITEM_SELECT_H_ */
