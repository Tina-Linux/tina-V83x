#include "fun_button_view.h"

FunButtonView *funButtonData = NULL;

FunButtonView* FunButtonDataInit(FunButtonView *funButtonData) {
	funButtonData = (FunButtonView*) malloc(sizeof(FunButtonView));
	if (NULL == funButtonData) {
		sm_error("malloc HeadBarView data error\n");
	}
	memset((void *) funButtonData, 0, sizeof(FunButtonView));

	int i = 0;
	for (i = 0; i < 7; i++) {
		funButtonData->funData[i] = (ButtonView*) malloc(sizeof(ButtonView));
		if (NULL == funButtonData->funData[i]) {
			sm_error("malloc ButtonView data error\n");
		}
		memset((void *) funButtonData->funData[i], 0, sizeof(ButtonView));

		funButtonData->funData[i]->selectStatus = SELECT_STATUS_NO;
		funButtonData->funData[i]->switchMode = SWITCH_MODE_NORMAL;

		setCurrentIconValue(ID_FUN_BUTTON_TPHOTO + i, 0);
		getResBmp(ID_FUN_BUTTON_TPHOTO + i, BMPTYPE_BASE,
				&funButtonData->funData[i]->bmpNormal);
		setCurrentIconValue(ID_FUN_BUTTON_TPHOTO + i, 1);
		getResBmp(ID_FUN_BUTTON_TPHOTO + i, BMPTYPE_BASE,
				&funButtonData->funData[i]->bmpSelect);
	}

	return funButtonData;
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		BroadcastMessage(MSG_FUN_BUTTON_CLICK, id, 0);
	}
}

static int FunButtonViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

	switch (message) {
	case MSG_CREATE: {
		smRect rect;
		getResRect(ID_FUN_BUTTON_TPHOTO, &rect);
		HWND funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[0]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_TAUDIO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[1]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_TVIDEO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[2]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_TOW_TVIDEO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[3]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_PHOTO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[4]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_AUDIO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[5]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		getResRect(ID_FUN_BUTTON_VIDEO, &rect);
		funHwnd = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 6, rect.x, rect.y,
				rect.w, rect.h, hwnd, (DWORD) funButtonData->funData[6]);
		SetNotificationCallback(funHwnd, my_notif_proc);

		return 0;
	}
	case MSG_DESTROY: {
		int i;
		for (i = 0; i < 7; i++) {
			unloadBitMap(&funButtonData->funData[i]->bmpNormal);
			unloadBitMap(&funButtonData->funData[i]->bmpSelect);
			if (funButtonData->funData[i] != NULL) {
				free(funButtonData->funData[i]);
				funButtonData->funData[i] = NULL;
			}
		}
		if (funButtonData != NULL) {
			free(funButtonData);
			funButtonData = NULL;
		}
		return 0;
	}
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND FunButtonViewInit(HWND hParent) {
	HWND mHwnd;
	smRect rect;
	getResRect(ID_FUN_BUTTON, &rect);
	funButtonData = FunButtonDataInit(funButtonData);
	mHwnd = CreateWindowEx(FUN_BUTTON_VIEW, "",
	WS_CHILD | WS_VISIBLE, WS_EX_NONE, 0, rect.x, rect.y, rect.w, rect.h,
			hParent, 0);

	if (mHwnd == HWND_INVALID) {
		return HWND_INVALID;
	}

	return mHwnd;
}

BOOL RegisterFunButtonView() {
	WNDCLASS MyClass;
	MyClass.spClassName = FUN_BUTTON_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = GetSystemCursor(0);
	MyClass.iBkColor = PIXEL_lightwhite;
	MyClass.WinProc = FunButtonViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterFunButtonView(void) {
	UnregisterWindowClass(FUN_BUTTON_VIEW);
}
