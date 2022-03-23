#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/listbox.h>

#include "CvrMenuList.h"
#include "CvrInclude.h"
#include "CvrResource.h"
#include "CvrMenuWindow.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CvrSubMenu.cpp"


#define IDC_SUB_TITLE			410
#define IDC_SUB_LIST_BOX		411

extern   HWND GetMenuWinHwnd(void);

static int SubMenuProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	subMenuData_t* subMenuData;
	HDC hdc;

	#ifdef USE_IPS_SCREEN
    static int eraseNum = 0;
	#endif

	cvr_debug("param:(0x%x, 0x%x, 0x%x, 0x%x)\n", hDlg, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_INITDIALOG:
		{
			int count;
			HWND hMenuList;
			MENULISTITEMINFO mlii;

			subMenuData = (subMenuData_t*)lParam;
			if(subMenuData == NULL)
			{
				cvr_error("invalid subMenuData\n");
				EndDialog(hDlg, -3);
			}

			SetWindowAdditionalData(hDlg, (DWORD)lParam);
			hMenuList = GetDlgItem(hDlg, IDC_SUB_LIST_BOX);

			SetWindowFont(hDlg, subMenuData->pLogFont);
			SetWindowBkColor(hDlg, subMenuData->menuListAttr.normalBgc);
			SetWindowElementAttr(GetDlgItem(hDlg, IDC_SUB_TITLE), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, subMenuData->menuListAttr.normalFgc));

			/* set the MENULIST item Height */
			SendMessage(hMenuList, LB_SETITEMHEIGHT, 0, 50);	/* set the item height */
			SendMessage(hMenuList, MSG_MLM_HILIGHTED_SPACE, 8, 8);
			SetFocusChild(hMenuList);

			/* set submenu title */
			SetWindowText(GetDlgItem(hDlg, IDC_SUB_TITLE), subMenuData->title);

			/* add item to sub menu list */
			count = subMenuData->contents.count;
			cvr_debug("count is %d\n", count);
			if(subMenuData->selectedIndex >= count || subMenuData->selectedIndex < 0)
			{
				cvr_debug("invalid menu current value\n");
				subMenuData->selectedIndex = 0;
			}

			for(int i = 0; i < count; i++)
			{
				memset(&mlii, 0, sizeof(mlii));
				mlii.string = (char*)subMenuData->contents.content[i];
				if(i == subMenuData->selectedIndex)
				{
					/* set the current item chioce image */
					mlii.flagsEX.imageFlag = 0;
					mlii.flagsEX.valueCount = 1;
					mlii.flagsEX.valueFlag[0] = VMFLAG_IMAGE;
					mlii.hValue[0] = (DWORD)&subMenuData->BmpChoice;
				}
				SendMessage(hMenuList, LB_ADDSTRING, 0, (LPARAM)&mlii);
			}

			/* set the current selected item */
			cvr_debug("current is %d\n", subMenuData->selectedIndex);
			SendMessage(hMenuList, LB_SETCURSEL, subMenuData->selectedIndex, 0);
			break;
		}

		/*
		case MSG_FONTCHANGED:
		{

			PLOGFONT pLogFont;

			pLogFont = GetWindowFont(hDlg);
			if(pLogFont == NULL)
				break;
			cvr_debug("type: %s, family: %s, charset: %s\n", pLogFont->type, pLogFont->family, pLogFont->charset);
			SetWindowFont(GetDlgItem(hDlg, IDC_SUB_TITLE), GetWindowFont(hDlg));
			SetWindowFont(GetDlgItem(hDlg, IDC_SUB_LIST_BOX), GetWindowFont(hDlg));
			break;
		}
		*/

		case MSG_FONTCHANGED:
		{
			HWND hChild;
			PLOGFONT pLogFont;

			//pLogFont = GetWindowFont(hDlg);
			pLogFont = getLogFont();
			if(!pLogFont)
			{
				cvr_error("invalid logFont\n");
				break;
			}

			cvr_debug("Font type:%s, charset:%s, family:%s\n", pLogFont->type, pLogFont->charset, pLogFont->family);
			hChild = GetNextChild(hDlg, 0);
			while( hChild && (hChild != HWND_INVALID))
			{
				SetWindowFont(hChild, pLogFont);
				hChild = GetNextChild(hDlg, hChild);
			}
			break;
		}

		case MSG_KEYUP:
		{
            HWND hMenuList;
            cvr_debug("******MSG_KEYUP********\n");

			#ifdef USE_IPS_SCREEN
            if(eraseNum <= 2)
                return 0;
			#endif

			int index;
			if(wParam == CVR_KEY_OK)
			{
				cvr_debug("press key ok\n");
				index = SendMessage(GetDlgItem(hDlg, IDC_SUB_LIST_BOX), LB_GETCURSEL, 0, 0);
                cvr_debug("-----index =%d-----\n", index);
				EndDialog(hDlg, index);
			}
			else if (wParam == CVR_KEY_POWER)
			{
#if 0
				int retval;
				native_sock_msg_t msg;

				memset(&msg, 0, sizeof(native_sock_msg_t));
				msg.id = hDlg;
				msg.messageid = MWM_CHANGE_SS;

				retval = send_sockmsg2server(SOCK_SERVER_NAME, &msg);
				if(retval == sizeof(native_sock_msg_t)) {
					db_msg("send message successd, retval is %d\n", retval);
				} else {
					db_msg("send message failed, retval is %d\n", retval);
				}
#endif
			}
			else if (wParam == CVR_KEY_MODE)
			{
				EndDialog(hDlg, -2);
			}
			else if (wParam == CVR_KEY_LEFT)
			{
				SendMessage(GetDlgItem(hDlg, IDC_SUB_LIST_BOX), MSG_MLM_NEW_KEY, wParam, lParam);
            }
			else if(wParam == CVR_KEY_RIGHT)
			{
				SendMessage(GetDlgItem(hDlg, IDC_SUB_LIST_BOX), MSG_MLM_NEW_KEY, wParam, lParam);
            }
			break;
		}

		case MSG_KEYDOWN:
		{
			#ifdef USE_IPS_SCREEN
            if(eraseNum <= 2)
                return 0;
			#endif

			#if 0
			int retval;
			native_sock_msg_t msg;

			memset(&msg, 0, sizeof(native_sock_msg_t));
			msg.id = hDlg;
			msg.messageid = MWM_WAKEUP_SS;

			retval = send_sockmsg2server(SOCK_SERVER_NAME, &msg);
			if(retval == sizeof(native_sock_msg_t)) {
				db_msg("send message successd, retval is %d\n", retval);
			} else {
				db_msg("send message failed, retval is %d\n", retval);
			}
			#endif
			break;
		}

		case MSG_PAINT:
		{
			RECT rect;
			hdc = BeginPaint(hDlg);

			subMenuData = (subMenuData_t*)GetWindowAdditionalData(hDlg);
			GetClientRect(GetDlgItem(hDlg, IDC_SUB_TITLE), &rect);
			SetPenColor(hdc, subMenuData->lincTitle);
			Line2(hdc, 0, RECTH(rect) + 2, RECTW(rect), RECTH(rect) + 2);

			EndPaint(hDlg, hdc);
			break;
		}

		case MSG_ERASEBKGND:
		{
			#ifdef USE_IPS_SCREEN
            //to avoid the extra erase message the first time
            if(eraseNum == 0 || eraseNum == 1)
            {
                eraseNum ++;
            }
            if(eraseNum ==2)
			{
                eraseNum = 3;
                return 0;
            }
			#endif
			break;
		}
		case MSG_DIALOG_CLOSE_SUB:
		{
			EndDialog(hDlg, -2);
			break;
		}
		case MSG_CLOSE:
		{
			subMenuData = (subMenuData_t*)GetWindowAdditionalData(hDlg);
			if(subMenuData->BmpChoice.bmBits)
			{
				UnloadBitmap(&subMenuData->BmpChoice);
			}
			EndDialog(hDlg, IDCANCEL);
			break;
		}

	case MSG_DESTROY:
        {
			#ifdef USE_IPS_SCREEN
            eraseNum = 0;
		    #endif
            subMenuData = (subMenuData_t*)GetWindowAdditionalData(hDlg);
            if(subMenuData->BmpChoice.bmBits)
			{
				UnloadBitmap(&subMenuData->BmpChoice);
            }
		    UpdateWindow(GetMenuWinHwnd(), 0);
			 break;
        }

		default:
			break;
	}

	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

