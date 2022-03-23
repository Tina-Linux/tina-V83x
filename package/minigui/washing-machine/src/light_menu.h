#ifndef LIGHT_MENU_H_
#define LIGHT_MENU_H_

#include <unistd.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "washing_res_cn.h"
#define BORDER 0
#define LY 0
#define BY 280
#define RX 800
#define SY 100

static BITMAP volume_bar_n;
static BITMAP progress_p;
static BITMAP volume_light_bkg;
static BITMAP main_bmp_bkgnd;
static BITMAP yuyue_bkg;
static BITMAP tongsuo_bkg;
static BITMAP language_bkg;
static BITMAP sgi_bkg;
static BITMAP ztty_p;
//BITMAP volume_light_bkg;
#if 0
void Register_res(void)
{
	LoadBitmap(HDC_SCREEN,&progress_p, "/usr/res/menu/progress_p.png");
	LoadBitmap(HDC_SCREEN,&volume_light_bkg, "/usr/res/menu/volume_light_bkg.png");
	LoadBitmap(HDC_SCREEN,&volume_bar_n, "/usr/res/menu/volume_bar_n.png");
	LoadBitmap(HDC_SCREEN,&main_bmp_bkgnd, "/usr/res/menu/background.png");
}
#endif

void Register_res(void) {
	LoadBitmap(HDC_SCREEN, &progress_p, "/usr/res/menu/progress_p.png");
	LoadBitmap(HDC_SCREEN, &volume_light_bkg,
			"/usr/res/menu/volume_light_bkg.png");
	LoadBitmap(HDC_SCREEN, &volume_bar_n, "/usr/res/menu/volume_bar_n.png");
	LoadBitmap(HDC_SCREEN, &main_bmp_bkgnd, "/usr/res/menu/background.png");
	LoadBitmap(HDC_SCREEN, &yuyue_bkg, "/usr/res/menu/yuyue_bkg.png");
	LoadBitmap(HDC_SCREEN, &tongsuo_bkg, "/usr/res/menu/tongsuo_bkg.png");
	LoadBitmap(HDC_SCREEN, &language_bkg, "/usr/res/menu/language_bkg.png");
	LoadBitmap(HDC_SCREEN, &sgi_bkg, "/usr/res/menu/sgj_bkg.png");
	LoadBitmap(HDC_SCREEN, &ztty_p, "/usr/res/menu/ztty.png");
}
void unregister_res(void) {
	UnloadBitmap(&progress_p);
	UnloadBitmap(&volume_light_bkg);
	UnloadBitmap(&volume_bar_n);
	UnloadBitmap(&main_bmp_bkgnd);
	UnloadBitmap(&yuyue_bkg);
	UnloadBitmap(&tongsuo_bkg);
	UnloadBitmap(&language_bkg);
	UnloadBitmap(&sgi_bkg);
	UnloadBitmap(&ztty_p);
}

#endif
