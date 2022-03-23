#include "resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"

ResDataType *resData = NULL;

int initLangAndFont(void) {

    smRect rect;
    getResRect(ID_SCREEN, &rect);
    int fontSize[7] = { 14, 18, 22, 24, 28, 40, 68 };
    if (rect.w > 240) {
        /* Adapt to 320x480 resolution */
        fontSize[0] = 22;
        fontSize[1] = 26;
        fontSize[2] = 30;
        fontSize[3] = 34;
        fontSize[4] = 38;
        fontSize[5] = 52;
        fontSize[6] = 94;
    }

    resData->mFont_Times14 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[0],
            0);

    resData->mFont_Times18 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[1],
            0);

    resData->mFont_Times22 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[2],
            0);

    resData->mFont_Times24 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[3],
            0);

    resData->mFont_Times28 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[4],
            0);

    resData->mFont_Times40 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[5],
            0);

    resData->mFont_Times68 = CreateLogFont("ttf", "pingfang", "UTF-8",
    FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
    FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, fontSize[6],
            0);

    resData->mCurLogFont = resData->mFont_Times18;

    return 0;
}

int deinitFont(void) {
    DestroyLogFont(resData->mFont_Times14);
    DestroyLogFont(resData->mFont_Times18);
    DestroyLogFont(resData->mFont_Times68);
    DestroyLogFont(resData->mFont_Times28);
    DestroyLogFont(resData->mFont_Times24);
    DestroyLogFont(resData->mFont_Times22);
    DestroyLogFont(resData->mFont_Times40);
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
    case ID_FONT_TIMES_14:
        resData->mCurLogFont = resData->mFont_Times14;
        break;
    case ID_FONT_TIMES_18:
        resData->mCurLogFont = resData->mFont_Times18;
        break;
    case ID_FONT_TIMES_68:
        resData->mCurLogFont = resData->mFont_Times68;
        break;
    case ID_FONT_TIMES_28:
        resData->mCurLogFont = resData->mFont_Times28;
        break;
    case ID_FONT_TIMES_24:
        resData->mCurLogFont = resData->mFont_Times24;
        break;
    case ID_FONT_TIMES_22:
        resData->mCurLogFont = resData->mFont_Times22;
        break;
    case ID_FONT_TIMES_40:
        resData->mCurLogFont = resData->mFont_Times40;
        break;
    default:
        sm_error("invalid Font ID index: %d\n", id);
        break;
    }
    return resData->mCurLogFont;
}

int getResRect(enum ResourceID resID, smRect *rect) {
    memset(rect, 0, sizeof(smRect));
    switch (resID) {
    case ID_SCREEN:
        *rect = resData->rScreenRect;
        break;
    case ID_TIME_TITLE_MAIN_AREA:
        *rect = resData->timeTitleRect[0];
        break;
    case ID_TIME_TITLE_WELCOME_AREA:
        *rect = resData->timeTitleRect[1];
        break;
    case ID_DESK_BUTTON_AREA:
        *rect = resData->deskButtonRect;
        break;
    case ID_HEADBAR_BUTTON_BMP_AREA:
        *rect = resData->headBarRect;
        break;
    case ID_CONTROL_BUTTON_NEXT_BMP_AREA:
        *rect = resData->controlButtonRect[0];
        break;
    case ID_CONTROL_BUTTON_VOLUME_BMP_AREA:
        *rect = resData->controlButtonRect[1];
        break;
    case ID_CONTROL_BUTTON_BLUETOOTH_BMP_AREA:
        *rect = resData->controlButtonRect[2];
        break;
    case ID_CONTROL_BUTTONS_BMP_AREA:
        *rect = resData->controlButtonsRect;
        break;
    case ID_PROGRESS_AREA:
        *rect = resData->progressRect;
        break;
    case ID_LIST_AREA:
        *rect = resData->listRect;
        break;
    default:
        sm_error("invalid resID index: %d\n", resID);
        return -1;
    }

    return 0;
}

