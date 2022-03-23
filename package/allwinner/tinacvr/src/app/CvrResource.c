#include "CvrResource.h"

#define ICON_PATH_SIZE	64
#define ICON_TOTAL_CNT	2

#define LANG_FILE_CN	"/usr/res/lang/zh-CN.bin"
#define LANG_FILE_TW	"/usr/res/lang/zh-TW.bin"
#define LANG_FILE_EN	"/usr/res/lang/en.bin"
#define LANG_FILE_JPN	"/usr/res/lang/jpn.bin"
#define LANG_FILE_KR	"/usr/res/lang/korean.bin"
#define LANG_FILE_RS	"/usr/res/lang/russian.bin"

#define CFG_VERIFICATION	"verification"

#define CFG_FILE_ML				"menu_list"
#define CFG_FILE_SUBMENU		"sub_menu"
#define CFG_FILE_ML_MB			"menu_list_message_box"
#define CFG_FILE_STB			"status_bar"
#define CFG_FILE_STB_WNDPIC		"status_bar_window_pic"
#define CFG_FILE_STB_LABEL1		"status_bar_label1"
#define CFG_FILE_STB_RESERVELABLE	"status_bar_reserve_label"
#define CFG_FILE_STB_WIFI			"status_bar_wifi"
#define CFG_FILE_STB_PARK			"status_bar_park"
#define CFG_FILE_STB_UVC			"status_bar_uvc"
#define CFG_FILE_STB_AWMD			"status_bar_awmd"
#define CFG_FILE_STB_LOCK			"status_bar_lock"
#define CFG_FILE_STB_TFCARD			"status_bar_tfcard"
#define CFG_FILE_STB_AUDIO			"status_bar_audio"
#define CFG_FILE_STB_BAT			"status_bar_battery"
#define CFG_FILE_STB_BAT1			"status_bar_battery1"
#define CFG_FILE_STB_BAT2			"status_bar_battery2"
#define CFG_FILE_STB_TIME			"status_bar_time"
#define CFG_FILE_STB_WOND           "status_bar_wond"
#define CFG_FILE_STB_LABEL2        "status_bar_label2"

#define CFG_FILE_ML_WIFI		"menu_list_wifi"
#define CFG_FILE_ML_PARK		"menu_list_park"
#define CFG_FILE_ML_VQ			"menu_list_video_quality"
#define CFG_FILE_ML_PQ			"menu_list_photo_quality"
#define CFG_FILE_ML_VTL			"menu_list_time_length"
#define CFG_FILE_ML_AWMD		"menu_list_awmd"
#define CFG_FILE_ML_WB			"menu_list_white_balance"
#define CFG_FILE_ML_CONTRAST		"menu_list_contrast"
#define CFG_FILE_ML_EXPOSURE		"menu_list_exposure"
#define CFG_FILE_ML_POR			"menu_list_power_on_record"
#define CFG_FILE_ML_SS			"menu_list_screen_switch"
#define CFG_FILE_ML_VOLUME		"menu_list_volume"
#define CFG_FILE_ML_DATE			"menu_list_date"
#define CFG_FILE_ML_LANG			"menu_list_language"
#define CFG_FILE_ML_TWM				"menu_list_time_water_mask"
#define CFG_FILE_ML_LICENSE_PLATE_WM		"menu_list_license_plate_wm"
#define CFG_FILE_ML_FORMAT			"menu_list_format"
#define CFG_FILE_ML_FACTORYRESET	"menu_list_factory_reset"
#define CFG_FILE_ML_FIRMWARE		"menu_list_firmware"
#define CFG_FILE_ML_SMARTALGORITHM		"menu_list_smartalgorithm"
#define CFG_FILE_ML_ALIGNLINE		"menu_list_alignline"
#define CFG_FILE_ML_GSENSOR			"menu_list_gsensor"
#define CFG_FILE_ML_VOICEVOL			"menu_list_voicevol"
#define CFG_FILE_ML_SHUTDOWN			"menu_list_shutdown"
#define CFG_FILE_ML_IMPACTLEVEL			"menu_list_impactlevel"
#define CFG_FILE_ML_FCWSENSITY			"menu_list_fcwsensity"

#define CFG_MENU_SWITCH		"switch"
#define CFG_MENU_WIFI		"wifi"
#define CFG_MENU_PARK		"park"
#define CFG_MENU_LANG		"language"
#define CFG_MENU_VQ			"video_quality"
#define CFG_MENU_PQ			"photo_quality"
#define CFG_MENU_VTL		"video_time_length"
#define CFG_MENU_WB			"white_balance"
#define CFG_MENU_CONTRAST	"contrast"
#define CFG_MENU_EXPOSURE	"exposure"
#define CFG_MENU_POR		"power_on_record"
#define CFG_MENU_SS			"screen_switch"
#define CFG_MENU_VOLUME		"volume"
#define CFG_MENU_TWM		"time_water_mask"
#define CFG_MENU_AWMD		"awmd"
#define CFG_MENU_SMARTALGORITHM		"smartalgorithm"
#define CFG_MENU_ALIGNLINE		"alignline"
#define CFG_MENU_GSENSOR		"gsensor"
#define CFG_MENU_VOICEVOL		"voicevol"
#define CFG_MENU_SHUTDOWN	"shutdown"
#define CFG_MENU_IMPACTLEVEL	"impactlevel"
#define CFG_MENU_FCWSENSITY		"fcwsensity"
#define CFG_VERIFICATION	"verification"

#define FGC_WIDGET				"fgc_widget"
#define BGC_WIDGET				"bgc_widget"
#define SUB_BGC_WIDGET				"sub_bgc_widget"
#define BGC_WIDGET1				"bgc_widget1"
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

#define CFG_FILE_PLBPREVIEW				"playback_preview"
#define CFG_FILE_PLBPREVIEW_IMAGE		"playback_preview_image"
#define CFG_FILE_PLBPREVIEW_ICON		"playback_preview_icon"
#define CFG_FILE_PLBPREVIEW_MENU		"playback_preview_menu"
#define CFG_FILE_PLBPREVIEW_MESSAGEBOX	"playback_preview_messagebox"

#define CFG_FILE_PLB_ICON				"playback_icon"
#define CFG_FILE_PLB_PGB				"playback_progress_bar"
#define CFG_FILE_CONNECT2PC				"connect2PC"
#define CFG_FILE_WARNING_MB				"warning_messagebox"
#define CFG_FILE_OTHER_PIC				"other_picture"
#define CFG_FILE_TIPLABEL				"tip_label"
#define CFG_FILE_SHOW_RECT              "preview_show_rect"
#define CFG_FILE_PIP_RECT               "preview_pip_rect"

typedef struct {
	const char* mainkey;
	void* addr;
}configTable2;

typedef struct {
	const char* mainkey;
	const char* subkey;
	void* addr;
}configTable3;

typedef struct
{
		unsigned int current;
		char *iconName[ICON_TOTAL_CNT];
}currentIcon_t;

typedef struct {
		unsigned int current;
		unsigned int count;

}menuItem_t;

typedef struct
{
		CvrRectType STBRect;
		gal_pixel bgc;
		gal_pixel fgc;

		gal_pixel label1Fgc;
		gal_pixel label2Fgc;
		gal_pixel reserveLabelFgc;

		CvrRectType windowPicRect;

		char recordPreviewPic[ICON_PATH_SIZE];
		char screenShotPic[ICON_PATH_SIZE];
		char playBackPreviewPic[ICON_PATH_SIZE];
		char playBackPic[ICON_PATH_SIZE];
		char menuPic[ICON_PATH_SIZE];

		CvrRectType label1Rect;
        CvrRectType label2Rect;
		CvrRectType reserveLabelRect;

		CvrRectType wifiRect;
		currentIcon_t wifiIcon;

		CvrRectType parkRect;
		currentIcon_t parkIcon;

		CvrRectType uvcRect;
		currentIcon_t uvcIcon;

		CvrRectType awmdRect;
		currentIcon_t awmdIcon;

		CvrRectType lockRect;
		currentIcon_t lockIcon;

		CvrRectType tfCardRect;
		currentIcon_t tfCardIcon;

		CvrRectType audioRect;
		currentIcon_t audioIcon;

		CvrRectType batteryRect;
		currentIcon_t batteryIcon;
		currentIcon_t batteryIcon1;
		currentIcon_t batteryIcon2;

        CvrRectType wondRect;
		currentIcon_t wondIcon;

}StatusBarDataType;

typedef struct
{
		CvrRectType imageRect;
		CvrRectType iconRect;
		currentIcon_t icon;

		CvrRectType popupMenuRect;
		unsigned int popupMenuItemWidth;
		unsigned int popupMenuItemHeight;
		gal_pixel popupMenuBgcWidget;
		gal_pixel popupMenuBgcItemNormal;
		gal_pixel popUpMenuBgcItemFocus;
		gal_pixel popUpMenuMain3dBox;

		CvrRectType messageBoxRect;
		unsigned int messageBoxItemWidth;
		unsigned int messageBoxItemHeight;
		gal_pixel messageBoxBgcWidget;
		gal_pixel messageBoxFgcWidget;
		gal_pixel messageBoxBgcItemNormal;
		gal_pixel messageBoxBgcItemFous;
		gal_pixel messageBoxMain3dBox;
}PlayBackPreviewDataType;

typedef struct
{
		CvrRectType iconRect;
		currentIcon_t icon;

		CvrRectType PGBRect;
		gal_pixel PGBBgcWidget;
		gal_pixel PGBFgcWidget;
}PlayBackDataType;

typedef struct
{
		bool LWMEnable;
		char lwaterMark[30];
}menuDataLWMType;

typedef struct
{
		CvrRectType menuListRect;
		CvrRectType subMenuRect;
		CvrRectType messageBoxRect;
		CvrRectType dateRect;

		unsigned int dateTileRectH;
		unsigned int dateHBorder;
		unsigned int dateYearW;
		unsigned int dateLabelW;
		unsigned int dateNumberW;
		unsigned int dateBoxH;


		char choicePic[ICON_PATH_SIZE];
		char checkedNormalPic[ICON_PATH_SIZE];
		char checkedPressPic[ICON_PATH_SIZE];
		char uncheckedNormalPic[ICON_PATH_SIZE];
		char uncheckedPressPic[ICON_PATH_SIZE];
		char unfoldNormalPic[ICON_PATH_SIZE];
		char unfoldPressPic[ICON_PATH_SIZE];

		gal_pixel bgcWidget;
		gal_pixel subbgcWidget;
		gal_pixel fgcWidget;
		gal_pixel linecItem;
		gal_pixel stringcNormal;
		gal_pixel stringcSelected;
		gal_pixel valuecNormal;
		gal_pixel valuecSelected;
		gal_pixel scrollbarc;

		gal_pixel subMenuBgcWidget;
		gal_pixel subMenuFgcWidget;
		gal_pixel subMenuLinecTitle;

		gal_pixel messageBoxBgcWidget;
		gal_pixel messageBoxBgcWidget1;
		gal_pixel messageBoxFgcWidget;
		gal_pixel messageBoxLinecTitle;
		gal_pixel messageBoxLinecItem;

		gal_pixel dateBgcWidget;
		gal_pixel dateFgc_label;
		gal_pixel dateFgc_number;
		gal_pixel dateLinecTitle;
		gal_pixel dateBordercSelected;
		gal_pixel dateBordercNormal;

		currentIcon_t wifiIcon;
	    currentIcon_t parkIcon;
		currentIcon_t videoQualityIcon;
		currentIcon_t photoQualityIcon;
		currentIcon_t timeLengthIcon;
		currentIcon_t moveDetectIcon;
		currentIcon_t whiteBalanceIcon;
		currentIcon_t contrastIcon;
		currentIcon_t exposureIcon;
		currentIcon_t PORIcon;
		currentIcon_t screenSwitchIcon;
		currentIcon_t volumeIcon;
		currentIcon_t dateIcon;
		currentIcon_t languageIcon;
		currentIcon_t TWMIcon;
		currentIcon_t licensePlateWMIcon;
		currentIcon_t formatIcon;
		currentIcon_t factoryResetIcon;
		currentIcon_t firmwareIcon;
		currentIcon_t smartalgorithm;
		currentIcon_t alignlineIcon;
		currentIcon_t gsensorIcon;
		currentIcon_t voicevolIcon;
		currentIcon_t shutdownIcon;
		currentIcon_t impactlevelIcon;
		currentIcon_t fcwsensityIcon;
}MenuListType;

typedef struct
{
		CvrRectType rect;
		gal_pixel fgcWidget;
		gal_pixel bgcWidget;
		gal_pixel linecTitle;
		gal_pixel linecItem;
}WarningMBType;

typedef struct
{
		CvrRectType rect;
		unsigned int itemWidth;
		unsigned int itemHeight;

		gal_pixel bgcWidget;
		gal_pixel bgcItemFocus;
		gal_pixel bgcItemNormal;
		gal_pixel mainc_3dbox;

		char usbStorageModem[ICON_PATH_SIZE];
		char pcCamModem[ICON_PATH_SIZE];
}ConnectToPCType;

typedef struct
{
		CvrRectType rect;
		unsigned int titleHeight;
		gal_pixel fgcWidget;
		gal_pixel bgcWidget;
		gal_pixel linecTitle;
}TipLabelType;

typedef struct
{
		CvrRectType showRect;
		CvrRectType pipRect;
}PreviewType;


typedef struct
{
	HWND mHwnd[SelfCtrlWinCnt+1];

	char configFile[32];
	GHANDLE mCfgFileHandle;

	char configMenu[32];
	char defaultConfigMenu[32];
	GHANDLE mCfgMenuHandle;

	BITMAP  mNoVideoImage;
	CvrRectType rScreenRect;
	char rPoweroffPic[ICON_PATH_SIZE];

	LANGUAGE lang;
	PLOGFONT mCurLogFont;
	PLOGFONT mLogFont;
	PLOGFONT mLogFont_MSYH;
	PLOGFONT mLogFont_Malgun_Gothic;	//korean font
	PLOGFONT mLogFont_Times;
	char *label[LANG_LABEL_MAX];

	StatusBarDataType rStatusBarData;
	PlayBackPreviewDataType rPlayBackPreviewData;
	PlayBackDataType rPlayBackData;
	MenuListType rMenuList;
	WarningMBType rWarningMB;
	ConnectToPCType rConnectToPC;
	TipLabelType rTipLabel;
	menuDataLWMType menuDataLWM;
	PreviewType  rPreview;

	menuItem_t menuDataLang;
	menuItem_t menuDataVQ;
	menuItem_t menuDataPQ;
	menuItem_t menuDataVTL;
	menuItem_t menuDataWB;
	menuItem_t menuDataContrast;
	menuItem_t menuDataExposure;
	menuItem_t menuDataSS;
	menuItem_t menuDataGsensor;
	menuItem_t menuDataVoiceVol;
	menuItem_t menuDataShutDown;
	menuItem_t menuDataImpactLevel;
	menuItem_t menuDatafcwsensity;
	menuItem_t menuDataSmartAlgorithmEnable;
	bool menuDataWIFIEnable;
	bool menuDataPARKEnable;
	bool menuDataPOREnable;
	bool menuDataVolumeEnable;
	bool menuDataTWMEnable;
	bool menuDataAWMDEnable;
	bool menuDataAlignlineEnable;

	bool mRecording;
}ResSelfDataType;
ResSelfDataType *resSelfData = NULL;

