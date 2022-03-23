#include "resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"

ResDataType *resData = NULL;
void setHwnd(unsigned int win_id, HWND hwnd) {
	resData->mHwnd[win_id] = hwnd;
}

int getPlatform(void) {
	if (resData == NULL)
		return -1;
	return resData->platform;
}

int initLangAndFont(void) {
	resData->mFont_Times18 = CreateLogFont("ttf", "times", "ISO8859-1",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 18, 0);

	resData->mFont_Times28 = CreateLogFont("ttf", "times", "ISO8859-1",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 28, 0);

	resData->mFont_Times15 = CreateLogFont("ttf", "times", "ISO8859-1",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 15, 0);

	resData->mFont_Times20 = CreateLogFont("ttf", "times", "ISO8859-1",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 20, 0);

	resData->mFont_Times25 = CreateLogFont("ttf", "times", "ISO8859-1",
	FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
	FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
	FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 30, 0);

	resData->mCurLogFont = resData->mFont_Times18;

	return 0;
}

int deinitFont(void) {
	DestroyLogFont(resData->mFont_Times18);
	DestroyLogFont(resData->mFont_Times28);
	DestroyLogFont(resData->mFont_Times15);
	DestroyLogFont(resData->mFont_Times20);
	DestroyLogFont(resData->mFont_Times25);
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
	case ID_FONT_TIMES_18:
		resData->mCurLogFont = resData->mFont_Times18;
		break;
	case ID_FONT_TIMES_28:
		resData->mCurLogFont = resData->mFont_Times28;
		break;
	case ID_FONT_TIMES_15:
		resData->mCurLogFont = resData->mFont_Times15;
		break;
	case ID_FONT_TIMES_20:
		resData->mCurLogFont = resData->mFont_Times20;
		break;
	case ID_FONT_TIMES_25:
		resData->mCurLogFont = resData->mFont_Times25;
		break;
	default:
		sm_error("invalid Font ID index: %d\n", id);
		break;
	}
	return resData->mCurLogFont;
}