void setCurrentIconValue(enum ResourceID resID, int cur_val) {
    switch (resID) {
    case ID_DESK_BUTTON_ONE_BMP:
        resData->deskButtonOneIcon.current = cur_val;
        break;
    case ID_DESK_BUTTON_TOW_BMP:
        resData->deskButtonTowIcon.current = cur_val;
        break;
    case ID_DESK_VOLUME_BMP:
        resData->deskVolumeIcon.current = cur_val;
        break;
    case ID_HEADBAR_BUTTON_BMP_AREA:
        resData->headbarButtonIcon.current = cur_val;
        break;
    case ID_MUSIC_PLAYING_BMP:
        resData->musicPlayingIcon.current = cur_val;
        break;
    case ID_CONTROL_BUTTON_NEXT_BMP_AREA:
        resData->controlButtonIcon[0].current = cur_val;
        break;
    case ID_CONTROL_BUTTON_VOLUME_BMP_AREA:
        resData->controlButtonIcon[1].current = cur_val;
        break;
    case ID_CONTROL_BUTTON_BLUETOOTH_BMP_AREA:
        resData->controlButtonIcon[2].current = cur_val;
        break;
    case ID_MUSIC_AUX_BMP:
        resData->musicAuxIcon.current = cur_val;
        break;
    case ID_CONTROL_BUTTONS_BMP_AREA:
        resData->controlButtonsIcon.current = cur_val;
        break;
    case ID_SETTING_BUTTON_BMP:
        resData->settingButtonIcon.current = cur_val;
        break;
    case ID_NORMAL_BUTTON_BMP:
        resData->normalButtonIcon.current = cur_val;
        break;
    case ID_SWITCH_BUTTON_BMP:
        resData->switchButtonIcon.current = cur_val;
        break;
    case ID_SELECT_BUTTON_BMP:
        resData->selectButtonIcon.current = cur_val;
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
    case ID_DESK_BUTTON_ONE_BMP:
        CHECK_CURRENT_ICON_VALID(resData->deskButtonOneIcon);
        current = resData->deskButtonOneIcon.current;
        memcpy((void *) file,
                (void *) resData->deskButtonOneIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_DESK_BUTTON_TOW_BMP:
        CHECK_CURRENT_ICON_VALID(resData->deskButtonTowIcon);
        current = resData->deskButtonTowIcon.current;
        memcpy((void *) file,
                (void *) resData->deskButtonTowIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_DESK_VOLUME_BMP:
        CHECK_CURRENT_ICON_VALID(resData->deskVolumeIcon);
        current = resData->deskVolumeIcon.current;
        memcpy((void *) file,
                (void *) resData->deskVolumeIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_HEADBAR_BUTTON_BMP_AREA:
        CHECK_CURRENT_ICON_VALID(resData->headbarButtonIcon);
        current = resData->headbarButtonIcon.current;
        memcpy((void *) file,
                (void *) resData->headbarButtonIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_MUSIC_PLAYING_BMP:
        CHECK_CURRENT_ICON_VALID(resData->musicPlayingIcon);
        current = resData->musicPlayingIcon.current;
        memcpy((void *) file,
                (void *) resData->musicPlayingIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_CONTROL_BUTTON_NEXT_BMP_AREA:
        CHECK_CURRENT_ICON_VALID(resData->controlButtonIcon[0]);
        current = resData->controlButtonIcon[0].current;
        memcpy((void *) file,
                (void *) resData->controlButtonIcon[0].iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_CONTROL_BUTTON_VOLUME_BMP_AREA:
        CHECK_CURRENT_ICON_VALID(resData->controlButtonIcon[1]);
        current = resData->controlButtonIcon[1].current;
        memcpy((void *) file,
                (void *) resData->controlButtonIcon[1].iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_CONTROL_BUTTON_BLUETOOTH_BMP_AREA:
        CHECK_CURRENT_ICON_VALID(resData->controlButtonIcon[2]);
        current = resData->controlButtonIcon[2].current;
        memcpy((void *) file,
                (void *) resData->controlButtonIcon[2].iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_MUSIC_AUX_BMP:
        CHECK_CURRENT_ICON_VALID(resData->musicAuxIcon);
        current = resData->musicAuxIcon.current;
        memcpy((void *) file, (void *) resData->musicAuxIcon.iconName[current],
        ICON_PATH_SIZE);
        break;
    case ID_CONTROL_BUTTONS_BMP_AREA:
        CHECK_CURRENT_ICON_VALID(resData->controlButtonsIcon);
        current = resData->controlButtonsIcon.current;
        memcpy((void *) file,
                (void *) resData->controlButtonsIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_SETTING_BUTTON_BMP:
        CHECK_CURRENT_ICON_VALID(resData->settingButtonIcon);
        current = resData->settingButtonIcon.current;
        memcpy((void *) file,
                (void *) resData->settingButtonIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_NORMAL_BUTTON_BMP:
        CHECK_CURRENT_ICON_VALID(resData->normalButtonIcon);
        current = resData->normalButtonIcon.current;
        memcpy((void *) file,
                (void *) resData->normalButtonIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_SWITCH_BUTTON_BMP:
        CHECK_CURRENT_ICON_VALID(resData->switchButtonIcon);
        current = resData->switchButtonIcon.current;
        memcpy((void *) file,
                (void *) resData->switchButtonIcon.iconName[current],
                ICON_PATH_SIZE);
        break;
    case ID_SELECT_BUTTON_BMP:
        CHECK_CURRENT_ICON_VALID(resData->selectButtonIcon);
        current = resData->selectButtonIcon.current;
        memcpy((void *) file,
                (void *) resData->selectButtonIcon.iconName[current],
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
    if (resID == ID_SCREEN) {
        if (getWindPicFileName(type, file) < 0) {
            sm_error("get window pic bmp failed\n");
            return -1;
        }
    } else if (resID >= ID_SCREEN && resID <= ID_BM_STAT_WIN_PAGE_INDEX) {
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

    configTable2 configTableRect[] =
            { {
            CFG_TIME_TITLE_MAIN_AREA, (void *) &resData->timeTitleRect[0] }, {
            CFG_TIME_TITLE_WELCOME_AREA, (void *) &resData->timeTitleRect[1] },
                    {
                    CFG_DESK_BUTTON_AREA, (void *) &resData->deskButtonRect },
                    {
                    CFG_HEADBAR_BUTTON_BMP_AREA, (void *) &resData->headBarRect },
                    {
                    CFG_CONTROL_BUTTON_NEXT_BMP_AREA,
                            (void *) &resData->controlButtonRect[0] }, {
                    CFG_CONTROL_BUTTON_VOLUME_BMP_AREA,
                            (void *) &resData->controlButtonRect[1] }, {
                    CFG_CONTROL_BUTTON_BLUETOOTH_BMP_AREA,
                            (void *) &resData->controlButtonRect[2] }, {
                    CFG_CONTROL_BUTTONS_BMP_AREA,
                            (void *) &resData->controlButtonsRect }, {
                    CFG_PROGRESS_AREA, (void *) &resData->progressRect }, {
                    CFG_LIST_AREA, (void *) &resData->listRect }, };

    configTable3 configTableColor[] = { };

    configTable2 configTableIcon[] =
            { {
            CFG_DESK_BUTTON_ONE_BMP, (void*) &resData->deskButtonOneIcon }, {
            CFG_DESK_BUTTON_TOW_BMP, (void*) &resData->deskButtonTowIcon }, {
            CFG_DESK_VOLUME_BMP, (void*) &resData->deskVolumeIcon }, {
            CFG_HEADBAR_BUTTON_BMP_AREA, (void*) &resData->headbarButtonIcon },
                    {
                    CFG_MUSIC_PLAYING_BMP, (void*) &resData->musicPlayingIcon },
                    {
                    CFG_CONTROL_BUTTON_NEXT_BMP_AREA,
                            (void*) &resData->controlButtonIcon[0] }, {
                    CFG_CONTROL_BUTTON_VOLUME_BMP_AREA,
                            (void*) &resData->controlButtonIcon[1] }, {
                    CFG_CONTROL_BUTTON_BLUETOOTH_BMP_AREA,
                            (void*) &resData->controlButtonIcon[2] }, {
                    CFG_MUSIC_AUX_BMP, (void*) &resData->musicAuxIcon }, {
                    CFG_CONTROL_BUTTONS_BMP_AREA,
                            (void*) &resData->controlButtonsIcon },
                    {
                    CFG_SETTING_BUTTON_BMP, (void*) &resData->settingButtonIcon },
                    {
                    CFG_NORMAL_BUTTON_BMP, (void*) &resData->normalButtonIcon },
                    {
                    CFG_SWITCH_BUTTON_BMP, (void*) &resData->switchButtonIcon },
                    {
                    CFG_SELECT_BUTTON_BMP, (void*) &resData->selectButtonIcon }, };

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

    if (initFastMenuResource() < 0) {
        sm_error("init fast menu resource failed\n");
        retval = -1;
        goto out;
    }

    initLangAndFont();

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

    languageType = Language_CN;

    return;
}

void ResourceUninit(void) {
    sm_debug("resource uninit\n");
    deinitFont();

    if (resData)
        free(resData);

    return;
}
