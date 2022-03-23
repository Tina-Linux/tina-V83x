/*
 * datafactory.c
 *
 * Note! ! !
 * Different controls, the same image is loaded many times,
 * here you can optimize the memory footprint! ! !
 *
 * Especially the music list
 *
 *  Created on: 2019/8/16
 *      Author: anruliu
 */

#include "datafactory.h"
#include "resource.h"
#include "globalization.h"

TimeTitleData *TimeTitleDataInit(TimeTitleData *timeTitleData, Uint32 flag) {
    timeTitleData = (TimeTitleData *) malloc(sizeof(TimeTitleData));
    if (NULL == timeTitleData) {
        sm_error("malloc TimeTitleData error\n");
    }
    memset((void *) timeTitleData, 0, sizeof(TimeTitleData));

    timeTitleData->flag = flag;

    if (flag == 0) {
        /* smart music player window */
        timeTitleData->timeLogfont = getLogFont(ID_FONT_TIMES_40);
        timeTitleData->timeDataLogfont = getLogFont(ID_FONT_TIMES_18);
        timeTitleData->volumeLogfont = getLogFont(ID_FONT_TIMES_18);
        setCurrentIconValue(ID_DESK_VOLUME_BMP, 0);
        getResBmp(ID_DESK_VOLUME_BMP, BMPTYPE_BASE, &timeTitleData->volumeBmp);
    } else {
        /* screen savers window */
        timeTitleData->timeLogfont = getLogFont(ID_FONT_TIMES_68);
        timeTitleData->timeDataLogfont = getLogFont(ID_FONT_TIMES_28);
    }

    return timeTitleData;
}

DeskButotnData *DeskButotnDataInit(DeskButotnData *deskButotnData, Uint32 flag) {
    deskButotnData = (DeskButotnData *) malloc(sizeof(DeskButotnData));
    if (NULL == deskButotnData) {
        sm_error("malloc DeskButotnData error\n");
    }
    memset((void *) deskButotnData, 0, sizeof(DeskButotnData));

    deskButotnData->flag = flag;
    deskButotnData->desLogfont = getLogFont(ID_FONT_TIMES_18);

    if (flag == 0) {
        /* smart music player window */
        int i;
        for (i = 0; i < 6; i++) {
            setCurrentIconValue(ID_DESK_BUTTON_ONE_BMP, i);
            getResBmp(ID_DESK_BUTTON_ONE_BMP, BMPTYPE_BASE,
                    &deskButotnData->buttonData[i].bmpNormal);
            setCurrentIconValue(ID_DESK_BUTTON_ONE_BMP, i + 6);
            getResBmp(ID_DESK_BUTTON_ONE_BMP, BMPTYPE_BASE,
                    &deskButotnData->buttonData[i].bmpSelect);
        }
    } else {
        /* scene window */
        int i;
        for (i = 0; i < 4; i++) {
            setCurrentIconValue(ID_DESK_BUTTON_TOW_BMP, i);
            getResBmp(ID_DESK_BUTTON_TOW_BMP, BMPTYPE_BASE,
                    &deskButotnData->buttonData[i].bmpNormal);
            setCurrentIconValue(ID_DESK_BUTTON_TOW_BMP, i + 4);
            getResBmp(ID_DESK_BUTTON_TOW_BMP, BMPTYPE_BASE,
                    &deskButotnData->buttonData[i].bmpSelect);
        }
    }
    return deskButotnData;
}

HeadbarData *HeadbarDataInit(HeadbarData *headbarData, Uint32 flag,
        Uint32 index) {
    headbarData = (HeadbarData *) malloc(sizeof(HeadbarData));
    if (NULL == headbarData) {
        sm_error("malloc TimeTitleData error\n");
    }
    memset((void *) headbarData, 0, sizeof(HeadbarData));

    headbarData->titleLogfont = getLogFont(ID_FONT_TIMES_24);
    headbarData->titleIndex = index;
    setCurrentIconValue(ID_HEADBAR_BUTTON_BMP_AREA, 0);
    getResBmp(ID_HEADBAR_BUTTON_BMP_AREA, BMPTYPE_BASE,
            &headbarData->buttonData.bmpNormal);
    setCurrentIconValue(ID_HEADBAR_BUTTON_BMP_AREA, 1);
    getResBmp(ID_HEADBAR_BUTTON_BMP_AREA, BMPTYPE_BASE,
            &headbarData->buttonData.bmpSelect);
    headbarData->flag = flag;

    if (flag) {
        /* Load music to play animated pictures */
        int i;
        for (i = 0; i < 4; i++) {
            setCurrentIconValue(ID_MUSIC_PLAYING_BMP, i);
            getResBmp(ID_MUSIC_PLAYING_BMP, BMPTYPE_BASE,
                    &headbarData->musicPlayingBmp[i]);
        }
    }

    return headbarData;
}

