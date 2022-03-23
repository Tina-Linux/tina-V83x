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

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

#define VOICE_POWER_ON 		"/usr/res/sound/on.wav"
#define VOICE_POWER_OFF 	"/usr/res/sound/chord.wav"

/* window name */
#define WINDOW_HEADBAR 		"SmheadbarWin"
#define WINDOW_SLIDER 		"SliderWin"
#define WINDOW_POP_MENU 	"PopWin"

/* window control */
#define SelfCtrlWinCnt		1

#define ALLWINNERTECH_R11 0
#define ALLWINNERTECH_R16 1
#define ALLWINNERTECH_R18 2
#define ALLWINNERTECH_R40 3
#define ALLWINNERTECH_F35 4

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
#define CFG_PLATFORM_NUM 		"platform_num"

#define CFG_BOTTOM_WIN_AREA 	"bottom_win_area"
#define CFG_BOTTOM_WIN_BACK 	"bottom_win_back"
#define CFG_BOTTOM_WIN_SETUP 	"bottom_win_setup"
#define CFG_BOTTOM_WIN_PHOTO 	"bottom_win_photo"
#define CFG_BOTTOM_WIN_AUDIO 	"bottom_win_audio"
#define CFG_BOTTOM_WIN_VIDEO 	"bottom_win_vdieo"

#define CFG_FUN_BUTTON_AREA 		"fun_button_area"
#define CFG_FUN_BUTTON_TPHOTO 		"fun_button_tphoto"
#define CFG_FUN_BUTTON_TAUDIO 		"fun_button_taudio"
#define CFG_FUN_BUTTON_TVIDEO		"fun_button_tvideo"
#define CFG_FUN_BUTTON_TOW_TVIDEO 	"fun_button_tow_tvideo"
#define CFG_FUN_BUTTON_PHOTO 		"fun_button_photo"
#define CFG_FUN_BUTTON_AUDIO 		"fun_button_audio"
#define CFG_FUN_BUTTON_VIDEO 		"fun_button_video"

#define CFG_FRONT_CAMRA_SIZE	"front_camera_size"
#define CFG_BACK_CAMRA_SIZE		"back_camera_size"

#define CFG_HEAD_BAR_SD	"head_bar_sd"
#define CFG_HEAD_BAR_UD	"head_bar_ud"

#define CFG_MOUNT_PATH 	"mount_path"

#define CFG_PLAYER_WINDOW_AREA       "player_window_area"
#define CFG_PLAYER_BOTTOM_MENU "player_bottom_menu"
#define CFG_PLAYER_RIGHT_MENU "player_right_menu"
#define CFG_PLAYERHEADBAR_MENU "player_headbar_menu"
#define CFG_PLAYER_PAINT "player_text"

#define CFG_BOTTOM_MENU_BTN1 "bottom_menu_voice"
#define CFG_BOTTOM_MENU_BTN2 "bottom_menu_button2"
#define CFG_BOTTOM_MUSIC_LEFT "bottom_music_left"
#define CFG_BOTTOM_MUSIC_MIDDLE "bottom_music_middle"
#define CFG_BOTTOM_MUSIC_RIGHT "bottom_music_right"
#define CFG_BOTTOM_NUSIC_SEEKTOBAR "bottom_music_seektobar"

#define CFG_FILE_STB_TFCARD                     "status_bar_tfcard"
#define CFG_BACK_BTN0 "head_back"
#define CFG_SDCARD "head_sdcard"
#define CFG_BATTERY "head_battery"
#define CFG_LIST_FILE_NAME "list_file_name"
#define CFG_HEADBAR_LIST "head_list"
#define CFG_HEADBAR_SETUP "head_setup"

#define CFG_RIGHT_PREBUTTON "right_pre_button"
#define CFG_RIGHT_NEXTBUTTON "right_next_button"

#define CFG_RIGHT_MENU_SETUPATTR "right_setupattr"

#define CFG_BOTTOM_STAT_AREA "bottom_stat_area"
#define CFG_BOTTOM_PAGE_INDEX "bottom_page_index"

#define FGC_WIDGET 			"fgc_widget"
#define BGC_WIDGET 			"bgc_widget"
#define FGC_HEAD_WIDGET 	"head_fgc_widget"
#define BGC_HEAD_WIDGET 	"head_bgc_widget"

#define CVR_KEY_LEFT	128		/*  0x80 */
#define CVR_KEY_RIGHT	129		/*  0x81 */
#define CVR_KEY_MODE	130		/*  0x83 */
#define CVR_KEY_OK		131		/*  0x82 */
#define CVR_KEY_POWER	132		/*  0x84 */

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
	MSG_STATBAR_UPDATAE_TITLE = 0x0801, MSG_HBAR_CLICK,	/* The bottom button is clicked */
	MSG_FUN_BUTTON_CLICK,		/* The main interface button is clicked */
	MSG_TAKE_SETUP_CLOSE,	/* The recording setup parameters interface is closed */
	MSG_TAKE_CAMERA_CLOSE,		/* Recording interface is closed */
	MSG_TAKE_CAMERA_SETUP,		/* Record interface Click the Settings button */
} UserMsgType;

