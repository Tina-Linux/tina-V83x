#include "CvrUsbWindow.h"
#include "CvrResource.h"
#include "ctrlclass.h"
#include "CvrMainWindow.h"

#define POPMENUS_V			0x01	// 垂直分布
#define POPMENUS_H			0x02
#define POPMENUS_VCENTER	0x04
#define POPMENUS_MASK		0x0F

#define VBORDER 5
#define HBORDER 5
#define BTN_WIDTH_BMP       20
#define BTN_HEIGHT_BMP      20
#define BTN_INTER_BMPTEXT   2

UsbWinDataType *usb=NULL;
static WNDPROC button_default_callback;

typedef struct tagBUTTONDATA
{
	DWORD status;           // button flags
	DWORD data;             // bitmap or icon of butotn
} BUTTONDATA;
typedef BUTTONDATA* PBUTTONDATA;

#define BUTTON_STATUS(pctrl)   (((PBUTTONDATA)((pctrl)->dwAddData2))->status)

#define BUTTON_GET_POSE(pctrl)  \
	(BUTTON_STATUS(pctrl) & BST_POSE_MASK)

#define BUTTON_SET_POSE(pctrl, status) \
	do { \
		BUTTON_STATUS(pctrl) &= ~BST_POSE_MASK; \
		BUTTON_STATUS(pctrl) |= status; \
	}while(0)

#define BUTTON_TYPE(pctrl)     ((pctrl)->dwStyle & BS_TYPEMASK)

#define BUTTON_IS_PUSHBTN(pctrl) \
	(BUTTON_TYPE(pctrl) == BS_PUSHBUTTON || \
	 BUTTON_TYPE(pctrl) == BS_DEFPUSHBUTTON)

#define BUTTON_IS_AUTO(pctrl) \
	(BUTTON_TYPE(pctrl) == BS_AUTO3STATE || \
	 BUTTON_TYPE(pctrl) == BS_AUTOCHECKBOX || \
	 BUTTON_TYPE(pctrl) == BS_AUTORADIOBUTTON)

static void btnGetRects (PCONTROL pctrl, RECT* prcClient, RECT* prcContent, RECT* prcBitmap)
{
	GetClientRect ((HWND)pctrl, prcClient);

	if (BUTTON_IS_PUSHBTN(pctrl) || (pctrl->dwStyle & BS_PUSHLIKE))
	{
		SetRect (prcContent, (prcClient->left   + BTN_WIDTH_BORDER),
				(prcClient->top    + BTN_WIDTH_BORDER),
				(prcClient->right  - BTN_WIDTH_BORDER),
				(prcClient->bottom - BTN_WIDTH_BORDER));

		if (BUTTON_GET_POSE(pctrl) == BST_PUSHED)
		{
			prcContent->left ++;
			prcContent->top ++;
			prcContent->right ++;
			prcContent->bottom++;
		}

		SetRectEmpty (prcBitmap);
		return;
	}

	if (pctrl->dwStyle & BS_LEFTTEXT) {
		SetRect (prcContent, prcClient->left + 1,
				prcClient->top + 1,
				prcClient->right - BTN_WIDTH_BMP - BTN_INTER_BMPTEXT,
				prcClient->bottom - 1);
		SetRect (prcBitmap, prcClient->right - BTN_WIDTH_BMP,
				prcClient->top + 1,
				prcClient->right - 1,
				prcClient->bottom - 1);
	}
	else {
		SetRect (prcContent, prcClient->left + BTN_WIDTH_BMP + BTN_INTER_BMPTEXT,
				prcClient->top + 1,
				prcClient->right - 1,
				prcClient->bottom - 1);
		SetRect (prcBitmap, prcClient->left + 1,
				prcClient->top + 1,
				prcClient->left + BTN_WIDTH_BMP,
				prcClient->bottom - 1);
	}
}