/*
 * @param index 0 is next, 1 is volume, 2 is bluetooth
 */
ControlButotnData *ControlButotnDataInit(ControlButotnData *controlButotnData,
        Uint32 index) {
    controlButotnData = (ControlButotnData *) malloc(sizeof(ControlButotnData));
    if (NULL == controlButotnData) {
        sm_error("malloc ControlButotnData error\n");
    }
    memset((void *) controlButotnData, 0, sizeof(ControlButotnData));

    controlButotnData->buttonData[1].switchMode = SWITCH_MODE_STATIC;
    controlButotnData->flag = index;

    int i;
    for (i = 0; i < 3; i++) {
        setCurrentIconValue(ID_CONTROL_BUTTON_NEXT_BMP_AREA + index, i);
        getResBmp(ID_CONTROL_BUTTON_NEXT_BMP_AREA + index, BMPTYPE_BASE,
                &controlButotnData->buttonData[i].bmpNormal);
        setCurrentIconValue(ID_CONTROL_BUTTON_NEXT_BMP_AREA + index, i + 3);
        getResBmp(ID_CONTROL_BUTTON_NEXT_BMP_AREA + index, BMPTYPE_BASE,
                &controlButotnData->buttonData[i].bmpSelect);
    }

    return controlButotnData;
}

ControlButotnsData *ControlButotnsDataInit(
        ControlButotnsData *controlButotnsData) {
    controlButotnsData = (ControlButotnsData *) malloc(
            sizeof(ControlButotnsData));
    if (NULL == controlButotnsData) {
        sm_error("malloc ControlButotnsData error\n");
    }
    memset((void *) controlButotnsData, 0, sizeof(ControlButotnsData));

    int i;
    for (i = 0; i < 3; i++) {
        setCurrentIconValue(ID_CONTROL_BUTTONS_BMP_AREA, i);
        getResBmp(ID_CONTROL_BUTTONS_BMP_AREA, BMPTYPE_BASE,
                &controlButotnsData->buttonData[i].bmpNormal);
        setCurrentIconValue(ID_CONTROL_BUTTONS_BMP_AREA, i + 3);
        getResBmp(ID_CONTROL_BUTTONS_BMP_AREA, BMPTYPE_BASE,
                &controlButotnsData->buttonData[i].bmpSelect);
    }

    for (i = 0; i < 4; i++) {
        setCurrentIconValue(ID_CONTROL_BUTTONS_BMP_AREA, i + 6);
        getResBmp(ID_CONTROL_BUTTONS_BMP_AREA, BMPTYPE_BASE,
                &controlButotnsData->playModeBmp[i]);
    }

    return controlButotnsData;
}

ProgressData *ProgressDataInit(ProgressData *progressData) {
    progressData = (ProgressData *) malloc(sizeof(ProgressData));
    if (NULL == progressData) {
        sm_error("malloc ProgressData error\n");
    }
    memset((void *) progressData, 0, sizeof(ProgressData));

    progressData->currentProgress = 0;

    return progressData;
}

/*
 * @param flag 0 is system password setting, 1 is administrator setting
 */
SettingButtonData *SettingButtonDataInit(SettingButtonData *settingButtonData,
        Uint32 flag) {
    settingButtonData = (SettingButtonData *) malloc(sizeof(SettingButtonData));
    if (NULL == settingButtonData) {
        sm_error("malloc SettingButtonData error\n");
    }
    memset((void *) settingButtonData, 0, sizeof(SettingButtonData));

    settingButtonData->settingLogfont = getLogFont(ID_FONT_TIMES_24);

    if (flag) {
        settingButtonData->settingText = getHeadbarDesText(7);
        setCurrentIconValue(ID_SETTING_BUTTON_BMP, 1);
        getResBmp(ID_SETTING_BUTTON_BMP, BMPTYPE_BASE,
                &settingButtonData->settingBmp);
    } else {
        settingButtonData->settingText = getHeadbarDesText(6);
        setCurrentIconValue(ID_SETTING_BUTTON_BMP, 0);
        getResBmp(ID_SETTING_BUTTON_BMP, BMPTYPE_BASE,
                &settingButtonData->settingBmp);
    }

    return settingButtonData;
}

