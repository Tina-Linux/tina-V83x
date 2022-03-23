/**
 * Project Name: smart-music-player
 * File Name: activity_setting_time.c
 * Date: 2019-08-28 09:25
 * Author: anruliu
 * Description:
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"

static int ActivitySettingTimeProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static HWND myListHwnd;

    switch (nMessage) {
    case MSG_CREATE: {
        smRect rect;
        HeadbarViewInit(hWnd, 0, 7);

        getResRect(ID_LIST_AREA, &rect);
        MyListData *myListData;
        myListData = MyListDataInit(myListData, 1);
        myListHwnd = CreateWindowEx(MY_LIST_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hWnd, (DWORD) myListData);
        break;
    }
    case MSG_MOUSE_FLING:
        PostMessage(myListHwnd, MSG_MOUSE_FLING, wParam, lParam);
        break;
    case MSG_MOUSE_SCROLL:
        PostMessage(myListHwnd, MSG_MOUSE_SCROLL, wParam, lParam);
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

HWND ActivitySettingTime(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySettingTimeProc;
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
