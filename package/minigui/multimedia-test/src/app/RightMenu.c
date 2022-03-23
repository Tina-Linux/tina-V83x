#include "RightMenu.h"
#include "button_view.h"
#include "player_int.h"
#include "HeadWindow.h"
#include "BottomMenu.h"
#include "multimedia_test.h"
extern WinDataType *g_win_data;
/*Indicates whether the body of the structure is released 0 indicates normal
return to the main interface release -1 indicates forced exit from
the release of space 1 means that no space is released
int boolfreespace = 0;*/
extern HbarWinDataType *headbar;
extern BottomMenuDataType *bottom;
static char find_file_name[64];
SetupDataType *setupData;
RightMenuDataType *RightMenuWnd = NULL;
int next_number = 0;
int MAX_FOR = -1;
extern int file_index;
static int prevbutton_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	int i = 0;
	if (nc == BN_CLICKED && RightMenuWnd->isButtonPre == TRUE) {
		KillTimer(RightMenuWnd->RightMenuHwnd, _ID_TIMER_RIGHTHIDE);
		if (bottom->media_number < FILE_MAX) {
			return 0;
		} else {
			int i = 0;
			for (i = 0; i < FILE_MAX; i++)
			/*	memset(RightMenuWnd->filename_list[i], 0, 64);*/
			SendMessage(RightMenuWnd->ListBox0, LB_RESETCONTENT, 0, 0);
			loadPrevFileList();
			for (i = 0; i < FILE_MAX; ++i) {
				RightMenuWnd->lbii.string = RightMenuWnd->filename_list[i];
				SendMessage(RightMenuWnd->ListBox0, LB_ADDSTRING, i,
						(LPARAM) &RightMenuWnd->lbii);
			}
		}
		RightMenuWnd->isButtonNext = TRUE;
	}
	return 0;
}

static int nextbutton_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	int i = 0;
	if (nc == BN_CLICKED && RightMenuWnd->isButtonNext == TRUE) {
		KillTimer(RightMenuWnd->RightMenuHwnd, _ID_TIMER_RIGHTHIDE);
		if (bottom->media_number < FILE_MAX) {
			return 0;
		} else {
			int i = 0;
			for (i = 0; i < FILE_MAX; i++)
			/*memset(RightMenuWnd->filename_list[i], 0, 64);*/
			SendMessage(RightMenuWnd->ListBox0, LB_RESETCONTENT, 0, 0);
			loadNextFileList();
			for (i = 0; i < MAX_FOR; ++i) {
				if (RightMenuWnd->filename_list[i] == NULL) {
					return 0;
				}
				RightMenuWnd->lbii.string = RightMenuWnd->filename_list[i];
				SendMessage(RightMenuWnd->ListBox0, LB_ADDSTRING, i,
						(LPARAM) &RightMenuWnd->lbii);
			}
		}
		RightMenuWnd->isButtonPre = TRUE;
	}

	return 0;
}

static void get_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	int index = 0;
	if (nc == LBN_CLICKED || nc == LBN_ENTER) {
		index = SendMessage(hwnd, LB_GETCURSEL, 0, 0L);
		SendMessage(hwnd, LB_GETTEXT, index, (LPARAM) find_file_name);
	}
}

void loadNextFileList() {
	int i = 0;
	if (RightMenuWnd->paginal_number <= next_number - 1) {
		for(i = 0; i < FILE_MAX; ++i){
			media_get_next_name_path(RightMenuWnd->filename_list[i], RightMenuWnd->filepath_list[i]);
		}
		RightMenuWnd->paginal_number++;
		MAX_FOR = FILE_MAX;
	} else {
		for(i = 0; i < RightMenuWnd->paginal_residual_number; ++i){
			media_get_next_name_path(RightMenuWnd->filename_list[i], RightMenuWnd->filepath_list[i]);
		}
		RightMenuWnd->paginal_number++;
		MAX_FOR = RightMenuWnd->paginal_residual_number;
		RightMenuWnd->isButtonNext = FALSE;
	}
}

void loadPrevFileList() {
	int i = 0;
	int t = 0;
	t = FILE_MAX * RightMenuWnd->paginal_number;
	if (RightMenuWnd->paginal_number > 1) {
		if (t > bottom->media_number) {
			skip_to_index(FILE_MAX + RightMenuWnd->paginal_residual_number, -1);
			for(i = 0; i < FILE_MAX; ++i){
				media_get_next_name_path(RightMenuWnd->filename_list[i], RightMenuWnd->filepath_list[i]);
			}
			RightMenuWnd->paginal_number--;
		} else {
			skip_to_index(FILE_MAX * 2, -1);
			for(i = 0; i < FILE_MAX; ++i){
				media_get_next_name_path(RightMenuWnd->filename_list[i], RightMenuWnd->filepath_list[i]);
			}
			RightMenuWnd->paginal_number--;
		}
	}else {
		RightMenuWnd->isButtonPre = FALSE;
	}
}

