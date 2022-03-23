#include "MessageBox.h"
#include "CvrResource.h"

#define IDC_BUTTON_OK		100
#define IDC_BUTTON_START	100
#define IDC_BUTTON_CANCEL	101
#define BUTTON_SELECTED	 0
#define BUTTON_UNSELECTED	 1

#define IDC_TITLE		650
#define IDC_TEXT		651
#define SS_VCENTER		0x00000040
#define TIMER_CONFIRM_CALLBACK	200

typedef struct
{
	unsigned int dwStyle;
	unsigned int IDCSelected;
	unsigned char flag_end_key;
	gal_pixel linecTitle;
	gal_pixel linecItem;
	unsigned int buttonNRs;
	void (*confirmCallback)(HWND hDlg, void* data);
	void* confirmData;
	const char* msg4ConfirmCallback;
	int id;
}MBPrivData;

bool bMsgBoxEn = 0;
HWND msgBoxHwnd = NULL;
static gal_pixel gp_hilite_bgc;
static gal_pixel gp_hilite_fgc;
static gal_pixel gp_normal_bgc;
static gal_pixel gp_normal_fgc;

void MsgBoxLine2 (HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveTo(hdc, x1, y1);
	LineTo(hdc, x2, y2);
}

static BOOL MsgtimerCallback(HWND hDlg, int id, DWORD data)
{
	MBPrivData* privData;
	privData = (MBPrivData *)GetWindowAdditionalData(hDlg);

	if(id == TIMER_CONFIRM_CALLBACK)
	{
		(*privData->confirmCallback)(hDlg , privData->confirmData);
	}

	return FALSE;
}

int CloseMessageBox(void)
{
	if(!bMsgBoxEn)
	{
		//cvr_warn("msgbox has aready disable\n");
		return -1;
	}
	bMsgBoxEn = 0;

	SendNotifyMessage(msgBoxHwnd, MSG_DIALOG_CLOSE_BOX, 0, 0);
	return 0;
}

static void drawButton(HWND hDlg, HDC IDCButton, unsigned int flag)
{
	HWND hWnd;
	RECT rect;
	MBPrivData* privData;
	DWORD old_bkmode;
	gal_pixel old_color;
	HDC hdc;
	char* text;

	privData = (MBPrivData*)GetWindowAdditionalData(hDlg);
	hWnd = GetDlgItem(hDlg, IDCButton);
	GetClientRect(hWnd, &rect);
	hdc = GetDC(hWnd);

	if(flag == BUTTON_SELECTED)
	{
		old_color = SetBrushColor(hdc, gp_hilite_bgc);
		old_bkmode = SetBkMode(hdc, BM_TRANSPARENT);
		FillBox(hdc, rect.left, rect.top, RECTW(rect), RECTH(rect));
		SetTextColor(hdc, gp_hilite_fgc);
		text = (char*)GetWindowAdditionalData2(hWnd);
		DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SetBrushColor(hdc, old_color);
		SetBkMode(hdc, old_bkmode);
	}
	else if(flag == BUTTON_UNSELECTED)
	{
		old_color = SetBrushColor(hdc, gp_normal_bgc);
		old_bkmode = SetBkMode(hdc, BM_TRANSPARENT);
		FillBox(hdc, rect.left, rect.top, RECTW(rect), RECTH(rect));
		SetTextColor (hdc, gp_normal_fgc);
		text = (char*)GetWindowAdditionalData2(hWnd);
		DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SetBrushColor(hdc, old_color);
		SetBkMode(hdc, old_bkmode);
	}

	ReleaseDC(hdc);
}


