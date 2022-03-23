#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "resource.h"
#include "common.h"

ResDataType *resData = NULL;
void setHwnd(unsigned int win_id, HWND hwnd) {
	resData->mHwnd[win_id] = hwnd;
}

int initLangAndFont(void) {
	resData->mFont_SimSun20 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 20, 0);

	resData->mFont_SimSun25 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 25, 0);

	resData->mFont_SimSun30 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 30, 0);

	resData->mFont_SimSun40 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 40, 0);

	resData->mFont_SimSun45 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 45, 0);

	resData->mFont_SimSun50 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 50, 0);

	resData->mFont_SimSun55 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 55, 0);

	resData->mFont_SimSun120 = CreateLogFont("ttf", "fzcircle", "UTF-8",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 120, 0);

	resData->mCurLogFont = resData->mFont_SimSun20;

	return 0;
}

int deinitFont(void) {
	DestroyLogFont(resData->mFont_SimSun20);
	DestroyLogFont(resData->mFont_SimSun25);
	DestroyLogFont(resData->mFont_SimSun30);
	DestroyLogFont(resData->mFont_SimSun40);
	DestroyLogFont(resData->mFont_SimSun45);
	DestroyLogFont(resData->mFont_SimSun50);
	DestroyLogFont(resData->mFont_SimSun55);
	DestroyLogFont(resData->mFont_SimSun120);
	return 0;
}

void unloadBitMap(BITMAP *bmp) {
	if (bmp->bmBits != NULL) {
		UnloadBitmap(bmp);
		bmp->bmBits = NULL;
	}
}

