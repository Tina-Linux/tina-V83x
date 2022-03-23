#include"CvrMenuWindow.h"
#include"CvrRecordWindow.h"
#include "DateEdit.h"

MenuWinDataType *menuWinData=NULL;
menuListAttr_t  *menuListAttr=NULL;

#define MB_HAVE_TITLE		0x00000002
#define MB_HAVE_TEXT		0x00000004
#define IDC_BUTTON_OK		100
#define IDC_BUTTON_START	100
#define IDC_BUTTON_CANCEL	101
#define TIME_INFINITE 0xFFFFFF

typedef struct
{
	CvrRectType rect;
	unsigned int titleHeight;
	const char* title;
	const char* text;
	PLOGFONT pLogFont;
	gal_pixel bgc_widget;
	gal_pixel fgc_widget;
	gal_pixel linec_title;
	bool disableKeyEvent;
	void (*callback)(HWND hText, void* data);
	void* callbackData;
	bool endLabelKeyUp;
	unsigned int timeoutMs;
}tipLabelData_t;

enum ResourceID ResourceIDByIndex[MENU_LIST_COUNT] =
{
	//ID_MENU_LIST_PARK,
	//ID_MENU_LIST_AWMD,
	ID_MENU_LIST_POR,
	ID_MENU_LIST_SILENTMODE,
	ID_MENU_LIST_TWM,
	//ID_MENU_LIST_ALIGNLINE,
	ID_MENU_LIST_VQ,
	ID_MENU_LIST_PQ,
	ID_MENU_LIST_VTL,
	//ID_MENU_LIST_GSENSOR,
	ID_MENU_LIST_IMPACTLEVEL,
	//ID_MENU_LIST_FCWSENSITY,
	ID_MENU_LIST_VOICEVOL,
	//ID_MENU_LIST_WB,
	//ID_MENU_LIST_CONTRAST,
	//ID_MENU_LIST_EXPOSURE,
	ID_MENU_LIST_SS,
	ID_MENU_LIST_LANG,
	//ID_MENU_LIST_SHUTDOWN,
	ID_MENU_LIST_FIRMWARE,
	ID_MENU_LIST_FORMAT,
	ID_MENU_LIST_FRESET,
	ID_MENU_LIST_DATE
};

MLFlags mlFlags[MENU_LIST_COUNT] =
{
	/*PARK*/
	//{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},

	/*AWMD*/
	//{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},

	/*POR*/
	{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},

	/*SILENTMODE*/
	{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},

	/*WTM*/

	{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},
    /*ALIGNLINE*/

    //{ IMGFLAG_IMAGE, {VMFLAG_IMAGE}, 1},

    /*VQ*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*PQ*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*VTL*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*GSENSOR*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*IMPACTLEVEL*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*FCWSENSITY*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*VOICEVOL*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*WB*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*CONTRAST*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

	/*EXPOSURE*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*SS*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*LANG*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*SHUTDOWN*/
	//{ IMGFLAG_IMAGE, {VMFLAG_STRING, VMFLAG_IMAGE}, 2},

    /*FIRMWARE*/
	{ IMGFLAG_IMAGE, {VMFLAG_STRING}, 1},

    /*FORMAT*/
	{ IMGFLAG_IMAGE, {0}, 0},

	/*FRESET*/
	{ IMGFLAG_IMAGE, {0}, 0},

    /*DATE*/
	{ IMGFLAG_IMAGE, {0}, 0},

};

unsigned int HaveValueString[MENU_LIST_COUNT] =
{
	//0,	/*PARK		 not have value string */
	//0,  /*AWMD         have not value string */
	0,  /*POWER ON VIDEO         have not value string */
	0,  /*SILENTMODE        have not value string */
	0,  /*WATERMARK           have not value string */
	//0,  /*ALIGNLINE        have not value string */
	1,  /*VQ        have value string */
	1,  /*PQ        have value string */
	1,  /*VTL        have value string */
	//1,  /*GSENSOR        have value string */
	1,  /*IMPACTLEVEL        have value string */
	//1,  /*FCWSENSITY        have value string */
	1,  /*VOICEVOL        have value string */
	//1,  /*WB        have value string */
	//1,  /*CONTRAST        have value string */
	//1,  /*EXPOSURE        have value string */
	1,  /*SS        have value string */
	1,  /*LANG        have value string */
	//1,  /*SHOTDOWN        have value string */
	1,  /*FIRMWARE        have value string */
	0,  /*FORMAT        have value string */
	0,  /*FRESET        have value string */
	0,  /*DATA        have value string */
};

unsigned int HaveSubMenu[MENU_LIST_COUNT] =
{
	//0,  /*PARK         not have sub menu */
	//0,  /*AWMD         have sub menu */
	0,  /*POR         have sub menu */
	0,  /*SILENTMODE        have sub menu */
	0,  /*WTM          not have sub menu */
	//0,  /*ALIGNLINE       not have sub menu */
	1,  /*VQ       not have sub menu */
	1,  /*PQ        have sub menu */
	1,  /*VTL        have sub menu */
	//1,  /*GSENSOR         have sub menu */
	1,  /*IMPACTLEVEL       not have sub menu */
	//1,  /*FCWSENSITY       have sub menu */
	1, /*VOICEVOL         have sub menu */
	//1,  /*WB       have sub menu */
	//1,  /*CONTRAST       have sub menu */
	//1,  /*EXPOSURE        not have sub menu */
	1,  /*SS         have sub menu */
	1,  /*LANG         not have sub menu */
	//1,  /*SHUTDOWN       have sub menu */
	0,  /*FIRMWARE        not have sub menu */
	0,  /*FORMAT         not have sub menu */
	0,  /*FRESET         not have sub menu */
	0,  /*DATA         not have sub menu */
};

