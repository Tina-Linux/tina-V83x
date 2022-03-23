/*
 *
 * resource.h
 *
 * xlf<xielinfei@allwinnertech.com>
 *
 * 2017-9-09
 */
#ifndef __SMART_RESOURCE_H__
#define __SMART_RESOURCE_H__

#include "common.h"

#define VOICE_POWER_ON 		"/usr/res/sound/on.wav"
#define VOICE_POWER_OFF 	"/usr/res/sound/chord.wav"

/* window name */
#define WINDOW_HEADBAR 		"SmheadbarWin"
#define WINDOW_SLIDER 		"SliderWin"
#define WINDOW_POP_MENU 	"PopWin"

/* window control */
#define SelfCtrlWinCnt		1

/* Define Window's id */
#define IDC_MODE           202
#define IDC_WIND           203
#define IDC_WIND_DIRECT    204
#define IDC_STOP           205

#define IDC_AUTO_CHU_SHI   206
#define IDC_CHILD_LOCK     207
#define IDC_SET    		   208
#define IDC_MORE           209

#define IDC_LEFT           210
#define IDC_RIGHT          211

#define ICON_PATH_SIZE	64
#define ICON_TOTAL_CNT	8

/* Define keywords match to system.cfg */
#define CFG_BACKGROUND_BMP 		"screen_bg"
#define CFG_HEAD_BACKGROUND_BMP "screen_head_bg"
#define CFG_FUN_BACKGROUND_BMP1 "fun_screen_bg1"
#define CFG_FUN_BACKGROUND_BMP2 "fun_screen_bg2"
#define CFG_FUN_BACKGROUND_BMP3 "fun_screen_bg3"

#define CFG_FAST_WIN_AREA 	"fast_win_area"
#define CFG_FAST_WIN_FAVOR 	"fast_win_favor"
#define CFG_FAST_WIN_HOME 	"fast_win_home"
#define CFG_FAST_WIN_SETUP 	"fast_win_setup"
#define CFG_FAST_WIN_WIFI 	"fast_win_wifi"

#define CFG_ACT_WIN_AREA 		"active_win_area"
#define CFG_ACT_SUB1_WIN_AREA 	"active_sub1win_area"
#define CFG_ACT_SUB2_WIN_AREA 	"active_sub2win_area"
#define CFG_ACT_SUB1_WIN_ITEM0 	"active_sub1win_item0"
#define CFG_ACT_SUB1_WIN_ITEM1 	"active_sub1win_item1"
#define CFG_ACT_SUB1_WIN_ITEM2 	"active_sub1win_item2"
#define CFG_ACT_SUB2_WIN_ITEM0 	"active_sub2win_item0"
#define CFG_ACT_SUB2_WIN_ITEM1 	"active_sub2win_item1"
#define CFG_ACT_SUB2_WIN_ITEM2  "active_sub2win_item2"

#define CFG_POP_MENU_AREA 	"pop_menu_area"
#define CFG_POP_MENU_BUTTON1 "pop_menu_button1"
#define CFG_POP_MENU_BUTTON2 "pop_menu_button2"
#define CFG_POP_MENU_BUTTON3 "pop_menu_button3"
#define CFG_POP_MENU_BUTTON4 "pop_menu_button4"
#define CFG_POP_MENU_BUTTON5 "pop_menu_button5"
#define CFG_POP_MENU_BUTTON6 "pop_menu_button6"
#define CFG_POP_MENU_BUTTON7 "pop_menu_button7"
#define CFG_POP_MENU_BUTTON8 "pop_menu_button8"
#define CFG_POP_MENU_BUTTON9 "pop_menu_button9"
#define CFG_POP_MENU_BUTTON10 "pop_menu_button10"
#define CFG_POP_MENU_BUTTON11 "pop_menu_button11"
#define CFG_POP_MENU_BUTTON12 "pop_menu_button12"

#define CFG_FET_MENU1 "fet_menu1_area"
#define CFG_FET_MENU1_BUTTON1 "fet_menu1_button1"
#define CFG_FET_MENU1_BUTTON2 "fet_menu1_button2"
#define CFG_FET_MENU1_BUTTON3 "fet_menu1_button3"
#define CFG_FET_MENU1_BUTTON4 "fet_menu1_button4"
#define CFG_FET_MENU1_BUTTON5 "fet_menu1_button5"
#define CFG_FET_MENU1_BUTTON6 "fet_menu1_button6"
#define CFG_FET_MENU1_BUTTON7 "fet_menu1_button7"
#define CFG_FET_MENU1_BUTTON8 "fet_menu1_button8"