gal_pixel getStatusBarColor(enum ResourceID resID, enum ColorType type) {
	gal_pixel color = 0;
	if (resID >= ID_BOTTOM_WIN) {
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
	case ID_BOTTOM_WIN:
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
	case ID_FRONT_CAMRA_SIZE:
		*rect = resData->rFrontCameraSize;
		break;
	case ID_BACK_CAMRA_SIZE:
		*rect = resData->rBackCameraSize;
		break;
	case ID_BOTTOM_WIN:
		*rect = resData->rFastMenuData.winRect;
		break;
	case ID_BOTTOM_WIN_BACK:
		*rect = resData->rFastMenuData.backRect;
		break;
	case ID_BOTTOM_WIN_SETUP:
		*rect = resData->rFastMenuData.setupRect;
		break;
	case ID_FUN_BUTTON:
		*rect = resData->rFunButonData.winRect;
		break;
	case ID_FUN_BUTTON_TPHOTO:
		*rect = resData->rFunButonData.rect[0];
		break;
	case ID_FUN_BUTTON_TAUDIO:
		*rect = resData->rFunButonData.rect[1];
		break;
	case ID_FUN_BUTTON_TVIDEO:
		*rect = resData->rFunButonData.rect[2];
		break;
	case ID_FUN_BUTTON_TOW_TVIDEO:
		*rect = resData->rFunButonData.rect[3];
		break;
	case ID_FUN_BUTTON_PHOTO:
		*rect = resData->rFunButonData.rect[4];
		break;
	case ID_FUN_BUTTON_AUDIO:
		*rect = resData->rFunButonData.rect[5];
		break;
	case ID_FUN_BUTTON_VIDEO:
		*rect = resData->rFunButonData.rect[6];
		break;
	case ID_HEAD_BAR_SD:
		*rect = resData->rHeadBarData.rect[0];
		break;
	case ID_HEAD_BAR_UD:
		*rect = resData->rHeadBarData.rect[1];
		break;
	case ID_BOTTOM_WIN_PHOTO:
		*rect = resData->rTakeButonData.rect[0];
		break;
	case ID_BOTTOM_WIN_AUDIO:
		*rect = resData->rTakeButonData.rect[1];
		break;
	case ID_BOTTOM_WIN_VIDEO:
		*rect = resData->rTakeButonData.rect[2];
		break;
	case ID_PLAYER_SCREEN:
		*rect = resData->rplayerbarRect;
		break;
	case ID_BOTTOM_MENU:
		*rect = resData->rbottombarRect;
		break;
	case ID_RIGHT_MENU:
		*rect = resData->rrightbarRect;
		break;
	case ID_BOTTOMMENU_BTN1:
		*rect = resData->rPopWinData.btnRect1[1];
		break;
	case ID_BOTTOMMENU_BTN2:
		*rect = resData->rPopWinData.btnRect1[0];
		break;
	case ID_HEADBAR:
		*rect = resData->rheadbarRect;
		break;
	case ID_BACK:
		*rect = resData->rPopWinData.btnRect[0];
		break;
	case ID_SDCARD:
		*rect = resData->rPopWinData.btnRect[1];
		break;
	case ID_BATTERY:
		*rect = resData->rPopWinData.btnRect[2];
		break;
	case ID_LIST:
		*rect = resData->rPopWinData.btnRect[3];
		break;
	case ID_LIST_FILE_NAME:
		*rect = resData->rPopWinData.btnRect[4];
		break;
	case ID_SETUP:
		*rect = resData->rPopWinData.btnRect[5];
		break;
	case ID_SETUPATTR:
		*rect = resData->rPopWinData.btnRect[6];
		break;
	case ID_MUSIC:
		*rect = resData->rPopWinData.btnRect[7];
		break;
	case ID_PLAYER:
		*rect = resData->rPopWinData.btnRect[8];
		break;
	case ID_BOTTOM_MUSIC_LEFT:
		*rect = resData->rPopWinData.btnRect[11];
		break;
	case ID_BOTTOM_MUSIC_MIDDLE:
		*rect = resData->rPopWinData.btnRect[12];
		break;
	case ID_BOTTOM_MUSIC_RIGHT:
		*rect = resData->rPopWinData.btnRect[13];
		break;
	case ID_PICTURE:
		*rect = resData->rPopWinData.btnRect[14];
		break;
	case ID_SEEKTOBAR:
		*rect = resData->rPopWinData.btnRect[15];
		break;
	case ID_PREVBUTTON:
		*rect = resData->rPopWinData.btnRect[16];
		break;
	case ID_NEXTBUTTON:
		*rect = resData->rPopWinData.btnRect[17];
		break;
	default:
		sm_error("invalid resID index: %d\n", resID);
		return -1;
	}

	return 0;
}

