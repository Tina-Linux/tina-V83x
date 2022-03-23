#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "smart_resource.h"
#include "common.h"

#if ((MINIGUI_MAJOR_VERSION >= 2 && MINIGUI_MICRO_VERSION >= 2 && MINIGUI_MINOR_VERSION >= 2) \
        || (MINIGUI_MAJOR_VERSION >= 3))

ResDataType *resData = NULL;

void setHwnd(unsigned int win_id, HWND hwnd)
{
	resData->mHwnd[win_id] = hwnd;
}

int initLangAndFont(void)
{
	resData->mLogFont_Times16 = CreateLogFont("ttf", "times", "ISO8859-1",
		FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
        FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE,
		FONT_STRUCKOUT_NONE,
        16, 0);

	resData->mFont_Times45 = CreateLogFont ("ttf", "times", "ISO8859-1",
		FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE,
		FONT_STRUCKOUT_NONE,
		45, 0);

	resData->mFont_Times150 = CreateLogFont ("ttf", "times", "ISO8859-1",
    	FONT_WEIGHT_BLACK, FONT_SLANT_ROMAN, FONT_SETWIDTH_BOLD,
    	FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE,
    	FONT_STRUCKOUT_NONE,
        150, 0);

	resData->mCurLogFont = resData->mLogFont_Times16;

	return 0;
}

int deinitFont(void)
{
	DestroyLogFont (resData->mLogFont_Times16);
	DestroyLogFont (resData->mFont_Times45);
	DestroyLogFont (resData->mFont_Times150);
	return 0;
}

void unloadBitMap(BITMAP *bmp)
{
	if(bmp->bmBits != NULL)
	{
		UnloadBitmap(bmp);

		bmp->bmBits = NULL;
	}
}

