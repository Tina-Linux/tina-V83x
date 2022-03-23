#include "washing_res_cn.h"
#include "reservation.h"
#include "resource.h"
#include "features_menu.h"

int closen = 1;
HWND HOSTING;
SlideNumberView *slideNumView = NULL;
static int my_usleep(useconds_t usec) {
	unsigned int now = times(NULL);
	unsigned int end = now + usec / (1000 * 10);
	while (((int) (end - now)) > 0) {
		usleep(((int) (end - now)) * 1000 * 10);
		now = times(NULL);
	}
	return 0;
}
static void slideMenu(HWND hWnd) {
	closen = 1;
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
//      SetSecondaryDC(hosting, sec_dc_hosting, ON_UPDSECDC_DONOTHING);
	useconds_t time = 900;
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
	BitBlt(sec_dc_hosting, WIDTH - 50, SY, 50, BY, HDC_SCREEN, RX - 50, SY, 0);
	my_usleep(time);
	BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 500, SY, 0);
	BitBlt(sec_dc_hosting, WIDTH - 100, SY, 50, BY, HDC_SCREEN, RX - 100, SY, 0);
	my_usleep(time);
	BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 550, SY, 0);
	BitBlt(sec_dc_hosting, WIDTH - 150, SY, 50, BY, HDC_SCREEN, RX - 150, SY, 0);
	my_usleep(time);
	BitBlt(sec_dc_active, BORDER, LY, 400, BY, HDC_SCREEN, RX - 600, SY, 0);
	BitBlt(sec_dc_hosting, WIDTH - 200, SY, 50, BY, HDC_SCREEN, RX - 200, SY, 0);
	SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
	SetCapture(hWnd);
	closen = 0;
}
static void closeMenu(HWND HOSTING, HWND hWnd) {
	int width = 480;
	MSG msg;
	HDC sec_dc_active, sec_dc_hosting, sec_dc_hosting_father;

	sec_dc_active = GetSecondaryDC(hWnd);
	SetSecondaryDC(hWnd, sec_dc_active, ON_UPDSECDC_DONOTHING);
	HWND hosting = GetHosting(hWnd);
	sec_dc_hosting = GetSecondaryDC(hosting);
	HWND hosting_father = GetHosting(HOSTING);
	sec_dc_hosting_father = GetSecondaryDC(hosting_father);
	SetSecondaryDC(hosting_father, sec_dc_hosting_father,
	ON_UPDSECDC_DONOTHING);
	useconds_t time = 900;
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
	ReleaseCapture();
	closen = 1;
}

