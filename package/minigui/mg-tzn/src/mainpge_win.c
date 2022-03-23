#include <string.h>
#include "mainpge_win.h"

#define STATBAR_TIMER_UPDATE_REC	101
#define STATBAR_TIMER_UPDATE	102
#define FIRST_PAGE 0
#define SECONDE_PAGE 1

maiPageWinData *hmainpge = NULL;

static int CreatMainPgeWin(HWND hWnd)
{
	smRect rect;
	gal_pixel pixel;

	/* show main push button */
	getResRect(ID_MAINPAGE_BTN_LEFT, &rect);
	getResBmp(ID_MAINPAGE_BTN_LEFT, BMPTYPE_BASE, &hmainpge->iconleft);
	hmainpge->leftHwnd = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE | SS_NOTIFY |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		ID_MAINPAGE_BTN_LEFT,
		rect.x, rect.y, rect.w, rect.h,  hWnd, (DWORD)&hmainpge->iconleft);

	getResRect(ID_MAINPAGE_BTN_RIGHT, &rect);
	getResBmp(ID_MAINPAGE_BTN_RIGHT, BMPTYPE_BASE, &hmainpge->iconright);
	hmainpge->rightHwnd = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP | WS_VISIBLE | SS_NOTIFY | SS_REALSIZEIMAGE | SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		ID_MAINPAGE_BTN_RIGHT,
		rect.x, rect.y, rect.w, rect.h,  hWnd, (DWORD)&hmainpge->iconright);
	if(hmainpge->rightHwnd)
		ShowWindow(hmainpge->rightHwnd, SW_SHOW);

	if(hmainpge->leftHwnd)
		ShowWindow(hmainpge->leftHwnd, SW_HIDE);

	getResRect(ID_MAINPAGE_BTN_ITEM1, &rect);
	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->iconItem[0]);
	/* show main push button */
	hmainpge->iconHwnd[0] = CreateWindowEx (CTRL_BUTTON, "",
			WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP | BS_REALSIZEIMAGE, WS_EX_NONE,
			ID_MAINPAGE_BTN_ITEM1,
			rect.x, rect.y, rect.w, rect.h,
			hWnd, (DWORD)"mode");

	getResRect(ID_MAINPAGE_BTN_ITEM2, &rect);
	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->iconItem[1]);
	hmainpge->iconHwnd[1] = CreateWindowEx (CTRL_BUTTON, "",
			WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP | BS_REALSIZEIMAGE, WS_EX_NONE,
			ID_MAINPAGE_BTN_ITEM2,
			rect.x, rect.y, rect.w, rect.h,
			hWnd, (DWORD)"wind");

	getResRect(ID_MAINPAGE_BTN_ITEM3, &rect);
	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->iconItem[2]);
	hmainpge->iconHwnd[2] = CreateWindowEx (CTRL_BUTTON, "",
			WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP | BS_REALSIZEIMAGE, WS_EX_NONE,
			ID_MAINPAGE_BTN_ITEM3,
			rect.x, rect.y, rect.w, rect.h,
			hWnd, (DWORD)"wind_direct");

	getResRect(ID_MAINPAGE_BTN_ITEM4, &rect);
	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->iconItem[3]);
	hmainpge->iconHwnd[3] = CreateWindowEx (CTRL_BUTTON, "",
			WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP | BS_REALSIZEIMAGE, WS_EX_NONE,
			ID_MAINPAGE_BTN_ITEM4,
			rect.x, rect.y, rect.w, rect.h,
			hWnd, (DWORD)"stop");

	 /* set button background */
	SetWindowElementAttr(hmainpge->iconHwnd[0], WE_LFSKIN_WND_BKGND, 0L);
	pixel = GetPixelInBitmap(&hmainpge->iconItem[0], 40, 40);
	SetWindowBkColor (hmainpge->iconHwnd[0], pixel);

	SetWindowElementAttr(hmainpge->iconHwnd[1], WE_LFSKIN_WND_BKGND, 0L);
	pixel = GetPixelInBitmap (&hmainpge->iconItem[1], 40, 40);
	SetWindowBkColor(hmainpge->iconHwnd[1], pixel);

	SetWindowElementAttr(hmainpge->iconHwnd[2], WE_LFSKIN_WND_BKGND, 0L);
	pixel = GetPixelInBitmap(&hmainpge->iconItem[2], 40, 40);
	SetWindowBkColor(hmainpge->iconHwnd[2], pixel);

	SetWindowElementAttr(hmainpge->iconHwnd[3], WE_LFSKIN_WND_BKGND, 0L);
	pixel = GetPixelInBitmap(&hmainpge->iconItem[3], 40, 40);
	SetWindowBkColor(hmainpge->iconHwnd[3], pixel);
	InvalidateRect(hmainpge->iconHwnd[3], NULL, TRUE);

	for(int i=0; i<4; i++)
		unloadBitMap(&hmainpge->iconItem[i]);

	/* set button bitmap */
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 5);
 	getResBmp(ID_MAINPAGE_BTN_ITEM1, BMPTYPE_BASE, &hmainpge->iconItem[0]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 5);
    getResBmp(ID_MAINPAGE_BTN_ITEM2, BMPTYPE_BASE, &hmainpge->iconItem[1]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 5);
    getResBmp(ID_MAINPAGE_BTN_ITEM3, BMPTYPE_BASE, &hmainpge->iconItem[2]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 5);
    getResBmp(ID_MAINPAGE_BTN_ITEM4, BMPTYPE_BASE, &hmainpge->iconItem[3]);

	SendMessage (hmainpge->iconHwnd[0], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[0]);
	SendMessage (hmainpge->iconHwnd[1], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[1]);
	SendMessage (hmainpge->iconHwnd[2], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[2]);
	SendMessage (hmainpge->iconHwnd[3], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[3]);

	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 1);
	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->fontItem[0]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 1);
	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->fontItem[1]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 1);
	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->fontItem[2]);
	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 1);
	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->fontItem[3]);
	/* set font on the button */
	hmainpge->fontHwnd[0] = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		IDC_STATIC,
		80, 120, 140, 40, hWnd, (DWORD)&hmainpge->fontItem[0]);
	hmainpge->fontHwnd[1] = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE|SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		IDC_STATIC,
		240, 120, 140, 40, hWnd, (DWORD)&hmainpge->fontItem[1]);
	hmainpge->fontHwnd[2] = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		IDC_STATIC,
		80, 300, 140, 40, hWnd, (DWORD)&hmainpge->fontItem[2]);
	hmainpge->fontHwnd[3] = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		IDC_STATIC,
		240, 300, 140, 40, hWnd, (DWORD)&hmainpge->fontItem[3]);
	return 0;
}