static void change_status_from_pose (PCONTROL pctrl, int new_pose, BOOL noti_pose_changing, BOOL change_valid, DWORD context)
{
	static int nt_code[3][3] = {
		/*normal->nochange, hilite, pressed*/
		{          0,  BN_HILITE, BN_PUSHED},
		/*hilite->normal, nochange, pressed*/
		{BN_UNHILITE,          0, BN_PUSHED},
		/*pressed->normal, hilite, nochange*/
		{BN_UNPUSHED, BN_UNPUSHED,        0},
	};

	DWORD dwStyle = pctrl->dwStyle;
	int old_pose = BUTTON_GET_POSE(pctrl);

	RECT rcClient;
	RECT rcContent;
	RECT rcBitmap;

	const WINDOW_ELEMENT_RENDERER* win_rdr;
	win_rdr = GetWindowInfo((HWND)pctrl)->we_rdr;

	/*make sure it can be changed, and avoid changing more than once*/
	MG_CHECK (old_pose!= BST_DISABLE && old_pose != new_pose);

	/*set new pose*/
	BUTTON_SET_POSE(pctrl, new_pose);
	if ((dwStyle & BS_NOTIFY) && noti_pose_changing)
	{
		if (new_pose != BST_DISABLE)
			NotifyParent ((HWND)pctrl, pctrl->id,
					nt_code[old_pose][new_pose]);
		/*disable the button*/
		else
		{
			/*before to disable, firstly to normal*/
			switch (old_pose)
			{
			case BST_PUSHED:
				NotifyParent ((HWND)pctrl, pctrl->id, BN_UNPUSHED);
			case BST_HILITE:
				NotifyParent ((HWND)pctrl, pctrl->id, BN_UNHILITE);
			case BST_NORMAL:
				NotifyParent ((HWND)pctrl, pctrl->id, BN_DISABLE);
				break;
			default:
				assert (0);
				break;
			}
		}
	}

	/*complete a click if the click if valid
	 * (mouse lbutton up in button )*/
	if (old_pose == BST_PUSHED && new_pose != BST_DISABLE
			&& change_valid)
	{
		NotifyParentEx((HWND)pctrl, pctrl->id, BN_CLICKED, context);
	}

	/* DK: When the radio or check button state changes do not erase the background,
	 * and the prospects for direct rendering */
	btnGetRects (pctrl, &rcClient, &rcContent, &rcBitmap);
	if (BUTTON_IS_PUSHBTN(pctrl) || (pctrl->dwStyle & BS_PUSHLIKE))
		InvalidateRect((HWND)pctrl, NULL, TRUE);
	else
		InvalidateRect((HWND)pctrl, &rcBitmap, FALSE);
}

static int btnGetTextFmt (int dwStyle)
{
	UINT dt_fmt;
	if (dwStyle & BS_MULTLINE)
		dt_fmt = DT_WORDBREAK;
	else
		dt_fmt = DT_SINGLELINE;

	if ((dwStyle & BS_TYPEMASK) == BS_PUSHBUTTON
			|| (dwStyle & BS_TYPEMASK) == BS_DEFPUSHBUTTON
			|| (dwStyle & BS_PUSHLIKE))
		dt_fmt |= DT_CENTER | DT_VCENTER;
	else {
		if ((dwStyle & BS_ALIGNMASK) == BS_RIGHT)
			dt_fmt = DT_WORDBREAK | DT_RIGHT;
		else if ((dwStyle & BS_ALIGNMASK) == BS_CENTER)
			dt_fmt = DT_WORDBREAK | DT_CENTER;
		else dt_fmt = DT_WORDBREAK | DT_LEFT;

		if ((dwStyle & BS_ALIGNMASK) == BS_TOP)
			dt_fmt |= DT_SINGLELINE | DT_TOP;
		else if ((dwStyle & BS_ALIGNMASK) == BS_BOTTOM)
			dt_fmt |= DT_SINGLELINE | DT_BOTTOM;
		else dt_fmt |= DT_SINGLELINE | DT_VCENTER;
	}

	return dt_fmt;
}

