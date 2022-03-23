#include "bottombar_view.h"
#include "swipe_view.h"
#include "washing_res_cn.h"

#define TIMER_ID 10
#ifdef ALLWINNERTECH_R6
#define SLIDE_OUT_SPEED 10
#else
#define SLIDE_OUT_SPEED 4
#endif

static int slideDistance = 0;
static HWND slideText1, slideText2, slideText3;
static HWND swipeHwnd, btn_switch;
/* Save button initial click status */
static int laundryPartClick[6] = {0};

int GetBottomBarDistance() {
	return slideDistance;
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    BottomBarView *bottomBarData = NULL;
    bottomBarData = (BottomBarView*) GetWindowAdditionalData(GetParent(hwnd));

	if (nc == STN_CLICKED && slideDistance >= 100) {
		switch (id) {
		case ID_DOUBLE_TEXT_VIEW_ONE:
			SlideTextViewSwitch(slideText1);
			break;
		case ID_DOUBLE_TEXT_VIEW_TOW:
			SlideTextViewSwitch(slideText2);
			break;
		case ID_DOUBLE_TEXT_VIEW_THR:
			SlideTextViewSwitch(slideText3);
			break;
		case ID_BUTTON_SWITCH:
		    SetTimer(swipeHwnd, SWIPE_TIMER_ID, 1);
			ChangeSlideTextPage(slideText1);
			ChangeSlideTextPage(slideText2);
			ChangeSlideTextPage(slideText3);
			break;
		case ID_BUTTON_START:
            printf("button select status = %d\n", (int) add_data);
            /* Set button click and clickable */
            if (bottomBarData->swipeData) {
                int i, j;
                for (i = 0; i < 2; i++) {
                    if (!bottomBarData->swipeData->laundryData[i])
                        break;
                    for (j = 0; j < 3; j++) {
                        if (!bottomBarData->swipeData->laundryData[i]->laundryPartData[j])
                            break;
                        if ((int) add_data == 1) {
                            bottomBarData->swipeData->laundryData[i]->laundryPartData[j]->enableClick =
                            LAUNDRY_PART_ENABLE_CLICK_NO;
                        } else {
                            /* If the initial state is not clickable, it is not set to yes */
                            if (laundryPartClick[i + j])
                                bottomBarData->swipeData->laundryData[i]->laundryPartData[j]->enableClick =
                                LAUNDRY_PART_ENABLE_CLICK_YES;
                        }
                        UpdateWindow(
                                bottomBarData->swipeData->laundryData[i]->laundryPartData[j]->laundryPartHwnd,
                                TRUE);
                    }
                }
            }
            if ((int) add_data == 1) {
                if (bottomBarData->swipeData)
                    EnableWindow(swipeHwnd, FALSE);
                if (bottomBarData->nextButton)
                    EnableWindow(btn_switch, FALSE);
            } else {
                if (bottomBarData->swipeData)
                    EnableWindow(swipeHwnd, TRUE);
                if (bottomBarData->nextButton)
                    EnableWindow(btn_switch, TRUE);
            }
			break;
		}
	}
}