ButtonData *NormalButtonDataInit(ButtonData *buttonData, Uint32 index) {
    buttonData = (ButtonData *) malloc(sizeof(ButtonData));
    if (NULL == buttonData) {
        sm_error("malloc ButtonData error\n");
    }
    memset((void *) buttonData, 0, sizeof(ButtonData));

    setCurrentIconValue(ID_NORMAL_BUTTON_BMP, index);
    getResBmp(ID_NORMAL_BUTTON_BMP, BMPTYPE_BASE, &buttonData->bmpNormal);

    setCurrentIconValue(ID_NORMAL_BUTTON_BMP, index + 4);
    getResBmp(ID_NORMAL_BUTTON_BMP, BMPTYPE_BASE, &buttonData->bmpSelect);

    return buttonData;
}

/*
 * @param flag 0 button default state, 1 button selected state
 * @param index 0 08:00, 1 is 09:00, 2 is 10:00, Up is the setting
 */
ListItemSwitchData *ListItemSwitchDataInit(
        ListItemSwitchData *listItemSwitchData, Uint32 flag, Uint32 index) {
    listItemSwitchData = (ListItemSwitchData *) malloc(
            sizeof(ListItemSwitchData));
    if (NULL == listItemSwitchData) {
        sm_error("malloc ListItemSwitchData error\n");
    }
    memset((void *) listItemSwitchData, 0, sizeof(ListItemSwitchData));

    listItemSwitchData->switchButtonData = (SwitchButtonData *) malloc(
            sizeof(SwitchButtonData));
    if (NULL == listItemSwitchData->switchButtonData) {
        sm_error("malloc SwitchButtonData error\n");
    }
    memset((void *) listItemSwitchData->switchButtonData, 0,
            sizeof(SwitchButtonData));

    listItemSwitchData->desLogfont = getLogFont(ID_FONT_TIMES_24);
    if (index == 0) {
        listItemSwitchData->desText = "08:00";
    } else if (index == 1) {
        listItemSwitchData->desText = "09:00";
    } else if (index == 2) {
        listItemSwitchData->desText = "10:00";
    } else {
        listItemSwitchData->clickDisable = 1;
        listItemSwitchData->desLogfont = getLogFont(ID_FONT_TIMES_22);
        listItemSwitchData->desText = getSettingDesText(index + 1);
    }

    listItemSwitchData->switchButtonData->buttonStatus = flag;
    setCurrentIconValue(ID_SWITCH_BUTTON_BMP, 0);
    getResBmp(ID_SWITCH_BUTTON_BMP, BMPTYPE_BASE,
            &listItemSwitchData->switchButtonData->bgBmp);

    setCurrentIconValue(ID_SWITCH_BUTTON_BMP, 1);
    getResBmp(ID_SWITCH_BUTTON_BMP, BMPTYPE_BASE,
            &listItemSwitchData->switchButtonData->onBmp);

    setCurrentIconValue(ID_SWITCH_BUTTON_BMP, 2);
    getResBmp(ID_SWITCH_BUTTON_BMP, BMPTYPE_BASE,
            &listItemSwitchData->switchButtonData->offBmp);

    return listItemSwitchData;
}

/*
 * @param displayMode 0 is horizontal, 1 is vertical
 */