static void paint_content_focus(HDC hdc, PCONTROL pctrl, RECT* prc_cont)
{
	DWORD dt_fmt = 0;
	BOOL is_get_fmt = 0;
	gal_pixel old_pixel;
	gal_pixel text_pixel;
	gal_pixel frame_pixel;
	RECT focus_rc;
	int status, type;
	const WINDOW_ELEMENT_RENDERER* win_rdr;

	status = BUTTON_STATUS (pctrl);
	type = BUTTON_TYPE (pctrl);
	win_rdr = GetWindowInfo((HWND)pctrl)->we_rdr;

	/*draw button content*/
	switch (pctrl->dwStyle & BS_CONTENTMASK)
	{
	case BS_BITMAP:
		break;
	case BS_ICON:
		break;
	default:
		if(BUTTON_STATUS(pctrl) & BST_FOCUS) {
			text_pixel = DWORD2Pixel(HDC_SCREEN, 0xFF292929);
		} else {
			text_pixel = DWORD2Pixel(HDC_SCREEN, 0xFFDCDCDC);
		}

		old_pixel = SetTextColor(hdc, text_pixel);
		dt_fmt = btnGetTextFmt(pctrl->dwStyle);
		is_get_fmt = 1;
		SetBkMode (hdc, BM_TRANSPARENT);
		DrawText (hdc, pctrl->spCaption, -1, prc_cont, dt_fmt);
		SetTextColor(hdc, old_pixel);

		/*disable draw text*/
		if ((BUTTON_GET_POSE(pctrl) == BST_DISABLE)
				| (pctrl->dwStyle & WS_DISABLED))
			win_rdr->disabled_text_out ((HWND)pctrl, hdc, pctrl->spCaption,
					prc_cont, dt_fmt);
	}

	/*draw focus frame*/
	if (BUTTON_STATUS(pctrl) & BST_FOCUS)
	{
		focus_rc = *prc_cont;

		if (!BUTTON_IS_PUSHBTN(pctrl) &&
				!(pctrl->dwStyle & BS_PUSHLIKE) &&
				is_get_fmt)
		{
			dt_fmt |= DT_CALCRECT;
			DrawText (hdc, pctrl->spCaption, -1, &focus_rc, dt_fmt);
		}
		frame_pixel = DWORD2Pixel(HDC_SCREEN, 0xFF050505);
#ifndef USE_IPS_SCREEN
		win_rdr->draw_focus_frame(hdc, &focus_rc, frame_pixel);   //UI@Button_FocusFrame
#endif
	}
}

static void paint_push_btn (HDC hdc, PCONTROL pctrl)
{
	const WINDOW_ELEMENT_RENDERER* win_rdr;
	DWORD main_color;

	RECT rcClient;
	RECT rcContent;
	RECT rcBitmap;
	win_rdr = GetWindowInfo((HWND)pctrl)->we_rdr;
	main_color = GetWindowElementAttr((HWND)pctrl, WE_MAINC_THREED_BODY);
	btnGetRects (pctrl, &rcClient, &rcContent, &rcBitmap);
	win_rdr->draw_push_button((HWND)pctrl, hdc, &rcClient, main_color, 0xFFFFFFFF, BUTTON_STATUS(pctrl));

	paint_content_focus(hdc, pctrl, &rcContent);
}

static void connectToPCCallback(HWND hWnd, int id, int nc, DWORD context)
{
	HWND hDlg;
	int screen_w,screen_h;

	//cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, id, nc, context);

	screen_w = usb->usbSize.w;
	screen_h = usb->usbSize.h;
	hDlg = GetParent(hWnd);
	if(nc == BN_CLICKED)
	{
		if(id == ID_CONNECT2PC)
		{
			if(1/*cdrMain->getUSBStorageMode() == false*/)
			{
				cvr_debug("open usb storage mode\n");
				//ptr = rm->getLabel(LANG_LABEL_CLOSE_USB_STORAGE_DEVICE);
				if(1/*ptr*/)
				{
					if(format_is_correct() == 1)
					{
						if(!usb->bUsbStorageOn)
						{
							usb->bUsbStorageOn = CVR_TRUE;
							usb_storage_adcard_on();
						}
					}

					ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC),SW_HIDE);
					ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC+1),SW_HIDE);
					ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC+2),SW_HIDE);
					UpdateWindow(hDlg,1);
					HDC hdc = GetDC(hDlg);
					getResBmp(ID_CONNECT2PC, BMPTYPE_USB_STORAGE_MODEM, &usb->usbModeBmp);
					FillBoxWithBitmap(hdc,0,0,screen_w,screen_h,&usb->usbModeBmp);
					EndPaint(hDlg,hdc);
				}
				else
				{
					cvr_debug("get label failed\n");
				}
			}
		}
		else if(id == ID_CONNECT2PC + 1)
		{
			cvr_debug("charge mode, hDlg is 0x%x\n", hDlg);
			SendNotifyMessage(hDlg, MSG_DIALOG_CLOSE_PC, 0, 0);
			//cvr_debug("###################################_2\n");
		}
		else if (id == ID_CONNECT2PC + 2)
		{
			if(1/*cdrMain->getPCCamMode() == false*/)
			{
				cvr_debug("open PCCamMode\n");
				ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC),SW_HIDE);
				ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC+1),SW_HIDE);
				ShowWindow(GetDlgItem(hDlg,ID_CONNECT2PC+2),SW_HIDE);
				UpdateWindow(hDlg,1);
				HDC hdc = GetDC(hDlg);
				getResBmp(ID_CONNECT2PC, BMPTYPE_PCCAM_MODEM, &usb->pcCamBmp);
				FillBoxWithBitmap(hdc,0,0,screen_w,screen_h,&usb->pcCamBmp);
				EndPaint(hDlg,hdc);
			}
		}
	}
}

