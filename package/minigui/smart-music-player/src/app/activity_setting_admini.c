/**
 * Project Name: smart-music-player
 * File Name: activity_setting_admini.c
 * Date: 2019-08-22 14:07
 * Author: anruliu
 * Description: Administrator settings window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

extern HWND ActivitySettingTime(HWND hosting);
extern HWND ActivitySettingRestore(HWND hosting);

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("list item clike hwnd=%p id=%d add_data=%ld\n", hwnd, id,
            add_data);

    if (nc == STN_CLICKED) {
        switch (id) {
        case 0:
            /* Chinese and English switching */
            if ((int) add_data == 0) {
                languageType = Language_CN;
            } else {
                languageType = Language_EN;
            }
            /* Send global broadcast */
            BroadcastMessage(MSG_LANGUAGE_SWITCH, 0, 0);
            break;
        case 7:
            ActivitySettingTime(GetParent(hwnd));
            break;
        case 9:
            ActivitySettingRestore(GetParent(hwnd));
            break;
        default:
            sm_error("Did not do anything %d\n", id);
            break;
        }
    }
}

static int ActivitySettingAdminiProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static MyListData *myListData;
    static HWND myListHwnd;

    switch (nMessage) {
    case MSG_CREATE: {
        smRect rect;
        HeadbarViewInit(hWnd, 0, 7);

        getResRect(ID_LIST_AREA, &rect);
        myListData = MyListDataInit(myListData, 0);
        myListHwnd = CreateWindowEx(MY_LIST_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hWnd, (DWORD) myListData);
        SetNotificationCallback(myListHwnd, my_notify_proc);
        break;
    }
    case MSG_LANGUAGE_SWITCH: {
        int i;
        for (i = 0; i < 4; i++) {
            ListItemSelectData *listItemSelectData =
                    (ListItemSelectData*) myListData->itemData[i];
            listItemSelectData->desText = getSettingDesText(i + 1);
            if (i == 0) {
                listItemSelectData->selectButtonData->listText[0] =
                        getSettingDesText(18);
                listItemSelectData->selectButtonData->listText[1] =
                        getSettingDesText(19);
            }
        }

        for (i = 4; i < 7; i++) {
            ListItemSwitchData *listItemSwitchData =
                    (ListItemSelectData*) myListData->itemData[i];
            listItemSwitchData->desText = getSettingDesText(i + 1);
        }

        for (i = 7; i < 10; i++) {
            ListItemArrowData *listItemArrowData =
                    (ListItemSelectData*) myListData->itemData[i];
            listItemArrowData->desText = getSettingDesText(i + 1);
        }
        UpdateWindow(hWnd, TRUE);
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

HWND ActivitySettingAdmini(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySettingAdminiProc;
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
