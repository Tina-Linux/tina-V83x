#ifndef FEATURES_MENU_H_
#define FEATURES_MENU_H_

#include <unistd.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#if 1
#define LX 416
#define TY 0
#define RX 800
#define BY 480
#define WIDTH 384
#endif

static BITMAP zhineng_gongneng_menu[4];
static const char* m_zhineng_gongneng_menu[8] = { "/usr/res/menu/xidiji_n.png",
		"/usr/res/menu/roushunji_n.png", "/usr/res/menu/honggan_n.png",
		"/usr/res/menu/yuyue_n.png", "/usr/res/menu/jiasuxi_n.png",
		"/usr/res/menu/tezhixi_n.png", "/usr/res/menu/wutaixi_n.png",
		"/usr/res/menu/jingyinxi_n.png", };

#endif