static int BottomBarViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

    BottomBarView *bottomBarData = NULL;
    bottomBarData = (BottomBarView*) GetWindowAdditionalData(hwnd);

	switch (message) {
	case MSG_CREATE: {
		if (bottomBarData->slideTextData1) {
			bottomBarData->slideTextData1->notifHwnd = hwnd;
			slideText1 = CreateWindowEx(SLIDE_TEXT_VIEW, "", WS_CHILD,
			WS_EX_TRANSPARENT, ID_SLIDE_TEXT_VIEW_ONE, 0, 154, 162, 226,
					GetParent(hwnd), (DWORD) bottomBarData->slideTextData1);
			SetWindowFont(slideText1, getLogFont(ID_FONT_SIMSUN_40));
		}
		if (bottomBarData->slideTextData2) {
			bottomBarData->slideTextData2->notifHwnd = hwnd;
			slideText2 = CreateWindowEx(SLIDE_TEXT_VIEW, "", WS_CHILD,
			WS_EX_TRANSPARENT, ID_SLIDE_TEXT_VIEW_TOW, 162, 154, 162, 226,
					GetParent(hwnd), (DWORD) bottomBarData->slideTextData2);
			SetWindowFont(slideText2, getLogFont(ID_FONT_SIMSUN_40));
		}
		if (bottomBarData->slideTextData3) {
			bottomBarData->slideTextData3->notifHwnd = hwnd;
			slideText3 = CreateWindowEx(SLIDE_TEXT_VIEW, "", WS_CHILD,
			WS_EX_TRANSPARENT, ID_SLIDE_TEXT_VIEW_THR, 324, 154, 162, 226,
					GetParent(hwnd), (DWORD) bottomBarData->slideTextData3);
			SetWindowFont(slideText3, getLogFont(ID_FONT_SIMSUN_40));
		}

        if (bottomBarData->swipeData) {
            swipeHwnd = CreateWindowEx(SWIPE_VIEW, "",
            WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT, 0, 0, 0, 486, 100, hwnd,
                    (DWORD) bottomBarData->swipeData);

            /* Get button initial click status */
            int i, j;
            for (i = 0; i < 2; i++) {
                if (!bottomBarData->swipeData->laundryData[i])
                    break;
                for (j = 0; j < 3; j++) {
                    if (!bottomBarData->swipeData->laundryData[i]->laundryPartData[j])
                        break;
                    laundryPartClick[i + j] =
                            bottomBarData->swipeData->laundryData[i]->laundryPartData[j]->enableClick;
                }
            }
        }

		if (bottomBarData->nextButton) {
			btn_switch = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
			ID_BUTTON_SWITCH, 486, 0, 51, 100, hwnd,
					(DWORD) bottomBarData->nextButton);
			SetNotificationCallback(btn_switch, my_notif_proc);
		}
		if (bottomBarData->beginButton) {
			HWND btn_start = CreateWindowEx(BUTTON_VIEW, KAISHI,
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
			ID_BUTTON_START, 556, 0, 225, 100, hwnd,
					(DWORD) bottomBarData->beginButton);
			SetNotificationCallback(btn_start, my_notif_proc);
			SetWindowFont(btn_start, getLogFont(ID_FONT_SIMSUN_50));
			SetWindowBkColor(btn_start, PIXEL_lightwhite);
		}

		slideDistance = 0;
		return 0;
	}
	case MSG_TIMER: {
		MoveWindow(hwnd, 0, 480 - slideDistance, 800, slideDistance, FALSE);
		slideDistance += SLIDE_OUT_SPEED;
		if (slideDistance >= 100) {
			MoveWindow(hwnd, 0, 380, 800, 100, FALSE);
			KillTimer(hwnd, TIMER_ID);
		}
		if (slideDistance == SLIDE_OUT_SPEED) {
			ShowWindow(hwnd, SW_SHOWNORMAL);
		}
		break;
	}
	case MSG_SLIDE_TEXT_CHANGE: {
	    /*
	     * Notify the laundryPart to change the text data
	     * (int) wParam is SLIDE_TEXT_VIEW id, 1,2,3
	     * lParam is change the text data
	     */
        PostMessage(
                bottomBarData->swipeData->laundryData[bottomBarData->swipeData->currentPageIndex]->laundryPartData[(int) wParam]->laundryPartHwnd,
                MSG_LAUNDRY_PART_CHANGE_VALUE, lParam, 0);
        break;
	}
    case MSG_LAUNDRY_CLICK: {
        int clickId = (int) wParam;
        if (clickId == ID_LAUNDRY_VIEW_ONE) {
            SlideTextViewSwitch(slideText1);
        } else if (clickId == ID_LAUNDRY_VIEW_TOW) {
            SlideTextViewSwitch(slideText2);
        } else if (clickId == ID_LAUNDRY_VIEW_THR) {
            SlideTextViewSwitch(slideText3);
        }
        break;
    }
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND BottomBarViewInit(HWND hParent, BottomBarView *bottomBarData) {
	HWND mHwnd;
	mHwnd = CreateWindowEx(BOTTOM_BAR_VIEW, "",
	WS_CHILD, WS_EX_TRANSPARENT, 0, 0, 380, 800, 100, hParent,
			(DWORD) bottomBarData);

	if (mHwnd == HWND_INVALID) {
		return HWND_INVALID;
	}

	return mHwnd;
}

BOOL RegisterBottomBarView() {
	WNDCLASS MyClass;
	MyClass.spClassName = BOTTOM_BAR_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = BottomBarViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterBottomBarView(void) {
	UnregisterWindowClass(BOTTOM_BAR_VIEW);
}
