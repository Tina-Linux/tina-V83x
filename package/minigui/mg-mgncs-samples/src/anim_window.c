#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/minigui.h>

#include <mgeff/mgeff.h>

/*****************************************************************************/
#define CAPTION    "animation_window"

/*****************************************************************************/
/* main window proc */
static LRESULT mainWindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
/*****************************************************************************/
int MiniGUIMain (int argc, const char *argv[])
{
    HWND hMainHwnd;
    MAINWINCREATE createInfo;
    MSG msg;

    MGEFF_WINDOW_ANIMATION_CONTEXT handle;

#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER, "animation", 0, 0);
#endif

    createInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    createInfo.dwExStyle = WS_EX_NONE;
    createInfo.spCaption = CAPTION;
    createInfo.hMenu = 0;
    createInfo.hCursor = GetSystemCursor (0);
    createInfo.hIcon = 0;
    createInfo.MainWindowProc = mainWindowProc;
    createInfo.lx = 0;
    createInfo.ty = 0;
    createInfo.rx = 240;
    createInfo.by = 320;
    createInfo.iBkColor = COLOR_lightwhite;
    createInfo.dwAddData = 0;
    createInfo.hHosting = HWND_DESKTOP;

    handle = mGEffCreateWindowAnimationContext (10000, MGEFF_EFFECTOR_ZOOM, Linear, NULL, NULL);

    hMainHwnd = mGEffCreateMainWindow (&createInfo, handle);

    if (hMainHwnd == HWND_INVALID) {
        return -1;
    }
    mGEffShowWindow (hMainHwnd, SW_SHOWNORMAL, handle);

    while (GetMessage (&msg, hMainHwnd)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }

    mGEffDestroyWindowAnimationContext (handle);

    MainWindowThreadCleanup (hMainHwnd);

    return 0;
}
/*****************************************************************************/
/* main window proc */
static LRESULT mainWindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_CREATE:
        /* init mgeff library */
        mGEffInit ();

        break;

    case MSG_CLOSE:
        DestroyMainWindow (hWnd);
        PostQuitMessage (hWnd);

        /* deinit mgeff library */
        mGEffDeinit ();
        break;

    default:
        break;
    }

    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}
