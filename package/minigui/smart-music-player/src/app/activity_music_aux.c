/**
 * Project Name: smart-music-player
 * File Name: activity_music_aux.c
 * Date: 2019-08-21 09:17
 * Author: anruliu
 * Description: Audio input interface, can output audio of electronic audio equipment
 *              including mp3 (general headphone jack), can output music in these
 *              devices through the sound of the car
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"

static int ActivityMusicAuxProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static BITMAP bmpBg;

    switch (nMessage) {
    case MSG_CREATE: {
        HeadbarViewInit(hWnd, 0, 3);
        ControlButotnViewInit(hWnd, 2);

        setCurrentIconValue(ID_MUSIC_AUX_BMP, 0);
        getResBmp(ID_MUSIC_AUX_BMP, BMPTYPE_BASE, &bmpBg);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);

        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 0, 151, 0));
        if (rect.right > 240) {
            FillBox(hdc, 0, 0, rect.right, 240);
            FillBoxWithBitmap(hdc, rect.right / 2 - bmpBg.bmWidth / 2, 86, 0, 0,
                    &bmpBg);
        } else {
            FillBox(hdc, 0, 0, rect.right, 160);
            FillBoxWithBitmap(hdc, rect.right / 2 - bmpBg.bmWidth / 2, 55, 0, 0,
                    &bmpBg);
        }

        EndPaint(hWnd, hdc);
        return 0;
    }
    case MSG_DESTROY:
        sm_debug("MSG_DESTROY\n");
        unloadBitMap(&bmpBg);
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        MainWindowThreadCleanup(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

HWND ActivityMusicAux(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivityMusicAuxProc;
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
