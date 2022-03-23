#include "CvrMainWindow.h"
#include "CvrHbarWindow.h"
#include "CvrRecordWindow.h"
#include "CvrMenuWindow.h"
#include "CvrUsbWindow.h"
#include <pthread.h>
#include "CvrMenuList.h"
#include "middle_ware.h"

#define ID_TIMER_KEY	100		// main timer id
#define LONG_PRESS_TIME		50	// speed unit 10ms

HWND mainWinHwnd;
MainWinDataType *mwd=NULL;

// define && function
void updateWindowFont(void)
{
	HWND hChild;

	hChild = GetNextChild(GetMainWinHwnd(), 0);
	while( hChild && (hChild != HWND_INVALID) )
	{
		SetWindowFont(hChild, GetWindowFont(GetMainWinHwnd()));
		hChild = GetNextChild(GetMainWinHwnd(), hChild);
	}
}

HWND WindowID2Window(MainWinDataType *mwd, WindowIDType ID)
{
	HWND hwnd = HWND_INVALID;

	switch(ID)
	{
	case WINDOW_RECORD_ID:
		hwnd= mwd->RecordWinHwnd;

		break;
	case WINDOW_MENU_ID:
		hwnd= mwd->MenuWinHwnd;

		break;
	case WINDOW_USB_ID:
		hwnd= mwd->UsbWinHwnd;

		break;
	case WINDOW_HBAR_ID:
		hwnd= mwd->HbarWinHwnd;

		break;
	case WINDOW_MAIN_ID:
		hwnd= mwd->MainWinHwnd;
		break;

	default:
		hwnd = NULL;
		cvr_error("invalid window id: %d\n", ID);
		break;

	}

	return hwnd;
}

void SelfCtrlWinInit(HWND hWnd, MainWinDataType *mwd)
{
	mwd->RecordWinHwnd = RecordWinInit(hWnd);
	//mwd->MenuWinHwnd = MenuWinInit(hWnd);
	mwd->UsbWinHwnd  = UsbWinInit(hWnd);
	mwd->HbarWinHwnd = HbarWinInit(hWnd);
}

void SelfCtrlWinUninit(HWND hWnd, MainWinDataType *mwd)
{
	HWND curHwnd;
	HWND hbarHwnd;
	WindowIDType curID;

	curID = mwd->curWindowID;
	curHwnd = WindowID2Window(mwd, curID);
	hbarHwnd = WindowID2Window(mwd, WINDOW_HBAR_ID);

	if(curID==WINDOW_RECORD_ID)
	{
		ShowWindow(curHwnd, SW_HIDE);
		SendMessage(curHwnd, MSG_WIN_SUSPEND, 0, 0);
		ShowWindow(hbarHwnd, SW_HIDE);
		SendMessage(hbarHwnd, MSG_WIN_SUSPEND, 0, 0);
	}
	else if(curID==WINDOW_MENU_ID)
	{
		ShowWindow(curHwnd, SW_HIDE);
		SendMessage(curHwnd, MSG_WIN_SUSPEND, 0, 0);
		ShowWindow(hbarHwnd, SW_HIDE);
		SendMessage(hbarHwnd, MSG_WIN_SUSPEND, 0, 0);
		SendMessage(GetRecordWinHwnd(), MSG_WIN_SUSPEND, 3, 0);		// 菜单场景也要关预览
	}
	else if(curID==WINDOW_USB_ID)
	{
		ShowWindow(curHwnd, SW_HIDE);
		SendMessage(curHwnd, MSG_WIN_SUSPEND, 0, 0);
	}
}

