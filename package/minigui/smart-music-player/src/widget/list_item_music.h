/**
 * Project Name: smart-music-player
 * File Name: list_item_music.h
 * Date: 2019-08-28 20:17
 * Author: anruliu
 * Description: List item containing music name and music playback animation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_LIST_ITEM_MUSIC_H_
#define SRC_WIDGET_LIST_ITEM_MUSIC_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define LIST_ITEM_MUSIC "listItemMusic"

typedef struct {
    BITMAP musicPlayingBmp[4];
    PLOGFONT desLogfont;
    char* musicText;
    int clickDisable;
    int playStatus;
} ListItemMusicData;

BOOL RegisterListItemMusic(void);
void UnregisterListItemMusic(void);

#endif /* SRC_WIDGET_LIST_ITEM_MUSIC_H_ */
