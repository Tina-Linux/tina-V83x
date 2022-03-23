/**
 * Project Name: smart-music-player
 * File Name: setting_button_view.h
 * Date: 2019-08-22 10:09
 * Author: anruliu
 * Description: Set custom buttons, including the image on the left and the text on the right
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */
#ifndef SRC_WIDGET_SETTING_BUTTON_VIEW_H_
#define SRC_WIDGET_SETTING_BUTTON_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define SETTING_BUTTON_VIEW "settingButtonView"

typedef struct {
    PLOGFONT settingLogfont;
    BITMAP settingBmp;
    char* settingText;
} SettingButtonData;

BOOL RegisterSettingButtonView(void);
void UnregisterSettingButtonView(void);

#endif /* SRC_WIDGET_SETTING_BUTTON_VIEW_H_ */