int copyFile(const char* from, const char* to) {
	sm_debug("copy file");

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

	int from_fd, to_fd;
	int bytes_read, bytes_write;
	char buffer[BUFFER_SIZE + 1];
	char *ptr;
	int retval = 0;

	if (access(from, F_OK | R_OK) != 0) {
		sm_error("access error\n");
		return -1;
	}

	if ((from_fd = open(from, O_RDONLY)) == -1) {
		sm_error("Open %s\n", from);
		return -1;
	}

	if ((to_fd = open(to, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
		sm_error("Open %s Error\n", to);
		close(from_fd);
		return -1;
	}

	while ((bytes_read = read(from_fd, buffer, BUFFER_SIZE))) {
		if ((bytes_read == -1)) {
			sm_error("read error\n");
			retval = -1;
			break;
		}

		else if (bytes_read > 0) {
			ptr = buffer;
			while ((bytes_write = write(to_fd, ptr, bytes_read))) {
				if ((bytes_write == -1)) {
					sm_error("wirte error\n");
					retval = -1;
					break;
				} else if (bytes_write == bytes_read) {
					break;
				} else if (bytes_write > 0) {
					ptr += bytes_write;
					bytes_read -= bytes_write;
				}
			}
			if (bytes_write == -1) {
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

PLOGFONT getLogFont(enum FontID id) {
	switch (id) {
	case ID_FONT_SIMSUN_20:
		resData->mCurLogFont = resData->mFont_SimSun20;
		break;
	case ID_FONT_SIMSUN_25:
		resData->mCurLogFont = resData->mFont_SimSun25;
		break;
	case ID_FONT_SIMSUN_30:
		resData->mCurLogFont = resData->mFont_SimSun30;
		break;
	case ID_FONT_SIMSUN_40:
		resData->mCurLogFont = resData->mFont_SimSun40;
		break;
	case ID_FONT_SIMSUN_45:
		resData->mCurLogFont = resData->mFont_SimSun45;
		break;
	case ID_FONT_SIMSUN_50:
		resData->mCurLogFont = resData->mFont_SimSun50;
		break;
	case ID_FONT_SIMSUN_55:
		resData->mCurLogFont = resData->mFont_SimSun55;
		break;
	case ID_FONT_SIMSUN_120:
		resData->mCurLogFont = resData->mFont_SimSun120;
		break;
	default:
		sm_error("invalid Font ID index: %d\n", id);
		break;
	}
	return resData->mCurLogFont;
}

gal_pixel getStatusBarColor(enum ResourceID resID, enum ColorType type) {
	gal_pixel color = 0;
	if (resID >= ID_FAST_WIN) {
		switch (type) {
		case COLOR_FGC:
			color = resData->rFastMenuData.fgc;
			break;
		case COLOR_BGC:
			color = resData->rFastMenuData.bgc;
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

gal_pixel getResColor(enum ResourceID resID, enum ColorType type) {
	gal_pixel color = 0;

	switch (resID) {
	case ID_FAST_WIN:
		color = getStatusBarColor(resID, type);
		break;
	default:
		sm_debug("invalid resIndex for the color\n");
		break;
	}

	return color;
}

int getResRect(enum ResourceID resID, smRect *rect) {
	memset(rect, 0, sizeof(smRect));
	switch (resID) {
	case ID_SCREEN:
		*rect = resData->rScreenRect;
		break;
	case ID_FAST_WIN:
		*rect = resData->rFastMenuData.winRect;
		break;
	case ID_FAST_WIN_FAVOR:
		*rect = resData->rFastMenuData.favorRect;
		break;
	case ID_FAST_WIN_HOME:
		*rect = resData->rFastMenuData.homeRect;
		break;
	case ID_FAST_WIN_SETUP:
		*rect = resData->rFastMenuData.setupRect;
		break;

	case ID_ACT_WIN:
		*rect = resData->rActMenuData.winRect;
		break;
	case ID_ACT_SUB1_WIN:
		*rect = resData->rActMenuData.winSub1Rect;
		break;
	case ID_ACT_SUB2_WIN:
		*rect = resData->rActMenuData.winSub2Rect;
		break;
	case ID_ACT_SUB1_WIN_ITEM0:
		*rect = resData->rActMenuData.btnRect[0];
		break;
	case ID_ACT_SUB1_WIN_ITEM1:
		*rect = resData->rActMenuData.btnRect[1];
		break;
	case ID_ACT_SUB1_WIN_ITEM2:
		*rect = resData->rActMenuData.btnRect[2];
		break;
	case ID_ACT_SUB2_WIN_ITEM0:
		*rect = resData->rActMenuData.btnRect[3];
		break;
	case ID_ACT_SUB2_WIN_ITEM1:
		*rect = resData->rActMenuData.btnRect[4];
		break;
	case ID_ACT_SUB2_WIN_ITEM2:
		*rect = resData->rActMenuData.btnRect[5];
		break;

	case ID_POP_WIN:
		*rect = resData->rPopWinData.winRect;
		break;
	case ID_POP_WIN_BTN1:
		*rect = resData->rPopWinData.btnRect[0];
		break;
	case ID_POP_WIN_BTN2:
		*rect = resData->rPopWinData.btnRect[1];
		break;
	case ID_POP_WIN_BTN3:
		*rect = resData->rPopWinData.btnRect[2];
		break;
	case ID_POP_WIN_BTN4:
		*rect = resData->rPopWinData.btnRect[3];
		break;
	case ID_POP_WIN_BTN5:
		*rect = resData->rPopWinData.btnRect[4];
		break;
	case ID_POP_WIN_BTN6:
		*rect = resData->rPopWinData.btnRect[5];
		break;
	case ID_POP_WIN_BTN7:
		*rect = resData->rPopWinData.btnRect[6];
		break;
	case ID_POP_WIN_BTN8:
		*rect = resData->rPopWinData.btnRect[7];
		break;
	case ID_POP_WIN_BTN9:
		*rect = resData->rPopWinData.btnRect[8];
		break;
	case ID_POP_WIN_BTN10:
		*rect = resData->rPopWinData.btnRect[9];
		break;
	case ID_POP_WIN_BTN11:
		*rect = resData->rPopWinData.btnRect[10];
		break;
	case ID_POP_WIN_BTN12:
		*rect = resData->rPopWinData.btnRect[11];
		break;
	case ID_FETURES1:
		*rect = resData->rPopWinData.winRect;
		break;
	case ID_FETURES1_BTN1:
		*rect = resData->rPopWinData.btnRect[0];
		break;
	case ID_FETURES1_BTN2:
		*rect = resData->rPopWinData.btnRect[1];
		break;
	case ID_FETURES1_BTN3:
		*rect = resData->rPopWinData.btnRect[2];
		break;
	case ID_FETURES1_BTN4:
		*rect = resData->rPopWinData.btnRect[3];
		break;
	case ID_BM_STAT_WIN_PAGE_INDEX:
		*rect = resData->pageIdxRect;
		break;
	default:
		sm_error("invalid resID index: %d\n", resID);
		return -1;
	}

	return 0;
}

void setCurrentIconValue(enum ResourceID resID, int cur_val) {
	switch (resID) {
	case ID_FAST_WIN_FAVOR:
		resData->rFastMenuData.favorIcon.current = cur_val;
		break;
	case ID_FAST_WIN_HOME:
		resData->rFastMenuData.homeIcon.current = cur_val;
		break;
	case ID_FAST_WIN_SETUP:
		resData->rFastMenuData.setupIcon.current = cur_val;
		break;
	case ID_FAST_WIN_WIFI:
		resData->rFastMenuData.wifiIcon.current = cur_val;
		break;
	case ID_ACT_SUB1_WIN_ITEM0:
		resData->rActMenuData.btnIcon[0].current = cur_val;
		break;
	case ID_ACT_SUB1_WIN_ITEM1:
		resData->rActMenuData.btnIcon[1].current = cur_val;
		break;
	case ID_ACT_SUB1_WIN_ITEM2:
		resData->rActMenuData.btnIcon[2].current = cur_val;
		break;
	case ID_ACT_SUB2_WIN_ITEM0:
		resData->rActMenuData.btnIcon[3].current = cur_val;
		break;
	case ID_ACT_SUB2_WIN_ITEM1:
		resData->rActMenuData.btnIcon[4].current = cur_val;
		break;
	case ID_ACT_SUB2_WIN_ITEM2:
		resData->rActMenuData.btnIcon[5].current = cur_val;
		break;
	case ID_BM_STAT_WIN_PAGE_INDEX:
		resData->rActMenuData.indexIcon.current = cur_val;
		break;
	case ID_POP_WIN_BTN1:
		resData->rPopWinData.btnIcon[0].current = cur_val;
		break;
	case ID_POP_WIN_BTN2:
		resData->rPopWinData.btnIcon[1].current = cur_val;
		break;
	case ID_POP_WIN_BTN3:
		resData->rPopWinData.btnIcon[2].current = cur_val;
		break;
	case ID_POP_WIN_BTN4:
		resData->rPopWinData.btnIcon[3].current = cur_val;
		break;
	case ID_POP_WIN_BTN5:
		resData->rPopWinData.btnIcon[4].current = cur_val;
		break;
	case ID_POP_WIN_BTN6:
		resData->rPopWinData.btnIcon[5].current = cur_val;
		break;
	case ID_POP_WIN_BTN7:
		resData->rPopWinData.btnIcon[6].current = cur_val;
		break;
	case ID_POP_WIN_BTN8:
		resData->rPopWinData.btnIcon[7].current = cur_val;
		break;
	case ID_POP_WIN_BTN9:
		resData->rPopWinData.btnIcon[8].current = cur_val;
		break;
	case ID_POP_WIN_BTN10:
		resData->rPopWinData.btnIcon[9].current = cur_val;
		break;
	case ID_POP_WIN_BTN11:
		resData->rPopWinData.btnIcon[10].current = cur_val;
		break;
	case ID_POP_WIN_BTN12:
		resData->rPopWinData.btnIcon[11].current = cur_val;
		break;
	case ID_FETURES1_BTN1:
		resData->rPopWinData.btnIcon1[0].current = cur_val;
		break;
	case ID_FETURES1_BTN2:
		resData->rPopWinData.btnIcon1[1].current = cur_val;
		break;
	case ID_FETURES1_BTN3:
		resData->rPopWinData.btnIcon1[2].current = cur_val;
		break;
	case ID_FETURES1_BTN4:
		resData->rPopWinData.btnIcon1[3].current = cur_val;
		break;
	case ID_FETURES1_BTN5:
		resData->rPopWinData.btnIcon1[4].current = cur_val;
		break;
	case ID_FETURES1_BTN6:
		resData->rPopWinData.btnIcon1[5].current = cur_val;
		break;
	case ID_FETURES1_BTN7:
		resData->rPopWinData.btnIcon1[6].current = cur_val;
		break;
	case ID_FETURES1_BTN8:
		resData->rPopWinData.btnIcon1[7].current = cur_val;
		break;
	case ID_FUN_SCREEN_BG1:
		resData->rFunWinData.bgFunImg[0].current = cur_val;
		break;
	case ID_FUN_SCREEN_BG2:
		resData->rFunWinData.bgFunImg[1].current = cur_val;
		break;
	case ID_FUN_SCREEN_BG3:
		resData->rFunWinData.bgFunImg[2].current = cur_val;
		break;
	case ID_FUN_WIN_BTN1:
		resData->rFunWinData.btnIcon[0].current = cur_val;
		break;
	case ID_FUN_WIN_BTN2:
		resData->rFunWinData.btnIcon[1].current = cur_val;
		break;
	case ID_FUN_WIN_BTN3:
		resData->rFunWinData.btnIcon[2].current = cur_val;
		break;
	case ID_FUN_WIN_BTN4:
		resData->rFunWinData.btnIcon[3].current = cur_val;
		break;
	case ID_FUN_WIN_ANIM:
		resData->rFunWinData.animIcon.current = cur_val;
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

int getCurrentIconFileName(enum ResourceID resID, char *file) {
	int current;

	switch (resID) {
	case ID_SCREEN_BG:
		CHECK_CURRENT_ICON_VALID(resData->bgImg);
		memcpy((void *) file, (void *) resData->bgImg.iconName[0],
		ICON_PATH_SIZE);
		break;
	case ID_FUN_SCREEN_BG1:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.bgFunImg[0]);
		current = resData->rFunWinData.bgFunImg[0].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.bgFunImg[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_SCREEN_BG2:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.bgFunImg[1]);
		current = resData->rFunWinData.bgFunImg[1].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.bgFunImg[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_SCREEN_BG3:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.bgFunImg[2]);
		current = resData->rFunWinData.bgFunImg[2].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.bgFunImg[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FAST_WIN_BG:
		CHECK_CURRENT_ICON_VALID(resData->headbgImg);
		memcpy((void *) file, (void *) resData->headbgImg.iconName[0],
		ICON_PATH_SIZE);
		break;
	case ID_FAST_WIN_FAVOR:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.favorIcon);
		current = resData->rFastMenuData.favorIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.favorIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FAST_WIN_HOME:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.homeIcon);
		current = resData->rFastMenuData.homeIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.homeIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FAST_WIN_SETUP:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.setupIcon);
		current = resData->rFastMenuData.setupIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.setupIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FAST_WIN_WIFI:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.wifiIcon);
		current = resData->rFastMenuData.wifiIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.wifiIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB1_WIN_ITEM0:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[0]);
		current = resData->rActMenuData.btnIcon[0].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB1_WIN_ITEM1:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[1]);
		current = resData->rActMenuData.btnIcon[1].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB1_WIN_ITEM2:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[2]);
		current = resData->rActMenuData.btnIcon[2].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB2_WIN_ITEM0:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[3]);
		current = resData->rActMenuData.btnIcon[3].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB2_WIN_ITEM1:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[4]);
		current = resData->rActMenuData.btnIcon[4].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[4].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_ACT_SUB2_WIN_ITEM2:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.btnIcon[5]);
		current = resData->rActMenuData.btnIcon[5].current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.btnIcon[5].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BM_STAT_WIN_PAGE_INDEX:
		CHECK_CURRENT_ICON_VALID(resData->rActMenuData.indexIcon);
		current = resData->rActMenuData.indexIcon.current;
		memcpy((void *) file,
				(void *) resData->rActMenuData.indexIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN1:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[0]);
		current = resData->rPopWinData.btnIcon[0].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN2:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[1]);
		current = resData->rPopWinData.btnIcon[1].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN3:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[2]);
		current = resData->rPopWinData.btnIcon[2].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN4:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[3]);
		current = resData->rPopWinData.btnIcon[3].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN5:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[4]);
		current = resData->rPopWinData.btnIcon[4].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[4].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN6:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[5]);
		current = resData->rPopWinData.btnIcon[5].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[5].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN7:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[6]);
		current = resData->rPopWinData.btnIcon[6].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[6].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN8:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[7]);
		current = resData->rPopWinData.btnIcon[7].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[7].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN9:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[8]);
		current = resData->rPopWinData.btnIcon[8].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[8].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN10:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[9]);
		current = resData->rPopWinData.btnIcon[9].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[9].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN11:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[10]);
		current = resData->rPopWinData.btnIcon[10].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[10].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_POP_WIN_BTN12:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon[11]);
		current = resData->rPopWinData.btnIcon[11].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon[11].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN1:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[0]);
		current = resData->rPopWinData.btnIcon1[0].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[0].iconName[current],
				ICON_PATH_SIZE);
		break;

	case ID_FETURES1_BTN2:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[1]);
		current = resData->rPopWinData.btnIcon1[1].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN3:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[2]);
		current = resData->rPopWinData.btnIcon1[2].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN4:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[3]);
		current = resData->rPopWinData.btnIcon1[3].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN5:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[4]);
		current = resData->rPopWinData.btnIcon1[4].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[4].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN6:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[5]);
		current = resData->rPopWinData.btnIcon1[5].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[5].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN7:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[6]);
		current = resData->rPopWinData.btnIcon1[6].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[6].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FETURES1_BTN8:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon1[7]);
		current = resData->rPopWinData.btnIcon1[7].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon1[7].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_WIN_BTN1:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.btnIcon[0]);
		current = resData->rFunWinData.btnIcon[0].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.btnIcon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_WIN_BTN2:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.btnIcon[1]);
		current = resData->rFunWinData.btnIcon[1].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.btnIcon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_WIN_BTN3:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.btnIcon[2]);
		current = resData->rFunWinData.btnIcon[2].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.btnIcon[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_WIN_BTN4:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.btnIcon[3]);
		current = resData->rFunWinData.btnIcon[3].current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.btnIcon[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_WIN_ANIM:
		CHECK_CURRENT_ICON_VALID(resData->rFunWinData.animIcon);
		current = resData->rFunWinData.animIcon.current;
		memcpy((void *) file,
				(void *) resData->rFunWinData.animIcon.iconName[current],
				ICON_PATH_SIZE);
		break;

	default:
		sm_debug("invalid resID %d\n", resID);
		return -1;
	}
	return 0;
}

