/*
 *
 * smart_ctrl.h
 *
 * xlf<xielinfei@allwinnertech.com>
 *
 * 2017-6-02
*/
#ifndef __SMART_RESOURCE_H__
#define __SMART_RESOURCE_H__

#include "common.h"
#define VOICE_POWER_ON 		"/usr/res/sound/on.wav"
#define VOICE_POWER_OFF 	"/usr/res/sound/chord.wav"
#define VOICE_TAKE_IMG 		"/usr/res/sound/on.wav"
#define VOICE_TAKE_KEY 		"/usr/res/sound/chord.wav"
#define VOICE_TAKE_NULL 	"/usr/res/sound/on.wav"

/* window name */
#define WINDOW_MAINPAGE 	"SmMainPageWin"
#define WINDOW_RUNSTAT		"SmRunStatWin"
#define WINDOW_INFOICON		"SmInfoIconWin"
#define WINDOW_STATBAR 		"SmStatbarWin"
#define WINDOW_HEADBAR 		"SmheadbarWin"
#define WINDOW_SETTEMP 		"SmsetTempWin"

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
#define IDC_SET_TEMPER 212
#define IDC_SET_TEMPER_UP 213
#define IDC_SET_TEMPER_DOWN 214

#define IDC_RETURN 215
#define IDC_STATUS_LOCK 216
#define IDC_DATE 217
#define IDC_CUR_TIME 218

/*Reserve 7 length for mode page*/
#define IDC_MODE_ITEM_COOL      0
#define IDC_MODE_ITEM_HEAT      1
#define IDC_MODE_ITEM_AUTO      2
#define IDC_MODE_ITEM_CHUSHI    3
#define IDC_MODE_ITEM_STRONGCS  4
#define IDC_MODE_ITEM_DRY  		5

#define TIMER_SEC           1000

#define MSG_CHANGE_ICONS    (MSG_USER + 10)
#define MSG_CHANGE_PAGE     (MSG_USER + 11)
#define MSG_SHOW_STATBAR     (MSG_USER + 12)
#define MSG_SHOW_ITEMS     (MSG_USER + 13)
#define MSG_SET_LOCKICON     (MSG_USER + 14)
#define MSG_ITEMS_RETURN     (MSG_USER + 15)


#define TIME_POS_X          315
#define TIME_POS_Y          0

#define TZN_MODE_BG				"bg_mode.png"
#define TZN_WIND_BG				"bg_wind.png"
#define TZN_WIND_DIRECT_BG		"bg_wind_direct.png"
#define TZN_STOP_BG				"bg_backlight1.png"

#define TZN_AUTO_CHU_SHI_BG		"bg_dry1.png"
#define TZN_CHILD_LOCK_BG		"bg_child_lock1.png"
#define TZN_SET_BG				"bg_set1.png"
#define TZN_MORE_BG				"bg_backlight1.png"

#define TZN_RETURN_HEAD			"bt_return_normal.png"

#define ICON_PATH_SIZE	64
#define ICON_TOTAL_CNT	8

#define CFG_BAR_STAT 			"status_bar"
#define CFG_BAR_STAT_DATE 		"status_bar_date"
#define CFG_BAR_STAT_CUR_TIME 	"status_bar_cur_time"
#define CFG_BAR_STAT_LOCK 		"status_bar_lock_stat"
#define CFG_BAR_STAT_SHAPE 		"status_bar_shape"
#define CFG_BAR_STAT_PREOPEN 	"status_bar_preopen"
#define CFG_BAR_STAT_BACK 		"status_bar_back"
#define CFG_BAR_STAT_TITLE 		"status_bar_title"
#define CFG_BAR_STAT_MODE_ICON 	"status_bar_mode_icon"
#define CFG_BAR_STAT_MODE_LABEL "status_bar_mode_label"

#define CFG_BAR_STAT_WIND_ICON 	"status_bar_wind_icon"
#define CFG_BAR_STAT_WIND_LABEL "status_bar_wind_label"

#define CFG_BAR_STAT_DIRECT_ICON "status_bar_direct_icon"
#define CFG_BAR_STAT_DIRECT_LABEL  "status_bar_direct_label"



