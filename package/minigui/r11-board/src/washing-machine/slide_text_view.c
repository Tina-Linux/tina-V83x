#include<stdio.h>
#include<time.h>
#include<sys/time.h>
#include "slide_text_view.h"
#include "common.h"

#define TIMER_ID 10
#ifdef ALLWINNERTECH_R6
#define SLIDE_UP_SPEED 30
#else
#define SLIDE_UP_SPEED 10
#endif

static int slideDistance = 0;
/* moveDistance is the distance that needs to be slid each time */
static int moveDistance = 0;
static RECT parentRect, clientRect;
static int listSize;
static int selectIndex;
static char *listData[LIST_MAX_SIZE];

/**
 * @ hwnd			The handle of the control
 */
BOOL ChangeSlideTextPage(HWND hwnd) {
	SlideTextView *slideTextData = NULL;
	slideTextData = (SlideTextView*) GetWindowAdditionalData(hwnd);
	if (slideTextData) {
		switch (slideTextData->currentPage) {
		case PAGE_ONE:
			slideTextData->currentPage = PAGE_TOW;
			break;
		case PAGE_TOW:
			slideTextData->currentPage = PAGE_ONE;
			break;
		default:
			slideTextData->currentPage = PAGE_ONE;
			break;
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
 * @ hwnd			The handle of the control
 */
void SlideTextViewSwitch(HWND hwnd) {
	if (slideDistance == 0) {
		if (IsWindowVisible(hwnd)) {
			ShowWindow(hwnd, SW_HIDE);
			if (GetCapture() == hwnd)
				ReleaseCapture();
		} else {
			SlideTextView *slideTextData = NULL;
			slideTextData = (SlideTextView*) GetWindowAdditionalData(hwnd);
			int i;
			switch (slideTextData->currentPage) {
			case PAGE_ONE:
				listSize = slideTextData->listSize1;
				selectIndex = slideTextData->selectIndex1;
				for (i = 0; i < listSize; i++)
					listData[i] = slideTextData->listData1[i];
				break;
			case PAGE_TOW:
				listSize = slideTextData->listSize2;
				selectIndex = slideTextData->selectIndex2;
				for (i = 0; i < listSize; i++)
					listData[i] = slideTextData->listData2[i];
				break;
			default:
				slideTextData->currentPage = PAGE_ONE;
				listSize = slideTextData->listSize1;
				selectIndex = slideTextData->selectIndex1;
				for (i = 0; i < listSize; i++)
					listData[i] = slideTextData->listData1[i];
				break;
			}

			GetWindowRect(hwnd, &parentRect);
			printf("parentRect:(%d, %d, %d, %d)\n", parentRect.right, parentRect.bottom, parentRect.left, parentRect.top);

			GetClientRect(hwnd, &clientRect);
			printf("parentRect:(%d, %d, %d, %d)\n", clientRect.right, clientRect.bottom, clientRect.left, clientRect.top);

			SetTimer(hwnd, TIMER_ID, 1);

			//Capture the mouse, in addition to the control can also get coordinate values
			SetCapture(hwnd);
		}
	}
}

static int SlideTextViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

	static BOOL isSlidText = FALSE;
	static int oldY = 0;
	static int oldGap = 0;
	static int newX = 0;
	static int newY = 0;
	static int moveNum = 0;
	static int slideYDown = 0;
	static int slideYUp = 0;
	static RECT rect;
	static struct timeval downStart, downEnd;

	SlideTextView *slideTextData = NULL;
	slideTextData = (SlideTextView*) GetWindowAdditionalData(hwnd);

	switch (message) {
    case MSG_TIMER: {
        int timerId = (int) wParam;
        if (timerId != SLIDE_TEXT_TIMER_ID) {
            /* Control moving animation */
            if (slideDistance == 0)
                ShowWindow(hwnd, SW_SHOWNORMAL);
            MoveWindow(hwnd, parentRect.left, parentRect.bottom - slideDistance,
                    clientRect.right, slideDistance, FALSE);
            slideDistance += SLIDE_UP_SPEED;
            if (slideDistance >= clientRect.bottom) {
                MoveWindow(hwnd, parentRect.left, parentRect.top,
                        clientRect.right, clientRect.bottom, FALSE);
                slideDistance = 0;
                KillTimer(hwnd, TIMER_ID);
            }
        } else {
            /* Finger sliding text animation */
            if (moveDistance > 0) {
                /* Finger slide down */
                if (selectIndex > 0) {
                    moveDistance --;
                    oldGap += abs(moveDistance);
                    InvalidateRect(hwnd, NULL, TRUE);
                } else {
                    moveDistance = 0;
                }
            } else {
                /* Finger slide up*/
                if (selectIndex < listSize - 1) {
                    moveDistance ++;
                    oldGap -= abs(moveDistance);
                    InvalidateRect(hwnd, NULL, TRUE);
                } else {
                    moveDistance = 0;
                }
            }
            if (abs(moveDistance) <= 1) {
                oldGap = 0;
                InvalidateRect(hwnd, NULL, TRUE);
                KillTimer(hwnd, SLIDE_TEXT_TIMER_ID);
            }
        }
        break;
    }
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		SIZE size;
		GetClientRect(hwnd, &rect);
		//Set the background image
		FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
				&slideTextData->bmpBg);

		SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 179, 9));
		SetBkMode(hdc, BM_TRANSPARENT);
		SelectFont(hdc, slideTextData->logfontSelect);
		//Sets the font and position of the selected value
		GetTextExtent(hdc, listData[selectIndex], -1, &size);

		TextOut(hdc, rect.right / 2 - size.cx / 2,
				rect.bottom / 2 - size.cy / 2 + oldGap, listData[selectIndex]);

		SetTextColor(hdc, COLOR_lightwhite);
		SelectFont(hdc, GetWindowFont(hwnd));

		//The other four use TextOut, which looks like scrolling, does not show more than the maximum and minimum
		if (selectIndex - 2 >= 0) {
			GetTextExtent(hdc, listData[selectIndex - 2], -1, &size);
			TextOut(hdc, rect.right / 2 - size.cx / 2,
					rect.bottom / 2 - size.cy / 2 - 150 + oldGap,
					listData[selectIndex - 2]);
		}
		if (selectIndex - 1 >= 0) {
			GetTextExtent(hdc, listData[selectIndex - 1], -1, &size);
			TextOut(hdc, rect.right / 2 - size.cx / 2,
					rect.bottom / 2 - size.cy / 2 - 75 + oldGap,
					listData[selectIndex - 1]);
		}
		if (selectIndex + 1 < listSize) {
			GetTextExtent(hdc, listData[selectIndex + 1], -1, &size);
			TextOut(hdc, rect.right / 2 - size.cx / 2,
					rect.bottom / 2 - size.cy / 2 + 75 + oldGap,
					listData[selectIndex + 1]);
		}
		if (selectIndex + 2 < listSize) {
			GetTextExtent(hdc, listData[selectIndex + 2], -1, &size);
			TextOut(hdc, rect.right / 2 - size.cx / 2,
					rect.bottom / 2 - size.cy / 2 + 150 + oldGap,
					listData[selectIndex + 2]);
		}

		//Replace the currently selected value every 60 steps and send a notification
		if (oldGap < -60) {
			if (slideTextData->currentPage == PAGE_ONE) {
				slideTextData->selectIndex1++;
				selectIndex = slideTextData->selectIndex1;
			} else {
				slideTextData->selectIndex2++;
				selectIndex = slideTextData->selectIndex2;
			}
			oldGap = 0;
			BroadcastMessage(MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
					(LPARAM) listData[selectIndex]);
			if (slideTextData->notifHwnd) {
				SendNotifyMessage(slideTextData->notifHwnd,
						MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
						(LPARAM) listData[selectIndex]);
			}
		} else if (oldGap > 60) {
			if (slideTextData->currentPage == PAGE_ONE) {
				slideTextData->selectIndex1--;
				selectIndex = slideTextData->selectIndex1;
			} else {
				slideTextData->selectIndex2--;
				selectIndex = slideTextData->selectIndex2;
			}
			oldGap = 0;
			BroadcastMessage(MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
					(LPARAM) listData[selectIndex]);
			if (slideTextData->notifHwnd) {
				SendNotifyMessage(slideTextData->notifHwnd,
						MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
						(LPARAM) listData[selectIndex]);
			}
		}
		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_LBUTTONDOWN: {
		newX = LOWORD(lParam);
		newY = HIWORD(lParam);
		slideYDown = HIWORD(lParam);

		ScreenToClient(hwnd, &newX, &newY);
		GetClientRect(hwnd, &rect);
		if (newX < 0 || newY < 0 || newX > rect.right || newY > rect.bottom) {
			//If you click outside the control, close the control
			SlideTextViewSwitch(hwnd);
			break;
		}

		gettimeofday(&downStart, NULL);
		moveDistance = 0;
		oldY = newY;
		oldGap = 0;
		moveNum = 0;
		break;
	}
	case MSG_LBUTTONUP: {
        //Release the mouse when the reset
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        slideYUp = HIWORD(lParam);
        ScreenToClient(hwnd, &x, &y);
        GetClientRect(hwnd, &rect);

        if (!(x < 0 || y < 0 || x > rect.right || y > rect.bottom)) {
            if (!isSlidText) {
                /* Click on the top and bottom areas to select text */
                if (y < 80) {
                    if (selectIndex > 0) {
                        if (slideTextData->currentPage == PAGE_ONE) {
                            slideTextData->selectIndex1--;
                            selectIndex = slideTextData->selectIndex1;
                        } else {
                            slideTextData->selectIndex2--;
                            selectIndex = slideTextData->selectIndex2;
                        }
                        BroadcastMessage(MSG_SLIDE_TEXT_CHANGE,
                                GetDlgCtrlID(hwnd),
                                (LPARAM) listData[selectIndex]);
                        if (slideTextData->notifHwnd) {
                            SendNotifyMessage(slideTextData->notifHwnd,
                                    MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
                                    (LPARAM) listData[selectIndex]);
                        }
                    }
                } else if (y > 160) {
                    if (selectIndex < listSize - 1) {
                        if (slideTextData->currentPage == PAGE_ONE) {
                            slideTextData->selectIndex1++;
                            selectIndex = slideTextData->selectIndex1;
                        } else {
                            slideTextData->selectIndex2++;
                            selectIndex = slideTextData->selectIndex2;
                        }
                        BroadcastMessage(MSG_SLIDE_TEXT_CHANGE,
                                GetDlgCtrlID(hwnd),
                                (LPARAM) listData[selectIndex]);
                        if (slideTextData->notifHwnd) {
                            SendNotifyMessage(slideTextData->notifHwnd,
                                    MSG_SLIDE_TEXT_CHANGE, GetDlgCtrlID(hwnd),
                                    (LPARAM) listData[selectIndex]);
                        }
                    }
                }
            }
        }

        gettimeofday(&downEnd, NULL);
        isSlidText = FALSE;
        moveDistance = slideYUp - slideYDown;
        /*
         * Finger sliding text animation
         * The moving distance between down and up is greater than 10
         * The interval between down and up does not exceed 200ms
         * */
        if (abs(moveDistance) > 10
                && (downEnd.tv_sec * 1000 + downEnd.tv_usec / 1000)
                        - (downStart.tv_sec * 1000 + downStart.tv_usec / 1000)
                        < 200) {
            moveDistance = moveDistance / 5;
            SetTimer(hwnd, SLIDE_TEXT_TIMER_ID, 1);
        } else {
            oldGap = 0;
            InvalidateRect(hwnd, NULL, TRUE);
        }
		break;
	}
	case MSG_MOUSEMOVE: {
        newX = LOWORD(lParam);
        newY = HIWORD(lParam);

        moveNum++;
        if (moveNum >= 3) {
            isSlidText = TRUE;
            moveNum = 0;
            //Calculate the step value and update the window
            ScreenToClient(hwnd, &newX, &newY);
            printf("newX=%d, newY=%d\n", newX, newY);

            int gap = newY - oldY;
            int newGap = oldGap + gap;
            //If the maximum or minimum value is reached, the sliding step value is no longer allowed to exceed 50

            printf("selectIndex=(%d, %d, %d)\n", selectIndex, gap, newGap);
            if ((selectIndex <= 0 && gap > 0 && newGap > 50)
                    || (selectIndex >= listSize - 1 && gap < 0 && newGap < -50))
                break;
            InvalidateRect(hwnd, NULL, TRUE);
            oldY = newY;
            oldGap = newGap;
        }
		break;
	}
	}

	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSlideTextView() {
	WNDCLASS MyClass;
	MyClass.spClassName = SLIDE_TEXT_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = SlideTextViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterSlideTextView(void) {
	UnregisterWindowClass(SLIDE_TEXT_VIEW);
}