static int button_callback(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	PopUpMenuInfo_t *info;
	PCONTROL    pctrl;

	//cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hWnd, message, wParam, (s32)lParam);
	switch(message)	{
	case MSG_PAINT:
		{
			HDC hdc = BeginPaint (hWnd);

			pctrl   = gui_Control(hWnd);
			SelectFont (hdc, GetWindowFont(hWnd));

			if (BUTTON_IS_PUSHBTN(pctrl) || (pctrl->dwStyle & BS_PUSHLIKE)) {
				paint_push_btn(hdc, pctrl);
			}

			EndPaint (hWnd, hdc);
			return 0;
		}
		break;
	case MSG_SETFOCUS:
		{
			info = (PopUpMenuInfo_t *)GetWindowAdditionalData(GetParent(hWnd));
			SetWindowBkColor(hWnd, info->bgc_item_focus);
		}
		break;
	case MSG_KILLFOCUS:
		{
			info = (PopUpMenuInfo_t *)GetWindowAdditionalData(GetParent(hWnd));
			SetWindowBkColor(hWnd, info->bgc_item_normal);
		}
		break;
	case MSG_KEYDOWN:
	case MSG_KEYUP:
		return 0;
	default:
		break;
	}

	return (*button_default_callback)(hWnd, message, wParam, lParam);
}