#define CFG_BACKGROUND_BMP 		"screen_bg"
#define CFG_HEAD_BACKGROUND_BMP "screen_head_bg"

#define CFG_MAIN_PAGE 			"mode_page"
#define CFG_MAINBTN_LEFT 		"mode_btn_left"
#define CFG_MAINBTN_RIGHT 		"mode_btn_right"

#define CFG_MAINBTN_ITEM1 		"main_btn_item1"
#define CFG_MAINBTN_ITEM2 		"main_btn_item2"
#define CFG_MAINBTN_ITEM3 		"main_btn_item3"
#define CFG_MAINBTN_ITEM4 		"main_btn_item4"

#define CFG_TEMPSET 			"temp_set"
#define CFG_TEMPSET_UP 			"temp_set_btn_up"
#define CFG_TEMPSET_DOWN 		"temp_set_btn_down"
#define CFG_TEMPSET_DEGREE 		"temp_set_label_degree"

#define FGC_WIDGET				"fgc_widget"
#define BGC_WIDGET				"bgc_widget"
#define BGC_ITEM_FOCUS			"bgc_item_focus"
#define BGC_ITEM_NORMAL			"bgc_item_normal"
#define MAINC_THREED_BOX		"mainc_3dbox"
#define LINEC_ITEM				"linec_item"
#define LINEC_TITLE				"linec_title"
#define STRINGC_NORMAL			"stringc_normal"
#define STRINGC_SELECTED		"stringc_selected"
#define VALUEC_NORMAL			"valuec_normal"
#define VALUEC_SELECTED			"valuec_selected"
#define SCROLLBARC				"scrollbarc"
#define DATE_NUM				"date_num"
#define DATE_WORD				"date_word"

#define FGC_HEAD_WIDGET				"head_fgc_widget"
#define BGC_HEAD_WIDGET				"head_bgc_widget"


enum ColorType{
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

typedef enum
{
	MSG_STATBAR_UPDATAE_TITLE 	= 0x0800,
	MSG_HBAR_UPDATAE_MODE  		= 0x0861,
	MSG_HBAR_UPDATAE_FONT 		= 0x0862,
}UserMsgType;

enum FontID{
	ID_FONT_TIMES_16 = 0,
	ID_FONT_TIMES_45,
	ID_FONT_TIMES_150,
};

enum ResourceID {
	ID_SCREEN	= 0,
	ID_HEAD_BAR,
	ID_SCREEN_BKG,
	ID_HEAD_BKG,
	ID_STATUSBAR,
	ID_STATUSBAR_ICON_DATE,
	ID_STATUSBAR_CUR_TIME,
	ID_STATUSBAR_LOCK,
	ID_STATUSBAR_SHAPE,
	ID_STATUSBAR_PREOPEN,
	ID_STATUSBAR_BACK,
	ID_STATUSBAR_TITLE,
	ID_STATUSBAR_ICON_ITEMS,
	ID_STATUSBAR_FONT_ITEMS,
	ID_STATUSBAR_ICON_WIND,
	ID_STATUSBAR_FONT_WIND,
	ID_STATUSBAR_ICON_DIRECT,
	ID_STATUSBAR_FONT_DIRECT,
	ID_MAINPAGE,
	ID_MAINPAGE_BTN_LEFT,
	ID_MAINPAGE_BTN_RIGHT,
	ID_MAINPAGE_BTN_ITEM1,
	ID_MAINPAGE_BTN_ITEM2,
	ID_MAINPAGE_BTN_ITEM3,
	ID_MAINPAGE_BTN_ITEM4,
	ID_MAINPAGE_FONT_ITEM1,
	ID_MAINPAGE_FONT_ITEM2,
	ID_MAINPAGE_FONT_ITEM3,
	ID_MAINPAGE_FONT_ITEM4,
	ID_TEMP_SETTING,
	ID_TEMP_SETTING_UP,
	ID_TEMP_SETTING_DOWN,
	ID_TEMP_SETTING_DEGREE,
};

enum BmpType
{
	BMPTYPE_BASE = 0,
	BMPTYPE_UNSELECTED,
	BMPTYPE_SELECTED,
	BMPTYPE_WINDOWPIC_MODE,
	BMPTYPE_WINDOWPIC_WIND,
	BMPTYPE_WINDOWPIC_WIND_DIRECT,
};

typedef enum {
	WINDOW_MAINPAGE_ID = 0,
	WINDOW_HEADBAR_ID,
	WINDOW_STATBAR_ID,
	WINDOW_SETTEMP_ID,
	WINDOW_RUNSTAT_ID,
	WINDOW_INFOICON_ID,
}WindowIDType;

#define SelfCtrlWinCnt		4

typedef struct
{
	unsigned int current;
	char *iconName[ICON_TOTAL_CNT];
}currentIcon_t;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
}smRect;

