#ifndef RESERVATION_H_
#define RESERVATION_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <unistd.h>
#include <string.h>

#define BORDER 0
#define LY 0
#define BY 280
#define RX 800
#define SY 100

#define MSG_SLIDE_NUMBER_CHANGE (MSG_USER + 1)
typedef struct {
	BITMAP bmp_bg_slide_number;
	PLOGFONT logfont_ttf_times_p;
	PLOGFONT logfont_ttf_times_n;
	int minimal;
	int maximum;
	int selectNumber;
	BOOL isFullyOpen;
} SlideNumberView;

static BITMAP yuyue_bkg;
static BITMAP bmp_bottom_fun;

static BOOL register_reservation_pic(void) {
	if (LoadBitmap(HDC_SCREEN, &yuyue_bkg, "/usr/res/menu/yuyue_bkg.png")) {
		return FALSE;
	}
	if (LoadBitmap(HDC_SCREEN, &bmp_bottom_fun,
			"/usr/res/menu/bottom_fun.png")) {
		return FALSE;
	}
	return TRUE;
}
static void unregister_reservation_pic(void) {
	UnloadBitmap(&yuyue_bkg);
	UnloadBitmap(&bmp_bottom_fun);
}
#endif