void ChangeWindow(MainWinDataType *mwd, WindowIDType oldWinID, WindowIDType newWinID)
{
	u32 ret;
	HWND toHwnd;
	HWND fromHwnd;
	HWND hbarHwnd;
	static bool bMenuExit = CVR_FALSE;
	static bool isMENUback = 0 ;
	// 第一次进的时候才去初始化菜单窗口
	if(!bMenuExit && (newWinID==WINDOW_MENU_ID))
	{
		bMenuExit = CVR_TRUE;
		mwd->MenuWinHwnd = MenuWinInit(mwd->MainWinHwnd);
		cvr_debug("init menu window\n");
	}

	toHwnd = WindowID2Window(mwd, newWinID);
	if(toHwnd == HWND_INVALID) {
		cvr_error("invalid toHwnd\n");
		return;
	}
	fromHwnd = WindowID2Window(mwd, oldWinID);
	hbarHwnd = WindowID2Window(mwd, WINDOW_HBAR_ID);

	if(oldWinID==WINDOW_MAIN_ID && newWinID==WINDOW_USB_ID)
	{
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(toHwnd, SW_SHOW);
	}
	else if(oldWinID==WINDOW_MAIN_ID && newWinID==WINDOW_RECORD_ID)
	{
		SendNotifyMessage(hbarHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(hbarHwnd, SW_SHOW);
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(toHwnd, SW_SHOW);
	}
	else if(oldWinID==WINDOW_RECORD_ID && newWinID==WINDOW_USB_ID)
	{
		ShowWindow(fromHwnd, SW_HIDE);
		SendNotifyMessage(fromHwnd, MSG_WIN_SUSPEND, 0, 0);
		ShowWindow(hbarHwnd, SW_HIDE);
		SendNotifyMessage(hbarHwnd, MSG_WIN_SUSPEND, 0, 0);
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(toHwnd, SW_SHOW);
	}
	else if(oldWinID==WINDOW_MENU_ID && newWinID==WINDOW_USB_ID)
	{
		ShowWindow(fromHwnd, SW_HIDE);
		SendNotifyMessage(fromHwnd, MSG_WIN_SUSPEND, 0, 0);
		ShowWindow(hbarHwnd, SW_HIDE);
		SendNotifyMessage(hbarHwnd, MSG_WIN_SUSPEND, 0, 0);
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(toHwnd, SW_SHOW);
		isMENUback = 1;
	}
	else if(oldWinID==WINDOW_RECORD_ID && newWinID==WINDOW_MENU_ID)
	{
		ShowWindow(fromHwnd, SW_HIDE);
		SendNotifyMessage(fromHwnd, MSG_WIN_SUSPEND, 1, 0);	// 不开关预览
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(toHwnd, SW_SHOW);
	}
	else if(oldWinID==WINDOW_MENU_ID && newWinID==WINDOW_RECORD_ID)
	{
		ShowWindow(fromHwnd, SW_HIDE);
		SendNotifyMessage(fromHwnd, MSG_WIN_SUSPEND, 0, 0);
		SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 1, 0);
		ShowWindow(toHwnd, SW_SHOW);
	}
	else if(oldWinID==WINDOW_USB_ID && newWinID==WINDOW_RECORD_ID)
	{
		ShowWindow(fromHwnd, SW_HIDE);
		SendNotifyMessage(fromHwnd, MSG_WIN_SUSPEND, 0, 0);
		SendNotifyMessage(hbarHwnd, MSG_WIN_RESUME, 0, 0);
		ShowWindow(hbarHwnd, SW_SHOW);
		if(isMENUback)
		{
			SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 1, 0);
			isMENUback=0;
		}
		else
		{
			SendNotifyMessage(toHwnd, MSG_WIN_RESUME, 0, 0);
		}
		ShowWindow(toHwnd, SW_SHOW);
	}
	else
	{
		return;
	}

	cvr_debug("oldWinID=%d, newWinID=%d\n", oldWinID, newWinID);
	mwd->oldWindowID = oldWinID;
	mwd->curWindowID = newWinID;

	return;
}

/*
void DispatchKeyEvent(MainWinDataType *mwd, HWND curHwnd, s32 keyCode, s32 isLongPress)
{
	WindowIDType windowID;
	if(curHwnd != HWND_INVALID)
	{
		windowID = SendMessage(curHwnd, MSG_CVR_KEY, keyCode, isLongPress);
		cvr_debug("windowID=%d\n", windowID);
		if(windowID!=WINDOW_HBAR_ID && windowID!=mwd->curWindowID)
		{
			ChangeWindow(mwd, mwd->curWindowID, windowID);
		}
	}
}
*/