static int mainpage_res_free(void)
{
	int i;
	for(i = 0; i < 4; i++){
		unloadBitMap(&hmainpge->iconItem[i]);
		unloadBitMap(&hmainpge->fontItem[i]);
	}
	return 0;
}

static int ChangePage(HWND hWnd, int firstpage)
{
	smRect rect;
	gal_pixel pixel;
	int i;

	mainpage_res_free();

	/* show main push button */
	if(FIRST_PAGE == firstpage) {
	   	if(hmainpge->rightHwnd)
	   		ShowWindow(hmainpge->rightHwnd, SW_HIDE);

	   	if(hmainpge->leftHwnd)
	   		ShowWindow(hmainpge->leftHwnd, SW_SHOW);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1,2);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->iconItem[0]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2,2);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->iconItem[1]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3,2);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->iconItem[2]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4,2);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->iconItem[3]);

	   	 /* set button background */
	   	SetWindowElementAttr(hmainpge->iconHwnd[0], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[0], 40, 40);
	   	SetWindowBkColor (hmainpge->iconHwnd[0], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[1], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap (&hmainpge->iconItem[1], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[1], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[2], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[2], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[2], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[3], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[3], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[3], pixel);
	   	InvalidateRect(hmainpge->iconHwnd[3], NULL, TRUE);

		for (i = 0; i < 4; i++)
			unloadBitMap(&hmainpge->iconItem[i]);

	   	/* set button bitmap */
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 6);
	    getResBmp(ID_MAINPAGE_BTN_ITEM1, BMPTYPE_BASE, &hmainpge->iconItem[0]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 6);
	    getResBmp(ID_MAINPAGE_BTN_ITEM2, BMPTYPE_BASE, &hmainpge->iconItem[1]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 6);
	    getResBmp(ID_MAINPAGE_BTN_ITEM3, BMPTYPE_BASE, &hmainpge->iconItem[2]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 6);
	    getResBmp(ID_MAINPAGE_BTN_ITEM4, BMPTYPE_BASE, &hmainpge->iconItem[3]);

	   	SendMessage (hmainpge->iconHwnd[0], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[0]);
	   	SendMessage (hmainpge->iconHwnd[1], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[1]);
	   	SendMessage (hmainpge->iconHwnd[2], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[2]);
	   	SendMessage (hmainpge->iconHwnd[3], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[3]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 3);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->fontItem[0]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 3);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->fontItem[1]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 3);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->fontItem[2]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 3);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->fontItem[3]);

	   	SendMessage(hmainpge->fontHwnd[0], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[0], 0);
	   	SendMessage(hmainpge->fontHwnd[1], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[1], 0);
	   	SendMessage(hmainpge->fontHwnd[2], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[2], 0);
	   	SendMessage(hmainpge->fontHwnd[3], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[3], 0);

	}else {
	   	if(hmainpge->rightHwnd)
	   		ShowWindow(hmainpge->rightHwnd, SW_SHOW);

	   	if(hmainpge->leftHwnd)
	   		ShowWindow(hmainpge->leftHwnd, SW_HIDE);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1,0);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->iconItem[0]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2,0);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->iconItem[1]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3,0);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->iconItem[2]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4,0);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->iconItem[3]);

	   	 /* set button background */
	   	SetWindowElementAttr(hmainpge->iconHwnd[0], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[0], 40, 40);
	   	SetWindowBkColor (hmainpge->iconHwnd[0], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[1], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap (&hmainpge->iconItem[1], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[1], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[2], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[2], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[2], pixel);

	   	SetWindowElementAttr(hmainpge->iconHwnd[3], WE_LFSKIN_WND_BKGND, 0L);
	   	pixel = GetPixelInBitmap(&hmainpge->iconItem[3], 40, 40);
	   	SetWindowBkColor(hmainpge->iconHwnd[3], pixel);
	   	InvalidateRect(hmainpge->iconHwnd[3], NULL, TRUE);

		for (i = 0; i < 4; i++)
			unloadBitMap(&hmainpge->iconItem[i]);

	   	/* set button bitmap */
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 5);
	    getResBmp(ID_MAINPAGE_BTN_ITEM1, BMPTYPE_BASE, &hmainpge->iconItem[0]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 5);
	    getResBmp(ID_MAINPAGE_BTN_ITEM2, BMPTYPE_BASE, &hmainpge->iconItem[1]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 5);
	    getResBmp(ID_MAINPAGE_BTN_ITEM3, BMPTYPE_BASE, &hmainpge->iconItem[2]);
	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 5);
	    getResBmp(ID_MAINPAGE_BTN_ITEM4, BMPTYPE_BASE, &hmainpge->iconItem[3]);

	   	SendMessage (hmainpge->iconHwnd[0], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[0]);
	   	SendMessage (hmainpge->iconHwnd[1], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[1]);
	   	SendMessage (hmainpge->iconHwnd[2], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[2]);
	   	SendMessage (hmainpge->iconHwnd[3], BM_SETIMAGE, BM_IMAGE_BITMAP, (LPARAM)&hmainpge->iconItem[3]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM1, 1);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM1,BMPTYPE_BASE, &hmainpge->fontItem[0]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM2, 1);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM2,BMPTYPE_BASE, &hmainpge->fontItem[1]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM3, 1);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM3,BMPTYPE_BASE, &hmainpge->fontItem[2]);

	   	setCurrentIconValue(ID_MAINPAGE_BTN_ITEM4, 1);
	   	getResBmp(ID_MAINPAGE_BTN_ITEM4,BMPTYPE_BASE, &hmainpge->fontItem[3]);

	   	SendMessage(hmainpge->fontHwnd[0], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[0], 0);
	   	SendMessage(hmainpge->fontHwnd[1], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[1], 0);
	   	SendMessage(hmainpge->fontHwnd[2], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[2], 0);
	   	SendMessage(hmainpge->fontHwnd[3], STM_SETIMAGE, (WPARAM)&hmainpge->fontItem[3], 0);

	}

	UpdateWindow(hmainpge->mainpgeHwnd, 1);

	return 0;
}

