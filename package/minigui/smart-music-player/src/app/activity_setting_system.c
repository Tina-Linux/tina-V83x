/**
 * Project Name: smart-music-player
 * File Name: activity_setting_system.c
 * Date: 2019-08-21 09:22
 * Author: anruliu
 * Description: System settings window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"

extern HWND ActivitySettingPwd(HWND hosting);
extern HWND ActivitySettingAdmini(HWND hosting);

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("setting button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        switch (id) {
        case 0:
            ActivitySettingPwd(GetParent(hwnd));
            break;
        case 1:
            ActivitySettingAdmini(GetParent(hwnd));
            break;
        default:
            sm_error("Do not open any windows id is %d\n", id);
            break;
        }
    }
}

static int ActivitySettingSystemProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static SettingButtonData *settingButtonData;
    static SettingButtonData *settingButtonData1;

    switch (nMessage) {
    case MSG_CREATE: {
        HeadbarViewInit(hWnd, 0, 5);
        RECT rect;
        int hwndHeight = 68;
        int hwndTop = 50;
        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            hwndHeight = 88;
            hwndTop = 72;
        }

        HWND settingButtonHwnd = HWND_INVALID;
        settingButtonData = SettingButtonDataInit(settingButtonData, 0);
        settingButtonData1 = SettingButtonDataInit(settingButtonData1, 1);

        settingButtonHwnd = CreateWindowEx(SETTING_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 0, hwndTop,
                rect.right, hwndHeight, hWnd, (DWORD) settingButtonData);
        SetNotificationCallback(settingButtonHwnd, my_notify_proc);
        settingButtonHwnd = CreateWindowEx(SETTING_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 0,
                hwndTop + hwndHeight, rect.right, hwndHeight, hWnd,
                (DWORD) settingButtonData1);
        SetNotificationCallback(settingButtonHwnd, my_notify_proc);
        break;
    }
    case MSG_LANGUAGE_SWITCH:
        settingButtonData->settingText = getHeadbarDesText(6);
        settingButtonData1->settingText = getHeadbarDesText(7);
        break;
    case MSG_DESTROY:
        sm_debug("MSG_DESTROY\n");
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        MainWindowThreadCleanup(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

HWND ActivitySettingSystem(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySettingSystemProc;
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