void DispatchKeyEvent(MainWinDataType *mwd, HWND curHwnd, s32 keyCode, s32 isLongPress)
{
	WindowIDType windowID;
	if(curHwnd != HWND_INVALID)
	{
		windowID = SendMessage(curHwnd, MSG_CVR_KEY, keyCode, isLongPress);
		if(windowID!=WINDOW_HBAR_ID && windowID!=mwd->curWindowID && (windowID == WINDOW_MENU_ID || mwd->curWindowID == WINDOW_MENU_ID))
		{
			ChangeWindow(mwd, mwd->curWindowID, windowID);
		}
	}
}

void *MainThreadProc(void *arg)
{
	UserDataType usrData;
	static s32 nLowPowerCnt = 0;
	static s32 PowerbatOnlyCnt = 0;
	static power_type_t newPwrStus = POWER_BAT_ONLY;
	static power_level_t newPwrLevel = POWER_LEVEL_3;
	static bool newTfcardStus = CVR_FALSE;
	int usbCnt = 0;
	power_type_t usbStaOld;
	power_type_t usbStaCur;

	MainWinDataType *mwd = (MainWinDataType *)arg;

	mwd->bThreadRun = CVR_TRUE;
	cvr_debug("main thread begin\n");
	while (1)
	{
		if(!mwd->bThreadRun)
		{
			cvr_debug("main thread end\n");
			pthread_exit((void*)1);
			sleep(1);
			cvr_error("mainthread end\n");
		}
		//cvr_debug("main thread run\n");
		usleep(100*1000);		// 延时100ms

		// usb拔插检测
		usbStaCur = power_is_charging();
		newPwrStus = mwd->powerStaus;
		if(usbStaCur == usbStaOld){
			if(usbCnt > 3){
				newPwrStus = usbStaCur;
				usbCnt = 0;
			}
			usbCnt++;
		}else{
			usbStaOld = usbStaCur;
			usbCnt = 0;
		}

		//cvr_debug("###################################_10000\n");
		//cvr_debug("%d, %d\n", newPwrStus, mwd->powerStaus);
		if(newPwrStus != mwd->powerStaus)
		{
			if(newPwrStus==POWER_PC_LINK)
			{
				SendNotifyMessage(mwd->MainWinHwnd, MSG_USB_PLUGIN, 0, 0);
			}
			else if(mwd->powerStaus==POWER_PC_LINK)
			{
				SendNotifyMessage(mwd->MainWinHwnd, MSG_USB_PLUGOUT, 0, 0);
			}
			mwd->powerStaus = newPwrStus;

			if(newPwrStus != POWER_BAT_ONLY)//更新为充电图标
			{
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_BATLEVEL, 5, 0);
			}
			else
			{
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_BATLEVEL, mwd->powerLevel, 0);
			}
		}

		//更新电池等级
		if(mwd->powerStaus == POWER_BAT_ONLY)
		{
			newPwrLevel = power_get_battery_level();

			if(newPwrLevel != mwd->powerLevel)
			{
				mwd->powerLevel = newPwrLevel;
				SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_BATLEVEL, mwd->powerLevel, 0);
			}
		}

		//cvr_debug("(%d, %d)\n", mwd->powerStaus, PowerbatOnlyCnt);
		if(mwd->powerStaus == POWER_BAT_ONLY)
		{
			PowerbatOnlyCnt = (PowerbatOnlyCnt>=100) ? 100 : (PowerbatOnlyCnt+1);
			if(PowerbatOnlyCnt==100-1)
			{

				SendNotifyMessage(mwd->MainWinHwnd, MSG_CLOSE, 0, 0);
			}

			if(mwd->powerLevel <= POWER_LEVEL_0)
			{
				nLowPowerCnt = (nLowPowerCnt>=100) ? 100 : (nLowPowerCnt+1);
				if(nLowPowerCnt == 100-1)
				{
					SendNotifyMessage(mwd->MainWinHwnd, MSG_POWER_LOW, 0, 0);
				}
			}
			else
			{
				nLowPowerCnt = 0;
			}
		}
		else
		{
			PowerbatOnlyCnt=0;
		}

		//if(format_is_correct() == 1)
		if(sdcard_is_mount_correct() == 1)
		{
			newTfcardStus = CVR_TRUE;
		}
		else
		{
			newTfcardStus = CVR_FALSE;
		}

		if(newTfcardStus != mwd->bTfcardExist)
		{
			if(newTfcardStus)
			{
				//sdcard_fs_flex();
				SendNotifyMessage(mwd->MainWinHwnd, MSG_TF_CARD_PLUGIN, 0, 0);
			}
			else
			{
				SendNotifyMessage(mwd->MainWinHwnd, MSG_TF_CARD_PLUGOUT, 0, 0);
			}

			mwd->bTfcardExist = newTfcardStus;
		}

		// 自动关屏功能
		GetUserData(&usrData);
		if(usrData.screen_switch == 0)
		{
			mwd->auto_lcd_off_timer = 10;
		}
		else if(usrData.screen_switch == 1)
		{
			mwd->auto_lcd_off_timer = 20;
		}
		else if(usrData.screen_switch == 2)
		{
			mwd->auto_lcd_off_timer = 30;
		}
		else if(usrData.screen_switch == 3)
		{
			mwd->auto_lcd_off_timer = 0;
		}

		if(mwd->auto_lcd_off_timer > 0)
		{
			if(mwd->auto_lcd_off_cnt >= (mwd->auto_lcd_off_timer*10))
			{
				mwd->auto_lcd_off_cnt = (mwd->auto_lcd_off_timer*10);
			}
			else
			{
				mwd->auto_lcd_off_cnt = mwd->auto_lcd_off_cnt + 1;
			}
			if(mwd->auto_lcd_off_cnt == (mwd->auto_lcd_off_timer*10 - 1))
			{
				cvr_debug("auto lcd off\n");
				SendNotifyMessage(GetMainWinHwnd(), MSG_MAIN_LCD_OFF, 0, 0);
			}
		}
	}

	cvr_error("main thread end fail\n");
	return (void*)0;
}