int copyFile(const char* from, const char* to)
{
	sm_debug("copy file");

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

	int from_fd,to_fd;
	int bytes_read,bytes_write;
	char buffer[BUFFER_SIZE + 1];
	char *ptr;
	int retval = 0;

	if(access(from, F_OK | R_OK) != 0) {
		sm_error("access error\n");
		return -1;
	}

	if((from_fd=open(from, O_RDONLY)) == -1 )
	{
		sm_error("Open %s\n", from);
		return -1;
	}

	if((to_fd = open(to, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
	{
		sm_error("Open %s Error\n", to);
		close(from_fd);
		return -1;
	}

	while( (bytes_read = read(from_fd, buffer, BUFFER_SIZE)) )
	{
		if((bytes_read == -1)) {
			sm_error("read error\n");
			retval = -1;
			break;
		}

		else if(bytes_read > 0)
		{
			ptr = buffer;
			while( (bytes_write = write(to_fd, ptr, bytes_read)) )
			{
				if((bytes_write == -1)) {
					sm_error("wirte error\n");
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
	sm_debug("copy %s to %s\n", from, to);
	fsync(to_fd);
	close(from_fd);
	close(to_fd);
	return retval;
}

PLOGFONT getLogFont(enum FontID id)
{
	switch(id) {
	case ID_FONT_TIMES_16:
		resData->mCurLogFont = resData->mLogFont_Times16;
		break;
	case ID_FONT_TIMES_45:
		resData->mCurLogFont = resData->mFont_Times45;
		break;
	case ID_FONT_TIMES_150:
		resData->mCurLogFont = resData->mFont_Times150;
		break;
	default:
		sm_error("invalid Font ID index: %d\n", id);
		break;
	}
	return resData->mCurLogFont;
}

gal_pixel getStatusBarColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_STATUSBAR) {
		switch(type) {
		case COLOR_FGC:
			color = resData->rStatusBarData.fgc;
			break;
		case COLOR_BGC:
			color = resData->rStatusBarData.bgc;
			break;
		default:
			sm_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_HEAD_BAR) {
		switch(type) {
		case COLOR_FGC:
			color = resData->rStatusBarData.head_bar_fgc;
			break;
		case COLOR_BGC:
			color = resData->rStatusBarData.head_bar_bgc;
			break;
		default:
			sm_debug("invalid ColorType: %d\n", type);
			break;
		}

	} else {
		sm_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getTempSettingColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_TEMP_SETTING) {
		switch(type) {
		case COLOR_FGC:
			color = resData->rTempSetData.fgc;
			break;
		case COLOR_BGC:
			color = resData->rTempSetData.bgc;
			break;
		default:
			sm_debug("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		sm_debug("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel getResColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;

	switch(resID) {
	case ID_STATUSBAR:
		color = getStatusBarColor(resID, type);
		break;
	case ID_TEMP_SETTING:
		color = getTempSettingColor(resID, type);
		break;
	case ID_HEAD_BAR:
		color = getStatusBarColor(resID, type);
		break;
	default:
		sm_debug("invalid resIndex for the color\n");
		break;
	}

	return color;
}

int getResRect(enum ResourceID resID, smRect *rect)
{
	memset(rect, 0, sizeof(smRect));
	switch(resID) {
	case ID_SCREEN:
		*rect = resData->rScreenRect;
		break;
	case ID_HEAD_BAR:
		*rect = resData->rheadbarRect;
		break;
	case ID_STATUSBAR:
		*rect = resData->rStatusBarData.STBRect;
		break;
	case ID_STATUSBAR_ICON_DATE:
		*rect = resData->rStatusBarData.dateRect;
		break;
	case ID_STATUSBAR_CUR_TIME:
		*rect = resData->rStatusBarData.curTimeRect;
		break;
	case ID_STATUSBAR_LOCK:
		*rect = resData->rStatusBarData.lockRect;
		break;
	case ID_STATUSBAR_SHAPE:
		*rect = resData->rStatusBarData.shapeRect;
		break;
	case ID_STATUSBAR_PREOPEN:
		*rect = resData->rStatusBarData.preOpenRect;
		break;
	case ID_STATUSBAR_BACK:
		*rect = resData->rStatusBarData.backRect;
		break;
	case ID_STATUSBAR_TITLE:
		*rect = resData->rStatusBarData.titleRect;
		break;
	case ID_STATUSBAR_ICON_ITEMS:
		*rect = resData->rStatusBarData.statItemsRect;
		break;
	case ID_STATUSBAR_FONT_ITEMS:
		*rect = resData->rStatusBarData.statItemsFontRect;
		break;
	case ID_MAINPAGE:
		*rect = resData->rMainPgeData.pgeRect;
		break;
	case ID_MAINPAGE_BTN_LEFT:
		*rect = resData->rMainPgeData.btnLeftRect;
		break;
	case ID_MAINPAGE_BTN_RIGHT:
		*rect = resData->rMainPgeData.btnRightRect;
		break;
	case ID_MAINPAGE_BTN_ITEM1:
		*rect = resData->rMainPgeData.btn_item[0];
		break;
	case ID_MAINPAGE_BTN_ITEM2:
		*rect = resData->rMainPgeData.btn_item[1];
		break;
	case ID_MAINPAGE_BTN_ITEM3:
		*rect = resData->rMainPgeData.btn_item[2];
		break;
	case ID_MAINPAGE_BTN_ITEM4:
		*rect = resData->rMainPgeData.btn_item[3];
		break;
	case ID_TEMP_SETTING:
		*rect = resData->rTempSetData.setRect;
		break;
	case ID_TEMP_SETTING_UP:
		*rect = resData->rTempSetData.btnUpRect;
		break;
	case ID_TEMP_SETTING_DOWN:
		*rect = resData->rTempSetData.btnDownRect;
		break;
	case ID_TEMP_SETTING_DEGREE:
		*rect = resData->rTempSetData.labelDegreeRect;
		break;
	default:
		sm_error("invalid resID index: %d\n", resID);
		return -1;
	}

	return 0;
}

void setCurrentIconValue(enum ResourceID resID, int cur_val)
{
	switch(resID) {
	case ID_STATUSBAR_LOCK:
		resData->rStatusBarData.lockIcon.current = cur_val;
		break;
	case ID_STATUSBAR_SHAPE:
		resData->rStatusBarData.shapeIcon.current = cur_val;
		break;
	case ID_STATUSBAR_PREOPEN:
		resData->rStatusBarData.preOpenIcon.current = cur_val;
		break;
	case ID_STATUSBAR_BACK:
		resData->rStatusBarData.backIcon.current = cur_val;
		break;
	case ID_STATUSBAR_TITLE:
		resData->rStatusBarData.titleIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_ITEMS:
		resData->rStatusBarData.statItemsIcon.current = cur_val;
		break;
	case ID_STATUSBAR_FONT_ITEMS:
		resData->rStatusBarData.statItemsFontIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_WIND:
		resData->rStatusBarData.statWindIcon.current = cur_val;
		break;
	case ID_STATUSBAR_FONT_WIND:
		resData->rStatusBarData.statWindFontIcon.current = cur_val;
		break;
	case ID_STATUSBAR_ICON_DIRECT:
		resData->rStatusBarData.statDirectIcon.current = cur_val;
		break;
	case ID_STATUSBAR_FONT_DIRECT:
		resData->rStatusBarData.statDirectFontIcon.current = cur_val;
		break;
	case ID_MAINPAGE_BTN_LEFT:
		resData->rMainPgeData.Iconleft.current = cur_val;
		break;
	case ID_MAINPAGE_BTN_RIGHT:
		resData->rMainPgeData.Iconright.current = cur_val;
		break;
	case ID_MAINPAGE_BTN_ITEM1:
		resData->rMainPgeData.Icon_item[0].current = cur_val;
		break;
	case ID_MAINPAGE_BTN_ITEM2:
		resData->rMainPgeData.Icon_item[1].current = cur_val;
		break;
	case ID_MAINPAGE_BTN_ITEM3:
		resData->rMainPgeData.Icon_item[2].current = cur_val;
		break;
	case ID_MAINPAGE_BTN_ITEM4:
		resData->rMainPgeData.Icon_item[3].current = cur_val;
	case ID_TEMP_SETTING_UP:
		resData->rTempSetData.Iconup.current = cur_val;
		break;
	case ID_TEMP_SETTING_DOWN:
		resData->rTempSetData.Icondown.current = cur_val;
		break;
	case ID_TEMP_SETTING_DEGREE:
		resData->rTempSetData.IconDegree.current = cur_val;
		break;
	default:
		sm_error("invalide resID: %d\n", resID);
		break;
	}
}

#define CHECK_CURRENT_ICON_VALID(curIcon) do{ \
	if(curIcon.current >= ICON_TOTAL_CNT) { \
		sm_error("invalid current value\n"); break; } \
}while(0)

int getCurrentIconFileName(enum ResourceID resID, char *file)
{
	int current;

	switch(resID) {
	case ID_SCREEN_BKG:
		CHECK_CURRENT_ICON_VALID(resData->bgImg);
		memcpy((void *)file, (void *)resData->bgImg.iconName[0], ICON_PATH_SIZE);
		break;
	case ID_HEAD_BKG:
		CHECK_CURRENT_ICON_VALID(resData->headbgImg);
		memcpy((void *)file, (void *)resData->headbgImg.iconName[0], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_LOCK:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.lockIcon);
		current = resData->rStatusBarData.lockIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.lockIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_SHAPE:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.shapeIcon);
		current = resData->rStatusBarData.shapeIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.shapeIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_PREOPEN:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.preOpenIcon);
		current = resData->rStatusBarData.preOpenIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.preOpenIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_BACK:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.backIcon);
		current = resData->rStatusBarData.backIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.backIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_TITLE:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.titleIcon);
		current = resData->rStatusBarData.titleIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.titleIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_ITEMS:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statItemsIcon);
		current = resData->rStatusBarData.statItemsIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statItemsIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_FONT_ITEMS:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statItemsFontIcon);
		current = resData->rStatusBarData.statItemsFontIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statItemsFontIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_WIND:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statWindIcon);
		current = resData->rStatusBarData.statWindIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statWindIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_FONT_WIND:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statWindFontIcon);
		current = resData->rStatusBarData.statWindFontIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statWindFontIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_ICON_DIRECT:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statDirectIcon);
		current = resData->rStatusBarData.statDirectIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statDirectIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_STATUSBAR_FONT_DIRECT:
		CHECK_CURRENT_ICON_VALID(resData->rStatusBarData.statDirectFontIcon);
		current = resData->rStatusBarData.statDirectFontIcon.current;
		memcpy((void *)file, (void *)resData->rStatusBarData.statDirectFontIcon.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_LEFT:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Iconleft);
		current = resData->rMainPgeData.Iconleft.current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Iconleft.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_RIGHT:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Iconright);
		current = resData->rMainPgeData.Iconright.current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Iconright.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_ITEM1:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Icon_item[0]);
		current = resData->rMainPgeData.Icon_item[0].current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Icon_item[0].iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_ITEM2:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Icon_item[1]);
		current = resData->rMainPgeData.Icon_item[1].current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Icon_item[1].iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_ITEM3:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Icon_item[2]);
		current = resData->rMainPgeData.Icon_item[2].current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Icon_item[2].iconName[current], ICON_PATH_SIZE);
		break;
	case ID_MAINPAGE_BTN_ITEM4:
		CHECK_CURRENT_ICON_VALID(resData->rMainPgeData.Icon_item[3]);
		current = resData->rMainPgeData.Icon_item[3].current;
		memcpy((void *)file, (void *)resData->rMainPgeData.Icon_item[3].iconName[current], ICON_PATH_SIZE);
		break;
	case ID_TEMP_SETTING_UP:
		CHECK_CURRENT_ICON_VALID(resData->rTempSetData.Iconup);
		current = resData->rTempSetData.Iconup.current;
		memcpy((void *)file, (void *)resData->rTempSetData.Iconup.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_TEMP_SETTING_DOWN:
		CHECK_CURRENT_ICON_VALID(resData->rTempSetData.Icondown);
		current = resData->rTempSetData.Icondown.current;
		memcpy((void *)file, (void *)resData->rTempSetData.Icondown.iconName[current], ICON_PATH_SIZE);
		break;
	case ID_TEMP_SETTING_DEGREE:
		CHECK_CURRENT_ICON_VALID(resData->rTempSetData.IconDegree);
		current = resData->rTempSetData.IconDegree.current;
		memcpy((void *)file, (void *)resData->rTempSetData.IconDegree.iconName[current], ICON_PATH_SIZE);
		break;
	default:
		sm_debug("invalid resID %d\n", resID);
		return -1;
	}
	return 0;
}

