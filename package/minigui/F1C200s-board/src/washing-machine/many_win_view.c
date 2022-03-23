#include "many_win_view.h"
#include "static_view.h"
#include "resource.h"
#include <math.h>

#define TIMER_ID 10

#ifdef ALLWINNERTECH_R6
#define TIMER_SPEED 1
#define MANY_WIN_SPEED 80
#define TOW_PAGE_SPEED 800
#define THR_PAGE_SPEED 1600
#else
#define TIMER_SPEED 1
#define MANY_WIN_SPEED 16
#define TOW_PAGE_SPEED 30
#define THR_PAGE_SPEED 60
#endif

static HWND bg_fun1;
static HWND bg_fun2;
static HWND bg_fun3;
static HWND po_fun1;
static HWND po_fun2;
static HWND po_fun3;
static BITMAP bmpPointSeletc;
static BITMAP bmpPointNormal;
static int currentPage = 1;
static BOOL slidStatus = FALSE;
static BOOL moveStatus = FALSE;
static BOOL isMove = FALSE;
static int distance = 0;
static int slideMode = 0;

#ifdef ALLWINNERTECH_R6
static int flingNum = 0;

HWND GetFunHwnd() {
	return bg_fun1;
}

static int startMoveWindow(HWND hwnd, int index, int direction) {
	double A_1 = 800;
	double LocalX_1;
	double w_1 = 1.5;
	//double B_1 = 0.8;
	double B_1 = 1.5;
	double t_1 = 0;
	double LocalY_temp = 0;
	double last_pos = 0;
	int start_x1, start_x2, start_x3;
	int offset;

	switch (direction) {
		case MOUSE_LEFT:
		offset = -800;
		switch (index) {
			case 1:
			start_x1 = 0;
			start_x2 = 800;
			start_x3 = 1600;
			break;
			case 2:
			start_x1 = -800;
			start_x2 = 0;
			start_x3 = 800;
			break;
			default:
			return 0;
		}
		break;
		case MOUSE_RIGHT:
		offset = 800;
		switch (index) {
			case 2:
			start_x1 = -800;
			start_x2 = 0;
			start_x3 = 800;
			break;
			case 3:
			start_x1 = -1600;
			start_x2 = -800;
			start_x3 = 0;
			break;
			default:
			return 0;
		}
		break;
		case MOUSE_UP:
		break;
		case MOUSE_DOWN:
		break;
		default:
		break;
	}

	while (1) {
		LocalX_1 = A_1 * exp(-1 * B_1 * t_1) * cos(sqrt(w_1 * t_1));
		t_1 = t_1 + 0.2;

		if (direction == MOUSE_RIGHT)
		LocalX_1 = -LocalX_1;

		if (last_pos == LocalX_1)
		continue;

		MoveWindow(bg_fun1, start_x1 + LocalX_1 + offset, 100, 800, 316, TRUE);
		MoveWindow(bg_fun2, start_x2 + LocalX_1 + offset, 100, 800, 316, TRUE);
		MoveWindow(bg_fun3, start_x3 + LocalX_1 + offset, 100, 800, 316, TRUE);
		//UpdateWindow(win_slider->winpageHnd[0], TRUE);
		UpdateWindow(hwnd, TRUE);

		isMove = TRUE;
		usleep(10000);
		if ((int) LocalX_1 == 0)
		break;

		last_pos = LocalX_1;
	}
}
#endif

