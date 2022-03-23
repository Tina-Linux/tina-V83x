/**
 * Project Name: smart-music-player
 * File Name: list_item_music.c
 * Date: 2019-08-28 20:19
 * Author: anruliu
 * Description: List item containing music name and music playback animation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "list_item_music.h"
#include "resource.h"
#include <sys/time.h>
#include <time.h>

static int ListItemArrowProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ListItemMusicData *listItemMusicData = NULL;
    listItemMusicData = (ListItemMusicData*) GetWindowAdditionalData(hwnd);

    static BOOL isUpClick;
    static BOOL doubleClick;
    static long oldtime;
    struct timeval tv;
    static int musicPlayingIndex;

    switch (message) {
    case MSG_CREATE: {
        isUpClick = FALSE;
        doubleClick = FALSE;
        gettimeofday(&tv, NULL);
        oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        musicPlayingIndex = 0;
        break;
    }
    case MSG_TIMER: {
        musicPlayingIndex = (musicPlayingIndex < 3) ? ++musicPlayingIndex : 0;
        UpdateWindow(hwnd, FALSE);
        break;
    }
    case MSG_MUSIC_PLAY_STOP:
        listItemMusicData->playStatus = 0;
        if (IsTimerInstalled(hwnd, ID_TIMER_MUSIC_PLAYING)) {
            KillTimer(hwnd, ID_TIMER_MUSIC_PLAYING);
        }
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_LBUTTONDBLCLK:
        if (listItemMusicData->clickDisable)
            break;
        doubleClick = TRUE;
        break;
    case MSG_LBUTTONDOWN:
        if (listItemMusicData->clickDisable)
            break;
        doubleClick = FALSE;
        isUpClick = FALSE;
        break;
    case MSG_LBUTTONUP:
        /* If the list is sliding, it won't be executed again because
         * it was intercepted by the list view.*/
        if (listItemMusicData->clickDisable)
            break;
        gettimeofday(&tv, NULL);
        long newtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        int lentime = newtime - oldtime;
        if (lentime < 300)
            doubleClick = TRUE;
        oldtime = newtime;
        isUpClick = TRUE;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        SIZE size;
        int margin;
        GetClientRect(hwnd, &rect);
        margin = 14;

        /* Fill background */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 0, 0, 0));
        FillBox(hdc, 0, 0, rect.right, rect.bottom);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, listItemMusicData->desLogfont);
        if (listItemMusicData->playStatus)
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 138, 0));
        else
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));

        GetTextExtent(hdc, listItemMusicData->musicText, -1, &size);
        TextOut(hdc, margin, rect.bottom / 2 - size.cy / 2,
                listItemMusicData->musicText);

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, margin, rect.bottom - 2, rect.right - 28, 2);

        /* Drawing arrows */
        if (listItemMusicData->playStatus)
            FillBoxWithBitmap(hdc,
                    rect.right - listItemMusicData->musicPlayingBmp[0].bmWidth
                            - margin,
                    rect.bottom / 2
                            - listItemMusicData->musicPlayingBmp[0].bmHeight
                                    / 2, 0, 0,
                    &listItemMusicData->musicPlayingBmp[musicPlayingIndex]);

        EndPaint(hwnd, hdc);

        /* Notify the parent window button to be clicked */
        if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick) {
            isUpClick = FALSE;
            NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED, 0);

            /* Switch button status */
            if (listItemMusicData->playStatus) {
                listItemMusicData->playStatus = 0;
                KillTimer(hwnd, ID_TIMER_MUSIC_PLAYING);

                UpdateWindow(hwnd, FALSE);
            } else {
                listItemMusicData->playStatus = 1;
                SetTimer(hwnd, ID_TIMER_MUSIC_PLAYING, 10);
            }
        }
        return 0;
    }
    case MSG_DESTROY: {
        if (IsTimerInstalled(hwnd, ID_TIMER_MUSIC_PLAYING)) {
            KillTimer(hwnd, ID_TIMER_MUSIC_PLAYING);
        }
        unloadBitMap(&listItemMusicData->musicPlayingBmp[0]);
        unloadBitMap(&listItemMusicData->musicPlayingBmp[1]);
        unloadBitMap(&listItemMusicData->musicPlayingBmp[2]);
        unloadBitMap(&listItemMusicData->musicPlayingBmp[3]);
        if (listItemMusicData) {
            free(listItemMusicData);
            listItemMusicData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterListItemMusic() {
    WNDCLASS MyClass;
    MyClass.spClassName = LIST_ITEM_MUSIC;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ListItemArrowProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterListItemMusic(void) {
    UnregisterWindowClass(LIST_ITEM_MUSIC);
}
