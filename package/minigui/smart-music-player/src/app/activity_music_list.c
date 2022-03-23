/**
 * Project Name: smart-music-player
 * File Name: activity_music_list.c
 * Date: 2019-08-21 09:07
 * Author: anruliu
 * Description: Local music list window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

static HWND playMusicHwnd;

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    MyListData *myListData = NULL;
    myListData = (MyListData*) GetWindowAdditionalData(hwnd);

    sm_debug("list item clike hwnd=%p id=%d add_data=%ld\n", hwnd, id,
            add_data);

    if (nc == STN_CLICKED) {
        if (playMusicHwnd != HWND_INVALID
                && playMusicHwnd != myListData->itemHwnd[id]) {
            /* Stop the last play music animation,
             * guarantee that only one animation is playing */
            PostMessage(playMusicHwnd, MSG_MUSIC_PLAY_STOP, 0, 0);
        }
        playMusicHwnd = myListData->itemHwnd[id];
    }
}

#define __ID_TIMER_SLIDER 100
static int ActivityMusicListProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static HWND myListHwnd;
    static int hintShow;

    switch (nMessage) {
    case MSG_CREATE: {
        hintShow = 1;
        playMusicHwnd = HWND_INVALID;
        HeadbarViewInit(hWnd, 0, 1);
        SetTimer(hWnd, __ID_TIMER_SLIDER, 10);
        break;
    }
    case MSG_TIMER: {
        smRect rect;
        MyListData *myListData;
        myListData = MyListDataInit(myListData, 2);

        getResRect(ID_LIST_AREA, &rect);
        myListHwnd = CreateWindowEx(MY_LIST_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hWnd, (DWORD) myListData);
        SetNotificationCallback(myListHwnd, my_notify_proc);
        hintShow = 0;
        KillTimer(hWnd, __ID_TIMER_SLIDER);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        if (hintShow) {
            SIZE size;
            RECT rect;
            GetClientRect(hWnd, &rect);

            /* Draw title text */
            SetBkMode(hdc, BM_TRANSPARENT);
            SelectFont(hdc, getLogFont(ID_FONT_TIMES_24));
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 138, 0));
            GetTextExtent(hdc, getHintText(0), -1, &size);
            TextOut(hdc, rect.right / 2 - size.cx / 2,
                    rect.bottom / 2 - size.cy / 2, getHintText(0));
        }
        EndPaint(hWnd, hdc);
        return 0;
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

HWND ActivityMusicList(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivityMusicListProc;
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
