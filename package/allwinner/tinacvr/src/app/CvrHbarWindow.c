#include "CvrHbarWindow.h"
#include "CvrMainWindow.h"
#include "CvrRecordWindow.h"
#include "CvrResource.h"

#define HBAR_TIMER_UPDATE_REC_TIME	101 //更新录像时间
#define HBAR_TIMER_UPDATE_HBAR_INFO	102 //更新状态栏信息 : 时间

HbarWinDataType *hbar=NULL;

s32 createSTBWidgets(HWND hParent)
{
	s32 index;
	int  retval;
	HWND retWnd;
	RGB    rgbs;
	gal_pixel  color;
	CvrRectType rect;
	UserDataType usrData;
	GetUserData(&usrData);

	setCurrentIconValue(ID_STATUSBAR_ICON_AUDIO, usrData.voice_en);
	getResBmp(ID_STATUSBAR_ICON_AUDIO, BMPTYPE_BASE, &hbar->audioBmp);
	getResRect(ID_STATUSBAR_ICON_AUDIO,&rect);
	hbar->audioHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_ICON_AUDIO,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&hbar->audioBmp));
	if(hbar->audioHwnd == HWND_INVALID)
	{
		cvr_error("create audio Hwnd failed\n");
		return -1;
	}

	if(sdcard_is_exist())
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_TFCARD, 1);
	}
	else
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_TFCARD, 0);
	}
	getResBmp(ID_STATUSBAR_ICON_TFCARD, BMPTYPE_BASE, &hbar->tfcardBmp);
	getResRect(ID_STATUSBAR_ICON_TFCARD,&rect);
	hbar->tfcardHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_ICON_TFCARD,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&hbar->tfcardBmp));
	if(hbar->tfcardHwnd == HWND_INVALID)
	{
		cvr_error("create tfcard Hwnd failed\n");
		return -1;
	}

	if(power_is_charging() != POWER_BAT_ONLY)//更新为充电图标
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT2, 1);
		getResBmp(ID_STATUSBAR_ICON_BAT2, BMPTYPE_BASE, &hbar->batBmp);
	}
	else
	{
		switch(power_get_battery_level())
		{
			case POWER_LEVEL_0:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT,0);
				getResBmp(ID_STATUSBAR_ICON_BAT, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			case POWER_LEVEL_1:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT,1);
				getResBmp(ID_STATUSBAR_ICON_BAT, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			case POWER_LEVEL_2:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT1,0);
				getResBmp(ID_STATUSBAR_ICON_BAT1, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			case POWER_LEVEL_3:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT1,1);
				getResBmp(ID_STATUSBAR_ICON_BAT1, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			case POWER_LEVEL_4:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT2,0);
				getResBmp(ID_STATUSBAR_ICON_BAT2, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			case POWER_LEVEL_5:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT2, 1);
				getResBmp(ID_STATUSBAR_ICON_BAT2, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}

			default:
			{
				setCurrentIconValue(ID_STATUSBAR_ICON_BAT,0);
				getResBmp(ID_STATUSBAR_ICON_BAT, BMPTYPE_BASE, &hbar->batBmp);
				break;
			}
		}

	}

	getResRect(ID_STATUSBAR_ICON_BAT,&rect);
	hbar->batHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_ICON_BAT,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&hbar->batBmp));
	if(hbar->batHwnd == HWND_INVALID)
	{
		cvr_error("create bat Hwnd failed\n");
		return -1;
	}

	getResBmp(ID_STATUSBAR_ICON_WINDOWPIC, BMPTYPE_WINDOWPIC_RECORDPREVIEW, &hbar->uvcBmp);
	getResRect(ID_STATUSBAR_ICON_WINDOWPIC,&rect);
	hbar->uvcHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_ICON_WINDOWPIC,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&hbar->uvcBmp));
	if(hbar->uvcHwnd == HWND_INVALID)
	{
		cvr_error("create uvc Hwnd failed\n");
		return -1;
	}

	setCurrentIconValue(ID_STATUSBAR_ICON_LOCK, 0);
	getResBmp(ID_STATUSBAR_ICON_LOCK, BMPTYPE_BASE, &hbar->lockBmp);
	getResRect(ID_STATUSBAR_ICON_LOCK,&rect);

	cvr_debug("(%d, %d, %d, %d)\n", rect.x, rect.y, rect.w, rect.h);
	hbar->lockHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE ,
			WS_EX_TRANSPARENT,
			ID_STATUSBAR_ICON_LOCK,
			rect.x,rect.y,rect.w,rect.h,
			hParent, (DWORD)(&hbar->lockBmp));
	if(hbar->lockHwnd == HWND_INVALID)
	{
		cvr_error("create uvc Hwnd failed\n");
		return -1;
	}

	retval = getResRect(ID_STATUSBAR_LABEL1, &rect);
	if(retval < 0)
	{
		cvr_error("get ID_STATUSBAR_LABEL1 failed\n");
		return -1;
	}
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_SIMPLE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_STATUSBAR_LABEL1,
			rect.x, rect.y, rect.w, rect.h,
			hParent, 0);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create status bar window name failed\n");
		return -1;
	}
	SetWindowText(retWnd, "");
	color = getResColor(ID_STATUSBAR_LABEL1, COLOR_FGC);
	Pixel2RGBA(HDC_SCREEN,color,&rgbs.r, &rgbs.g, &rgbs.b, &rgbs.a);
	color = MakeRGBA (rgbs.r, rgbs.g, rgbs.b, rgbs.a);
	SetWindowElementAttr(retWnd, WE_FGC_WINDOW, color);
	UpdateWindow(retWnd, 0);
	hbar->label1Hwnd = retWnd;

	retval = getResRect(ID_STATUSBAR_LABEL2, &rect);
	if(retval < 0)
	{
		cvr_error("get ID_STATUSBAR_LABEL2 failed\n");
		return -1;
	}
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_SIMPLE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_STATUSBAR_LABEL2,
			rect.x, rect.y, rect.w, rect.h,
			hParent, 0);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create status bar window name failed\n");
		return -1;
	}
	color = getResColor(ID_STATUSBAR_LABEL2, COLOR_FGC);
	Pixel2RGBA(HDC_SCREEN,color,&rgbs.r, &rgbs.g, &rgbs.b, &rgbs.a);
	color = MakeRGBA (rgbs.r, rgbs.g, rgbs.b, rgbs.a);
	SetWindowElementAttr(retWnd, WE_FGC_WINDOW, color);
	UpdateWindow(retWnd, 0);
	hbar->label2Hwnd = retWnd;

	{
		struct tm u_time;
		char buff[64];

		get_local_time(&u_time);

		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", \
				u_time.tm_year, u_time.tm_mon, u_time.tm_mday, u_time.tm_hour, u_time.tm_min, u_time.tm_sec);

		SetWindowText(retWnd, buff);
		UpdateWindow(retWnd, 0);
	}

	retval = getResRect(ID_STATUSBAR_LABEL_RESERVE, &rect);
	if(retval < 0)
	{
		cvr_error("get ID_STATUSBAR_LABEL_RESERVE failed\n");
		return -1;
	}
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_SIMPLE ,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_STATUSBAR_LABEL_RESERVE,
			rect.x, rect.y, rect.w, rect.h,
			hParent, 0);
	if(retWnd == HWND_INVALID)
	{
		cvr_error("create status bar reserved label failed\n");
		return -1;
	}
	color = getResColor(ID_STATUSBAR_LABEL_RESERVE, COLOR_FGC);
	Pixel2RGBA(HDC_SCREEN,color,&rgbs.r, &rgbs.g, &rgbs.b, &rgbs.a);
	color = MakeRGBA (rgbs.r, rgbs.g, rgbs.b, rgbs.a);
	SetWindowElementAttr(retWnd, WE_FGC_WINDOW, color);
	hbar->label3Hwnd = retWnd;

	return 0;
}

