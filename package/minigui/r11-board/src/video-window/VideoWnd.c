#include "VideoWnd.h"
#include "middle_ware.h"
#include "videowindow.h"
HWND videoWnd;
static HeadBarView *headBarData = NULL;
int myvideo_flag;
extern char *filepath_list[100];
extern int bool_connect;
void *playerinit(void)
{
	tplayer_init(TPLAYER_VIDEO_ROTATE_DEGREE_0);
	//pthread_exit(NULL);
}
void *readplay(int index)
{
	if(myvideo_flag == NATIVE_VIDEO){
	tplayer_play_url(filepath_list[index]);
	tplayer_play();
            tplayer_setlooping(TRUE);
	}else if(myvideo_flag == URL_VIDEO){
		if(bool_connect == 0){
			tplayer_play_url(mediaFile[index]);
        		tplayer_play();
            		tplayer_setlooping(TRUE);
		}else{
			InvalidateRect(GetVideoWnd(), NULL, TRUE);
		}
	}
	//pthread_exit(NULL);
}
void media_notify_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
	if (nc == STN_CLICKED){
			if(myvideo_flag == URL_VIDEO && bool_connect != 0){

			}else{
                        	MediafilePopWin(hwnd, myvideo_flag);
			}
	}
}
#define __ID_TIMER_SLIDER 100
static int VideowndProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static PLOGFONT Font_45;
	switch (message){
	case MSG_CREATE:{
		pthread_create(&pthread_playerinit, NULL, &playerinit, NULL);
		SetTimer(hwnd, __ID_TIMER_SLIDER, 1);
		break;
	}
	case MSG_TIMER:{
		KillTimer(hwnd, __ID_TIMER_SLIDER);
		headBarData->winDes = WIN_DES_VIDEO_PLAY;
                video_headbarwnd = HeadBarViewInit(hwnd, 0, headBarData);
                SendMessage(video_headbarwnd, MSG_UPDATETIME, NULL, NULL);
		break;
	}
	case MSG_PAINT:{
		hdc = BeginPaint(hwnd);
		if(myvideo_flag == URL_VIDEO){
               		 if(bool_connect != 0){
               		         Font_45 = getLogFont(ID_FONT_SIMSUN_45);
               		         SelectFont(hdc, Font_45);
               		         SetBkMode(hdc, BM_TRANSPARENT);
               		         SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 0, 0));
               		         TextOut(hdc, 100, 250, NET_WARNNING);
               		 }
		}else{
			//return 0;
		}
		EndPaint(hwnd, hdc);
		break;
	}
	case MSG_LBUTTONDOWN:{
		if (LOWORD(lParam) < 416 && IsWindowEnabled(GetMediafileHwnd())
                        && IsWindowVisible(GetMediafileHwnd())){
			return 0;
                }
		break;
	}
	case MSG_LBUTTONUP:{
		if (LOWORD(lParam) < 416 && IsWindowEnabled(GetMediafileHwnd())
			&& IsWindowVisible(GetMediafileHwnd())){
			PostMessage(GetMediafileHwnd(), MSG_CLOSE, 0, 0);
			return 0;
		}
		break;
	}
	case MSG_PLAYVIDEO:{
		int index = (int)lParam;
		printf("index = %d\n", index);
		pthread_create(&pthread_readplay, NULL, &readplay, index);
		break;
	}
	case MSG_DESTROY:{
                DestroyAllControls(hwnd);
                return 0;
        }
        case MSG_CLOSE: {
		tplayer_stop();
		tplayer_exit();
                DestroyMainWindow(hwnd);
                PostQuitMessage(0);
                return 0;
        }
	}
	return DefaultMainWinProc(hwnd, message, wParam, lParam);
}
HWND GetVideoWnd(void)
{
	return videoWnd;
}
int VideoWnd(HWND hosting, int video_flag, HeadBarView *myHeadBarData)
{
        MSG Msg;
        MAINWINCREATE CreateInfo;
	myvideo_flag = video_flag;
	headBarData = myHeadBarData;
        CreateInfo.dwStyle = WS_NONE;
        CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
        CreateInfo.spCaption = "";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = VideowndProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = PIXEL_black;

        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hosting;

        headBarData = myHeadBarData;
        videoWnd = CreateMainWindow(&CreateInfo);
	if (videoWnd == HWND_INVALID)
                return -1;
        ShowWindow(videoWnd, SW_SHOWNORMAL);

        while (GetMessage(&Msg, videoWnd)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
        }

        MainWindowThreadCleanup(videoWnd);
        return 0;
}