void setCurrentIconValue(enum ResourceID resID, int cur_val) {
	switch (resID) {
	case ID_BOTTOM_WIN_BACK:
		resData->rFastMenuData.backIcon.current = cur_val;
		break;
	case ID_BOTTOM_WIN_SETUP:
		resData->rFastMenuData.setupIcon.current = cur_val;
		break;
	case ID_FUN_BUTTON_TPHOTO:
		resData->rFunButonData.icon[0].current = cur_val;
		break;
	case ID_FUN_BUTTON_TAUDIO:
		resData->rFunButonData.icon[1].current = cur_val;
		break;
	case ID_FUN_BUTTON_TVIDEO:
		resData->rFunButonData.icon[2].current = cur_val;
		break;
	case ID_FUN_BUTTON_TOW_TVIDEO:
		resData->rFunButonData.icon[3].current = cur_val;
		break;
	case ID_FUN_BUTTON_PHOTO:
		resData->rFunButonData.icon[4].current = cur_val;
		break;
	case ID_FUN_BUTTON_AUDIO:
		resData->rFunButonData.icon[5].current = cur_val;
		break;
	case ID_FUN_BUTTON_VIDEO:
		resData->rFunButonData.icon[6].current = cur_val;
		break;
	case ID_HEAD_BAR_SD:
		resData->rHeadBarData.icon[0].current = cur_val;
		break;
	case ID_HEAD_BAR_UD:
		resData->rHeadBarData.icon[1].current = cur_val;
		break;
	case ID_BOTTOM_WIN_PHOTO:
		resData->rTakeButonData.icon[0].current = cur_val;
		break;
	case ID_BOTTOM_WIN_AUDIO:
		resData->rTakeButonData.icon[1].current = cur_val;
		break;
	case ID_BOTTOM_WIN_VIDEO:
		resData->rTakeButonData.icon[2].current = cur_val;
		break;
	case ID_MOUNT_PATH:
		resData->mountPath.current = cur_val;
		break;
	case ID_BOTTOMMENU_BTN1:
		resData->rPopWinData.btnIcon3[1].current = cur_val;
		break;
	case ID_BOTTOMMENU_BTN2:
		resData->rPopWinData.btnIcon3[2].current = cur_val;
		break;
	case ID_HEADBAR:
		resData->rPopWinData.btnIcon4[12].current = cur_val;
		break;
	case ID_BACK:
		resData->rPopWinData.btnIcon4[0].current = cur_val;
		break;
	case ID_SDCARD:
		resData->rPopWinData.btnIcon4[1].current = cur_val;
		break;
	case ID_BATTERY:
		resData->rPopWinData.btnIcon4[2].current = cur_val;
		break;
	case ID_LIST:
		resData->rPopWinData.btnIcon4[13].current = cur_val;
		break;
	case ID_SETUP:
		resData->rPopWinData.btnIcon4[3].current = cur_val;
		break;
	case ID_SETUPATTR:
		resData->rPopWinData.btnIcon4[4].current = cur_val;
		break;
	case ID_MUSIC:
		resData->rPopWinData.btnIcon4[5].current = cur_val;
		break;
	case ID_PLAYER:
		resData->rPopWinData.btnIcon4[6].current = cur_val;
		break;
	case ID_BOTTOM_MUSIC_LEFT:
		resData->rPopWinData.btnIcon4[8].current = cur_val;
		break;
	case ID_BOTTOM_MUSIC_MIDDLE:
		resData->rPopWinData.btnIcon4[9].current = cur_val;
		break;
	case ID_BOTTOM_MUSIC_RIGHT:
		resData->rPopWinData.btnIcon4[10].current = cur_val;
		break;
	case ID_PICTURE:
		resData->rPopWinData.btnIcon4[11].current = cur_val;
		break;
	case ID_SEEKTOBAR:
		resData->rPopWinData.btnIcon4[14].current = cur_val;
		break;
	case ID_PREVBUTTON:
		resData->rPopWinData.btnIcon4[15].current = cur_val;
		break;
	case ID_NEXTBUTTON:
		resData->rPopWinData.btnIcon4[16].current = cur_val;
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
	case ID_BOTTOM_WIN_BACK:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.backIcon);
		current = resData->rFastMenuData.backIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.backIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_WIN_SETUP:
		CHECK_CURRENT_ICON_VALID(resData->rFastMenuData.setupIcon);
		current = resData->rFastMenuData.setupIcon.current;
		memcpy((void *) file,
				(void *) resData->rFastMenuData.setupIcon.iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_TPHOTO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[0]);
		current = resData->rFunButonData.icon[0].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_TAUDIO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[1]);
		current = resData->rFunButonData.icon[1].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_TVIDEO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[2]);
		current = resData->rFunButonData.icon[2].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_TOW_TVIDEO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[3]);
		current = resData->rFunButonData.icon[3].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_PHOTO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[4]);
		current = resData->rFunButonData.icon[4].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[4].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_AUDIO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[5]);
		current = resData->rFunButonData.icon[5].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[5].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_FUN_BUTTON_VIDEO:
		CHECK_CURRENT_ICON_VALID(resData->rFunButonData.icon[6]);
		current = resData->rFunButonData.icon[6].current;
		memcpy((void *) file,
				(void *) resData->rFunButonData.icon[6].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_HEAD_BAR_SD:
		CHECK_CURRENT_ICON_VALID(resData->rHeadBarData.icon[0]);
		current = resData->rHeadBarData.icon[0].current;
		memcpy((void *) file,
				(void *) resData->rHeadBarData.icon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_HEAD_BAR_UD:
		CHECK_CURRENT_ICON_VALID(resData->rHeadBarData.icon[1]);
		current = resData->rHeadBarData.icon[1].current;
		memcpy((void *) file,
				(void *) resData->rHeadBarData.icon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_WIN_PHOTO:
		CHECK_CURRENT_ICON_VALID(resData->rTakeButonData.icon[0]);
		current = resData->rTakeButonData.icon[0].current;
		memcpy((void *) file,
				(void *) resData->rTakeButonData.icon[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_WIN_AUDIO:
		CHECK_CURRENT_ICON_VALID(resData->rTakeButonData.icon[1]);
		current = resData->rTakeButonData.icon[1].current;
		memcpy((void *) file,
				(void *) resData->rTakeButonData.icon[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_WIN_VIDEO:
		CHECK_CURRENT_ICON_VALID(resData->rTakeButonData.icon[2]);
		current = resData->rTakeButonData.icon[2].current;
		memcpy((void *) file,
				(void *) resData->rTakeButonData.icon[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_MOUNT_PATH:
		CHECK_CURRENT_ICON_VALID(resData->mountPath);
		current = resData->mountPath.current;
		memcpy((void *) file, (void *) resData->mountPath.iconName[current],
		ICON_PATH_SIZE);
		break;
	case ID_BOTTOMMENU_BTN1:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon3[1]);
		current = resData->rPopWinData.btnIcon3[1].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon3[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_HEADBAR:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[12]);
		current = resData->rPopWinData.btnIcon4[12].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[12].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BACK:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[0]);
		current = resData->rPopWinData.btnIcon4[0].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[0].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_SDCARD:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[1]);
		current = resData->rPopWinData.btnIcon4[1].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[1].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BATTERY:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[2]);
		current = resData->rPopWinData.btnIcon4[2].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[2].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_LIST:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[13]);
		current = resData->rPopWinData.btnIcon4[13].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[13].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_SETUP:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[3]);
		current = resData->rPopWinData.btnIcon4[3].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[3].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_SETUPATTR:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[4]);
		current = resData->rPopWinData.btnIcon4[4].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[4].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_MUSIC:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[5]);
		current = resData->rPopWinData.btnIcon4[5].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[5].iconName[current],
				ICON_PATH_SIZE);
		break;

	case ID_PLAYER:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[6]);
		current = resData->rPopWinData.btnIcon4[6].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[6].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_MUSIC_LEFT:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[8]);
		current = resData->rPopWinData.btnIcon4[8].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[8].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_MUSIC_MIDDLE:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[9]);
		current = resData->rPopWinData.btnIcon4[9].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[9].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_BOTTOM_MUSIC_RIGHT:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[10]);
		current = resData->rPopWinData.btnIcon4[10].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[10].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_PICTURE:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[11]);
		current = resData->rPopWinData.btnIcon4[11].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[11].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_SEEKTOBAR:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[14]);
		current = resData->rPopWinData.btnIcon4[14].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[14].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_PREVBUTTON:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[15]);
		current = resData->rPopWinData.btnIcon4[15].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[15].iconName[current],
				ICON_PATH_SIZE);
		break;
	case ID_NEXTBUTTON:
		CHECK_CURRENT_ICON_VALID(resData->rPopWinData.btnIcon4[16]);
		current = resData->rPopWinData.btnIcon4[16].current;
		memcpy((void *) file,
				(void *) resData->rPopWinData.btnIcon4[16].iconName[current],
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
	if (resID == ID_BOTTOM_WIN) {
		if (getWindPicFileName(type, file) < 0) {
			sm_error("get window pic bmp failed\n");
			return -1;
		}
	} else if (resID >= ID_BOTTOM_WIN && resID <= ID_BM_STAT_WIN_PAGE_INDEX) {
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
		sm_error("currentIcon->current:%ud\n", currentIcon->current);
		return -1;
	}

	return ETC_OK;
}