static int msgBoxProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam )
{
	MBPrivData* privData;

	//cvr_debug("msgBoxProc is 0x%x, 0x%x, 0x%x, 0x%x\n", hDlg, message, wParam, lParam);
	switch(message) {
	case MSG_FONTCHANGED:
		{
			HWND hChild;
			hChild = GetNextChild(hDlg, 0);
			while( hChild && (hChild != HWND_INVALID) )
			{
				SetWindowFont(hChild, GetWindowFont(hDlg));
				hChild = GetNextChild(hDlg, hChild);
			}
		}
		break;
	case MSG_INITDIALOG:
		{
			unsigned int dwStyle;
			MessageBox_t* info = (MessageBox_t *)lParam;
			if(info == NULL) {
				cvr_error("invalid info\n");
				return -1;
			}
			dwStyle = info->dwStyle & MB_TYPEMASK;

			privData = (MBPrivData*)malloc(sizeof(MBPrivData));
			privData->dwStyle = dwStyle;
			if(privData->dwStyle & MB_OKCANCEL)
			{
				privData->IDCSelected = IDC_BUTTON_START + 1;
				privData->buttonNRs = 2;
			}
			else
			{
				privData->IDCSelected = IDC_BUTTON_START;
				privData->buttonNRs = 1;
			}

			privData->flag_end_key = info->flag_end_key;
			privData->linecTitle = info->linecTitle;
			privData->linecItem = info->linecItem;
			privData->confirmCallback = info->confirmCallback;
			privData->confirmData = info->confirmData;
			privData->msg4ConfirmCallback = info->msg4ConfirmCallback;
			SetWindowAdditionalData(hDlg, (DWORD)privData);

			SetWindowFont(hDlg, info->pLogFont);

			msgBoxHwnd = hDlg;
			gp_hilite_bgc = GetWindowElementPixel(hDlg, WE_BGC_HIGHLIGHT_ITEM);
			gp_hilite_fgc = GetWindowElementPixel(hDlg, WE_FGC_HIGHLIGHT_ITEM);

			gp_normal_bgc = info->bgcWidget;
			gp_normal_fgc = info->fgcWidget;
			SetWindowBkColor(hDlg, gp_normal_bgc);

			SetWindowElementAttr(GetDlgItem(hDlg, IDC_TITLE), WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, gp_normal_fgc));
			SetWindowElementAttr(GetDlgItem(hDlg, IDC_TEXT), WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, gp_normal_fgc));
			SetWindowText(GetDlgItem(hDlg, IDC_TITLE), info->title);
			SetWindowText(GetDlgItem(hDlg, IDC_TEXT), info->text);

			for(unsigned int i = 0; i < privData->buttonNRs; i++)
			{
				SetWindowElementAttr( GetDlgItem(hDlg, IDC_BUTTON_START + i), WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, gp_normal_fgc));
				SetWindowAdditionalData2( GetDlgItem(hDlg, IDC_BUTTON_START + i), (DWORD)info->buttonStr[i]);
			}
		}
		break;
	case MSG_KEYUP:
		{
			privData = (MBPrivData *)GetWindowAdditionalData(hDlg);
			switch(wParam) {
			case CVR_KEY_OK:
				if(privData->flag_end_key == 1)
				{
					if( (privData->IDCSelected == IDC_BUTTON_START) && privData->confirmCallback)
					{
						if(privData->msg4ConfirmCallback)
						{
							SetWindowText(GetDlgItem(hDlg, IDC_TEXT), privData->msg4ConfirmCallback);
							SetTimerEx(hDlg, TIMER_CONFIRM_CALLBACK, 5, MsgtimerCallback);
							SetNullFocus(hDlg);
						}
						else
						{
							(*privData->confirmCallback)(hDlg , privData->confirmData);
						}
					}
					else
					{
						CloseMessageBox();
					}
				}
				break;
			case CVR_KEY_MODE:
				if(privData->flag_end_key == 1)
				{
					CloseMessageBox();
				}
				break;

			case CVR_KEY_LEFT:
				privData->IDCSelected--;
				if(privData->IDCSelected < IDC_BUTTON_START)
				{
					privData->IDCSelected = IDC_BUTTON_START + privData->buttonNRs - 1;
				}
				InvalidateRect(hDlg, NULL, TRUE);
				break;

			case CVR_KEY_RIGHT:
				privData->IDCSelected++;
				if(privData->IDCSelected > (IDC_BUTTON_START + privData->buttonNRs - 1) )
				{
					privData->IDCSelected = IDC_BUTTON_START;
				}
				InvalidateRect(hDlg, NULL, TRUE);
				break;
			default:
				break;
			}
		}
		break;
	case MSG_KEYDOWN:
		{
			RECT rect;
			GetClientRect(hDlg, &rect);
			privData = (MBPrivData *)GetWindowAdditionalData(hDlg);
			switch(wParam) {
			case CVR_KEY_OK:
				if(privData->flag_end_key == 0)
				{
					if( (privData->IDCSelected == IDC_BUTTON_START) && privData->confirmCallback )
					{
						if(privData->msg4ConfirmCallback)
						{
							SetWindowText(GetDlgItem(hDlg, IDC_TEXT), privData->msg4ConfirmCallback);
							SetTimerEx(hDlg, TIMER_CONFIRM_CALLBACK, 5, MsgtimerCallback);
							SetNullFocus(hDlg);
						}
						else
						{
							(*privData->confirmCallback)(hDlg , privData->confirmData);
						}
					}
					else
					{
						CloseMessageBox();
					}
				}
				break;
			case CVR_KEY_MODE:
				if(privData->flag_end_key == 0)
				{
					CloseMessageBox();
				}
				break;

			default:
				break;
			}
		}
		break;
	case MSG_PAINT:
		{
			RECT rect, rect1;
			HDC hdc = BeginPaint(hDlg);
			privData = (MBPrivData *)GetWindowAdditionalData(hDlg);

			GetClientRect(GetDlgItem(hDlg, IDC_TITLE), &rect);
			GetClientRect(hDlg, &rect1);
			SetPenColor(hdc, privData->linecTitle );
			MsgBoxLine2(hdc, 1, RECTH(rect) + 1, RECTW(rect) - 2, RECTH(rect) + 1);

			SetPenColor(hdc, privData->linecItem );
			MsgBoxLine2(hdc, 1, RECTH(rect1) * 3 / 4 - 2, RECTW(rect1) - 2, RECTH(rect1) * 3 / 4 - 2);

			unsigned int i;
			for(i = 0; i < privData->buttonNRs; i++)
			{
				if(IDC_BUTTON_START + i == privData->IDCSelected)
				{
					drawButton(hDlg, IDC_BUTTON_START + i, BUTTON_SELECTED);
				}
				else
				{
					drawButton(hDlg, IDC_BUTTON_START + i, BUTTON_UNSELECTED);
				}
			}

			EndPaint(hDlg, hdc);
		}
		break;
	case MSG_DESTROY:
		{
			privData = (MBPrivData *)GetWindowAdditionalData(hDlg);
			if(privData)
			{
				free(privData);
			}
		}
		break;
	case MSG_DIALOG_CLOSE_BOX:
		EndDialog(hDlg, 0);
		break;
	default:
		break;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int showMessageBox (HWND hParentWnd, MessageBox_t *info)
{
	CvrRectType rect;
	unsigned int dwStyle;
	int	 retval;

	DLGTEMPLATE MsgBoxDlg;
	CTRLDATA *CtrlData = NULL;
	unsigned int controlNrs;

	if(bMsgBoxEn)
	{
		cvr_warn("tip label has aready enable\n");
		return -1;
	}
	bMsgBoxEn = 1;

	dwStyle = info->dwStyle & MB_TYPEMASK;
	rect = info->rect;

	controlNrs = 3;
	if(dwStyle & MB_OKCANCEL)
	{
		controlNrs = 4;
		cvr_debug("button dwStyle is MB_OKCANCEL\n");
	}

	CtrlData = (PCTRLDATA)malloc(sizeof(CTRLDATA) * controlNrs);
	memset(CtrlData, 0, sizeof(CTRLDATA) * controlNrs);
	memset(&MsgBoxDlg, 0, sizeof(MsgBoxDlg));
	MsgBoxDlg.dwStyle = WS_NONE;
	MsgBoxDlg.dwExStyle = WS_EX_USEPARENTFONT;

	CtrlData[0].class_name = CTRL_STATIC;
	CtrlData[0].dwStyle = SS_CENTER | SS_VCENTER;
	CtrlData[0].dwExStyle = WS_EX_USEPARENTFONT | WS_EX_TRANSPARENT;
	CtrlData[0].id = IDC_TITLE;
	CtrlData[0].caption = "";
	if(dwStyle & MB_HAVE_TITLE)
	{
		CtrlData[0].dwStyle |= WS_VISIBLE;
		CtrlData[0].x = 0;
		CtrlData[0].y = 0;
		CtrlData[0].w = rect.w;
		CtrlData[0].h = rect.h / 4;
	}
	else
	{
		CtrlData[0].x = 0;
		CtrlData[0].y = 0;
		CtrlData[0].w = rect.w;
		CtrlData[0].h = rect.h / 8;
	}

	CtrlData[1].class_name = CTRL_STATIC;
	CtrlData[1].dwStyle = SS_LEFT | SS_CENTER | SS_VCENTER;
	CtrlData[1].dwExStyle = WS_EX_USEPARENTFONT | WS_EX_TRANSPARENT;
	CtrlData[1].id = IDC_TEXT;
	CtrlData[1].caption = "";
	if(dwStyle & MB_HAVE_TEXT)
	{
		CtrlData[1].dwStyle |= WS_VISIBLE;
		CtrlData[1].x = rect.w / 16;
		CtrlData[1].y = CtrlData[0].h + CtrlData[0].y + 20;
		CtrlData[1].w = rect.w - CtrlData[1].x;
		CtrlData[1].h = rect.h / 2 - 5;
	}
	else
	{
		CtrlData[1].x = rect.w / 16;
		CtrlData[1].y = CtrlData[0].h + CtrlData[0].y + 20;
		CtrlData[1].w = rect.w - CtrlData[1].x;
		CtrlData[1].h = rect.h / 8;
	}

	CtrlData[2].class_name = CTRL_STATIC;
	CtrlData[2].dwStyle = SS_CENTER | SS_VCENTER | WS_VISIBLE;
	CtrlData[2].dwExStyle = WS_EX_USEPARENTFONT | WS_EX_TRANSPARENT;
	CtrlData[2].id = IDC_BUTTON_START;
	CtrlData[2].caption = "";

	if(dwStyle & MB_OKCANCEL)
	{
		CtrlData[3].class_name = CTRL_STATIC;
		CtrlData[3].dwStyle = SS_CENTER | SS_VCENTER | WS_VISIBLE;
		CtrlData[3].dwExStyle = WS_EX_USEPARENTFONT | WS_EX_TRANSPARENT;
		CtrlData[3].id = IDC_BUTTON_START + 1;
		CtrlData[3].caption = "";
	}

	if( (dwStyle & MB_HAVE_TEXT) || (dwStyle & MB_HAVE_TITLE) )
	{
		if(dwStyle & MB_OKCANCEL)
		{
			CtrlData[2].x = 1;
			CtrlData[2].y = rect.h * 3 / 4 + 1;
			CtrlData[2].w = rect.w / 2 - 2;
			CtrlData[2].h = rect.h / 4 - 2;

			CtrlData[3].x = rect.w / 2 + 1;
			CtrlData[3].y = rect.h * 3 / 4 + 1;
			CtrlData[3].w = rect.w / 2 - 2;
			CtrlData[3].h = rect.h / 4 - 2;
		}
		else
		{
			CtrlData[2].x = rect.w / 4;
			CtrlData[2].y = rect.h * 3 / 4 + 1;
			CtrlData[2].w = rect.w / 2;
			CtrlData[2].h = rect.h / 4 - 2;
		}
	}
	else
	{
		if(dwStyle & MB_OKCANCEL)
		{
			CtrlData[2].x = 1;
			CtrlData[2].y = CtrlData[1].y + CtrlData[1].h + 2;
			CtrlData[2].w = rect.w / 2 - 2;
			CtrlData[2].h = rect.h - (CtrlData[2].y + 8);

			CtrlData[3].x = rect.w / 2 + 1;
			CtrlData[3].y = CtrlData[1].y + CtrlData[1].h + 2;
			CtrlData[3].w = rect.w / 2 - 2;
			CtrlData[3].h = rect.h - (CtrlData[3].y + 8);
		}
		else
		{
			CtrlData[2].x = rect.w / 4;
			CtrlData[2].y = CtrlData[1].y + CtrlData[1].h + 5;
			CtrlData[2].w = rect.w / 2;
			CtrlData[2].h = rect.h - CtrlData[2].y - 5;
		}
	}

	MsgBoxDlg.x = rect.x;
	MsgBoxDlg.y = rect.y;
	MsgBoxDlg.w = rect.w;
	MsgBoxDlg.h = rect.h;
	MsgBoxDlg.caption = "";
	MsgBoxDlg.controlnr = controlNrs;
	MsgBoxDlg.controls = CtrlData;

	retval = DialogBoxIndirectParam(&MsgBoxDlg, hParentWnd, msgBoxProc, (LPARAM)info);
	free(CtrlData);
	return retval;
}

