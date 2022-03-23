#ifndef __CDR_WIDGETS_H__
#define __CDR_WIDGETS_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/listbox.h>

#include <string.h>
#include "CvrInclude.h"
#ifdef __cplusplus
extern "C" {
#endif

#define CTRL_CDRMenuList		"CDRMenuList"
#define SS_VCENTER		0x00000040
#define LBS_HAVE_VALUE		0x00001000		/* stead of the LBS_CHECKBOX */
#define LBS_USEBITMAP		0x00004000		/* stead of the LBS_AUTOCHECK*/

#define CMFLAG_WITHCHECKBOX	0x00200000		/*add check box for a item*/
#define VMFLAG_IMAGE		0x00010000		/* LBIF_VALUE_IMAGE dwValueImage */
#define VMFLAG_STRING		0x00020000		/* LBIF_VALUE_STRING dwValueString */
#define IMGFLAG_IMAGE		0x00040000


/* if create the menu list use the LBS_USEBITMAP LBS_HAVE_VALUE
 * then the lParam use the PMENULISTITEMINFO
 * */
#define MAX_VALUE_COUNT	3

char* content;
typedef struct __MLFlags
{
	DWORD imageFlag;
	DWORD valueFlag[MAX_VALUE_COUNT];
	unsigned int valueCount;	/* the value's count */
}MLFlags;

typedef struct __MENULISTITEMINFO
{
    char* string;		/* Item string */

    DWORD   cmFlag;     /* check mark flag */

	MLFlags flagsEX;

    /** Handle to the Bitmap (or pointer to bitmap object) of the item */
	DWORD	hBmpImage;
	/*  address of the value image1 or value string1*/
	DWORD	hValue[MAX_VALUE_COUNT];
} MENULISTITEMINFO;

typedef MENULISTITEMINFO* PMENULISTITEMINFO;

typedef struct
{
	gal_pixel normalBgc;
	gal_pixel normalFgc;
	gal_pixel linec;
	gal_pixel normalStringc;
	gal_pixel normalValuec;
	gal_pixel selectedStringc;
	gal_pixel selectedValuec;
	gal_pixel scrollbarc;
	int itemHeight;
}menuListAttr_t;

typedef struct
{
    int count;
    char* content[10];
}contents_t;

typedef struct
{
	CvrRectType rect;
	char* title;
	contents_t contents;
	int selectedIndex;
	BITMAP BmpChoice;
	gal_pixel lincTitle;
	menuListAttr_t menuListAttr;
	PLOGFONT pLogFont;
}subMenuData_t;

int MenuListCallback(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
int showSubMenu(HWND hWnd, subMenuData_t* subMenuData);

#ifdef __cplusplus
}  /* end of extern "C" */
#endif
#endif
