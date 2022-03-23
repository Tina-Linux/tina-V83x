#ifndef HEADBAR_VIEW_H_
#define HEADBAR_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <string.h>
#include <time.h>
#include "button_view.h"

#define HEAD_BAR_VIEW "headBarView"
#define ID_BUTTON_FAVOR 1
#define ID_BUTTON_HOME 2
#define ID_BUTTON_SETUP 3
#define ID_BUTTON_VIDEO 4
#define ID_BUTTON_SENSOR 5
#define ID_UPDATETIME 6
#define ID_BUTTON_VIDEOMEDIA 7
#define WIN_DES_MAIN "main"
#define WIN_DES_ZNXYZJ "znxizj"
#define WIN_DES_MIANMA "mianMa"
#define WIN_DES_HUNHEXI "hunHeXi"
#define WIN_DES_KUAIXI "kuaiXi"
#define WIN_DES_DAJIAN "daJian"
#define WIN_DES_CENSAN "cenSan"
#define WIN_DES_HOUXINMEI "houXinMei"
#define WIN_DES_YANMAO "yanMao"
#define WIN_DES_TONZHUAN "tonZhuan"
#define WIN_DES_NEIYI "neiYi"
#define WIN_DES_YURONFU "yuRonFu"
#define WIN_DES_JIENEN "jieNen"
#define WIN_DES_PIAOXITUOSHUI "piaoXiTouShui"
#define WIN_DES_DANTUOSHUI "danTuoShui"
#define WIN_DES_DANHONGAN "danHonGan"
#define WIN_DES_XIHONG "xiHong"
#define WIN_DES_KONGQIXI "kongQiXi"
#define WIN_DES_JIANZIJIE "jianZiJie"

#define WIN_DES_WIFIWND "wifiwnd"

#define WIN_DES_VIDEO "video"
#define WIN_DES_VIDEO_PLAY "videoplay"
#define WIN_DES_SENSOR "sensor"
#define MSG_CHAGE_WIFI_BMP 102
#define MSG_CHAGE_NOWIFI_BMP 103
#define MSG_CHAGE_HEADBAR 104
#define MSG_UPDATETIME 105

#define STATUS_ON 1
#define STATUS_OFF 0

HWND btn_wifi;
HWND headbarbtnwifi[19];
HWND switch_button;
HWND btn_home;
HWND btn_updatetime;
HWND btn_mediafile;
HWND headbarWnd;
int wifi_icon_change;
int status_switchbutton;
int UPDATETIME;
int TIMER_UPDATETIME_STATUS;
char szText[100];
time_t tim;
struct tm *ptm;

typedef struct {
	BITMAP bmpBg;
	BITMAP bmpWifi;
	BITMAP no_bmpWifi;
	BITMAP time_wait;
	BITMAP mediafile;
	ButtonView *favorData;
	ButtonView *homeData;
	ButtonView *setupData;
	ButtonView *videoData;
	ButtonView *sensorData;
	//ButtonView *videoplayData;
	char *winDes;
} HeadBarView;

BOOL RegisterHeadBarView(void);
void UnregisterHeadBarView(void);
void *update_time(void);
void *wifi_change(void);
HWND HeadBarViewInit(HWND hParent, HWND myBottomBarHwnd,
		HeadBarView *headBarData);
HWND getHeadBarHwnd();

#endif /* HEADBAR_VIEW_H_ */