enum FontID {
	ID_FONT_TIMES_18 = 0,
	ID_FONT_TIMES_28,
	ID_FONT_TIMES_15,
	ID_FONT_TIMES_20,
	ID_FONT_TIMES_25,
};

enum ResourceID {
	ID_SCREEN = 0,
	ID_SCREEN_BG,
	ID_BOTTOM_WIN,
	ID_BOTTOM_WIN_BACK,
	ID_BOTTOM_WIN_SETUP,
	ID_BOTTOM_WIN_PHOTO,
	ID_BOTTOM_WIN_AUDIO,
	ID_BOTTOM_WIN_VIDEO,

	ID_FUN_BUTTON,
	ID_FUN_BUTTON_TPHOTO,
	ID_FUN_BUTTON_TAUDIO,
	ID_FUN_BUTTON_TVIDEO,
	ID_FUN_BUTTON_TOW_TVIDEO,
	ID_FUN_BUTTON_PHOTO,
	ID_FUN_BUTTON_AUDIO,
	ID_FUN_BUTTON_VIDEO,

	ID_FRONT_CAMRA_SIZE,
	ID_BACK_CAMRA_SIZE,

	ID_HEAD_BAR_SD,
	ID_HEAD_BAR_UD,

	ID_FAST_WIN,
	ID_PLAYER_SCREEN,
	ID_BOTTOM_MENU,
	ID_RIGHT_MENU,
	ID_HEADBAR,

	ID_BOTTOMMENU_BTN1,
	ID_BOTTOMMENU_BTN2,
	ID_STATUSBAR_ICON_TFCARD,
	ID_PLAYER,
	ID_MUSIC,
	ID_BACK,
	ID_PLAYER_START,
	ID_PLAYER_VOICE,
	ID_SDCARD,
	ID_BATTERY,
	ID_LIST,
	ID_LIST_FILE_NAME,
	ID_SETUP,
	ID_SETUPATTR,
	ID_PLAYER_PAINT,
	ID_BOTTOM_MUSIC_LEFT,
	ID_BOTTOM_MUSIC_MIDDLE,
	ID_BOTTOM_MUSIC_RIGHT,
	ID_SETUPSTATIC0,
	ID_SETUPBTN0,
	ID_SETUPBTN1,
	ID_SETUPBTN2,
	ID_SETUPSTATIC1,
	ID_SETUPBTN3,
	ID_SETUPBTN4,
	ID_SETUPSTATIC2,
	ID_SETUPBTN9,
	ID_SETUPBTN10,
	ID_SETUPBTN11,
	ID_SETUPBTN12,
	ID_OK,
	ID_CANCEL,
	ID_PICTURE,
	ID_BOTTOM_PICTURE_LEFT,
	ID_BOTTOM_PICTURE_MIDDLE,
	ID_BOTTOM_PICTURE_RIGHT,
	ID_SEEKTOBAR,
	ID_NEXTBUTTON,
	ID_PREVBUTTON,

	ID_BM_STAT_WIN_PAGE_INDEX,

	ID_MOUNT_PATH,
};

enum BmpType {
	BMPTYPE_BASE = 0,
	BMPTYPE_UNSELECTED,
	BMPTYPE_SELECTED,
	BMPTYPE_WINDOWPIC_MODE,
	BMPTYPE_WINDOWPIC_WIND,
	BMPTYPE_WINDOWPIC_WIND_DIRECT,
};

enum VIDEO_TOW_BNUTTON_ID {
	ID_VIDEO_TOW_UP = 0,
	ID_VIDEO_TOW_DOWN,
	ID_VIDEO_TOW_LEFT,
	ID_VIDEO_TOW_RIGHT,
	ID_VIDEO_TOW_HOST,
	ID_VIDEO_TOW_PIP,

	ID_VIDEO_TOW_END,
};

enum PHOTO_BNUTTON_ID {
	ID_PHOTO_SIZE_640x480 = 0,
	ID_PHOTO_SIZE_800x600,
	ID_PHOTO_SIZE_1280x960,
	ID_PHOTO_SIZE_1600x1200,
	ID_PHOTO_SIZE_2048x1536,
	ID_PHOTO_SIZE_2560x1920,
	ID_PHOTO_SIZE_3264x2448,

	ID_PHOTO_FORMAT_JPG,

	ID_PHOTO_END,
};

