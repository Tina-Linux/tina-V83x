#ifndef _RIGHTMENU_H_
#define _RIGHTMENU_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "common.h"
/* #define WIDTH 100 */
/* #define LX 1000 */
#define FENBIANLV "Screen Resolution"
#define SCREEN_1280X800 "1280x800"
#define SCREEN_800X480 "800x480"
#define SCREEN_1280X480 "1280x480"
#define FORLOOP_YES "forloop"
#define ROTATE_YES "rotate"
#define OK "ok"
#define CANCEL "cancel"
#define SCALEDOWN "scaledown"
#define NEXT "next"
#define PREV "prev"
#if 0
#define SCALEDOWN0 "1/1"
#define SCALEDOWN1 "1/2"
#define SCALEDOWN2 "1/4"
#define SCALEDOWN3 "1/8"
#endif
#define SELECT_BTN_W 200
#define SELECT_BTN_H 30
#define OK_BTN_W 80
#define OK_BTN_H 50
#define CANCEL_BTN_W 80
#define CANCEL_BTN_H 50

#define FILE_MAX 18

#define _ID_TIMER_RIGHTHIDE 102
typedef struct {
	HWND RightMenuHwnd;
	HWND Btn[14];
	HWND ListBox0;
	HWND rightStatic;
	HWND nextbutton;
	HWND prevbutton;
	CvrRectType rightSize;
	CvrRectType prebuttonSize;
	CvrRectType nextbuttonSize;
	BITMAP nextbuttonBmp;
	BITMAP prebuttonBmp;
	PLOGFONT logFont20;
	PLOGFONT logFont25;
	PLOGFONT logFont15;
	int boolfreespace;
	int addID;
	int addfiletype; /* Which type of file to represent */
	char *filename_list[FILE_MAX];
	char *filepath_list[FILE_MAX];
	int listboxskip;
	LISTBOXITEMINFO lbii;
	BOOL isButtonNext;
	BOOL isButtonPre;

	/* Dynamically according to the number of files to apply for space, */
	/* if the number exceeds FILE_MAX apply for FILE_MAX size space */
	int request_number;
	/* Indicates the number of pages displayed */
	int paginal_number;

	/* Indicates the amount of data under the page */
	int paginal_residual_number;
} RightMenuDataType;

int RightResourceInit();
int createlist(int filetype);
void CreateListFileName(HWND hwnd, int filetype);
char * get_find_file_name();
PLOGFONT getFont();
#endif