static int ManyWinViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	ManyWinView *manyWinData = NULL;
	manyWinData = (ManyWinView*) GetWindowAdditionalData(hwnd);

	static int oldX = 0;
	static int oldGap = 0;

	switch (message) {
	case MSG_CREATE: {
		manyWinData->funBtn[0] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 0, 20, 0, 110, 110, hwnd,
				(DWORD) &manyWinData->bmpFun[0]);
		//SetWindowFont(manyWinData->funBtn[0], getLogFont(ID_FONT_SIMSUN_25));

		manyWinData->funBtn[1] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 1, 180, 0, 110, 110, hwnd,
				(DWORD) &manyWinData->bmpFun[1]);
		//SetWindowFont(manyWinData->funBtn[1], getLogFont(ID_FONT_SIMSUN_25));

		manyWinData->funBtn[2] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 2, 340, 0, 110, 110, hwnd,
				(DWORD) &manyWinData->bmpFun[2]);
		//SetWindowFont(manyWinData->funBtn[2], getLogFont(ID_FONT_SIMSUN_25));

		/*
		manyWinData->funBtn[3] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 3, 20, 168, 241, 148, hwnd,
				(DWORD) &manyWinData->bmpFun[3]);
		//SetWindowFont(manyWinData->funBtn[3], getLogFont(ID_FONT_SIMSUN_25));

		manyWinData->funBtn[4] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 4, 281, 168, 241, 148, hwnd,
				(DWORD) &manyWinData->bmpFun[4]);
		//SetWindowFont(manyWinData->funBtn[4], getLogFont(ID_FONT_SIMSUN_25));

		manyWinData->funBtn[5] = CreateWindowEx(STATIC_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		WS_EX_TRANSPARENT, 5, 542, 168, 241, 148, hwnd,
				(DWORD) &manyWinData->bmpFun[5]);
		//SetWindowFont(manyWinData->funBtn[5], getLogFont(ID_FONT_SIMSUN_25));
		*/
		return 0;
	}
	case MSG_TIMER: {
		if (slideMode == SLIDE_MODE_APPEAR) {
			if (distance >= 780 && distance < 820) {
				MoveWindow(manyWinData->funBtn[0], 20, 0, 241, 148,
				FALSE);
			} else if (distance < 780) {
				MoveWindow(manyWinData->funBtn[0], 800 - distance, 0, 241, 148,
				FALSE);
			}
			if (distance >= 980) {
				MoveWindow(manyWinData->funBtn[1], 281, 0, 241, 148,
				FALSE);
			} else {
				MoveWindow(manyWinData->funBtn[1], 1261 - distance, 0, 241, 148,
				FALSE);
			}
			if (distance >= 1180) {
				MoveWindow(manyWinData->funBtn[2], 542, 0, 241, 148,
				FALSE);
			} else {
				MoveWindow(manyWinData->funBtn[2], 1722 - distance, 0, 241, 148,
				FALSE);
			}
			if (distance >= 980) {
				MoveWindow(manyWinData->funBtn[3], 20, 168, 241, 148,
				FALSE);
			} else {
				MoveWindow(manyWinData->funBtn[3], 1000 - distance, 168, 241,
						148,
						FALSE);
			}
			if (distance >= 1180) {
				MoveWindow(manyWinData->funBtn[4], 281, 168, 241, 148,
				FALSE);
			} else {
				MoveWindow(manyWinData->funBtn[4], 1461 - distance, 168, 241,
						148,
						FALSE);
			}
			if (distance >= 1380) {
				MoveWindow(manyWinData->funBtn[5], 542, 168, 241, 148,
				FALSE);
				slidStatus = FALSE;
				KillTimer(hwnd, TIMER_ID);
			} else {
				MoveWindow(manyWinData->funBtn[5], 1922 - distance, 168, 241,
						148,
						FALSE);
			}
			distance += MANY_WIN_SPEED;
			if (distance == MANY_WIN_SPEED) {
				ShowWindow(bg_fun1, SW_SHOWNORMAL);
				ShowWindow(bg_fun2, SW_SHOWNORMAL);
				ShowWindow(bg_fun3, SW_SHOWNORMAL);
			}
		} else if (slideMode == SLIDE_MODE_SWITCH) {
			if (currentPage == 2) {
				MoveWindow(bg_fun1, -800 + distance, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, 0 + distance, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 800 + distance, 100, 800, 316, FALSE);
				if (distance >= 800) {
					currentPage = 1;
					oldGap = 0;
					slidStatus = FALSE;
					MoveWindow(bg_fun1, 0, 100, 800, 316, FALSE);
					MoveWindow(bg_fun2, 800, 100, 800, 316, FALSE);
					MoveWindow(bg_fun3, 1600, 100, 800, 316, FALSE);
					SendNotifyMessage(po_fun1, STM_SETIMAGE,
							(DWORD) &bmpPointSeletc, 0);
					SendNotifyMessage(po_fun2, STM_SETIMAGE,
							(DWORD) &bmpPointNormal, 0);
					SendNotifyMessage(po_fun3, STM_SETIMAGE,
							(DWORD) &bmpPointNormal, 0);
					KillTimer(hwnd, TIMER_ID);
				}
				distance += TOW_PAGE_SPEED;
			} else if (currentPage == 3) {
				MoveWindow(bg_fun1, -1600 + distance, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, -800 + distance, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 0 + distance, 100, 800, 316, FALSE);
				if (distance >= 1600) {
					currentPage = 1;
					oldGap = 0;
					slidStatus = FALSE;
					MoveWindow(bg_fun1, 0, 100, 800, 316, FALSE);
					MoveWindow(bg_fun2, 800, 100, 800, 316, FALSE);
					MoveWindow(bg_fun3, 1600, 100, 800, 316, FALSE);
					SendNotifyMessage(po_fun1, STM_SETIMAGE,
							(DWORD) &bmpPointSeletc, 0);
					SendNotifyMessage(po_fun2, STM_SETIMAGE,
							(DWORD) &bmpPointNormal, 0);
					SendNotifyMessage(po_fun3, STM_SETIMAGE,
							(DWORD) &bmpPointNormal, 0);
					KillTimer(hwnd, TIMER_ID);
				}
				distance += THR_PAGE_SPEED;
			}
		}
		break;
	}
	case MSG_STATIC_CLICK:
		if (!slidStatus && !moveStatus && !isMove) {
			BroadcastMessage(MSG_MANY_WIN_CLICK, currentPage, (LPARAM) wParam);
		}
		break;