static void __uint2str(s32 input, char * str)
{
	if(input == 0)
	{
		str[0] = '0';
		str[1] = '0';
		str[2] = '\0';
	}
    else
    {
        str[0] = input / 10 + '0';
        str[1] = input % 10 + '0';
        str[2] = '\0';
    }
}

void  seconds_to_timestr(s32 seconds, char * str)
{
    s32   sec, min, hour;

	if(seconds == -1)
	{
		seconds = 0;
	}

    hour = seconds / (60 * 60);
    min  = (seconds - (hour * 60 * 60)) / 60;
    sec  = seconds - (hour * 60 * 60) - (min * 60);

  //  __uint2str(hour, str);
  //  str[2] = ':';

    __uint2str(min, str);
    str[2] = ':';

    __uint2str(sec, str + 3);
    str[5] = '\0';
}


bool HbarTimerID1Proc(HWND hWnd, int id, DWORD lm)	// 不要返回CVR_FALSE，否则会删除定时器
{
	struct tm u_time;
	char     buff[64];

	get_local_time(&u_time);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", \
			u_time.tm_year, u_time.tm_mon, u_time.tm_mday, u_time.tm_hour, u_time.tm_min, u_time.tm_sec);

	SetWindowText(hbar->label2Hwnd, buff);
	UpdateWindow(hbar->label2Hwnd, 0);

	return CVR_TRUE;
}