int getWindPicFileName(enum BmpType type, char *file) {
	switch (type) {
	case BMPTYPE_SELECTED:
		break;
	default:
		sm_error("invalid BmpType %d\n", type);
		return -1;
	}

	return 0;
}

int getResBmp(enum ResourceID resID, enum BmpType type, BITMAP *bmp) {
	int err_code;
	char file[ICON_PATH_SIZE];
	if (resID == ID_FAST_WIN) {
		if (getWindPicFileName(type, file) < 0) {
			sm_error("get window pic bmp failed\n");
			return -1;
		}
	} else if (resID >= ID_SCREEN_BG && resID <= ID_BM_STAT_WIN_PAGE_INDEX) {
		if (getCurrentIconFileName(resID, file) < 0) {
			sm_error("get current icon pic bmp failed\n");
			return -1;
		}

	} else {
		sm_debug("invalid resID: %d\n", resID);
		return -1;
	}

	sm_debug("resID: %d, bmp file is %s\n", resID, file);
	err_code = LoadBitmapFromFile(HDC_SCREEN, bmp, file);
	if (err_code != ERR_BMP_OK) {
		sm_debug("load %s bitmap failed\n", file);
	}
	return 0;
}

int getCfgFileHandle(void) {
	if (resData->mCfgFileHandle != 0) {
		return 0;
	}

	resData->mCfgFileHandle = LoadEtcFile(resData->configFile);
	if (resData->mCfgFileHandle == 0) {
		sm_debug("getCfgFileHandle failed\n");
		return -1;
	}

	return 0;
}

