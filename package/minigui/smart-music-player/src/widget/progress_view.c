/**
 * Project Name: smart-music-player
 * File Name: progress_view.c
 * Date: 2019-08-21 17:43
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "progress_view.h"
#include "datafactory.h"
#include "resource.h"

static int ProgressViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ProgressData *progressData = NULL;
    progressData = (ProgressData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE:
        SetTimer(hwnd, ID_TIMER_PROGRESS_UPDATE, 50);
        break;
    case MSG_TIMER: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        progressData->currentProgress =
                (progressData->currentProgress <= rect.right) ?
                        ++progressData->currentProgress : 0;
        UpdateWindow(hwnd, TRUE);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        GetClientRect(hwnd, &rect);

        /* Draw the left progress bar */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 250, 138, 3));
        FillBox(hdc, 0, 0, progressData->currentProgress, rect.bottom);

        /* Draw the right progress bar */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 51, 51, 51));
        FillBox(hdc, progressData->currentProgress, 0,
                rect.right - progressData->currentProgress, rect.bottom);

        /* Draw a smooth angle on the left side of the progress bar */
        if (progressData->currentProgress != 0) {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 230, 126, 4));
        } else {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 46, 46, 46));
        }
        FillBox(hdc, 1, 0, 1, 1);
        FillBox(hdc, 0, 1, 1, 1);

        FillBox(hdc, 0, rect.bottom - 2, 1, 1);
        FillBox(hdc, 1, rect.bottom - 1, 1, 1);

        if (progressData->currentProgress != 0) {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 73, 40, 1));
        } else {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 15, 15, 15));
        }

        FillBox(hdc, 0, 0, 1, 1);
        FillBox(hdc, 0, rect.bottom - 1, 1, 1);

        /* Draw a smooth angle on the right side of the progress bar */
        if (progressData->currentProgress != rect.right) {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 46, 46, 46));
        } else {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 230, 126, 4));
        }
        FillBox(hdc, rect.right - 2, 0, 1, 1);
        FillBox(hdc, rect.right - 1, 1, 1, 1);

        FillBox(hdc, rect.right - 1, rect.bottom - 2, 1, 1);
        FillBox(hdc, rect.right - 2, rect.bottom - 1, 1, 1);

        if (progressData->currentProgress != rect.right) {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 15, 15, 15));
        } else {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 43, 40, 1));
        }
        FillBox(hdc, rect.right - 1, 0, 1, 1);
        FillBox(hdc, rect.right - 1, rect.bottom - 1, 1, 1);

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        KillTimer(hwnd, ID_TIMER_PROGRESS_UPDATE);
        if (progressData) {
            free(progressData);
            progressData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND ProgressViewInit(HWND hosting) {
    smRect rect;
    HWND progressHwnd = HWND_INVALID;
    ProgressData *progressData = NULL;
    progressData = ProgressDataInit(progressData);

    getResRect(ID_PROGRESS_AREA, &rect);
    progressHwnd = CreateWindowEx(PROGRESS_VIEW, "",
    WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
            rect.w, rect.h, hosting, (DWORD) progressData);

    return progressHwnd;
}

BOOL RegisterProgressView() {
    WNDCLASS MyClass;
    MyClass.spClassName = PROGRESS_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ProgressViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterProgressView(void) {
    UnregisterWindowClass(PROGRESS_VIEW);
}
