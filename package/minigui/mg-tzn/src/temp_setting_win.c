#include <string.h>
#include "temp_setting_win.h"

#define STATBAR_TIMER_UPDATE_REC	101
#define STATBAR_TIMER_UPDATE	102

tempSetWinData *setTemp = NULL;

static int CreatTempSettingWin(HWND hWnd)
{
	smRect rect;
	char buff[8];
    PLOGFONT timefont;

	/* Show temparature and degree */

	setTemp->label_temp_num = CreateWindowEx (CTRL_STATIC,
		 "",
		 WS_CHILD | WS_VISIBLE | SS_BITMAP| SS_NOTIFY |SS_REALSIZEIMAGE, WS_EX_TRANSPARENT,
		 IDC_SET_TEMPER,
		 20, 110, 200, 150, hWnd, (DWORD)0);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%.2d", setTemp->set_temp_num);
    buff[3] = '\0';

	timefont = getLogFont(ID_FONT_TIMES_150);
	SetWindowFont(setTemp->label_temp_num, timefont);
	SetWindowText(setTemp->label_temp_num, buff);
	UpdateWindow(setTemp->label_temp_num, 0);

	getResRect(ID_TEMP_SETTING_DEGREE, &rect);
	setCurrentIconValue(ID_TEMP_SETTING_DEGREE, 0);
	getResBmp(ID_TEMP_SETTING_DEGREE, BMPTYPE_BASE, &setTemp->label_degree_icon);
	setTemp->label_temp_deg = CreateWindowEx (CTRL_STATIC,
		 "",
		 WS_CHILD | SS_BITMAP| WS_VISIBLE|SS_REALSIZEIMAGE, WS_EX_TRANSPARENT,
		 ID_TEMP_SETTING_DEGREE,
		 rect.x, rect.y, rect.w, rect.h, hWnd, (DWORD)&setTemp->label_degree_icon);

	/* Create up/down button */
	getResRect(ID_TEMP_SETTING_UP, &rect);
	setCurrentIconValue(ID_TEMP_SETTING_UP, 0);
	getResBmp(ID_TEMP_SETTING_UP, BMPTYPE_BASE, &setTemp->label_up_icon);
	setTemp->labelTempUp = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE | SS_NOTIFY |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		ID_TEMP_SETTING_UP,
		rect.x, rect.y, rect.w, rect.h,  hWnd, (DWORD)&setTemp->label_up_icon);

	getResRect(ID_TEMP_SETTING_DOWN, &rect);
	setCurrentIconValue(ID_TEMP_SETTING_DOWN, 0);
	getResBmp(ID_TEMP_SETTING_DOWN, BMPTYPE_BASE, &setTemp->label_down_icon);
	setTemp->labelTempDown = CreateWindowEx (CTRL_STATIC,
		"",
		WS_CHILD | SS_BITMAP| WS_VISIBLE | SS_NOTIFY |SS_REALSIZEIMAGE |SS_CENTERIMAGE, WS_EX_TRANSPARENT,
		ID_TEMP_SETTING_DOWN,
		rect.x, rect.y, rect.w, rect.h,  hWnd, (DWORD)&setTemp->label_down_icon);

	if(setTemp->labelTempUp)
		ShowWindow (setTemp->labelTempUp, SW_HIDE);

	if(setTemp->labelTempDown)
		ShowWindow (setTemp->labelTempDown, SW_HIDE);

	return 0;
}


int TempSetUpdate(HWND hWnd)
{
	char buff[8];
    PLOGFONT timefont;

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%.2d", setTemp->set_temp_num);
    buff[3] = '\0';

	timefont = getLogFont(ID_FONT_TIMES_150);
	SetWindowFont(setTemp->label_temp_num, timefont);
	SetWindowText(setTemp->label_temp_num, buff);
	UpdateWindow(setTemp->label_temp_num, 0);
	return 0;
}

int TempSetCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int index;
	RECT rect;
	tempSetWinData *setTemp = NULL;

	index = (int)wParam;
	setTemp = (tempSetWinData*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
        case MSG_COMMAND:
       	{
        	int id = LOWORD(wParam);
			switch (id)
          	{
				case IDC_SET_TEMPER:
				{
					if (0 == setTemp->set_temp_flag) {
						if(setTemp->labelTempUp)
							ShowWindow (setTemp->labelTempUp, SW_SHOW);
						if(setTemp->labelTempDown)
							ShowWindow (setTemp->labelTempDown, SW_SHOW);
							setTemp->set_temp_flag = 1;
					} else {
						if(setTemp->labelTempUp)
							ShowWindow (setTemp->labelTempUp, SW_HIDE);
						if(setTemp->labelTempDown)
							ShowWindow (setTemp->labelTempDown, SW_HIDE);
						setTemp->set_temp_flag = 0;
					}
					break;
				}
				case ID_TEMP_SETTING_UP:
				{
					setTemp->set_temp_num++;
					if(setTemp->set_temp_num > 50)
						setTemp->set_temp_num = 50;
					TempSetUpdate(hWnd);
					break;
				}
				case ID_TEMP_SETTING_DOWN:
				{
					setTemp->set_temp_num--;
					if(setTemp->set_temp_num < 10)
						setTemp->set_temp_num = 10;
					TempSetUpdate(hWnd);
					break;
				}
				default:
					break;
                }
            }
            break;

		default:
		{
			break;
		}
	}

	return 0;
}

tempSetWinData* GetTempSetWinData(void)
{
	if(setTemp != NULL) {
		return setTemp;
	}

	return NULL;
}

int TempSetWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	tempSetWinData *setTemp = NULL;
	setTemp = (tempSetWinData*)GetWindowAdditionalData(hWnd);

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
			setTemp->set_temp_num = 23;
			color = getResColor(ID_STATUSBAR, COLOR_BGC);
			SetWindowBkColor(hWnd, color);
			CreatTempSettingWin(hWnd);
			break;
		}
		case MSG_DESTROY:
		{
			if(setTemp)
				free(setTemp);
			break;
		}
		default:
		{
			TempSetCallBack(hWnd, message, wParam, lParam);
			break;
		}
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND GetTempSetWin(void)
{
	if(setTemp != NULL && setTemp->tempSetHwnd != 0) {
		return setTemp->tempSetHwnd;
	}

	return 0;
}

HWND TempSetWinInit(HWND hParent)
{
	HWND mHwnd;
	smRect rect;

	setTemp = (tempSetWinData*)malloc(sizeof(tempSetWinData));
	if(NULL == setTemp)
	{
		sm_error("malloc hbar window data error\n");
		return -1;
	}
	memset((void *)setTemp, 0, sizeof(tempSetWinData));

	getResRect(ID_TEMP_SETTING, &rect);

	setTemp->tempSetSize.x = rect.x;
	setTemp->tempSetSize.y = rect.y;
	setTemp->tempSetSize.w = rect.w;
	setTemp->tempSetSize.h = rect.h;
	rect = setTemp->tempSetSize;
	mHwnd = CreateWindowEx(WINDOW_SETTEMP, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_SETTEMP_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)setTemp);

	if(mHwnd == HWND_INVALID)
	{
		sm_error("Create temperature set window failed\n");
		return HWND_INVALID;
	}

	setHwnd(WINDOW_SETTEMP_ID, mHwnd);
	//ShowWindow(mHwnd, SW_HIDE);
	ShowWindow(mHwnd, SW_SHOW);

	setTemp->parentHwnd = hParent;
	setTemp->tempSetHwnd = mHwnd;

	return mHwnd;
}