static int MainWinProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	static BOOL isSlidNumber = FALSE;
	static int oldY = 0;
	static int oldGap = 0;
	static int newX = 0;
	static int newY = 0;
	HDC hdc;
	static RECT rect, parentrect;
	static PLOGFONT Font_20, Font_45;
	Font_20 = getLogFont(ID_FONT_SIMSUN_20);
	Font_45 = getLogFont(ID_FONT_SIMSUN_45);

	switch (message) {
	case MSG_PAINT: {
		hdc = BeginPaint(hwnd);
		SIZE size;
		char str[4];
		GetClientRect(hwnd, &rect);
		SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 179, 9));
		SetBkMode(hdc, BM_TRANSPARENT);
		SelectFont(hdc, slideNumView->logfont_ttf_times_p);
		if (oldGap < -50) {
			slideNumView->selectNumber++;
			oldGap = 0;
			SendNotifyMessage(GetParent(hwnd), MSG_SLIDE_NUMBER_CHANGE, hwnd,
					slideNumView->selectNumber);
		} else if (oldGap > 30) {
			slideNumView->selectNumber--;
			oldGap = 0;
			SendNotifyMessage(GetParent(hwnd), MSG_SLIDE_NUMBER_CHANGE, hwnd,
					slideNumView->selectNumber);
		}
		sprintf(str, "%d", slideNumView->selectNumber);
		GetTextExtent(hdc, str, -1, &size);
		TextOut(hdc, rect.right / 2 - size.cx / 2,
				rect.bottom / 2 - size.cy / 2 + oldGap, str);

		SetTextColor(hdc, COLOR_black);
		SelectFont(hdc, slideNumView->logfont_ttf_times_n);
		SelectFont(hdc, Font_45);
		TextOut(hdc, 125, 20, XIAOSHIYUYUE);
		SelectFont(hdc, Font_20);
		TextOut(hdc, 50, 150, WORLD_LEFT);

		SelectFont(hdc, Font_20);
		TextOut(hdc, 230, 150, WORLD_RIGHT);

		SelectFont(hdc, Font_20);
		TextOut(hdc, 80, 250, WORLD_LEFT_BOTTOM);

		SelectFont(hdc, Font_20);
		TextOut(hdc, 300, 250, WORLD_RIGHT_BOTTOM);
		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_LBUTTONDOWN: {
		newX = LOWORD(lParam);
		newY = HIWORD(lParam);
		printf("newX =%d\n", newX);
		printf("newY =%d\n", newY);

		if (newX > 200 || newY > 100 || newX < 600 || newY < 320) {
			isSlidNumber = TRUE;
		}
		oldY = newY;
		oldGap = 0;
		break;
	}
	case MSG_LBUTTONUP:
		newX = LOWORD(lParam);
		newY = HIWORD(lParam);
		isSlidNumber = FALSE;
		oldGap = 0;
		InvalidateRect(hwnd, NULL, TRUE);
		if (newX > 200 && newY > 320 && newX < 600 && newY < 380) {
			if (closen == 1) {
				break;
			} else if (closen == 0) {
				closeMenu(HOSTING, hwnd);
				PostMessage(hwnd, MSG_CLOSE, 0, 0);
			}
		}
		break;
	case MSG_MOUSEMOVE: {
		newX = LOWORD(lParam);
		newY = HIWORD(lParam);
		if (newX > 200 && newY > 100 && newX < 600 && newY < 320) {
			if (isSlidNumber) {
				ScreenToClient(hwnd, &newX, &newY);
				int gap = newY - oldY;
				int newGap = oldGap + gap;
				printf("newgap = %d\n", newGap);
				if ((slideNumView->selectNumber == slideNumView->minimal
						&& gap > 0 && newGap > 50)
						|| (slideNumView->selectNumber == slideNumView->maximum
								&& gap < 0 && newGap < -50))
					break;

				InvalidateRect(hwnd, NULL, TRUE);
				oldY = newY;
				oldGap = newGap;

			}
		}
		break;
	}
	case MSG_ERASEBKGND:
		hdc = (HDC) wParam;
		FillBoxWithBitmap(hdc, 0, 0, 400, 280, &yuyue_bkg);
		return 0;

	case MSG_CLOSE:
		if (slideNumView)
			free(slideNumView);
		DestroyMainWindow(hwnd);
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND hreserveWnd = 0;
int yuyue_menu(HWND hParent) {
	if (hreserveWnd != 0) {
		return -1;
	}
	HOSTING = hParent;
	MSG Msg;
	register_reservation_pic();
	MAINWINCREATE CreateInfo;
	slideNumView = (SlideNumberView*) malloc(sizeof(SlideNumberView));
#ifdef _MGRM_PROCESSES
	JoinLayer(NAME_DEF_LAYER , MACHINE_TITLE , 0 , 0);
#endif
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS | WS_EX_TOPMOST
			| WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = 0; /*GetSystemCursor(0);*/
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MainWinProc;
	CreateInfo.lx = 200;
	CreateInfo.ty = 100;
	CreateInfo.rx = 600;
	CreateInfo.by = 380;

	CreateInfo.iBkColor = PIXEL_lightwhite;
	CreateInfo.hHosting = hParent;
	hreserveWnd = CreateMainWindow(&CreateInfo);
	/*LoadBitmap(HDC_SCREEN, &slideNumView->bmp_bg_slide_number,
	 "res/bottom_fun.png");*/
	slideNumView->logfont_ttf_times_p = getLogFont(ID_FONT_SIMSUN_40);
	slideNumView->logfont_ttf_times_n = getLogFont(ID_FONT_SIMSUN_30);

	slideNumView->minimal = 0;
	slideNumView->maximum = 23;
	slideNumView->selectNumber = 2;
	slideNumView->isFullyOpen = FALSE;

	if (hreserveWnd == HWND_INVALID)
		return -1;
	slideMenu(hreserveWnd);
	//ShowWindow(hreserveWnd, SW_SHOWNORMAL);
	while (GetMessage(&Msg, hreserveWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	ReleaseCapture();
	MainWindowThreadCleanup(hreserveWnd);
	unregister_reservation_pic();
	hreserveWnd = 0;
	return 0;
}
