/*
 * time_title_view.c
 *
 * Display time control
 * 11:52
 * 11/12 Wednesday
 *
 *  Created on: 2019/8/16
 *      Author: anruliu
 */

#include "time_title_view.h"
#include "resource.h"
#include "globalization.h"
#include "datafactory.h"

static int TimeTitleViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    TimeTitleData *timeTitleData = NULL;
    timeTitleData = (TimeTitleData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE:
        SetTimer(hwnd, ID_TIMER_UPDATE_TIME, 100 * 60);
        break;
    case MSG_TIMER:
        UpdateWindow(hwnd, TRUE);
        break;
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        time_t t;
        struct tm * lt;
        SIZE size;
        char str[32];
        GetClientRect(hwnd, &rect);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, timeTitleData->timeLogfont);

        /* Obtained Unix timestamp */
        time(&t);
        /* Turn into time structure */
        lt = localtime(&t);
        sm_debug("hwnd=%p %d/%d/%d %d:%d:%d %d\n", hwnd, lt->tm_year + 1900,
                lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min,
                lt->tm_sec, lt->tm_wday);
        sprintf(str, "%02d:%02d", lt->tm_hour, lt->tm_min);
        /* Display as 11:02 */
        GetTextExtent(hdc, str, -1, &size);
        if (timeTitleData->flag == 0) {
            /* #aeff00 */
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 174, 255, 0));
            if (rect.right > 240)
                TextOut(hdc, 32, 40, str);
            else
                TextOut(hdc, 20, 20, str);
        } else {
            /* #ffa200 */
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 162, 0));
            TextOut(hdc, rect.right / 2 - size.cx / 2, 0, str);
        }

        sprintf(str, "%02d%s%02d%s%s", lt->tm_mon + 1, getDateText(7),
                lt->tm_mday, getDateText(8), getDateText(lt->tm_wday));
        SelectFont(hdc, timeTitleData->timeDataLogfont);
        GetTextExtent(hdc, str, -1, &size);
        /* Display as 01/02 Saturday */
        if (timeTitleData->flag == 0) {
            if (rect.right > 240)
                TextOut(hdc, 36, rect.bottom - size.cy + 3, str);
            else
                TextOut(hdc, 22, rect.bottom - size.cy + 3, str);
            /* Draw volume icon */
            SelectFont(hdc, timeTitleData->volumeLogfont);
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
            if (rect.right > 240) {
                FillBoxWithBitmap(hdc, 247, 16, 27, 21,
                        &timeTitleData->volumeBmp);
                TextOut(hdc, 274, 8, "20");
            } else {
                FillBoxWithBitmap(hdc, 188, 10, 0, 0,
                        &timeTitleData->volumeBmp);
                TextOut(hdc, 208, 3, "20");
            }
        } else {
            TextOut(hdc, rect.right / 2 - size.cx / 2, rect.bottom - size.cy,
                    str);
        }

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        KillTimer(hwnd, ID_TIMER_UPDATE_TIME);
        if (timeTitleData) {
            free(timeTitleData);
            timeTitleData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

/*
 * @param flag 0 is smart music player window, 1 is screen savers window
 */
HWND TimeTitleViewInit(HWND hosting, Uint32 flag) {
    smRect rect;
    HWND timeTitleHwnd = HWND_INVALID;
    TimeTitleData *timeTitleData = NULL;
    timeTitleData = TimeTitleDataInit(timeTitleData, flag);

    if (timeTitleData->flag == 0) {
        getResRect(ID_TIME_TITLE_MAIN_AREA, &rect);
        timeTitleHwnd = CreateWindowEx(TIME_TITLE_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) timeTitleData);
    } else {
        getResRect(ID_TIME_TITLE_WELCOME_AREA, &rect);
        timeTitleHwnd = CreateWindowEx(TIME_TITLE_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) timeTitleData);
    }

    return timeTitleHwnd;
}

BOOL RegisterTimeTitleView() {
    WNDCLASS MyClass;
    MyClass.spClassName = TIME_TITLE_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = TimeTitleViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterTimeTitleView(void) {
    UnregisterWindowClass(TIME_TITLE_VIEW);
}