void releaseCfgFileHandle(void) {
	if (resData->mCfgFileHandle == 0) {
		return;
	}

	UnloadEtcFile(resData->mCfgFileHandle);
	resData->mCfgFileHandle = 0;
}

int getRectFromFileHandle(const char *pWindow, smRect *rect) {
	int err_code;

	if (resData->mCfgFileHandle == 0) {
		sm_error("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if ((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "x",
			&rect->x)) != ETC_OK) {
		return err_code;
	}

	if ((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "y",
			&rect->y)) != ETC_OK) {
		return err_code;
	}

	if ((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "w",
			&rect->w)) != ETC_OK) {
		return err_code;
	}

	if ((err_code = GetIntValueFromEtc(resData->mCfgFileHandle, pWindow, "h",
			&rect->h)) != ETC_OK) {
		return err_code;
	}

	return ETC_OK;
}

int getABGRColorFromFileHandle(const char* pWindow, const char* subkey,
		gal_pixel *pixel) {
	char buf[20] = { 0 };
	int err_code;
	unsigned long int hex;
	unsigned char r, g, b, a;

	if (pWindow == NULL || subkey == NULL)
		return ETC_INVALIDOBJ;

	if (resData->mCfgFileHandle == 0) {
		sm_debug("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if ((err_code = GetValueFromEtc(resData->mCfgFileHandle, pWindow, subkey,
			buf, sizeof(buf))) != ETC_OK) {
		return err_code;
	}

	hex = strtoul(buf, NULL, 16);

	a = (hex >> 24) & 0xff;
	b = (hex >> 16) & 0xff;
	g = (hex >> 8) & 0xff;
	r = hex & 0xff;

	*pixel = RGBA2Pixel(HDC_SCREEN, r, g, b, a);

	return ETC_OK;
}

