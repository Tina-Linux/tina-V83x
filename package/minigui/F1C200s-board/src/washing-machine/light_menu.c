#include "light_menu.h"
#include "button_view.h"
#include "washing_res_cn.h"
#include "resource.h"
#include "include.h"

int ID = 0;
HWND HOSTING;

static newGapX = 0;
static unsigned int length = 31;
static BOOL isSlidVolume = FALSE;
int msgclose = 1;

#define IP_CFG_SIZE 64
//char ip_adr[IP_CFG_SIZE] = {'A', 'W', '-', 'G', 'u', 'e', 's', 't'};
//char ip_psw[IP_CFG_SIZE] = {'A', 'w', '.', 'g', 'u', 'e', 's', 't', '6', '6', '8', '8'};
//static BOOL g_ime_opened = FALSE;

/*
void notify_ime_status_2(BOOL opened)
{
    g_ime_opened = opened;
}
*/

/*
int GetIpAdr(char *str)
{
	if(str == NULL)
	{
		return -1;
	}

	if (strlen(ip_adr) <= 0)
	{
		return -1;
	}

	memcpy(str, ip_adr, IP_CFG_SIZE);

	return strlen(ip_adr);
}

int GetIpPsw(char *str)
{
	if(str == NULL)
	{
		return -1;
	}

	if (strlen(ip_psw) <= 0)
	{
		return -1;
	}

	memcpy(str, ip_psw, IP_CFG_SIZE);

	return strlen(ip_psw);
}
*/

//BITMAP volume_light_bkg;
#if 0
void Register_res(void)
{
	LoadBitmap(HDC_SCREEN,&progress_p, "/usr/res/menu/progress_p.png");
	LoadBitmap(HDC_SCREEN,&volume_light_bkg, "/usr/res/menu/volume_light_bkg.png");
	LoadBitmap(HDC_SCREEN,&volume_bar_n, "/usr/res/menu/volume_bar_n.png");
	LoadBitmap(HDC_SCREEN,&main_bmp_bkgnd, "/usr/res/menu/background.png");
}
#endif

void Register_res(void) {
	LoadBitmap(HDC_SCREEN, &progress_p, "/usr/res/menu/progress_p.png");
	LoadBitmap(HDC_SCREEN, &volume_light_bkg,
			"/usr/res/menu/volume_light_bkg.png");
	LoadBitmap(HDC_SCREEN, &volume_bar_n, "/usr/res/menu/volume_bar_n.png");
	LoadBitmap(HDC_SCREEN, &main_bmp_bkgnd, "/usr/res/menu/background.png");
	LoadBitmap(HDC_SCREEN, &yuyue_bkg, "/usr/res/menu/yuyue_bkg.png");
	LoadBitmap(HDC_SCREEN, &tongsuo_bkg, "/usr/res/menu/tongsuo_bkg.png");
	LoadBitmap(HDC_SCREEN, &language_bkg, "/usr/res/menu/language_bkg.png");
	LoadBitmap(HDC_SCREEN, &sgi_bkg, "/usr/res/menu/sgj_bkg.png");
	LoadBitmap(HDC_SCREEN, &ztty_p, "/usr/res/menu/ztty.png");
}
void unregister_res(void) {
	UnloadBitmap(&progress_p);
	UnloadBitmap(&volume_light_bkg);
	UnloadBitmap(&volume_bar_n);
	UnloadBitmap(&main_bmp_bkgnd);
	UnloadBitmap(&yuyue_bkg);
	UnloadBitmap(&tongsuo_bkg);
	UnloadBitmap(&language_bkg);
	UnloadBitmap(&sgi_bkg);
	UnloadBitmap(&ztty_p);
}

