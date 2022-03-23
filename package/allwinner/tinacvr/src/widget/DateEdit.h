#ifndef __DATE_EDIT_H__
#define __DATE_EDIT_H__

#include "CvrInclude.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

typedef struct
{
	const char* title;
	const char* year;
	const char* month;
	const char* day;
	const char* hour;
	const char* minute;
	const char* second;
	CvrRectType rect;
	int titleRectH;
	int hBorder;
	int yearW;
	int numberW;
	int dateLabelW;
	int boxH;

	gal_pixel bgc_widget;
	gal_pixel fgc_label;
	gal_pixel fgc_number;
	gal_pixel linec_title;
	gal_pixel borderc_selected;
	gal_pixel borderc_normal;
	PLOGFONT pLogFont;
}dateSettingData_t;

int DateEditDialog(HWND hParent, dateSettingData_t* configData);

#endif