#define CFG_FUN_WIN_BUTTON1 "fun_win_button1"
#define CFG_FUN_WIN_BUTTON2 "fun_win_button2"
#define CFG_FUN_WIN_BUTTON3 "fun_win_button3"
#define CFG_FUN_WIN_BUTTON4 "fun_win_button4"
#define CFG_FUN_WIN_ANIM "fun_win_anim"

#define CFG_BOTTOM_STAT_AREA "bottom_stat_area"
#define CFG_BOTTOM_PAGE_INDEX "bottom_page_index"

#define FGC_WIDGET 			"fgc_widget"
#define BGC_WIDGET 			"bgc_widget"
#define FGC_HEAD_WIDGET 	"head_fgc_widget"
#define BGC_HEAD_WIDGET 	"head_bgc_widget"

#define NUM_WND 6
HWND hbtnWndPageOne[NUM_WND];
HWND hbtnWndPageTwo[NUM_WND];
HWND hbtnWndPageThree[NUM_WND];
HWND hbtnEditPageOne[NUM_WND];
HWND hbtnEditPageTwo[NUM_WND];
HWND hbtnEditPageThree[NUM_WND];
HWND hbtnMenuPopWnd[NUM_WND];
HWND hpageOne[3];
enum ColorType {
	COLOR_BGC = 0,
	COLOR_FGC,
	COLOR_FGC_LABEL,
	COLOR_FGC_NUMBER,
	COLOR_BGC_ITEMFOCUS,
	COLOR_BGC_ITEMNORMAL,
	COLOR_MAIN3DBOX,
	COLOR_LINEC_ITEM,
	COLOR_LINEC_TITLE,
	COLOR_STRINGC_NORMAL,
	COLOR_STRINGC_SELECTED,
	COLOR_SCROLLBARC,
	COLOR_VALUEC_NORMAL,
	COLOR_VALUEC_SELECTED,
	COLOR_BORDERC_NORMAL,
	COLOR_BORDERC_SELECTED,
};

/* define user Message */
typedef enum {
	MSG_STATBAR_UPDATAE_TITLE = 0x0801,
	MSG_HOME_SETUP,
	MSG_HOME_CLICK,
	MSG_HBAR_FAVOR,
	MSG_MANY_WIN_CLICK,
	MSG_SLIDE_TEXT_CHANGE,
	MSG_STATIC_CLICK,
	MSG_WIN_POP_CLOSE,
	MSG_LIGHT_MENU_CLOSE,
} UserMsgType;

enum FontID {
	ID_FONT_SIMSUN_20 = 0,
	ID_FONT_SIMSUN_25,
	ID_FONT_SIMSUN_30,
	ID_FONT_SIMSUN_40,
	ID_FONT_SIMSUN_45,
	ID_FONT_SIMSUN_50,
	ID_FONT_SIMSUN_55,
	ID_FONT_SIMSUN_120,
};

enum ResourceID {
	ID_SCREEN = 0,
	ID_SCREEN_BG,

	ID_FAST_WIN,
	ID_FAST_WIN_BG,
	ID_FAST_WIN_FAVOR,
	ID_FAST_WIN_HOME,
	ID_FAST_WIN_SETUP,
	ID_FAST_WIN_WIFI,

	ID_ACT_WIN,
	ID_ACT_SUB1_WIN,
	ID_ACT_SUB2_WIN,
	ID_ACT_SUB1_WIN_ITEM0,
	ID_ACT_SUB1_WIN_ITEM1,
	ID_ACT_SUB1_WIN_ITEM2,
	ID_ACT_SUB2_WIN_ITEM0,
	ID_ACT_SUB2_WIN_ITEM1,
	ID_ACT_SUB2_WIN_ITEM2,

	ID_POP_WIN,
	ID_POP_WIN_BTN1,
	ID_POP_WIN_BTN2,
	ID_POP_WIN_BTN3,
	ID_POP_WIN_BTN4,
	ID_POP_WIN_BTN5,
	ID_POP_WIN_BTN6,
	ID_POP_WIN_BTN7,
	ID_POP_WIN_BTN8,
	ID_POP_WIN_BTN9,
	ID_POP_WIN_BTN10,
	ID_POP_WIN_BTN11,
	ID_POP_WIN_BTN12,

	ID_FETURES_WIN,
	ID_FETURES1_WIN_BTN,
	ID_FETURES2_WIN_BTN,
	ID_FETURES3_WIN_BTN,
	ID_FETURES1_WIN_BTN_ITEM0,
	ID_FETURES1_WIN_BTN_ITEM1,
	ID_FETURES1_WIN_BTN_ITEM2,
	ID_FETURES1_WIN_BTN_ITEM3,