enum ResourceID GetResourceID(u32 nIndex)
{
	enum ResourceID nData = 0;
	switch(nIndex)
	{
		case 0:
			nData = ID_MENU_LIST_PARK;
			break;
		case 1:
			nData = ID_MENU_LIST_AWMD;
			break;
        case 2:
			nData = ID_MENU_LIST_POR;
			break;
        case 3:
			nData = ID_MENU_LIST_SILENTMODE;
			break;
        case 4:
			nData = ID_MENU_LIST_TWM;
			break;
        case 5:
			nData = ID_MENU_LIST_ALIGNLINE;
			break;
        case 6:
            nData = ID_MENU_LIST_VQ;
            break;
        case 7:
           nData = ID_MENU_LIST_PQ;
           break;
        case 8:
           nData = ID_MENU_LIST_VTL;
           break;
        case 9:
          nData = ID_MENU_LIST_GSENSOR;
          break;
        case 10:
          nData = ID_MENU_LIST_IMPACTLEVEL;
          break;
        case 11:
          nData = ID_MENU_LIST_FCWSENSITY;
          break;
        case 12:
           nData = ID_MENU_LIST_VOICEVOL;
           break;
        case 13:
          nData = ID_MENU_LIST_WB;
          break;
        case 14:
          nData = ID_MENU_LIST_CONTRAST;
          break;
        case 15:
          nData = ID_MENU_LIST_EXPOSURE;
          break;
        case 16:
          nData = ID_MENU_LIST_SS;
          break;
        case 17:
          nData = ID_MENU_LIST_LANG;
          break;
        case 18:
          nData = ID_MENU_LIST_SHUTDOWN;
          break;
        case 19:
          nData = ID_MENU_LIST_FIRMWARE;
          break;
        case 20:
          nData = ID_MENU_LIST_FORMAT;
          break;
        case 21:
          nData = ID_MENU_LIST_FRESET;
          break;
        case 22:
          nData = ID_MENU_LIST_DATE;
          break;
		default:
			break;
	}
	return nData;
}

unsigned int haveValueString(u32 nIndex)
{
    unsigned int nData;
    switch(nIndex)
    {
        case 0:
            nData = 0;	/*PARK		 not have value string */
            break;
	case 1:
            nData = 0;  /*AWMD         have not value string */
            break;
	case 2:
            nData = 0;  /*POWER ON VIDEO         have not value string */
            break;
	case 3:
            nData = 0;  /*SILENTMODE        have not value string */
            break;
	case 4:
            nData = 0;  /*WATERMARK           have not value string */
            break;
	case 5:
            nData = 0;  /*ALIGNLINE        have not value string */
            break;
        case 6:
            nData = 1;  /*VQ        have value string */
            break;
        case 7:
            nData = 1;  /*PQ        have value string */
            break;
        case 8:
            nData = 1;  /*VTL        have value string */
            break;
        case 9:
            nData = 1;  /*GSENSOR        have value string */
            break;
        case 10:
            nData = 1;  /*IMPACTLEVEL        have value string */
            break;
        case 11:
            nData = 1;  /*FCWSENSITY        have value string */
            break;
        case 12:
            nData = 1;  /*VOICEVOL        have value string */
            break;
        case 13:
            nData = 1;  /*WB        have value string */
            break;
        case 14:
            nData = 1;  /*CONTRAST        have value string */
            break;
        case 15:
            nData = 1;  /*EXPOSURE        have value string */
            break;
        case 16:
            nData = 1;  /*SS        have value string */
            break;
        case 17:
            nData = 1;  /*LANG        have value string */
            break;
        case 18:
            nData = 1;  /*SHOTDOWN        have value string */
            break;
        case 19:
           nData = 1;  /*FIRMWARE        have value string */
           break;
        case 20:
            nData = 0;  /*FORMAT        have value string */
            break;
        case 21:
           nData = 0;  /*FRESET        have value string */
           break;
        case 22:
           nData = 0;  /*DATA        have value string */
           break;
        default:
			break;
    }
    return nData;
}

unsigned int haveSubMenu(u32 nIndex)
{
    unsigned int ret;
    switch(nIndex)
    {
	case 0:
            ret = 0;  /*PARK         not have sub menu */
            break;
	case 1:
            ret = 0;  /*AWMD         have sub menu */
            break;
	case 2:
            ret = 0;  /*POR         have sub menu */
            break;
	case 3:
            ret = 0;  /*SILENTMODE        have sub menu */
            break;
	case 4:
            ret = 0;  /*WTM          not have sub menu */
            break;
	case 5:
            ret = 0;  /*ALIGNLINE       not have sub menu */
            break;
	case 6:
            ret = 1;  /*VQ       not have sub menu */
            break;
	case 7:
            ret = 1;  /*PQ        have sub menu */
            break;
	case 8:
            ret = 1;  /*VTL        have sub menu */
            break;
	case 9:
            ret = 1;  /*GSENSOR         have sub menu */
            break;
	case 10:
            ret = 1;  /*IMPACTLEVEL       not have sub menu */
            break;
	case 11:
            ret = 1;  /*FCWSENSITY       have sub menu */
            break;
	case 12:
            ret = 1; /*VOICEVOL         have sub menu */
            break;
	case 13:
            ret = 1;  /*WB       have sub menu */
            break;
	case 14:
            ret = 1;  /*CONTRAST       have sub menu */
            break;
	case 15:
            ret = 1;  /*EXPOSURE        not have sub menu */
            break;
	case 16:
            ret = 1;  /*SS         have sub menu */
            break;
	case 17:
            ret = 1;  /*LANG         not have sub menu */
            break;
	case 18:
            ret = 1;  /*LANG       not have sub menu */
            break;
	case 19:
            ret = 0;  /*SHUTDOWN       have sub menu */
            break;
	case 20:
            ret = 0;  /*FIRMWARE        not have sub menu */
            break;
	case 21:
            ret = 0;  /*FORMAT         not have sub menu */
            break;
	case 22:
            ret = 0;  /*FORMAT         not have sub menu */
            break;
        default:
            break;
    }
    return ret;
}

unsigned int haveCheckBox(u32 nIndex)
{
    unsigned int ret;
    switch(nIndex)
    {
	case 0:
            ret = 1;  /*SMARTALGORITHM         have check box */
            break;
	case 1:
            ret = 1;  /*VQ          have check box */
            break;
	case 2:
            ret = 1;  /*PQ          have check box */
            break;
	case 3:
            ret = 1;  /*VTL         have check box */
            break;
	case 4:
            ret = 1;  /*ALIGNLINE          have check box */
            break;
        case 5:
            ret = 1;  /*ALIGNLINE          have check box */
            break;
        case 6:
            ret = 0;  /*ALIGNLINE          have check box */
            break;
        case 7:
            ret = 0;  /*ALIGNLINE          have check box */
            break;
        case 8:
            ret = 0;  /*ALIGNLINE          have check box */
            break;
        case 9:
            ret = 0;  /*GSENSOR          have check box */
            break;
        case 10:
            ret = 0;  /*IMPACTLEVEL          have check box */
            break;
        case 11:
            ret = 0;  /*FCWSENSITY          have check box */
            break;
        case 12:
            ret = 0;  /*VOICEVOL          have check box */
            break;
        case 13:
            ret = 0;  /*WB          have check box */
            break;
        case 14:
            ret = 0;  /*CONTRAST          have check box */
            break;
        case 15:
            ret = 0;  /*EXPOSURE          have check box */
            break;
        case 16:
            ret = 0;  /*SS          have check box */
            break;
        case 17:
            ret = 0;  /*LANG          have check box */
            break;
        case 18:
            ret = 0;  /*SHOTDOWN          have check box */
            break;
        case 19:
           ret = 0;  /*FIRMWARE          have check box */
           break;
        case 20:
            ret = 0;  /*IMGFLAG_IMAGE          have check box */
            break;
        case 21:
           ret = 0;  /*FRESET          have check box */
           break;
        case 22:
           ret = 0;  /*DATA             have check box */
           break;
            default:
	        break;
    }
    return ret;
};

gal_pixel getStatusBarColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_STATUSBAR) {
		switch(type) {
		case COLOR_FGC:
			color = resSelfData->rStatusBarData.fgc;
			break;
		case COLOR_BGC:
			color = resSelfData->rStatusBarData.bgc;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getStatusBarLabelColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_STATUSBAR_LABEL1) {
		switch(type) {
		case COLOR_FGC:
			color = resSelfData->rStatusBarData.label1Fgc;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	}else if(resID == ID_STATUSBAR_LABEL2) {
		switch(type) {
		case COLOR_FGC:
			color = resSelfData->rStatusBarData.label2Fgc;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_STATUSBAR_LABEL_RESERVE) {
		switch(type) {
		case COLOR_FGC:
			color = resSelfData->rStatusBarData.reserveLabelFgc;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}


gal_pixel getPlaybackPreviewColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_PLAYBACKPREVIEW_MENU) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rPlayBackPreviewData.popupMenuBgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = resSelfData->rPlayBackPreviewData.popUpMenuBgcItemFocus;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = resSelfData->rPlayBackPreviewData.popupMenuBgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = resSelfData->rPlayBackPreviewData.popUpMenuMain3dBox;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_PLAYBACKPREVIEW_MB) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rPlayBackPreviewData.messageBoxBgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rPlayBackPreviewData.messageBoxFgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = resSelfData->rPlayBackPreviewData.messageBoxBgcItemFous;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = resSelfData->rPlayBackPreviewData.messageBoxBgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = resSelfData->rPlayBackPreviewData.messageBoxMain3dBox;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getPlaybackColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_PLAYBACK_PGB) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rPlayBackData.PGBBgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rPlayBackData.PGBFgcWidget;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getMenuListColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_MENU_LIST) {
		switch(type) {
		case COLOR_SUB_BGC:
			color = resSelfData->rMenuList.subbgcWidget;
			break;
		case COLOR_BGC:
			color = resSelfData->rMenuList.bgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rMenuList.fgcWidget;
			break;
		case COLOR_LINEC_ITEM:
			color = resSelfData->rMenuList.linecItem;
			break;
		case COLOR_STRINGC_NORMAL:
			color = resSelfData->rMenuList.stringcNormal;
			break;
		case COLOR_STRINGC_SELECTED:
			color = resSelfData->rMenuList.stringcSelected;
			break;
		case COLOR_VALUEC_NORMAL:
			color = resSelfData->rMenuList.valuecNormal;
			break;
		case COLOR_VALUEC_SELECTED:
			color = resSelfData->rMenuList.valuecSelected;
			break;
		case COLOR_SCROLLBARC:
			color = resSelfData->rMenuList.scrollbarc;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_MENU_LIST_MB) {
		switch(type) {
		case COLOR_BGC1:
			color = resSelfData->rMenuList.messageBoxBgcWidget1;
			break;
		case COLOR_BGC:
			color = resSelfData->rMenuList.messageBoxBgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rMenuList.messageBoxFgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = resSelfData->rMenuList.messageBoxLinecTitle;
			break;
		case COLOR_LINEC_ITEM:
			color = resSelfData->rMenuList.messageBoxLinecItem;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_MENU_LIST_DATE) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rMenuList.dateBgcWidget;
			break;
		case COLOR_FGC_LABEL:
			color = resSelfData->rMenuList.dateFgc_label;
			break;
		case COLOR_FGC_NUMBER:
			color = resSelfData->rMenuList.dateFgc_number;
			break;
		case COLOR_LINEC_TITLE:
			color = resSelfData->rMenuList.dateLinecTitle;
			break;
		case COLOR_BORDERC_NORMAL:
			color = resSelfData->rMenuList.dateBordercNormal;
			break;
		case COLOR_BORDERC_SELECTED:
			color = resSelfData->rMenuList.dateBordercSelected;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}

	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getSubMenuColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_SUBMENU) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rMenuList.subMenuBgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rMenuList.subMenuFgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = resSelfData->rMenuList.subMenuLinecTitle;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getWarningMBColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_WARNNING_MB) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rWarningMB.bgcWidget;
			break;
		case COLOR_FGC:
			color = resSelfData->rWarningMB.fgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = resSelfData->rWarningMB.linecTitle;
			break;
		case COLOR_LINEC_ITEM:
			color = resSelfData->rWarningMB.linecItem;
			break;
		default:
			cvr_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getConnect2PCColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_CONNECT2PC) {
		switch(type) {
		case COLOR_BGC:
			color = resSelfData->rConnectToPC.bgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = resSelfData->rConnectToPC.bgcItemFocus;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = resSelfData->rConnectToPC.bgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = resSelfData->rConnectToPC.mainc_3dbox;
			break;
		default:
			cvr_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		cvr_error("invalid resID: %d\n", resID);
	}

	return color;
}

void setCurrentIconValue(enum ResourceID resID, int cur_val)
{
	switch(resID) {
	case ID_STATUSBAR_ICON_BAT:
		resSelfData->rStatusBarData.batteryIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_BAT1:
			resSelfData->rStatusBarData.batteryIcon1.current = cur_val;
			break;
	case ID_STATUSBAR_ICON_BAT2:
			resSelfData->rStatusBarData.batteryIcon2.current = cur_val;
			break;
	case ID_STATUSBAR_ICON_TFCARD:
		resSelfData->rStatusBarData.tfCardIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_AUDIO:
		resSelfData->rStatusBarData.audioIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_LOCK:
		resSelfData->rStatusBarData.lockIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_AWMD:
		resSelfData->rStatusBarData.awmdIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_UVC:
		resSelfData->rStatusBarData.uvcIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_WIFI:
		resSelfData->rStatusBarData.wifiIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_PARK:
		resSelfData->rStatusBarData.parkIcon.current= cur_val;
		break;
    case ID_STATUSBAR_ICON_WOND:
        resSelfData->rStatusBarData.wondIcon.current= cur_val;
        break;
	default:
		cvr_error("invalide resID: %d\n", resID);
		break;
	}
}