int getWindPicFileName(enum BmpType type, char *file)
{

	switch(type) {
	case BMPTYPE_SELECTED:
		break;
	default:
		sm_error("invalid BmpType %d\n", type);
		return -1;
	}

	return 0;
}

int getMLPICFileName(enum ResourceID resID, enum BmpType type, char *file)
{
	switch(resID){
		case ID_STATUSBAR_LOCK:
			if (type == BMPTYPE_SELECTED)
				memcpy((void *)file, (void *)resData->rStatusBarData.lockIcon.iconName[1], ICON_PATH_SIZE);
			else
				memcpy((void *)file, (void *)resData->rStatusBarData.lockIcon.iconName[0], ICON_PATH_SIZE);
			break;

		default:
			break;
	}
	return 0;
}

int getConnect2PcFileName(enum BmpType type, char *file)
{

	switch(type) {
	case ID_STATUSBAR_LOCK:

		break;
	default:
		sm_error("invalid BmpType %d\n", type);
		return -1;
	}

	return 0;
}

int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp)
{
	int err_code;
	char file[ICON_PATH_SIZE];

	if(resID == ID_STATUSBAR) {
		if(getWindPicFileName(type, file) < 0 ) {
			sm_error("get window pic bmp failed\n");
			return -1;
		}
	} else if(resID >= ID_SCREEN_BKG && resID <= ID_TEMP_SETTING_DEGREE) {
		if(getCurrentIconFileName(resID, file) < 0) {
			sm_error("get current icon pic bmp failed\n");
			return -1;
		}

	} else {
		sm_debug("invalid resID: %d\n", resID);
		return -1;
	}

	sm_debug("resID: %d, bmp file is %s\n", resID, file);
	err_code = LoadBitmapFromFile(HDC_SCREEN, bmp, file);
	if(err_code != ERR_BMP_OK) {
		sm_debug("load %s bitmap failed\n", file);
	}
	return 0;
}