static int DialogProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	static int mImageNum = 1;
	static BITMAP mShowAdasImage;

	//cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hDlg, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_INITDIALOG:
		{
			unsigned int count;
			PopUpMenuInfo_t *info = (PopUpMenuInfo_t *)lParam;
			if(info == NULL) {
				cvr_error("info is NULL\n");
				EndDialog(hDlg, -1);
			}
			SetWindowAdditionalData(hDlg, (DWORD)lParam);
			SetWindowFont(hDlg, info->pLogFont);
			SetWindowBkColor(hDlg, info->bgc_widget);
			SetFocusChild(GetDlgItem(hDlg, info->id));

			for(count = 0; count < info->item_nrs; count++) {
				if(info->callback[count]) {
					SetNotificationCallback(GetDlgItem(hDlg, info->id + count), info->callback[count]);
				}
				button_default_callback = SetWindowCallbackProc(GetDlgItem(hDlg, info->id + count), button_callback);

				#ifndef USE_IPS_SCREEN
				SetWindowElementAttr(GetDlgItem(hDlg, info->id + count), WE_MAINC_THREED_BODY, Pixel2DWORD(HDC_SCREEN, info->mainc_3dbox));
				#endif

				if(count == 0)
					SetWindowBkColor(GetDlgItem(hDlg, info->id + count), info->bgc_item_focus);
				else
					SetWindowBkColor(GetDlgItem(hDlg, info->id + count), info->bgc_item_normal);
			}
		}
		break;

		case MSG_FONTCHANGED:
		{
			PLOGFONT pLogFont;
			HWND hChild;

			pLogFont = GetWindowFont(hDlg);
			if(!pLogFont) {
				cvr_error("invalid logFont\n");
				break;
			}
			hChild = GetNextChild(hDlg, 0);
			while( hChild && (hChild != HWND_INVALID) )
			{
				SetWindowFont(hChild, pLogFont);
				hChild = GetNextChild(hDlg, hChild);
			}
		}
		break;

		case MSG_KEYDOWN:
		{
			PCONTROL    pCtrl;
			int id;
			PopUpMenuInfo_t *info;
			info = (PopUpMenuInfo_t *)GetWindowAdditionalData(hDlg);
			if(info->showImage == 1)
			{
					UpdateWindow( hDlg, 1);
					break;
			}

			id = GetDlgCtrlID(GetFocusChild(hDlg));
			if(id == -1)
				break;
			pCtrl   = gui_Control(GetDlgItem(hDlg, id));

			if(wParam == CVR_KEY_OK) {
				change_status_from_pose(pCtrl, BST_PUSHED, 1, 1, info->context);
				return 0;
			}
		}
		break;

		case MSG_KEYUP:
		{
			PCONTROL pCtrl;
			int id;
			PopUpMenuInfo_t *info;

			info = (PopUpMenuInfo_t *)GetWindowAdditionalData(hDlg);
			id = GetDlgCtrlID(GetFocusChild(hDlg));
			if(id == -1)
				break;
			pCtrl   = gui_Control(GetDlgItem(hDlg, id));

			if(wParam == CVR_KEY_OK) {
				change_status_from_pose(pCtrl, BST_NORMAL, 1, 1, info->context);
				return 0;
			} else if(wParam == CVR_KEY_LEFT) {
				HWND hNewFocus, hCurFocus;
				hCurFocus = GetFocusChild (hDlg);
				hNewFocus = GetNextDlgTabItem (hDlg, hCurFocus, 1);
				if (hNewFocus != hCurFocus) {
					SetNullFocus (hCurFocus);
					SetFocus (hNewFocus);
				}
			} else if(wParam == CVR_KEY_RIGHT) {
				HWND hNewFocus, hCurFocus;
				hCurFocus = GetFocusChild (hDlg);
				hNewFocus = GetNextDlgTabItem (hDlg, hCurFocus, 0);
				if (hNewFocus != hCurFocus) {
					SetNullFocus (hCurFocus);
					SetFocus (hNewFocus);
				}
			}
			else if(wParam == CVR_KEY_POWER)
			{
				ShowWindow(hDlg, SW_HIDE);
				ShowWindow(GetUsbWinHwnd(), SW_HIDE);
				SendMessage(GetMainWinHwnd(), MSG_CLOSE, 0, 0);	// 在usb场景，短按即可关机
			}
		}
		break;

		case MSG_DIALOG_CLOSE_PC:
		{
			//cvr_debug("###################################_1\n");
			if(!usb->bConnect2Pc)
			{
				break;
			}

			usb->bConnect2Pc = CVR_FALSE;
			cvr_debug("close connect To PC Dialog\n");


			EndDialog(hDlg, 0);

			unloadBitMap(&usb->usbModeBmp);
			unloadBitMap(&usb->pcCamBmp);

			if(usb->ctrldata != NULL)
			{
				free(usb->ctrldata);
				usb->ctrldata = NULL;
			}

			if(usb->info != NULL)
			{
				free(usb->info);
				usb->info = NULL;
			}

			if(usb->bUsbStorageOn)
			{
				usb->bUsbStorageOn = CVR_FALSE;
				usb_storage_adcard_off();
			}

			ChangeWindow(GetMainWinData(), WINDOW_USB_ID, WINDOW_RECORD_ID);
		}
		break;

		default:
		break;
	}

	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