#define CHECK_CURRENT_VALID(menuItem) do{ \
	if(menuItem.current >= menuItem.count) { \
		cvr_error("invalid current value\n"); break; } \
}while(0)
const char* getResSubMenuCurString(enum ResourceID resID)
{
	const char* ptr = NULL;
	unsigned int current;
	switch(resID) {
	case ID_MENU_LIST_LANG:
		CHECK_CURRENT_VALID(resSelfData->menuDataLang);
		current = resSelfData->menuDataLang.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_LANG_CONTENT0 + current);
		break;
	case ID_MENU_LIST_VQ:
		CHECK_CURRENT_VALID(resSelfData->menuDataVQ);
		current = resSelfData->menuDataVQ.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_VQ_CONTENT0 + current);
		break;
	case ID_MENU_LIST_PQ:
		CHECK_CURRENT_VALID(resSelfData->menuDataPQ);
		current = resSelfData->menuDataPQ.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_PQ_CONTENT0 + current);
		break;
	case ID_MENU_LIST_VTL:
		CHECK_CURRENT_VALID(resSelfData->menuDataVTL);
		current = resSelfData->menuDataVTL.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_VTL_CONTENT0 + current);
		break;
	case ID_MENU_LIST_WB:
		CHECK_CURRENT_VALID(resSelfData->menuDataWB);
		current = resSelfData->menuDataWB.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_WB_CONTENT0 + current);
		break;
	case ID_MENU_LIST_CONTRAST:
		CHECK_CURRENT_VALID(resSelfData->menuDataContrast);
		current = resSelfData->menuDataContrast.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_CONTRAST_CONTENT0 + current);
		break;
	case ID_MENU_LIST_EXPOSURE:
		CHECK_CURRENT_VALID(resSelfData->menuDataExposure);
		current = resSelfData->menuDataExposure.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_EXPOSURE_CONTENT0 + current);
		break;
	case ID_MENU_LIST_SS:
		CHECK_CURRENT_VALID(resSelfData->menuDataSS);
		current = resSelfData->menuDataSS.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_SS_CONTENT0 + current);
		break;
	case ID_MENU_LIST_FIRMWARE:
		ptr = FIRMWARE_VERSION;
		break;
	case ID_MENU_LIST_GSENSOR:
		CHECK_CURRENT_VALID(resSelfData->menuDataGsensor);
		current = resSelfData->menuDataGsensor.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_GSENSOR_CONTENT0 + current);
		break;
	case ID_MENU_LIST_VOICEVOL:
		CHECK_CURRENT_VALID(resSelfData->menuDataVoiceVol);
		current = resSelfData->menuDataVoiceVol.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_VOICEVOL_CONTENT0 + current);
		break;
	case ID_MENU_LIST_SHUTDOWN:
		CHECK_CURRENT_VALID(resSelfData->menuDataShutDown);
		current = resSelfData->menuDataShutDown.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT0 + current);
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		CHECK_CURRENT_VALID(resSelfData->menuDataImpactLevel);
		current = resSelfData->menuDataImpactLevel.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_IMPACTLEVEL_CONTENT0 + current);
		break;
	case ID_MENU_LIST_FCWSENSITY:
		CHECK_CURRENT_VALID(resSelfData->menuDatafcwsensity);
		current = resSelfData->menuDatafcwsensity.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_FCWSENSITY_CONTENT0 + current);
		break;
		/*
	case ID_MENU_LIST_SMARTALGORITHM:
		CHECK_CURRENT_VALID(menuDataSmartAlgorithmEnable);
		current = menuDataSmartAlgorithmEnable.current;
		ptr = getLabel(LANG_LABEL_SUBMENU_SMARTALGORITHM_CONTENT0 + current);
		break;
		*/
	default:
		cvr_error("invalid resIndex %d\n", resID);
		return NULL;
	}

	return ptr;
}

const char* getResSubMenuTitle(enum ResourceID resID)
{
	const char* ptr = NULL;
	switch(resID) {
	case ID_MENU_LIST_LANG:
		ptr = getLabel(LANG_LABEL_SUBMENU_LANG_TITLE);
		break;
	case ID_MENU_LIST_VQ:
		ptr = getLabel(LANG_LABEL_SUBMENU_VQ_TITLE);
		break;
	case ID_MENU_LIST_PQ:
		ptr = getLabel(LANG_LABEL_SUBMENU_PQ_TITLE);
		break;
	case ID_MENU_LIST_VTL:
		ptr = getLabel(LANG_LABEL_SUBMENU_VTL_TITLE);
		break;
	case ID_MENU_LIST_WB:
		ptr = getLabel(LANG_LABEL_SUBMENU_WB_TITLE);
		break;
	case ID_MENU_LIST_CONTRAST:
		ptr = getLabel(LANG_LABEL_SUBMENU_CONTRAST_TITLE);
		break;
	case ID_MENU_LIST_EXPOSURE:
		ptr = getLabel(LANG_LABEL_SUBMENU_EXPOSURE_TITLE);
		break;
	case ID_MENU_LIST_SS:
		ptr = getLabel(LANG_LABEL_SUBMENU_SS_TITLE);
		break;
	case ID_MENU_LIST_GSENSOR:
		ptr = getLabel(LANG_LABEL_SUBMENU_GSENSOR_TITLE);
		break;
	case ID_MENU_LIST_VOICEVOL:
		ptr = getLabel(LANG_LABEL_SUBMENU_VOICEVOL_TITLE);
		break;
	case ID_MENU_LIST_SHUTDOWN:
		ptr = getLabel(LANG_LABEL_SUBMENU_SHUTDOWN_TITLE);
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		ptr = getLabel(LANG_LABEL_SUBMENU_IMPACTLEVEL_TITLE);
		break;
	case ID_MENU_LIST_FCWSENSITY:
		ptr = getLabel(LANG_LABEL_SUBMENU_FCWSENSITY_TITLE);
		break;
		/*
	case ID_MENU_LIST_SMARTALGORITHM:
		ptr = getLabel(LANG_LABEL_SUBMENU_SMARTALGORITHM_TITLE);
		break;
		*/
	default:
		cvr_error("invalid resIndex %d\n", resID);
		return NULL;
	}

	return ptr;
}

LANGUAGE getLanguage(void)
{
	return resSelfData->lang;
}

PLOGFONT getLogFont(void)
{
	/*
	PLOGFONT font;
	font = GetSystemFont (SYSLOGFONT_WCHAR_DEF);
	return font;
	*/

	return resSelfData->mCurLogFont;
}

int getResVTLms(void)
{
	int value;
	value = resSelfData->menuDataVTL.current;
	if(value == 0)
		return 60 * 1000;
	else if(value == 1)
		return 120 * 1000;
	else
		return -1;
}

const char* getResMenuItemString(enum ResourceID resID)
{
	const char* ptr = NULL;
	switch(resID) {
	case ID_MENU_LIST_WIFI:
		/*
		{
			static char  wifiString[64]={0};
			char ssid[32]={0};
			cfgDataGetString("persist.sys.wifi.ssid", ssid, " ");
			if (strcmp(ssid, " ") == 0) {
				StorageManager *sm = StorageManager::getInstance();
				sm->generateSsid(ssid);
				cfgDataSetString("persist.sys.wifi.ssid", ssid);
			}
			sprintf(wifiString, "%s        ssid: %s", getLabel(LANG_LABEL_MENU_WIFI), ssid);
			db_msg("  wifiString:%s ", wifiString);
			ptr = wifiString;
		}
		*/
		break;
	case ID_MENU_LIST_PARK:
		ptr = getLabel(LANG_LABEL_MENU_PARK);
		break;
#ifdef APP_ADAS
/*
	case ID_MENU_LIST_SMARTALGORITHM:
		ptr = getLabel(LANG_LABEL_MENU_SMARTALGORITHM);
		break;
*/
#endif
	case ID_MENU_LIST_ALIGNLINE:
		ptr = getLabel(LANG_LABEL_MENU_ALIGNLINE);
		break;
	case ID_MENU_LIST_VQ:
		ptr = getLabel(LANG_LABEL_MENU_VQ);
		break;
	case ID_MENU_LIST_PQ:
		ptr = getLabel(LANG_LABEL_MENU_PQ);
		break;
	case ID_MENU_LIST_VTL:
		ptr = getLabel(LANG_LABEL_MENU_VTL);
		break;
	case ID_MENU_LIST_AWMD:
		ptr = getLabel(LANG_LABEL_MENU_AWMD);
		break;
	case ID_MENU_LIST_WB:
		ptr = getLabel(LANG_LABEL_MENU_WB);
		break;
	case ID_MENU_LIST_CONTRAST:
		ptr = getLabel(LANG_LABEL_MENU_CONTRAST);
		break;
	case ID_MENU_LIST_EXPOSURE:
		ptr = getLabel(LANG_LABEL_MENU_EXPOSURE);
		break;
	case ID_MENU_LIST_POR:
		ptr = getLabel(LANG_LABEL_MENU_POR);
		break;
	case ID_MENU_LIST_SS:
		ptr = getLabel(LANG_LABEL_MENU_SS);
		break;
	case ID_MENU_LIST_SILENTMODE:
		ptr = getLabel(LANG_LABEL_MENU_SILENTMODE);
		break;
	case ID_MENU_LIST_DATE:
		ptr = getLabel(LANG_LABEL_MENU_DATE);
		break;
	case ID_MENU_LIST_LANG:
		ptr = getLabel(LANG_LABEL_MENU_LANG);
		break;
	case ID_MENU_LIST_TWM:
		ptr = getLabel(LANG_LABEL_MENU_TWM);
		break;
	case ID_MENU_LIST_FORMAT:
		ptr = getLabel(LANG_LABEL_MENU_FORMAT);
		break;
	case ID_MENU_LIST_FRESET:
		ptr = getLabel(LANG_LABEL_MENU_FRESET);
		break;
	case ID_MENU_LIST_FIRMWARE:
		ptr = getLabel(LANG_LABEL_MENU_FIRMWARE);
		break;
	case ID_MENU_LIST_LICENSE_PLATE_WM:
		ptr = getLabel(LANG_LABEL_MENU_LICENSE_PLATE_WM);
		break;
	case ID_MENU_LIST_GSENSOR:
		ptr = getLabel(LANG_LABEL_MENU_GSENSOR);
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		ptr = getLabel(LANG_LABEL_MENU_IMPACTLEVEL);
		break;
	case ID_MENU_LIST_VOICEVOL:
		ptr = getLabel(LANG_LABEL_MENU_VOICEVOL);
		break;
	case ID_MENU_LIST_SHUTDOWN:
		ptr = getLabel(LANG_LABEL_MENU_SHUTDOWN);
		break;
	case ID_MENU_LIST_FCWSENSITY:
		ptr = getLabel(LANG_LABEL_MENU_FCWSENSITY);
		break;
	default:
		cvr_error("invalid resIndex %d\n", resID);
		return NULL;
	}

	return ptr;
}

BITMAP *getNoVideoImage()
{
	if(resSelfData->mNoVideoImage.bmWidth == -1){
		return NULL;
	}
	return &resSelfData->mNoVideoImage;
}

int notifyWaterMark()
{
	/*
	db_msg(" ");
	int wmFlag = 0;
	if (menuDataTWMEnable) {
		wmFlag |= WATERMARK_TWM;
	}
	if (menuDataLWM.LWMEnable) {
		wmFlag |= WATERMARK_LWM;
	}
	SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_WATER_MARK, (WPARAM)wmFlag, 0);
	*/
	return 0;
}

void getFirmwareInfo(FirmwareInfo *fwInfo)
{
	/*
	struct tm *tm;
	char updated[10];
	fwInfo->product_type = "X1";
	fwInfo->software_version = FIRMWARE_VERSION;
	getDateTime(&tm);
	sprintf(updated, "%d%02d%02d", tm->tm_year, tm->tm_hour, tm->tm_mday);
	fwInfo->updated = updated;
	*/
}

#define CHECK_CURRENT_ICON_VALID(curIcon) do{ \
	if(curIcon.current >= ICON_TOTAL_CNT) { \
		cvr_error("invalid current value\n"); break; } \
}while(0)

int getCurrentIconFileName(enum ResourceID resID, char *file)
{
	int current;

	switch(resID) {
	case ID_STATUSBAR_ICON_WIFI:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.wifiIcon);
		current = resSelfData->rStatusBarData.wifiIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.wifiIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_PARK:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.parkIcon);
		current = resSelfData->rStatusBarData.parkIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.parkIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_AWMD:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.awmdIcon);
		current = resSelfData->rStatusBarData.awmdIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.awmdIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_LOCK:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.lockIcon);
		current = resSelfData->rStatusBarData.lockIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.lockIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_UVC:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.uvcIcon);
		current = resSelfData->rStatusBarData.uvcIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.uvcIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_TFCARD:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.tfCardIcon);
		current = resSelfData->rStatusBarData.tfCardIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.tfCardIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_AUDIO:
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.audioIcon);
		current = resSelfData->rStatusBarData.audioIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.audioIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_BAT :
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.batteryIcon);
		current = resSelfData->rStatusBarData.batteryIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.batteryIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_BAT1 :
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.batteryIcon1);
		current = resSelfData->rStatusBarData.batteryIcon1.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.batteryIcon1.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_BAT2 :
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.batteryIcon2);
		current = resSelfData->rStatusBarData.batteryIcon2.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.batteryIcon2.iconName[current], ICON_PATH_SIZE);
		break;
    case ID_STATUSBAR_ICON_WOND :
		CHECK_CURRENT_ICON_VALID(resSelfData->rStatusBarData.wondIcon);
		current = resSelfData->rStatusBarData.wondIcon.current;
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.wondIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_PLAYBACKPREVIEW_ICON:
		CHECK_CURRENT_ICON_VALID(resSelfData->rPlayBackPreviewData.icon);
		current = resSelfData->rPlayBackPreviewData.icon.current;
		memcpy((void *)file, (void *)resSelfData->rPlayBackPreviewData.icon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_PLAYBACK_ICON:
		CHECK_CURRENT_ICON_VALID(resSelfData->rPlayBackData.icon);
		current = resSelfData->rPlayBackData.icon.current;
		memcpy((void *)file, (void *)resSelfData->rPlayBackData.icon.iconName[current], ICON_PATH_SIZE);
		break;
	default:
		cvr_debug("invalid resID %d\n", resID);
		return -1;
	}

	return 0;
}

int getWindPicFileName(enum BmpType type, char *file)
{

	switch(type) {
	case BMPTYPE_WINDOWPIC_RECORDPREVIEW:
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.recordPreviewPic, ICON_PATH_SIZE);
		break;
	case BMPTYPE_WINDOWPIC_PHOTOGRAPH:
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.screenShotPic, ICON_PATH_SIZE);
		break;
	case BMPTYPE_WINDOWPIC_PLAYBACKPREVIEW:
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.playBackPreviewPic, ICON_PATH_SIZE);
		break;
	case BMPTYPE_WINDOWPIC_PLAYBACK:
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.playBackPic, ICON_PATH_SIZE);
		break;
	case BMPTYPE_WINDOWPIC_MENU:
		memcpy((void *)file, (void *)resSelfData->rStatusBarData.menuPic, ICON_PATH_SIZE);
		break;
	default:
		cvr_error("invalid BmpType %d\n", type);
		return -1;
	}

	return 0;
}

