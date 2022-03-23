#ifndef _HEAD_WINDOW_H__
#define _HEAD_WINDOW_H__

#include "common.h"
#include "resource.h"
#include "power.h"
#include "sunxi_display_v2.h"
#include "mid_list.h"

#include <sys/time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define HEAD_BAR_VIEW "headBarView"
typedef struct {
	HWND ParentHwnd;
	HWND HbarHwnd;
	HWND backHwnd;
	HWND tfcardHwnd;
	HWND batteryHwnd;
	HWND listHwnd;
	HWND setupHwnd;
	CvrRectType hbarSize;
	CvrRectType backSize;
	CvrRectType tfcardSize;
	CvrRectType batterySize;
	CvrRectType listSize;
	CvrRectType setupSize;
	BITMAP tfcardBmp;
	BITMAP batteryBmp;
	BITMAP backBmp;
	BITMAP listBmp;
	BITMAP setupBmp;
	int addID;
	BOOL isCreateMediaList;
	BOOL isRightWndOpen;
	int is1280x800;
	int is800x480;
	int is1280x480;
	int isforloop;
	int isrotate;
	/* Zoom mode, 0 means no zoom, 1 means Zoom 1/2 2 means zoom 1/4 */
	int scaledown;
	/* Indicates whether to release the structure space 0 Normal return */
	/* to the main interface release -1 Forced to exit the release of */
	/* space 1 That space is not released */
	CvrRectType Displayrect;
	int GetBtnNum;
} HbarWinDataType;

void SetUpInit(void);
static void headbar_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
void CreateHeadBarBtn(HWND hWnd, int id);
static int HeadBarProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
#endif