unsigned int HaveCheckBox[MENU_LIST_COUNT] =
{
	//1,	/*PARK         have check box */
	//1,    /*AWMD         have check box */
	1,	/*POR         have check box */
	1,	/*SILENTMODE        have check box */
	1,	/*WTM          have check box */
	//1,	/*ALIGNLINE       have check box */
	0,	/*VQ       not have check box */
	0,	/*PQ        not have check box */
	0,	/*VTL        not have check box */
	//0,	/*GSENSOR         not have check box */
	0,	/*IMPACTLEVEL       not have check box */
	//0,	/*FCWSENSITY       not have check box */
	0,	/*VOICEVOL         not have check box */
	//0,	/*WB       not have check box */
	//0,	/*CONTRAST       not have check box*/
	//0,	/*EXPOSURE        not have check box */
	0,		/*SS         not have check box */
	0,		/*LANG         not have check box */
	//0,	/*SHOTDOWN        not have check box */
	0,	/*FIRMWARE        not have check box */
	0,		/*FORMAT        not have check box */
	0,	/*FRESET        not have check box */
	0,		/*DATA        not have check box */
};

int subMenuContent0Cmd[] =
{
	//-1,	/*PARK not have sub menu */
    //-1,     /*AWMD not have sub menu */
    -1,     /*POR not have sub menu */
    -1,     /*SILENTMODE not have sub menu */
    -1,     /*WTM not have sub menu */
    //-1,     /*ALIGNLINE not have sub menu */
	LANG_LABEL_SUBMENU_VQ_CONTENT0,
	LANG_LABEL_SUBMENU_PQ_CONTENT0,
	LANG_LABEL_SUBMENU_VTL_CONTENT0,
	//LANG_LABEL_SUBMENU_GSENSOR_CONTENT0,
	LANG_LABEL_SUBMENU_IMPACTLEVEL_CONTENT0,
	//LANG_LABEL_SUBMENU_FCWSENSITY_CONTENT0,
	LANG_LABEL_SUBMENU_VOICEVOL_CONTENT0,
	//LANG_LABEL_SUBMENU_WB_CONTENT0,
	//LANG_LABEL_SUBMENU_CONTRAST_CONTENT0,
	//LANG_LABEL_SUBMENU_EXPOSURE_CONTENT0,
	LANG_LABEL_SUBMENU_SS_CONTENT0,
	LANG_LABEL_SUBMENU_LANG_CONTENT0,
	//LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT0,
	-1	/*FIRMWARE not have sub menu */
	-1,	/*FORMAT not have sub menu */
	-1,	/*FRESET not have sub menu */
	-1,	/*DATE not have sub menu */
};

int getMenuListAttr(menuListAttr_t *attr)
{
	attr->normalBgc = getResColor(ID_MENU_LIST, COLOR_BGC);
	attr->normalFgc = getResColor(ID_MENU_LIST, COLOR_FGC);
	attr->linec = getResColor(ID_MENU_LIST, COLOR_LINEC_ITEM);
	attr->normalStringc = getResColor(ID_MENU_LIST, COLOR_STRINGC_NORMAL);
	attr->normalValuec = getResColor(ID_MENU_LIST, COLOR_VALUEC_NORMAL);
	attr->selectedStringc = getResColor(ID_MENU_LIST, COLOR_STRINGC_SELECTED);
	attr->selectedValuec = getResColor(ID_MENU_LIST, COLOR_VALUEC_SELECTED);
	attr->scrollbarc = getResColor(ID_MENU_LIST, COLOR_SCROLLBARC);
	attr->itemHeight = 56;

	return 0;
}

int getItemStrings(MENULISTITEMINFO* mlii)
{
	for(unsigned int i = 0; i < MENU_LIST_COUNT; i++)
	{
		mlii[i].string = getResMenuItemString(ResourceIDByIndex[i]);
		if(mlii[i].string == NULL)
		{
			cvr_error("get the %d label string failed\n", i);
			return -1;
		}
	}

	return 0;
}

int getItemImages(MENULISTITEMINFO *mlii)
{
	int retval;

	for(unsigned int i = 0; i < MENU_LIST_COUNT; i++)
	{
		retval = getResBmp(ResourceIDByIndex[i], BMPTYPE_UNSELECTED, &itemImageArray[i] );
		if(retval < 0)
		{
			cvr_error("get bmp failed, i is %d, resID is %d\n", i, ResourceIDByIndex[i]);
			return -1;
		}

		mlii[i].hBmpImage = (DWORD)&itemImageArray[i];
	}
	return 0;
}

int getFirstValueStrings(MENULISTITEMINFO* mlii, int menuIndex)
{
	const char* ptr;

	if(menuIndex == -1)
	{
		for(unsigned int i = 0; i < MENU_LIST_COUNT ; i++)
		{
			if(HaveValueString[i] == 1)
			{
				ptr = getResSubMenuCurString(ResourceIDByIndex[i]);
				if(ptr == NULL)
				{
					cvr_error("get ResSubMenuString %d failed\n", i);
					return -1;
				}
				mlii[ i ].hValue[0] = (DWORD)ptr;
			}
		}
	}
	else if(menuIndex >=0 && menuIndex < MENU_LIST_COUNT )
	{
		if(HaveSubMenu[menuIndex] == 1)
		{
			ptr = getResSubMenuCurString(ResourceIDByIndex[menuIndex]);
			if(ptr == NULL)
			{
				cvr_error("get ResSubMenuString %d failed\n", menuIndex);
				return -1;
			}
			mlii->hValue[0] = (DWORD)ptr;
		}
	}

	return 0;
}