int changeModeIcon(enum ResourceID id, int mode, int index)
{
	//smRect rect;
	RECT rect;
	unloadBitMap(&hmainpge->iconItem[mode]);
	GetClientRect(hmainpge->iconHwnd[mode], &rect);
	InvalidateRect(hmainpge->iconHwnd[mode], &rect, TRUE);

	setCurrentIconValue(id, index);
	getResBmp(id, BMPTYPE_BASE, &hmainpge->iconItem[mode]);
	SendMessage(hmainpge->iconHwnd[mode], STM_SETIMAGE, (WPARAM)&hmainpge->iconItem[mode], 0);

	return 0;
}

static int ChangeIcons(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int mode;
	int index;

	mode = LOWORD(lParam);
	index = LOWORD(wParam);
	switch(mode)
	{
		case ID_MAINPAGE_BTN_ITEM1:
			changeModeIcon(ID_STATUSBAR_ICON_ITEMS, 0, index);
			break;
		case ID_MAINPAGE_BTN_ITEM2:
			changeModeIcon(ID_STATUSBAR_ICON_WIND, 1, index);
			break;
		case ID_MAINPAGE_BTN_ITEM3:
			changeModeIcon(ID_STATUSBAR_ICON_DIRECT, 2, index);
			break;
		default:
			break;
	}

	return 0;
}

int MainBtnCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	maiPageWinData *hmainpge = NULL;
	int items;

	hmainpge = (maiPageWinData*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
	   	case MSG_COMMAND:
       	{
        	int id = LOWORD(wParam);
			switch (id)
          	{
				case ID_MAINPAGE_BTN_LEFT:
				{
					ChangePage(hWnd, hmainpge->first_page);
					hmainpge->first_page = FIRST_PAGE;
					break;
				}
				case ID_MAINPAGE_BTN_RIGHT:
				{
					ChangePage(hWnd, hmainpge->first_page);
					hmainpge->first_page = SECONDE_PAGE;
					break;
				}
				case ID_MAINPAGE_BTN_ITEM1:
				case ID_MAINPAGE_BTN_ITEM2:
				case ID_MAINPAGE_BTN_ITEM3:
				case ID_MAINPAGE_BTN_ITEM4:
				{
					if(hmainpge->first_page == FIRST_PAGE) {
						SendMessage(hmainpge->parentHwnd, MSG_SHOW_STATBAR, wParam, id);
					} else {
						if(ID_MAINPAGE_BTN_ITEM2 == id){
							SendMessage(hmainpge->parentHwnd, MSG_SET_LOCKICON, wParam, 0);
						}
					}
					break;
				}
			}
	   	}
		case MSG_CHANGE_ICONS:
		{
			ChangeIcons(hWnd, wParam, lParam);
			break;
		}
		default:
		{
			break;
		}
	}

	return 0;
}

maiPageWinData* GetMainPgeWinData(void)
{
	if(hmainpge != NULL) {
		return hmainpge;
	}

	return NULL;
}

int MainPgeWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	maiPageWinData *hmainpge = NULL;
	hmainpge = (maiPageWinData*)GetWindowAdditionalData(hWnd);

	/* sm_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n",
	 * hWnd, message, wParam, (s32)lParam);
	 */
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
			CreatMainPgeWin(hWnd);
			hmainpge->first_page = FIRST_PAGE;
			break;
		}
		case MSG_PAINT:
		{
			break;
		}
		case MSG_DESTROY:
			if(hmainpge)
				free(hmainpge);
			break;
		default:
		{
			MainBtnCallBack(hWnd, message, wParam, lParam);
			break;
		}
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND GetMainPgeWin(void)
{
	if(hmainpge != NULL && hmainpge->mainpgeHwnd != 0) {
		return hmainpge->mainpgeHwnd;
	}

	return 0;
}

HWND MainPgeWinInit(HWND hParent)
{
	HWND mHwnd;
	smRect rect;

	hmainpge = (maiPageWinData*)malloc(sizeof(maiPageWinData));
	if(NULL == hmainpge)
	{
		sm_error("malloc hbar window data error\n");
		return -1;
	}

	memset((void *)hmainpge, 0, sizeof(maiPageWinData));
	getResRect(ID_MAINPAGE, &rect);
	hmainpge->mainpgeSize.x = rect.x;
	hmainpge->mainpgeSize.y = rect.y;
	hmainpge->mainpgeSize.w = rect.w;
	hmainpge->mainpgeSize.h = rect.h;

	rect = hmainpge->mainpgeSize;
	mHwnd = CreateWindowEx(WINDOW_MAINPAGE, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_MAINPAGE_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)hmainpge);

	if(mHwnd == HWND_INVALID)
	{
		sm_error("create status bar failed\n");
		return HWND_INVALID;
	}
	setHwnd(WINDOW_MAINPAGE_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);
	hmainpge->parentHwnd = hParent;
	hmainpge->mainpgeHwnd = mHwnd;

	return mHwnd;
}