int PopUpMenu(HWND hWnd, PopUpMenuInfo_t *info)
{
	int tmp;
	int retval;
	unsigned int dwStyle;
	unsigned int count;
	DLGTEMPLATE dlg;

	if(info->item_nrs > POPMENU_MAX_ITEMS) {
		cvr_debug("the max items is %d\n", POPMENU_MAX_ITEMS);
		return -1;
	}
	usb->ctrldata = (PCTRLDATA)malloc(info->item_nrs * sizeof(CTRLDATA));
	if(usb->ctrldata == NULL)
	{
		cvr_error("malloc ctrldata fail\n");
		return -1;
	}
	memset(usb->ctrldata, 0, info->item_nrs * sizeof(CTRLDATA));


	dwStyle = info->style & POPMENUS_MASK;
	for(count = 0; count < info->item_nrs; count++)
	{
		usb->ctrldata[count].class_name = CTRL_BUTTON;
		if(count != 0)
		{
 #ifndef USE_IPS_SCREEN
			 usb->ctrldata[count].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP;
 #else
			 usb->ctrldata[count].dwStyle = WS_VISIBLE | WS_TABSTOP;
 #endif
		}
		usb->ctrldata[count].id = info->id + count;
		usb->ctrldata[count].caption = info->name[count];
		if(dwStyle & POPMENUS_V)
		{
			if(dwStyle & POPMENUS_VCENTER)
			{
				usb->ctrldata[count].x = info->rect.w / 2 -info->item_width / 2;
				tmp = (count * info->item_height) + (count + 1) * VBORDER;
				tmp += (info->rect.h - (info->item_nrs * info->item_height + (info->item_nrs + 1) * VBORDER)) / 2;
				usb->ctrldata[count].y = tmp;
			}
			else
			{
				usb->ctrldata[count].x = info->rect.w / 2 - info->item_width/2;
				usb->ctrldata[count].y = (count * info->item_height) + (count + 1) * VBORDER;
			}
		}
		else
		{
			usb->ctrldata[count].x =	(count * info->item_width) + (count + 1) * HBORDER;
			usb->ctrldata[count].y = info->rect.h / 2 - info->item_height / 2;
		}
		usb->ctrldata[count].w = info->item_width;
		usb->ctrldata[count].h = info->item_height;
	}

#ifndef USE_IPS_SCREEN
    usb->ctrldata[0].dwStyle = WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP | WS_GROUP;
    usb->dlg.dwStyle = WS_BORDER;
#else
	usb->ctrldata[0].dwStyle = WS_VISIBLE | WS_TABSTOP | WS_GROUP;
    usb->dlg.dwStyle = NULL;
#endif

	usb->dlg.dwExStyle = WS_EX_NONE;
	usb->dlg.x = info->rect.x;
	usb->dlg.y = info->rect.y;
	usb->dlg.w = info->rect.w;
	usb->dlg.h = info->rect.h;
	usb->dlg.caption = "";
	usb->dlg.hIcon = 0;
	usb->dlg.hMenu = 0;
	usb->dlg.controlnr = info->item_nrs;
	usb->dlg.controls = usb->ctrldata;
	usb->dlg.dwAddData = 0;

	retval = DialogBoxIndirectParam(&usb->dlg, hWnd, DialogProc, (LPARAM)info);

	return retval;
}

static int closeConnectToPCDialog(HWND hWnd, void* context)
{
       BroadcastMessage(MSG_DIALOG_CLOSE_PC, 0, 0);
	   return 0;
}