/*static int my_usleep(useconds_t usec) {
 unsigned int now = times(NULL);
 unsigned int end = now + usec / (1000 * 10);
 while (((int) (end - now)) > 0) {
 usleep(((int) (end - now)) * 1000 * 10);
 now = times(NULL);
 }
 return 0;
 }

 static void slideMenu(HWND hWnd) {
 msgclose = 1;
 MSG msg;
 HDC sec_dc_active, sec_dc_hosting;

 sec_dc_active = GetSecondaryDC(hWnd);
 SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DONOTHING);
 ShowWindow(hWnd, SW_SHOWNORMAL);
 while (GetMessage(&msg, hWnd)) {
 if (msg.message == MSG_IDLE)
 break;
 DispatchMessage(&msg);
 }
 HWND hosting = GetHosting(hWnd);
 sec_dc_hosting = GetSecondaryDC(hosting);
 SetSecondaryDC(hosting, sec_dc_hosting, ON_UPDSECDC_DONOTHING);
 useconds_t time = 25000;
 BitBlt(sec_dc_active, BORDER, LY, 50, BY, HDC_SCREEN, RX - 50, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 100, BY, HDC_SCREEN, RX - 100, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 150, BY, HDC_SCREEN, RX - 150, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 200, BY, HDC_SCREEN, RX - 200, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 250, BY, HDC_SCREEN, RX - 250, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 300, BY, HDC_SCREEN, RX - 300, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 350, BY, HDC_SCREEN, RX - 350, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 400, SY, 0);
 my_usleep(time);

 BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 450, SY, 0);
 BitBlt(sec_dc_hosting, 384 - 50, SY, 50, BY, HDC_SCREEN, RX - 50, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 500, SY, 0);
 BitBlt(sec_dc_hosting, 384 - 100, SY, 50, BY, HDC_SCREEN, RX - 100, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 550, SY, 0);
 BitBlt(sec_dc_hosting, 384 - 150, SY, 50, BY, HDC_SCREEN, RX - 150, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 600, SY, 0);
 BitBlt(sec_dc_hosting, 384 - 200, SY, 50, BY, HDC_SCREEN, RX - 200, SY, 0);
 SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
 SetSecondaryDC(hosting, sec_dc_hosting, ON_UPDSECDC_DEFAULT);

 msgclose = 0;
 }

 static void closeMenu(HWND HOSTING, HWND hWnd) {
 int width = 480;
 MSG msg;
 HDC sec_dc_active, sec_dc_hosting, sec_dc_hosting_father;

 sec_dc_active = GetSecondaryDC(hWnd);
 SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DONOTHING);
 ShowWindow(hWnd, SW_SHOWNORMAL);
 while (GetMessage(&msg, hWnd)) {
 if (msg.message == MSG_IDLE)
 break;
 DispatchMessage(&msg);
 }
 HWND hosting = GetHosting(hWnd);
 sec_dc_hosting = GetSecondaryDC(hosting);
 HWND hosting_father = GetHosting(HOSTING);
 sec_dc_hosting_father = GetSecondaryDC(hosting_father);
 //      SetSecondaryDC(hosting, sec_dc_hosting, ON_UPDSECDC_DONOTHING);
 useconds_t time = 25000;
 BitBlt(sec_dc_hosting_father, 200, 100, 100, BY, HDC_SCREEN, 200, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 300, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_hosting_father, 300, 100, 100, BY, HDC_SCREEN, 300, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 400, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_hosting_father, 400, 100, 16, BY, HDC_SCREEN, 400, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 416, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_hosting, 0, 100, 100, BY, HDC_SCREEN, 416, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 516, SY, 0);
 my_usleep(time);
 BitBlt(sec_dc_hosting, 100, 100, 100, BY, HDC_SCREEN, 516, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 616, SY, 0);
 my_usleep(time);

 BitBlt(sec_dc_hosting, 200, 100, 100, BY, HDC_SCREEN, 616, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 716, SY, 0);
 my_usleep(time);

 BitBlt(sec_dc_hosting, 300, 100, 100, BY, HDC_SCREEN, 716, SY, 0);
 BitBlt(sec_dc_active, BORDER, 0, 400, BY, HDC_SCREEN, 800, SY, 0);
 my_usleep(time);
 SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
 SetSecondaryDC(hosting, sec_dc_hosting, ON_UPDSECDC_DEFAULT);
 SetSecondaryDC(hosting_father, sec_dc_hosting_father, ON_UPDSECDC_DEFAULT);
 printf("close close\n");
 ReleaseCapture();
 msgclose = 1;
 }*/

