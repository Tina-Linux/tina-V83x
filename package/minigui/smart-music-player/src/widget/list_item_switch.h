/**
 * Project Name: smart-music-player
 * File Name: list_item_switch.h
 * Date: 2019-08-23 14:30
 * Author: anruliu
 * Description: List item containing switch button
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_LIST_ITEM_SWITCH_H_
#define SRC_WIDGET_LIST_ITEM_SWITCH_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "switch_button_view.h"

#define LIST_ITEM_SWITCH "listItemSwitch"

typedef struct {
    SwitchButtonData *switchButtonData;
    PLOGFONT desLogfont;
    char* desText;
    int clickDisable;
} ListItemSwitchData;

BOOL RegisterListItemSwitch(void);
void UnregisterListItemSwitch(void);

#endif /* SRC_WIDGET_LIST_ITEM_SWITCH_H_ */