s32 CreateMainThread(HWND hWnd, MainWinDataType *mwd)
{
	s32 ret = 0;

	ret = pthread_create(&mwd->threadID, NULL, MainThreadProc, (void *)mwd);
	if (ret == -1)
	{
		cvr_debug("create thread fail\n");
		return -1;
	}
	cvr_debug("mwd->threadID=%d\n", mwd->threadID);

	return 0;
}

s32 CloseMainThread(HWND hWnd, MainWinDataType *mwd)
{
	mwd->bThreadRun = CVR_FALSE;
	pthread_join(mwd->threadID, NULL);
	mwd->threadID = 0;
	return 0;
}

void GsensorCallBack(void)
{
	cvr_debug("gsensor call back active\n");
	SendNotifyMessage(GetRecordWinHwnd(), MSG_GSENSOR_HIT, 0, 0);
}

void InitSystemTime(HWND hWnd)
{
	struct tm u_time;
	memset(&u_time, 0, sizeof(struct tm));
	get_local_time(&u_time);

	if(u_time.tm_year < 2017)
	{
		cvr_debug("init the system time to defualt\n");
		u_time.tm_year = 2017;
		u_time.tm_mon = 1;
		u_time.tm_mday = 1;
		u_time.tm_hour = 0;
		u_time.tm_min = 0;
		u_time.tm_sec = 0;
		set_local_time(&u_time);
	}
}

void MainkeyProc(MainWinDataType *mwd, s32 keyCode, s32 isLongPress)
{
	HWND curWinHwnd;

	cvr_debug("key %x %d\n", keyCode, isLongPress);

	if(keyCode>CVR_KEY_MENU || keyCode<CVR_KEY_LEFT)
	{
		cvr_debug("invalid key %x\n", keyCode);
		return;
	}

	if(keyCode == CVR_KEY_POWER)
	{
		if(isLongPress == LONG_PRESS)
		{
			CloseTipLabel();
			ShowTipLabel(GetMainWinHwnd(), LABEL_SHUTDOWN_NOW, 1, 1000);
			SendMessage(GetMainWinHwnd(), MSG_CLOSE, 0, 0);		// power off
		}
		else
		{
			if(mwd->bLcdOn)
			{
				mwd->bLcdOn = CVR_FALSE;
				display_lcd_backlight_onoff(0);
			}
			else
			{
				mwd->bLcdOn = CVR_TRUE;
				display_lcd_backlight_onoff(1);
			}

			return;
		}
	}

	curWinHwnd = WindowID2Window(mwd, mwd->curWindowID);
	if(curWinHwnd != HWND_INVALID)
	{
		DispatchKeyEvent(mwd, curWinHwnd, keyCode, isLongPress);	// pitch message
	}

	return;
}