int createlist(int filetype) {
	s32 ret;
	int i = 0;
	/* Add judgment sd card is mounted successfully */
	if (sdcard_is_mount_correct() || udisk_is_mount_correct() == 1) {
		printf("sd card mount ok\n");
		if (filetype == ID_PLAYER) {
			printf("sd card mount ok\n");
			ret = create_media_list_player();
			if (ret != 0) {
				sm_error("create media list fail\n");
			}
		} else if (filetype == ID_MUSIC) {
			ret = create_media_list_music();
			if (ret != 0) {
				sm_error("create media list fail\n");
			}
		} else if (filetype == ID_PICTURE) {
			ret = create_media_list_picture();
			if (ret != 0) {
				sm_error("create media list fail\n");
			}
		}
		headbar->isCreateMediaList = TRUE;
	} else {
		printf("sd card mount error \n");
	}
	bottom->media_number = media_get_total_num();
	next_number = bottom->media_number / FILE_MAX;
	RightMenuWnd->paginal_residual_number = bottom->media_number % FILE_MAX;
	if (bottom->media_number <= FILE_MAX) {
		RightMenuWnd->request_number = bottom->media_number;
	} else {
		RightMenuWnd->request_number = FILE_MAX;
	}
	for (i = 0; i < RightMenuWnd->request_number; ++i) {
		RightMenuWnd->filename_list[i] = (char *) malloc(64);
		if (RightMenuWnd->filename_list[i] == NULL) {
			printf("malloc error = i =%d\n", i);
			return -1;
		}
		RightMenuWnd->filepath_list[i] = (char *) malloc(64);
		if (RightMenuWnd->filepath_list[i] == NULL) {
			printf("malloc error i = %d\n", i);
		}
	}

	reorder_media_list();
	de_show();
	skip_to_head();
	for(i = 0; i < RightMenuWnd->request_number; ++i){
		media_get_next_name_path(RightMenuWnd->filename_list[i], RightMenuWnd->filepath_list[i]);
	}
	for (i = 0; i < RightMenuWnd->request_number; ++i) {
		printf("filename_number[%d] = %s\n", i, RightMenuWnd->filename_list[i]);
		printf("filepath number[%d] = %s\n", i, RightMenuWnd->filepath_list[i]);
	}

	return 0;
}

void CreateListFileName(HWND hwnd, int filetype) {
	int i = 0;
	s32 ret;
	CvrRectType rect;
	getResRect(ID_LIST_FILE_NAME, &rect);
	createlist(filetype);
	HICON hIcon1 = 0L;
	RightMenuWnd->lbii.hIcon = hIcon1;
	RightMenuWnd->lbii.cmFlag = CMFLAG_CHECKED;
	RightMenuWnd->ListBox0 = CreateWindowEx(CTRL_LISTBOX, " ",
	WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | LBS_AUTOCHECKBOX, 0,
			ID_LIST_FILE_NAME, rect.x, rect.y, rect.w, rect.h, hwnd, 0);
	if (g_rcScr.bottom <= 480) {
		SetWindowFont(RightMenuWnd->ListBox0, RightMenuWnd->logFont15);
	} else {
		SetWindowFont(RightMenuWnd->ListBox0, RightMenuWnd->logFont25);
	}
	for (i = 0; i < RightMenuWnd->request_number; ++i) {
		RightMenuWnd->lbii.string = RightMenuWnd->filename_list[i];
		printf("lbii.string[%d] = %s\n", i, RightMenuWnd->filename_list[i]);
		SendMessage(RightMenuWnd->ListBox0, LB_ADDSTRING, i,
				(LPARAM) &RightMenuWnd->lbii);
	}

	SetNotificationCallback(RightMenuWnd->ListBox0, get_notif_proc);
	getResRect(ID_PREVBUTTON, &RightMenuWnd->prebuttonSize);
	getResBmp(ID_PREVBUTTON, BMPTYPE_BASE, &RightMenuWnd->prebuttonBmp);

	RightMenuWnd->prevbutton = CreateWindowEx(CTRL_BUTTON, "PREV",
	WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_BITMAP,
	WS_EX_TRANSPARENT, ID_PREVBUTTON, RightMenuWnd->prebuttonSize.x,
			RightMenuWnd->prebuttonSize.y, RightMenuWnd->prebuttonSize.w,
			RightMenuWnd->prebuttonSize.h, hwnd,
			(DWORD) &RightMenuWnd->prebuttonBmp);
	/*SetWindowFont(RightMenuWnd->prevbutton, RightMenuWnd->logFont25);*/
	SetNotificationCallback(RightMenuWnd->prevbutton, prevbutton_notif_proc);

	getResRect(ID_NEXTBUTTON, &RightMenuWnd->nextbuttonSize);
	getResBmp(ID_NEXTBUTTON, BMPTYPE_BASE, &RightMenuWnd->nextbuttonBmp);
	RightMenuWnd->nextbutton = CreateWindowEx(CTRL_BUTTON, "NEXT",
	WS_CHILD | WS_VISIBLE | BS_NOTIFY | BS_BITMAP,
	WS_EX_TRANSPARENT, ID_NEXTBUTTON, RightMenuWnd->nextbuttonSize.x,
			RightMenuWnd->nextbuttonSize.y, RightMenuWnd->nextbuttonSize.w,
			RightMenuWnd->nextbuttonSize.h, hwnd,
			(DWORD) &RightMenuWnd->nextbuttonBmp);
	/*SetWindowFont(RightMenuWnd->nextbutton, RightMenuWnd->logFont25);*/
	SetNotificationCallback(RightMenuWnd->nextbutton, nextbutton_notif_proc);
}