int getFirstValueImages(MENULISTITEMINFO* mlii, int menuIndex)
{
	int retval;
	int current;
	HWND hMenuList;

	if(menuIndex == -1)
	{
		for(unsigned int i = 0; i < MENU_LIST_COUNT; i++)
		{
			if( HaveCheckBox[i] == 1)
			{
				retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[i], 0, &value1ImageArray[i] );
				if(retval < 0)
				{
					cvr_error("get first value images failed, i is %d\n", i);
					return -1;
				}
				mlii[i].hValue[0] = (DWORD)&value1ImageArray[i];
                //cvr_debug("mlii[i].hValue[0] =%x\n",mlii[i].hValue[0]);
			}
		}
	}
	else if(menuIndex >= 0 && menuIndex < MENU_LIST_COUNT )
	{
		if(menuWinData->menuHwnd && menuWinData->menuHwnd != HWND_INVALID)
		{
			hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
			current = SendMessage(hMenuList, LB_GETCURSEL, 0, 0);
			unloadBitMap(&value1ImageArray[menuIndex]);
			if(current == menuIndex)
			{
				retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[menuIndex], 1, &value1ImageArray[menuIndex] );
			}
			else
			{
				retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[menuIndex], 0, &value1ImageArray[menuIndex] );
			}

			if(retval < 0)
			{
				cvr_error("get first value images failed, menuIndex is %d\n", menuIndex);
				return -1;
			}
		}
	}

	return 0;
}

int getSecondValueImages(MENULISTITEMINFO* mlii, MLFlags *mlFlags)
{
	int retval;

	retval = getResBmp(ID_MENU_LIST_UNFOLD_PIC, BMPTYPE_UNSELECTED, &unfoldImageLight);
	if(retval < 0)
	{
		cvr_error("get secondValue image failed\n");
		return -1;
	}

	retval = getResBmp(ID_MENU_LIST_UNFOLD_PIC, BMPTYPE_SELECTED, &unfoldImageDark);
	if(retval < 0)
	{
		cvr_error("get secondValue image failed\n");
		return -1;
	}

	for(unsigned int i = 0; i < MENU_LIST_COUNT; i++)
	{
		if(mlFlags[i].valueCount == 2)
		{
			mlii[i].hValue[1] = (DWORD)&unfoldImageLight;
	    }
	}
	cvr_debug("unfoldImageLight.bmWidth =%d\n",unfoldImageLight.bmWidth);
    cvr_debug("unfoldImageLight.bmHeight =%d\n",unfoldImageLight.bmHeight);

	return 0;
}

int createMenuListContents(HWND hMenuList)
{
    int itemCount;
    unsigned int selectedItem;

	MENULISTITEMINFO menuListII[MENU_LIST_COUNT];

	memset(&menuListII, 0, sizeof(menuListII));

	if(getItemStrings(menuListII) < 0)
	{
		cvr_error("get item strings failed\n");
		return -1;
	}
    if(getItemImages(menuListII) < 0)
	{
		cvr_error("get item images failed\n");
		return -1;
	}
	if(getFirstValueImages(menuListII, -1) < 0)
	{
		cvr_error("get first value images failed\n");
		return -1;
	}
    if(getFirstValueStrings(menuListII, -1) < 0)
	{
		cvr_error("get first value strings failed\n");
		return -1;
	}
    if(getSecondValueImages(menuListII, mlFlags) < 0)
	{
		cvr_error("get second value images failed\n");
		return -1;
	}

    for(unsigned int i = 0; i < MENU_LIST_COUNT; i++)
	{
		memcpy( (void*)&menuListII[i].flagsEX, (void*)&mlFlags[i], sizeof(MLFlags) );
	}

	SendMessage(hMenuList, LB_MULTIADDITEM, MENU_LIST_COUNT, (LPARAM)menuListII);
	SendMessage(hMenuList, LB_SETITEMHEIGHT,0,35);
	#ifdef USE_IPS_SCREEN
    SendMessage(hMenuList, LB_SETITEMHEIGHT,0,56);
	#endif
	SendMessage(hMenuList, MSG_MLM_HILIGHTED_SPACE, 8, 8);
	cvr_debug("height is %d\n", SendMessage(hMenuList, LB_GETITEMHEIGHT, 0, 0));
    itemCount = SendMessage(hMenuList, LB_GETCOUNT, 0, 0);
    cvr_debug("itemCount =%d\n",itemCount);
    selectedItem = SendMessage(hMenuList, LB_GETCURSEL, 0, 0);
    cvr_debug("selectedItem =%d\n",selectedItem);
    SendMessage(hMenuList, LB_SETCURSEL, 0, 0);

	return 0;
}

int getSubMenuData(unsigned int subMenuIndex, subMenuData_t *subMenuData)
{
	int retval;
	int contentCount;
	const char* ptr;

	if(HaveSubMenu[subMenuIndex] != 1)
	{
		cvr_error("invalid index %d\n", subMenuIndex);
		return -1;
	}

	retval = getResRect(ID_SUBMENU, &subMenuData->rect);
	if(retval < 0)
	{
		cvr_error("get subMenu rect failed\n");
		return -1;
	}

	retval = getResBmp(ID_SUBMENU_CHOICE_PIC, BMPTYPE_BASE, &subMenuData->BmpChoice);
	if(retval < 0)
	{
		cvr_error("get subMenu choice bmp failed\n");
		return -1;
	}

	//getMenuListAttr(&subMenuData->menuListAttr);
	subMenuData->menuListAttr.normalBgc = getResColor(ID_MENU_LIST, COLOR_SUB_BGC);
	subMenuData->menuListAttr.normalFgc = getResColor(ID_MENU_LIST, COLOR_FGC);
	subMenuData->menuListAttr.linec = getResColor(ID_MENU_LIST, COLOR_LINEC_ITEM);
	subMenuData->menuListAttr.normalStringc = getResColor(ID_MENU_LIST, COLOR_STRINGC_NORMAL);
	subMenuData->menuListAttr.normalValuec = getResColor(ID_MENU_LIST, COLOR_VALUEC_NORMAL);
	subMenuData->menuListAttr.selectedStringc = getResColor(ID_MENU_LIST, COLOR_STRINGC_SELECTED);
	subMenuData->menuListAttr.selectedValuec = getResColor(ID_MENU_LIST, COLOR_VALUEC_SELECTED);
	subMenuData->menuListAttr.scrollbarc = getResColor(ID_MENU_LIST, COLOR_SCROLLBARC);
	subMenuData->menuListAttr.itemHeight = 56;

	subMenuData->lincTitle = getResColor(ID_SUBMENU, COLOR_LINEC_TITLE);
	subMenuData->pLogFont = getLogFont();

	retval = getResIntValue(ResourceIDByIndex[subMenuIndex], INTVAL_SUBMENU_INDEX );
	if(retval < 0)
	{
		cvr_error("get res submenu index failed\n");
		return -1;
	}
	subMenuData->selectedIndex = retval;

	contentCount = getResIntValue(ResourceIDByIndex[subMenuIndex], INTVAL_SUBMENU_COUNT);
    subMenuData->contents.count = contentCount;
    cvr_debug("contentCount =%d\n",contentCount);
	ptr = getResSubMenuTitle(ResourceIDByIndex[subMenuIndex]);
	if(ptr == NULL)
	{
		cvr_error("get video quality title failed\n");
		return -1;
	}
	subMenuData->title = ptr;

	for(int i = 0; i < contentCount; i++)
	{
		ptr = getLabel(subMenuContent0Cmd[subMenuIndex] + i);
		if(ptr == NULL)
		{
			cvr_error("get video quality content %d failed", i);
			return -1;
		}
		subMenuData->contents.content[i] = ptr;
	}

	return 0;
}

