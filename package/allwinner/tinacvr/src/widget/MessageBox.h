#ifndef __MESSAGE_BOX_H__
#define __MESSAGE_BOX_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "CvrInclude.h"

#define MESSAGEBOX_MAX_BUTTONS	3

typedef struct
{
	unsigned char flag_end_key;
	unsigned int dwStyle;
	const char* title;
	const char* text;
	PLOGFONT pLogFont;

	CvrRectType rect;
	gal_pixel bgcWidget;
	gal_pixel fgcWidget;
	gal_pixel linecTitle;
	gal_pixel linecItem;
	const char* buttonStr[MESSAGEBOX_MAX_BUTTONS];
	void (*confirmCallback)(HWND hText, void* data);
	void* confirmData;
	const char* msg4ConfirmCallback;
}MessageBox_t;

int showMessageBox (HWND hParentWnd, MessageBox_t *info);
int CloseMessageBox(void);

#endif
