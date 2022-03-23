/*
 * datafactory.h
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

#ifndef SRC_MIDDLE_WARE_DATAFACTORY_H_
#define SRC_MIDDLE_WARE_DATAFACTORY_H_

#include "time_title_view.h"
#include "desk_button_view.h"
#include "headbar_view.h"
#include "control_button_view.h"
#include "control_buttons_view.h"
#include "progress_view.h"
#include "setting_button_view.h"
#include "list_item_switch.h"
#include "select_button_view.h"
#include "list_item_select.h"
#include "list_view.h"
#include "list_item_arrow.h"
#include "list_item_music.h"

TimeTitleData *TimeTitleDataInit(TimeTitleData *timeTitleData, Uint32 flag);
DeskButotnData *DeskButotnDataInit(DeskButotnData *deskButotnData, Uint32 flag);
HeadbarData *HeadbarDataInit(HeadbarData *headbarData, Uint32 flag,
        Uint32 index);
ControlButotnData *ControlButotnDataInit(ControlButotnData *controlButotnData,
        Uint32 index);
ControlButotnsData *ControlButotnsDataInit(
        ControlButotnsData *controlButotnsData);
ProgressData *ProgressDataInit(ProgressData *progressData);
SettingButtonData *SettingButtonDataInit(SettingButtonData *settingButtonData,
        Uint32 flag);
ButtonData *NormalButtonDataInit(ButtonData *buttonData, Uint32 index);
ListItemSwitchData *ListItemSwitchDataInit(
        ListItemSwitchData *listItemSwitchData, Uint32 flag, Uint32 index);
SelectButtonData *SelectButtonDataInit(SelectButtonData *selectButtonData,
        Uint32 displayMode, Uint32 index);
ListItemSelectData *ListItemSelectDataInit(
        ListItemSelectData *listItemSelectData, Uint32 index);
ListItemArrowData *ListItemArrowDataInit(ListItemArrowData *listItemArrowData,
        Uint32 index);
ListItemMusicData *ListItemMusicDataInit(ListItemMusicData *listItemMusicData,
        Uint32 index);
MyListData *MyListDataInit(MyListData *myListData, Uint32 index);

#endif /* SRC_MIDDLE_WARE_DATAFACTORY_H_ */