enum AUDIO_BNUTTON_ID {
	ID_AUDIO_OPF_AAC = 0,
	ID_AUDIO_OPF_MP3,
	ID_AUDIO_AEF_PCM,
	ID_AUDIO_AEF_AAC,
	ID_AUDIO_AEF_MP3,
	ID_AUDIO_AEF_LPCM,
	ID_AUDIO_MRTMS_1,
	ID_AUDIO_MRTMS_2,
	ID_AUDIO_MRTMS_3,
	ID_AUDIO_MRTMS_5,
	ID_AUDIO_EBR_1,
	ID_AUDIO_EBR_3,
	ID_AUDIO_EBR_6,
	ID_AUDIO_EBR_8,
	ID_AUDIO_MIPF,
	ID_AUDIO_MSR,
	ID_AUDIO_MC,

	ID_AUDIO_END,
};

enum VIDEO_BNUTTON_ID {
	ID_VIDEO_OPF = 0,
	ID_VIDEO_VEF,
	ID_VIDEO_AEF,
	ID_VIDEO_MRTMS,
	ID_VIDEO_EBR,
	ID_VIDEO_EFR,
	ID_VIDEO_VES,
	ID_VIDEO_CCS,
	ID_VIDEO_MIPF,
	ID_VIDEO_MSR,
	ID_VIDEO_MC,
	ID_VIDEO_AM,
	ID_VIDEO_PROUT,
	ID_VIDEO_PROTATE,
	ID_VIDEO_VESDR,
	ID_VIDEO_CEWM,
	ID_VIDEO_CIPF,

	ID_SINGLE_BOX1,
	ID_SINGLE_BOX2,
	ID_SINGLE_BOX3,
	ID_SINGLE_BOX4,
	ID_SINGLE_BOX5,
	ID_SINGLE_BOX6,
	ID_SINGLE_BOX7,
	ID_SINGLE_BOX8,
	ID_SINGLE_BOX9,
	ID_SINGLE_BOX10,
	ID_SINGLE_BOX11,
	ID_SINGLE_BOX12,
	ID_SINGLE_BOX13,
	ID_SINGLE_BOX14,
	ID_SINGLE_BOX15,
	ID_SINGLE_BOX16,

	ID_BUTTON_OK,
	ID_BNUTTON_END,
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

	smRect backRect;
	currentIcon_t backIcon;

	smRect setupRect;
	currentIcon_t setupIcon;
} FastMenuDataType;

typedef struct {
	smRect winRect;

	smRect rect[7];
	currentIcon_t icon[7];
} FunButonDataType;

typedef struct {
	smRect winRect;

	smRect rect[3];
	currentIcon_t icon[3];
} TakeButonDataType;

typedef struct {
	smRect rect[2];
	currentIcon_t icon[2];
} HeadBarDataType;

typedef struct {
	int ScreenResolution1280x800;
	int ScreenResolution800x480;
	int ScreenResolution1280x480;
	int IsForLoop;
	int IsRotate;
	int PictureScaledown;
} SetupDataType;

typedef struct {
	smRect winRect;

	gal_pixel bgc;
	gal_pixel fgc;
	gal_pixel head_bar_bgc;
	gal_pixel head_bar_fgc;

	smRect btnRect[18];
	smRect btnRect1[2];
	currentIcon_t btnIcon3[3];
	currentIcon_t btnIcon4[17];
} PopWinDataType;

typedef struct {
	s32 videoTotalTime;
	s32 videoCurTime;
} RePlayData;

typedef struct {
	RePlayData rpData;
} RecordWinDataType;

typedef struct {
	HWND mHwnd[4];

	int platform;
	char configFile[32];
	currentIcon_t mountPath;
	GHANDLE mCfgFileHandle;

	smRect rScreenRect;
	smRect rheadbarRect;
	smRect rbottombarRect;
	smRect rrightbarRect;
	smRect rplayerbarRect;
	smRect rFrontCameraSize;
	smRect rBackCameraSize;

	PLOGFONT mFont_Times18;
	PLOGFONT mFont_Times28;

	PLOGFONT mFont_Times15;
	PLOGFONT mFont_Times20;
	PLOGFONT mFont_Times25;
	PLOGFONT mCurLogFont;

	FastMenuDataType rFastMenuData;
	FunButonDataType rFunButonData;
	TakeButonDataType rTakeButonData;
	HeadBarDataType rHeadBarData;
	PopWinDataType rPopWinData;
} ResDataType;

void setHwnd(unsigned int win_id, HWND hwnd);
void setCurrentIconValue(enum ResourceID resID, int cur_val);
gal_pixel getResColor(enum ResourceID resID, enum ColorType type);
int getResRect(enum ResourceID resID, smRect *rect);
PLOGFONT getLogFont(enum FontID id);
int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp);
void unloadBitMap(BITMAP *bmp);
int getPlatform(void);

void ResourceInit(void);
void ResourceUninit(void);
#endif
