/**
 * Project Name: smart-music-player
 * File Name: activity_setting_pwd.c
 * Date: 2019-08-22 14:04
 * Author: anruliu
 * Description: System password setting window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("setting pwd button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
    }
}

static int ActivitySettingPwdProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static ButtonData *buttonData;

    switch (nMessage) {
    case MSG_CREATE: {
        HWND textHwnd;
        RECT rect;
        int textTop, textTop1, textHeight, selectTop, selectSpacing, buttonTop;
        HeadbarViewInit(hWnd, 0, 6);

        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            textTop = 84;
            selectTop = 160;
            buttonTop = 380;
            textTop1 = 384;
            textHeight = 40;
            selectSpacing = 76;
        } else {
            textTop = 54;
            selectTop = 98;
            buttonTop = 254;
            textTop1 = 258;
            textHeight = 30;
            selectSpacing = 50;
        }

        HWND selectButtonHwnd = HWND_INVALID;
        SelectButtonData *selectButtonData[4] = { NULL };
        int i;
        for (i = 0; i < 4; i++) {
            selectButtonData[i] = SelectButtonDataInit(selectButtonData[i],
            SELECT_BUTTON_VERTICAL, 0);
            selectButtonHwnd = CreateWindowEx(SELECT_BUTTON_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                    20 + i * selectSpacing, selectTop,
                    selectButtonData[i]->bgBmp.bmWidth,
                    selectButtonData[i]->bgBmp.bmHeight, hWnd,
                    (DWORD) selectButtonData[i]);
        }

        buttonData = NormalButtonDataInit(buttonData, 1);
        buttonData->buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right / 2 - buttonData->bmpNormal.bmWidth / 2, buttonTop,
                buttonData->bmpNormal.bmWidth, buttonData->bmpNormal.bmHeight,
                hWnd, (DWORD) buttonData);
        SetNotificationCallback(buttonData->buttonWnd, my_notify_proc);

        textHwnd = CreateWindowEx(CTRL_STATIC, getSettingDesText(0),
        WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
        WS_EX_TRANSPARENT, 0, 0, textTop, rect.right, textHeight, hWnd, 0);
        SetWindowFont(textHwnd, getLogFont(ID_FONT_TIMES_18));

        textHwnd = CreateWindowEx(CTRL_STATIC, getButtonDesText(1),
        WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
        WS_EX_TRANSPARENT, 0, 0, textTop1, rect.right, textHeight, hWnd, 0);
        SetWindowFont(textHwnd, getLogFont(ID_FONT_TIMES_24));
        break;
    }
    case MSG_DESTROY:
        sm_debug("MSG_DESTROY\n");
        unloadBitMap(&buttonData->bmpNormal);
        unloadBitMap(&buttonData->bmpSelect);
        if (buttonData) {
            free(buttonData);
            buttonData = NULL;
        }
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        MainWindowThreadCleanup(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

HWND ActivitySettingPwd(HWND hosting) {

    MSG Msg;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , SMART_MUSIC_PLAYER , 0 , 0);
#endif

    smRect rect;
    getResRect(ID_SCREEN, &rect);

    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ActivitySettingPwdProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = rect.w;
    CreateInfo.by = rect.h;
    CreateInfo.iBkColor = PIXEL_black;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = hosting;

    HWND hMainWnd = CreateMainWindow(&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return HWND_INVALID;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    return hMainWnd;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
