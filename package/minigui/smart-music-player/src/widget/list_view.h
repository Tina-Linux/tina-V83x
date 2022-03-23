/**
 * Project Name: smart-music-player
 * File Name: list_view.h
 * Date: 2019-08-27 13:44
 * Author: anruliu
 * Description: List custom control
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_LIST_VIEW_H_
#define SRC_WIDGET_LIST_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define MY_LIST_VIEW "myListView"
#define MY_LIST_SIZE 64

typedef struct {
    char* itemType[MY_LIST_SIZE];
    void* itemData[MY_LIST_SIZE];
    HWND itemHwnd[MY_LIST_SIZE];
    Uint32 itemHeight; /* Height of each item */
    Uint32 listSize; /* Up to MY_LIST_SIZE */
    int slidingDistance; /* Record the distance of the slide */
    int slidingMaxDistance; /* Record the maximum distance that can slide up */
} MyListData;

BOOL RegisterMyListView(void);
void UnregisterMyListView(void);

#endif /* SRC_WIDGET_LIST_VIEW_H_ */