	ID_FUN_SCREEN_BG1,
	ID_FUN_SCREEN_BG2,
	ID_FUN_SCREEN_BG3,
	ID_FUN_WIN_BTN1,
	ID_FUN_WIN_BTN2,
	ID_FUN_WIN_BTN3,
	ID_FUN_WIN_BTN4,
	ID_FUN_WIN_ANIM,

	ID_FETURES1,
	ID_FETURES1_BTN1,
	ID_FETURES1_BTN2,
	ID_FETURES1_BTN3,
	ID_FETURES1_BTN4,
	ID_FETURES1_BTN5,
	ID_FETURES1_BTN6,
	ID_FETURES1_BTN7,
	ID_FETURES1_BTN8,

	ID_BM_STAT_WIN,
	ID_BM_STAT_WIN_PAGE_INDEX,

	ID_EDIT_FONT1,
	ID_EDIT_FONT2,
	ID_EDIT_FONT3,
	ID_EDIT_FONT4,
	ID_EDIT_FONT5,
	ID_EDIT_FONT6,

	ID_FEATURES0,
};

enum BmpType {
	BMPTYPE_BASE = 0,
	BMPTYPE_UNSELECTED,
	BMPTYPE_SELECTED,
	BMPTYPE_WINDOWPIC_MODE,
	BMPTYPE_WINDOWPIC_WIND,
	BMPTYPE_WINDOWPIC_WIND_DIRECT,
};

typedef enum {
	WINDOW_HEADBAR_ID = 0, WINDOW_SLIDER_ID, WINDOW_POP_ID,
} WindowIDType;

typedef struct {
	unsigned int current;
	char *iconName[ICON_TOTAL_CNT];
} currentIcon_t;

typedef struct {
	int x;
	int y;
	int w;
	int h;
} smRect;

typedef struct {
	const char* mainkey;
	void* addr;
} configTable2;

typedef struct {
	const char* mainkey;
	const char* subkey;
	void* addr;
} configTable3;

typedef struct {
	unsigned int current;
	unsigned int count;
} menuItem_t;

typedef struct {
	smRect winRect;

	gal_pixel bgc;
	gal_pixel fgc;
	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	smRect favorRect;
	currentIcon_t favorIcon;

	smRect homeRect;
	currentIcon_t homeIcon;

	smRect setupRect;
	currentIcon_t setupIcon;

	currentIcon_t wifiIcon;
} FastMenuDataType;

typedef struct {

	currentIcon_t bgFunImg[3];
	currentIcon_t btnIcon[4];
	currentIcon_t animIcon;

} FunWinDataType;

typedef struct {
	smRect winRect;
	smRect winSub1Rect;
	smRect winSub2Rect;

	gal_pixel bgc;
	gal_pixel fgc;
	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	smRect btnRect[6];
	currentIcon_t btnIcon[6];
	currentIcon_t indexIcon;
} ActMenuDataType;

typedef struct {
	smRect winRect;

	gal_pixel bgc;
	gal_pixel fgc;
	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	smRect btnRect[12];
	currentIcon_t btnIcon[12];
	currentIcon_t btnIcon1[8];
} PopWinDataType;

typedef struct {
	smRect winRect;

	gal_pixel bgc;
	gal_pixel fgc;
	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	smRect btnRect[8];
	currentIcon_t btnIcon[8];
} FetWinDataType;

typedef struct {
	HWND mHwnd[4];

	char configFile[32];
	GHANDLE mCfgFileHandle;

	smRect rScreenRect;
	smRect rheadbarRect;
	smRect rbottombarRect;

	currentIcon_t headbgImg;
	currentIcon_t bgImg;

	smRect pageIdxRect;
	currentIcon_t pageIdxImg;

	PLOGFONT mFont_SimSun20;
	PLOGFONT mFont_SimSun25;
	PLOGFONT mFont_SimSun30;
	PLOGFONT mFont_SimSun40;
	PLOGFONT mFont_SimSun45;
	PLOGFONT mFont_SimSun50;
	PLOGFONT mFont_SimSun55;
	PLOGFONT mFont_SimSun120;
	PLOGFONT mCurLogFont;

	FastMenuDataType rFastMenuData;
	ActMenuDataType rActMenuData;
	PopWinDataType rPopWinData;
	FunWinDataType rFunWinData;
} ResDataType;

void setHwnd(unsigned int win_id, HWND hwnd);
void setCurrentIconValue(enum ResourceID resID, int cur_val);
gal_pixel getResColor(enum ResourceID resID, enum ColorType type);
int getResRect(enum ResourceID resID, smRect *rect);
PLOGFONT getLogFont(enum FontID id);
int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp);
void unloadBitMap(BITMAP *bmp);

void ResourceInit(void);
void ResourceUninit(void);
#endif