int getValueFromFileHandle(const char*mainkey, const char* subkey, char* value,
		unsigned int len) {
	int err_code;
	if (mainkey == NULL || subkey == NULL) {
		sm_debug("NULL pointer\n");
		return ETC_INVALIDOBJ;
	}

	if (resData->mCfgFileHandle == 0) {
		sm_debug("cfgHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if ((err_code = GetValueFromEtc(resData->mCfgFileHandle, mainkey, subkey,
			value, len)) != ETC_OK) {
		sm_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
		return err_code;
	}

	return ETC_OK;
}

int getIntValueFromHandle(GHANDLE cfgHandle, const char* mainkey,
		const char* subkey) {
	int retval = 0;
	int err_code;
	if (mainkey == NULL || subkey == NULL) {
		sm_debug("NULL pointer\n");
		return -1;
	}

	if (cfgHandle == 0) {
		sm_debug("cfgHandle not initialized\n");
		return -1;
	}

	if ((err_code = GetIntValueFromEtc(cfgHandle, mainkey, subkey, &retval))
			!= ETC_OK) {
		sm_debug("get %s->%s failed:%d\n", mainkey, subkey, err_code);
		return -1;
	}

	return retval;
}

int fillCurrentIconInfoFromFileHandle(const char* pWindow,
		currentIcon_t *currentIcon) {
	int current;
	char buf[10] = { 0 };
	char bufIcon[ICON_PATH_SIZE] = { 0 };

	if (pWindow == NULL) {
		sm_debug("invalid pWindow\n");
		return ETC_INVALIDOBJ;
	}

	current = getIntValueFromHandle(resData->mCfgFileHandle, pWindow,
			"current");
	if (current < 0) {
		sm_error("get current value from %s failed\n", pWindow);
		return ETC_KEYNOTFOUND;
	}
	currentIcon->current = current;
	/*sm_debug("current is %d\n", currentIcon->current);*/

	current = 0;
	do {
		int err_code;
		sprintf(buf, "%s%d", "icon", current);

		err_code = GetValueFromEtc(resData->mCfgFileHandle, pWindow, buf,
				bufIcon, sizeof(bufIcon));
		if (err_code != ETC_OK) {
			if (current == 0) {
				return err_code;
			} else {
				break;
			}
		}

		if (current >= ICON_TOTAL_CNT) {
			sm_error("current:%d\n", current);
			return -1;
		}

		currentIcon->iconName[current] = (char *) malloc(ICON_PATH_SIZE);
		if (currentIcon->iconName[current] == NULL) {
			sm_error("malloc icon Name error\n");
			return -1;
		}
		memcpy((void *) currentIcon->iconName[current], (void *) bufIcon,
				sizeof(bufIcon));
		current++;
	} while (1);

	if (currentIcon->current >= ICON_TOTAL_CNT) {
		sm_error("currentIcon->current:%d\n", (int)currentIcon->current);
		return -1;
	}

	return ETC_OK;
}