typedef struct {
	const char* mainkey;
	void* addr;
}configTable2;

typedef struct {
	const char* mainkey;
	const char* subkey;
	void* addr;
}configTable3;

typedef struct {
		unsigned int current;
		unsigned int count;

}menuItem_t;

typedef struct
{
	smRect STBRect;
	gal_pixel bgc;
	gal_pixel fgc;

	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	gal_pixel label1Fgc;
	gal_pixel label2Fgc;
	gal_pixel reserveLabelFgc;

	smRect dateRect;
 	smRect curTimeRect;
	smRect reserveLabelRect;

	smRect lockRect;
	currentIcon_t lockIcon;

	smRect shapeRect;
	currentIcon_t shapeIcon;

	smRect preOpenRect;
	currentIcon_t preOpenIcon;

	smRect backRect;
	currentIcon_t backIcon;

	smRect titleRect;
	currentIcon_t titleIcon;

	smRect statItemsRect;
	currentIcon_t statItemsIcon;

	smRect statItemsFontRect;
	currentIcon_t statItemsFontIcon;

	currentIcon_t statWindIcon;
	currentIcon_t statWindFontIcon;

	currentIcon_t statDirectIcon;
	currentIcon_t statDirectFontIcon;

	unsigned int dateTileRectH;
	unsigned int dateHBorder;
	unsigned int dateYearW;
	unsigned int dateLabelW;
	unsigned int dateNumberW;
	unsigned int dateBoxH;

}StatusBarDataType;

typedef struct
{
	smRect setRect;
	gal_pixel bgc;
	gal_pixel fgc;
	smRect btnUpRect;
	currentIcon_t Iconup;

	smRect btnDownRect;
	currentIcon_t Icondown;

	smRect labelDegreeRect;
	currentIcon_t IconDegree;
}TempSetDataType;

typedef struct
{
	smRect pgeRect;
	gal_pixel bgc;
	gal_pixel fgc;
	smRect btnLeftRect;
	currentIcon_t Iconleft;
	smRect btnRightRect;
	currentIcon_t Iconright;


	smRect btn_item[4];
	gal_pixel itembgc[8];
	gal_pixel itemfgc[8];
	gal_pixel itemselect[8];

	currentIcon_t Icon_item[4];
}MainPageDataType;

typedef struct
{
	HWND mHwnd[4];

	char configFile[32];
	GHANDLE mCfgFileHandle;

	char configMenu[32];
	char defaultConfigMenu[32];
	GHANDLE mCfgMenuHandle;

	smRect rScreenRect;
	smRect rheadbarRect;

	currentIcon_t headbgImg;
	currentIcon_t bgImg;

	//LANGUAGE lang;
	PLOGFONT mLogFont_Times16;
	PLOGFONT mFont_Times45;
	PLOGFONT mFont_Times150;
	PLOGFONT mCurLogFont;

	StatusBarDataType rStatusBarData;
	MainPageDataType rMainPgeData;
	TempSetDataType rTempSetData;
}ResDataType;

void setCurrentIconValue(enum ResourceID resID, int cur_val);
gal_pixel getResColor(enum ResourceID resID, enum ColorType type);
int getResRect(enum ResourceID resID, smRect *rect);
int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp);
void setHwnd(unsigned int win_id, HWND hwnd);
PLOGFONT getLogFont(enum FontID id);
void unloadBitMap(BITMAP *bmp);

void ResourceInit(void);
void ResourceUninit(void);

BOOL register_all_pic (void);
void unregister_all_pic (void);

#endif

