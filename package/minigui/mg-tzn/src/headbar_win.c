#include <string.h>
#include "headbar_win.h"

#define STATBAR_TIMER_UPDATE_REC	101
#define STATBAR_TIMER_UPDATE	102

headBarWinData *hbar = NULL;
#define _ID_TIMER   100

static int CreatHeadStatusWin(HWND hWnd)
{
	smRect rect;
	gal_pixel color;
	struct tm u_time;
	char buff[16];
    PLOGFONT timefont;

	get_local_time(&u_time);
	memset(buff, 0, sizeof(buff));
	timefont = (PLOGFONT)getLogFont(ID_FONT_TIMES_45);

	/* show status bar, date,time and lock status */
	getResRect(ID_STATUSBAR_ICON_DATE, &rect);
	hbar->dateHwnd = CreateWindowEx (CTRL_STATIC,
    	"",
     	WS_CHILD | WS_VISIBLE | SS_CENTER | SS_BITMAP | SS_REALSIZEIMAGE
     	| SS_CENTERIMAGE,
     	WS_EX_NONE,
     	ID_STATUSBAR_ICON_DATE,
     	rect.x, rect.y, rect.w, rect.h, hWnd, 0);

	getResRect(ID_STATUSBAR_CUR_TIME, &rect);
	hbar->timeHwnd = CreateWindowEx (CTRL_STATIC,
    	"",
     	WS_CHILD | WS_VISIBLE | SS_CENTER | SS_BITMAP | SS_REALSIZEIMAGE
     	| SS_CENTERIMAGE,
     	WS_EX_NONE,
     	ID_STATUSBAR_CUR_TIME,
     	rect.x, rect.y, rect.w, rect.h, hWnd, 0);

	color = getResColor(ID_HEAD_BAR, COLOR_BGC);
	SetWindowBkColor(hbar->timeHwnd, color);
	SetWindowBkColor(hbar->dateHwnd, color);

	SetWindowFont(hbar->dateHwnd, timefont);
	SetWindowFont(hbar->timeHwnd, timefont);
	sprintf(buff, "%.2d:%.2d", u_time.tm_hour, u_time.tm_min);
    buff[5] = '\0';
	SetWindowText(hbar->timeHwnd, buff);
	sprintf(buff, "%.4d-%.2d-%.2d", u_time.tm_year,
		u_time.tm_mon, u_time.tm_mday);
	buff[11] = '\0';
	SetWindowText(hbar->dateHwnd, buff);

	setCurrentIconValue(ID_STATUSBAR_LOCK, 1);
	getResBmp(ID_STATUSBAR_LOCK, BMPTYPE_BASE, &hbar->s_Head_lock);
	getResRect(ID_STATUSBAR_LOCK,&rect);
	hbar->labelLock = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_NOTIFY | SS_REALSIZEIMAGE
		| SS_CENTERIMAGE,
		WS_EX_TRANSPARENT,
		IDC_STATUS_LOCK,
		rect.x, rect.y, rect.w, rect.h, hWnd, (DWORD)&hbar->s_Head_lock);
	if(hbar->labelLock)
		ShowWindow(hbar->labelLock, SW_HIDE);

	setCurrentIconValue(ID_STATUSBAR_SHAPE, 0);
	getResBmp(ID_STATUSBAR_SHAPE, BMPTYPE_BASE, &hbar->s_Head_shape);
	getResRect(ID_STATUSBAR_SHAPE,&rect);
	CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_REALSIZEIMAGE | SS_CENTERIMAGE,
		WS_EX_TRANSPARENT,
		ID_STATUSBAR_SHAPE,
		rect.x, rect.y, rect.w, rect.h, hWnd, (DWORD)&hbar->s_Head_shape);

	setCurrentIconValue(ID_STATUSBAR_PREOPEN, 0);
	getResBmp(ID_STATUSBAR_PREOPEN, BMPTYPE_BASE, &hbar->s_Head_preopen);
	getResRect(ID_STATUSBAR_PREOPEN,&rect);
	CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_REALSIZEIMAGE | SS_CENTERIMAGE,
		WS_EX_TRANSPARENT,
		ID_STATUSBAR_PREOPEN,
		rect.x, rect.y, rect.w, rect.h, hWnd, (DWORD)&hbar->s_Head_preopen);

	return 0;
}