int initFastMenuResource(void) {
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = { { CFG_FAST_WIN_AREA,
			(void*) &resData->rFastMenuData.winRect }, { CFG_FAST_WIN_FAVOR,
			(void*) &resData->rFastMenuData.favorRect }, { CFG_FAST_WIN_HOME,
			(void*) &resData->rFastMenuData.homeRect }, { CFG_FAST_WIN_SETUP,
			(void*) &resData->rFastMenuData.setupRect }, };

	configTable3 configTableColor[] = { { CFG_FAST_WIN_AREA, FGC_WIDGET,
			(void*) &resData->rFastMenuData.fgc }, { CFG_FAST_WIN_AREA,
	BGC_WIDGET, (void*) &resData->rFastMenuData.bgc }, {
	CFG_FAST_WIN_AREA, FGC_HEAD_WIDGET,
			(void*) &resData->rFastMenuData.head_bar_fgc }, { CFG_FAST_WIN_AREA,
	BGC_HEAD_WIDGET, (void*) &resData->rFastMenuData.head_bar_bgc }, };

	/*configTable3 configTableIntValue[] = {
	 {CFG_BAR_STAT_DATE,	"hBorder",	 (void*)&resData->rFastMenuData.dateHBorder},
	 {CFG_BAR_STAT_DATE,	"yearW",  (void*)&resData->rFastMenuData.dateYearW  },
	 {CFG_BAR_STAT_DATE,	"dateLabelW", (void*)&resData->rFastMenuData.dateLabelW},
	 {CFG_BAR_STAT_DATE,	"numberW", (void*)&resData->rFastMenuData.dateNumberW},
	 {CFG_BAR_STAT_DATE,	"boxH", (void*)&resData->rFastMenuData.dateBoxH  },
	 };*/

	configTable2 configTableIcon[] = { { CFG_FAST_WIN_FAVOR,
			(void*) &resData->rFastMenuData.favorIcon }, { CFG_FAST_WIN_HOME,
			(void*) &resData->rFastMenuData.homeIcon }, {
	CFG_FAST_WIN_SETUP, (void*) &resData->rFastMenuData.setupIcon }, {
	CFG_FAST_WIN_WIFI, (void*) &resData->rFastMenuData.wifiIcon },

	{ CFG_FUN_WIN_BUTTON1, (void*) &resData->rFunWinData.btnIcon[0] }, {
	CFG_FUN_WIN_BUTTON2, (void*) &resData->rFunWinData.btnIcon[1] }, {
	CFG_FUN_WIN_BUTTON3, (void*) &resData->rFunWinData.btnIcon[2] }, {
	CFG_FUN_WIN_BUTTON4, (void*) &resData->rFunWinData.btnIcon[3] }, {
	CFG_FUN_WIN_ANIM, (void*) &resData->rFunWinData.animIcon },

	{ CFG_BACKGROUND_BMP, (void*) &resData->bgImg }, { CFG_HEAD_BACKGROUND_BMP,
			(void*) &resData->headbgImg }, {
	CFG_FUN_BACKGROUND_BMP1, (void*) &resData->rFunWinData.bgFunImg[0] }, {
	CFG_FUN_BACKGROUND_BMP2, (void*) &resData->rFunWinData.bgFunImg[1] }, {
	CFG_FUN_BACKGROUND_BMP3, (void*) &resData->rFunWinData.bgFunImg[2] }, };

	unsigned int i;
	for (i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey,
				(smRect*) configTableRect[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s rect failed: %d\n", configTableRect[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}

	for (i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey,
				configTableColor[i].subkey,
				(gal_pixel*) configTableColor[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s %s failed: %d\n", configTableColor[i].mainkey,
					configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}

	/*for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
	 err_code = getIntValueFromHandle(resData->mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
	 if(err_code < 0) {
	 sm_debug("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
	 retval = -1;
	 goto out;
	 }
	 *(unsigned int*)configTableIntValue[i].addr = err_code;
	 }*/

	for (i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey,
				(currentIcon_t*) configTableIcon[i].addr);
		if (err_code != ETC_OK) {
			sm_error("get %s icon failed: %d\n", configTableIcon[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}

	out: return retval;
}

int initPopMenuResource(void) {
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];
	configTable2 configTableRect[] = { { CFG_POP_MENU_AREA,
			(void*) &resData->rPopWinData.winRect }, {
	CFG_POP_MENU_BUTTON1, (void*) &resData->rPopWinData.btnRect[0] }, {
	CFG_POP_MENU_BUTTON2, (void*) &resData->rPopWinData.btnRect[1] }, {
	CFG_POP_MENU_BUTTON3, (void*) &resData->rPopWinData.btnRect[2] }, {
	CFG_POP_MENU_BUTTON4, (void*) &resData->rPopWinData.btnRect[3] }, {
	CFG_POP_MENU_BUTTON5, (void*) &resData->rPopWinData.btnRect[4] }, {
	CFG_POP_MENU_BUTTON6, (void*) &resData->rPopWinData.btnRect[5] }, {
	CFG_POP_MENU_BUTTON7, (void*) &resData->rPopWinData.btnRect[6] }, {
	CFG_POP_MENU_BUTTON8, (void*) &resData->rPopWinData.btnRect[7] }, {
	CFG_POP_MENU_BUTTON9, (void*) &resData->rPopWinData.btnRect[8] }, {
	CFG_POP_MENU_BUTTON10, (void*) &resData->rPopWinData.btnRect[9] }, {
	CFG_POP_MENU_BUTTON11, (void*) &resData->rPopWinData.btnRect[10] }, {
	CFG_POP_MENU_BUTTON12, (void*) &resData->rPopWinData.btnRect[11] }, };
	configTable3 configTableColor[] = { { CFG_POP_MENU_AREA, FGC_WIDGET,
			(void*) &resData->rPopWinData.fgc }, { CFG_POP_MENU_AREA,
	BGC_WIDGET, (void*) &resData->rPopWinData.bgc }, };
	configTable2 configTableIcon[] = { { CFG_POP_MENU_BUTTON1,
			(void*) &resData->rPopWinData.btnIcon[0] }, {
	CFG_POP_MENU_BUTTON2, (void*) &resData->rPopWinData.btnIcon[1] }, {
	CFG_POP_MENU_BUTTON3, (void*) &resData->rPopWinData.btnIcon[2] }, {
	CFG_POP_MENU_BUTTON4, (void*) &resData->rPopWinData.btnIcon[3] }, {
	CFG_POP_MENU_BUTTON5, (void*) &resData->rPopWinData.btnIcon[4] }, {
	CFG_POP_MENU_BUTTON6, (void*) &resData->rPopWinData.btnIcon[5] }, {
	CFG_POP_MENU_BUTTON7, (void*) &resData->rPopWinData.btnIcon[6] }, {
	CFG_POP_MENU_BUTTON8, (void*) &resData->rPopWinData.btnIcon[7] }, {
	CFG_POP_MENU_BUTTON9, (void*) &resData->rPopWinData.btnIcon[8] }, {
	CFG_POP_MENU_BUTTON10, (void*) &resData->rPopWinData.btnIcon[9] }, {
	CFG_POP_MENU_BUTTON11, (void*) &resData->rPopWinData.btnIcon[10] }, {
	CFG_POP_MENU_BUTTON12, (void*) &resData->rPopWinData.btnIcon[11] }, {
	CFG_FET_MENU1_BUTTON1, (void*) &resData->rPopWinData.btnIcon1[0] }, {
	CFG_FET_MENU1_BUTTON2, (void*) &resData->rPopWinData.btnIcon1[1] }, {
	CFG_FET_MENU1_BUTTON3, (void*) &resData->rPopWinData.btnIcon1[2] }, {
	CFG_FET_MENU1_BUTTON4, (void*) &resData->rPopWinData.btnIcon1[3] }, {
	CFG_FET_MENU1_BUTTON5, (void*) &resData->rPopWinData.btnIcon1[4] }, {
	CFG_FET_MENU1_BUTTON6, (void*) &resData->rPopWinData.btnIcon1[5] }, {
	CFG_FET_MENU1_BUTTON7, (void*) &resData->rPopWinData.btnIcon1[6] }, {
	CFG_FET_MENU1_BUTTON8, (void*) &resData->rPopWinData.btnIcon1[7] }, };

	unsigned int i;
	for (i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey,
				(smRect*) configTableRect[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s rect failed: %d\n", configTableRect[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}
	for (i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey,
				configTableColor[i].subkey,
				(gal_pixel*) configTableColor[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s %s failed: %d\n", configTableColor[i].mainkey,
					configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}
	for (i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey,
				(currentIcon_t*) configTableIcon[i].addr);
		if (err_code != ETC_OK) {
			sm_error("get %s icon failed: %d\n", configTableIcon[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}
	out: return retval;
}
int initActMenuResource(void) {
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = { { CFG_ACT_WIN_AREA,
			(void*) &resData->rActMenuData.winRect }, {
	CFG_ACT_SUB1_WIN_AREA, (void*) &resData->rActMenuData.winSub1Rect }, {
	CFG_ACT_SUB2_WIN_AREA, (void*) &resData->rActMenuData.winSub2Rect }, {
	CFG_ACT_SUB1_WIN_ITEM0, (void*) &resData->rActMenuData.btnRect[0] }, {
	CFG_ACT_SUB1_WIN_ITEM1, (void*) &resData->rActMenuData.btnRect[1] }, {
	CFG_ACT_SUB1_WIN_ITEM2, (void*) &resData->rActMenuData.btnRect[2] }, {
	CFG_ACT_SUB2_WIN_ITEM0, (void*) &resData->rActMenuData.btnRect[3] }, {
	CFG_ACT_SUB2_WIN_ITEM1, (void*) &resData->rActMenuData.btnRect[4] }, {
	CFG_ACT_SUB2_WIN_ITEM2, (void*) &resData->rActMenuData.btnRect[5] }, };

	configTable3 configTableColor[] = { { CFG_ACT_WIN_AREA, FGC_WIDGET,
			(void*) &resData->rFastMenuData.fgc }, { CFG_ACT_WIN_AREA,
	BGC_WIDGET, (void*) &resData->rFastMenuData.bgc }, {
	CFG_ACT_WIN_AREA, FGC_HEAD_WIDGET,
			(void*) &resData->rFastMenuData.head_bar_fgc }, { CFG_ACT_WIN_AREA,
	BGC_HEAD_WIDGET, (void*) &resData->rFastMenuData.head_bar_bgc }, };

	/*configTable3 configTableIntValue[] = {
	 {CFG_BAR_STAT_DATE,	"hBorder",	 (void*)&resData->rFastMenuData.dateHBorder},
	 {CFG_BAR_STAT_DATE,	"yearW",  (void*)&resData->rFastMenuData.dateYearW  },
	 {CFG_BAR_STAT_DATE,	"dateLabelW", (void*)&resData->rFastMenuData.dateLabelW},
	 {CFG_BAR_STAT_DATE,	"numberW", (void*)&resData->rFastMenuData.dateNumberW},
	 {CFG_BAR_STAT_DATE,	"boxH", (void*)&resData->rFastMenuData.dateBoxH  },
	 };*/

	configTable2 configTableIcon[] = { { CFG_ACT_SUB1_WIN_ITEM0,
			(void*) &resData->rActMenuData.btnIcon[0] }, {
	CFG_ACT_SUB1_WIN_ITEM1, (void*) &resData->rActMenuData.btnIcon[1] }, {
	CFG_ACT_SUB1_WIN_ITEM2, (void*) &resData->rActMenuData.btnIcon[2] }, {
	CFG_ACT_SUB2_WIN_ITEM0, (void*) &resData->rActMenuData.btnIcon[3] }, {
	CFG_ACT_SUB2_WIN_ITEM1, (void*) &resData->rActMenuData.btnIcon[4] }, {
	CFG_ACT_SUB2_WIN_ITEM2, (void*) &resData->rActMenuData.btnIcon[5] }, {
	CFG_BOTTOM_PAGE_INDEX, (void*) &resData->rActMenuData.indexIcon }, };

	unsigned int i;
	for (i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey,
				(smRect*) configTableRect[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s rect failed: %d\n", configTableRect[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}

	for (i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey,
				configTableColor[i].subkey,
				(gal_pixel*) configTableColor[i].addr);
		if (err_code != ETC_OK) {
			sm_debug("get %s %s failed: %d\n", configTableColor[i].mainkey,
					configTableColor[i].subkey, err_code);
			retval = -1;
			goto out;
		}
	}

	/*for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
	 err_code = getIntValueFromHandle(resData->mCfgFileHandle,
	 configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
	 if(err_code < 0) {
	 sm_debug("get %s %s failed\n", configTableIntValue[i].mainkey,
	 configTableIntValue[i].subkey);
	 retval = -1;
	 goto out;
	 }
	 *(unsigned int*)configTableIntValue[i].addr = err_code;
	 }*/

	for (i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey,
				(currentIcon_t*) configTableIcon[i].addr);
		if (err_code != ETC_OK) {
			sm_error("get %s icon failed: %d\n", configTableIcon[i].mainkey,
					err_code);
			retval = -1;
			goto out;
		}
	}

	out: return retval;
}

int firstStage(void) {
	int err_code;
	int retval = 0;
	smRect rScreenRect;

	if (getCfgFileHandle() < 0) {
		sm_error("get file handle failed\n");
		return -1;
	}

	err_code = getRectFromFileHandle("screen_area", &resData->rScreenRect);
	if (err_code != ETC_OK) {
		sm_error("get screen rect failed: %d\n", err_code);
		retval = -1;
		goto out;
	}
	sm_debug("Screen Rect:(%d, %d)\n", resData->rScreenRect.w,
			resData->rScreenRect.h);
	err_code = getRectFromFileHandle("fast_win_area", &resData->rheadbarRect);
	if (err_code != ETC_OK) {
		sm_error("get screen rect failed: %d\n", err_code);
		retval = -1;
		goto out;
	}

	sm_debug("Screen Rect:(%d, %d)\n", resData->rheadbarRect.w,
			resData->rheadbarRect.h);
	return 0;
	out: releaseCfgFileHandle();
	return retval;
}

int secondStage(void) {
	int retval = 0;
	sm_debug("initStage2\n");

	initLangAndFont();

	if (initFastMenuResource() < 0) {
		sm_error("init fast menu resource failed\n");
		retval = -1;
		goto out;
	}

	if (initActMenuResource() < 0) {
		sm_error("init slider menu resource failed\n");
		retval = -1;
		goto out;
	}
	if (initPopMenuResource() < 0) {
		sm_error("init pop menu resource failed\n");
		retval = -1;
		goto out;
	}

	out: return retval;
}

void ResourceInit(void) {
	char cfgStr[] = "/usr/res/config/system.cfg";
	resData = (ResDataType*) malloc(sizeof(ResDataType));
	if (NULL == resData) {
		sm_error("malloc resource error\n");
		return;
	}
	memset((void *) resData, 0, sizeof(ResDataType));
	memcpy((void *) resData->configFile, (void *) cfgStr, sizeof(cfgStr));

	if (firstStage() < 0) {
		sm_debug("init stage1 error\n");
		return;
	}

	if (secondStage() < 0) {
		sm_debug("init stage2 error\n");
	}

	return;
}

void ResourceUninit(void) {
	sm_debug("resource uninit\n");
	deinitFont();

	if (resData)
		free(resData);

	return;
}