void CreateSetup(HWND hwnd) {
	CvrRectType rect;
	getResRect(ID_SETUPATTR, &rect);
	int i = 0;
	int height = (rect.h - rect.y) / 15;
	if (rect.h < 480 && rect.h > 320) {
		height = (rect.h - rect.y) / 15;
	} else if (rect.h < 320) {
		height = (rect.h - rect.y) / 12;
	}
	RightMenuWnd->Btn[0] = CreateWindowEx(CTRL_STATIC, FENBIANLV,
	WS_CHILD | WS_VISIBLE | SS_GROUPBOX, 0, ID_SETUPSTATIC0, rect.x, rect.y,
			rect.w, rect.h, hwnd, 0);

	RightMenuWnd->Btn[1] = CreateWindowEx(CTRL_BUTTON, SCREEN_1280X800,
	BS_AUTORADIOBUTTON | WS_VISIBLE | WS_TABSTOP | WS_GROUP, 0, ID_SETUPBTN0,
			rect.x, rect.y + height, SELECT_BTN_W, SELECT_BTN_H, hwnd, 0);

	RightMenuWnd->Btn[2] = CreateWindowEx(CTRL_BUTTON, SCREEN_800X480,
	BS_AUTORADIOBUTTON | WS_VISIBLE, 0, ID_SETUPBTN1, rect.x,
			rect.y + 2 * height, SELECT_BTN_W, SELECT_BTN_H, hwnd, 0);

	RightMenuWnd->Btn[3] = CreateWindowEx(CTRL_BUTTON, SCREEN_1280X480,
	BS_AUTORADIOBUTTON | WS_VISIBLE, 0, ID_SETUPBTN2, rect.x,
			rect.y + 3 * height, SELECT_BTN_W, SELECT_BTN_H, hwnd, 0);

	RightMenuWnd->Btn[5] = CreateWindowEx(CTRL_STATIC, FORLOOP_YES,
	WS_CHILD | WS_VISIBLE | SS_GROUPBOX, 0, ID_SETUPSTATIC1, rect.x,
			rect.y + 4 * height, rect.w, rect.h, hwnd, 0);
#if 0
	RightMenuWnd->Btn[4] = CreateWindowEx(CTRL_BUTTON, ROTATE_YES,
			BS_AUTORADIOBUTTON | WS_VISIBLE | WS_TABSTOP | WS_GROUP, 0, ID_SETUPBTN3,
			rect.x, rect.y + 5 * height, SELECT_BTN_W, SELECT_BTN_H, hwnd, 0);
#endif

	RightMenuWnd->Btn[4] = CreateWindowEx(CTRL_BUTTON, FORLOOP_YES,
	BS_AUTORADIOBUTTON | WS_VISIBLE | WS_TABSTOP | WS_GROUP, 0, ID_SETUPBTN4,
			rect.x, rect.y + 5 * height, SELECT_BTN_W, SELECT_BTN_H, hwnd, 0);

	if (rect.h < 480 && rect.h > 320) {
		RightMenuWnd->Btn[6] = CreateWindowEx(CTRL_BUTTON, OK,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_OK, rect.x + 10, rect.y + 250,
		OK_BTN_W, OK_BTN_H, hwnd, 0);

		RightMenuWnd->Btn[7] = CreateWindowEx(CTRL_BUTTON, CANCEL,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_CANCEL, rect.x + 100,
				rect.y + 250, CANCEL_BTN_W, CANCEL_BTN_H, hwnd, 0);
		for (i = 0; i < 8; ++i) {
			SetWindowFont(RightMenuWnd->Btn[i], RightMenuWnd->logFont15);
		}

	} else if (rect.h > 480) {

		RightMenuWnd->Btn[6] = CreateWindowEx(CTRL_BUTTON, OK,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_OK, rect.x + 50, rect.y + 500,
		OK_BTN_W + 10, OK_BTN_H + 10, hwnd, 0);

		RightMenuWnd->Btn[7] = CreateWindowEx(CTRL_BUTTON, CANCEL,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_CANCEL, rect.x + 200,
				rect.y + 500, CANCEL_BTN_W + 10, CANCEL_BTN_H + 10, hwnd, 0);
		for (i = 0; i < 8; ++i) {
			SetWindowFont(RightMenuWnd->Btn[i], RightMenuWnd->logFont25);
		}
	} else if (rect.h < 320) {
		RightMenuWnd->Btn[6] = CreateWindowEx(CTRL_BUTTON, OK,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_OK, rect.x + 50, rect.y + 180,
		OK_BTN_W, OK_BTN_H, hwnd, 0);

		RightMenuWnd->Btn[7] = CreateWindowEx(CTRL_BUTTON, CANCEL,
		WS_CHILD | WS_VISIBLE | BS_CHECKED, 0, ID_CANCEL, rect.x + 200,
				rect.y + 180, CANCEL_BTN_W, CANCEL_BTN_H, hwnd, 0);
		for (i = 0; i < 8; ++i) {
			SetWindowFont(RightMenuWnd->Btn[i], RightMenuWnd->logFont15);
		}
	}

	if (setupData->ScreenResolution1280x800 == 1) {
		SendMessage(RightMenuWnd->Btn[1], BM_SETCHECK, BST_CHECKED, 0);
	} else if (setupData->ScreenResolution800x480 == 1) {
		SendMessage(RightMenuWnd->Btn[2], BM_SETCHECK, BST_CHECKED, 0);
	} else if (setupData->ScreenResolution1280x480 == 1) {
		SendMessage(RightMenuWnd->Btn[3], BM_SETCHECK, BST_CHECKED, 0);
	}
	if (setupData->IsForLoop == 1) {
		SendMessage(RightMenuWnd->Btn[5], BM_SETCHECK, BST_CHECKED, 0);
	}
}

