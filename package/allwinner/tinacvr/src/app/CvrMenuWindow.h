#ifndef __CVR_MENU_WINDOW_H__
#define __CVR_MENU_WINDOW_H__

#include "CvrInclude.h"
#include "CvrMainWindow.h"
#include "CvrResource.h"
#include "CvrMenuList.h"
#include "TipLabel.h"
#include "common.h"
#include "MessageBox.h"
#include <minigui/window.h>

typedef struct
{
	HWND ParentHwnd;
	HWND menuHwnd;
    HWND menulistHwnd;
	CvrRectType menuSize;
}MenuWinDataType;

#define IDC_MENULIST			400
#define MENU_LIST_COUNT  (23-9)		// remove not use item
#define MAX_VALUE_COUNT	3

typedef struct
{
  bool isModified;
}isModified_t;

BITMAP *itemImageArray;
BITMAP *value1ImageArray;
BITMAP unfoldImageLight;
BITMAP unfoldImageDark;

MenuWinDataType* GetMenuWinData(void);
HWND GetMenuWinHwnd(void);

HWND MenuWinInit(HWND hParent);
int MenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

int createSubWidgets(HWND hWnd);
int createMenuListContents(HWND hWnd);

int ShowSubMenu(unsigned int menuIndex, isModified_t *Modified);
int getSubMenuData(unsigned subMenuIndex, subMenuData_t *subMenuData);

int getMenuListAttr(menuListAttr_t *attr);

int updateMenuTexts(void);
int updateNewSelected(int oldSel, int newSel);
int updateSwitchIcons(void);

int HandleSubMenuChange(unsigned int menuIndex, int newSel);
int HandleSubMenuChange2(unsigned int menuIndex, bool newValue);

int getCheckBoxStatus(unsigned int menuIndex);
int getCheckBoxStatus(unsigned int menuIndex);

int getMessageBoxData(unsigned int menuIndex, MessageBox_t *messageBoxData);
int ShowMessageBox( HWND mHwnd, unsigned int menuIndex,int val);

static void formatCallback(HWND hDlg, void* data);
int ShowFormattingTip(HWND mHwnd);

#endif