SelectButtonData *SelectButtonDataInit(SelectButtonData *selectButtonData,
        Uint32 displayMode, Uint32 index) {
    selectButtonData = (SelectButtonData *) malloc(sizeof(SelectButtonData));
    if (NULL == selectButtonData) {
        sm_error("malloc SelectButtonData error\n");
    }
    memset((void *) selectButtonData, 0, sizeof(SelectButtonData));
    selectButtonData->displayMode = displayMode;

    switch (index) {
    case 0:
        /* Password */
        selectButtonData->listSize = 10;
        selectButtonData->listText[0] = "0";
        selectButtonData->listText[1] = "1";
        selectButtonData->listText[2] = "2";
        selectButtonData->listText[3] = "3";
        selectButtonData->listText[4] = "4";
        selectButtonData->listText[5] = "5";
        selectButtonData->listText[6] = "6";
        selectButtonData->listText[7] = "7";
        selectButtonData->listText[8] = "8";
        selectButtonData->listText[9] = "9";
        break;
    case 1:
        /* Language */
        if (languageType == Language_EN)
            selectButtonData->curIndex = 1;
        selectButtonData->listSize = 2;
        selectButtonData->listText[0] = getSettingDesText(18);
        selectButtonData->listText[1] = getSettingDesText(19);
        break;
    case 2:
        /* Automatic screen off time */
        selectButtonData->curIndex = 3;
        selectButtonData->listSize = 12;
        selectButtonData->listText[0] = "15s";
        selectButtonData->listText[1] = "20s";
        selectButtonData->listText[2] = "25s";
        selectButtonData->listText[3] = "30s";
        selectButtonData->listText[4] = "35s";
        selectButtonData->listText[5] = "40s";
        selectButtonData->listText[6] = "45s";
        selectButtonData->listText[7] = "50s";
        selectButtonData->listText[8] = "60s";
        selectButtonData->listText[9] = "65s";
        selectButtonData->listText[10] = "70s";
        selectButtonData->listText[11] = "75s";
        break;
    case 3:
        /* Default volume */
        selectButtonData->curIndex = 6;
        selectButtonData->listSize = 12;
        selectButtonData->listText[0] = "0";
        selectButtonData->listText[1] = "5";
        selectButtonData->listText[2] = "10";
        selectButtonData->listText[3] = "15";
        selectButtonData->listText[4] = "20";
        selectButtonData->listText[5] = "25";
        selectButtonData->listText[6] = "30";
        selectButtonData->listText[7] = "35";
        selectButtonData->listText[8] = "40";
        selectButtonData->listText[9] = "45";
        selectButtonData->listText[10] = "50";
        selectButtonData->listText[11] = "55";
        break;
    case 4:
        /* Play time */
        selectButtonData->listSize = 10;
        selectButtonData->listText[0] = "0";
        selectButtonData->listText[1] = "1";
        selectButtonData->listText[2] = "2";
        selectButtonData->listText[3] = "3";
        selectButtonData->listText[4] = "4";
        selectButtonData->listText[5] = "5";
        selectButtonData->listText[6] = "6";
        selectButtonData->listText[7] = "7";
        selectButtonData->listText[8] = "8";
        selectButtonData->listText[9] = "9";
        break;
    case 11:
        /* Years */
        selectButtonData->listSize = 12;
        selectButtonData->listText[0] = "2019";
        selectButtonData->listText[1] = "2020";
        selectButtonData->listText[2] = "2021";
        selectButtonData->listText[3] = "2022";
        selectButtonData->listText[4] = "2023";
        selectButtonData->listText[5] = "2024";
        selectButtonData->listText[6] = "2025";
        selectButtonData->listText[7] = "2026";
        selectButtonData->listText[8] = "2027";
        selectButtonData->listText[9] = "2028";
        selectButtonData->listText[10] = "2029";
        selectButtonData->listText[11] = "2030";
        break;
    case 12:
        /* Month */
    case 13:
        /* Date */
    case 14:
        /* Hours */
    case 15:
        /* Minutes */
        selectButtonData->listSize = 12;
        selectButtonData->listText[0] = "1";
        selectButtonData->listText[1] = "2";
        selectButtonData->listText[2] = "3";
        selectButtonData->listText[3] = "4";
        selectButtonData->listText[4] = "5";
        selectButtonData->listText[5] = "6";
        selectButtonData->listText[6] = "7";
        selectButtonData->listText[7] = "8";
        selectButtonData->listText[8] = "9";
        selectButtonData->listText[9] = "10";
        selectButtonData->listText[10] = "11";
        selectButtonData->listText[11] = "12";
        break;
    default:
        break;
    }

    int i;

    if (displayMode == SELECT_BUTTON_VERTICAL) {
        selectButtonData->textLogfont = getLogFont(ID_FONT_TIMES_40);

        setCurrentIconValue(ID_SELECT_BUTTON_BMP, 0);
        getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE, &selectButtonData->bgBmp);

        for (i = 0; i < 2; i++) {
            selectButtonData->buttonData[i].doubleClickDisable = 1;

            setCurrentIconValue(ID_SELECT_BUTTON_BMP, 1 + i);
            getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE,
                    &selectButtonData->buttonData[i].bmpNormal);

            setCurrentIconValue(ID_SELECT_BUTTON_BMP, 3 + i);
            getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE,
                    &selectButtonData->buttonData[i].bmpSelect);
        }
    } else {
        selectButtonData->textLogfont = getLogFont(ID_FONT_TIMES_18);

        setCurrentIconValue(ID_SELECT_BUTTON_BMP, 5);
        getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE, &selectButtonData->bgBmp);

        for (i = 0; i < 2; i++) {
            selectButtonData->buttonData[i].doubleClickDisable = 1;

            setCurrentIconValue(ID_SELECT_BUTTON_BMP, 6 + i);
            getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE,
                    &selectButtonData->buttonData[i].bmpNormal);

            setCurrentIconValue(ID_SELECT_BUTTON_BMP, 8 + i);
            getResBmp(ID_SELECT_BUTTON_BMP, BMPTYPE_BASE,
                    &selectButtonData->buttonData[i].bmpSelect);
        }
    }
    return selectButtonData;
}