int HeadBarTimerProcess(HWND hWnd)
{
	struct tm u_time;
	char buff[16];
	RECT rect;
    PLOGFONT timefont;

	GetClientRect(hbar->dateHwnd, &rect);
	InvalidateRect(hbar->dateHwnd, &rect, FALSE);
	GetClientRect(hbar->timeHwnd, &rect);
	InvalidateRect(hbar->timeHwnd, &rect, FALSE);

	get_local_time(&u_time);
	memset(buff, 0, sizeof(buff));
	timefont = (PLOGFONT)getLogFont(ID_FONT_TIMES_45);
	SetWindowFont(hbar->dateHwnd, timefont);
	SetWindowFont(hbar->timeHwnd, timefont);
	sprintf(buff, "%.2d:%.2d", u_time.tm_hour, u_time.tm_min);
    buff[5] = '\0';
	SetWindowText(hbar->timeHwnd, buff);
	sprintf(buff, "%.4d-%.2d-%.2d", u_time.tm_year,
		u_time.tm_mon, u_time.tm_mday);
	buff[11] = '\0';
	SetWindowText(hbar->dateHwnd, buff);
	UpdateWindow(hbar->timeHwnd, 0);
	UpdateWindow(hbar->dateHwnd, 0);

	return 0;
}

int HeadBarCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int index;
	RECT rect;
	headBarWinData *hbar = NULL;

	index = (int)wParam;
	hbar = (headBarWinData*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
		case MSG_SET_LOCKICON:
		{
			if(hbar->labelLock)
	   			ShowWindow(hbar->labelLock, SW_SHOW);
			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

headBarWinData* GetHeadBarWinData(void)
{
	if(hbar != NULL) {
		return hbar;
	}

	return NULL;
}

int HeadBarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	headBarWinData *hbar = NULL;
	hbar = (headBarWinData*)GetWindowAdditionalData(hWnd);

	//sm_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_FONTCHANGED:
		{
			HWND hChild;
			PLOGFONT pLogFont;

			pLogFont = GetWindowFont(hWnd);
			if(!pLogFont)
			{
				sm_error("invalid logFont\n");
				break;
			}

			hChild = GetNextChild(hWnd, 0);
			while( hChild && (hChild != HWND_INVALID))
			{
				SetWindowFont(hChild, pLogFont);
				hChild = GetNextChild(hWnd, hChild);
			}
		}
			break;

		case MSG_CREATE:
		{
			gal_pixel color;
			color = getResColor(ID_HEAD_BAR, COLOR_BGC);
			SetWindowBkColor(hWnd, color);

			CreatHeadStatusWin(hWnd);
			SetTimer (hWnd, STATBAR_TIMER_UPDATE, 6000);
			break;
		}
		case MSG_TIMER:
		{
			if(wParam == STATBAR_TIMER_UPDATE){
				HeadBarTimerProcess(hWnd);
			}
			break;
		}
		case MSG_PAINT:
			break;
		case MSG_DESTROY:
			if(hbar)
				free(hbar);
			break;
		default:
		{
			HeadBarCallBack(hWnd, message, wParam, lParam);
			break;
		}
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND GetHeadBarWin(void)
{
	if(hbar != NULL && hbar->headBarHwnd != 0) {
		return hbar->headBarHwnd;
	}

	return 0;
}

HWND HeadBarWinInit(HWND hParent)
{
	HWND mHwnd;
	smRect rect;

	hbar = (headBarWinData*)malloc(sizeof(headBarWinData));
	if(NULL == hbar)
	{
		sm_error("malloc hbar window data error\n");
		return -1;
	}
	memset((void *)hbar, 0, sizeof(headBarWinData));

	getResRect(ID_HEAD_BAR, &rect);

	hbar->headBarSize.x = rect.x;
	hbar->headBarSize.y = rect.y;
	hbar->headBarSize.w = rect.w;
	hbar->headBarSize.h = rect.h;

	rect = hbar->headBarSize;
	mHwnd = CreateWindowEx(WINDOW_HEADBAR, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_HEADBAR_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)hbar);

	if(mHwnd == HWND_INVALID)
	{
		sm_error("create status bar failed\n");
		return HWND_INVALID;
	}

	setHwnd(WINDOW_HEADBAR_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);

	hbar->parentHwnd = hParent;
	hbar->headBarHwnd = mHwnd;

	return mHwnd;
}