s32 MainWinMsgCreate(HWND hWnd, MainWinDataType *mwd)
{
	WindowIDType toType;

	if(power_is_charging() == POWER_PC_LINK)
	{
		toType = WINDOW_USB_ID;
	}
	else
	{
		toType = WINDOW_RECORD_ID;
	}

	SelfCtrlWinInit(hWnd, mwd);
	ChangeWindow(mwd, WINDOW_MAIN_ID, toType);

	return;
}

static int MainCallBack(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	MainWinDataType *mwd;
	mwd = (MainWinDataType*)GetWindowAdditionalData(hWnd);

	switch(message)
	{
		case MSG_LANG_CHANGED:
		{
			PLOGFONT logFont;
			logFont = getLogFont();
			if(logFont == NULL)
			{
				cvr_error("invalid log font\n");
				break;
			}
			SetWindowFont(hWnd, logFont);
			cvr_debug("Font type:%s, charset:%s, family:%s, size:%d\n", logFont->type, logFont->charset, logFont->family, logFont->size);
			break;
		}

		case MSG_USB_PLUGIN:
		{
			if(mwd->curWindowID == WINDOW_USB_ID)
			{
				cvr_warn("is usb window aready\n");
				break;
			}
			ChangeWindow(mwd, mwd->curWindowID, WINDOW_USB_ID);
			break;
		}

		case MSG_USB_PLUGOUT:
		{
			if(mwd->curWindowID != WINDOW_USB_ID)
			{
				cvr_warn("is not usb window aready\n");
				break;
			}
			SendNotifyMessage(mwd->UsbWinHwnd, MSG_USB_PLUGOUT, 0, 0);

			break;
		}

		case MSG_TF_CARD_PLUGIN:
		{

			cvr_debug("tf card plugin...\n");
			if(mwd->curWindowID == WINDOW_RECORD_ID)
			{
				SendNotifyMessage(mwd->RecordWinHwnd, MSG_TF_CARD_PLUGIN, 0, 0);
			}
			else if(mwd->curWindowID == WINDOW_MENU_ID)
			{
				SendNotifyMessage(mwd->MenuWinHwnd, MSG_TF_CARD_PLUGIN, 0, 0);
			}
			else
			{
				if(GetHbarWinHwnd() != HWND_INVALID)
				{
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 1, 0);
				}
			}
			break;
		}

		case MSG_TF_CARD_PLUGOUT:
		{
			cvr_debug("tf card plugout...\n");
			if(mwd->curWindowID == WINDOW_RECORD_ID)
			{
				SendNotifyMessage(mwd->RecordWinHwnd, MSG_TF_CARD_PLUGOUT, 0, 0);
			}
			else if(mwd->curWindowID == WINDOW_MENU_ID)
			{
				SendNotifyMessage(mwd->MenuWinHwnd, MSG_TF_CARD_PLUGOUT, 0, 0);
			}
			else
			{
				if(GetHbarWinHwnd() != HWND_INVALID)
				{
					SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_TFCARD, 0, 0);
				}
			}
			break;
		}

		case MSG_HBAR_UPDATAE_BATLEVEL:
		{
			SendNotifyMessage(GetHbarWinHwnd(), MSG_HBAR_UPDATAE_BATLEVEL, mwd->powerLevel, 0);
			break;
		}

		case MSG_MAIN_LCD_OFF:
		{
			if(mwd->bLcdOn)
			{
				mwd->bLcdOn = CVR_FALSE;
				display_lcd_backlight_onoff(0);
			}
			break;
		}

		case MSG_POWER_LOW:
		{
			if(mwd->curWindowID == WINDOW_RECORD_ID)
			{
				CloseTipLabel();
				ShowTipLabel(GetRecordWinHwnd(), LABEL_LOW_POWER_SHUTDOWN, 1, 10000);
			}
			else if(mwd->curWindowID == WINDOW_MENU_ID)
			{
				CloseTipLabel();
				ShowTipLabel(GetMenuWinHwnd(), LABEL_LOW_POWER_SHUTDOWN, 1, 10000);
			}

			SendMessage(GetMainWinHwnd(), MSG_CLOSE, 0, 0);

			break;
		}

		default:
		{
			break;
		}
	}

	return 0;
}