static int MainWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {
	int oldX = 0;
	int oldY = 0;
	int lengthX = 0;
	int lengthY = 0;
	HWND hnumberWnd;
	HWND hEdit0, hEdit1;
	HDC hdc;
	static PLOGFONT Font_20, Font_45, Font_30;
	int id = 0;
	int len = 50;
	int upX;
	RECT rcTemp;

	HWND wifi_adr;
	HWND wifi_psw;
	HWND soft_key;
	char str[IP_CFG_SIZE];

	app_debug("param: (0x%x, 0x%x, %d, %d)\n", hWnd, message, wParam, lParam);

	switch (message) {
	case MSG_CREATE:
		Font_20 = getLogFont(ID_FONT_SIMSUN_20);
		Font_30 = getLogFont(ID_FONT_SIMSUN_30);
		Font_45 = getLogFont(ID_FONT_SIMSUN_45);
		switch (ID)
		{
			case 2:
				wifi_adr = CreateWindowEx(CTRL_SLEDIT, "",
				WS_VISIBLE | WS_TABSTOP | WS_BORDER,
				WS_EX_TRANSPARENT, 1, 120, 88, 200, 25, hWnd, NULL);
				SetFocusChild(wifi_adr);

				wifi_psw = CreateWindowEx(CTRL_SLEDIT, "",
				WS_VISIBLE | WS_TABSTOP | WS_BORDER,
				WS_EX_TRANSPARENT, 2, 120, 148, 200, 25, hWnd, NULL);

				//SetWindowText (GetDlgItem(hWnd, 1), ip_adr);
				//SetWindowText (GetDlgItem(hWnd, 2), ip_psw);
				/*
				soft_key = mgiCreateSoftKeypad(notify_ime_status_2);
				SetIMEStatus(IME_STATUS_ENCODING, IME_ENCODING_LOCAL);
				SetIMEStatus(IME_STATUS_ENABLED, TRUE);
				*/
				break;

			case 3:
			case 4:
				break;
		}
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		DestroyAllControls(hWnd);
		PostMessage(HOSTING, MSG_LIGHT_MENU_CLOSE, 0, 0);
		return 0;
	case MSG_PAINT:
		hdc = BeginPaint(hWnd);
		int slideX = LOWORD(lParam);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 45, 83, 126));
		switch (ID) {
		case 0:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 30, 20, TONGSUO_N);
			SelectFont(hdc, Font_30);
			TextOut(hdc, 50, 240, QUXIAO_N);
			SelectFont(hdc, Font_30);
			TextOut(hdc, 240, 240, KAIQI_N);
			break;

		case 2:
			SelectFont(hdc, Font_30);
			TextOut(hdc, 200, 20, WIFI);
			SelectFont(hdc, Font_30);
			TextOut(hdc, 100, 240, QUXIAO_N);
			SelectFont(hdc, Font_30);
			TextOut(hdc, 300, 240, WORLD_RIGHT_BOTTOM);

			SelectFont(hdc, Font_30);
			TextOut(hdc, 80, 80, "1:");
			SelectFont(hdc, Font_30);
			TextOut(hdc, 80, 140, "2:");
			break;

		case 3:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 200, 20, YINLIAN);
			FillBoxWithBitmap(hdc, 10, 150, 360, 20, &volume_bar_n);
			FillBoxWithBitmap(hdc, 30, 150, length - 30, 20, &progress_p);
			break;
		case 4:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 200, 20, LIANDU);
			FillBoxWithBitmap(hdc, 10, 150, 360, 20, &volume_bar_n);
			FillBoxWithBitmap(hdc, 30, 150, length - 30, 20, &progress_p);
			printf("length %d\n", (int)length);
			break;
		case 5:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 150, 20, YUYAN);
			SelectFont(hdc, Font_30);
			TextOut(hdc, 70, 150, ZHONGWEN);
			TextOut(hdc, 240, 150, YINGWEN);

			break;
		case 6:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 150, 20, SHUIGUANJIA);
			break;
		case 7:
			break;
		case 8:
			SelectFont(hdc, Font_45);
			TextOut(hdc, 100, 120, SHIPIN_N);
			break;
		case 9:

			break;
		}
		EndPaint(hWnd, hdc);
		return 0;
	case MSG_ERASEBKGND:
		hdc = (HDC) wParam;
		switch (ID) {
		case 0:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &tongsuo_bkg);
			break;

		case 2:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &tongsuo_bkg);
			break;

		case 4:
		case 3:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &volume_light_bkg);
			break;
		case 5:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &language_bkg);
			break;
		case 6:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &sgi_bkg);
			break;
		case 7:
			break;
		case 8:
			FillBoxWithBitmap(hdc, 0, 0, 400, 280, &sgi_bkg);
			break;
		}
		return 0;
	case MSG_LBUTTONDOWN:
#if 1
		switch (ID) {
		case 1:
			break;
		case 7:
			break;
		case 4:
		case 3:
			//SetCapture(hWnd);
			isSlidVolume = TRUE;
			lengthX = LOWORD(lParam);
			lengthY = HIWORD(lParam);
			lengthX = lengthX - 200;
			printf("lengthX = %d\n", lengthX);
			printf("lengthY = %d\n", lengthY);
			if (lengthX < 345 && lengthX > 30) {
				printf("join\n");
				length = lengthX;
				InvalidateRect(hWnd, NULL, TRUE);
			} else {
				return -1;
			}
			break;
		}
		break;
	case 8:
	case 6:
		break;
	case 5:
		break;