static int connectToPCDialog(HWND hWnd, void* context)
{
	int retval  = 0;

	if(usb->bConnect2Pc)
	{
		return -1;
	}
	usb->bConnect2Pc = CVR_TRUE;
	cvr_debug("connect To PC Dialog\n");

	usb->info = (PopUpMenuInfo_t *)malloc(sizeof(PopUpMenuInfo_t));
	if(usb->info == NULL)
	{
		cvr_error("malloc usb info fail\n");
		return -1;
	}
	memset(usb->info, 0, sizeof(PopUpMenuInfo_t));

	usb->info->rect.x = usb->usbSize.x;
	usb->info->rect.y = usb->usbSize.y;
	usb->info->rect.w = usb->usbSize.w;
	usb->info->rect.h = usb->usbSize.h;

	usb->info->item_width  = getResIntValue(ID_CONNECT2PC,INTVAL_ITEMWIDTH);
	usb->info->item_height = getResIntValue(ID_CONNECT2PC,INTVAL_ITEMHEIGHT);

	usb->info->style = POPMENUS_V | POPMENUS_VCENTER;
	//usb->info->item_nrs = 3;
	usb->info->item_nrs = 2;	// remove pccam
	usb->info->id = ID_CONNECT2PC;
	usb->info->pLogFont = getLogFont();

	usb->info->name[0] = getLabel(LANG_LABEL_OPEN_USB_STORAGE_DEVICE);
	usb->info->name[1] = getLabel(LANG_LABEL_CHARGE_MODE);
	usb->info->name[2] = getLabel(LANG_LABEL_OPEN_PCCAM);

	usb->info->callback[0] = connectToPCCallback;
	usb->info->callback[1] = connectToPCCallback;
	usb->info->callback[2] = connectToPCCallback;

	//we don't have this
	usb->info->context = NULL;

	usb->info->end_key = CVR_KEY_MODE;
	usb->info->flag_end_key = 1;
	usb->info->push_key = CVR_KEY_OK;
	usb->info->table_key = CVR_KEY_RIGHT;

#if 1
	usb->info->bgc_widget      = getResColor(ID_CONNECT2PC,COLOR_BGC);
	usb->info->bgc_item_focus  = getResColor(ID_CONNECT2PC,COLOR_BGC_ITEMFOCUS);
	usb->info->bgc_item_normal = getResColor(ID_CONNECT2PC,COLOR_BGC_ITEMNORMAL);
	usb->info->mainc_3dbox     = getResColor(ID_CONNECT2PC,COLOR_MAIN3DBOX);
#endif

	retval = PopUpMenu(hWnd, usb->info);
	return retval;
}

s32 UsbkeyProc(UsbWinDataType *usb, s32 keyCode, s32 isLongPress)
{
	switch(keyCode)
	{
	case CVR_KEY_OK:

		break;

	case CVR_KEY_MODE:
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
}

int UsbWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	UsbWinDataType *usb = NULL;
	CvrRectType rect;
	gal_pixel color;

	usb = (UsbWinDataType*)GetWindowAdditionalData(hWnd);

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
			break;

		case MSG_WIN_RESUME:
			connectToPCDialog(hWnd, NULL);
			break;
		case MSG_WIN_SUSPEND:
			break;

		case MSG_CVR_KEY:
			return UsbkeyProc(usb, wParam, lParam);
			break;

		case MSG_TIMER:
			break;

		case MSG_CLOSE:
			break;

		case MSG_DESTROY:
			break;

		case MSG_USB_PLUGOUT:
			closeConnectToPCDialog(hWnd, NULL);
			//cvr_debug("###################################_3\n");
			break;

		default:
			break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

UsbWinDataType* GetUsbWinData(void)
{
	if(usb != NULL)
	{
		return usb;
	}

	return NULL;
}

HWND GetUsbWinHwnd(void)
{
	if(usb!=NULL && usb->usbHwnd!=0)
	{
		return usb->usbHwnd;
	}

	return 0;
}

HWND UsbWinInit(HWND hParent)
{
	HWND mHwnd;
	CvrRectType rect;

	usb = (UsbWinDataType*)malloc(sizeof(UsbWinDataType));
	if(NULL == usb)
	{
		cvr_error("malloc usb window data error\n");
		return -1;
	}
	memset((void *)usb, 0, sizeof(UsbWinDataType));

	getResRect(ID_CONNECT2PC,&rect);

	usb->usbSize.x = rect.x;
	usb->usbSize.y = rect.y;
	usb->usbSize.w = rect.w;
	usb->usbSize.h = rect.h;

	usb->bConnect2Pc = CVR_FALSE;

	// WS_VISIBLE
	mHwnd = CreateWindowEx(WINDOW_USB, "",
			WS_CHILD | SS_SIMPLE | WS_VISIBLE,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			WINDOW_USB_ID,
			rect.x, rect.y, rect.w, rect.h,
			hParent, (DWORD)usb);

	if(mHwnd == HWND_INVALID)
	{
		cvr_error("create status bar failed\n");
		return HWND_INVALID;
	}

	setHwnd(WINDOW_USB_ID, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);

	usb->ParentHwnd = hParent;
	usb->usbHwnd = mHwnd;

	return mHwnd;
}
