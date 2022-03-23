#ifndef WIFIWINDOW_H
#define WIFIWINDOW_H

#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgncs/mgncs.h>
#include <mgeff/mgeff.h>

#include <mgncs4touch/mgncs4touch.h>
#include <mgncs4touch/mtouchdebug.h>
#include "msimpletableviewpiece.h"
#include "washing_res_cn.h"
#define ID_WIFIWIN 1
#define ID_SWBUTTON 1
#define ID_WIFIBACK 0
#define ID_WIFIWAIT 2
#define ID_WIFIITEM 3
#define ID_WIFILIST 4
#define TIMER_ID 10
#define REFRESH_TIMER_ID 11
#define SPINNING_SPEED 380

#define TIMER_WIFIOFF_PROMPT 112
#define TIMER_PASSWORD_ERROR 113
#define TIMER_WIFI_EWADON 114

#define MSG_WIFIWAIT 101
#define MSG_CREATE_ITEM 103
#define MSG_WIFI_PASSWORD 106
#define MSG_PASSWORD_ERROR 105
#define MSG_REFASH_ITEM 107
#define MSG_SWITCH_ONOFF 110
#define MSG_CONNECT_ERROR 111
#define MSG_MOVE_TOP 112
#define MSG_DISCONNECT_AP 113
#define MSG_WIFI_READON 114
#define TRUE 1
#define FALSE 0
#define MAX_TABLEVIEW_NUM 128
#define ITEM_W 740
#define ITEM_H 50

BOOL WIFIWIN_OPEN;
BOOL STATUS_WIFI_ITEM_OPEN;

HWND wifi_waitbutton;
HWND wifi_item[MAX_TABLEVIEW_NUM];
HWND WifiWnd;
HWND password_errorwnd;
HWND connect_errorwnd;
HWND wifi_readonwnd;
HWND switch_button;
HWND headbarwnd;

static BITMAP wifi_wait;
static BITMAP bmp_left;

int status_wifilist;
int wifi_list_num;
int password_error_status;
int light_menu_flag;
int first_scan;
int first_wifion;
int switch_on_num;
int ctnr_wnd_close;
int text_connecting;
int wifi_scan_noend;
static int angle = 0;

char connect_wifiname[64];
static mSimpleTableView* table;
static mSimpleTableView* table_index;
mPanelPiece* panel;
mHotPiece *txt_piece;
typedef struct _itemInfo{
    char *key;
    char *text;
    char *picture;
    int index;
}ItemInfo;

typedef struct _sectionInfo{
    char* index;
    int num;
    ItemInfo* rows;
}SectionInfo;
typedef struct _wifiinfo{
	char *wifi_name;
	char *wifi_password;
	int selectindex;
}Wifiinfo;
struct wifi_connect_name{
	char wifi_name[48];
	char *picture;
};
struct wifi_connect_name wifi_connect_array[128];
mTableViewItemPiece* item;
Wifiinfo wifiinfo;
mContainerCtrl* ctnr;
int wifiselectindex;
int listinfo_index;
HWND GetWifiWnd();
HWND GetContainerWnd();
void switchbutton_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
#endif

