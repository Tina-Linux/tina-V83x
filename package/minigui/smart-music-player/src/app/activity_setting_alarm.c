/**
 * Project Name: smart-music-player
 * File Name: activity_setting_alarm.c
 * Date: 2019-08-21 09:21
 * Author: anruliu
 * Description: Alarm setting window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"

extern HWND ActivitySettingAlarmDetailed(HWND hosting, Uint32 index);

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("list item switch button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        ActivitySettingAlarmDetailed(GetParent(hwnd), id);
    }
}

static int ActivitySettingAlarmProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    switch (nMessage) {
    case MSG_CREATE: {
        HeadbarViewInit(hWnd, 0, 4);
        RECT rect;
        int hwndHeight = 68;
        int hwndTop = 50;
        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            hwndHeight = 88;
            hwndTop = 72;
        }

        ListItemSwitchData *listItemSwitchData = NULL;
        listItemSwitchData = ListItemSwitchDataInit(listItemSwitchData, 0, 0);
        ListItemSwitchData *listItemSwitchData1 = NULL;
        listItemSwitchData1 = ListItemSwitchDataInit(listItemSwitchData1, 1, 1);
        ListItemSwitchData *listItemSwitchData2 = NULL;
        listItemSwitchData2 = ListItemSwitchDataInit(listItemSwitchData2, 0, 2);

        HWND listItemSwitchHwnd = HWND_INVALID;
        listItemSwitchHwnd = CreateWindowEx(LIST_ITEM_SWITCH, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 0, hwndTop,
                rect.right, hwndHeight, hWnd, (DWORD) listItemSwitchData);
        SetNotificationCallback(listItemSwitchHwnd, my_notify_proc);

        listItemSwitchHwnd = CreateWindowEx(LIST_ITEM_SWITCH, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 0,
                hwndTop + hwndHeight, rect.right, hwndHeight, hWnd,
                (DWORD) listItemSwitchData1);
        SetNotificationCallback(listItemSwitchHwnd, my_notify_proc);

        listItemSwitchHwnd = CreateWindowEx(LIST_ITEM_SWITCH, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 0,
                hwndTop + hwndHeight + hwndHeight, rect.right, hwndHeight, hWnd,
                (DWORD) listItemSwitchData2);
        SetNotificationCallback(listItemSwitchHwnd, my_notify_proc);
        break;
    }
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

HWND ActivitySettingAlarm(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySettingAlarmProc;
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