int getCfgFileHandle(void)
{
	if (resData->mCfgFileHandle != 0)
	{
		return 0;
	}

	resData->mCfgFileHandle = LoadEtcFile(resData->configFile);
	if(resData->mCfgFileHandle == 0)
	{
		sm_debug("getCfgFileHandle failed\n");
		return -1;
	}

	return 0;
}

int getCfgMenuHandle(void)
{
	if (resData->mCfgMenuHandle != 0) {
		return 0;
	}
	resData->mCfgMenuHandle = LoadEtcFile(resData->configMenu);
	if(resData->mCfgMenuHandle == 0) {
		sm_debug("getCfgMenuHandle failed\n");
		return -1;
	}

	return 0;
}

void releaseCfgFileHandle(void)
{
	if(resData->mCfgFileHandle == 0)
	{
		return;
	}

	UnloadEtcFile(resData->mCfgFileHandle);
	resData->mCfgFileHandle = 0;
}

void releaseCfgMenuHandle(void)
{
	if(resData->mCfgMenuHandle == 0) {
		return;
	}
	UnloadEtcFile(resData->mCfgMenuHandle);
	resData->mCfgMenuHandle = 0;
}

int getRectFromFileHandle(const char *pWindow, smRect *rect)
{
	int err_code;

	if(resData->mCfgFileHandle == 0) {
		sm_error("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "x", &rect->x)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "y", &rect->y)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "w", &rect->w)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "h", &rect->h)) != ETC_OK) {
		return err_code;
	}

	return ETC_OK;
}

int getABGRColorFromFileHandle(const char* pWindow, const char* subkey, gal_pixel *pixel)
{
	char buf[20] = {0};
	int err_code;
	unsigned long int hex;
	unsigned char r, g, b, a;

	if(pWindow == NULL || subkey == NULL)
		return ETC_INVALIDOBJ;

	if(resData->mCfgFileHandle == 0) {
		sm_debug("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(resData->mCfgFileHandle, pWindow, subkey, buf, sizeof(buf))) != ETC_OK) {
		return err_code;
	}

	hex = strtoul(buf, NULL, 16);

	a = (hex >> 24) & 0xff;
	b = (hex >> 16) & 0xff;
	g = (hex >> 8) & 0xff;
	r = hex & 0xff;

	//sm_debug("abgr:%x, %x, %x, %x\n", a, b, g, r);

	*pixel = RGBA2Pixel(HDC_SCREEN, r, g, b, a);

	//sm_debug("*pixel=%x\n\n", *pixel);

	return ETC_OK;
}

int getValueFromFileHandle(const char*mainkey, const char* subkey, char* value, unsigned int len)
{
	int err_code;
	if(mainkey == NULL || subkey == NULL)
	{
		sm_debug("NULL pointer\n");
		return ETC_INVALIDOBJ;
	}

	if(resData->mCfgFileHandle == 0) {
		sm_debug("cfgHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(resData->mCfgFileHandle, mainkey, subkey, value, len)) != ETC_OK) {
		sm_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
		return err_code;
	}

	return ETC_OK;
}

int getIntValueFromHandle(GHANDLE cfgHandle, const char* mainkey, const char* subkey)
{
	int retval = 0;
	int err_code;
	if(mainkey == NULL || subkey == NULL) {
		sm_debug("NULL pointer\n");
		return -1;
	}

	if(cfgHandle == 0) {
		sm_debug("cfgHandle not initialized\n");
		return -1;
	}

	if((err_code = GetIntValueFromEtc(cfgHandle, mainkey, subkey, &retval)) != ETC_OK) {
		sm_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
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
		sm_debug("invalid pWindow\n");
		return ETC_INVALIDOBJ;
	}

	current = getIntValueFromHandle(resData->mCfgFileHandle, pWindow, "current");
	if(current < 0) {
		sm_error("get current value from %s failed\n", pWindow);
		return ETC_KEYNOTFOUND;
	}
	currentIcon->current = current;
	sm_debug("current is %d\n", currentIcon->current);

	current = 0;
	do
	{
		int err_code;
		sprintf(buf, "%s%d", "icon", current);

		err_code = GetValueFromEtc(resData->mCfgFileHandle, pWindow, buf, bufIcon, sizeof(bufIcon));
		if(err_code != ETC_OK) {
			if(current == 0) {
				return err_code;
			} else {
				break;
			}
		}

		if(current >= ICON_TOTAL_CNT)
		{
			sm_error("current:%d\n", current);
			return -1;
		}

		currentIcon->iconName[current] = (char *)malloc(ICON_PATH_SIZE);
		if(currentIcon->iconName[current] == NULL)
		{
			sm_error("malloc icon Name error\n");
			return -1;
		}
		memcpy((void *)currentIcon->iconName[current], (void *)bufIcon, sizeof(bufIcon));
		current++;
	}while(1);

	if(currentIcon->current >=ICON_TOTAL_CNT)
	{
		sm_error("currentIcon->current:%d\n", currentIcon->current);
		return -1;
	}

	return ETC_OK;
}

int initStatusBarResource(void)
{
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = {
		{CFG_BAR_STAT,			(void*)&resData->rStatusBarData.STBRect },
		{CFG_BAR_STAT_DATE,		(void*)&resData->rStatusBarData.dateRect },
		{CFG_BAR_STAT_CUR_TIME,	(void*)&resData->rStatusBarData.curTimeRect },
		{CFG_BAR_STAT_LOCK,		(void*)&resData->rStatusBarData.lockRect },
		{CFG_BAR_STAT_SHAPE,	(void*)&resData->rStatusBarData.shapeRect },
		{CFG_BAR_STAT_PREOPEN,	(void*)&resData->rStatusBarData.preOpenRect },
		{CFG_BAR_STAT_BACK,		(void*)&resData->rStatusBarData.backRect },
		{CFG_BAR_STAT_TITLE,	(void*)&resData->rStatusBarData.titleRect },
		{CFG_BAR_STAT_MODE_ICON,	(void*)&resData->rStatusBarData.statItemsRect },
		{CFG_BAR_STAT_MODE_LABEL,	(void*)&resData->rStatusBarData.statItemsFontRect },
	};

	configTable3 configTableColor[] = {
		{CFG_BAR_STAT, 	FGC_WIDGET, (void*)&resData->rStatusBarData.fgc},
		{CFG_BAR_STAT, 	BGC_WIDGET, (void*)&resData->rStatusBarData.bgc},
		{CFG_BAR_STAT, 	FGC_HEAD_WIDGET, (void*)&resData->rStatusBarData.head_bar_fgc},
		{CFG_BAR_STAT, 	BGC_HEAD_WIDGET, (void*)&resData->rStatusBarData.head_bar_bgc},
	};

	configTable3 configTableIntValue[] = {
		{CFG_BAR_STAT_DATE,	"hBorder",		(void*)&resData->rStatusBarData.dateHBorder	},
		{CFG_BAR_STAT_DATE,	"yearW",			(void*)&resData->rStatusBarData.dateYearW  },
		{CFG_BAR_STAT_DATE,	"dateLabelW",	(void*)&resData->rStatusBarData.dateLabelW  },
		{CFG_BAR_STAT_DATE,	"numberW",		(void*)&resData->rStatusBarData.dateNumberW	},
		{CFG_BAR_STAT_DATE,	"boxH", 			(void*)&resData->rStatusBarData.dateBoxH  },
	};

	configTable2 configTableIcon[] = {
		{CFG_BAR_STAT_LOCK, 	(void*)&resData->rStatusBarData.lockIcon},
		{CFG_BAR_STAT_SHAPE, 	(void*)&resData->rStatusBarData.shapeIcon},
		{CFG_BAR_STAT_PREOPEN,	(void*)&resData->rStatusBarData.preOpenIcon},
		{CFG_BAR_STAT_BACK, 	(void*)&resData->rStatusBarData.backIcon},
		{CFG_BAR_STAT_TITLE, 	(void*)&resData->rStatusBarData.titleIcon},
		{CFG_BAR_STAT_MODE_ICON,	(void*)&resData->rStatusBarData.statItemsIcon},
		{CFG_BAR_STAT_MODE_LABEL,	(void*)&resData->rStatusBarData.statItemsFontIcon },

		{CFG_BAR_STAT_WIND_ICON,	(void*)&resData->rStatusBarData.statWindIcon},
		{CFG_BAR_STAT_WIND_LABEL,	(void*)&resData->rStatusBarData.statWindFontIcon },

		{CFG_BAR_STAT_DIRECT_ICON,	(void*)&resData->rStatusBarData.statDirectIcon},
		//{CFG_BAR_STAT_DIRECT_LABEL,	(void*)&resData->rStatusBarData.statDirectFontIcon },
		{CFG_BACKGROUND_BMP,	(void*)&resData->bgImg},
		{CFG_HEAD_BACKGROUND_BMP,	(void*)&resData->headbgImg},
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (smRect*)configTableRect[i].addr );
		if(err_code != ETC_OK) {
			sm_debug("get %s rect failed: %d\n", configTableRect[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			sm_debug("get %s %s failed: %d\n", configTableColor[i].mainkey, configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(resData->mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			sm_debug("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, (currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			sm_error("get %s icon failed: %d\n", configTableIcon[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
	}

	out:
	return retval;
}

int initMainPageResource(void)
{
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = {
		{CFG_MAIN_PAGE, (void*)&resData->rMainPgeData.pgeRect},
		{CFG_MAINBTN_LEFT, (void*)&resData->rMainPgeData.btnLeftRect},
		{CFG_MAINBTN_RIGHT, (void*)&resData->rMainPgeData.btnRightRect},
		{CFG_MAINBTN_ITEM1, (void*)&resData->rMainPgeData.btn_item[0]},
		{CFG_MAINBTN_ITEM2, (void*)&resData->rMainPgeData.btn_item[1]},
		{CFG_MAINBTN_ITEM3, (void*)&resData->rMainPgeData.btn_item[2]},
		{CFG_MAINBTN_ITEM4, (void*)&resData->rMainPgeData.btn_item[3]},

		{CFG_TEMPSET, (void*)&resData->rTempSetData.setRect},
		{CFG_TEMPSET_UP, (void*)&resData->rTempSetData.btnUpRect},
		{CFG_TEMPSET_DOWN, (void*)&resData->rTempSetData.btnDownRect},
		{CFG_TEMPSET_DEGREE, (void*)&resData->rTempSetData.labelDegreeRect},
	};

	configTable3 configTableColor[] = {
		{CFG_MAIN_PAGE, FGC_WIDGET, (void*)&resData->rMainPgeData.fgc},
		{CFG_MAIN_PAGE, BGC_WIDGET, (void*)&resData->rMainPgeData.bgc},

		{CFG_MAINBTN_ITEM1, FGC_WIDGET, (void*)&resData->rMainPgeData.itemfgc[0]},
		{CFG_MAINBTN_ITEM1, BGC_WIDGET, (void*)&resData->rMainPgeData.itembgc[0]},

		{CFG_MAINBTN_ITEM2, FGC_WIDGET, (void*)&resData->rMainPgeData.itemfgc[1]},
		{CFG_MAINBTN_ITEM2, BGC_WIDGET, (void*)&resData->rMainPgeData.itembgc[1]},

		{CFG_MAINBTN_ITEM3, FGC_WIDGET, (void*)&resData->rMainPgeData.itemfgc[2]},
		{CFG_MAINBTN_ITEM3, BGC_WIDGET, (void*)&resData->rMainPgeData.itembgc[2]},

		{CFG_MAINBTN_ITEM4, FGC_WIDGET, (void*)&resData->rMainPgeData.itemfgc[3]},
		{CFG_MAINBTN_ITEM4, BGC_WIDGET, (void*)&resData->rMainPgeData.itembgc[3]},

		{CFG_TEMPSET, FGC_WIDGET, (void*)&resData->rTempSetData.fgc},
		{CFG_TEMPSET, BGC_WIDGET, (void*)&resData->rTempSetData.bgc},
	};

	configTable2 configTableIcon[] = {
		{CFG_MAINBTN_LEFT,  (void*)&resData->rMainPgeData.Iconleft},
		{CFG_MAINBTN_RIGHT, (void*)&resData->rMainPgeData.Iconright},
		{CFG_MAINBTN_ITEM1, (void*)&resData->rMainPgeData.Icon_item[0]},
		{CFG_MAINBTN_ITEM2, (void*)&resData->rMainPgeData.Icon_item[1]},
		{CFG_MAINBTN_ITEM3,	(void*)&resData->rMainPgeData.Icon_item[2]},
		{CFG_MAINBTN_ITEM4, (void*)&resData->rMainPgeData.Icon_item[3]},

		{CFG_TEMPSET_UP, (void*)&resData->rTempSetData.Iconup},
		{CFG_TEMPSET_DOWN, (void*)&resData->rTempSetData.Icondown},
		{CFG_TEMPSET_DEGREE, (void*)&resData->rTempSetData.IconDegree},
	};

	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (smRect*)configTableRect[i].addr );
		if(err_code != ETC_OK) {
			sm_debug("get %s rect failed: %d\n", configTableRect[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, (gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			sm_debug("get %s %s failed: %d\n", configTableColor[i].mainkey, configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}

	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, (currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			sm_error("get %s icon failed: %d\n", configTableIcon[i].mainkey, err_code);
			retval = -1;
			goto out;
		}
	}

	out:
	return retval;
}

int initStage1(void)
{
	int retx;
	int err_code;
	int retval = 0;
	smRect rScreenRect;

	if(getCfgMenuHandle() < 0) {
		sm_warn("restore menu.cfg\n");
		if(copyFile(resData->defaultConfigMenu, resData->configMenu) < 0) {
			sm_error("copyFile config file failed\n");
		}
	} else {
		releaseCfgMenuHandle();
	}
	retx = LoadBitmapFromFile(HDC_SCREEN,(PBITMAP)&resData->bgImg,"/usr/res/menu/bg.png");
	if(retx!= ERR_BMP_OK){
		sm_error("load the background bitmap error");
	}
	retx = LoadBitmapFromFile(HDC_SCREEN,(PBITMAP)&resData->headbgImg,"/usr/res/menu/bg_head.png");
	if(retx!= ERR_BMP_OK){
		sm_error("load head background bitmap error");
	}

	if(getCfgFileHandle() < 0) {
		sm_error("get file handle failed\n");
		return -1;
	}

	err_code = getRectFromFileHandle("rect_screen", &resData->rScreenRect);
	if(err_code != ETC_OK) {
		sm_error("get screen rect failed: %d\n", err_code);
		retval = -1;
		goto out;
	}
	sm_debug("Screen Rect:(%d, %d)\n", resData->rScreenRect.w, resData->rScreenRect.h);
	err_code = getRectFromFileHandle("rect_head_bar", &resData->rheadbarRect);
	if(err_code != ETC_OK) {
		sm_error("get screen rect failed: %d\n", err_code);
		retval = -1;
		goto out;
	}
	sm_debug("Screen Rect:(%d, %d)\n", resData->rheadbarRect.w, resData->rheadbarRect.h);

	if(initStatusBarResource() < 0) {
		sm_error("init status bar resource failed\n");
		retval = -1;
		goto out;
	}

	if(initMainPageResource() < 0) {
		sm_error("init main page resource failed\n");
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

	sm_debug("initStage2\n");

	initLangAndFont();

	if(getCfgFileHandle() < 0) {
		sm_error("get file handle failed\n");
		return -1;
	}
	if(getCfgMenuHandle() < 0) {
		sm_error("get menu handle failed\n");
		retval = -1;
		goto out;
	}

out:
	releaseCfgFileHandle();
	releaseCfgMenuHandle();
	return retval;
}

void ResourceInit(void)
{
	char cfgStr[] = "/usr/res/config/system.cfg";
	char menucfg[] = "/usr/res/config/mainmenu.cfg";
	char defMenuStr[] = "/usr/res/config/system.cfg";

	resData = (ResDataType*)malloc(sizeof(ResDataType));
	if(NULL == resData)
	{
		sm_error("malloc resource error\n");
		return;
	}
	memset((void *)resData, 0, sizeof(ResDataType));
	memcpy((void *)resData->configFile, (void *)cfgStr, sizeof(cfgStr));
	memcpy((void *)resData->configMenu, (void *)menucfg, sizeof(menucfg));
	memcpy((void *)resData->defaultConfigMenu, (void *)defMenuStr, sizeof(defMenuStr));

	if(initStage1() < 0)
	{
		sm_debug("init stage1 error\n");
	}

	if(initStage2() < 0)
	{
		sm_debug("init stage2 error\n");
	}

	return;
}

void ResourceUninit(void)
{
	sm_debug("resource uninit\n");
	deinitFont();

	if(resData)
		free(resData);

	return;
}

#define ARRAY_LEN(array) \
    (sizeof(array)/sizeof(array[0]))

static char* all_pic[] = {
	TZN_MODE_BG,
  	TZN_WIND_BG,
  	TZN_WIND_DIRECT_BG,
  	TZN_STOP_BG,
  	TZN_RETURN_HEAD
};

BOOL register_all_pic (void)
{
    int i;
    SetResPath ("/usr/res/menu/");

    for (i = 0; i < ARRAY_LEN(all_pic); i++)
    {
        if (RegisterResFromFile (HDC_SCREEN, all_pic[i]) == FALSE)
        {
            fprintf (stderr, "can't register %s\n", all_pic[i]);
            return FALSE;
        }
    }
    return TRUE;
}

void unregister_all_pic (void)
{
    int i;

    for (i = 0; i < ARRAY_LEN(all_pic); i++)
    {
        UnregisterRes(all_pic[i]);
    }
}
#else
BOOL register_all_pic (void)
{
    return FALSE;
}

void unregister_all_pic (void)
{
}
#endif
