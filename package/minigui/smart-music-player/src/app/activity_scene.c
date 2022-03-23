/**
 * Project Name: smart-music-player
 * File Name: activity_scene.c
 * Date: 2019-08-19 15:13
 * Author: anruliu
 * Description: Scene window, including going home, working, dining, sleeping
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

static int ActivitySceneProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static PLOGFONT logfontScene;
    static PLOGFONT logfontDes;

    switch (nMessage) {
    case MSG_CREATE: {
        logfontScene = getLogFont(ID_FONT_TIMES_40);
        logfontDes = getLogFont(ID_FONT_TIMES_18);
        DeskButotnViewInit(hWnd, 1);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        SIZE size;
        RECT rect;
        GetClientRect(hWnd, &rect);

        /* Draw title text */
        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, logfontScene);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 66, 239, 255));
        GetTextExtent(hdc, getSceneText(0), -1, &size);
        if (rect.right > 240) {
            TextOut(hdc, 36, 44, getSceneText(0));
        } else {
            TextOut(hdc, 26, 26, getSceneText(0));
        }

        SelectFont(hdc, logfontDes);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 152, 152, 152));
        if (rect.right > 240) {
            TextOut(hdc, 36, size.cy - 4 + 44, getSceneText(1));
        } else {
            TextOut(hdc, 26, size.cy - 4 + 26, getSceneText(1));
        }

        EndPaint(hWnd, hdc);
        return 0;
    }
    case MSG_LBUTTONUP: {
        /* Click on the time control to jump to the scene window */
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (rect.right > 240) {
            if (HIWORD(lParam) < 146) {
                PostMessage(hWnd, MSG_CLOSE, 0, 0);
            }
        } else {
            if (HIWORD(lParam) < 126) {
                PostMessage(hWnd, MSG_CLOSE, 0, 0);
            }
        }
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

HWND ActivityScene(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySceneProc;
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