void show_close_logo(HWND hWnd, MainWinDataType *mwd)
{
	HDC hdc;
	BITMAP closeLogoBmp;

	hdc= GetDC(hWnd);
	LoadBitmapFromFile(HDC_SCREEN, &closeLogoBmp, "/usr/res/others/shutdown.png");
	FillBoxWithBitmap(hdc, 0, 0, 1280, 320, &closeLogoBmp);
	sleep(1);
	unloadBitMap(&closeLogoBmp);
	EndPaint(hWnd,hdc);
}

int MsgHook(void* context, HWND dst_wnd, int msg, WPARAM wparam, LPARAM lparam)
{
	int ret = HOOK_GOON;	// need other work
	MainWinDataType *mwd = NULL;

	mwd = (MainWinDataType*)context;

	mwd->auto_lcd_off_cnt = 0;

	if(wparam == CVR_KEY_POWER)
	{
		SendMessage(mwd->MainWinHwnd, msg, wparam, lparam);
		ret = !HOOK_GOON;	// need not other work
	}
	else
	{
		if(!mwd->bLcdOn)
		{
			ret = !HOOK_GOON;
		}
	}

	return ret;
}

static int MainWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	s32 ret;
	HDC hdc;
	MainWinDataType *mwd;
	mwd = (MainWinDataType*)GetWindowAdditionalData(hWnd);

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
			InitSystemTime(hWnd);
			ShowCursor(FALSE);
			MainWinMsgCreate(hWnd, mwd);
			CreateMainThread(hWnd, mwd);
			gsensor_init(GsensorCallBack);
			//sdcard_fs_flex();
		}
			break;

		case MSG_PAINT:
			break;

		case MSG_KEYDOWN:
		{
			if(mwd->isKeyUp)
			{
				mwd->downKey = wParam;
				SetTimer(hWnd, ID_TIMER_KEY, LONG_PRESS_TIME);
				mwd->isKeyUp = CVR_FALSE;
			}
			//keytone_play();
			//ret = play_wav_music(VOICE_TAKE_KEY);
			//if(ret != 0)
			//{
			//	cvr_warn("play %s fail\n", VOICE_POWER_ON);
			//}
		}
			break;

		case MSG_KEYUP:
		{
			mwd->isKeyUp = CVR_TRUE;
			if(wParam == mwd->downKey)
			{
				KillTimer(hWnd, ID_TIMER_KEY);
				cvr_debug("short press\n");
				MainkeyProc(mwd, wParam, SHORT_PRESS);
			}
		}
			break;

		case MSG_KEYLONGPRESS:
		{
			mwd->downKey = -1;
			cvr_debug("long press\n");
			MainkeyProc(mwd, wParam, LONG_PRESS);
		}
			break;

		case MSG_TIMER:
		{
			if(wParam == ID_TIMER_KEY)
			{
				mwd->isKeyUp = CVR_TRUE;
				SendMessage(hWnd, MSG_KEYLONGPRESS, mwd->downKey, 0);
				KillTimer(hWnd, ID_TIMER_KEY);
			}
		}
			break;

		case MSG_CLOSE:
		{
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SelfCtrlWinUninit(hWnd, mwd);
			show_close_logo(hWnd, mwd);
			gsensor_uninit(NULL);
			CloseMainThread(hWnd, mwd);
			ExitMainWindow(mwd);	// free ram
	        DestroyMainWindow (hWnd);		// destrony window
	        PostQuitMessage (hWnd);			// exit window
		}
			break;

		case MSG_DESTROY:
			break;

		default:
		{
			MainCallBack(hWnd, message, wParam, lParam);
		}
			break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

