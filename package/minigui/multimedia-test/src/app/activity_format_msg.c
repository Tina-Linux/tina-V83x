#include "resource.h"

static int sd_or_ud;
/* Without the touch screen and mouse, mark the currently selected button */
static int curIndex = 0;

static HWND hMainWnd = HWND_INVALID;
static HWND msgHwnd;
static HWND cancelBtn;
static HWND okBtn;

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == BN_CLICKED) {
		if (id == 0) {
			PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
		} else if (id == 1) {
			int ret;
			ret = format_disk(sd_or_ud);
			if (ret == -1) {
				SetWindowText(msgHwnd, "Failed to format.");
				sm_warn("Failed to format.\n");
			} else {
				EnableWindow(okBtn, FALSE);
				SetWindowText(msgHwnd, "Format successfully.");
				sm_debug("Format successfully.\n");
			}
		}
	}
}

static int ActivityFormatMsgProc(HWND hWnd, int nMessage, WPARAM wParam,
		LPARAM lParam) {

	switch (nMessage) {
	case MSG_CREATE: {
		RECT rect;
		PLOGFONT curFont;
		GetClientRect(hWnd, &rect);
		int btnHight = rect.bottom / 5;
		if (g_rcScr.right <= 800) {
			curFont = getLogFont(ID_FONT_TIMES_18);
		} else {
			curFont = getLogFont(ID_FONT_TIMES_28);
		}

		if (sd_or_ud == 0) {
			msgHwnd = CreateWindowEx(CTRL_STATIC,
					"Do you need to format the SD card?",
					WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
					WS_EX_TRANSPARENT, 0, 0,
					(rect.bottom - btnHight) / 2 - btnHight / 2, rect.right,
					btnHight, hWnd, 0);
		} else {
			msgHwnd = CreateWindowEx(CTRL_STATIC,
					"Do you need to format the U disk?",
					WS_CHILD | WS_VISIBLE | BS_NOTIFY | SS_CENTER,
					WS_EX_TRANSPARENT, 0, 0,
					(rect.bottom - btnHight) / 2 - btnHight / 2, rect.right,
					btnHight, hWnd, 0);
		}

		SetWindowFont(msgHwnd, curFont);

		cancelBtn = CreateWindowEx(CTRL_BUTTON, "Cancel",
		WS_CHILD | WS_VISIBLE | BS_NOTIFY,
		WS_EX_TRANSPARENT, 0, 0, rect.bottom - btnHight, rect.right / 2,
				btnHight, hWnd, 0);
		SetWindowFont(cancelBtn, curFont);
		SetFocusChild(cancelBtn);
		SetNotificationCallback(cancelBtn, my_notify_proc);

		okBtn = CreateWindowEx(CTRL_BUTTON, "OK",
		WS_CHILD | WS_VISIBLE | BS_NOTIFY,
		WS_EX_TRANSPARENT, 1, rect.right / 2, rect.bottom - btnHight,
				rect.right / 2, btnHight, hWnd, 0);
		SetWindowFont(okBtn, curFont);
		SetNotificationCallback(okBtn, my_notify_proc);
		break;
	}
	case MSG_KEYUP: {
		switch (wParam) {
		case CVR_KEY_RIGHT:
			if (curIndex == 1) {
				curIndex = 0;
				SetFocusChild(cancelBtn);
			} else {
				curIndex++;
				SetFocusChild(okBtn);
			}
			sm_debug("CVR_KEY_RIGHT %d\n", curIndex);
			break;
		case CVR_KEY_LEFT:
			if (curIndex == 0) {
				curIndex = 1;
				SetFocusChild(okBtn);
			} else {
				curIndex--;
				SetFocusChild(cancelBtn);
			}
			sm_debug("CVR_KEY_LEFT %d\n", curIndex);
			break;
		case CVR_KEY_OK:
			sm_debug("CVR_KEY_OK %d\n", curIndex);
			if (curIndex == 0) {
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			} else if (curIndex == 1) {
				int ret;
				ret = format_disk(sd_or_ud);
				if (ret == -1) {
					SetWindowText(msgHwnd, "Failed to format.");
					sm_warn("Failed to format.\n");
				} else {
					EnableWindow(okBtn, FALSE);
					SetWindowText(msgHwnd, "Format successfully.");
					sm_debug("Format successfully.\n");
				}
			}
			break;
		}
		break;
	}
	case MSG_DESTROY:
		DestroyAllControls(hWnd);
		hMainWnd = HWND_INVALID;
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		MainWindowThreadCleanup(hWnd);
		return 0;
	}
	return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

/**
 * The formatting dialog is displayed
 *
 * @mode 0 formatted SD card, 1 formatted USB flash drive
 */
int ActivityFormatMsg(HWND hosting, int mode) {

	if (hMainWnd != HWND_INVALID)
		return -1;

	if (mode < 0 || mode > 1)
		return -1;

	sd_or_ud = mode;

	smRect rect;
	getResRect(ID_FUN_BUTTON, &rect);

	MSG Msg;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TOPMOST;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityFormatMsgProc;
	CreateInfo.lx = rect.x;
	CreateInfo.ty = rect.y;
	CreateInfo.rx = rect.x + rect.w;
	CreateInfo.by = rect.y + rect.h;
	CreateInfo.iBkColor = PIXEL_lightgray;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	hMainWnd = CreateMainWindow(&CreateInfo);

	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