ListItemSelectData *ListItemSelectDataInit(
        ListItemSelectData *listItemSelectData, Uint32 index) {
    listItemSelectData = (ListItemSelectData *) malloc(
            sizeof(ListItemSelectData));
    if (NULL == listItemSelectData) {
        sm_error("malloc ListItemSelectData error\n");
    }
    memset((void *) listItemSelectData, 0, sizeof(ListItemSelectData));

    listItemSelectData->desLogfont = getLogFont(ID_FONT_TIMES_22);
    listItemSelectData->desText = getSettingDesText(index + 1);

    listItemSelectData->selectButtonData = SelectButtonDataInit(
            listItemSelectData->selectButtonData,
            SELECT_BUTTON_HORIZONTAL, index + 1);

    return listItemSelectData;
}

ListItemArrowData *ListItemArrowDataInit(ListItemArrowData *listItemArrowData,
        Uint32 index) {
    listItemArrowData = (ListItemArrowData *) malloc(sizeof(ListItemArrowData));
    if (NULL == listItemArrowData) {
        sm_error("malloc ListItemArrowData error\n");
    }
    memset((void *) listItemArrowData, 0, sizeof(ListItemArrowData));

    listItemArrowData->desLogfont = getLogFont(ID_FONT_TIMES_22);
    listItemArrowData->desText = getSettingDesText(index + 1);

    setCurrentIconValue(ID_SETTING_BUTTON_BMP, 2);
    getResBmp(ID_SETTING_BUTTON_BMP, BMPTYPE_BASE,
            &listItemArrowData->arrowBmp);

    return listItemArrowData;
}

ListItemMusicData *ListItemMusicDataInit(ListItemMusicData *listItemMusicData,
        Uint32 index) {
    listItemMusicData = (ListItemMusicData *) malloc(sizeof(ListItemMusicData));
    if (NULL == listItemMusicData) {
        sm_error("malloc ListItemMusicData error\n");
    }
    memset((void *) listItemMusicData, 0, sizeof(ListItemMusicData));

    listItemMusicData->desLogfont = getLogFont(ID_FONT_TIMES_22);
    if (index > 15)
        index -= 16;
    listItemMusicData->musicText = getMusicText(index);

    int i;
    for (i = 0; i < 4; i++) {
        setCurrentIconValue(ID_MUSIC_PLAYING_BMP, i + 4);
        getResBmp(ID_MUSIC_PLAYING_BMP, BMPTYPE_BASE,
                &listItemMusicData->musicPlayingBmp[i]);
    }

    return listItemMusicData;
}

/*
 * @param index 0 is setting admini, 1 is setting time, 2 is music list
 */
MyListData *MyListDataInit(MyListData *myListData, Uint32 index) {
    myListData = (MyListData *) malloc(sizeof(MyListData));
    if (NULL == myListData) {
        sm_error("malloc MyListData error\n");
    }
    memset((void *) myListData, 0, sizeof(MyListData));
    myListData->itemHeight = 68;

    smRect rect;
    getResRect(ID_SCREEN, &rect);
    if (rect.w > 240)
        myListData->itemHeight = 88;

    if (index == 0) {
        myListData->listSize = 10;

        int i;
        for (i = 0; i < 4; i++) {
            myListData->itemType[i] = LIST_ITEM_SELECT;
            myListData->itemData[i] = ListItemSelectDataInit(
                    myListData->itemData[i], i);
        }

        for (i = 4; i < 7; i++) {
            myListData->itemType[i] = LIST_ITEM_SWITCH;
            if (i < 5)
                myListData->itemData[i] = ListItemSwitchDataInit(
                        myListData->itemData[i], 0, i);
            else
                myListData->itemData[i] = ListItemSwitchDataInit(
                        myListData->itemData[i], 1, i);
        }

        for (i = 7; i < 10; i++) {
            myListData->itemType[i] = LIST_ITEM_ARROW;
            myListData->itemData[i] = ListItemArrowDataInit(
                    myListData->itemData[i], i);
        }
    } else if (index == 1) {
        myListData->listSize = 5;

        int i;
        for (i = 0; i < 5; i++) {
            myListData->itemType[i] = LIST_ITEM_SELECT;
            myListData->itemData[i] = ListItemSelectDataInit(
                    myListData->itemData[i], i + 10);
        }
    } else {
        myListData->listSize = 32;

        int i;
        for (i = 0; i < 32; i++) {
            myListData->itemType[i] = LIST_ITEM_MUSIC;
            myListData->itemData[i] = ListItemMusicDataInit(
                    myListData->itemData[i], i);
        }
    }

    return myListData;
}