int ShowSubMenu(unsigned int menuIndex, isModified_t *Modified)
{
	int retval;
	int oldSel;
    int count,i;
    HWND hMenuList;
	subMenuData_t subMenuData;

	if(getSubMenuData(menuIndex, &subMenuData) < 0)
	{
		cvr_error("get submenu data failed\n");
		return -1;
	}

	oldSel = subMenuData.selectedIndex;
    cvr_debug("oldSel =%d\n",oldSel);
	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
	retval = showSubMenu(hMenuList, &subMenuData);
	cvr_debug("retval = %d\n", retval);

	if(retval >= 0)
	{
		if(retval != oldSel)
		{
			cvr_debug("change from %d to %d\n", oldSel, retval);
			Modified->isModified = 1;
		}
	}

	return retval;
}

int updateMenuTexts(void)
{
	int itemCount;
	HWND hMenuList;
	MENULISTITEMINFO menuListII[MENU_LIST_COUNT];

	hMenuList = GetDlgItem(GetMenuWinHwnd(), IDC_MENULIST);
	itemCount = SendMessage(hMenuList, LB_GETCOUNT, 0, 0);
	if(itemCount < 0)
	{
		cvr_error("get menu counts failed\n");
		return -1;
	}
    cvr_debug("itemCount =%d\n",itemCount);

	memset(menuListII, 0, sizeof(menuListII));
	for(int count = 0; count < itemCount; count++)
	{
		if( SendMessage(hMenuList, LB_GETITEMDATA, count, (LPARAM)&menuListII[count]) != LB_OKAY)
		{
			cvr_error("get the %d item data failed\n", count);
			return -1;
		}
	}

	if(getItemStrings(menuListII) < 0)
	{
		cvr_error("get item strings failed\n");
		return -1;
	}
	if(getFirstValueStrings(menuListII, -1) < 0)
	{
		cvr_error("get item value strings failed\n");
		return -1;
	}

	for(int count = 0; count < itemCount; count++)
	{
		SendMessage(hMenuList, LB_SETITEMDATA, count, (LPARAM)&menuListII[count] );
	}

	return 0;
}

int updateNewSelected(int oldSel, int newSel)
{
	HWND hMenuList;
	MENULISTITEMINFO mlii;
	int retval;

	if(oldSel >= MENU_LIST_COUNT || newSel >= MENU_LIST_COUNT)
	{
		return -1;
	}

	if(oldSel == newSel)
	{
		return 0;
	}

	if(menuWinData->menuHwnd == 0 || menuWinData->menuHwnd == HWND_INVALID)
	{
		return -1;
	}

	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
	if(oldSel >= 0)
	{
		if(SendMessage(hMenuList, LB_GETITEMDATA, oldSel, (LPARAM)&mlii) != LB_OKAY)
		{
			cvr_error("get item info failed\n");
			return -1;
		}
		unloadBitMap(&itemImageArray[oldSel]);
		retval = getResBmp(ResourceIDByIndex[oldSel] ,BMPTYPE_UNSELECTED, &itemImageArray[oldSel]);
		if(retval < 0)
		{
			cvr_error("get item image failed, oldSel is %d\n", oldSel);
			return -1;
		}
		mlii.hBmpImage = (DWORD)&itemImageArray[oldSel];

		if(HaveCheckBox[oldSel] == 1)
		{
			unloadBitMap(&value1ImageArray[oldSel]);
			retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[oldSel], 0, &value1ImageArray[oldSel]);
			if(retval < 0)
			{
				cvr_error("get resBmp SubMenu Check box failed, oldSel is %d\n", oldSel);
				return -1;
			}
			mlii.hValue[0] = (DWORD)&value1ImageArray[oldSel];
		}
		else if(mlii.flagsEX.valueCount == 2)
		{
			mlii.hValue[1] = (DWORD)&unfoldImageLight;
		}

		SendMessage(hMenuList, LB_SETITEMDATA, oldSel, (LPARAM)&mlii);
	}

	if(newSel >= 0)
	{
		if(SendMessage(hMenuList, LB_GETITEMDATA, newSel, (LPARAM)&mlii) != LB_OKAY)
		{
			cvr_error("get item info failed\n");
			return -1;
		}
		unloadBitMap(&itemImageArray[newSel]);
		retval = getResBmp(ResourceIDByIndex[newSel],BMPTYPE_SELECTED, &itemImageArray[newSel]);
		if(retval < 0)
		{
			cvr_error("get item image failed, oldSel is %d\n", oldSel);
			return -1;
		}
		mlii.hBmpImage = (DWORD)&itemImageArray[newSel];

		if(HaveCheckBox[newSel] == 1)
		{
			unloadBitMap(&value1ImageArray[newSel]);
			retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[newSel], 1, &value1ImageArray[newSel]);
			if(retval < 0)
			{
				cvr_error("get resBmp SubMenu Check box failed, oldSel is %d\n", oldSel);
				return -1;
			}
			mlii.hValue[0] = (DWORD)&value1ImageArray[newSel];
		}
		else if(mlii.flagsEX.valueCount == 2)
		{
			mlii.hValue[1] = (DWORD)&unfoldImageDark;
		}

		SendMessage(hMenuList, LB_SETITEMDATA, newSel, (LPARAM)&mlii);
	}

	return 0;
}