int getMLPICFileName(enum ResourceID resID, enum BmpType type, char *file)
{
	if(resID >= ID_MENU_LIST_VQ && resID <= ID_MENU_LIST_UNCHECKBOX_PIC) {
		switch(resID) {
		case ID_MENU_LIST_WIFI:
			if (type == BMPTYPE_SELECTED)
				memcpy((void *)file, (void *)resSelfData->rMenuList.wifiIcon.iconName[1], ICON_PATH_SIZE);
			else
				memcpy((void *)file, (void *)resSelfData->rMenuList.wifiIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_PARK:
				if (type == BMPTYPE_SELECTED)
				memcpy((void *)file, (void *)resSelfData->rMenuList.parkIcon.iconName[1], ICON_PATH_SIZE);
				else
				memcpy((void *)file, (void *)resSelfData->rMenuList.parkIcon.iconName[0], ICON_PATH_SIZE);
				break;
		case ID_MENU_LIST_ALIGNLINE:
				if (type == BMPTYPE_SELECTED)
				memcpy((void *)file, (void *)resSelfData->rMenuList.alignlineIcon.iconName[1], ICON_PATH_SIZE);
				else
				memcpy((void *)file, (void *)resSelfData->rMenuList.alignlineIcon.iconName[0], ICON_PATH_SIZE);
				break;
		case ID_MENU_LIST_VQ:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.videoQualityIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.videoQualityIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_PQ:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.photoQualityIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.photoQualityIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_VTL:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.timeLengthIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.timeLengthIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_AWMD:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.moveDetectIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.moveDetectIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_WB:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.whiteBalanceIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.whiteBalanceIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_CONTRAST:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.contrastIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.contrastIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_EXPOSURE:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.exposureIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.exposureIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_POR:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.PORIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.PORIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_SS:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.screenSwitchIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.screenSwitchIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_SILENTMODE:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.volumeIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.volumeIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_DATE:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.dateIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.dateIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_LANG:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.languageIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.languageIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_TWM:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.TWMIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.TWMIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_FORMAT:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.formatIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.formatIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_FRESET:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.factoryResetIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.factoryResetIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_FIRMWARE:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.firmwareIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.firmwareIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_UNFOLD_PIC:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.unfoldPressPic, ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.unfoldNormalPic, ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_CHECKBOX_PIC:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.checkedPressPic, ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.checkedNormalPic, ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_UNCHECKBOX_PIC:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.uncheckedPressPic, ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.uncheckedNormalPic, ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_GSENSOR:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.gsensorIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.gsensorIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_VOICEVOL:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.voicevolIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.voicevolIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_SHUTDOWN:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.shutdownIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.shutdownIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_IMPACTLEVEL:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.impactlevelIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.impactlevelIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_FCWSENSITY:
			if(type == BMPTYPE_SELECTED)
			memcpy((void *)file, (void *)resSelfData->rMenuList.fcwsensityIcon.iconName[1], ICON_PATH_SIZE);
			else
			memcpy((void *)file, (void *)resSelfData->rMenuList.fcwsensityIcon.iconName[0], ICON_PATH_SIZE);
			break;
		case ID_MENU_LIST_LICENSE_PLATE_WM:
			memcpy((void *)file, (void *)resSelfData->rMenuList.licensePlateWMIcon.iconName[0], ICON_PATH_SIZE);
			break;
		default:
			break;
		}
	} else {
		cvr_debug("invalid resID %d\n", resID);
		return -1;
	}

	return 0;
}

int getConnect2PcFileName(enum BmpType type, char *file)
{

	switch(type) {
	case BMPTYPE_USB_STORAGE_MODEM:
		memcpy((void *)file, (void *)resSelfData->rConnectToPC.usbStorageModem, ICON_PATH_SIZE);
		break;
	case BMPTYPE_PCCAM_MODEM:
		memcpy((void *)file, (void *)resSelfData->rConnectToPC.pcCamModem, ICON_PATH_SIZE);
		break;
	default:
		cvr_error("invalid BmpType %d\n", type);
		return -1;
	}

	return 0;
}


int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp)
{
	int err_code;
	char file[ICON_PATH_SIZE];

	if(resID == ID_STATUSBAR_ICON_WINDOWPIC) {
		if(getWindPicFileName(type, file) < 0 ) {
			cvr_error("get window pic bmp failed\n");
			return -1;
		}
	} else if(resID >= ID_STATUSBAR_ICON_AWMD && resID <= ID_STATUSBAR_ICON_BAT2) {
		if(getCurrentIconFileName(resID, file) < 0) {
			cvr_error("get current icon pic bmp failed\n");
			return -1;
		}
	}
	else if(resID >= ID_MENU_LIST_VQ && resID <= ID_MENU_LIST_UNCHECKBOX_PIC) {
		err_code = getMLPICFileName(resID, type, file);
		if(err_code < 0) {
			cvr_debug("get menu_list pic bmp failed\n");
			return -1;
		}
	}else if(resID == ID_SUBMENU_CHOICE_PIC){
		memcpy((void *)file, (void *)resSelfData->rMenuList.choicePic, ICON_PATH_SIZE);
	} else if(resID == ID_PLAYBACKPREVIEW_ICON || resID == ID_PLAYBACK_ICON) {
		if(getCurrentIconFileName(resID, file) < 0) {
			cvr_error("get current icon pic bmp failed\n");
			return -1;
		}
	} else if(resID == ID_POWEROFF) {
		memcpy((void *)file, (void *)resSelfData->rPoweroffPic, ICON_PATH_SIZE);
	} else if(resID == ID_CONNECT2PC) {
		if(getConnect2PcFileName(type, file) < 0 ) {
			cvr_error("get connect2pc pic bmp failed\n");
			return -1;
		}
	} else {
		cvr_debug("invalid resID: %d\n", resID);
		return -1;
	}

	//cvr_debug("resID: %d, bmp file is %s\n", resID, file);

	err_code = LoadBitmapFromFile(HDC_SCREEN, bmp, file);
	if(err_code != ERR_BMP_OK) {
		cvr_debug("load %s bitmap failed\n", file);
	}
	//cvr_debug("LoadBitmapFromFile %s finished\n", file);

	return 0;
}

int loadBmpFromConfig(const char* pSection, const char* pKey, PBITMAP bmp)
{

	int retval;
	char buf[64] = {0};

	if((retval = GetValueFromEtcFile(resSelfData->configFile, pSection, pKey, buf, sizeof(buf))) != ETC_OK) {
		cvr_error("get %s->%s failed\n", pSection, pKey);
		return -1;
	}

	if( (retval = LoadBitmapFromFile(HDC_SCREEN, bmp, buf)) != ERR_BMP_OK) {
		cvr_error("load %s failed: %s\n", buf);
		return -1;
	}

	return 0;
}

gal_pixel getResColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;

	switch(resID) {
	case ID_STATUSBAR:
		color = getStatusBarColor(resID, type);
		break;
	case ID_STATUSBAR_LABEL1:
	case ID_STATUSBAR_LABEL2:
	case ID_STATUSBAR_LABEL_RESERVE:
		color = getStatusBarLabelColor(resID, type);
		break;
	case ID_PLAYBACKPREVIEW_MENU:
	case ID_PLAYBACKPREVIEW_MB:
		color = getPlaybackPreviewColor(resID, type);
		break;
	case ID_PLAYBACK_PGB:
		color = getPlaybackColor(resID, type);
		break;
	case ID_MENU_LIST:
	case ID_MENU_LIST_MB:
	case ID_MENU_LIST_DATE:
		color = getMenuListColor(resID, type);
		break;
	case ID_SUBMENU:
		color = getSubMenuColor(resID, type);
		break;
	case ID_WARNNING_MB:
		color = getWarningMBColor(resID, type);
		break;
	case ID_CONNECT2PC:
		color = getConnect2PCColor(resID, type);
		break;
	case ID_TIPLABEL:
		if(type == COLOR_BGC)
			color = resSelfData->rTipLabel.bgcWidget;
		else if(type == COLOR_FGC)
			color = resSelfData->rTipLabel.fgcWidget;
		else if(type == COLOR_LINEC_TITLE)
			color = resSelfData->rTipLabel.linecTitle;
		break;
	default:
		cvr_debug("invalid resIndex for the color\n");
		break;
	}

	return color;
}

int getResRect(enum ResourceID resID, CvrRectType *rect)
{
	rect->x = 0;
	rect->y = 0;
	rect->w = 0;
	rect->h = 0;

	switch(resID) {
	case ID_SCREEN:
	case ID_POWEROFF:
		*rect = resSelfData->rScreenRect;
		break;
	case ID_STATUSBAR:
		*rect = resSelfData->rStatusBarData.STBRect;
		break;
	case ID_STATUSBAR_ICON_WINDOWPIC:
		*rect = resSelfData->rStatusBarData.windowPicRect;
		break;
	case ID_STATUSBAR_LABEL1:
		*rect = resSelfData->rStatusBarData.label1Rect;
		break;
	case ID_STATUSBAR_LABEL2:
		*rect = resSelfData->rStatusBarData.label2Rect;
		break;
	case ID_STATUSBAR_LABEL_RESERVE:
		*rect = resSelfData->rStatusBarData.reserveLabelRect;
		break;
	case ID_STATUSBAR_ICON_WIFI:
		*rect = resSelfData->rStatusBarData.wifiRect;
		break;
	case ID_STATUSBAR_ICON_PARK:
		*rect = resSelfData->rStatusBarData.parkRect;
		break;
	case ID_STATUSBAR_ICON_AWMD:
		*rect = resSelfData->rStatusBarData.awmdRect;
		break;
	case ID_STATUSBAR_ICON_LOCK:
		*rect = resSelfData->rStatusBarData.lockRect;
		break;
	case ID_STATUSBAR_ICON_UVC:
		*rect = resSelfData->rStatusBarData.uvcRect;
		break;
	case ID_STATUSBAR_ICON_TFCARD:
		*rect = resSelfData->rStatusBarData.tfCardRect;
		break;
	case ID_STATUSBAR_ICON_AUDIO:
		*rect = resSelfData->rStatusBarData.audioRect;
		break;
	case ID_STATUSBAR_ICON_BAT:
		*rect = resSelfData->rStatusBarData.batteryRect;
		break;
	case ID_STATUSBAR_ICON_WOND:
		*rect = resSelfData->rStatusBarData.wondRect;
		break;
	case ID_PLAYBACKPREVIEW_IMAGE:
		*rect = resSelfData->rPlayBackPreviewData.imageRect;
		break;
	case ID_PLAYBACKPREVIEW_ICON:
		*rect = resSelfData->rPlayBackPreviewData.iconRect;
		break;
	case ID_PLAYBACKPREVIEW_MENU:
		*rect = resSelfData->rPlayBackPreviewData.popupMenuRect;
		break;
	case ID_PLAYBACKPREVIEW_MB:
		*rect = resSelfData->rPlayBackPreviewData.messageBoxRect;
		break;
	case ID_PLAYBACK_ICON:
		*rect = resSelfData->rPlayBackData.iconRect;
		break;
	case ID_PLAYBACK_PGB:
		*rect = resSelfData->rPlayBackData.PGBRect;
		break;
	case ID_MENU_LIST:
		*rect  = resSelfData->rMenuList.menuListRect;
		break;
	case ID_SUBMENU:
		*rect  = resSelfData->rMenuList.subMenuRect;
		break;
	case ID_MENU_LIST_MB:
		*rect  = resSelfData->rMenuList.messageBoxRect;
		break;
	case ID_WARNNING_MB:
		*rect  = resSelfData->rWarningMB.rect;
		break;
	case ID_MENU_LIST_DATE:
		*rect  = resSelfData->rMenuList.dateRect;
		break;
	case ID_CONNECT2PC:
		*rect  = resSelfData->rConnectToPC.rect;
		break;
	case ID_TIPLABEL:
		*rect  = resSelfData->rTipLabel.rect;
		break;
	case ID_PREVIEW_SHOW:
		*rect  = resSelfData->rPreview.showRect;
		break;
	case ID_PREVIEW_PIP:
		*rect  = resSelfData->rPreview.pipRect;
		break;

	default:
		cvr_error("invalid resID index: %d\n", resID);
		return -1;
	}

	return 0;
}

int getRectFromFileHandle(const char *pWindow, CvrRectType *rect)
{
	int err_code;

	if(resSelfData->mCfgFileHandle == 0) {
		cvr_error("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetIntValueFromEtc(resSelfData->mCfgFileHandle, pWindow, "x", &rect->x)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resSelfData->mCfgFileHandle, pWindow, "y", &rect->y)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resSelfData->mCfgFileHandle, pWindow, "w", &rect->w)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resSelfData->mCfgFileHandle, pWindow, "h", &rect->h)) != ETC_OK) {
		return err_code;
	}

	return ETC_OK;
}

int getSubMenuCurrentIndex(enum ResourceID resID)
{
	int index;

	switch(resID) {
	case ID_MENU_LIST_VQ:
		index = resSelfData->menuDataVQ.current;
		break;
	case ID_MENU_LIST_PQ:
		index = resSelfData->menuDataPQ.current;
		break;
	case ID_MENU_LIST_VTL:
		index = resSelfData->menuDataVTL.current;
		break;
	case ID_MENU_LIST_WB:
		index = resSelfData->menuDataWB.current;
		break;
	case ID_MENU_LIST_CONTRAST:
		index = resSelfData->menuDataContrast.current;
		break;
	case ID_MENU_LIST_EXPOSURE:
		index = resSelfData->menuDataExposure.current;
		break;
	case ID_MENU_LIST_SS:
		index = resSelfData->menuDataSS.current;
		break;
	case ID_MENU_LIST_LANG:
		index = resSelfData->menuDataLang.current;
		break;
	case ID_MENU_LIST_GSENSOR:
		index = resSelfData->menuDataGsensor.current;
		break;
	case ID_MENU_LIST_VOICEVOL:
		index = resSelfData->menuDataVoiceVol.current;
		break;
	case ID_MENU_LIST_SHUTDOWN:
		index = resSelfData->menuDataShutDown.current;
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		index = resSelfData->menuDataImpactLevel.current;
		break;
	case ID_MENU_LIST_FCWSENSITY:
		index = resSelfData->menuDatafcwsensity.current;
		break;
	default:
		cvr_error("invalid resID: %d\n", resID);
		index = -1;
		break;
	}

	return index;
}

int getSubMenuCounts(enum ResourceID resID)
{
	int counts;

	switch(resID) {
	case ID_MENU_LIST_VQ:
		counts = resSelfData->menuDataVQ.count;
		break;
	case ID_MENU_LIST_PQ:
		counts = resSelfData->menuDataPQ.count;
		break;
	case ID_MENU_LIST_VTL:
		counts = resSelfData->menuDataVTL.count;
		break;
	case ID_MENU_LIST_WB:
		counts = resSelfData->menuDataWB.count;
		break;
	case ID_MENU_LIST_CONTRAST:
		counts = resSelfData->menuDataContrast.count;
		break;
	case ID_MENU_LIST_EXPOSURE:
		counts = resSelfData->menuDataExposure.count;
		break;
	case ID_MENU_LIST_SS:
		counts = resSelfData->menuDataSS.count;
		break;
	case ID_MENU_LIST_LANG:
		counts = resSelfData->menuDataLang.count;
		break;
	case ID_MENU_LIST_GSENSOR:
		counts = resSelfData->menuDataGsensor.count;
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		counts = resSelfData->menuDataImpactLevel.count;
		break;
	case ID_MENU_LIST_VOICEVOL:
		counts = resSelfData->menuDataVoiceVol.count;
		break;
	case ID_MENU_LIST_SHUTDOWN:
		counts = resSelfData->menuDataShutDown.count;
		break;
	case ID_MENU_LIST_FCWSENSITY:
		counts = resSelfData->menuDatafcwsensity.count;
		break;
	default:
		cvr_debug("invalid resID: %d\n", resID);
		counts = -1;
		break;
	}

	return counts;
}

int getSubMenuIntAttr(enum ResourceID resID, enum IntValueType type)
{
	int intValue = -1;
	if(resID == ID_MENU_LIST_DATE) {
		switch(type) {
		case INTVAL_TITLEHEIGHT:
			intValue = resSelfData->rMenuList.dateTileRectH;
			break;
		case INTVAL_HBORDER:
			intValue = resSelfData->rMenuList.dateHBorder;
			break;
		case INTVAL_YEARWIDTH:
			intValue = resSelfData->rMenuList.dateYearW;
			break;
		case INTVAL_LABELWIDTH:
			intValue = resSelfData->rMenuList.dateLabelW;
			break;
		case INTVAL_NUMBERWIDTH:
			intValue = resSelfData->rMenuList.dateNumberW;
			break;
		case INTVAL_BOXHEIGHT:
			intValue = resSelfData->rMenuList.dateBoxH;
			break;
		default:
			cvr_debug("invalid IntValueType: %d\n", type);
		}
	} else {
		cvr_error("invalid ResourceID: %d\n", resID);
	}

	return intValue;
}

void updateLangandFont(int langValue)
{
	LANGUAGE newLang;

	if(langValue == LANG_TW)
		newLang = LANG_TW;
	else if(langValue == LANG_CN)
		newLang = LANG_CN;
	else if(langValue == LANG_EN)
		newLang = LANG_EN;
	else if(langValue == LANG_JPN)
		newLang = LANG_JPN;
	else if(langValue == LANG_KR)
		newLang = LANG_KR;
	else if(langValue == LANG_RS)
		newLang = LANG_RS;
	else return;

	if(newLang == resSelfData->lang)
		return;

/*
	resSelfData->mCurLogFont = resSelfData->mLogFont;

#ifdef USE_IPS_SCREEN
	if (newLang == LANG_KR) {
		resSelfData->mCurLogFont = resSelfData->mLogFont_Malgun_Gothic;
	} else {
		resSelfData->mCurLogFont = resSelfData->mLogFont_MSYH;
	}
#endif
*/

	resSelfData->mCurLogFont = resSelfData->mLogFont_Times;
	resSelfData->lang = newLang;

	initLabel();
	SendMessage(resSelfData->mHwnd[WINDOW_MAIN_ID], MSG_LANG_CHANGED, (WPARAM)&resSelfData->lang, 0);
}

int setSubMenuCurrentIndex(enum ResourceID resID, int newSel)
{

	int count;
	switch(resID) {
	case ID_MENU_LIST_LANG:
		count = resSelfData->menuDataLang.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataLang.current = newSel;
		updateLangandFont(newSel);
		break;
	case ID_MENU_LIST_VQ:
		count = resSelfData->menuDataVQ.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataVQ.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_VIDEO_QUALITY, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_PQ:
		count = resSelfData->menuDataPQ.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataPQ.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_PIC_QUALITY, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_VTL:
		count = resSelfData->menuDataVTL.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataVTL.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_VIDEO_TIME_LENGTH, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_WB:
		count = resSelfData->menuDataWB.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataWB.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_WHITEBALANCE, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_CONTRAST:
		count = resSelfData->menuDataContrast.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataContrast.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_CONTRAST, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_EXPOSURE:
		count = resSelfData->menuDataExposure.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataExposure.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_EXPOSURE, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_SS:
		count = resSelfData->menuDataSS.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataSS.current = newSel;
		//SendMessage(mHwnd[WINDOWID_MAIN], MSG_RM_SCREENSWITCH, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_GSENSOR:
		count = resSelfData->menuDataGsensor.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataGsensor.current = newSel;
		//SendMessage(mHwnd[WINDOWID_MAIN], MSG_GSENSOR_SENSITIVITY, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_VOICEVOL:
		count = resSelfData->menuDataVoiceVol.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataVoiceVol.current = newSel;
		//mCdrVolume = VolNum[newSel].voice;
		//SendMessage(mHwnd[WINDOWID_PLAYBACK],MSG_SET_VOLUME,(WPARAM)(mCdrVolume*100),0);
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW],MSG_ADAS_AUDIO_VOL,(WPARAM)(mCdrVolume*10),0);
		//setKeySoundVol(mCdrVolume);
		break;
	case ID_MENU_LIST_SHUTDOWN:
		count = resSelfData->menuDataShutDown.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataShutDown.current = newSel;
		//SendMessage(mHwnd[WINDOWID_MAIN], MSG_AUTO_SHUTDOWN, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_IMPACTLEVEL:
		count = resSelfData->menuDataImpactLevel.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDataImpactLevel.current = newSel;
		//SendMessage(mHwnd[WINDOWID_MAIN], MSG_IMPACTLEVEL_VALUE, (WPARAM)newSel, 0);
		break;
	case ID_MENU_LIST_FCWSENSITY:
		count = resSelfData->menuDatafcwsensity.count;
		if(newSel >= count) {
			cvr_error("invalid value: %d, count is %d\n", newSel, count);
			return -1;
		}
		resSelfData->menuDatafcwsensity.current = newSel;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_ADAS_FCW_SENSITY, (WPARAM)newSel, 0);
		break;
	default:
		cvr_error("invalid resID: %d\n", resID);
		return -1;
	}

	return 0;
}

