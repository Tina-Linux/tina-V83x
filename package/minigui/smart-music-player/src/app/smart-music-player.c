/*
 ** smart-music-player.c 2019-08-15 14:00:18
 **
 ** smart-music-player: Smart Music Player control UI
 **
 **
 ** Copyright (C) 2017 ~ 2020 Allwinnertech.
 **
 ** License: GPL
 */

#include "smart-music-player.h"

#include "resource.h"
#include "volume.h"
#include "datafactory.h"

#define SMART_MUSIC_PLAYER "Smart Music Player"

extern HWND ActivityScreensavers(HWND hosting);
extern HWND ActivityScene(HWND hosting);

WinDataType *g_win_data = NULL;

/* The program did not create the thread */
static void *MainThreadProc(void *arg) {
    WinDataType *mwd = (WinDataType *) arg;
    mwd->bThreadRun = TRUE;
    while (1) {
        if (!mwd->bThreadRun) {
            sm_debug("main thread end\n");
            pthread_exit((void*) 1);
            /* sleep(1); */
            sm_error("mainthread end\n");
        }
        /* usleep(1000 * 1000); */ /* Delay 1000ms */

    }
    sm_error("main thread end fail\n");
    return (void*) 0;
}

static s32 CreateMainThread(HWND hWnd, WinDataType *mwd) {
    s32 ret = 0;

    ret = pthread_create(&mwd->threadID, NULL, MainThreadProc, (void *) mwd);
    if (ret == -1) {
        sm_debug("create thread fail\n");
        return -1;
    }
    sm_debug("g_win_data->threadID=%p\n", mwd->threadID);

    return 0;
}

static s32 CloseMainThread(HWND hWnd, WinDataType *mwd) {
    mwd->bThreadRun = FALSE;
    pthread_join(mwd->threadID, NULL);
    mwd->threadID = 0;
    return 0;
}

static int MainWindowProc(HWND hWnd, int nMessage, WPARAM wParam, LPARAM lParam) {

    WinDataType *mwd;
    mwd = (WinDataType*) GetWindowAdditionalData(hWnd);

    switch (nMessage) {
    case MSG_CREATE: {
        /* CreateMainThread(hWnd, mwd); */
        TimeTitleViewInit(hWnd, 0);
        DeskButotnViewInit(hWnd, 0);

        /* The first time the program runs, first enter the screen saver window. */
        ActivityScreensavers(hWnd);
        break;
    }
    case MSG_LBUTTONUP: {
        /* Click on the time control to jump to the scene window */
        RECT rect;
        GetClientRect(hWnd, &rect);
        if (rect.right > 240) {
            if (HIWORD(lParam) < 146) {
                ActivityScene(hWnd);
            }
        } else {
            if (HIWORD(lParam) < 126) {
                ActivityScene(hWnd);
            }
        }
        break;
    }
    case MSG_DESTROY:
        /* CloseMainThread(hWnd, mwd); */
        if (g_win_data)
            free(g_win_data);
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        PostQuitMessage(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

void HardwareInit(void) {
    int ret;
    volume_init();
    volume_set_volume(24);
    ret = play_wav_music(VOICE_POWER_OFF);
    if (ret != 0) {
        printf("play %s fail\n", VOICE_POWER_ON);
    }
}

void SystemInit(void) {
    /* HardwareInit(); */
    ResourceInit();
}

void SystemUninit(void) {
    ResourceUninit();
    /*system("ubus call boot-play boot_complete '{\"stop\":true}'"); */
}

BOOL RegisterExWindows(void) {
    BOOL ret;
    ret = RegisterButtonView();
    ret = RegisterTimeTitleView();
    ret = RegisterDeskButotnView();
    ret = RegisterHeadbarView();
    ret = RegisterControlButotnView();
    ret = RegisterControlButotnsView();
    ret = RegisterProgressView();
    ret = RegisterSettingButtonView();
    ret = RegisterSwitchButtonView();
    ret = RegisterListItemSwitch();
    ret = RegisterSelectButtonView();
    ret = RegisterListItemSelect();
    ret = RegisterMyListView();
    ret = RegisterListItemArrow();
    ret = RegisterListItemMusic();
    return ret;
}

void UnRegisterExWindows(void) {
    UnregisterButtonView();
    UnregisterTimeTitleView();
    UnregisterDeskButotnView();
    UnregisterHeadbarView();
    UnregisterControlButotnView();
    UnregisterControlButotnsView();
    UnregisterProgressView();
    UnregisterSettingButtonView();
    UnregisterSwitchButtonView();
    UnregisterListItemSwitch();
    UnregisterSelectButtonView();
    UnregisterListItemSelect();
    UnregisterMyListView();
    UnregisterListItemArrow();
    UnregisterListItemMusic();
}

int MiniGUIMain(int argc, const char* argv[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
	JoinLayer(NAME_DEF_LAYER , SMART_MUSIC_PLAYER , 0 , 0);
#endif

    SystemInit();

    g_win_data = (WinDataType*) malloc(sizeof(WinDataType));
    if (NULL == g_win_data) {
        sm_error("malloc window data error\n");
        return -1;
    }
    memset((void *) g_win_data, 0, sizeof(WinDataType));

    if (!RegisterExWindows()) {
        sm_error("register windows failed\n");
        return -1;
    }

    smRect rect;
    getResRect(ID_SCREEN, &rect);

    SetDefaultWindowElementRenderer("classic");
    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = SMART_MUSIC_PLAYER;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0); /*GetSystemCursor (0);*/
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MainWindowProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = rect.w;
    CreateInfo.by = rect.h;
    CreateInfo.iBkColor = PIXEL_black;
    CreateInfo.hHosting = HWND_DESKTOP;
    CreateInfo.dwAddData = (DWORD) g_win_data;
    g_win_data->ParentHwnd = CreateMainWindow(&CreateInfo);
    if (g_win_data->ParentHwnd == HWND_INVALID)
        return -1;

    ShowWindow(g_win_data->ParentHwnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, g_win_data->ParentHwnd)) {
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup(g_win_data->ParentHwnd);
    UnRegisterExWindows();
    SystemUninit();

    return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

