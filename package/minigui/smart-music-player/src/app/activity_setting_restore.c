/**
 * Project Name: smart-music-player
 * File Name: activity_setting_restore.c
 * Date: 2019-08-26 17:01
 * Author: anruliu
 * Description: Restore factory settings window
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

static int ActivitySettingRestoreProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    static ButtonData *buttonData;
    static ButtonData *buttonData1;

    switch (nMessage) {
    case MSG_CREATE: {
        RECT rect;
        HWND textHwnd;
        int buttonTop, buttonTop1, textTop, textTop1, textHeight;
        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            buttonTop = 270;
            buttonTop1 = 346;
            textTop = buttonTop + 4;
            textTop1 = buttonTop1 + 6;
            textHeight = 40;
        } else {
            buttonTop = 180;
            buttonTop1 = 232;
            textTop = buttonTop + 2;
            textTop1 = buttonTop1 + 4;
            textHeight = 30;
        }

        buttonData = NormalButtonDataInit(buttonData, 2);
        buttonData->buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right / 2 - buttonData->bmpNormal.bmWidth / 2, buttonTop,
                buttonData->bmpNormal.bmWidth, buttonData->bmpNormal.bmHeight,
                hWnd, (DWORD) buttonData);
        SetNotificationCallback(buttonData->buttonWnd, my_notify_proc);

        buttonData1 = NormalButtonDataInit(buttonData1, 3);
        buttonData1->buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right / 2 - buttonData1->bmpNormal.bmWidth / 2, buttonTop1,
                buttonData1->bmpNormal.bmWidth, buttonData1->bmpNormal.bmHeight,
                hWnd, (DWORD) buttonData1);
        SetNotificationCallback(buttonData1->buttonWnd, my_notify_proc);

        textHwnd = CreateWindowEx(CTRL_STATIC, getButtonDesText(2),
        WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
        WS_EX_TRANSPARENT, 0, 0, textTop, rect.right, textHeight, hWnd, 0);
        SetWindowFont(textHwnd, getLogFont(ID_FONT_TIMES_24));

        textHwnd = CreateWindowEx(CTRL_STATIC, getButtonDesText(3),
        WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
        WS_EX_TRANSPARENT, 0, 0, textTop1, rect.right, textHeight, hWnd, 0);
        SetWindowFont(textHwnd, getLogFont(ID_FONT_TIMES_24));
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        RECT rect;
        SIZE size;
        int textTop, textTop1, lineTop, lineTop1;
        GetClientRect(hWnd, &rect);

        /* Draw two intersecting rectangles, x, y are 10 apart */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 27, 27, 27));
        FillBox(hdc, 34, 24, rect.right - 68, rect.bottom - 48);
        FillBox(hdc, 24, 34, rect.right - 48, rect.bottom - 68);

        /* Draw a circle of four vertices */
        FillCircle(hdc, 34, 34, 10);
        FillCircle(hdc, rect.right - 34, 34, 10);
        FillCircle(hdc, 34, rect.bottom - 35, 10);
        FillCircle(hdc, rect.right - 34, rect.bottom - 35, 10);

        if (rect.right > 240) {
            textTop = 56;
            textTop1 = 140;
            lineTop = 108;
            lineTop1 = 216;
        } else {
            textTop = 37;
            textTop1 = 100;
            lineTop = 78;
            lineTop1 = 156;
        }

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, 38, lineTop, rect.right - 38 * 2, 2);
        FillBox(hdc, 38, lineTop1, rect.right - 38 * 2, 2);

        /* Draw prompt text */
        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, getLogFont(ID_FONT_TIMES_18));
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        GetTextExtent(hdc, getSettingDesText(16), -1, &size);
        TextOut(hdc, rect.right / 2 - size.cx / 2, textTop,
                getSettingDesText(16));

        SelectFont(hdc, getLogFont(ID_FONT_TIMES_24));
        GetTextExtent(hdc, getSettingDesText(17), -1, &size);
        TextOut(hdc, rect.right / 2 - size.cx / 2, textTop1,
                getSettingDesText(17));

        EndPaint(hWnd, hdc);
        return 0;
    }
    case MSG_DESTROY:
        sm_debug("MSG_DESTROY\n");
        unloadBitMap(&buttonData->bmpNormal);
        unloadBitMap(&buttonData->bmpSelect);
        if (buttonData) {
            free(buttonData);
            buttonData = NULL;
        }
        unloadBitMap(&buttonData1->bmpNormal);
        unloadBitMap(&buttonData1->bmpSelect);
        if (buttonData1) {
            free(buttonData1);
            buttonData1 = NULL;
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

HWND ActivitySettingRestore(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivitySettingRestoreProc;
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