/*
int getMessageBoxData(unsigned int menuIndex, MessageBox_t *messageBoxData)
{
	const char* ptr;
	unsigned int i;

	unsigned int indexTable [] = {
		MENU_INDEX_FORMAT,
		MENU_INDEX_FRESET
	};

	unsigned int resCmd[][2] = {
		{LANG_LABEL_SUBMENU_FORMAT_TITLE, LANG_LABEL_SUBMENU_FORMAT_TEXT},
		{LANG_LABEL_SUBMENU_FRESET_TITLE, LANG_LABEL_SUBMENU_FRESET_TEXT},
	};

	messageBoxData->dwStyle = MB_OKCANCEL | MB_HAVE_TITLE | MB_HAVE_TEXT;
	messageBoxData->flag_end_key = 1;

	for(i = 0; i < TABLESIZE(indexTable); i++) {
		if(indexTable[i] == menuIndex) {
			ptr = getLabel(resCmd[i][0]);
			if(ptr == NULL) {
				return -1;
			}
			messageBoxData->title = ptr;

			getLabel(resCmd[i][1]);
			if(ptr == NULL) {
				return -1;
			}
			messageBoxData->text = ptr;

			ptr = getLabel(LANG_LABEL_SUBMENU_OK);
			if(ptr == NULL) {
				return -1;
			}
			messageBoxData->buttonStr[0] = ptr;

			ptr = getLabel(LANG_LABEL_SUBMENU_CANCEL);
			if(ptr == NULL) {
				return -1;
			}
			messageBoxData->buttonStr[1] = ptr;
		}
	}

	if(i > TABLESIZE(indexTable)) {
		return -1;
	}

	messageBoxData->pLogFont = getLogFont();

	getResRect(ID_MENU_LIST_MB, &messageBoxData->rect);
	messageBoxData->fgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_FGC);
	messageBoxData->bgcWidget = getResColor(ID_MENU_LIST_MB, COLOR_BGC);
	messageBoxData->linecTitle = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_TITLE);
	messageBoxData->linecItem = getResColor(ID_MENU_LIST_MB, COLOR_LINEC_ITEM);

	return 0;
}
*/

/*
int ShowMessageBox( HWND mHwnd, unsigned int menuIndex,int val)
{
	int retval;
	MessageBox_t messageBoxData;

	memset(&messageBoxData, 0, sizeof(messageBoxData));
	if(getMessageBoxData(menuIndex, &messageBoxData) < 0) {
		return -1;
	}

	messageBoxData.text = getLabel(LANG_LABEL_FORMAT_SDCARD);
	retval = showMessageBox(mHwnd, &messageBoxData);
	return retval;
}
*/