int initFastMenuResource(void) {
	int err_code;
	int retval = 0;
	char buf[ICON_PATH_SIZE];

	configTable2 configTableRect[] = { { CFG_BOTTOM_WIN_AREA,
			(void*) &resData->rFastMenuData.winRect }, { CFG_BOTTOM_WIN_BACK,
			(void*) &resData->rFastMenuData.backRect }, {
	CFG_BOTTOM_WIN_SETUP, (void*) &resData->rFastMenuData.setupRect }, {
	CFG_FUN_BUTTON_AREA, (void*) &resData->rFunButonData.winRect }, {
	CFG_FUN_BUTTON_TPHOTO, (void*) &resData->rFunButonData.rect[0] }, {
	CFG_FUN_BUTTON_TAUDIO, (void*) &resData->rFunButonData.rect[1] }, {
	CFG_FUN_BUTTON_TVIDEO, (void*) &resData->rFunButonData.rect[2] }, {
	CFG_FUN_BUTTON_TOW_TVIDEO, (void*) &resData->rFunButonData.rect[3] }, {
	CFG_FUN_BUTTON_PHOTO, (void*) &resData->rFunButonData.rect[4] }, {
	CFG_FUN_BUTTON_AUDIO, (void*) &resData->rFunButonData.rect[5] }, {
	CFG_FUN_BUTTON_VIDEO, (void*) &resData->rFunButonData.rect[6] }, {
	CFG_BOTTOM_WIN_PHOTO, (void*) &resData->rTakeButonData.rect[0] }, {
	CFG_BOTTOM_WIN_AUDIO, (void*) &resData->rTakeButonData.rect[1] }, {
	CFG_BOTTOM_WIN_VIDEO, (void*) &resData->rTakeButonData.rect[2] }, {
	CFG_HEAD_BAR_SD, (void*) &resData->rHeadBarData.rect[0] }, {
	CFG_HEAD_BAR_UD, (void*) &resData->rHeadBarData.rect[1] }, {
	CFG_FRONT_CAMRA_SIZE, (void*) &resData->rFrontCameraSize }, {
	CFG_BACK_CAMRA_SIZE, (void*) &resData->rBackCameraSize }, {
	CFG_PLAYER_WINDOW_AREA, (void*) &resData->rplayerbarRect }, {
	CFG_PLAYER_BOTTOM_MENU, (void*) &resData->rbottombarRect }, {
	CFG_PLAYER_RIGHT_MENU, (void*) &resData->rrightbarRect }, {
	CFG_BOTTOM_MENU_BTN1, (void*) &resData->rPopWinData.btnRect1[1] }, {
	CFG_BOTTOM_MENU_BTN2, (void*) &resData->rPopWinData.btnRect1[2] }, {
	CFG_PLAYERHEADBAR_MENU, (void*) &resData->rheadbarRect }, {
	CFG_BACK_BTN0, (void*) &resData->rPopWinData.btnRect[0] }, {
	CFG_SDCARD, (void*) &resData->rPopWinData.btnRect[1] }, {
	CFG_BATTERY, (void*) &resData->rPopWinData.btnRect[2] }, {
	CFG_HEADBAR_LIST, &resData->rPopWinData.btnRect[3] }, {
	CFG_LIST_FILE_NAME, (void*) &resData->rPopWinData.btnRect[4] }, {
	CFG_HEADBAR_SETUP, (void*) &resData->rPopWinData.btnRect[5] }, {
	CFG_RIGHT_MENU_SETUPATTR, (void*) &resData->rPopWinData.btnRect[6] }, {
	CFG_BOTTOM_MUSIC_LEFT, (void*) &resData->rPopWinData.btnRect[11] }, {
	CFG_BOTTOM_MUSIC_MIDDLE, (void*) &resData->rPopWinData.btnRect[12] }, {
	CFG_BOTTOM_MUSIC_RIGHT, (void*) &resData->rPopWinData.btnRect[13] }, {
	CFG_BOTTOM_NUSIC_SEEKTOBAR, (void*) &resData->rPopWinData.btnRect[15] }, {
	CFG_RIGHT_PREBUTTON, (void*) &resData->rPopWinData.btnRect[16] }, {
	CFG_RIGHT_NEXTBUTTON, (void*) &resData->rPopWinData.btnRect[17] }, };

	configTable3 configTableColor[] = { { CFG_BOTTOM_WIN_AREA, FGC_WIDGET,
			(void*) &resData->rFastMenuData.fgc }, { CFG_BOTTOM_WIN_AREA,
	BGC_WIDGET, (void*) &resData->rFastMenuData.bgc }, {
	CFG_BOTTOM_WIN_AREA, FGC_HEAD_WIDGET,
			(void*) &resData->rFastMenuData.head_bar_fgc }, {
	CFG_BOTTOM_WIN_AREA,
	BGC_HEAD_WIDGET, (void*) &resData->rFastMenuData.head_bar_bgc }, };

	configTable2 configTableIcon[] = { { CFG_MOUNT_PATH,
			(void*) &resData->mountPath }, {
	CFG_BOTTOM_WIN_BACK, (void*) &resData->rFastMenuData.backIcon }, {
	CFG_BOTTOM_WIN_SETUP, (void*) &resData->rFastMenuData.setupIcon }, {
	CFG_FUN_BUTTON_TPHOTO, (void*) &resData->rFunButonData.icon[0] }, {
	CFG_FUN_BUTTON_TAUDIO, (void*) &resData->rFunButonData.icon[1] }, {
	CFG_FUN_BUTTON_TVIDEO, (void*) &resData->rFunButonData.icon[2] }, {
	CFG_FUN_BUTTON_TOW_TVIDEO, (void*) &resData->rFunButonData.icon[3] }, {
	CFG_FUN_BUTTON_PHOTO, (void*) &resData->rFunButonData.icon[4] }, {
	CFG_FUN_BUTTON_AUDIO, (void*) &resData->rFunButonData.icon[5] }, {
	CFG_FUN_BUTTON_VIDEO, (void*) &resData->rFunButonData.icon[6] }, {
	CFG_BOTTOM_WIN_PHOTO, (void*) &resData->rTakeButonData.icon[0] }, {
	CFG_BOTTOM_WIN_AUDIO, (void*) &resData->rTakeButonData.icon[1] }, {
	CFG_BOTTOM_WIN_VIDEO, (void*) &resData->rTakeButonData.icon[2] }, {
	CFG_HEAD_BAR_SD, (void*) &resData->rHeadBarData.icon[0] }, {
	CFG_HEAD_BAR_UD, (void*) &resData->rHeadBarData.icon[1] }, {
	CFG_BOTTOM_MENU_BTN1, (void*) &resData->rPopWinData.btnIcon3[1] }, {
	CFG_BACK_BTN0, (void*) &resData->rPopWinData.btnIcon4[0] }, {
	CFG_SDCARD, (void*) &resData->rPopWinData.btnIcon4[1] }, {
	CFG_BATTERY, (void*) &resData->rPopWinData.btnIcon4[2] }, {
	CFG_HEADBAR_SETUP, (void*) &resData->rPopWinData.btnIcon4[3] }, {
	CFG_RIGHT_MENU_SETUPATTR, (void*) &resData->rPopWinData.btnIcon4[4] }, {
	CFG_BOTTOM_MUSIC_LEFT, (void*) &resData->rPopWinData.btnIcon4[8] }, {
	CFG_BOTTOM_MUSIC_MIDDLE, (void*) &resData->rPopWinData.btnIcon4[9] }, {
	CFG_BOTTOM_MUSIC_RIGHT, (void*) &resData->rPopWinData.btnIcon4[10] }, {
	CFG_PLAYERHEADBAR_MENU, (void*) &resData->rPopWinData.btnIcon4[12] }, {
	CFG_HEADBAR_LIST, (void*) &resData->rPopWinData.btnIcon4[13] }, {
	CFG_BOTTOM_NUSIC_SEEKTOBAR, (void*) &resData->rPopWinData.btnIcon4[14] }, {
	CFG_RIGHT_PREBUTTON, (void*) &resData->rPopWinData.btnIcon4[15] }, {
	CFG_RIGHT_NEXTBUTTON, (void*) &resData->rPopWinData.btnIcon4[16] }, };

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

	if ((err_code = GetIntValueFromEtc(resData->mCfgFileHandle,
	CFG_PLATFORM_NUM, "platform", &resData->platform)) != ETC_OK) {
		retval = -1;
		goto out;
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