int updateSwitchIcons(void)
{
	int itemCount;
	HWND hMenuList;
	MENULISTITEMINFO menuListII[MENU_LIST_COUNT];

	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
	itemCount = SendMessage(hMenuList, LB_GETCOUNT, 0, 0);
	if(itemCount < 0)
	{
		cvr_error("get menu counts failed\n");
		return -1;
	}

	memset(menuListII, 0, sizeof(menuListII));
	for(int count = 0; count < itemCount; count++)
	{
		if( SendMessage(hMenuList, LB_GETITEMDATA, count, (LPARAM)&menuListII[count]) != LB_OKAY)
		{
			cvr_debug("get the %d item data failed\n", count);
			return -1;
		}
	}

	getFirstValueImages(menuListII, -1);
	for(int count = 0; count < itemCount; count++)
	{
		SendMessage(hMenuList, LB_SETITEMDATA, count, (LPARAM)&menuListII[count] );
	}

	return 0;
}

int HandleSubMenuChange(unsigned int menuIndex, int newSel)
{
	HWND hMenuList;
	MENULISTITEMINFO mlii;

	switch(menuIndex)
	{
		case MENU_INDEX_LANG:
		case MENU_INDEX_VQ:
		case MENU_INDEX_PQ:
		case MENU_INDEX_VTL:
		//case MENU_INDEX_WB:
		//case MENU_INDEX_CONTRAST:
		//case MENU_INDEX_EXPOSURE:
		case MENU_INDEX_SS:
		//case MENU_INDEX_GSENSOR:
		case MENU_INDEX_VOICEVOL:
		//case MENU_INDEX_SHUTDOWN:
		case MENU_INDEX_IMPACTLEVEL:
		//case MENU_INDEX_FCWSENSITY:
			if(HaveSubMenu[menuIndex] != 1)
			{
				cvr_error("invalid menuIndex %d\n", menuIndex);
				return -1;
			}
			if(setResIntValue(ResourceIDByIndex[menuIndex], INTVAL_SUBMENU_INDEX, newSel) < 0)
			{
				cvr_error("set %d to %d failed\n", menuIndex, newSel);
				return -1;
			}
			break;
		default:
			cvr_error("invalid menuIndex %d\n", menuIndex);
			return -1;
	}

	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
	if(menuIndex != MENU_INDEX_LANG)
	{
		if(SendMessage(hMenuList, LB_GETITEMDATA, menuIndex, (LPARAM)&mlii) != LB_OKAY)
		{
			cvr_error("get item info failed, menuIndex is %d\n", menuIndex);
			return -1;
		}

		if(getFirstValueStrings(&mlii, menuIndex) < 0)
		{
			cvr_error("get first value strings failed\n");
			return -1;
		}

		SendMessage(hMenuList, LB_SETITEMDATA, menuIndex, (LPARAM)&mlii);
	}

	return 0;
}

int HandleSubMenuChange2(unsigned int menuIndex, bool newValue)
{
	HWND hMenuList;
	MENULISTITEMINFO mlii;

	switch(menuIndex)
	{
		case MENU_INDEX_POR:
		case MENU_INDEX_SILENTMODE:
		case MENU_INDEX_TWM:
		//case MENU_INDEX_AWMD:
		//case MENU_INDEX_PARK:
		//case MENU_INDEX_ALIGNLINE:
			if(HaveCheckBox[menuIndex] != 1)
			{
				cvr_error("invalid menuIndex %d\n", menuIndex);
				return -1;
			}
			if(setResBoolValue(ResourceIDByIndex[menuIndex], newValue) < 0)
			{
				cvr_error("set %d to %d failed\n", menuIndex, newValue);
				return -1;
			}
			break;
		default:
			cvr_error("invalid menuIndex %d\n", menuIndex);
			return -1;
	}

	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
	memset(&mlii, 0, sizeof(mlii));
	if(SendMessage(hMenuList, LB_GETITEMDATA, menuIndex, (LPARAM)&mlii) != LB_OKAY)
	{
		cvr_error("get item info failed, menuIndex is %d\n", menuIndex);
		return -1;
	}

	if(getFirstValueImages(&mlii, menuIndex) < 0)
	{
		cvr_error("get first value images failed, menuIndex is %d\n", menuIndex);
		return -1;
	}
	SendMessage(hMenuList, LB_SETITEMDATA, menuIndex, (LPARAM)&mlii);

	return 0;
}

int getCheckBoxStatus(unsigned int menuIndex)
{
	bool curStatus;
    bool isChecked;

	if(menuIndex >= MENU_LIST_COUNT )
	{
		return -1;
	}

	if(HaveCheckBox[menuIndex] != 1)
	{
		cvr_error("menuIndex: %d, don not have checkbox\n", menuIndex);
		return -1;
	}

	curStatus = getResBoolValue(ResourceIDByIndex[menuIndex]);
	isChecked = ( (curStatus == 1) ? 0 : 1 );
	return isChecked;
}

