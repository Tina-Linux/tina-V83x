#include <string.h>
#include "statbar_win.h"

#define STATBAR_TIMER_UPDATE_REC	101
#define STATBAR_TIMER_UPDATE	102

stBarWinData *stbar = NULL;

int CreateStatBarWidgets(HWND hParent)
{
	int  retval;
	HWND retWnd;
	RGB    rgbs;
	gal_pixel  color;
	smRect rect;
	int i,j;

	setCurrentIconValue(ID_STATUSBAR_BACK, 0);
	getResBmp(ID_STATUSBAR_BACK, BMPTYPE_BASE, &stbar->backBmp);
	getResRect(ID_STATUSBAR_BACK, &rect);
	stbar->backHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE | SS_REALSIZEIMAGE | SS_NOTIFY,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_BACK,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&stbar->backBmp));
	if(stbar->backHwnd == HWND_INVALID)
	{
		sm_error("create back Hwnd failed\n");
		return -1;
	}

	setCurrentIconValue(ID_STATUSBAR_TITLE, 0);
	getResBmp(ID_STATUSBAR_TITLE, BMPTYPE_BASE, &stbar->titleBmp);
	getResRect(ID_STATUSBAR_TITLE,&rect);
	stbar->titleHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE | SS_REALSIZEIMAGE,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_TITLE,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&stbar->titleBmp));
	if(stbar->titleHwnd == HWND_INVALID)
	{
		sm_error("create title Hwnd failed\n");
		return -1;
	}

	for (j = 0; j < 2; j ++) {
		for (i = 0; i < 4; i++) {

    		/* Mode have 7 icons, list in two row
    			 * So remove the eight one
    		  	 */
    		if((3 == i) &&(1 == j))
    			continue;

    		setCurrentIconValue(ID_STATUSBAR_ICON_ITEMS, j*4+i);
    		getResBmp(ID_STATUSBAR_ICON_ITEMS, BMPTYPE_BASE, &stbar->iconItem[j*4+i]);
    		getResRect(ID_STATUSBAR_ICON_ITEMS,&rect);

    		stbar->iconHwnd[i+j*4] = CreateWindowEx (CTRL_STATIC,
    		"",
    		WS_CHILD | SS_NOTIFY | SS_BITMAP | WS_VISIBLE | SS_REALSIZEIMAGE | SS_CENTERIMAGE, WS_EX_TRANSPARENT,
    		IDC_MODE_ITEM_COOL+j*4+i,
    		rect.x+(i%4)*160, rect.y+j*200, rect.w, rect.h, hParent, (DWORD)&stbar->iconItem[j*4+i]);

    		setCurrentIconValue(ID_STATUSBAR_FONT_ITEMS, j*4+i);
    		getResBmp(ID_STATUSBAR_FONT_ITEMS, BMPTYPE_BASE, &stbar->fontItem[j*4+i]);
    		getResRect(ID_STATUSBAR_FONT_ITEMS,&rect);
    		stbar->fontHwnd[i+j*4] = CreateWindowEx (CTRL_STATIC,
    		"",
    		WS_CHILD | SS_NOTIFY | SS_BITMAP | WS_VISIBLE | SS_REALSIZEIMAGE | SS_CENTERIMAGE, WS_EX_TRANSPARENT,
    		IDC_STATIC,
    		rect.x+(i%4)*160, rect.y+j*200, rect.w, rect.h, hParent, (DWORD)&stbar->fontItem[j*4+i]);
		}
	}

	stbar->mode_idx = ID_MAINPAGE_BTN_ITEM1;

	return 0;
}

