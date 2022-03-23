/*
 * activity_screensavers.c
 *
 * Screen saver interface, only show time,
 * click anywhere to enter the main window
 *
 *  Created on: 2019/8/16
 *      Author: anruliu
 */

#include "resource.h"
#include "datafactory.h"

static int ActivityScreensaversProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    switch (nMessage) {
    case MSG_CREATE: {
        TimeTitleViewInit(hWnd, 1);
        break;
    }
    case MSG_LBUTTONUP:
        PostMessage(hWnd, MSG_CLOSE, 0, 0);
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

HWND ActivityScreensavers(HWND hosting) {

    MSG Msg;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , SMART_MUSIC_PLAYER , 0 , 0);
#endif

    smRect rect;
    getResRect(ID_SCREEN, &rect);

    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_TOPMOST;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ActivityScreensaversProc;
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