int len = 0;
void openMenu(HWND hwnd) {
	printf("1 bu\n");
	len = RightMenuWnd->rightSize.w - RightMenuWnd->rightSize.x;
	MSG msg;
	HDC sec_dc_active;
	sec_dc_active = GetSecondaryDC(hwnd);
	printf("2 bu\n");
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DONOTHING);
	printf("3 bu\n");
	ShowWindow(hwnd, SW_SHOWNORMAL);
	printf("4 bu\n");
#if 0
	while (GetMessage(&msg, hwnd)) {
		if (msg.message == MSG_IDLE)
		break;
		DispatchMessage(&msg);
	}
#endif
	printf("5 bu\n");
	int distance = RightMenuWnd->rightSize.x;
	BitBlt(sec_dc_active, 0, 0, len, RightMenuWnd->rightSize.h, HDC_SCREEN,
			RightMenuWnd->rightSize.x, RightMenuWnd->rightSize.y, 0);
	usleep(900000);
	printf("6 bu\n");
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
	printf("7 bu\n");
	headbar->isRightWndOpen = TRUE;
	printf("open ok\n");
}

static int RightMenuProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	int curindex = 0;
	switch (message) {
	case MSG_FONTCHANGED:
		return 0;
	case MSG_CREATE:
		RightMenuWnd->logFont20 = getLogFont(ID_FONT_TIMES_20);
		RightMenuWnd->logFont25 = getLogFont(ID_FONT_TIMES_25);
		switch (RightMenuWnd->addID) {
		case ID_LIST:
			CreateListFileName(hWnd, RightMenuWnd->addfiletype);
			SetTimer(hWnd, _ID_TIMER_RIGHTHIDE, 300);
			break;
		case ID_SETUP:
			CreateSetup(hWnd);
			break;
		}
		return 0;
	case MSG_COMMAND:
		switch (wParam) {
		case ID_SETUPBTN0:
			headbar->GetBtnNum = 1;
			break;
		case ID_SETUPBTN1:
			headbar->GetBtnNum = 2;
			break;
		case ID_SETUPBTN2:
			headbar->GetBtnNum = 3;
			break;
#if 0
			case ID_SETUPBTN3:
			headbar->GetBtnNum = 4;
			break;
#endif
		case ID_SETUPBTN4:
			headbar->GetBtnNum = 5;
			break;
		case ID_OK:
			if (headbar->GetBtnNum == 1) {
				headbar->Displayrect.x = 0;
				headbar->Displayrect.y = 0;
				headbar->Displayrect.w = 1280;
				headbar->Displayrect.h = 800;
				SendMessage(RightMenuWnd->Btn[0], BM_SETCHECK, BST_CHECKED, 0);
				headbar->is1280x800 = 1;
				headbar->is800x480 = 0;
				headbar->is1280x480 = 0;
			} else if (headbar->GetBtnNum == 2) {
				headbar->Displayrect.x = 0;
				headbar->Displayrect.y = 0;
				headbar->Displayrect.w = 800;
				headbar->Displayrect.h = 480;
				SendMessage(RightMenuWnd->Btn[1], BM_SETCHECK, BST_CHECKED, 0);
				headbar->is800x480 = 1;
				headbar->is1280x800 = 0;
				headbar->is1280x480 = 0;
			} else if (headbar->GetBtnNum == 3) {
				headbar->Displayrect.x = 0;
				headbar->Displayrect.y = 0;
				headbar->Displayrect.w = 1280;
				headbar->Displayrect.h = 480;
				SendMessage(RightMenuWnd->Btn[2], BM_GETCHECK, BST_CHECKED, 0);
				headbar->is1280x800 = 0;
				headbar->is800x480 = 0;
				headbar->is1280x480 = 1;
			} else {
				DispGetScreenWidth(&headbar->Displayrect);
				DispGetScreenHeight(&headbar->Displayrect);
				tplayer_setdisplayrect(0, 0, headbar->Displayrect.w,
						headbar->Displayrect.h);
			}
#if 0
			if (headbar->GetBtnNum == 4) {
				SendMessage(RightMenuWnd->Btn[3], BM_GETCHECK, BST_CHECKED, 0);
				headbar->isrotate = 1;
				headbar->isforloop = 0;
			} else
#endif
			if (headbar->GetBtnNum == 5) {
				SendMessage(RightMenuWnd->Btn[4], BM_GETCHECK, BST_CHECKED, 0);
				headbar->isrotate = 0;
				headbar->isforloop = 1;
			}

			RightMenuWnd->boolfreespace = 1;
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;

			break;
		case ID_CANCEL:
			RightMenuWnd->boolfreespace = 1;
			tplayer_setdisplayrect(0, 0, g_rcScr.right, g_rcScr.bottom);
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
			headbar->isRightWndOpen = FALSE;
			break;
		}
		return 0;
#if 0
		case MSG_ERASEBKGND:
		hdc = BeginPaint(hWnd);
		SetMemDCColorKey(hdc, MEMDC_FLAG_SRCCOLORKEY,PIXEL_transparent);
		EndPaint(hWnd, hdc);
		return 0;
#endif

	case MSG_KEYUP:
		switch (wParam) {
		case CVR_KEY_OK:
			sm_debug("right CVR_KEY_OK\n");
			if (bottom->addID == ID_PICTURE) {
				startPicture();
			} else if (bottom->addID == ID_PLAYER
					|| bottom->addID == ID_MUSIC) {
				startMusic();
			}
			break;
		case CVR_KEY_MODE:
			if (bottom->isPlaying == FALSE) {
				pauseMusic();
			}
			break;
		case CVR_KEY_LEFT:
			sm_debug("right CVR_KEY_LEFT\n");
			if (bottom->addID == ID_PICTURE) {
				startPictureLeft();
			} else if (bottom->addID == ID_PLAYER
					|| bottom->addID == ID_MUSIC) {
				startMusicLeft();
			}

			break;
		case CVR_KEY_RIGHT:
			sm_debug("right CVR_KEY_RIGHT\n");
			if (bottom->addID == ID_PICTURE) {
				startPictureRight();
			} else if (bottom->addID == ID_PLAYER
					|| bottom->addID == ID_MUSIC) {
				startMusicRight();
			}
			break;
		case CVR_KEY_POWER:
			sm_debug("CVR_KEY_POWER\n");
			head_back(headbar->backHwnd);
			g_win_data->curIndex = 0;
			break;

		}
		return 0;
	case MSG_TIMER:
		curindex = SendMessage(RightMenuWnd->ListBox0, LB_GETCURSEL, 0, 0L);
		/*if (headbar->isRightWndOpen == TRUE && curindex == LB_ERR) {
		 ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		 headbar->isRightWndOpen = FALSE;
		 }*/
		if (curindex == LB_ERR) {
			ShowWindow(RightMenuWnd->RightMenuHwnd, SW_HIDE);
		}
		KillTimer(hWnd, _ID_TIMER_RIGHTHIDE);
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		break;
	case MSG_DESTROY:
		if (NULL != RightMenuWnd) {
			unloadBitMap(&RightMenuWnd->nextbuttonBmp);
			unloadBitMap(&RightMenuWnd->prebuttonBmp);
			if (headbar->isCreateMediaList == TRUE) {
				destroy_media_list();
			}
			int i = 0;
			for (i = 0; i < FILE_MAX; i++) {
				if (RightMenuWnd->filename_list[i]) {
					free(RightMenuWnd->filename_list[i]);
					RightMenuWnd->filename_list[i] = NULL;
				}
				if (RightMenuWnd->filepath_list[i]) {
					free(RightMenuWnd->filepath_list[i]);
					RightMenuWnd->filepath_list[i] = NULL;
				}
			}
			if (RightMenuWnd->boolfreespace == 0
					|| RightMenuWnd->boolfreespace == -1) {
				free(RightMenuWnd);
				RightMenuWnd = NULL;
			}
		} else {
			printf("right window == NULL\n");
		}
		DestroyAllControls(hWnd);
		return 0;
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int RightResourceInit() {
	RightMenuWnd = (RightMenuDataType*) malloc(sizeof(RightMenuDataType));
	if (NULL == RightMenuWnd) {
		sm_error("malloc usb window data error\n");
		return -1;
	}
	memset((void *) RightMenuWnd, 0, sizeof(RightMenuDataType));

	return 0;
}
void InitRightData() {
	RightMenuWnd->boolfreespace = -1;
	RightMenuWnd->isButtonNext = TRUE;
	RightMenuWnd->isButtonPre = TRUE;
	RightMenuWnd->paginal_residual_number = 0;
	RightMenuWnd->paginal_number = 1;
}
int RightMenuInit(HWND hosting, int id, int filetype) {
	MSG Msg;
	MAINWINCREATE CreateInfo;
	if (headbar->isRightWndOpen == TRUE) {
		printf("right menu isRightWndOpen == TRUE\n");
		return -1;
	}
	RightResourceInit();
	RightMenuWnd->addID = id;
	RightMenuWnd->addfiletype = filetype;
	RightMenuWnd->listboxskip = 0;
	InitRightData();
	getResRect(ID_RIGHT_MENU, &RightMenuWnd->rightSize);
	printf("right menu:[%d][%d][%d][%d]\n", RightMenuWnd->rightSize.x,
			RightMenuWnd->rightSize.y, RightMenuWnd->rightSize.w,
			RightMenuWnd->rightSize.h);
	if (RightMenuWnd->filename_list[0])
		printf("%s\n", RightMenuWnd->filename_list[0]);
	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = RightMenuProc;
	CreateInfo.lx = RightMenuWnd->rightSize.x;
	CreateInfo.ty = RightMenuWnd->rightSize.y;
	CreateInfo.rx = RightMenuWnd->rightSize.w;
	CreateInfo.by = RightMenuWnd->rightSize.h;
/*	CreateInfo.iBkColor = PIXEL_transparent;*/
	CreateInfo.iBkColor = PIXEL_black;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;
	RightMenuWnd->RightMenuHwnd = CreateMainWindow(&CreateInfo);
	if (RightMenuWnd->RightMenuHwnd == HWND_INVALID)
		return -1;
	/*openMenu(RightMenuWnd->RightMenuHwnd);*/
	ShowWindow(RightMenuWnd->RightMenuHwnd, SW_SHOWNORMAL);

	MainWindowThreadCleanup(RightMenuWnd->RightMenuHwnd);
	return 0;
}
