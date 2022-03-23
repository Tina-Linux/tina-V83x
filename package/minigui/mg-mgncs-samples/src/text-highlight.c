#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgncs/mgncs.h>
#include <mgeff/mgeff.h>

#include <mgncs4touch/mgncs4touch.h>
#include <mgncs4touch/mtouchdebug.h>
#include <mgncs4touch/highlight.h>
static HighLight *highlight;
static HighLight *HighlightInit(HighLight *highlight)
{
	highlight = (HighLight *)malloc(sizeof(HighLight));
	if(NULL == highlight){
		printf("malloc highlight data error\n");
	}
	memset((void *)highlight, 0, sizeof(HighLight));
	RECT textrect = {0,0,800,200};
	gal_pixel textcolor = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	PLOGFONT textfont = CreateLogFont("ttf", "simsun", "UTF-8",
        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
        FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 40, 0);
	highlight->text = "This just a highlight text demo";
	highlight->TextColor = textcolor;
	highlight->TextFont = textfont;
	highlight->HighLightStartX = -50;
	highlight->AnimDuration = 2000;
	highlight->TextRect = textrect;
	return highlight;
}
static int MainWindowProc(HWND hwnd, int nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case MSG_CREATE:{
		highlight = HighlightInit(highlight);
		CreateWindowEx(HIGHLIGHT, "",
                WS_CHILD | WS_VISIBLE | SS_NOTIFY, 0, 0, 0, 0, 800,200, hwnd, (DWORD) highlight);
		break;
	}
	case MSG_DESTROY:{
		if(highlight != NULL){
			free(highlight);
			highlight = NULL;
		}
                DestroyAllControls(hwnd);
                return 0;
        }
        case MSG_CLOSE: {
                DestroyMainWindow(hwnd);
                PostQuitMessage(0);
                return 0;
        }
	}
	return DefaultMainWinProc(hwnd, nMessage, wParam, lParam);
}
HWND hMainWnd;

HWND GetMainWnd()
{
	return hMainWnd;
}
int MiniGUIMain(int argc, const char* argv[])
{
#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER, NULL, 0, 0);
#endif
	MSG Msg;
        MAINWINCREATE CreateInfo;
	ncsInitialize();
	ncs4TouchInitialize();
	RegisterHighLight();
        CreateInfo.dwStyle = WS_NONE;
        CreateInfo.dwExStyle = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS | WS_EX_TOPMOST
                        | WS_EX_AUTOSECONDARYDC;
        CreateInfo.spCaption = "";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = 0; /*GetSystemCursor(0);*/
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = MainWindowProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = 800;
        CreateInfo.by = 480;
        CreateInfo.iBkColor = PIXEL_lightgray;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = HWND_DESKTOP;
        hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID){
                return -1;
        }
        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&Msg, hMainWnd)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
        }
	UnregisterHighLight();
        MainWindowThreadCleanup(hMainWnd);
        return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