static void FormatDialogCallback(HWND hDlg, void* data)
{
	CloseMessageBox();

	if(sdcard_is_exist() == CVR_FALSE)
	{
		CloseTipLabel();
		ShowTipLabel(GetRecordWinHwnd(), LABEL_NO_TFCARD,1,2000);
		return;
	}

	CloseTipLabel();
	format_disk();
	ShowTipLabel(GetRecordWinHwnd(), LABEL_TFCARD_FORMATTING, 1, 2000);
}

static void FresetDialogCallback(HWND hDlg, void* data)
{
	UserDataType usrData;

	resetResource();
	updateMenuTexts();
	updateSwitchIcons();

	GetUserData(&usrData);
	HandleSubMenuChange2(MENU_INDEX_SILENTMODE, usrData.voice_en);

	CloseMessageBox();

	cvr_debug("restore factory setting\n");
}

int getMessageBoxData(unsigned int menuIndex, MessageBox_t *messageBoxData)
{
	const char* ptr;
	unsigned int i;

	unsigned int indexTable [] =
	{
		MENU_INDEX_FORMAT,
		MENU_INDEX_FRESET
	};

	unsigned int resCmd[][2] =
	{
		{LANG_LABEL_SUBMENU_FORMAT_TITLE, LANG_LABEL_SUBMENU_FORMAT_TEXT},
		{LANG_LABEL_SUBMENU_FRESET_TITLE, LANG_LABEL_SUBMENU_FRESET_TEXT},
	};

	messageBoxData->dwStyle = MB_OKCANCEL | MB_HAVE_TITLE | MB_HAVE_TEXT;
	messageBoxData->flag_end_key = 1;

	for(i = 0; i < TABLESIZE(indexTable); i++)
	{
		if(indexTable[i] == menuIndex)
		{
			ptr = getLabel(resCmd[i][0]);
			if(ptr == NULL)
			{
				return -1;
			}
			messageBoxData->title = ptr;

			ptr = getLabel(resCmd[i][1]);
			if(ptr == NULL)
			{
				return -1;
			}
			messageBoxData->text = ptr;

			ptr = getLabel(LANG_LABEL_SUBMENU_OK);
			if(ptr == NULL)
			{
				return -1;
			}
			messageBoxData->buttonStr[0] = ptr;

			ptr = getLabel(LANG_LABEL_SUBMENU_CANCEL);
			if(ptr == NULL)
			{
				return -1;
			}
			messageBoxData->buttonStr[1] = ptr;
		}
	}

	if(i > TABLESIZE(indexTable))
	{
		return -1;
	}

	messageBoxData->pLogFont = getLogFont();
	getResRect(ID_MENU_LIST_MB, &messageBoxData->rect);
	messageBoxData->fgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_FGC);
	messageBoxData->bgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_BGC);
	messageBoxData->linecTitle = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_TITLE);
	messageBoxData->linecItem = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_ITEM);
	if(indexTable[0] == menuIndex)
	{
		messageBoxData->confirmCallback = FormatDialogCallback;
	}
	else if(indexTable[1] == menuIndex)
	{
		messageBoxData->confirmCallback = FresetDialogCallback;
	}
	return 0;
}


static void formatCallback(HWND hDlg, void* data)
{
	cvr_debug("formatMessageBoxCallback\n");
	format_disk();
	EndDialog(hDlg, IDC_BUTTON_OK);
}

int ShowFormattingTip(HWND mHwnd)
{
	int retval;
	tipLabelData_t tipLabelData;

	memset(&tipLabelData, 0, sizeof(tipLabelData));
	if(getTipLabelData(&tipLabelData) < 0)
	{
		cvr_error("get TipLabel data failed\n");
		return -1;
	}

	tipLabelData.title = getLabel(LANG_LABEL_TIPS);
	if(!tipLabelData.title)
	{
		cvr_error("get FormattingTip titile failed\n");
		return -1;
	}

	tipLabelData.text = getLabel(LANG_LABEL_SUBMENU_FORMATTING_TEXT);
	if(!tipLabelData.text)
	{
		cvr_error("get LANG_LABEL_LOW_POWER_TEXT failed\n");
		return -1;
	}

	tipLabelData.timeoutMs = TIME_INFINITE;
	tipLabelData.disableKeyEvent = CVR_TRUE;
	tipLabelData.callback = formatCallback;
	tipLabelData.callbackData = (void*)0;

	retval = showTipLabel(mHwnd, &tipLabelData);

	return retval;
}

int ShowMessageBox( HWND mHwnd, unsigned int menuIndex,int val)
{
	int retval;
	MessageBox_t messageBoxData;

	memset(&messageBoxData, 0, sizeof(messageBoxData));
	if(getMessageBoxData(menuIndex, &messageBoxData) < 0)
	{
		return -1;
	}

	retval = showMessageBox(mHwnd, &messageBoxData);
	return retval;
}

int updateCheckBoxImage(int newSel)
{
	HWND hMenuList;
	MENULISTITEMINFO mlii;
	int retval;
	hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);

	if(newSel >= 0)
	{
		if(SendMessage(hMenuList, LB_GETITEMDATA, newSel, (LPARAM)&mlii) != LB_OKAY)
		{
			cvr_error("get item info failed\n");
			return -1;
		}
		unloadBitMap(&itemImageArray[newSel]);
		retval = getResBmp(ResourceIDByIndex[newSel],BMPTYPE_SELECTED, &itemImageArray[newSel]);
		if(retval < 0)
		{
			cvr_error("get item image failed, oldSel is \n");
			return -1;
		}
		mlii.hBmpImage = (DWORD)&itemImageArray[newSel];

		if(HaveCheckBox[newSel] == 1)
		{
			unloadBitMap(&value1ImageArray[newSel]);
			retval = getResBmpSubMenuCheckbox(ResourceIDByIndex[newSel], 1, &value1ImageArray[newSel]);
			if(retval < 0)
			{
				cvr_error("get resBmp SubMenu Check box failed, oldSel is \n");
				return -1;
			}
			mlii.hValue[0] = (DWORD)&value1ImageArray[newSel];
		}
		SendMessage(hMenuList, LB_SETITEMDATA, newSel, (LPARAM)&mlii);
	}

	return 0;
}