int showSubMenu(HWND hWnd, subMenuData_t* subMenuData)
{
	CvrRectType rect;
	DLGTEMPLATE DlgSubMenu;
	CTRLDATA CtrlSubMenu[2];

	memset(&DlgSubMenu, 0, sizeof(DlgSubMenu));
	memset(CtrlSubMenu, 0, sizeof(CtrlSubMenu));

	rect = subMenuData->rect;
    cvr_debug("rect.x =%d rect.y =%d rect.w=%d rect.h=%d\n",subMenuData->rect.x,subMenuData->rect.y,subMenuData->rect.w,subMenuData->rect.h);

	CtrlSubMenu[0].class_name = CTRL_STATIC;
	CtrlSubMenu[0].dwStyle = WS_CHILD | WS_VISIBLE | SS_CENTER | SS_VCENTER;
	CtrlSubMenu[0].dwExStyle = WS_EX_TRANSPARENT;
	CtrlSubMenu[0].caption = "";
	CtrlSubMenu[0].x = 0;
	CtrlSubMenu[0].y = 0;
	CtrlSubMenu[0].w = rect.w;
	CtrlSubMenu[0].h = 40;
	CtrlSubMenu[0].id = IDC_SUB_TITLE;

	CtrlSubMenu[1].class_name = CTRL_CDRMenuList;
	CtrlSubMenu[1].dwStyle = WS_CHILD| WS_VISIBLE | WS_VSCROLL | LBS_USEBITMAP | LBS_HAVE_VALUE;
	CtrlSubMenu[1].dwExStyle = WS_EX_TRANSPARENT;
	CtrlSubMenu[1].caption = "";
	CtrlSubMenu[1].x = 0;
	CtrlSubMenu[1].y = 40;
	CtrlSubMenu[1].w = rect.w;
	CtrlSubMenu[1].h = rect.h - 40;
	CtrlSubMenu[1].id = IDC_SUB_LIST_BOX;
	CtrlSubMenu[1].dwAddData = (DWORD)&subMenuData->menuListAttr;

	DlgSubMenu.dwStyle = WS_VISIBLE | WS_CHILD;
	DlgSubMenu.dwExStyle = WS_EX_TRANSPARENT;
	DlgSubMenu.x = rect.x;
	DlgSubMenu.y = rect.y;
	DlgSubMenu.w = rect.w;
	DlgSubMenu.h = rect.h;
	DlgSubMenu.caption = "";
	DlgSubMenu.controlnr = 2;
	DlgSubMenu.controls = (PCTRLDATA)CtrlSubMenu;

	return DialogBoxIndirectParam(&DlgSubMenu, hWnd, SubMenuProc, (DWORD)subMenuData);
}