#ifdef ALLWINNERTECH_R6
		/**
		 * run on AllWinnertech R6
		 **/
		case MSG_LBUTTONUP:
		/*
		if (!slidStatus && isMove) {
			switch (currentPage) {
				case 1:
				MoveWindow(bg_fun1, 0, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, 800, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 1600, 100, 800, 316, FALSE);
				SendNotifyMessage(po_fun1, STM_SETIMAGE,
						(DWORD) &bmpPointSeletc, 0);
				SendNotifyMessage(po_fun2, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				SendNotifyMessage(po_fun3, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				oldGap = 0;
				break;
				case 2:
				MoveWindow(bg_fun1, -800, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, 0, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 800, 100, 800, 316, FALSE);
				SendNotifyMessage(po_fun1, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				SendNotifyMessage(po_fun2, STM_SETIMAGE,
						(DWORD) &bmpPointSeletc, 0);
				SendNotifyMessage(po_fun3, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				oldGap = -800;
				break;
				case 3:
				MoveWindow(bg_fun1, -1600, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, -800, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 0, 100, 800, 316, FALSE);
				SendNotifyMessage(po_fun1, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				SendNotifyMessage(po_fun2, STM_SETIMAGE,
						(DWORD) &bmpPointNormal, 0);
				SendNotifyMessage(po_fun3, STM_SETIMAGE,
						(DWORD) &bmpPointSeletc, 0);
				oldGap = -1600;
				break;
			}
			if (flingNum > 0) {
				flingNum--;
				return 0;
			}
			isMove = FALSE;
		}
		*/
		break;
		case MSG_MOUSE_FLING: {
			/*
			if (!slidStatus) {
				int x = LOSWORD(lParam);
				int y = HISWORD(lParam);
				int direct = LOSWORD(wParam);

				isMove = TRUE;
				flingNum++;

				if (direct == MOUSE_LEFT) {
					startMoveWindow(hwnd, currentPage, direct);
					currentPage++;
					if (currentPage > 3) {
						currentPage = 3;
					}
				} else if (direct == MOUSE_RIGHT) {
					startMoveWindow(hwnd, currentPage, direct);
					currentPage--;
					if (currentPage < 1) {
						currentPage = 1;
					}
				}

				switch (currentPage) {
					case 1:
					oldGap = 0;
					break;
					case 2:
					oldGap = -800;
					break;
					case 3:
					oldGap = -1600;
					break;
				}
			}
			*/
			break;
		}
		case MSG_MOUSE_SCROLL: {
			/*
			if (!slidStatus) {
				int x = LOSWORD(lParam);
				int y = HISWORD(lParam);
				int direct = LOSWORD(wParam);
				int velocity = HISWORD(wParam);

				if (velocity == x || y < 100 || y > 416)
				break;

				isMove = TRUE;

				if (direct == MOUSE_LEFT) {
					if (currentPage == 3 && velocity > 100)
					velocity = 100;
					oldGap -= velocity;
					if (oldGap < -1700) {
						oldGap = -1700;
						break;
					}
				} else if (direct == MOUSE_RIGHT) {
					if (currentPage == 1 && velocity > 100)
					velocity = 100;
					oldGap += velocity;
					if (oldGap > 100) {
						oldGap = 100;
						break;
					}
				}

				MoveWindow(bg_fun1, oldGap, 100, 800, 316, FALSE);
				MoveWindow(bg_fun2, 800 + oldGap, 100, 800, 316, FALSE);
				MoveWindow(bg_fun3, 1600 + oldGap, 100, 800, 316, FALSE);
			}
			*/
			break;
		}
