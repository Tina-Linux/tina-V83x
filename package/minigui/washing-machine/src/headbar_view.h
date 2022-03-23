#ifndef HEADBAR_VIEW_H_
#define HEADBAR_VIEW_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <string.h>
#include "button_view.h"

#define HEAD_BAR_VIEW "headBarView"
#define ID_BUTTON_FAVOR 1
#define ID_BUTTON_HOME 2
#define ID_BUTTON_SETUP 3
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

typedef struct {
	BITMAP bmpBg;
	BITMAP bmpWifi;
	ButtonView *favorData;
	ButtonView *homeData;
	ButtonView *setupData;
	char *winDes;
} HeadBarView;

BOOL RegisterHeadBarView(void);
void UnregisterHeadBarView(void);
HWND HeadBarViewInit(HWND hParent, HWND myBottomBarHwnd,
		HeadBarView *headBarData);

#endif /* HEADBAR_VIEW_H_ */