int ShowDateDialog(HWND hWnd)
{
	int retval;
	dateSettingData_t configData;

	configData.title = getLabel(LANG_LABEL_DATE_TITLE);
	configData.year = getLabel(LANG_LABEL_DATE_YEAR);
	configData.month = getLabel(LANG_LABEL_DATE_MONTH);
	configData.day = getLabel(LANG_LABEL_DATE_DAY);
	configData.hour = getLabel(LANG_LABEL_DATE_HOUR);
	configData.minute = getLabel(LANG_LABEL_DATE_MINUTE);
	configData.second = getLabel(LANG_LABEL_DATE_SECOND);
	if(configData.title == NULL)
		return -1;
	if(configData.year == NULL)
		return -1;
	if(configData.month == NULL)
		return -1;
	if(configData.day == NULL)
		return -1;
	if(configData.hour == NULL)
		return -1;
	if(configData.minute == NULL)
		return -1;
	if(configData.second == NULL)
		return -1;

	retval = getResRect(ID_MENU_LIST_DATE, &configData.rect);
	if(retval < 0) {
		cvr_debug("get date rect failed\n");
		return -1;
	}

	configData.titleRectH = getResIntValue(ID_MENU_LIST_DATE, INTVAL_TITLEHEIGHT);
	configData.hBorder	= getResIntValue(ID_MENU_LIST_DATE, INTVAL_HBORDER);
	configData.yearW	= getResIntValue(ID_MENU_LIST_DATE, INTVAL_YEARWIDTH);
	configData.numberW	= getResIntValue(ID_MENU_LIST_DATE, INTVAL_NUMBERWIDTH);
	configData.dateLabelW = getResIntValue(ID_MENU_LIST_DATE, INTVAL_LABELWIDTH);
	configData.boxH	= getResIntValue(ID_MENU_LIST_DATE, INTVAL_BOXHEIGHT);
	if(configData.titleRectH < 0)
		return -1;
	if(configData.hBorder < 0)
		return -1;
	if(configData.yearW < 0)
		return -1;
	if(configData.numberW < 0)
		return -1;
	if(configData.dateLabelW < 0)
		return -1;
	if(configData.boxH < 0)
		return -1;

	configData.bgc_widget = getResColor(ID_MENU_LIST_DATE, COLOR_BGC);
	configData.fgc_label = getResColor(ID_MENU_LIST_DATE, COLOR_FGC_LABEL);
	configData.fgc_number = getResColor(ID_MENU_LIST_DATE, COLOR_FGC_NUMBER);
	configData.linec_title = getResColor(ID_MENU_LIST_DATE, COLOR_LINEC_TITLE);
	configData.borderc_selected = getResColor(ID_MENU_LIST_DATE, COLOR_BORDERC_SELECTED);
	configData.borderc_normal = getResColor(ID_MENU_LIST_DATE, COLOR_BORDERC_NORMAL);
	configData.pLogFont = getLogFont();

	retval = DateEditDialog(hWnd, &configData);

	return retval;
}

int createSubWidgets(HWND hWnd)
{
	HWND retWnd;
	CvrRectType rect;

	GetClientRect(hWnd, &rect);
	getMenuListAttr(menuListAttr);
	retWnd = CreateWindowEx(CTRL_CDRMenuList, "",
			WS_CHILD  | WS_VSCROLL | LBS_USEBITMAP | WS_VISIBLE | LBS_HAVE_VALUE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			IDC_MENULIST,
			rect.x, rect.y, rect.w, rect.h,
			hWnd,(DWORD)menuListAttr);

	if(retWnd == HWND_INVALID) {
		cvr_error("create Menu List failed\n");
		return -1;
	}
	//ShowWindow(retWnd, SW_HIDE);
    menuWinData->ParentHwnd = hWnd;
	menuWinData->menuHwnd = retWnd;

    createMenuListContents(retWnd);

	return 0;
}

s32 MenukeyProc(MenuWinDataType *menuWinData, s32 keyCode, s32 isLongPress)
{
    HWND hMenuList;

	switch(keyCode)
	{
		case CVR_KEY_OK:
        {
			HWND hMenuList;
			unsigned int selectedItem;
			int newSel, retval;
			bool  isChecked;
            isModified_t Modified;

			hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
			if(isLongPress == SHORT_PRESS)
			{
				selectedItem = SendMessage(hMenuList, LB_GETCURSEL, 0, 0);
				cvr_debug("selectedItem is %d\n", selectedItem);
				switch(selectedItem)
				{
					case MENU_INDEX_VQ:
					case MENU_INDEX_PQ:
					case MENU_INDEX_VTL:
					case MENU_INDEX_SS:
					case MENU_INDEX_LANG:
					case MENU_INDEX_VOICEVOL:
					case MENU_INDEX_IMPACTLEVEL:
					//case MENU_INDEX_WB:
					//case MENU_INDEX_CONTRAST:
					//case MENU_INDEX_EXPOSURE:
					//case MENU_INDEX_GSENSOR:
					//case MENU_INDEX_SHUTDOWN:
					//case MENU_INDEX_FCWSENSITY:
						if( ( newSel = ShowSubMenu(selectedItem, &Modified) ) < 0) {
							cvr_error("show submenu %d failed\n", selectedItem);
							break;
						}
						if(Modified.isModified == 1)
						{
							HandleSubMenuChange(selectedItem, newSel);
						}
						break;
					case MENU_INDEX_POR:	case MENU_INDEX_SILENTMODE:
					case MENU_INDEX_TWM:
					//case MENU_INDEX_AWMD:
					//case MENU_INDEX_PARK:
					//case MENU_INDEX_ALIGNLINE:
						isChecked = getCheckBoxStatus(selectedItem);
						HandleSubMenuChange2(selectedItem, isChecked);
						break;
					case MENU_INDEX_DATE:
						ShowDateDialog(hMenuList);
						break;
					case MENU_INDEX_FORMAT:
					case MENU_INDEX_FRESET:
						if(selectedItem == MENU_INDEX_FORMAT)
						{
							if (!sdcard_is_exist())
							{
								ShowTipLabel(hMenuList, LABEL_NO_TFCARD,1,2000);
								break;
							}
						}
						retval = ShowMessageBox(hMenuList,selectedItem,0);
						if(retval < 0) {
							cvr_error("ShowMessageBox err, retval is %d\n", retval);
							break;
						}
						break;
					default:
						break;
				}
			}
			break;
		}

	case CVR_KEY_MODE:
	{
		return WINDOW_RECORD_ID;
		break;
	}

	case CVR_KEY_LEFT:
		{
            hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
            SendMessage(hMenuList, MSG_MLM_NEW_KEY,keyCode,isLongPress);
		break;
	}

	case CVR_KEY_RIGHT:
		{
            hMenuList = GetDlgItem(menuWinData->menuHwnd, IDC_MENULIST);
            SendMessage(hMenuList, MSG_MLM_NEW_KEY,keyCode,isLongPress);
		break;
	}

	default:
		break;
	}

	return WINDOW_MENU_ID;
}

int MenuWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	MenuWinDataType *menuWinData = NULL;
	menuWinData = (MenuWinDataType*)GetWindowAdditionalData(hWnd);

	//cvr_debug("menu:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_FONTCHANGED:
	        updateMenuTexts();

			{
				PLOGFONT pLogFont;
				HWND hChild;

				pLogFont = GetWindowFont(hWnd);
				if(!pLogFont) {
					cvr_error("invalid logFont\n");
					break;
				}
				hChild = GetNextChild(hWnd, 0);
				while( hChild && (hChild != HWND_INVALID) )
				{
					SetWindowFont(hChild, pLogFont);
					hChild = GetNextChild(hWnd, hChild);
				}
			}
			break;

		case MSG_CREATE:
	        createSubWidgets(hWnd);
			break;

		case MSG_WIN_RESUME:
			updateMenuTexts();
			updateSwitchIcons();

			ShowWindow(GetDlgItem(hWnd, IDC_MENULIST), SW_SHOW);
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_UVC, 3, 0);
			break;

		case MSG_CVR_KEY:
			return MenukeyProc(menuWinData, wParam, lParam);
			break;

		case MSG_TIMER:
			break;

		case MSG_WIN_SUSPEND:
			ShowWindow(GetDlgItem(hWnd, IDC_MENULIST), SW_HIDE);
			break;

		case MSG_DESTROY:
	        if(itemImageArray != NULL){
	            free(itemImageArray);
	            itemImageArray = NULL;}
	        if(value1ImageArray != NULL){
	            free(value1ImageArray);
	            value1ImageArray = NULL;}
			break;

	    case MSG_MLM_NEW_SELECTED:
	        updateNewSelected(wParam, lParam);
	        break;

		case MSG_MLM_HILIGHTED_SPACE:
			SendMessage(GetDlgItem(hWnd, IDC_MENULIST), LB_SETCURSEL, 0, 0);
		    updateMenuTexts();
			updateSwitchIcons();
			updateCheckBoxImage(0);
			break;

		case MSG_TF_CARD_PLUGIN:
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 1, 0);
			break;

		case MSG_TF_CARD_PLUGOUT:
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 0, 0);
			break;

		default:
			break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND MenuWinInit(HWND hParent)
{
	HWND mHwnd;
	CvrRectType rect;

    memset(&unfoldImageLight, 0, sizeof(BITMAP));
    memset(&unfoldImageDark, 0, sizeof(BITMAP));

    itemImageArray = (BITMAP*)malloc(sizeof(BITMAP) * MENU_LIST_COUNT);
	memset(itemImageArray, 0, sizeof(BITMAP) * MENU_LIST_COUNT);

	value1ImageArray = (BITMAP*)malloc(sizeof(BITMAP) * MENU_LIST_COUNT);
	memset(value1ImageArray, 0, sizeof(BITMAP) * MENU_LIST_COUNT);

    menuListAttr = (menuListAttr_t*)malloc(sizeof(menuListAttr_t));
	if(NULL == menuListAttr)
	{
		cvr_error("malloc menuListAttr data error\n");
		return -1;
	}

	menuWinData = (MenuWinDataType*)malloc(sizeof(MenuWinDataType));
	if(NULL == menuWinData)
	{
		cvr_error("malloc hbar window data error\n");
		return -1;
	}
	memset((void *)menuWinData, 0, sizeof(MenuWinDataType));

	menuWinData->menuSize.x = 140;
	menuWinData->menuSize.y = 40;//0;
	menuWinData->menuSize.w = 1280-140 - 140;
	menuWinData->menuSize.h = 320 - 40;//400;

	rect = menuWinData->menuSize;
	mHwnd = CreateWindowEx(WINDOW_MENU, "",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_MENU_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)menuWinData);

	if(mHwnd == HWND_INVALID)
	{
		cvr_error("create status bar failed\n");
		return;
	}
	setHwnd(WINDOW_MENU_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);

	menuWinData->ParentHwnd = hParent;
	menuWinData->menuHwnd = mHwnd;

	return mHwnd;
}

MenuWinDataType* GetMenuWinData(void)
{
	if(menuWinData != NULL)
	{
		return menuWinData;
	}

	return NULL;
}

HWND GetMenuWinHwnd(void)
{
	if(menuWinData!=NULL && menuWinData->menuHwnd!=0)
	{
		return menuWinData->menuHwnd;
	}

	return 0;
}