int setResIntValue(enum ResourceID resID, enum IntValueType type, int value)
{
	if(resID >= ID_MENU_LIST_VQ && resID <= ID_MENU_LIST_FIRMWARE) {
		if(type == INTVAL_SUBMENU_INDEX)
			return setSubMenuCurrentIndex(resID, value);
		else {
			cvr_error("invalide IntValueType: %d\n", type);
		}
	} else {
		cvr_error("invalid resID %d\n", resID);
	}

	return -1;
}

int getResIntValue(enum ResourceID resID, enum IntValueType type)
{
	unsigned int intValue = -1;

	if(resID >= ID_MENU_LIST_VQ && resID <= ID_MENU_LIST_FIRMWARE) {
		if(type == INTVAL_SUBMENU_INDEX) {
			return getSubMenuCurrentIndex(resID);
		} else if(type == INTVAL_SUBMENU_COUNT) {
			return getSubMenuCounts(resID);
		} else {
			return getSubMenuIntAttr(resID, type);
		}
	} else {
		switch(resID) {
		case ID_PLAYBACKPREVIEW_MENU:
			if(type == INTVAL_ITEMWIDTH)
				intValue = resSelfData->rPlayBackPreviewData.popupMenuItemWidth;
			else if(type == INTVAL_ITEMHEIGHT)
				intValue = resSelfData->rPlayBackPreviewData.popupMenuItemHeight;
			else
				cvr_error("invalid IntValueType: %d\n", type);
			break;
		case ID_PLAYBACKPREVIEW_MB:
			if(type == INTVAL_ITEMWIDTH)
				intValue = resSelfData->rPlayBackPreviewData.messageBoxItemWidth;
			else if(type == INTVAL_ITEMHEIGHT)
				intValue = resSelfData->rPlayBackPreviewData.messageBoxItemHeight;
			else
				cvr_error("invalid IntValueType: %d\n", type);
			break;
		case ID_CONNECT2PC:
			if(type == INTVAL_ITEMWIDTH)
				intValue = resSelfData->rConnectToPC.itemWidth;
			else if(type == INTVAL_ITEMHEIGHT)
				intValue = resSelfData->rConnectToPC.itemHeight;
			else
				cvr_error("invalid IntValueType: %d\n", type);
			break;
		case ID_TIPLABEL:
			if(type == INTVAL_TITLEHEIGHT)
				intValue = resSelfData->rTipLabel.titleHeight;
			else
				cvr_error("invalid IntValueType: %d\n", type);
			break;
		default:
			cvr_error("invalid resID: %d\n", resID);
			break;
		}
	}

	return intValue;
}