HWND createMainWindows(void)
{
	CvrRectType         rect;
	MAINWINCREATE CreateInfo;

	mwd = (MainWinDataType*)malloc(sizeof(MainWinDataType));
	if(NULL == mwd)
	{
		cvr_error("malloc main window data error\n");
		return NULL;
	}

	memset((void *)mwd, 0, sizeof(MainWinDataType));

	getResRect(ID_SCREEN,&rect);

	mwd->mainSize.x = rect.x;
	mwd->mainSize.y = rect.y;
	mwd->mainSize.w = rect.w;
	mwd->mainSize.h = rect.h;
	mwd->isKeyUp = CVR_TRUE;
	mwd->downKey = -1;
	mwd->curWindowID = WINDOW_MAIN_ID;
	mwd->oldWindowID = WINDOW_MAIN_ID;
	mwd->bLcdOn = CVR_TRUE;

	RegisterKeyMsgHook((void*)mwd, MsgHook);

	CreateInfo.dwStyle = WS_VISIBLE;
	//CreateInfo.dwExStyle = WS_EX_NONE | WS_EX_AUTOSECONDARYDC;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWinProc;

	CreateInfo.lx = mwd->mainSize.x;
	CreateInfo.ty = mwd->mainSize.y;
	CreateInfo.rx = mwd->mainSize.w;
	CreateInfo.by = mwd->mainSize.h;

	CreateInfo.iBkColor = COLOR_black;//RGBA2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00, 0x00);
	CreateInfo.dwAddData = (DWORD)mwd;
	CreateInfo.hHosting = HWND_DESKTOP;

	mainWinHwnd = CreateMainWindow(&CreateInfo);
	if (mainWinHwnd == HWND_INVALID) {
		cvr_error("create Mainwindow failed\n");
		return NULL;
	}

	setHwnd(WINDOW_MAIN_ID, mainWinHwnd);
	mwd->MainWinHwnd = mainWinHwnd;

	ShowWindow(mainWinHwnd, SW_SHOWNORMAL);

	return mainWinHwnd;
}

