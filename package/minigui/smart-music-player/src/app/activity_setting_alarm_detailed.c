/**
 * Project Name: smart-music-player
 * File Name: activity_setting_alarm_detailed.c
 * Date: 2019-08-23 19:36
 * Author: anruliu
 * Description: Alarm clock settings detailed interface, you can choose music
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("setting alarm detailed button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
    }
}

static int ActivitySettingAlarmDetailedProc(HWND hWnd, int nMessage,
        WPARAM wParam, LPARAM lParam) {

    int index = 0;
    index = (int) GetWindowAdditionalData(hWnd);

    static ButtonData *buttonData;

    switch (nMessage) {
    case MSG_CREATE: {
        HWND textHwnd;
        RECT rect;
        int buttonTop, textTop, textHeight;
        HeadbarViewInit(hWnd, 0, 4);
        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            buttonTop = 380;
            textTop = buttonTop + 6;
            textHeight = 40;
        } else {
            buttonTop = 254;
            textTop = buttonTop + 4;
            textHeight = 30;
        }

        buttonData = NormalButtonDataInit(buttonData, 0);
        buttonData->buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right / 2 - buttonData->bmpNormal.bmWidth / 2, buttonTop,
                buttonData->bmpNormal.bmWidth, buttonData->bmpNormal.bmHeight,
                hWnd, (DWORD) buttonData);
        SetNotificationCallback(buttonData->buttonWnd, my_notify_proc);

        textHwnd = CreateWindowEx(CTRL_STATIC, getButtonDesText(0),
        WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
        WS_EX_TRANSPARENT, 0, 0, textTop, rect.right, textHeight, hWnd, 0);
        SetWindowFont(textHwnd, getLogFont(ID_FONT_TIMES_24));
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        RECT rect;
        int headHeight, textTop, alarmTop, desTop1, desTop2, itemHeight;
        GetClientRect(hWnd, &rect);

        if (rect.right > 240) {
            headHeight = 72;
            textTop = 90;
            alarmTop = textTop + 106;
            desTop1 = textTop + 40;
            desTop2 = desTop1 + 106;
            itemHeight = 108;
        } else {
            headHeight = 50;
            textTop = 58;
            alarmTop = textTop + 80;
            desTop1 = textTop + 36;
            desTop2 = desTop1 + 80;
            itemHeight = 78;
        }

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, getLogFont(ID_FONT_TIMES_24));
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        if (index == 0) {
            TextOut(hdc, 14, textTop, "08:00");
        } else if (index == 1) {
            TextOut(hdc, 14, textTop, "09:00");
        } else if (index == 2) {
            TextOut(hdc, 14, textTop, "10:00");
        }
        TextOut(hdc, 14, alarmTop, "alarm.mp3");

        SelectFont(hdc, getLogFont(ID_FONT_TIMES_14));
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 152, 152, 152));
        TextOut(hdc, 14, desTop1, getAlarmText(0));
        TextOut(hdc, 14, desTop2, getAlarmText(1));

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, 14, itemHeight + headHeight - 2, rect.right - 28, 2);
        FillBox(hdc, 14, 2 * itemHeight + headHeight - 2, rect.right - 28, 2);
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
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        MainWindowThreadCleanup(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

/*
 * @param index 0 08:00, 1 is 09:00, 2 is 10:00
 */
HWND ActivitySettingAlarmDetailed(HWND hosting, Uint32 index) {

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
    CreateInfo.MainWindowProc = ActivitySettingAlarmDetailedProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = rect.w;
    CreateInfo.by = rect.h;
    CreateInfo.iBkColor = PIXEL_black;
    CreateInfo.dwAddData = (DWORD) index;
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
