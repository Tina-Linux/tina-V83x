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

#define ICON_PATH_SIZE	64
#define ICON_TOTAL_CNT	12

/* Define keywords match to system.cfg */
#define CFG_TIME_TITLE_MAIN_AREA             "time_title_main_area"
#define CFG_TIME_TITLE_WELCOME_AREA          "time_title_welcome_area"
#define CFG_DESK_BUTTON_AREA                 "desk_button_area"
#define CFG_PROGRESS_AREA                    "progress_area"
#define CFG_LIST_AREA                        "list_area"

#define CFG_DESK_BUTTON_ONE_BMP             "desk_button_one"
#define CFG_DESK_BUTTON_TOW_BMP             "desk_button_tow"
#define CFG_DESK_VOLUME_BMP                 "desk_volume"
#define CFG_HEADBAR_BUTTON_BMP_AREA         "headbar_button"
#define CFG_MUSIC_PLAYING_BMP               "music_playing"
#define CFG_CONTROL_BUTTON_NEXT_BMP_AREA    "control_button_next"
#define CFG_CONTROL_BUTTON_VOLUME_BMP_AREA  "control_button_volume"
#define CFG_CONTROL_BUTTON_BLUETOOTH_BMP_AREA    "control_button_bluetooth"
#define CFG_MUSIC_AUX_BMP                   "music_aux"
#define CFG_CONTROL_BUTTONS_BMP_AREA        "control_buttons"
#define CFG_SETTING_BUTTON_BMP              "setting_button"
#define CFG_NORMAL_BUTTON_BMP               "normal_button"
#define CFG_SWITCH_BUTTON_BMP               "switch_button"
#define CFG_SELECT_BUTTON_BMP               "select_button"

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

typedef enum {
    Language_CN = 0, Language_EN,
} LanguageType;

/* define user Message */
typedef enum {
    MSG_LANGUAGE_SWITCH = 0x0801, /* The main interface button is clicked */
    MSG_PLAYBACK_COMPLETE, MSG_MUSIC_PLAY_STOP,
} UserMsgType;

enum TimerID {
    ID_TIMER_UPDATE_TIME = 200,
    ID_TIMER_MUSIC_PLAYING,
    ID_TIMER_PROGRESS_UPDATE,
    ID_TIMER_SWITCH_BUTTON,
    ID_TIMER_LIST_SLIDE,
};

enum FontID {
    ID_FONT_TIMES_14 = 0,
    ID_FONT_TIMES_18,
    ID_FONT_TIMES_68,
    ID_FONT_TIMES_28,
    ID_FONT_TIMES_24,
    ID_FONT_TIMES_22,
    ID_FONT_TIMES_40,
};

enum ResourceID {
    ID_SCREEN = 0,
    ID_TIME_TITLE_MAIN_AREA,
    ID_TIME_TITLE_WELCOME_AREA,
    ID_DESK_BUTTON_AREA,
    ID_PROGRESS_AREA,
    ID_LIST_AREA,

    ID_DESK_BUTTON_ONE_BMP,
    ID_DESK_BUTTON_TOW_BMP,
    ID_DESK_VOLUME_BMP,
    ID_HEADBAR_BUTTON_BMP_AREA,
    ID_MUSIC_PLAYING_BMP,
    ID_CONTROL_BUTTON_NEXT_BMP_AREA,
    ID_CONTROL_BUTTON_VOLUME_BMP_AREA,
    ID_CONTROL_BUTTON_BLUETOOTH_BMP_AREA,
    ID_MUSIC_AUX_BMP,
    ID_CONTROL_BUTTONS_BMP_AREA,
    ID_SETTING_BUTTON_BMP,
    ID_NORMAL_BUTTON_BMP,
    ID_SWITCH_BUTTON_BMP,
    ID_SELECT_BUTTON_BMP,

    ID_BM_STAT_WIN_PAGE_INDEX,
};

enum BmpType {
    BMPTYPE_BASE = 0,
    BMPTYPE_UNSELECTED,
    BMPTYPE_SELECTED,
    BMPTYPE_WINDOWPIC_MODE,
    BMPTYPE_WINDOWPIC_WIND,
    BMPTYPE_WINDOWPIC_WIND_DIRECT,
};

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
    HWND mHwnd[4];

    char configFile[32];
    GHANDLE mCfgFileHandle;

    smRect rScreenRect;
    smRect timeTitleRect[2];
    smRect deskButtonRect;
    smRect headBarRect;
    smRect controlButtonRect[3];
    smRect controlButtonsRect;
    smRect progressRect;
    smRect listRect;

    currentIcon_t deskButtonOneIcon;
    currentIcon_t deskButtonTowIcon;
    currentIcon_t deskVolumeIcon;
    currentIcon_t headbarButtonIcon;
    currentIcon_t musicPlayingIcon;
    currentIcon_t controlButtonIcon[3];
    currentIcon_t musicAuxIcon;
    currentIcon_t controlButtonsIcon;
    currentIcon_t settingButtonIcon;
    currentIcon_t normalButtonIcon;
    currentIcon_t switchButtonIcon;
    currentIcon_t selectButtonIcon;

    PLOGFONT mFont_Times14;
    PLOGFONT mFont_Times18;
    PLOGFONT mFont_Times68;
    PLOGFONT mFont_Times28;
    PLOGFONT mFont_Times24;
    PLOGFONT mFont_Times22;
    PLOGFONT mFont_Times40;
    PLOGFONT mCurLogFont;

} ResDataType;

LanguageType languageType;

void setCurrentIconValue(enum ResourceID resID, int cur_val);
int getResRect(enum ResourceID resID, smRect *rect);
PLOGFONT getLogFont(enum FontID id);
int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp);
void unloadBitMap(BITMAP *bmp);

void ResourceInit(void);
void ResourceUninit(void);
#endif