void MessageLoop(void)
{
	MSG Msg;

	while(GetMessage(&Msg, mainWinHwnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

s32 ExitMainWindow(MainWinDataType *mwd)
{
	if(NULL != mwd)
	{
		free((void*)mwd);
	}

	return 0;
}

int RegisterMLMControls(void)
{
	WNDCLASS WndClass;

	WndClass.spClassName = CTRL_CDRMenuList;
	WndClass.dwStyle     = WS_NONE;
	WndClass.dwExStyle   = WS_EX_NONE;
	WndClass.hCursor     = GetSystemCursor (0);
	WndClass.iBkColor    = COLOR_lightgray;
	WndClass.WinProc = MenuListCallback;

	if(RegisterWindowClass(&WndClass) == FALSE)
	{
		cvr_error("register MenuList failed\n");
		return -1;
	}

	return 0;
}

void UnRegisterMLMControls(void)
{
	UnregisterWindowClass(CTRL_CDRMenuList);
}

s32 RegisterCdrWindows(void)
{
	int count;

	const char *windowClass[] =
	{
		WINDOW_RECORD,
		WINDOW_MENU,
		WINDOW_USB,
		WINDOW_HBAR,
	};

	WNDCLASS WndClass =
	{
		NULL,
		0,
		WS_CHILD | WS_VISIBLE,
		WS_EX_USEPARENTFONT,
		GetSystemCursor(0),
		COLOR_lightgray,
		NULL,
		0
	};

	CvrWinProc windowProc[] =
	{
		RecordWinProc,
		MenuWinProc,
		UsbWinProc,
		HbarWinProc,
	};

	for(count=0; count<SelfCtrlWinCnt; count++)
	{
		WndClass.spClassName = windowClass[count];
		WndClass.WinProc = windowProc[count];
		WndClass.iBkColor = COLOR_black;//RGBA2Pixel(HDC_SCREEN, 0xFF, 0x00, 0x00, 0x00);

		if(RegisterWindowClass(&WndClass) == FALSE)
		{
			cvr_warn("register %s failed\n", windowClass[count]);
			return -1;
		}
	}

	return 0;
}

void UnRegisterCvrWindows(void)
{
	UnregisterWindowClass(WINDOW_HBAR);
	UnregisterWindowClass(WINDOW_USB);
	UnregisterWindowClass(WINDOW_MENU);
	UnregisterWindowClass(WINDOW_RECORD);
}

/*
int DispHdle = NULL;
void SetLcdBackLightPwm(void)
{
	int retval;
	unsigned long args[4]={0};

	DispHdle = open("/dev/disp", O_RDWR);
	if (DispHdle < 0)
	{
		cvr_debug("fail to open %s", "/dev/disp");
	}

	args[0] = 0;
	args[1] = 128;
	retval = ioctl(DispHdle, 0x102, args);
	if (retval < 0)
	{
		return -1;
	}
}
*/

void HardwareInit(void)
{
	s32 ret;
	volume_init();

	/*
	//volume_set_volume(24);
	//keytone_init(VOICE_TAKE_KEY);

	//ret = play_wav_music(VOICE_POWER_ON);
	//if(ret != 0)
	//{
	//	cvr_warn("play %s fail\n", VOICE_POWER_ON);
	//}
	*/

	display_lcd_set_brightness(100);
}

void SystemInit(void)
{
	UserDataType usrData;

	HardwareInit();
	ResourceInit();

	GetUserData(&usrData);
	// 设置音量大小
	if(usrData.voice_volume == 0)
	{
		volume_set_volume(30);		// 高
	}
	else if(usrData.voice_volume == 1)
	{
		volume_set_volume(24);		// 中
	}
	else if(usrData.voice_volume == 2)
	{
		volume_set_volume(6);		// 低
	}
}

void SystemUninit(void)
{
	ResourceUninit();
	//keytone_exit();
}

HWND GetMainWinHwnd(void)
{
	return mainWinHwnd;
}

MainWinDataType* GetMainWinData(void)
{
	if(mwd != NULL)
	{
		return mwd;
	}

	return NULL;
}

int MiniGUIMain (int argc, const char* argv[])
{
	HWND hwnd;

	cvr_debug("#########################################\n");
	cvr_debug("############# tinacvr start #############\n");
	cvr_debug("#########################################\n");

	#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "tinacvr" , 0 , 0);
	#endif

	SystemInit();	// system init

	// register menu ctrl
    if(RegisterMLMControls()<0) {
		cvr_error("register cvr menu controls failed\n");
		return -1;
	}
	// register self ctrl
	if(RegisterCdrWindows() < 0)
	{
		cvr_error("register cvr windows failed\n");
		return -1;
	}

	hwnd = createMainWindows();
	if(NULL == hwnd)	// create main window
	{
		cvr_error("create mainw indows failed\n");
		return -1;
	}
	initStage3();

	MessageLoop();			// message loop

	cvr_debug("exit main window\n");
	MainWindowThreadCleanup(hwnd);
	UnRegisterCvrWindows();	// unregister self ctrl
	// unregister menu ctrl
	UnRegisterMLMControls();
	SystemUninit();		// system uninit

	cvr_debug("tinacvr power off...\n");
	power_off();
	while(1)
	{
		cvr_debug("exit tinacvr...\n");
		sleep(2);
	}

	return 0;
}

/*
static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	  HDC hdc;
	  printf("message = %x, wParam = %x, lParam = %x \n", message, wParam, lParam);
    switch (message) {
        case MSG_CREATE:
            hdc = BeginPaint(hWnd);
            TextOut(hdc, 60, 60, " 你好，世界! ");
            EndPaint(hWnd, hdc);
            break;

        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "helloworld" , 0 , 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "标题";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 1280;
    CreateInfo.by = 320;
    CreateInfo.iBkColor = COLOR_black;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);
	ShowCursor(FALSE);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}
*/