int ChangeModeWidgets(HWND hParent, int index)
{
	RECT rect;
	int tite_index;
	int icon_idx;
	int font_idx;
	int i,j;


	unloadBitMap(&stbar->titleBmp);
	GetClientRect(stbar->titleHwnd, &rect);
	InvalidateRect(stbar->titleHwnd, &rect, TRUE);

	switch(index){
		case ID_MAINPAGE_BTN_ITEM1:
			tite_index = 0;
			icon_idx = ID_STATUSBAR_ICON_ITEMS;
			font_idx = ID_STATUSBAR_FONT_ITEMS;
			break;
		case ID_MAINPAGE_BTN_ITEM2:
			tite_index = 1;
			icon_idx = ID_STATUSBAR_ICON_WIND;
			font_idx = ID_STATUSBAR_FONT_WIND;
			break;
		case ID_MAINPAGE_BTN_ITEM3:
			tite_index = 2;
			icon_idx = ID_STATUSBAR_ICON_DIRECT;
			font_idx = ID_STATUSBAR_FONT_DIRECT;
			break;
		default:
			break;
	}
	setCurrentIconValue(ID_STATUSBAR_TITLE, tite_index);
	getResBmp(ID_STATUSBAR_TITLE, BMPTYPE_BASE, &stbar->titleBmp);
	SendMessage(stbar->titleHwnd, STM_SETIMAGE, (WPARAM)&stbar->titleBmp, 0);

	if ((index == ID_MAINPAGE_BTN_ITEM1)
		||(index == ID_MAINPAGE_BTN_ITEM2)) {
		for (j = 0; j < 2; j ++) {
			for (i = 0; i < 4; i++) {
				if((3 == i) &&(1 == j))
	    			continue;

				unloadBitMap(&stbar->iconItem[j*4+i]);
				GetClientRect(stbar->iconHwnd[j*4+i], &rect);
				InvalidateRect(stbar->iconHwnd[j*4+i], &rect, TRUE);

				unloadBitMap(&stbar->fontItem[j*4+i]);
				GetClientRect(stbar->fontHwnd[j*4+i], &rect);
				InvalidateRect(stbar->fontHwnd[j*4+i], &rect, TRUE);

				setCurrentIconValue(icon_idx, j*4+i);
	    		getResBmp(icon_idx, BMPTYPE_BASE, &stbar->iconItem[j*4+i]);
				SendMessage(stbar->iconHwnd[j*4+i], STM_SETIMAGE, (WPARAM)&stbar->iconItem[j*4+i], 0);

				setCurrentIconValue(font_idx, j*4+i);
	    		getResBmp(font_idx, BMPTYPE_BASE, &stbar->fontItem[j*4+i]);
				SendMessage(stbar->fontHwnd[j*4+i], STM_SETIMAGE, (WPARAM)&stbar->fontItem[j*4+i], 0);
			}
		}
	}else if(index == ID_MAINPAGE_BTN_ITEM3) {
		for (j = 0; j < 2; j ++) {
				for (i = 0; i < 4; i++) {
					if((3 == i) &&(1 == j))
	    				continue;
					unloadBitMap(&stbar->iconItem[j*4+i]);
					GetClientRect(stbar->iconHwnd[j*4+i], &rect);
					InvalidateRect(stbar->iconHwnd[j*4+i], &rect, TRUE);

					unloadBitMap(&stbar->fontItem[j*4+i]);
					GetClientRect(stbar->fontHwnd[j*4+i], &rect);
					InvalidateRect(stbar->fontHwnd[j*4+i], &rect, TRUE);
					if((i > 1) &&(1 == j))
						continue;

					setCurrentIconValue(icon_idx, j*4+i);
					getResBmp(icon_idx, BMPTYPE_BASE, &stbar->iconItem[j*4+i]);
					SendMessage(stbar->iconHwnd[j*4+i], STM_SETIMAGE, (WPARAM)&stbar->iconItem[j*4+i], 0);
				}
		}
	}
	return 0;
}

int StatBarTimerProc(HWND hWnd, int id, DWORD lm)
{
	struct tm u_time;
	char buff[64];
    PLOGFONT timefont;

	get_local_time(&u_time);
	memset(buff, 0, sizeof(buff));
	timefont = getLogFont(ID_FONT_TIMES_45);
	SetWindowFont(stbar->dateHwnd, timefont);
	SetWindowFont(stbar->timeHwnd, timefont);

	sprintf(buff, "%.2d:%.2d", u_time.tm_hour, u_time.tm_min);
    buff[5] = '\0';
	SetWindowText(stbar->timeHwnd, buff);
	sprintf(buff, "%.4d-%.2d-%.2d", u_time.tm_year,
		u_time.tm_mon, u_time.tm_mday);
	buff[11] = '\0';
	SetWindowText(stbar->dateHwnd, buff);
	UpdateWindow(stbar->timeHwnd, 0);
	UpdateWindow(stbar->dateHwnd, 0);

	return 0;
}

stBarWinData* Gethbar(void)
{
	if(stbar != NULL)
	{
		return stbar;
	}

	return NULL;
}

int statbar_res_free(void)
{
	int i;
	unloadBitMap(&stbar->titleBmp);
	unloadBitMap(&stbar->bgBmp);
	unloadBitMap(&stbar->headbgBmp);
	for(i = 0; i < 8; i++){
		unloadBitMap(&stbar->iconItem[i]);
		unloadBitMap(&stbar->fontItem[i]);
	}
	return 0;
}

int StatBarCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int index;
	RECT rect;
	stBarWinData *stbar = NULL;

	index = (int)wParam;
	stbar = (stBarWinData*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
		case MSG_STATBAR_UPDATAE_TITLE:
		{
			unloadBitMap((BITMAP *)&stbar->titleBmp);
			GetClientRect((HWND)stbar->titleHwnd, (PRECT)&rect);
			InvalidateRect((HWND)stbar->titleHwnd, (PRECT)&rect, TRUE);

			if(index == 0)
			{
				getResBmp(ID_STATUSBAR_TITLE, BMPTYPE_WINDOWPIC_MODE, (BITMAP *)&stbar->titleBmp);
			}
			else if(index == 1)
			{
				getResBmp(ID_STATUSBAR_TITLE, BMPTYPE_WINDOWPIC_WIND, (BITMAP *)&stbar->titleBmp);
			}
			else if(index == 2)
			{
				getResBmp(ID_STATUSBAR_TITLE, BMPTYPE_WINDOWPIC_WIND_DIRECT, (BITMAP *)&stbar->titleBmp);
			}

			SendMessage((HWND)stbar->titleHwnd, STM_SETIMAGE, (WPARAM)&stbar->titleBmp, 0);
			break;
		}

		case MSG_HBAR_UPDATAE_MODE:
		{
			unloadBitMap((BITMAP *)&stbar->iconItem);
			GetClientRect((HWND)stbar->iconHwnd, (PRECT)&rect);
			InvalidateRect((HWND)stbar->iconHwnd, (PRECT)&rect, TRUE);

			setCurrentIconValue(ID_STATUSBAR_ICON_ITEMS, index);
			getResBmp(ID_STATUSBAR_ICON_ITEMS, BMPTYPE_BASE, (BITMAP *)&stbar->iconItem);
			SendMessage((HWND)stbar->iconHwnd, STM_SETIMAGE, (WPARAM)&stbar->iconItem, 0);

			break;
		}
		case MSG_SHOW_ITEMS:
		{
			int items = LOWORD(lParam);
			ChangeModeWidgets(hWnd, items);
			stbar->mode_idx = items;
			break;
		}
		case MSG_COMMAND:
		{
			int items = LOWORD(lParam);
			if (LOWORD(wParam) == ID_STATUSBAR_BACK ) {
				SendMessage ((HWND)stbar->parentHwnd, MSG_ITEMS_RETURN, wParam, items);
				statbar_res_free();
			}

			if ((LOWORD(wParam) >= IDC_MODE_ITEM_COOL)
				&& (LOWORD(wParam) <= IDC_MODE_ITEM_COOL+6)){
				SendMessage ((HWND)stbar->parentHwnd, MSG_CHANGE_ICONS, wParam, stbar->mode_idx);
				statbar_res_free();
			}
			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

stBarWinData* GetStBarWinData(void)
{
	if(stbar != NULL) {
		return stbar;
	}

	return NULL;
}

int StBarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	stBarWinData *stbar = NULL;
	stbar = (stBarWinData*)GetWindowAdditionalData(hWnd);

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
			color = getResColor(ID_STATUSBAR, COLOR_BGC);
			SetWindowBkColor(hWnd, color);
			CreateStatBarWidgets(hWnd);

			break;
		}
		case MSG_PAINT:
		{
			HDC hdc;
         	hdc = BeginPaint (hWnd);
			getResBmp(ID_SCREEN_BKG, BMPTYPE_BASE, &stbar->bgBmp);
			getResBmp(ID_HEAD_BKG, BMPTYPE_BASE, &stbar->headbgBmp);
       		FillBoxWithBitmap (hdc, 0, 0, 800, 480, &stbar->bgBmp);
			FillBoxWithBitmap (hdc, 0, 0, 800, 60, &stbar->headbgBmp);
			EndPaint (hWnd, hdc);
			break;
		}
		case MSG_DESTROY:
			if(stbar)
				free(stbar);
			break;
		default:
		{
			StatBarCallBack(hWnd, message, wParam, lParam);
			break;
		}
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND GetStBarWin(void)
{
	if(stbar != NULL && stbar->statBarHwnd != 0) {
		return stbar->statBarHwnd;
	}

	return 0;
}

HWND StBarWinInit(HWND hParent)
{
	HWND mHwnd;
	smRect rect;

	stbar = (stBarWinData*)malloc(sizeof(stBarWinData));
	if(NULL == stbar)
	{
		sm_error("malloc hbar window data error\n");
		return -1;
	}
	memset((void *)stbar, 0, sizeof(stBarWinData));

	getResRect(ID_SCREEN, &rect);

	stbar->statBarSize.x = rect.x;
	stbar->statBarSize.y = rect.y;
	stbar->statBarSize.w = rect.w;
	stbar->statBarSize.h = rect.h;

	rect = stbar->statBarSize;
	mHwnd = CreateWindowEx(WINDOW_STATBAR, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_STATBAR_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)stbar);

	if(mHwnd == HWND_INVALID)
	{
		sm_error("create status bar failed\n");
		return HWND_INVALID;
	}

	setHwnd(WINDOW_STATBAR_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);

	stbar->parentHwnd = hParent;
	stbar->statBarHwnd = mHwnd;

	return mHwnd;
}

