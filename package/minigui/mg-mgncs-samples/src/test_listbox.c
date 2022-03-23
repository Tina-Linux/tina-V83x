#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

static int MainWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {
	switch(message){
	case MSG_CREATE:{
		ActivityListbox(hWnd);
		return 0;
	}

	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain(int argc, const char* argv[])
{
	MSG Msg;
	MAINWINCREATE CreateInfo;
	HWND hMainWnd;
#ifdef _MGRM_PROCESSES
	JoinLayer(NAME_DEF_LAYER , "111" , 0 , 0);
#endif
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS | WS_EX_TOPMOST
			| WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWinProc;
		CreateInfo.lx = 200;
		CreateInfo.ty = 100;
		CreateInfo.rx = 600;
		CreateInfo.by = 380;

	CreateInfo.iBkColor = PIXEL_lightwhite;
	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

