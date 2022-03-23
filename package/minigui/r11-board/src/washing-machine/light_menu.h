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

#define rotate_TIMERID 109
#define IP_CFG_SIZE 64
static BITMAP volume_bar_n;
static BITMAP progress_p;
static BITMAP volume_light_bkg;
static BITMAP main_bmp_bkgnd;
static BITMAP yuyue_bkg;
static BITMAP tongsuo_bkg;
static BITMAP language_bkg;
static BITMAP sgi_bkg;
static BITMAP ztty_p;
static pthread_t pthread_ID;

int bool_connect;
int ROTATE_STATUS;
int first_connect;
int GetIpAdr(char *str);
int GetIpPsw(char *str);
char get_wifi_name[128];
HWND GetlightMenuHwnd();

#endif