int getResBmpSubMenuCheckbox(enum ResourceID resID, bool isHilight, BITMAP *bmp)
{

	bool isChecked = CVR_FALSE;
	switch(resID) {
	case ID_MENU_LIST_WIFI:
		if(resSelfData->menuDataWIFIEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_PARK:
		if(resSelfData->menuDataPARKEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_ALIGNLINE:
		if(resSelfData->menuDataAlignlineEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_POR:
		if(resSelfData->menuDataPOREnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_SILENTMODE:
		if(resSelfData->menuDataVolumeEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_TWM:
		if(resSelfData->menuDataTWMEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	case ID_MENU_LIST_AWMD:
		if(resSelfData->menuDataAWMDEnable == CVR_TRUE)
			isChecked = CVR_TRUE;
		break;
	default:
		cvr_error("invalid resID: %d\n", resID);
		return -1;
	}

	if(isChecked == CVR_TRUE) {
		if(isHilight == CVR_TRUE)
			return getResBmp(ID_MENU_LIST_CHECKBOX_PIC, BMPTYPE_SELECTED, bmp);
		else
			return getResBmp(ID_MENU_LIST_CHECKBOX_PIC, BMPTYPE_UNSELECTED, bmp);
	} else {
		if(isHilight == CVR_TRUE)
			return getResBmp(ID_MENU_LIST_UNCHECKBOX_PIC, BMPTYPE_SELECTED, bmp);
		else
			return getResBmp(ID_MENU_LIST_UNCHECKBOX_PIC, BMPTYPE_UNSELECTED, bmp);
	}

}

void unloadBitMap(BITMAP *bmp)
{
	if(bmp->bmBits != NULL)
	{
		UnloadBitmap(bmp);

		bmp->bmBits = NULL;
	}
}

bool getResBoolValue(enum ResourceID resID)
{
	bool value = CVR_FALSE;
	switch(resID) {
	case ID_MENU_LIST_WIFI:
		value = resSelfData->menuDataWIFIEnable;
		break;
	case ID_MENU_LIST_PARK:
		value = resSelfData->menuDataPARKEnable;
		break;
	case ID_MENU_LIST_ALIGNLINE:
		value = resSelfData->menuDataAlignlineEnable;
		break;
	case ID_MENU_LIST_POR:
		value = resSelfData->menuDataPOREnable;
		break;
	case ID_MENU_LIST_SILENTMODE:
		value = resSelfData->menuDataVolumeEnable;
		break;
	case ID_MENU_LIST_TWM:
		value = resSelfData->menuDataTWMEnable;
		break;
	case ID_MENU_LIST_LWM:
		value = resSelfData->menuDataLWM.LWMEnable;
		break;
	case ID_MENU_LIST_AWMD:
		value = resSelfData->menuDataAWMDEnable;
		break;
	default:
		cvr_error("invalid resID %d\n", resID);
	}

	return value;
}

int setResBoolValue(enum ResourceID resID, bool value)
{
	switch(resID) {
	case ID_MENU_LIST_WIFI:
		resSelfData->menuDataWIFIEnable = value;
		//SendMessage(resSelfData->mHwnd[WINDOW_HBAR_ID], STBM_WIFI, (WPARAM)value, 0);
		//SendMessage(mHwnd[WINDOWID_MAIN], MSG_WIFI_SWITCH, (WPARAM)value, 0);
		break;
	case ID_MENU_LIST_PARK:
		resSelfData->menuDataPARKEnable = value;
		if(value == CVR_TRUE)
			resSelfData->rStatusBarData.parkIcon.current = 1;
		else
			resSelfData->rStatusBarData.parkIcon.current = 0;
		//SendMessage(mHwnd[WINDOWID_STATUSBAR], STBM_PARK, (WPARAM)value, 0);
		break;
	case ID_MENU_LIST_ALIGNLINE:
		resSelfData->menuDataAlignlineEnable = value;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_DRAW_ALIGN_LINE, value, 0);
		break;
	case ID_MENU_LIST_POR:
		resSelfData->menuDataPOREnable = value;
		break;
	case ID_STATUSBAR_ICON_AUDIO:
	case ID_MENU_LIST_SILENTMODE:
		resSelfData->menuDataVolumeEnable = value;
		if(value == CVR_TRUE)
			resSelfData->rStatusBarData.audioIcon.current = 1;
		else
			resSelfData->rStatusBarData.audioIcon.current = 0;

		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_AUDIO_SILENT, (WPARAM)value, 0);
		SendMessage(resSelfData->mHwnd[WINDOW_HBAR_ID], MSG_HBAR_UPDATAE_AUDIO, (WPARAM)value, 0);
		break;
	case ID_MENU_LIST_TWM:
		resSelfData->menuDataTWMEnable = value;
		//notifyWaterMark();
		break;
	case ID_MENU_LIST_LWM:
		resSelfData->menuDataLWM.LWMEnable = value;
		//notifyWaterMark();
		break;
	case ID_MENU_LIST_AWMD:
		resSelfData->menuDataAWMDEnable = value;
		//SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_AWMD, (WPARAM)value, 0);
		//SendMessage(mHwnd[WINDOWID_STATUSBAR], STBM_AWMD, (WPARAM)value, 0);
		break;
	default:
		cvr_error("invalide resID: %d\n", resID);
		return -1;
	}

	return 0;
}

int getABGRColorFromFileHandle(const char* pWindow, const char* subkey, gal_pixel *pixel)
{
	char buf[20] = {0};
	int err_code;
	unsigned long int hex;
	unsigned char r, g, b, a;

	if(pWindow == NULL || subkey == NULL)
		return ETC_INVALIDOBJ;

	if(resSelfData->mCfgFileHandle == 0) {
		cvr_debug("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(resSelfData->mCfgFileHandle, pWindow, subkey, buf, sizeof(buf))) != ETC_OK) {
		return err_code;
	}

	hex = strtoul(buf, NULL, 16);

	a = (hex >> 24) & 0xff;
	b = (hex >> 16) & 0xff;
	g = (hex >> 8) & 0xff;
	r = hex & 0xff;

	//cvr_debug("abgr:%x, %x, %x, %x\n", a, b, g, r);

	*pixel = RGBA2Pixel(HDC_SCREEN, r, g, b, a);

	//cvr_debug("*pixel=%x\n\n", *pixel);

	return ETC_OK;
}

int getValueFromFileHandle(const char*mainkey, const char* subkey, char* value, unsigned int len)
{
	int err_code;
	if(mainkey == NULL || subkey == NULL)
	{
		cvr_debug("NULL pointer\n");
		return ETC_INVALIDOBJ;
	}

	if(resSelfData->mCfgFileHandle == 0) {
		cvr_debug("cfgHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(resSelfData->mCfgFileHandle, mainkey, subkey, value, len)) != ETC_OK) {
		cvr_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
		return err_code;
	}

	return ETC_OK;
}

int getCfgFileHandle(void)
{
	if (resSelfData->mCfgFileHandle != 0)
	{
		return 0;
	}

	resSelfData->mCfgFileHandle = LoadEtcFile(resSelfData->configFile);
	if(resSelfData->mCfgFileHandle == 0)
	{
		cvr_debug("getCfgFileHandle failed\n");
		return -1;
	}

	return 0;
}

int getCfgMenuHandle(void)
{
	if (resSelfData->mCfgMenuHandle != 0) {
		return 0;
	}
	resSelfData->mCfgMenuHandle = LoadEtcFile(resSelfData->configMenu);
	if(resSelfData->mCfgMenuHandle == 0) {
		cvr_debug("getCfgMenuHandle failed\n");
		return -1;
	}

	return 0;
}

void releaseCfgMenuHandle(void)
{
	if(resSelfData->mCfgMenuHandle == 0) {
		return;
	}
	UnloadEtcFile(resSelfData->mCfgMenuHandle);
	resSelfData->mCfgMenuHandle = 0;
}

bool verifyConfig()
{
	int retval;
	int err_code;

	if((err_code = GetIntValueFromEtc(resSelfData->mCfgMenuHandle, CFG_VERIFICATION, "current", &retval)) != ETC_OK) {
		cvr_debug("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
		return 0;
	}

	if (retval != 1) {
		cvr_debug("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
		return 0;
	}
	cvr_debug("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
	return 1;
}

int copyFile(const char* from, const char* to)
{
	cvr_debug("copy file");

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

	int from_fd,to_fd;
	int bytes_read,bytes_write;
	char buffer[BUFFER_SIZE + 1];
	char *ptr;
	int retval = 0;

	if(access(from, F_OK | R_OK) != 0) {
		cvr_error("access error\n");
		return -1;
	}

	if((from_fd=open(from, O_RDONLY)) == -1 )
	{
		cvr_error("Open %s\n", from);
		return -1;
	}

	if((to_fd = open(to, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
	{
		cvr_error("Open %s Error\n", to);
		close(from_fd);
		return -1;
	}

	while( (bytes_read = read(from_fd, buffer, BUFFER_SIZE)) )
	{
		if((bytes_read == -1)) {
			cvr_error("read error\n");
			retval = -1;
			break;
		}

		else if(bytes_read > 0)
		{
			ptr = buffer;
			while( (bytes_write = write(to_fd, ptr, bytes_read)) )
			{
				if((bytes_write == -1)) {
					cvr_error("wirte error\n");
					retval = -1;
					break;
				}
				else if(bytes_write == bytes_read) {
					break;
				}
				else if( bytes_write > 0 )
				{
					ptr += bytes_write;
					bytes_read -= bytes_write;
				}
			}
			if(bytes_write == -1) {
				retval = -1;
				break;
			}
		}
	}
	cvr_debug("copy %s to %s\n", from, to);
	fsync(to_fd);
	close(from_fd);
	close(to_fd);
	return retval;
}


int getIntValueFromHandle(GHANDLE cfgHandle, const char* mainkey, const char* subkey)
{
	int retval = 0;
	int err_code;
	if(mainkey == NULL || subkey == NULL) {
		cvr_debug("NULL pointer\n");
		return -1;
	}

	if(cfgHandle == 0) {
		cvr_debug("cfgHandle not initialized\n");
		return -1;
	}

	if((err_code = GetIntValueFromEtc(cfgHandle, mainkey, subkey, &retval)) != ETC_OK) {
		cvr_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
		return -1;
	}

	return retval;
}

int fillCurrentIconInfoFromFileHandle(const char* pWindow, currentIcon_t *currentIcon)
{
	int current;
	char buf[10] = {0};
	char bufIcon[ICON_PATH_SIZE] = {0};

	if(pWindow == NULL) {
		cvr_debug("invalid pWindow\n");
		return ETC_INVALIDOBJ;
	}

	current = getIntValueFromHandle(resSelfData->mCfgFileHandle, pWindow, "current");
	if(current < 0) {
		cvr_error("get current value from %s failed\n", pWindow);
		return ETC_KEYNOTFOUND;
	}
	currentIcon->current = current;
	//cvr_debug("current is %d\n", currentIcon->current);

	current = 0;
	do
	{
		int err_code;
		sprintf(buf, "%s%d", "icon", current);

		err_code = GetValueFromEtc(resSelfData->mCfgFileHandle, pWindow, buf, bufIcon, sizeof(bufIcon));
		if(err_code != ETC_OK) {
			if(current == 0) {
				return err_code;
			} else {
				break;
			}
		}

		if(current >= ICON_TOTAL_CNT)
		{
			cvr_error("current:%d\n", current);
			return -1;
		}

		currentIcon->iconName[current] = (char *)malloc(ICON_PATH_SIZE);
		if(currentIcon->iconName[current] == NULL)
		{
			cvr_error("malloc icon Name error\n");
			return -1;
		}

		memset((void *)currentIcon->iconName[current], 0, ICON_PATH_SIZE);
		memcpy((void *)currentIcon->iconName[current], (void *)bufIcon, sizeof(bufIcon));
		current++;

	}while(1);

	if(currentIcon->current >=ICON_TOTAL_CNT)
	{
		cvr_error("currentIcon->current:%d\n", currentIcon->current);
		return -1;
	}

	return ETC_OK;
}

void releaseCfgFileHandle(void)
{
	if(resSelfData->mCfgFileHandle == 0)
	{
		return;
	}

	UnloadEtcFile(resSelfData->mCfgFileHandle);
	resSelfData->mCfgFileHandle = 0;
}

int initStatusBarResource(void)
{
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = {
		{CFG_FILE_STB,			(void*)&resSelfData->rStatusBarData.STBRect },
		{CFG_FILE_STB_WNDPIC,	(void*)&resSelfData->rStatusBarData.windowPicRect },
		{CFG_FILE_STB_LABEL1,	(void*)&resSelfData->rStatusBarData.label1Rect },
		{CFG_FILE_STB_LABEL2,	(void*)&resSelfData->rStatusBarData.label2Rect },
		{CFG_FILE_STB_RESERVELABLE,	(void*)&resSelfData->rStatusBarData.reserveLabelRect },
		{CFG_FILE_STB_WIFI,			(void*)&resSelfData->rStatusBarData.wifiRect },
		{CFG_FILE_STB_PARK,		(void*)&resSelfData->rStatusBarData.parkRect },
		{CFG_FILE_STB_UVC,			(void*)&resSelfData->rStatusBarData.uvcRect },
		{CFG_FILE_STB_AWMD,			(void*)&resSelfData->rStatusBarData.awmdRect },
		{CFG_FILE_STB_LOCK,			(void*)&resSelfData->rStatusBarData.lockRect },
		{CFG_FILE_STB_TFCARD,		(void*)&resSelfData->rStatusBarData.tfCardRect },
		{CFG_FILE_STB_AUDIO,		(void*)&resSelfData->rStatusBarData.audioRect },
		{CFG_FILE_STB_BAT,			(void*)&resSelfData->rStatusBarData.batteryRect },
		{CFG_FILE_STB_WOND,         (void*)&resSelfData->rStatusBarData.wondRect},
	};

	configTable3 configTableColor[] = {
		{CFG_FILE_STB,              FGC_WIDGET, (void*)&resSelfData->rStatusBarData.fgc},
		{CFG_FILE_STB,              BGC_WIDGET, (void*)&resSelfData->rStatusBarData.bgc},
		{CFG_FILE_STB_LABEL1,       FGC_WIDGET, (void*)&resSelfData->rStatusBarData.label1Fgc},
		{CFG_FILE_STB_LABEL2,       FGC_WIDGET, (void*)&resSelfData->rStatusBarData.label2Fgc},
		{CFG_FILE_STB_RESERVELABLE, FGC_WIDGET, (void*)&resSelfData->rStatusBarData.reserveLabelFgc},
	};

	configTable3 configTableValue[] = {
		{CFG_FILE_STB_WNDPIC,  "record_preview",	(void*)resSelfData->rStatusBarData.recordPreviewPic },
		{CFG_FILE_STB_WNDPIC,  "screen_shot",		(void*)resSelfData->rStatusBarData.screenShotPic },
		{CFG_FILE_STB_WNDPIC,  "playback_preview",	(void*)resSelfData->rStatusBarData.playBackPreviewPic },
		{CFG_FILE_STB_WNDPIC,  "playback",			(void*)resSelfData->rStatusBarData.playBackPic },
		{CFG_FILE_STB_WNDPIC,  "menu",				(void*)resSelfData->rStatusBarData.menuPic },
	};

	configTable2 configTableIcon[] = {
		{CFG_FILE_STB_WIFI,			(void*)&resSelfData->rStatusBarData.wifiIcon},
		{CFG_FILE_STB_PARK,		(void*)&resSelfData->rStatusBarData.parkIcon},
		{CFG_FILE_STB_UVC,			(void*)&resSelfData->rStatusBarData.uvcIcon},
		{CFG_FILE_STB_AWMD,			(void*)&resSelfData->rStatusBarData.awmdIcon},
		{CFG_FILE_STB_LOCK,			(void*)&resSelfData->rStatusBarData.lockIcon},
		{CFG_FILE_STB_TFCARD,		(void*)&resSelfData->rStatusBarData.tfCardIcon },
		{CFG_FILE_STB_AUDIO,		(void*)&resSelfData->rStatusBarData.audioIcon },
		{CFG_FILE_STB_BAT,			(void*)&resSelfData->rStatusBarData.batteryIcon },
		{CFG_FILE_STB_BAT1,			(void*)&resSelfData->rStatusBarData.batteryIcon1 },
		{CFG_FILE_STB_BAT2,			(void*)&resSelfData->rStatusBarData.batteryIcon2 },
        {CFG_FILE_STB_WOND,         (void*)&resSelfData->rStatusBarData.wondIcon},
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CvrRectType*)configTableRect[i].addr );
		if(err_code != ETC_OK) {
			cvr_debug("get %s rect failed: %d\n", configTableRect[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			cvr_debug("get %s %s failed: %d\n", configTableColor[i].mainkey, configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableValue); i++) {
		err_code = getValueFromFileHandle(configTableValue[i].mainkey, configTableValue[i].subkey, buf, sizeof(buf));
		if(err_code != ETC_OK) {
			cvr_debug("get %s %s pic failed: %d\n", configTableValue[i].mainkey, configTableValue[i].subkey, err_code);
			retval = -1;
			goto out;
		}
		memcpy(configTableValue[i].addr, (void *)buf, sizeof(buf));
		//cvr_debug("%s, %s, %s, %d\n\n", resSelfData->rStatusBarData.recordPreviewPic, configTableValue[i].addr, buf, sizeof(buf));
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, (currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			cvr_error("get %s icon failed: %d\n", configTableIcon[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
		/*
		cvr_debug("%s current=%d\n", configTableIcon[i].mainkey, (*(currentIcon_t*)configTableIcon[i].addr).current);
		if((*(currentIcon_t*)configTableIcon[i].addr).iconName[0] != NULL)
		{
			cvr_debug("iconName[0]=%s\n", (*(currentIcon_t*)configTableIcon[i].addr).iconName[0]);
		}
		if((*(currentIcon_t*)configTableIcon[i].addr).iconName[1] != NULL)
		{
			cvr_debug("iconName[1]=%s\n\n", (*(currentIcon_t*)configTableIcon[i].addr).iconName[1]);
		}
		*/
	}

	out:
	return retval;
}

int initPlayBackPreviewResource(void)
{
	int err_code;
	int retval = 0;

	configTable2 configTableRect[] = {
		{CFG_FILE_PLBPREVIEW_IMAGE,			(void*)&resSelfData->rPlayBackPreviewData.imageRect},
		{CFG_FILE_PLBPREVIEW_ICON,			(void*)&resSelfData->rPlayBackPreviewData.iconRect},
		{CFG_FILE_PLBPREVIEW_MENU,			(void*)&resSelfData->rPlayBackPreviewData.popupMenuRect},
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	(void*)&resSelfData->rPlayBackPreviewData.messageBoxRect},
	};

	configTable3 configTableIntValue[] = {
		{CFG_FILE_PLBPREVIEW_MENU,			"item_width",  (void*)&resSelfData->rPlayBackPreviewData.popupMenuItemWidth  },
		{CFG_FILE_PLBPREVIEW_MENU,			"item_height", (void*)&resSelfData->rPlayBackPreviewData.popupMenuItemHeight },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	"item_width",  (void*)&resSelfData->rPlayBackPreviewData.messageBoxItemWidth },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	"item_height", (void*)&resSelfData->rPlayBackPreviewData.messageBoxItemHeight}
	};

	configTable3 configTableColor[] = {
		{CFG_FILE_PLBPREVIEW_MENU, BGC_WIDGET,			(void*)&resSelfData->rPlayBackPreviewData.popupMenuBgcWidget },
		{CFG_FILE_PLBPREVIEW_MENU, BGC_ITEM_NORMAL,		(void*)&resSelfData->rPlayBackPreviewData.popupMenuBgcItemNormal },
		{CFG_FILE_PLBPREVIEW_MENU, BGC_ITEM_FOCUS,		(void*)&resSelfData->rPlayBackPreviewData.popUpMenuBgcItemFocus },
		{CFG_FILE_PLBPREVIEW_MENU, MAINC_THREED_BOX,	(void*)&resSelfData->rPlayBackPreviewData.popUpMenuMain3dBox },

		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_WIDGET,			(void*)&resSelfData->rPlayBackPreviewData.messageBoxBgcWidget },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, FGC_WIDGET,			(void*)&resSelfData->rPlayBackPreviewData.messageBoxFgcWidget },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_ITEM_NORMAL,		(void*)&resSelfData->rPlayBackPreviewData.messageBoxBgcItemNormal },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_ITEM_FOCUS,		(void*)&resSelfData->rPlayBackPreviewData.messageBoxBgcItemFous },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, MAINC_THREED_BOX,		(void*)&resSelfData->rPlayBackPreviewData.messageBoxMain3dBox },
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CvrRectType*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			cvr_debug("get %s rect failed\n", configTableRect[i].mainkey);
			retval = -1;
			goto out;
		}
	}

	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLBPREVIEW_ICON, &resSelfData->rPlayBackPreviewData.icon);
	if(err_code != ETC_OK) {
		cvr_debug("get playback preview current icon info failed: %s\n");
		retval = -1;
		goto out;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(resSelfData->mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			cvr_debug("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			cvr_debug("get %s %s failed\n", configTableColor[i].mainkey, configTableColor[i].subkey);
			retval = -1;
			goto out;
		}
	}

out:
	return retval;
}

int initPlayBackResource(void)
{
	int err_code;
	int retval = 0;

	err_code = getRectFromFileHandle(CFG_FILE_PLB_ICON, &resSelfData->rPlayBackData.iconRect);
	if(err_code != ETC_OK) {
		cvr_error("get playback icon rect failed\n");
		retval = -1;
		goto out;
	}
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLB_ICON, &resSelfData->rPlayBackData.icon);
	if(err_code != ETC_OK) {
		cvr_error("get playback current icon info failed\n");
		retval = -1;
		goto out;
	}

	err_code = getRectFromFileHandle(CFG_FILE_PLB_PGB, &resSelfData->rPlayBackData.PGBRect);
	if(err_code != ETC_OK) {
		cvr_error("get playback ProgressBar rect failed\n");
		retval = -1;
		goto out;
	}
	err_code = getABGRColorFromFileHandle(CFG_FILE_PLB_PGB, BGC_WIDGET, &resSelfData->rPlayBackData.PGBBgcWidget);
	if(err_code != ETC_OK) {
		cvr_error("get playback bgc_widget failed\n");
		retval = -1;
		goto out;
	}
	err_code = getABGRColorFromFileHandle(CFG_FILE_PLB_PGB, FGC_WIDGET, &resSelfData->rPlayBackData.PGBFgcWidget);
	if(err_code != ETC_OK) {
		cvr_error("get playback fgc_widget failed\n");
		retval = -1;
		goto out;
	}

out:
	return retval;
}

int initMenuListResource(void)
{
	int err_code;
	int retval = 0;
	char buf[64];

	configTable3 configTableValue[] = {
		{CFG_FILE_ML, "choice",						(void*)resSelfData->rMenuList.choicePic },
		{CFG_FILE_ML, "checkbox_checked_normal",	(void*)resSelfData->rMenuList.checkedNormalPic },
		{CFG_FILE_ML, "checkbox_checked_press",		(void*)resSelfData->rMenuList.checkedPressPic },
		{CFG_FILE_ML, "checkbox_unchecked_normal",	(void*)resSelfData->rMenuList.uncheckedNormalPic },
		{CFG_FILE_ML, "checkbox_unchecked_press",	(void*)resSelfData->rMenuList.uncheckedPressPic },
		{CFG_FILE_ML, "unfold_normal",	(void*)resSelfData->rMenuList.unfoldNormalPic },
		{CFG_FILE_ML, "unfold_press",	(void*)resSelfData->rMenuList.unfoldPressPic },
	};

	configTable3 configTableColor[] = {
		{CFG_FILE_ML, FGC_WIDGET,		(void*)&resSelfData->rMenuList.fgcWidget },
		{CFG_FILE_ML, BGC_WIDGET,		(void*)&resSelfData->rMenuList.bgcWidget },
		{CFG_FILE_ML, SUB_BGC_WIDGET,		(void*)&resSelfData->rMenuList.subbgcWidget },
		{CFG_FILE_ML, LINEC_ITEM,		(void*)&resSelfData->rMenuList.linecItem },
		{CFG_FILE_ML, STRINGC_NORMAL,	(void*)&resSelfData->rMenuList.stringcNormal },
		{CFG_FILE_ML, STRINGC_SELECTED, (void*)&resSelfData->rMenuList.stringcSelected },
		{CFG_FILE_ML, VALUEC_NORMAL,	(void*)&resSelfData->rMenuList.valuecNormal },
		{CFG_FILE_ML, VALUEC_SELECTED,	(void*)&resSelfData->rMenuList.valuecSelected },
		{CFG_FILE_ML, SCROLLBARC,		(void*)&resSelfData->rMenuList.scrollbarc },
		{CFG_FILE_SUBMENU, LINEC_TITLE,		(void*)&resSelfData->rMenuList.subMenuLinecTitle },
		{CFG_FILE_ML_MB, FGC_WIDGET,	(void*)&resSelfData->rMenuList.messageBoxFgcWidget },
		{CFG_FILE_ML_MB, BGC_WIDGET,	(void*)&resSelfData->rMenuList.messageBoxBgcWidget },
		{CFG_FILE_ML_MB, BGC_WIDGET1,	(void*)&resSelfData->rMenuList.messageBoxBgcWidget1 },
		{CFG_FILE_ML_MB, LINEC_TITLE,	(void*)&resSelfData->rMenuList.messageBoxLinecTitle },
		{CFG_FILE_ML_MB, LINEC_ITEM,	(void*)&resSelfData->rMenuList.messageBoxLinecItem },
		{CFG_FILE_ML_DATE, BGC_WIDGET,	(void*)&resSelfData->rMenuList.dateBgcWidget },
		{CFG_FILE_ML_DATE, "fgc_label",	(void*)&resSelfData->rMenuList.dateFgc_label },
		{CFG_FILE_ML_DATE, "fgc_number",	(void*)&resSelfData->rMenuList.dateFgc_number },
		{CFG_FILE_ML_DATE, LINEC_TITLE,	(void*)&resSelfData->rMenuList.dateLinecTitle },
		{CFG_FILE_ML_DATE, "borderc_selected",	(void*)&resSelfData->rMenuList.dateBordercSelected },
		{CFG_FILE_ML_DATE, "borderc_normal",	(void*)&resSelfData->rMenuList.dateBordercNormal },
	};

	configTable2 configTableRect[] = {
		{CFG_FILE_ML,		(void*)&resSelfData->rMenuList.menuListRect},
		{CFG_FILE_SUBMENU,	(void*)&resSelfData->rMenuList.subMenuRect},
		{CFG_FILE_ML_MB,	(void*)&resSelfData->rMenuList.messageBoxRect},
		{CFG_FILE_ML_DATE,	(void*)&resSelfData->rMenuList.dateRect},
	};

	configTable2 configTableIcon[] = {
		{CFG_FILE_ML_WIFI,	(void*)&resSelfData->rMenuList.wifiIcon},
		{CFG_FILE_ML_PARK,	(void*)&resSelfData->rMenuList.parkIcon},
		{CFG_FILE_ML_VQ,	(void*)&resSelfData->rMenuList.videoQualityIcon},
		{CFG_FILE_ML_PQ,	(void*)&resSelfData->rMenuList.photoQualityIcon},
		{CFG_FILE_ML_VTL,	(void*)&resSelfData->rMenuList.timeLengthIcon},
		{CFG_FILE_ML_AWMD,	(void*)&resSelfData->rMenuList.moveDetectIcon},
		{CFG_FILE_ML_WB,	(void*)&resSelfData->rMenuList.whiteBalanceIcon},
		{CFG_FILE_ML_CONTRAST,	(void*)&resSelfData->rMenuList.contrastIcon},
		{CFG_FILE_ML_EXPOSURE,	(void*)&resSelfData->rMenuList.exposureIcon},
		{CFG_FILE_ML_POR,		(void*)&resSelfData->rMenuList.PORIcon},
		{CFG_FILE_ML_SS,		(void*)&resSelfData->rMenuList.screenSwitchIcon},
		{CFG_FILE_ML_VOLUME,	(void*)&resSelfData->rMenuList.volumeIcon},
		{CFG_FILE_ML_DATE,		(void*)&resSelfData->rMenuList.dateIcon},
		{CFG_FILE_ML_LANG,		(void*)&resSelfData->rMenuList.languageIcon},
		{CFG_FILE_ML_TWM,		(void*)&resSelfData->rMenuList.TWMIcon},
		{CFG_FILE_ML_FORMAT,	(void*)&resSelfData->rMenuList.formatIcon},
		{CFG_FILE_ML_FACTORYRESET,	(void*)&resSelfData->rMenuList.factoryResetIcon},
		{CFG_FILE_ML_FIRMWARE,		(void*)&resSelfData->rMenuList.firmwareIcon},
		{CFG_FILE_ML_ALIGNLINE,                (void*)&resSelfData->rMenuList.alignlineIcon},
		{CFG_FILE_ML_GSENSOR,	(void*)&resSelfData->rMenuList.gsensorIcon},
		{CFG_FILE_ML_VOICEVOL,	(void*)&resSelfData->rMenuList.voicevolIcon},
		{CFG_FILE_ML_SHUTDOWN,	(void*)&resSelfData->rMenuList.shutdownIcon},
		{CFG_FILE_ML_IMPACTLEVEL,	(void*)&resSelfData->rMenuList.impactlevelIcon},
		{CFG_FILE_ML_FCWSENSITY,	(void*)&resSelfData->rMenuList.fcwsensityIcon},
		{CFG_FILE_ML_LICENSE_PLATE_WM,	(void*)&resSelfData->rMenuList.licensePlateWMIcon},
#ifdef APP_ADAS
		{CFG_FILE_ML_SMARTALGORITHM,                (void*)&resSelfData->rMenuList.smartalgorithm}
#endif
	};

	configTable3 configTableIntValue[] = {
		{CFG_FILE_ML_DATE,		"title_height",	(void*)&resSelfData->rMenuList.dateTileRectH  },
		{CFG_FILE_ML_DATE,		"hBorder",		(void*)&resSelfData->rMenuList.dateHBorder  },
		{CFG_FILE_ML_DATE,		"yearW",		(void*)&resSelfData->rMenuList.dateYearW  },
		{CFG_FILE_ML_DATE,		"dateLabelW",	(void*)&resSelfData->rMenuList.dateLabelW  },
		{CFG_FILE_ML_DATE,		"numberW",		(void*)&resSelfData->rMenuList.dateNumberW  },
		{CFG_FILE_ML_DATE,		"boxH",			(void*)&resSelfData->rMenuList.dateBoxH  }
	};


	for(unsigned int i = 0; i < TABLESIZE(configTableValue); i++) {
		err_code = getValueFromFileHandle(configTableValue[i].mainkey, configTableValue[i].subkey, buf, sizeof(buf));
		if(err_code != ETC_OK) {
			cvr_error("get %s %s pic failed\n", configTableValue[i].mainkey, configTableValue[i].subkey);
			retval = -1;
			goto out;
		}
		memcpy(configTableValue[i].addr, (void *)buf, sizeof(buf));
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			cvr_error("get %s %s failed\n", configTableColor[i].mainkey, configTableColor[i].subkey);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CvrRectType*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			cvr_error("get %s rect failed\n", configTableRect[i].mainkey);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, (currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			cvr_error("get %s icon failed\n", configTableIcon[i].mainkey);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(resSelfData->mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			cvr_debug("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

out:
	return retval;
}

int initMenuResource(void)
{
	int value;

	configTable2 configTableMenuItem[] = {
		{CFG_MENU_LANG, (void*)&resSelfData->menuDataLang},
		{CFG_MENU_VQ,	(void*)&resSelfData->menuDataVQ},
		{CFG_MENU_PQ,	(void*)&resSelfData->menuDataPQ},
		{CFG_MENU_VTL,	(void*)&resSelfData->menuDataVTL},
		{CFG_MENU_WB,	(void*)&resSelfData->menuDataWB},
		{CFG_MENU_CONTRAST, (void*)&resSelfData->menuDataContrast},
		{CFG_MENU_EXPOSURE, (void*)&resSelfData->menuDataExposure},
		{CFG_MENU_SS,		(void*)&resSelfData->menuDataSS},
		{CFG_MENU_GSENSOR,		(void*)&resSelfData->menuDataGsensor},
		{CFG_MENU_VOICEVOL,		(void*)&resSelfData->menuDataVoiceVol},
		{CFG_MENU_SHUTDOWN,		(void*)&resSelfData->menuDataShutDown},
		{CFG_MENU_IMPACTLEVEL,		(void*)&resSelfData->menuDataImpactLevel},
		{CFG_MENU_FCWSENSITY,		(void*)&resSelfData->menuDatafcwsensity},
#ifdef APP_ADAS
		{CFG_MENU_SMARTALGORITHM,	(void*)&resSelfData->menuDataSmartAlgorithmEnable}
#endif
	};

	configTable2 configTableSwitch[] = {
		{CFG_MENU_WIFI, (void*)&resSelfData->menuDataWIFIEnable},
		{CFG_MENU_PARK, (void*)&resSelfData->menuDataPARKEnable},
		{CFG_MENU_POR, (void*)&resSelfData->menuDataPOREnable},
		{CFG_MENU_VOLUME,	(void*)&resSelfData->menuDataVolumeEnable},
		{CFG_MENU_TWM,		(void*)&resSelfData->menuDataTWMEnable},
		{CFG_MENU_AWMD,		(void*)&resSelfData->menuDataAWMDEnable},
		{CFG_MENU_ALIGNLINE,	(void*)&resSelfData->menuDataAlignlineEnable},
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableMenuItem); i++) {
		value = getIntValueFromHandle(resSelfData->mCfgMenuHandle, configTableMenuItem[i].mainkey, "current");
		if(value < 0) {
			cvr_error("get current %s failed\n", configTableMenuItem[i].mainkey);
			return -1;
		}
		((menuItem_t*)(configTableMenuItem[i].addr))->current = value;

		value = getIntValueFromHandle(resSelfData->mCfgMenuHandle, configTableMenuItem[i].mainkey, "count");
		if(value < 0) {
			cvr_error("get count %s failed\n", configTableMenuItem[i].mainkey);
			return -1;
		}
		((menuItem_t*)(configTableMenuItem[i].addr))->count = value;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableSwitch); i++) {
		value = getIntValueFromHandle(resSelfData->mCfgMenuHandle, "switch", configTableSwitch[i].mainkey);
		if(value < 0) {
			cvr_error("get switch %s failed\n", configTableSwitch[i].mainkey);
			return -1;
		}
		if(value == 0) {
			*((bool*)configTableSwitch[i].addr) = CVR_FALSE;
		} else {
			*((bool*)configTableSwitch[i].addr) = CVR_TRUE;
		}
	}

	return 0;
}

int initOtherResource(void)
{
	char buf[64];
	int err_code;
	int retval = 0;
	configTable2 configTableRect[] = {
		{CFG_FILE_WARNING_MB,		(void*)&resSelfData->rWarningMB.rect},
		{CFG_FILE_CONNECT2PC,		(void*)&resSelfData->rConnectToPC.rect},
		{CFG_FILE_TIPLABEL,			(void*)&resSelfData->rTipLabel.rect},
		{CFG_FILE_SHOW_RECT,		(void*)&resSelfData->rPreview.showRect},
		{CFG_FILE_PIP_RECT,			(void*)&resSelfData->rPreview.pipRect}
	};

	configTable3 configTableColor[] = {
		{CFG_FILE_WARNING_MB, FGC_WIDGET,	(void*)&resSelfData->rWarningMB.fgcWidget },
		{CFG_FILE_WARNING_MB, BGC_WIDGET,	(void*)&resSelfData->rWarningMB.bgcWidget },
		{CFG_FILE_WARNING_MB, LINEC_TITLE,	(void*)&resSelfData->rWarningMB.linecTitle },
		{CFG_FILE_WARNING_MB, LINEC_ITEM,	(void*)&resSelfData->rWarningMB.linecItem },
		{CFG_FILE_CONNECT2PC, BGC_WIDGET,	(void*)&resSelfData->rConnectToPC.bgcWidget },
		{CFG_FILE_CONNECT2PC, BGC_ITEM_FOCUS,	(void*)&resSelfData->rConnectToPC.bgcItemFocus},
		{CFG_FILE_CONNECT2PC, BGC_ITEM_NORMAL,	(void*)&resSelfData->rConnectToPC.bgcItemNormal},
		{CFG_FILE_CONNECT2PC, MAINC_THREED_BOX,	(void*)&resSelfData->rConnectToPC.mainc_3dbox},
		{CFG_FILE_TIPLABEL,	  FGC_WIDGET,	(void*)&resSelfData->rTipLabel.fgcWidget},
		{CFG_FILE_TIPLABEL,	  BGC_WIDGET,	(void*)&resSelfData->rTipLabel.bgcWidget},
		{CFG_FILE_TIPLABEL,	  LINEC_TITLE,	(void*)&resSelfData->rTipLabel.linecTitle},
	};

	configTable3 configTableIntValue[] = {
		{CFG_FILE_CONNECT2PC, "item_width",		(void*)&resSelfData->rConnectToPC.itemWidth  },
		{CFG_FILE_CONNECT2PC, "item_height",	(void*)&resSelfData->rConnectToPC.itemHeight  },
		{CFG_FILE_TIPLABEL,	  "title_height",	(void*)&resSelfData->rTipLabel.titleHeight }
	};

	configTable3 configTableValue[] = {
		{CFG_FILE_OTHER_PIC,  "poweroff",		    (void*)&resSelfData->rPoweroffPic},
		{CFG_FILE_CONNECT2PC, "usb_storage_modem",	(void*)&resSelfData->rConnectToPC.usbStorageModem  },
		{CFG_FILE_CONNECT2PC, "pccam_modem",	    (void*)&resSelfData->rConnectToPC.pcCamModem  }
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CvrRectType*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			cvr_debug("get %s rect failed: %s\n", configTableRect[i].mainkey);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			cvr_debug("get %s %s failed: %s\n", configTableColor[i].mainkey, configTableColor[i].subkey);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(resSelfData->mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			cvr_debug("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableValue); i++) {
		err_code = getValueFromFileHandle(configTableValue[i].mainkey, configTableValue[i].subkey, buf, sizeof(buf));
		if(err_code != ETC_OK) {
			cvr_debug("get %s %s pic failed\n", configTableValue[i].mainkey, configTableValue[i].subkey);
			retval = -1;
			goto out;
		}
		memcpy(configTableValue[i].addr, (void *)buf, sizeof(buf));
	}

out:
	return retval;
}


LANGUAGE getLangFromConfigFile(void)
{
	int retval;
	if(GetIntValueFromEtcFile(resSelfData->configMenu, "language", "current", &retval) != ETC_OK)
	{
		cvr_error("get language failed\n");
		return LANG_ERR;
	}

	if(retval == LANG_CN)
		return LANG_CN;
	else if(retval == LANG_TW)
		return LANG_TW;
	else if(retval == LANG_EN)
		return LANG_EN;
	else if(retval == LANG_JPN)
		return LANG_JPN;
	else if(retval == LANG_KR)
		return LANG_KR;
	else if(retval == LANG_RS)
		return LANG_RS;

	return LANG_ERR;
}

static ssize_t  __getline(char **lineptr, ssize_t *n, FILE *stream)
{
	ssize_t count=0;
	int buf;

	if(*lineptr == NULL) {
		*n=256;
		*lineptr = (char*)malloc(*n);
	}

	if(( buf = fgetc(stream) ) == EOF ) {
		return -1;
	}

	do
	{
		if(buf=='\n') {
			count += 1;
			break;
		}

		count++;

		*(*lineptr+count-1) = buf;
		*(*lineptr+count) = '\0';

		if(*n <= count)
		{
			cvr_error("count=%d over *n=%d\n", count, *n);
			return -1;
		}

		buf = fgetc(stream);
	} while( buf != EOF);

	return count;
}


int loadLabelFromLangFile(char *langFile)
{
	FILE *fp;
	char *line = NULL;
	ssize_t len = 0;
	ssize_t rel_len = 0;
	int i = 0;

	fp = fopen(langFile, "r");
	if (fp == NULL) {
		cvr_error("open file %s failed\n", langFile);
		return -1;
	}

	i = 0;
	while ((rel_len = __getline(&line, &len, fp)) != -1)
	{
		resSelfData->label[i] = (char *)malloc(rel_len);
		if(resSelfData->label[i] == NULL)
		{
			cvr_error("malloc fail\n");
		}

		memcpy((void *)resSelfData->label[i], (void *)line, rel_len);
		resSelfData->label[i][rel_len-2] = '\0';
		resSelfData->label[i][rel_len-1] = '\0';
		//cvr_debug("i=%d, len=%d, str=%s\n\n", i, rel_len, resSelfData->label[i]);

		i++;
	}

	free(line);
	fclose(fp);

	return 0;
}

int initLabel(void)
{
	char dataFile[64];
	if(resSelfData->lang == LANG_ERR) {
		cvr_debug("lang is no initialized\n");
		return -1;
	}

	if(resSelfData->lang == LANG_CN)
	{
		memcpy((void*)dataFile, (void *)LANG_FILE_CN, sizeof(LANG_FILE_CN));
	}
	else if(resSelfData->lang == LANG_TW)
		memcpy((void*)dataFile, (void *)LANG_FILE_TW, sizeof(LANG_FILE_TW));
	else if(resSelfData->lang == LANG_EN)
		memcpy((void*)dataFile, (void *)LANG_FILE_EN, sizeof(LANG_FILE_EN));
	else if(resSelfData->lang == LANG_JPN)
		memcpy((void*)dataFile, (void *)LANG_FILE_JPN, sizeof(LANG_FILE_JPN));
	else if(resSelfData->lang == LANG_KR)
		memcpy((void*)dataFile, (void *)LANG_FILE_KR, sizeof(LANG_FILE_KR));
	else if(resSelfData->lang == LANG_RS)
		memcpy((void*)dataFile, (void *)LANG_FILE_RS, sizeof(LANG_FILE_RS));
	else {
		cvr_error("invalid lang %d\n", resSelfData->lang);
		return -1;
	}

	cvr_debug("lang=%d, dataFile=%s\n", resSelfData->lang, dataFile);

	if(loadLabelFromLangFile(dataFile) < 0) {
		cvr_error("load label from %s failed\n", dataFile);
		return -1;
	}

	return 0;
}

int initLangAndFont(void)
{
	resSelfData->lang = getLangFromConfigFile();
	if(resSelfData->lang == LANG_ERR) {
		cvr_error("get language failed\n");
		return -1;
	}

/*
#ifdef USE_IPS_SCREEN
		resSelfData->mLogFont = CreateLogFont("sxf", "arialuni", "UTF-8",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 30, 0);

		resSelfData->mLogFont_MSYH = CreateLogFont("sxf", "msyh", "UTF-8",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 30, 0);

		resSelfData->mLogFont_Malgun_Gothic = CreateLogFont("sxf", "gothic", "UTF-8",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 30, 0);
		if (resSelfData->lang == LANG_KR) {
			resSelfData->mCurLogFont = resSelfData->mLogFont_Malgun_Gothic;
		} else {
			resSelfData->mCurLogFont = resSelfData->mLogFont_MSYH;
		}
#else
		resSelfData->mLogFont = CreateLogFont("sxf", "arialuni", "UTF-8",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 16, 0);
		resSelfData->mCurLogFont = resSelfData->mLogFont;
#endif
*/
	/*
	resSelfData->mLogFont_Times = CreateLogFont("ttf", "times", "ISO8859-1",
		FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
        FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE,
		FONT_STRUCKOUT_NONE,
        16, 0);
    */

	// 使用系统字库
	/*
	resSelfData->mLogFont_Times = CreateLogFont("rbf", "fixed", "GB2312-0",
		FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
        FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE,
		FONT_STRUCKOUT_NONE,
        16, 0);
       */

	resSelfData->mLogFont_Times = CreateLogFont("rbf", "fixed", "GB2312-0",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
        FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE,
		FONT_STRUCKOUT_NONE,
        16, 0);

	resSelfData->mCurLogFont = resSelfData->mLogFont_Times;

	initLabel();

	return 0;
}

const char* getLabel(unsigned int labelIndex)
{
	if(labelIndex < LANG_LABEL_MAX)
	{
		return (const char*)resSelfData->label[labelIndex];
	}
	else
	{
		cvr_error("invalide label Index: %d, size is %zd\n", labelIndex, LANG_LABEL_MAX);
		return NULL;
	}
}

int initStage1(void)
{
	int retx;
	int err_code;
	int retval = 0;
	CvrRectType rScreenRect;

	if(getCfgMenuHandle() < 0) {
		cvr_warn("restore menu.cfg\n");
		if(copyFile(resSelfData->defaultConfigMenu, resSelfData->configMenu) < 0) {
			cvr_error("copyFile config file failed\n");
		}
	} else {
		if (!verifyConfig()){
			cvr_debug("restore menu.cfg\n");
			if(copyFile(resSelfData->defaultConfigMenu, resSelfData->configMenu) < 0) {
				cvr_error("copyFile config file failed\n");
			}
		}
		releaseCfgMenuHandle();
	}

	retx = LoadBitmapFromFile(HDC_SCREEN,&resSelfData->mNoVideoImage,"/usr/res/others/no_video_image.png");
	if(retx!= ERR_BMP_OK){
		cvr_error("load the mNoVideoImage bitmap error");
		resSelfData->mNoVideoImage.bmWidth= -1;
	}

	if(getCfgFileHandle() < 0) {
		cvr_error("get file handle failed\n");
		return -1;
	}

	err_code = getRectFromFileHandle("rect_screen", &resSelfData->rScreenRect);
	if(err_code != ETC_OK) {
		cvr_error("get screen rect failed: %d\n", err_code);
		retval = -1;
		goto out;
	}
	cvr_debug("Screen Rect:(%d, %d)\n", resSelfData->rScreenRect.w, resSelfData->rScreenRect.h);

	if(initStatusBarResource() < 0) {
		cvr_error("init status bar resource failed\n");
		retval = -1;
		goto out;
	}

out:
	releaseCfgFileHandle();
	releaseCfgMenuHandle();
	return retval;
}

int initStage2(void)
{
	int retval = 0;

	cvr_debug("initStage2\n");

	initLangAndFont();

	if(getCfgFileHandle() < 0) {
		cvr_error("get file handle failed\n");
		return -1;
	}
	if(getCfgMenuHandle() < 0) {
		cvr_error("get menu handle failed\n");
		retval = -1;
		goto out;
	}

	if(initPlayBackPreviewResource() < 0) {
		cvr_error("init PlayBackPreview resource failed\n");
		retval = -1;
		goto out;
	}

	if(initPlayBackResource() < 0) {
		cvr_error("init PlayBack resource failed\n");
		retval = -1;
		goto out;
	}

	if(initMenuListResource() < 0) {
		cvr_error("init Menu List resource failed\n");
		retval = -1;
		goto out;
	}

	if(initMenuResource() < 0) {
		cvr_error("init menu resource failed\n");
		retval = -1;
		goto out;
	}

	if(initOtherResource() < 0) {
		cvr_error("init other resource failed\n");
		retval = -1;
		goto out;
	}

out:
	releaseCfgFileHandle();
	releaseCfgMenuHandle();
	return retval;
}

int initStage3(void)
{
	SendMessage(resSelfData->mHwnd[WINDOW_MAIN_ID], MSG_LANG_CHANGED, (WPARAM)&resSelfData->lang, 0);

	return 0;
}

void setHwnd(unsigned int win_id, HWND hwnd)
{
	resSelfData->mHwnd[win_id] = hwnd;
}

void resetResource(void)
{
	releaseCfgMenuHandle();
	if(copyFile(resSelfData->defaultConfigMenu, resSelfData->configMenu) < 0) {
		cvr_error("copyFile config file failed\n");
	}

	if(initStage1() < 0)
	{
		cvr_error("initStage1 failed\n");
		return;
	}
	if(initStage2() < 0)
	{
		cvr_error("initStage2 failed\n");
		return;
	}
}

int GetUserData(UserDataType *config)
{
	config->pwr_record_en = resSelfData->menuDataPOREnable;
	config->voice_en = resSelfData->menuDataVolumeEnable;
	config->video_quality = resSelfData->menuDataVQ.current;
	config->photo_quality = resSelfData->menuDataPQ.current;
	config->video_time = resSelfData->menuDataVTL.current;
	config->gsensor_level = resSelfData->menuDataImpactLevel.current;
	config->voice_volume = resSelfData->menuDataVoiceVol.current;
	config->screen_switch = resSelfData->menuDataSS.current;

	config->time_water_en = resSelfData->menuDataTWMEnable;
	config->language = resSelfData->menuDataLang.current;

	return 0;
}

int sessionControl(int sid, int cmd, char* buf, void *srv)
{
	return 0;
}

int setRecordingStatus(bool recording)
{
	resSelfData->mRecording = recording;
	return 0;
}


void syncConfigureToDisk(void)
{
	char buf[3] = {0};

	configTable2 configTableSwitch[] = {
		{CFG_MENU_POR, (void*)&resSelfData->menuDataPOREnable},
		{CFG_MENU_VOLUME,	(void*)&resSelfData->menuDataVolumeEnable},
		{CFG_MENU_TWM,		(void*)&resSelfData->menuDataTWMEnable},
		{CFG_MENU_AWMD,		(void*)&resSelfData->menuDataAWMDEnable},
		{CFG_MENU_WIFI,		(void*)&resSelfData->menuDataWIFIEnable},
		{CFG_MENU_PARK,		(void*)&resSelfData->menuDataPARKEnable},
		{CFG_MENU_ALIGNLINE,		(void*)&resSelfData->menuDataAlignlineEnable}
	};

	configTable2 configTableMenuItem[] = {
		{CFG_MENU_LANG, (void*)&resSelfData->menuDataLang},
		{CFG_MENU_VQ,	(void*)&resSelfData->menuDataVQ},
		{CFG_MENU_PQ,	(void*)&resSelfData->menuDataPQ},
		{CFG_MENU_VTL,	(void*)&resSelfData->menuDataVTL},
		{CFG_MENU_WB,	(void*)&resSelfData->menuDataWB},
		{CFG_MENU_CONTRAST, (void*)&resSelfData->menuDataContrast},
		{CFG_MENU_EXPOSURE, (void*)&resSelfData->menuDataExposure},
		{CFG_MENU_SS,		(void*)&resSelfData->menuDataSS},
		{CFG_MENU_GSENSOR,	(void*)&resSelfData->menuDataGsensor},
		{CFG_MENU_IMPACTLEVEL,	(void*)&resSelfData->menuDataImpactLevel},
		{CFG_MENU_FCWSENSITY,	(void*)&resSelfData->menuDatafcwsensity},
		{CFG_MENU_VOICEVOL,	(void*)&resSelfData->menuDataVoiceVol},
		{CFG_MENU_SHUTDOWN,	(void*)&resSelfData->menuDataShutDown},
#ifdef APP_ADAS
		{CFG_MENU_SMARTALGORITHM,	(void*)&resSelfData->menuDataSmartAlgorithmEnable}
#endif
	};

	if(getCfgMenuHandle() < 0) {
		cvr_error("get config menu handle failed\n");
		return;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableSwitch); i++) {
		if(*(bool*)configTableSwitch[i].addr)
			buf[0] = '1';
		else
			buf[0] = '0';
		SetValueToEtc(resSelfData->mCfgMenuHandle, "switch", configTableSwitch[i].mainkey, buf);
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableMenuItem); i++) {
		sprintf(buf, "%d", ((menuItem_t*)(configTableMenuItem[i].addr))->current % 10);
		SetValueToEtc(resSelfData->mCfgMenuHandle, configTableMenuItem[i].mainkey, "current", buf);

		sprintf(buf, "%d", ((menuItem_t*)(configTableMenuItem[i].addr))->count % 10);
		SetValueToEtc(resSelfData->mCfgMenuHandle, configTableMenuItem[i].mainkey, "count", buf);
	}
#if 0
	sprintf(buf, "%d", (menuDataLWM.LWMEnable) % 10);
	db_msg("menuDataLWM.LWMEnable buf is %s\n", buf);
	SetValueToEtc(mCfgMenuHandle, CFG_MENU_DEFAULTLICENSE , "current", buf);

//	if(menuDataLWM.LWMEnable && (menuDataLWM.lwaterMark[9] != 0) )
	if(menuDataLWM.LWMEnable  )
		SetValueToEtc(mCfgMenuHandle, CFG_MENU_DEFAULTLICENSE , "position", menuDataLWM.lwaterMark);

	db_msg("menuDataLWM.lwaterMark  is %s\n", menuDataLWM.lwaterMark);
#endif
	SetValueToEtc(resSelfData->mCfgMenuHandle, CFG_VERIFICATION, "current", (char*)"1");
	SaveEtcToFile(resSelfData->mCfgMenuHandle, resSelfData->configMenu);
	releaseCfgMenuHandle();
	cvr_debug("syncConfigureToDisk finished\n");
}


void ResourceInit(void)
{
	char cfgStr[] = "/usr/res/cfg/icon.cfg";
	char menuStr[] = "/usr/res/data/menu.cfg";
	char defMenuStr[] = "/usr/res/cfg/menu.cfg";

	resSelfData = (ResSelfDataType*)malloc(sizeof(ResSelfDataType));
	if(NULL == resSelfData)
	{
		cvr_error("malloc resource error\n");
		return;
	}
	memset((void *)resSelfData, 0, sizeof(ResSelfDataType));

	memcpy((void *)resSelfData->configFile, (void *)cfgStr, sizeof(cfgStr));
	memcpy((void *)resSelfData->configMenu, (void *)menuStr, sizeof(menuStr));
	memcpy((void *)resSelfData->defaultConfigMenu, (void *)defMenuStr, sizeof(defMenuStr));

	if(initStage1() < 0)
	{
		cvr_debug("init stage1 error\n");
	}

	if(initStage2() < 0)
	{
		cvr_debug("init stage2 error\n");
	}

	return;
}

void ResourceUninit(void)
{
	cvr_debug("resource uninit\n");
	UnloadBitmap(&resSelfData->mNoVideoImage);
	syncConfigureToDisk();
	return;
}
