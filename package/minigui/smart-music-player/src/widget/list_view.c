/**
 * Project Name: smart-music-player
 * File Name: list_view.c
 * Date: 2019-08-27 14:42
 * Author: anruliu
 * Description: Sliding list control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "list_view.h"
#include "resource.h"
#include "common.h"
#include <sys/time.h>
#include <time.h>

#ifdef __SMART_MUSIC_PLAYER_240x320__
#define SLIDING_STEP_VALUE  4   /* Decrease 4 each time */
#define SLIDING_INTERVALS   4   /* 40ms slide once  */
#define SLIDING_MAX_NUM     50  /* Slide up to 50 times at a time */
#else
#define SLIDING_STEP_VALUE  8   /* Decrease 8 each time */
#define SLIDING_INTERVALS   4   /* 40ms slide once  */
#define SLIDING_MAX_NUM     50  /* Slide up to 50 times at a time */
#endif

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("list item clike hwnd=%p id=%d add_data=%ld\n", hwnd, id,
            add_data);

    if (nc == STN_CLICKED) {
        /* GetDlgCtrlID(hwnd) directly passes the ID of each item */
        NotifyParentEx(GetParent(hwnd), GetDlgCtrlID(hwnd), STN_CLICKED,
                add_data);
    }
}

static int MyListViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

    MyListData *myListData = NULL;
    myListData = (MyListData*) GetWindowAdditionalData(hwnd);

    static int isScroll;
    static int isFling;
    static int isStop; /* When the isFling is TRUE, press the finger to end the slide */
    static int slidingNum;
    static int slidingDirect;
    static int slidingVelocity;
    static long oldtime;
    struct timeval tv;

    switch (message) {
    case MSG_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int i;
        /* Creating too many controls at once can be slow
         * and can be created batch by slide */
        for (i = 0; i < myListData->listSize; i++) {
            myListData->itemHwnd[i] = CreateWindowEx(myListData->itemType[i],
                    "", WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, i,
                    0, i * myListData->itemHeight, rect.right,
                    myListData->itemHeight, hwnd,
                    (DWORD) myListData->itemData[i]);
            SetNotificationCallback(myListData->itemHwnd[i], my_notify_proc);
        }
        if (rect.bottom % myListData->itemHeight == 0) {
            /* The height of the list view page is just right to display several items */
            myListData->slidingMaxDistance = (myListData->listSize
                    - rect.bottom / myListData->itemHeight)
                    * myListData->itemHeight;
        } else {
            myListData->slidingMaxDistance = (myListData->listSize - 1
                    - rect.bottom / myListData->itemHeight)
                    * myListData->itemHeight
                    + (myListData->itemHeight
                            - rect.bottom % myListData->itemHeight);
        }
        isScroll = FALSE;
        isFling = FALSE;
        isStop = FALSE;
        break;
    }
    case MSG_TIMER:
        sm_debug(
                "list view timer slidingDistance=%d slidingVelocity=%d slidingNum=%d slidingDirect=%d\n",
                myListData->slidingDistance, slidingVelocity, slidingNum,
                slidingDirect);
        if (slidingDirect == MOUSE_UP) {
            /* Sliding to the bottom and no longer slide */
            if (myListData->slidingDistance
                    <= -myListData->slidingMaxDistance) {
                slidingNum = SLIDING_MAX_NUM;
                goto slidingEnd;
            }
            /* Make it just slide to the bottom */
            if (myListData->slidingDistance - slidingVelocity
                    < -myListData->slidingMaxDistance)
                slidingVelocity = myListData->slidingMaxDistance
                        + myListData->slidingDistance;
            myListData->slidingDistance -= slidingVelocity;
            ScrollWindow(hwnd, 0, -slidingVelocity, NULL, NULL);
        } else if (slidingDirect == MOUSE_DOWN) {
            /* Sliding to the top and no longer slide */
            if (myListData->slidingDistance >= 0) {
                slidingNum = SLIDING_MAX_NUM;
                goto slidingEnd;
            }
            /* Make it just slide to the top */
            if (myListData->slidingDistance + slidingVelocity > 0)
                slidingVelocity = -myListData->slidingDistance;
            myListData->slidingDistance += slidingVelocity;
            ScrollWindow(hwnd, 0, slidingVelocity, NULL, NULL);
        }
        /* End sliding when slidingVelocity becomes zero */
        if (slidingVelocity - SLIDING_STEP_VALUE < 0) {
            slidingNum = 0;
            isFling = FALSE;
            KillTimer(hwnd, ID_TIMER_LIST_SLIDE);
        } else {
            slidingVelocity -= SLIDING_STEP_VALUE;
        }
        /* End sliding when slidingVelocity becomes SLIDING_MAX_NUM */
        slidingEnd: slidingNum++;
        if (slidingNum > SLIDING_MAX_NUM) {
            slidingNum = 0;
            isFling = FALSE;
            KillTimer(hwnd, ID_TIMER_LIST_SLIDE);
        }
        break;
    case MSG_LBUTTONDOWN:
        if (isFling) {
            KillTimer(hwnd, ID_TIMER_LIST_SLIDE);
            slidingNum = 0;
            isFling = FALSE;
            isStop = TRUE;
            return 0;
        }
        isScroll = FALSE;
        isFling = FALSE;
        isStop = FALSE;
        gettimeofday(&tv, NULL);
        oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        break;
    case MSG_LBUTTONUP:
        /* The up event is no longer distributed after the list is sliding!!! */
        if (isScroll || isFling)
            return 0;
        if (isStop) {
            isStop = FALSE;
            return 0;
        }
        break;
    case MSG_MOUSE_SCROLL: {
        if (isFling)
            break;
        isScroll = TRUE;
        int x = LOSWORD(lParam);
        int y = HISWORD(lParam);
        int direct = LOSWORD(wParam);
        int velocity = HISWORD(wParam);

        sm_debug("list view scroll slidingDistance=%d velocity=%d\n",
                myListData->slidingDistance, velocity);

        if (direct == MOUSE_UP) {
            /* Sliding to the bottom and no longer slide */
            if (myListData->slidingDistance <= -myListData->slidingMaxDistance)
                break;
            /* Make it just slide to the bottom */
            if (myListData->slidingDistance - velocity
                    < -myListData->slidingMaxDistance)
                velocity = myListData->slidingMaxDistance
                        + myListData->slidingDistance;
            myListData->slidingDistance -= velocity;
            ScrollWindow(hwnd, 0, -velocity, NULL, NULL);
        } else if (direct == MOUSE_DOWN) {
            /* Sliding to the top and no longer slide */
            if (myListData->slidingDistance >= 0)
                break;
            /* Make it just slide to the top */
            if (myListData->slidingDistance + velocity > 0)
                velocity = -myListData->slidingDistance;
            myListData->slidingDistance += velocity;
            ScrollWindow(hwnd, 0, velocity, NULL, NULL);
        } else {
            break;
        }
        break;
    }
    case MSG_MOUSE_FLING: {
        if (isFling)
            break;
        int x = LOSWORD(lParam);
        int y = HISWORD(lParam);
        int direct = LOSWORD(wParam);
        int velocity = HISWORD(wParam);

        sm_debug("list view fling slidingDistance=%d velocity=%d\n",
                myListData->slidingDistance, velocity);

        gettimeofday(&tv, NULL);
        long newtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        int lentime = newtime - oldtime;
        /* The time between DOWN and UP exceeds 200 milliseconds and does not slid */
        if (velocity > 10 && lentime < 200) {
            slidingNum = 0;
            slidingDirect = direct;
            slidingVelocity = velocity / 3 * 2;
            isFling = TRUE;
            SetTimer(hwnd, ID_TIMER_LIST_SLIDE, SLIDING_INTERVALS);
        }
        break;
    }
    case MSG_DESTROY: {
        if (IsTimerInstalled(hwnd, ID_TIMER_LIST_SLIDE)) {
            KillTimer(hwnd, ID_TIMER_LIST_SLIDE);
        }
        if (myListData) {
            free(myListData);
            myListData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterMyListView() {
    WNDCLASS MyClass;
    MyClass.spClassName = MY_LIST_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = MyListViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterMyListView(void) {
    UnregisterWindowClass(MY_LIST_VIEW);
}