#endif
	case MSG_LBUTTONUP:
		oldX = LOWORD(lParam);
		oldY = HIWORD(lParam);
		switch (ID) {
		case 0:
			if (oldX > 200 && oldY > 320 && oldX < 600 && oldY < 380) {
				if (msgclose == 1) {
					break;
				} else if (msgclose == 0) {
					//closeMenu(HOSTING, hWnd);
					ReleaseCapture();
					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				}

			}
			break;

		case 2:
			if (oldX > 0 && oldY > 220 && oldX < 200 && oldY < 280) {
				if (msgclose == 1) {
					break;
				} else if (msgclose == 0) {
					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}

			}
			else if (oldX > 200 && oldY > 220 && oldX < 400 && oldY < 280)
			{
				if (msgclose == 1)
				{
					break;
				}
				else if (msgclose == 0)
				{
					/*
					GetWindowText(GetDlgItem(hWnd, 1), str, IP_CFG_SIZE);
					memcpy(ip_adr, str, IP_CFG_SIZE);
					printf("len=%d, ip_adr=%s\n", strlen(ip_adr), ip_adr);

					GetWindowText(GetDlgItem(hWnd, 2), str, IP_CFG_SIZE);
					memcpy(ip_psw, str, IP_CFG_SIZE);
					printf("len=%d, ip_psw=%s\n", strlen(ip_psw), ip_psw);
					*/

					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
				}
			}
			break;

		case 4:
		case 3:
			if (GetCapture() != hWnd)
				break;
			upX = LOWORD(lParam);
			isSlidVolume = FALSE;

			if (LOWORD(lParam) < 200 || HIWORD(lParam) > 380) {
				if (msgclose == 1) {
					break;
				} else if (msgclose == 0) {
					//closeMenu(HOSTING, hWnd);
					ReleaseCapture();
					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				}

			}

			break;
		case 5:
			if (oldX > 200 && oldY > 230 && oldX < 600 && oldY < 300) {
				if (msgclose == 1) {
					break;
				} else if (msgclose == 0) {
					//closeMenu(HOSTING, hWnd);
					ReleaseCapture();
					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				}
			}
			break;
		case 1:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			oldX = LOWORD(lParam);
			oldY = HIWORD(lParam);
			if (oldX > 200 && oldY > 100 && oldX < 600 && oldY < 380) {
				if (msgclose == 1) {
					break;
				} else if (msgclose == 0) {
					//closeMenu(HOSTING, hWnd);
					ReleaseCapture();
					msgclose = 1;
					PostMessage(hWnd, MSG_CLOSE, 0, 0);

				}
			}
			break;
		}
		break;

	case MSG_MOUSEMOVE: {
		unsigned int gap = 0;
		unsigned int oldGapX = 0;
		SetCapture(hWnd);
		oldX = LOWORD(lParam);
		oldY = HIWORD(lParam);

		switch (ID) {
		case 4:
		case 3:
			if (isSlidVolume) {
				unsigned int newX = LOWORD(lParam) - 200;
				gap = newX - lengthX;
				newGapX = 0 + gap;
				length = gap + oldX;
				if (length < 345 && length > 30) {
					InvalidateRect(hWnd, NULL, TRUE);
				} else {
					return 0;
				}
			}
			break;
		}
	}
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND hMainWnd = 0;

HWND GetlightMenuHwnd() {
	printf("%d\n", hMainWnd);
	return hMainWnd;

}

int light_menu(HWND hParent, int id) {
	if (hMainWnd != 0) {
		printf("!=0\n");
		return -1;
	}
	HOSTING = hParent;
	MSG Msg;
	ID = id;
	MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
	JoinLayer(NAME_DEF_LAYER , MACHINE_TITLE , 0 , 0);
#endif
	Register_res();
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS | WS_EX_TOPMOST
			| WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = 0; /*GetSystemCursor(0);*/
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWinProc;

	if(ID != 2)
	{
		CreateInfo.lx = 200;
		CreateInfo.ty = 100;
		CreateInfo.rx = 600;
		CreateInfo.by = 380;
	}
	else
	{
		CreateInfo.lx = 200;
		CreateInfo.ty = 70;
		CreateInfo.rx = 600;
		CreateInfo.by = 350;
	}

	CreateInfo.iBkColor = PIXEL_lightwhite;
	CreateInfo.hHosting = hParent;
	hMainWnd = CreateMainWindow(&CreateInfo);
	printf("%d\n", hMainWnd);
	if (hMainWnd == HWND_INVALID)
		return -1;

	if(ID != 2)
	{
		SetCapture(hMainWnd);
	}

	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	msgclose = 0;
	//slideMenu(hMainWnd);

	while (GetMessage(&Msg, hMainWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	unregister_res();
	hMainWnd = 0;
	return 0;
}