#else
		/**
		 * run on Ubuntu
		 **/
	case MSG_LBUTTONDOWN:
		if (!slidStatus) {
			moveStatus = TRUE;
			isMove = FALSE;
			oldX = LOWORD(lParam);
		}
		break;
	case MSG_LBUTTONUP: {
		moveStatus = FALSE;
		if (GetCapture() != hwnd)
			break;
		ReleaseCapture();
		switch (currentPage) {
		case 1:
			MoveWindow(bg_fun1, 0, 100, 800, 316, FALSE);
			MoveWindow(bg_fun2, 800, 100, 800, 316, FALSE);
			MoveWindow(bg_fun3, 1600, 100, 800, 316, FALSE);
			SendNotifyMessage(po_fun1, STM_SETIMAGE, (DWORD) &bmpPointSeletc,
					0);
			SendNotifyMessage(po_fun2, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			SendNotifyMessage(po_fun3, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			oldGap = 0;
			break;
		case 2:
			MoveWindow(bg_fun1, -800, 100, 800, 316, FALSE);
			MoveWindow(bg_fun2, 0, 100, 800, 316, FALSE);
			MoveWindow(bg_fun3, 800, 100, 800, 316, FALSE);
			SendNotifyMessage(po_fun1, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			SendNotifyMessage(po_fun2, STM_SETIMAGE, (DWORD) &bmpPointSeletc,
					0);
			SendNotifyMessage(po_fun3, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			oldGap = -800;
			break;
		case 3:
			MoveWindow(bg_fun1, -1600, 100, 800, 316, FALSE);
			MoveWindow(bg_fun2, -800, 100, 800, 316, FALSE);
			MoveWindow(bg_fun3, 0, 100, 800, 316, FALSE);
			SendNotifyMessage(po_fun1, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			SendNotifyMessage(po_fun2, STM_SETIMAGE, (DWORD) &bmpPointNormal,
					0);
			SendNotifyMessage(po_fun3, STM_SETIMAGE, (DWORD) &bmpPointSeletc,
					0);
			oldGap = -1600;
			break;
		}
		break;
	}
	case MSG_MOUSEMOVE: {
		int newX = LOWORD(lParam);
		if (moveStatus) {
			if (GetCapture() != hwnd)
				SetCapture(hwnd);
			isMove = TRUE;
			int gap = newX - oldX;
			int newGap = oldGap + gap;
			if (gap < 0) {
				if (newGap < -1050) {
					currentPage = 3;
					if (newGap < -1700)
						break;
				} else if (newGap < -250) {
					currentPage = 2;
				} else {
					currentPage = 1;
				}
			} else if (gap > 0) {
				if (newGap < -1350) {
					currentPage = 3;
				} else if (newGap < -550) {
					currentPage = 2;
				} else {
					currentPage = 1;
					if (newGap > 100)
						break;
				}
			}
			MoveWindow(bg_fun1, newGap, 100, 800, 316, FALSE);
			MoveWindow(bg_fun2, 800 + newGap, 100, 800, 316, FALSE);
			MoveWindow(bg_fun3, 1600 + newGap, 100, 800, 316, FALSE);
			oldX = newX;
			oldGap = newGap;
		}
		break;
	}
#endif
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL GetSlideStatus() {
	return slidStatus;
}

int GetCurrentPage() {
	return currentPage;
}

void ManyWinAppear() {
	distance = 0;
	slideMode = SLIDE_MODE_APPEAR;
	slidStatus = TRUE;
	ShowWindow(bg_fun1, SW_HIDE);
	ShowWindow(bg_fun2, SW_HIDE);
	ShowWindow(bg_fun3, SW_HIDE);
	switch (currentPage) {
	case 1:
		SetTimer(bg_fun1, TIMER_ID, TIMER_SPEED);
		break;
	case 2:
		SetTimer(bg_fun2, TIMER_ID, TIMER_SPEED);
		break;
	case 3:
		SetTimer(bg_fun3, TIMER_ID, TIMER_SPEED);
		break;
	}
}

void OnePageSwitch() {
	if (currentPage != 1 && !slidStatus) {
		distance = 0;
		slideMode = SLIDE_MODE_SWITCH;
		slidStatus = TRUE;
		SetTimer(bg_fun1, TIMER_ID, TIMER_SPEED);
	}
}

void ManyWinViewInit(HWND hParent, ManyWinView *manyWinData1,
		ManyWinView *manyWinData2, ManyWinView *manyWinData3, BITMAP bmpPointN,
		BITMAP bmpPointP) {


	bg_fun1 = CreateWindowEx(MANY_WIN_VIEW, "",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 0, 100, 800, 316,
			hParent, (DWORD) manyWinData1);

	/*
	bg_fun2 = CreateWindowEx(MANY_WIN_VIEW, "",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 800, 100, 800, 316,
			hParent, (DWORD) manyWinData2);
	bg_fun3 = CreateWindowEx(MANY_WIN_VIEW, "",
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 1600, 100, 800,
			316, hParent, (DWORD) manyWinData3);

	bmpPointSeletc = bmpPointP;
	bmpPointNormal = bmpPointN;
	*/

	/*
	po_fun1 = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP, WS_EX_TRANSPARENT, 0, 359, 436, 14, 14,
			hParent, (DWORD) &bmpPointSeletc);
	po_fun2 = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP, WS_EX_TRANSPARENT, 0, 393, 436, 14, 14,
			hParent, (DWORD) &bmpPointNormal);
	po_fun3 = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP, WS_EX_TRANSPARENT, 0, 427, 436, 14, 14,
			hParent, (DWORD) &bmpPointNormal);
	*/
}

BOOL RegisterManyWinView() {
	WNDCLASS MyClass;
	MyClass.spClassName = MANY_WIN_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = ManyWinViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterManyWinView(void) {
	UnregisterWindowClass(MANY_WIN_VIEW);
}