bool HbarTimerID2Proc(HWND hWnd, int id, DWORD tm)	// 不要返回CVR_FALSE，否则会删除定时器
{
	char    str[32];
	RecordWinDataType *rcd = NULL;

	rcd = GetRecordWinData();
	if(rcd == NULL)
	{
		return CVR_TRUE;
	}

	// 在抓拍的时候，不显示录像时间
	if(!rcd->ptData.bTakePicture)
	{
		memset(str,0,sizeof(str));
		seconds_to_timestr(GetRecordTime(),str);

		SetWindowText(hbar->label1Hwnd, str);
		UpdateWindow(hbar->label1Hwnd, 0);
	}

	return CVR_TRUE;
}

HbarWinDataType* Gethbar(void)
{
	if(hbar != NULL)
	{
		return hbar;
	}

	return NULL;
}

void GetbatResBmp(HbarWinDataType *hbar,s32 index)
{
	if(index == 0)
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT, 0);//index
		getResBmp(ID_STATUSBAR_ICON_BAT, BMPTYPE_BASE, &hbar->batBmp);
	}
	else if(index == 1)
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT, 1);
		getResBmp(ID_STATUSBAR_ICON_BAT, BMPTYPE_BASE, &hbar->batBmp);
	}
	else if(index == 2)
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT1, 0);
		getResBmp(ID_STATUSBAR_ICON_BAT1, BMPTYPE_BASE, &hbar->batBmp);
	}
	else if(index == 3)
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT1, 1);
		getResBmp(ID_STATUSBAR_ICON_BAT1, BMPTYPE_BASE, &hbar->batBmp);
	}
	else if(index == 4)
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT2, 0);
		getResBmp(ID_STATUSBAR_ICON_BAT2, BMPTYPE_BASE, &hbar->batBmp);
	}
	else if(index == 5)// 充电图标
	{
		setCurrentIconValue(ID_STATUSBAR_ICON_BAT2, 1);
		getResBmp(ID_STATUSBAR_ICON_BAT2, BMPTYPE_BASE, &hbar->batBmp);
	}
}

s32 HbarkeyProc(HbarWinDataType *hbar, s32 keyCode, s32 isLongPress)
{
	switch(keyCode)
	{
	case CVR_KEY_OK:
		break;

	case CVR_KEY_MODE:
		cvr_debug("checking mode key\n");
		break;

	case CVR_KEY_LEFT:
		break;

	case CVR_KEY_RIGHT:
		break;

    case CVR_KEY_MENU:
        break;

	default:
		break;

	}

	return WINDOW_HBAR_ID;
}

int HbarCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	s32 index;
	RECT rect;
	HbarWinDataType *hbar = NULL;

	index = (s32)wParam;
	hbar = (HbarWinDataType*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
		case MSG_HBAR_UPDATAE_UVC:
		{
			unloadBitMap(&hbar->uvcBmp);
			GetClientRect(hbar->uvcHwnd, &rect);
			InvalidateRect(hbar->uvcHwnd, &rect, true);

			if(index == 0)
			{
				getResBmp(ID_STATUSBAR_ICON_WINDOWPIC, BMPTYPE_WINDOWPIC_RECORDPREVIEW, &hbar->uvcBmp);
			}
			else if(index == 1)
			{
				getResBmp(ID_STATUSBAR_ICON_WINDOWPIC, BMPTYPE_WINDOWPIC_PHOTOGRAPH, &hbar->uvcBmp);
			}
			else if(index == 2)
			{
				getResBmp(ID_STATUSBAR_ICON_WINDOWPIC, BMPTYPE_WINDOWPIC_PLAYBACKPREVIEW, &hbar->uvcBmp);
			}
			else if(index == 3)
			{
				getResBmp(ID_STATUSBAR_ICON_WINDOWPIC, BMPTYPE_WINDOWPIC_MENU, &hbar->uvcBmp);
			}

			SendMessage(hbar->uvcHwnd, STM_SETIMAGE, (WPARAM)&hbar->uvcBmp, 0);
			break;
		}

		case MSG_HBAR_UPDATAE_AUDIO:
		{
			unloadBitMap(&hbar->audioBmp);
			GetClientRect(hbar->audioHwnd, &rect);
			InvalidateRect(hbar->audioHwnd, &rect, true);

			setCurrentIconValue(ID_STATUSBAR_ICON_AUDIO, index);
			getResBmp(ID_STATUSBAR_ICON_AUDIO, BMPTYPE_BASE, &hbar->audioBmp);
			SendMessage(hbar->audioHwnd, STM_SETIMAGE, (WPARAM)&hbar->audioBmp, 0);

			break;
		}

		case MSG_HBAR_UPDATAE_LABLE3:
		{
			char name[64] = "";
			s32 index = (s32)wParam;
			ThumbFileInfo *info = (ThumbFileInfo *)lParam;

			if(index == 1)
			{
				cvr_debug("%s(%02d/%02d)\n", info->curFileName, info->curItem, info->totalItem);
				snprintf(name, sizeof(name), "%s(%2d/%2d)", info->curFileName, info->curItem, info->totalItem);
				SetWindowText(hbar->label3Hwnd, name);
				UpdateWindow(hbar->label3Hwnd, 0);
			}
			else if(index == 0)
			{
				SetWindowText(hbar->label3Hwnd, "");
				UpdateWindow(hbar->label3Hwnd, 0);
			}

			break;
		}

		case MSG_HBAR_UPDATAE_RECINFO:
		{
			const char* ptr = NULL;

			if(index == 0) //开始录像计时
			{
				SetWindowText(hbar->label1Hwnd, "00:00");
				UpdateWindow(hbar->label1Hwnd, 0);
				SetTimerEx(hWnd, HBAR_TIMER_UPDATE_REC_TIME, 20, HbarTimerID2Proc);
			}
			else if(index == 1) //停止录像计时
			{
				KillTimer(hWnd, HBAR_TIMER_UPDATE_REC_TIME);
				SetWindowText(hbar->label1Hwnd, "");
				UpdateWindow(hbar->label1Hwnd, 0);
			}
			else if(index == 3)
			{
				ptr = getLabel(LANG_LABEL_TAKE_PICTURE);
				SetWindowText(hbar->label1Hwnd, ptr);
				UpdateWindow(hbar->label1Hwnd, 0);
			}
			else if(index == 4)
			{
				SetWindowText(hbar->label1Hwnd, "");
				UpdateWindow(hbar->label1Hwnd, 0);
			}
			break;
		}

		case MSG_HBAR_UPDATAE_TFCARD:
		{
			unloadBitMap(&hbar->tfcardBmp);
			GetClientRect(hbar->tfcardHwnd, &rect);
			InvalidateRect(hbar->tfcardHwnd, &rect, true);

			setCurrentIconValue(ID_STATUSBAR_ICON_TFCARD, index);
			getResBmp(ID_STATUSBAR_ICON_TFCARD, BMPTYPE_BASE, &hbar->tfcardBmp);
			SendMessage(hbar->tfcardHwnd, STM_SETIMAGE, (WPARAM)&hbar->tfcardBmp, 0);
			break;
		}

		case MSG_HBAR_UPDATAE_LOCK:
		{
			unloadBitMap(&hbar->lockBmp);
			GetClientRect(hbar->lockHwnd, &rect);
			InvalidateRect(hbar->lockHwnd, &rect, true);

			setCurrentIconValue(ID_STATUSBAR_ICON_LOCK, index);
			getResBmp(ID_STATUSBAR_ICON_LOCK, BMPTYPE_BASE, &hbar->lockBmp);
			SendMessage(hbar->lockHwnd, STM_SETIMAGE, (WPARAM)&hbar->lockBmp, 0);

			break;

		}

		case MSG_HBAR_UPDATAE_BATLEVEL:
		{
			unloadBitMap(&hbar->batBmp);
			GetClientRect(hbar->batHwnd, &rect);
			InvalidateRect(hbar->batHwnd, &rect, true);
			GetbatResBmp(hbar,index);
			SendMessage(hbar->batHwnd, STM_SETIMAGE, (WPARAM)&hbar->batBmp, 0);
			break;
		}

		default:
		{
			break;
		}

	}

	return 0;

}


int HbarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)

{

	HDC hdc;

	HbarWinDataType *hbar = NULL;

	hbar = (HbarWinDataType*)GetWindowAdditionalData(hWnd);


	//cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, message, wParam, (s32)lParam);

	switch(message)

	{

		case MSG_FONTCHANGED:

		{

			HWND hChild;

			PLOGFONT pLogFont;
			pLogFont = GetWindowFont(hWnd);

			if(!pLogFont)

			{

				cvr_error("invalid logFont\n");

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

			color = getResColor(ID_STATUSBAR, COLOR_BGC);	// argb

			SetWindowBkColor(hWnd, color);

			createSTBWidgets(hWnd);

			SetTimerEx(hWnd, HBAR_TIMER_UPDATE_HBAR_INFO, 10, HbarTimerID1Proc);

			break;

		}

		case MSG_TIMER:
			break;

		case MSG_DESTROY:
			KillTimer(hWnd, HBAR_TIMER_UPDATE_HBAR_INFO);
			break;

		case MSG_CVR_KEY:
			return HbarkeyProc(hbar, wParam, lParam);
			break;

		case MSG_WIN_RESUME:
			break;

		case MSG_WIN_SUSPEND:
			break;

		default:
		{
			HbarCallBack(hWnd, message, wParam, lParam);
			break;
		}
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND GetHbarWinHwnd(void)
{
	if(hbar!=NULL && hbar->HbarHwnd!=0)
	{
		return hbar->HbarHwnd;
	}

	return 0;
}

HWND HbarWinInit(HWND hParent)
{
	HWND mHwnd;
	CvrRectType rect;

	hbar = (HbarWinDataType*)malloc(sizeof(HbarWinDataType));
	if(NULL == hbar)
	{
		cvr_error("malloc hbar window data error\n");
		return -1;
	}
	memset((void *)hbar, 0, sizeof(HbarWinDataType));

	getResRect(ID_STATUSBAR,&rect);

	hbar->hbarSize.x = rect.x;
	hbar->hbarSize.y = rect.y;
	hbar->hbarSize.w = rect.w;
	hbar->hbarSize.h = rect.h;

	rect = hbar->hbarSize;
	mHwnd = CreateWindowEx(WINDOW_HBAR, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_HBAR_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)hbar);

	if(mHwnd == HWND_INVALID)
	{
		cvr_error("create status bar failed\n");
		return HWND_INVALID;
	}

	setHwnd(WINDOW_HBAR_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);


	hbar->ParentHwnd = hParent;
	hbar->HbarHwnd = mHwnd;

	return mHwnd;
}
