#ifndef __CVR_USB_WINDOW_H__
#define __CVR_USB_WINDOW_H__

#include "CvrInclude.h"
#include "CvrResource.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define POPMENU_MAX_ITEMS	10

typedef struct {
	unsigned int style;
	unsigned int item_nrs;
	int id;		// items' start id
	const char *name[POPMENU_MAX_ITEMS];
	void (*callback[POPMENU_MAX_ITEMS])(HWND hWnd, int id, int nc, DWORD context);
	DWORD context;
	CvrRectType rect;
	unsigned int item_width;
	unsigned int item_height;
	unsigned char flag_end_key;
	unsigned int push_key;		// the key value to push the button
	unsigned int table_key;
	unsigned int end_key;		// key value to end the dialog
	gal_pixel bgc_widget;
	gal_pixel bgc_item_focus;
	gal_pixel bgc_item_normal;
	gal_pixel mainc_3dbox;
	BITMAP bmp;
	PLOGFONT pLogFont;
	unsigned int showImage;    // 1: MSG_PAINT show Image
}PopUpMenuInfo_t;

typedef struct
{
	HWND ParentHwnd;
	HWND usbHwnd;
	CvrRectType usbSize;

	PopUpMenuInfo_t *info;
	PCTRLDATA ctrldata;
	DLGTEMPLATE dlg;

	BITMAP usbModeBmp;
	BITMAP pcCamBmp;

	bool bConnect2Pc;
	bool bUsbStorageOn;

}UsbWinDataType;

UsbWinDataType* GetUsbWinData(void);
HWND GetUsbWinHwnd(void);

HWND UsbWinInit(HWND hParent);
int UsbWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#endif