#include <string.h>
#include <stdlib.h>

#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/listbox.h>

#include "CvrMenuList.h"
#include "CvrMainWindow.h"
#include "listbox_impl.h"
#include "ctrlclass.h"
#include "internals.h"
#include "CvrResource.h"

#define _USE_FIXSTR      1
#define ITEM_BOTTOM(x)  (x->itemTop + x->itemVisibles - 1)

#define LST_INTER_BMPTEXT       2
#define LFRDR_LB_CHECKBMP_MIN   6
static unsigned char HILIGHT_LEFT = 0;
static unsigned char HILIGHT_RIGHT = 0;

static gal_pixel gp_hilite_bgc;
static gal_pixel gp_hilite_fgc;
static gal_pixel gp_sign_bgc;
static gal_pixel gp_sign_fgc;
static gal_pixel gp_normal_bgc;
static gal_pixel gp_normal_fgc;
static gal_pixel gp_linec;
static gal_pixel gp_normal_stringc;
static gal_pixel gp_selected_stringc;
static gal_pixel gp_normal_valuec;
static gal_pixel gp_selected_valuec;

void Line2 (HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveTo(hdc, x1, y1);
	LineTo(hdc, x2, y2);
}

PCONTROL gui_Control (HWND hWnd)
{
	PCONTROL pCtrl;

	pCtrl = (PCONTROL) hWnd;

	if (pCtrl && pCtrl->WinType == TYPE_CONTROL)
		return pCtrl;

	return NULL;
}

static int CheckMarkHeight (int item_height)
{
	int checkmark_height = item_height - LFRDR_LB_CHECKBMP_MIN;

	if (0 < checkmark_height)
	{
		checkmark_height += checkmark_height >> 2;
	}

	return checkmark_height;
}

static BOOL lstInitListBoxData (HWND hwnd, PCONTROL pCtrl, LISTBOXDATA* pData, int len)
{
	int i;
	PLISTBOXITEM plbi;

	pData->itemHeight = pCtrl->pLogFont->size;
	pData->itemHilighted = -1;
	pData->selItem = pData->itemHilighted;
	pData->itemCount = 0;
	pData->str_cmp = strncmp;

	if (!(pData->buffStart = (LISTBOXITEM *)malloc(len * sizeof (LISTBOXITEM))))
	{
		return FALSE;
	}

	pData->buffLen = len;
	pData->buffEnd = pData->buffStart + len;
	pData->freeList = pData->buffStart;

	plbi = pData->freeList;
	for (i = 0; i < len - 1; i++)
	{
		plbi->next = plbi + 1;
		plbi ++;
	}
	plbi->next = NULL;

	if (GetWindowStyle (hwnd) & LBS_SBALWAYS)
	{
		pData->sbPolicy = SB_POLICY_ALWAYS;
	}
	else
	{
		pData->sbPolicy = SB_POLICY_AUTOMATIC;
	}

	return TRUE;
}

static void lstListBoxCleanUp (LISTBOXDATA* pData)
{
	PLISTBOXITEM plbi;
	PLISTBOXITEM next;

	plbi = pData->head;
	while (plbi)
	{
		#if _USE_FIXSTR
		FreeFixStr (plbi->key);
		#else
		free (plbi->key);
		#endif

		next = plbi->next;
		if (plbi < pData->buffStart || plbi > pData->buffEnd)
		{
			free (plbi);
		}

		plbi = next;
	}

	free (pData->buffStart);
}

static void lstResetListBoxContent (PLISTBOXDATA pData)
{
	int i;
	PLISTBOXITEM plbi, next;

	pData->itemLeft = 0;
	pData->itemWidth = 0;
	pData->itemMaxWidth = 0;

	pData->itemCount = 0;
	pData->itemTop = 0;
	pData->itemHilighted = -1;
	pData->selItem = pData->itemHilighted;

	plbi = pData->head;
	while (plbi)
	{
		#if _USE_FIXSTR
		FreeFixStr (plbi->key);
		#else
		free (plbi->key);
		#endif

		next = plbi->next;
		if (plbi < pData->buffStart || plbi > pData->buffEnd)
		{
			free (plbi);
		}

		plbi = next;
	}

	pData->head = NULL;
	pData->freeList = pData->buffStart;

	plbi = pData->freeList;
	for (i = 0; i < pData->buffLen - 1; i++)
	{
		plbi->next = plbi + 1;
		plbi ++;
	}
	plbi->next = NULL;

	return;
}

static PLISTBOXITEM lstAllocItem (PLISTBOXDATA pData)
{
	PLISTBOXITEM plbi;

	if (pData->freeList)
	{
		plbi = pData->freeList;
		pData->freeList = plbi->next;
	}
	else
	{
		plbi = (PLISTBOXITEM) malloc (sizeof (LISTBOXITEM));
	}

	return plbi;
}

static void lstFreeItem (PLISTBOXDATA pData, PLISTBOXITEM plbi)
{
	if (plbi < pData->buffStart || plbi > pData->buffEnd)
	{
		free (plbi);
	}
	else
	{
		plbi->next = pData->freeList;
		pData->freeList = plbi;
	}
}

static int lstGetItemWidth (HWND hwnd, PLISTBOXITEM plbi, int item_height)
{
	PCONTROL    pCtrl;
	DWORD       dwStyle;

	int         x = 0;

	pCtrl = gui_Control (hwnd);
	dwStyle = pCtrl->dwStyle;

	x = LST_INTER_BMPTEXT;

	if (dwStyle & LBS_CHECKBOX)
	{
		x += CheckMarkHeight (item_height) + LST_INTER_BMPTEXT;
	}

	if (dwStyle & LBS_USEICON && plbi->dwImage)
	{
		if (plbi->dwFlags &LBIF_USEBITMAP)
		{
			x += ((PBITMAP)(plbi->dwImage))->bmWidth;
		}
		else
		{
			int width;
			GetIconSize ((HICON) plbi->dwImage, &width, NULL);
			x += width;
		}
		x += LST_INTER_BMPTEXT;
	}

	if (plbi->key && plbi->key[0] != '\0')
	{
		SIZE size;
		HDC hdc;
		hdc = GetClientDC (hwnd);
		GetTabbedTextExtent (hdc, plbi->key, -1, &size);
		ReleaseDC (hdc);
		x += size.cx + LST_INTER_BMPTEXT;
	}

	return x;
}

static int lstGetMaxWidth(HWND hwnd, PLISTBOXDATA pData)
{
	PLISTBOXITEM plbi;
	int max = 0;
	int width;

	plbi = pData->head;

	while (plbi)
	{
		width = lstGetItemWidth (hwnd, plbi, pData->itemHeight);
		max = MAX(max, width);
		plbi = plbi->next;
	}

	return max;
}

static int lstAddNewItem (DWORD dwStyle, PLISTBOXDATA pData, PLISTBOXITEM newItem, int pos)
{
	PLISTBOXITEM plbi;
	PLISTBOXITEM insPosItem = NULL;
	int insPos = 0;

	newItem->next = NULL;
	if (!pData->head)
	{
		insPosItem = NULL;
	}
	else if (dwStyle & LBS_SORT)
	{
		plbi = pData->head;

		if (pData->str_cmp (newItem->key, plbi->key, (size_t)-1) < 0)
		{
			insPosItem = NULL;
			insPos = 0;
		}
		else
		{
			while (plbi->next)
			{
				if (pData->str_cmp (newItem->key, plbi->next->key, (size_t)-1) <= 0)
					break;

				plbi = plbi->next;
				insPos ++;
			}
			insPosItem = plbi;
		}
	}
	else
	{
		plbi = pData->head;

		if (pos < 0)
		{
			while (plbi->next)
			{
				plbi = plbi->next;
				insPos ++;
			}
			insPosItem = plbi;
		}
		else if (pos == 0)
		{
			insPosItem = NULL;
			insPos = 0;
		}
		else
		{
			int index = 1;

			while (plbi)
			{
				if (pos == index)
					break;
				plbi = plbi->next;
				index ++;
				insPos ++;
			}
			insPosItem = plbi;
		}
	}

	if (insPosItem)
	{
		plbi = insPosItem->next;
		insPosItem->next = newItem;
		newItem->next = plbi;

		insPos ++;
	}
	else
	{
		plbi = pData->head;
		pData->head = newItem;
		newItem->next = plbi;
	}

	pData->itemCount ++;

	return insPos;
}

static PLISTBOXITEM lstRemoveItem (PLISTBOXDATA pData, int* pos)
{
	int index = 0;
	PLISTBOXITEM plbi, prev;

	if (!pData->head)
	{
		return NULL;
	}

	if (*pos < 0 || *pos >= pData->itemCount)
	{
		return NULL;
	}

	if (*pos == 0)
	{
		plbi = pData->head;
		pData->head = plbi->next;
		return plbi;
	}
	else
	{
		index = 0;
		prev = pData->head;
		plbi = pData->head;
		while (plbi->next)
		{
			if (*pos == index)
				break;

			prev = plbi;
			plbi = plbi->next;
			index ++;
		}

		if (plbi == pData->head)
		{
			pData->head = pData->head->next;
			*pos = 0;
			return plbi;
		}
		else
		{
			prev->next = plbi->next;
			*pos = index;
			return plbi;
		}
	}

	return NULL;
}

static PLISTBOXITEM lstGetItem (PLISTBOXDATA pData, int pos)
{
	int i;
	PLISTBOXITEM plbi;

	plbi = pData->head;
	for (i=0; i < pos && plbi; i++)
	{
		plbi = plbi->next;
	}

	return plbi;
}

static int lstFindItem (PLISTBOXDATA pData, int start, char* key, BOOL bExact)
{
	PLISTBOXITEM plbi;
	unsigned int keylen = strlen (key);

	if (start >= pData->itemCount)
	{
		start = 0;
	}
	else if (start < 0)
	{
		start = 0;
	}

	plbi = lstGetItem (pData, start);
	while (plbi)
	{
		if (bExact && (keylen != strlen (plbi->key)))
		{
			plbi = plbi->next;
			start ++;
			continue;
		}

		if (pData->str_cmp (key, plbi->key, keylen) == 0)
			return start;

		plbi = plbi->next;
		start ++;
	}

	return LB_ERR;
}

static void lstGetItemsRect (PLISTBOXDATA pData, int start, int end, RECT* prc)
{
	if (start < 0)
		start = 0;

	prc->top = (start - pData->itemTop)*pData->itemHeight;

	if (end >= 0)
		prc->bottom = (end - pData->itemTop + 1)*pData->itemHeight;

}

static void lstInvalidateItem (HWND hwnd, PLISTBOXDATA pData, int pos)
{
	RECT rcInv;

	if (pos < pData->itemTop || pos > (pData->itemTop + pData->itemVisibles))
		return;

	GetClientRect (hwnd, &rcInv);
	rcInv.top = (pos - pData->itemTop)*pData->itemHeight;
	rcInv.bottom = rcInv.top + pData->itemHeight;

	InvalidateRect (hwnd, &rcInv, TRUE);
}

static BOOL lstInvalidateUnderMultiItem (HWND hwnd, PLISTBOXDATA pData, int start, int end)
{
	RECT rcInv;
	int pos;
	int max;

	if (start < 0) start = 0;
	if (end < 0) end = 0;

	pos = start > end ? end : start;
	max = start > end ? start : end;

	if (pos > (pData->itemTop + pData->itemVisibles))
		return FALSE;

	if (max < pData->itemTop)
		return FALSE;

	GetClientRect (hwnd, &rcInv);
	lstGetItemsRect (pData, pos, max, &rcInv);

	if (rcInv.top < rcInv.bottom) {
		InvalidateRect (hwnd, &rcInv, TRUE);
	}

	return TRUE;
}

static BOOL lstInvalidateUnderItem (HWND hwnd, PLISTBOXDATA pData, int pos)
{
	RECT rcInv;

	if (pos > (pData->itemTop + pData->itemVisibles))
		return FALSE;

	if (pos < pData->itemTop)
		return FALSE;

	GetClientRect (hwnd, &rcInv);

	lstGetItemsRect (pData, pos, -1, &rcInv);

	if (rcInv.top < rcInv.bottom)
		InvalidateRect (hwnd, &rcInv, TRUE);

	return TRUE;
}

static void lstCalcParams (const RECT* rcClient, PLISTBOXDATA pData)
{
	pData->itemVisibles = ((RECTHP (rcClient)) + pData->itemHeight / 5) / pData->itemHeight;
	pData->itemWidth = RECTWP(rcClient);
	cvr_debug("itemVisibles is %d\n", pData->itemVisibles);
}

static void lstOnDrawSListBoxItems (HWND hWnd, HDC hdc, PLISTBOXDATA pData, int width)
{
	PLISTBOXITEM plbi;
	PCONTROL pWin;
	RECT rect, tempRect;
	int i;
	int x = 0, y = 0;
	int value1, value2, value3;
	unsigned int valueNumber;
	gal_pixel old_color;

	pWin = (PCONTROL) hWnd;
	if (NULL == pWin)
	{
		return;
	}

	if (!pWin->we_rdr)
	{
		_MG_PRINTF ("CONTROL>ListBox: NULL LFRDR.\n");
		return;
	}

	plbi = lstGetItem (pData, pData->itemTop);
	SetBkMode(hdc, BM_TRANSPARENT);

	for (i = 0; plbi && i < (pData->itemVisibles + 1); i++)
	{
		rect.left  = 0;
		rect.top   = y;
		rect.right = width;
		rect.bottom = rect.top + pData->itemHeight;

		if (plbi->dwFlags & LBIF_SELECTED)
		{
			SetTextColor (hdc, gp_hilite_fgc);
			SetBrushColor (hdc, gp_hilite_bgc);
			/** render hilited item */
			tempRect.left	= rect.left + HILIGHT_LEFT;
			tempRect.top	= rect.top +2;
			tempRect.right	= rect.right - HILIGHT_RIGHT;
			tempRect.bottom = rect.bottom -1;
			old_color = SetBrushColor(hdc, gp_hilite_bgc);
			FillBox(hdc, tempRect.left, tempRect.top, RECTW(tempRect), RECTH(tempRect));
			SetBrushColor(hdc, old_color);
		}
		else
		{
			SetTextColor (hdc, gp_normal_fgc);
			SetBrushColor (hdc, gp_normal_bgc);

			/** render normal item */
			if (!(pWin->dwExStyle & WS_EX_TRANSPARENT))
				pWin->we_rdr->draw_normal_item(hWnd, hdc, &rect, PIXEL2DWORD(hdc,gp_normal_bgc));
		}

		/** render significant item */
		if ( (plbi->dwFlags & LBIF_SIGNIFICANT) && !(plbi->dwFlags & LBIF_DISABLE))
		{
			SetTextColor (hdc, gp_sign_fgc);
			SetBrushColor (hdc, gp_sign_bgc);

			pWin->we_rdr->draw_significant_item (hWnd, hdc, &rect,
					GetWindowElementAttr (hWnd, WE_BGC_SIGNIFICANT_ITEM));
		}

		if (pData->lst_font != INV_LOGFONT)
		{
			if (plbi->dwFlags & LBIF_BOLDSTYLE)
				pData->lst_font->style |= FS_WEIGHT_BOLD;
			else
				pData->lst_font->style &= ~FS_WEIGHT_BOLD;
		}

		x= 18;	/* set the x start from 18 pixel */

		if(plbi->imageFlag & LBIF_IMAGE && plbi->dwImage != 0)
		{
			value1 = ((PBITMAP)(plbi->dwImage))->bmWidth;	/* image width */
			value2 = ((PBITMAP)(plbi->dwImage))->bmHeight;	/* image height */
			if(value2 < pData->itemHeight)
			{
				value3 = (pData->itemHeight - value2) / 2 + y;
			}
			else
			{
				value3 =y;
			}
			FillBoxWithBitmap (hdc, x, value3, value1, value2, (PBITMAP)plbi->dwImage);
			x += ((PBITMAP)(plbi->dwImage))->bmWidth;
		}

		x += 16;

		if (plbi->dwFlags & LBIF_SELECTED) {
			SetTextColor(hdc, gp_selected_stringc);
		} else {
			SetTextColor(hdc, gp_normal_stringc);
		}
		TextOut (hdc, x, y + ((pData->itemHeight - GetCurFont(hdc)->size) >> 1), plbi->key);

		x = width - 34;

		for(valueNumber = plbi->valueCount; valueNumber > 0; valueNumber--)
		{
			if(plbi->valueFlag[valueNumber - 1] & LBIF_VALUE_IMAGE && (plbi->dwValueImage[valueNumber - 1] != 0) )
			{
				value1 = ((PBITMAP)(plbi->dwValueImage[valueNumber - 1]))->bmWidth;	/* image width */
				value2 = ((PBITMAP)(plbi->dwValueImage[valueNumber - 1]))->bmHeight;	/* image height */
				if(value2 < pData->itemHeight)
				{
					value3 = (pData->itemHeight - value2) / 2 + y;
				}
				else
				{
					value3 =y;
				}
				x -= value1;
				FillBoxWithBitmap (hdc, x, value3, value1, value2, (PBITMAP)(plbi->dwValueImage[valueNumber - 1]));
			}
			else if(plbi->valueFlag[valueNumber - 1] & LBIF_VALUE_STRING && plbi->dwValueString[valueNumber - 1] != 0)
			{
				if ( !(plbi->dwFlags & LBIF_SELECTED) )
				{
					SetTextColor(hdc, gp_normal_valuec);
				}
				else
				{
					SetTextColor(hdc, gp_selected_valuec);
				}
				x -= 10;
				tempRect.left = 0;
				tempRect.right = x;
				tempRect.top = y + ((pData->itemHeight - GetCurFont(hdc)->size) >> 1);
				tempRect.bottom = y + pData->itemHeight;

				DrawText(hdc, (char *)(plbi->dwValueString[valueNumber - 1]), -1, &tempRect, DT_RIGHT);
			}
		}

		y += pData->itemHeight;
		plbi = plbi->next;
	}

}

static int lstSelectItem (HWND hwnd, PLISTBOXDATA pData, int newSel)
{
	PLISTBOXITEM plbi, newItem;
	int index;

	newItem = lstGetItem (pData, newSel);

	if (newItem->dwFlags & LBIF_DISABLE)
		return newSel;

	#ifdef _DEBUG
	if (!newItem)
		_MG_PRINTF ("CONTROL>ListBox: return value of lstGetItem is NULL. \n");
	#endif

	if (GetWindowStyle (hwnd) & LBS_MULTIPLESEL)
	{
		newItem->dwFlags ^= LBIF_SELECTED;
		return newSel;
	}

	index = 0;
	plbi = pData->head;
	while (plbi)
	{
		if (plbi->dwFlags & LBIF_SELECTED)
		{
			if (index != newSel)
			{
				plbi->dwFlags &= ~LBIF_SELECTED;
				newItem->dwFlags |= LBIF_SELECTED;
				return index;
			}
			break;
		}

		plbi = plbi->next;
		index ++;
	}

	newItem->dwFlags |= LBIF_SELECTED;
	return pData->itemHilighted;
}

static int lstCancelSelected (HWND hwnd, PLISTBOXDATA pData)
{
	PLISTBOXITEM plbi;
	int index;

	index = 0;
	plbi = pData->head;
	while (plbi)
	{
		if (plbi->dwFlags & LBIF_SELECTED)
		{
			RECT rc;

			plbi->dwFlags &= ~LBIF_SELECTED;
			lstGetItemsRect (pData, index, index, &rc);
			InvalidateRect (hwnd, &rc, TRUE);
			return index;
		}

		plbi = plbi->next;
		index ++;
	}

	return -1;
}

static void lstDrawFocusRect (HWND hwnd, HDC hdc, PLISTBOXDATA pData)
{
	RECT rc;
	PCONTROL pWin;
	DWORD light_dword;
	pWin = (PCONTROL) hwnd;
	if (!pWin || !pWin->we_rdr) return;

	if (pData->itemHilighted < pData->itemTop
			|| pData->itemHilighted > (pData->itemTop + pData->itemVisibles))
		return;

	if (pData->dwFlags & LBF_FOCUS)
	{
		GetClientRect (hwnd, &rc);
		lstGetItemsRect (pData, pData->itemHilighted, pData->itemHilighted,&rc);
		InflateRect (&rc, -1, -1);
		light_dword = pWin->we_rdr->calc_3dbox_color(GetWindowElementAttr (hwnd, WE_BGC_WINDOW), LFRDR_3DBOX_COLOR_LIGHTEST);
		pWin->we_rdr->draw_focus_frame (hdc, &rc, light_dword);
	}
}

static void change_scrollbar (HWND hwnd, PLISTBOXDATA pData, BOOL bShow, int iSBar)
{
	if (pData->sbPolicy == SB_POLICY_ALWAYS)
	{
		if (iSBar != SB_VERT)
			EnableScrollBar (hwnd, iSBar, bShow);
	}
	else if (pData->sbPolicy == SB_POLICY_AUTOMATIC)
	{
		ShowScrollBar (hwnd, iSBar, bShow);
	}
}

static void lstSetVScrollInfo (HWND hwnd, PLISTBOXDATA pData, BOOL fRedraw)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	if (pData->sbPolicy == SB_POLICY_NEVER)
	{
		ShowScrollBar (hwnd, SB_VERT, FALSE);
		return;
	}

	if (pData->itemVisibles >= pData->itemCount)
	{
		SetScrollPos (hwnd, SB_VERT, 0);
		if (pData->sbPolicy == SB_POLICY_ALWAYS)
		{
			si.fMask = SIF_PAGE;
			si.nMax  = 1;
			si.nMin  = 0;
			si.nPage = 0;
			SetScrollInfo (hwnd, SB_VERT, &si, fRedraw);
		}
		change_scrollbar (hwnd, pData, FALSE, SB_VERT);
		return;
	}

	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMax = pData->itemCount*pData->itemHeight - 1;
	if (si.nMax < 0) si.nMax = 0; /*when Init. itemcount is 0*/
	si.nMin = 0;
	si.nPage = MIN (pData->itemVisibles, pData->itemCount)*pData->itemHeight;
	si.nPos = pData->itemTop * pData->itemHeight;

	SetScrollInfo (hwnd, SB_VERT, &si, fRedraw);
	EnableScrollBar (hwnd, SB_VERT, TRUE);
	change_scrollbar (hwnd, pData, TRUE, SB_VERT);
}

static void lstSetHScrollInfo (HWND hwnd, PLISTBOXDATA pData, BOOL fRedraw)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	if (pData->sbPolicy == SB_POLICY_NEVER)
	{
		ShowScrollBar (hwnd, SB_HORZ, FALSE);
		return;
	}

	if (pData->itemWidth >= pData->itemMaxWidth)
	{
		SetScrollPos (hwnd, SB_HORZ, 0);
		change_scrollbar (hwnd, pData, FALSE, SB_HORZ);
		return;
	}

	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMax = pData->itemMaxWidth;
	si.nMin = 0;
	si.nPage = pData->itemWidth;
	si.nPos = pData->itemLeft;

	SetScrollInfo (hwnd, SB_HORZ, &si, fRedraw);
	EnableScrollBar (hwnd, SB_HORZ, TRUE);
	change_scrollbar (hwnd, pData, TRUE, SB_HORZ);
}

static void lstSetScrollInfo (HWND hwnd, PLISTBOXDATA pData, BOOL fRedraw)
{
	lstSetVScrollInfo (hwnd, pData, fRedraw);
	lstSetHScrollInfo (hwnd, pData, fRedraw);
}

int MenuListCallback(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	PCONTROL    pCtrl;
	PLISTBOXDATA    pData;
	DWORD           dwStyle;
	int count;

	#if _USE_FIXSTR
	int             len;
	#endif

	pCtrl   = gui_Control(hwnd);
	dwStyle = pCtrl->dwStyle;
	//cvr_debug("menulist:(0x%x, 0x%x, 0x%x, 0x%x)\n", hwnd, message, wParam, (s32)lParam);
	switch(message)
	{
		case MSG_CREATE:
		{
			menuListAttr_t* menuListAttr;

			menuListAttr = (menuListAttr_t*)lParam;
			if(menuListAttr == NULL)
			{
				cvr_error("invalid menuListAttr\n");
				return -1;
			}

			// global attribute from MiniGUI.cfg
			gp_hilite_bgc	= GetWindowElementPixelEx(hwnd, HDC_SCREEN, WE_BGC_HIGHLIGHT_ITEM);
			gp_hilite_fgc   = GetWindowElementPixelEx(hwnd, HDC_SCREEN, WE_FGC_HIGHLIGHT_ITEM);
			gp_sign_bgc     = GetWindowElementPixelEx(hwnd, HDC_SCREEN, WE_BGC_SIGNIFICANT_ITEM);
			gp_sign_fgc     = GetWindowElementPixelEx(hwnd, HDC_SCREEN, WE_FGC_SIGNIFICANT_ITEM);

			gp_normal_bgc = menuListAttr->normalBgc;
			gp_normal_fgc = menuListAttr->normalFgc;
			SetWindowElementAttr(hwnd, WE_BGC_WINDOW, Pixel2DWORD(HDC_SCREEN, gp_normal_bgc));
			SetWindowElementAttr(hwnd, WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, gp_normal_fgc));
			SetWindowBkColor(hwnd, gp_normal_bgc);

			gp_linec = menuListAttr->linec;
			gp_normal_stringc = menuListAttr->normalStringc;
			gp_normal_valuec = menuListAttr->normalValuec;
			gp_selected_stringc = menuListAttr->selectedStringc;
			gp_selected_valuec = menuListAttr->selectedValuec;

			SetWindowElementAttr(hwnd, WE_MAINC_THREED_BODY, Pixel2DWORD(HDC_SCREEN, menuListAttr->scrollbarc));
			SetWindowElementAttr(hwnd, WE_METRICS_SCROLLBAR, 20);

			pData = (LISTBOXDATA*) calloc (1, sizeof(LISTBOXDATA));
			if (pData == NULL)
			{
				return -1;
			}
			memset(pData, 0, sizeof(LISTBOXDATA));

			pCtrl->dwAddData2 = (DWORD)pData;
			if (!lstInitListBoxData (hwnd, pCtrl, pData, DEF_LB_BUFFER_LEN))
			{
				free (pData);
				return -1;
			}
			pData->itemHeight = menuListAttr->itemHeight;

			lstSetScrollInfo (hwnd, pData, FALSE);

			break;
		}

		case MSG_PAINT:
		{
			HDC		hdc;
			RECT	rc;

			gp_normal_bgc = getResColor(ID_MENU_LIST, COLOR_BGC);

			hdc = BeginPaint (hwnd);
			GetClientRect (hwnd, &rc);
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			SelectFont(hdc, GetWindowFont(hwnd));

			lstOnDrawSListBoxItems (hwnd, hdc, pData, RECTW (rc));
			lstDrawFocusRect (hwnd, hdc, pData);

			SetPenColor(hdc, gp_linec);

			for( count = 1; count <= pData->itemCount; count++ )
			{
				Line2(hdc, 0, pData->itemHeight * count, RECTW(rc), pData->itemHeight * count);
			}

			EndPaint (hwnd, hdc);
			return 0;
		}
		case MSG_SIZECHANGED:
		{
			int oldvisible, oldwidth;
			const RECT *rc = (const RECT*)lParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			oldvisible = pData->itemVisibles;
			oldwidth = pData->itemWidth;

			if ( (rc->left >= rc->right) || (rc->top >= rc->bottom))
			{
				break;
			}

			lstCalcParams (rc, pData);
			if (oldvisible !=  pData->itemVisibles || oldwidth != pData->itemWidth )
			{
				lstSetScrollInfo (hwnd, pData, FALSE);
			}

			break;
		}
		case MSG_DESTROY:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if(pData->lst_font != INV_LOGFONT)
			{
				DestroyLogFont(pData->lst_font);

			}

			lstListBoxCleanUp (pData);
			free (pData);
			break;
		}

		case LB_SETSTRCMPFUNC:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (pData->itemCount == 0 && lParam)
			{
				pData->str_cmp = (STRCMP)lParam;
				return LB_OKAY;
			}
			return LB_ERR;
		}

		case LB_RESETCONTENT:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			lstResetListBoxContent (pData);
			lstSetScrollInfo (hwnd, pData, TRUE);
			InvalidateRect (hwnd, NULL, TRUE);
			return 0;
		}

		case LB_ADDSTRING:
		case LB_INSERTSTRING:
		{
			char* string = NULL;
			PMENULISTITEMINFO pmlii = NULL;
			PLISTBOXITEM newItem;
			int itemWidth;
			int pos;
			unsigned int count;

			if (dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEBITMAP || dwStyle & LBS_HAVE_VALUE)
			{
				pmlii = (PMENULISTITEMINFO)lParam;
				if (!pmlii)
					return LB_ERR;

				string = pmlii->string;
			}
			else
			{
				string = (char*)lParam;
				if (string == NULL || string [0] == '\0')
					return LB_ERR;
			}

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			newItem = lstAllocItem (pData);
			if (!newItem)
			{
				if (dwStyle & LBS_NOTIFY)
					NotifyParent (hwnd, pCtrl->id, LBN_ERRSPACE);
				return LB_ERRSPACE;
			}

			#if _USE_FIXSTR
			len = strlen (string);
			newItem->key = FixStrAlloc (len);
			if (!newItem->key)
			{
				lstFreeItem (pData, newItem);
				return LB_ERRSPACE;
			}
			if (len > 0)
			{
				strcpy (newItem->key, string);
			}
			#else
			newItem->key = strdup (string);
			if (!newItem->key)
			{
				lstFreeItem (pData, newItem);
				return LB_ERRSPACE;
			}
			#endif

			newItem->dwFlags = LBIF_NORMAL;
			newItem->imageFlag = LBIF_NORMAL;
			memset(&newItem->valueFlag, 0, sizeof(newItem->valueFlag));
			if (pmlii)
			{
				if (pmlii->cmFlag & CMFLAG_CHECKED)
				{
					newItem->dwFlags |= LBIF_CHECKED;
				}
				else if (pmlii->cmFlag & CMFLAG_PARTCHECKED)
				{
					newItem->dwFlags |= LBIF_PARTCHECKED;
				}

				if (pmlii->cmFlag & LBIS_SIGNIFICANT)
				{
					newItem->dwFlags |= LBIF_SIGNIFICANT;
				}

				if (dwStyle & LBS_USEBITMAP && (DWORD)pmlii->flagsEX.imageFlag & IMGFLAG_IMAGE)
				{
					newItem->dwImage = (DWORD)pmlii->hBmpImage;
					newItem->imageFlag |= LBIF_IMAGE;
				}
				else
				{
					newItem->dwImage = 0L;
				}

				/* have multi value */
				newItem->valueCount = pmlii->flagsEX.valueCount;
				for(count = 0; count < pmlii->flagsEX.valueCount; count++)
				{
					newItem->dwValueImage[count] = 0L;
					newItem->dwValueString[count] = 0L;
					if (dwStyle & LBS_USEBITMAP && dwStyle & LBS_HAVE_VALUE && (DWORD)pmlii->flagsEX.valueFlag[count] & VMFLAG_IMAGE)
					{
						newItem->dwValueImage[count] = (DWORD)pmlii->hValue[count];
						newItem->valueFlag[count] |= LBIF_VALUE_IMAGE;
					}
					else if(dwStyle & LBS_USEBITMAP && dwStyle & LBS_HAVE_VALUE && (DWORD)pmlii->flagsEX.valueFlag[count] & VMFLAG_STRING)
					{
						newItem->dwValueString[count] = (DWORD)pmlii->hValue[count];
						newItem->valueFlag[count] |= LBIF_VALUE_STRING;
					}
				}
			}
			newItem->dwAddData = 0L;

			if (message == LB_ADDSTRING)
			{
				pos = lstAddNewItem (dwStyle, pData, newItem, -1);
			}
			else
			{
				pos = lstAddNewItem (dwStyle, pData, newItem, (int)wParam);
			}

			itemWidth = lstGetItemWidth (hwnd, newItem, pData->itemHeight);
			if (pData->itemMaxWidth < itemWidth)
			{
				pData->itemMaxWidth = itemWidth;
			}

			lstInvalidateUnderItem (hwnd, pData, pos);
			lstSetScrollInfo (hwnd, pData, TRUE);
			return pos;
		}

		case LB_DELETESTRING:
		{
			PLISTBOXITEM removed;
			int delte_index;

			delte_index = (int)wParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			removed = lstRemoveItem (pData, &delte_index);
			if (removed)
			{
				int width;
				width = lstGetItemWidth (hwnd, removed, pData->itemHeight);
				if (width >= pData->itemMaxWidth)
				{
					pData->itemMaxWidth = lstGetMaxWidth(hwnd, pData);
				}

				#if _USE_FIXSTR
				FreeFixStr (removed->key);
				#else
				free (removed->key);
				#endif

				lstFreeItem (pData, removed);

				pData->itemCount --;

				if (pData->itemTop != 0 && pData->itemCount <= pData->itemVisibles)
				{
					pData->itemTop = 0;
					InvalidateRect (hwnd, NULL, TRUE);
				}
				else
				{
					lstInvalidateUnderItem (hwnd, pData, delte_index);
					if (delte_index <= pData->itemTop)
					{
						pData->itemTop --;
						if (pData->itemTop < 0)
						{
							pData->itemTop = 0;
						}
					}
				}

				if (pData->itemHilighted >= pData->itemCount)
				{
					pData->itemHilighted = pData->itemCount - 1;
					if (pData->itemHilighted < 0)
					{
						pData->itemHilighted = 0;
					}
				}

				if (pData->itemHilighted < pData->itemTop)
				{
					pData->itemHilighted = pData->itemTop;
				}

				if (pData->itemHilighted > ITEM_BOTTOM (pData))
				{
					pData->itemHilighted = ITEM_BOTTOM (pData);
				}

				pData->selItem = pData->itemHilighted;
				lstSetScrollInfo (hwnd, pData, TRUE);
				return LB_OKAY;
			}
			return LB_ERR;
		}

		case LB_FINDSTRING:
		{
			if( *(char*)lParam == '\0')
				return LB_ERR;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return lstFindItem(pData, (int)wParam, (char*)lParam, FALSE);
		}

		case LB_FINDSTRINGEXACT:
		{
			if( *(char*)lParam == '\0' )
				return LB_ERR;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return lstFindItem(pData, (int)wParam, (char*)lParam, TRUE);
		}

		case LB_SETTOPINDEX:
		{
			int newTop = (int) wParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (newTop <0)
			{
				newTop = 0;
			}
			else if (newTop > pData->itemCount - pData->itemVisibles)
			{
				newTop = pData->itemCount - pData->itemVisibles;
			}

			if (pData->itemTop != newTop)
			{
				pData->itemTop = newTop;

				if (pData->itemHilighted < pData->itemTop)
				{
					pData->itemHilighted = pData->itemTop;
				}
				if (pData->itemHilighted > ITEM_BOTTOM (pData))
				{
					pData->itemHilighted = ITEM_BOTTOM (pData);
				}

				pData->selItem = pData->itemHilighted;
				lstSetScrollInfo (hwnd, pData, TRUE);
				InvalidateRect (hwnd, NULL, TRUE);
			}

			return 0;
		}

		case LB_SETCURSEL:
		case LB_SETCARETINDEX:
		{
			int new_index = (int)wParam;
			int old, newTop;
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (new_index < 0 || new_index > pData->itemCount - 1)
			{
				if (dwStyle & LBS_MULTIPLESEL)
				{
					return LB_ERR;
				}
				else
				{
					return lstCancelSelected (hwnd, pData);
				}
			}

			old = pData->itemHilighted;
			if (new_index >= 0 && new_index != old)
			{
				if (pData->itemCount - new_index >= pData->itemVisibles)
				{
					newTop = new_index;
				}
				else
				{
					newTop = MAX (pData->itemCount - pData->itemVisibles, 0);
				}

				pData->itemTop = newTop;
				pData->itemHilighted = new_index;
				pData->selItem = pData->itemHilighted;
				lstSetScrollInfo (hwnd, pData, TRUE);
			}
			if (!(dwStyle & LBS_MULTIPLESEL))
			{
				PLISTBOXITEM newItem = lstGetItem (pData, new_index);
				if (newItem->dwFlags & LBIF_DISABLE)
				{
					lstCancelSelected(hwnd, pData);
					return old;
				}
				else
				{
					lstSelectItem (hwnd, pData, new_index);
				}
			}
			InvalidateRect (hwnd, NULL, TRUE);
			cvr_debug("old=%d, new_index=%d\n",old,new_index);
			SendMessage(GetParent(hwnd), MSG_MLM_NEW_SELECTED, old, new_index);
			return old;
		}

		case LB_GETCOUNT:
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return pData->itemCount;

		case LB_GETCURSEL:
		{
			PLISTBOXITEM plbi;
			int index = 0;
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (dwStyle & LBS_MULTIPLESEL)
				return pData->itemHilighted;

			plbi = pData->head;
			while (plbi)
			{
				if (plbi->dwFlags & LBIF_SELECTED)
					return index;

				index ++;
				plbi = plbi->next;
			}

			return LB_ERR;
		}

		case LB_GETSELCOUNT:
		{
			int nSel;
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			nSel = 0;
			plbi = pData->head;
			while (plbi)
			{
				if (plbi->dwFlags & LBIF_SELECTED)
				{
					nSel ++;
				}
				plbi = plbi->next;
			}

			return nSel;
		}

		case LB_GETTOPINDEX:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return pData->itemTop;
		}

		case LB_GETCARETINDEX:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return pData->itemHilighted;
		}

		case LB_GETTEXTLEN:
		{
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = lstGetItem (pData, (int)wParam);
			if (plbi)
			{
				return strlen (plbi->key);
			}

			return LB_ERR;
		}

		case LB_GETTEXT:
		{
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = lstGetItem (pData, (int)wParam);
			if (plbi)
			{
				strcpy ((char*)lParam, plbi->key);
				return LB_OKAY;
			}

			return LB_ERR;
		}

		case LB_SETTEXT:
		{
			PLISTBOXITEM plbi;
			char* newStr;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = lstGetItem (pData, (int)wParam);
			if (plbi)
			{
				len = strlen ((char*)lParam);
				newStr = FixStrAlloc (len);
				if (newStr)
				{
					FreeFixStr (plbi->key);
					plbi->key = newStr;

					if (len > 0)
						strcpy (plbi->key, (char*)lParam);

					lstInvalidateItem (hwnd, pData, (int)wParam);
				}
				else
				{
					return LB_ERR;
				}
			}

			return LB_ERR;
		}

		case LB_GETITEMDATA:
		{
			PLISTBOXITEM plbi;
			PMENULISTITEMINFO pmlii;
			unsigned int count;

			if (!(dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEICON))
			{
				return LB_ERR;
			}

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
				return LB_ERR;

			pmlii = (PMENULISTITEMINFO)lParam;
			if (!pmlii)
				return LB_ERR;

			pmlii->string = plbi->key;
			pmlii->cmFlag = LBIF_NORMAL;
			pmlii->flagsEX.imageFlag = LBIF_NORMAL;
			memset(&pmlii->flagsEX.valueFlag, 0, sizeof(pmlii->flagsEX.valueFlag));

			if (plbi->dwFlags & LBIF_CHECKED)
			{
				pmlii->cmFlag = CMFLAG_CHECKED;
			}
			else if (plbi->dwFlags & LBIF_PARTCHECKED)
			{
				pmlii->cmFlag = CMFLAG_PARTCHECKED;
			}
			else
			{
				pmlii->cmFlag = CMFLAG_BLANK;
			}

			if (plbi->dwFlags & LBIF_SIGNIFICANT)
			{
				pmlii->cmFlag |= LBIS_SIGNIFICANT;
			}

			if(plbi->imageFlag & LBIF_IMAGE)
			{
				pmlii->hBmpImage = (DWORD)plbi->dwImage;
				pmlii->flagsEX.imageFlag |=  IMGFLAG_IMAGE;
			}
			else
			{
				pmlii->hBmpImage = 0;
			}

			pmlii->flagsEX.valueCount = plbi->valueCount;
			for(count = 0; count < plbi->valueCount; count++)
			{
				if (plbi->valueFlag[count] & LBIF_VALUE_IMAGE)
				{
					pmlii->hValue[count] = (HICON)plbi->dwValueImage[count];
					pmlii->flagsEX.valueFlag[count] |=  VMFLAG_IMAGE;
				}
				else if (plbi->valueFlag[count] & LBIF_VALUE_STRING)
				{
					pmlii->hValue[count] = (HICON)plbi->dwValueString[count];
					pmlii->flagsEX.valueFlag[count] |=  VMFLAG_STRING;
				}
			}
			return LB_OKAY;
		}

		case LB_SETITEMDATA:
		{
			PLISTBOXITEM plbi;
			PMENULISTITEMINFO pmlii;
			unsigned int count;

			if (!(dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEICON))
			{
				return LB_ERR;
			}

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
			{
				return LB_ERR;
			}

			pmlii = (PMENULISTITEMINFO)lParam;
			if (!pmlii)
				return LB_ERR;

			plbi->dwFlags &= ~LBIF_CHECKMARKMASK;
			plbi->imageFlag = LBIF_NORMAL;
			memset(&plbi->valueFlag, 0, sizeof(plbi->valueFlag));

			if (pmlii->cmFlag & CMFLAG_CHECKED)
				plbi->dwFlags |= LBIF_CHECKED;
			if (pmlii->cmFlag & CMFLAG_PARTCHECKED)
				plbi->dwFlags |= LBIF_PARTCHECKED;
			if (pmlii->cmFlag & IMGFLAG_BITMAP)
				plbi->dwFlags |= LBIF_USEBITMAP;
			if (pmlii->cmFlag & LBIS_SIGNIFICANT)
				plbi->dwFlags |= LBIF_SIGNIFICANT;

			if (dwStyle & LBS_USEBITMAP && pmlii->flagsEX.imageFlag &IMGFLAG_IMAGE)
			{
				plbi->dwImage = (DWORD)pmlii->hBmpImage;
				plbi->imageFlag |= LBIF_IMAGE;
			}
			else
			{
				plbi->dwImage = 0;
			}

			plbi->valueCount = pmlii->flagsEX.valueCount;
			for(count = 0; count < pmlii->flagsEX.valueCount; count++ )
			{
				plbi->dwValueImage[count] = 0;
				plbi->dwValueString[count] = 0;
				if (dwStyle & LBS_USEBITMAP && pmlii->flagsEX.valueFlag[count] & VMFLAG_IMAGE)
				{
					plbi->dwValueImage[count] = (DWORD)pmlii->hValue[count];
					plbi->valueFlag[count] |= LBIF_VALUE_IMAGE;
				}
				else if (dwStyle & LBS_USEBITMAP && pmlii->flagsEX.valueFlag[count] &VMFLAG_STRING)
				{
					plbi->dwValueString[count] = (DWORD)pmlii->hValue[count];
					plbi->valueFlag[count] |= LBIF_VALUE_STRING;
				}
			}

			if (pmlii->string && pmlii->string[0] != '\0')
			{
				int oldlen = strlen (plbi->key);
				int newlen = strlen (pmlii->string);

				if (oldlen >= newlen)
				{
					memcpy (plbi->key, pmlii->string, newlen);
					plbi->key[newlen] = '\0';
				}
				else
				{
					#if _USE_FIXSTR
					FreeFixStr (plbi->key);
					plbi->key = FixStrAlloc (newlen + 1);
					pData = (PLISTBOXDATA)pCtrl->dwAddData2;
					if (!plbi->key)
					{
						lstFreeItem (pData, plbi);
						return LB_ERRSPACE;
					}

					strcpy (plbi->key, pmlii->string);
					plbi->key[newlen] = '\0';
					#else
					free (plbi->key);
					plbi->key = strdup (pmlii->string);
					if (!plbi->key)
					{
						lstFreeItem (pData, plbi);
						return LB_ERRSPACE;
					}
					#endif
				}
			}

			lstInvalidateItem (hwnd, pData, (int)wParam);
			return LB_OKAY;
		}

		case LB_GETITEMADDDATA:
		{
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
			{
				return LB_ERR;
			}

			return plbi->dwAddData;
		}

		case LB_SETITEMADDDATA:
		{
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
			{
				return LB_ERR;
			}

			plbi->dwAddData = (DWORD)lParam;

			return LB_OKAY;
		}

		case LB_GETCHECKMARK:
		{
			PLISTBOXITEM plbi;

			if (!(dwStyle & LBS_CHECKBOX))
				return LB_ERR;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
				return LB_ERR;

			if (plbi->dwFlags & LBIF_CHECKED)
				return CMFLAG_CHECKED;

			if (plbi->dwFlags & LBIF_PARTCHECKED)
				return CMFLAG_PARTCHECKED;

			return CMFLAG_BLANK;
		}

		case LB_SETCHECKMARK:
		{
			PLISTBOXITEM plbi;

			if (!(dwStyle & LBS_CHECKBOX))
				return LB_ERR;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (!(plbi = lstGetItem (pData, (int)wParam)))
				return LB_ERR;

			plbi->dwFlags &= ~LBIF_CHECKMARKMASK;
			switch (lParam)
			{
				case CMFLAG_CHECKED:
					plbi->dwFlags |= LBIF_CHECKED;
					break;
				case CMFLAG_PARTCHECKED:
					plbi->dwFlags |= LBIF_PARTCHECKED;
					break;
			}

			lstInvalidateItem (hwnd, pData, (int)wParam);
			return LB_OKAY;
		}

		case LB_GETSELITEMS:
		{
			int  nItem;
			int  nSel = 0;
			int  index = 0;
			int* pInt;
			PLISTBOXITEM plbi;

			nItem = (int)wParam;
			pInt  = (int*)lParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = pData->head;
			while (plbi)
			{
				if (plbi->dwFlags & LBIF_SELECTED)
				{
					if (pInt)
					{
						if (nSel < nItem)
						{
							*(pInt + nSel) = index;
						}
						else
						{
							return nItem;
						}
					}
					nSel ++;
				}

				plbi = plbi->next;
				index ++;
			}

			return nSel;
		}
		break;

		case LB_GETSEL:
		{
			PLISTBOXITEM plbi;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = lstGetItem (pData, (int)wParam);
			if (plbi)
			{
				return plbi->dwFlags & LBIF_SELECTED;
			}

			return LB_ERR;
		}

		case LB_SETSEL:
		{
			PLISTBOXITEM plbi;

			if (!(dwStyle & LBS_MULTIPLESEL))
				return LB_ERR;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			plbi = lstGetItem (pData, (int)lParam);
			if (plbi)
			{
				if (plbi->dwFlags & LBIF_DISABLE)
					return LB_ERR;

				if (wParam == 1)
				{
					plbi->dwFlags ^= LBIF_SELECTED;
				}
				else if (wParam == 0)
				{
					plbi->dwFlags &= ~LBIF_SELECTED;
				}
				else
				{
					plbi->dwFlags |= LBIF_SELECTED;
				}

				lstInvalidateItem (hwnd, pData, (int)lParam);
			}

			return LB_ERR;
		}

		case LB_GETITEMHEIGHT:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			return pData->itemHeight;
		}

		case LB_SETITEMHEIGHT:
		{
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (pData->itemHeight != LOWORD (lParam))
			{
				RECT rcClient;

				pData->itemHeight = LOWORD (lParam);
				if (pData->itemHeight < pCtrl->pLogFont->size)
					pData->itemHeight = pCtrl->pLogFont->size;

				GetClientRect (hwnd, &rcClient);
				lstCalcParams (&rcClient, pData);

				lstSetScrollInfo (hwnd, pData, TRUE);
				InvalidateRect (hwnd, NULL, TRUE);
			}
			return  pData->itemHeight;
		}

		case MSG_SETFOCUS:
		{
			RECT rc;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (pData->dwFlags & LBF_FOCUS)
				break;

			pData->dwFlags |= LBF_FOCUS;

			if (pData->itemHilighted >= 0)
			{
				GetClientRect (hwnd, &rc);
				lstGetItemsRect (pData, pData->itemHilighted, pData->itemHilighted, &rc);
				InvalidateRect (hwnd, &rc, TRUE);
			}

			if (dwStyle & LBS_NOTIFY)
				NotifyParent (hwnd, pCtrl->id, LBN_SETFOCUS);
			break;
		}

		case MSG_KILLFOCUS:
		{
			RECT rc;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (pData->itemHilighted >= 0)
			{
				GetClientRect (hwnd, &rc);
				lstGetItemsRect (pData, pData->itemHilighted, pData->itemHilighted, &rc);
				InvalidateRect (hwnd, &rc, TRUE);
			}

			pData->dwFlags &= ~LBF_FOCUS;

			if (dwStyle & LBS_NOTIFY)
			{
				NotifyParent (hwnd, pCtrl->id, LBN_SELCANCEL);
				NotifyParent (hwnd, pCtrl->id, LBN_KILLFOCUS);
			}
			break;
		}

		case MSG_GETDLGCODE:
		return DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTENTER;

		case MSG_FONTCHANGED:
		{
			RECT rcClient;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (pData->itemHeight < pCtrl->pLogFont->size)
				pData->itemHeight = pCtrl->pLogFont->size;

			GetClientRect (hwnd, &rcClient);
			lstCalcParams (&rcClient, pData);

			lstSetScrollInfo (hwnd, pData, TRUE);
			InvalidateRect (hwnd, NULL, TRUE);
			return 0;
		}
		break;

		case MSG_GETTEXTLENGTH:
		case MSG_GETTEXT:
		case MSG_SETTEXT:
			return -1;

		case MSG_LBUTTONDBLCLK:
		{
			if (dwStyle & LBS_NOTIFY)
			{
				NotifyParent (hwnd, pCtrl->id, LBN_DBLCLK);
			}
			break;
		}

		case MSG_MOUSEMOVE:
		case MSG_LBUTTONDOWN:
		{
			if ((dwStyle & LBS_MULTIPLESEL) || !(dwStyle & LBS_MOUSEFOLLOW))
				break;
		}

		case MSG_LBUTTONUP:
		{
			int oldSel, mouseX, mouseY, hit;
			RECT rcInv;
			BOOL click_mark = FALSE;
			PLISTBOXITEM hitItem;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (pData->itemCount == 0)
				break;

			mouseX = LOSWORD (lParam);
			mouseY = HISWORD (lParam);
			hit = mouseY / pData->itemHeight;
			hit += pData->itemTop;

			if (hit >= pData->itemCount)
				break;

			hitItem = lstGetItem (pData, hit);
			if (hitItem->dwFlags & LBIF_DISABLE)
				break;

			oldSel = lstSelectItem (hwnd, pData, hit);

			if (dwStyle & LBS_CHECKBOX)
			{
				if (mouseX > 0 && mouseX < CheckMarkHeight (pData->itemHeight))
				{
					click_mark = TRUE;

					if (message == MSG_LBUTTONUP && dwStyle & LBS_NOTIFY)
						NotifyParent (hwnd, pCtrl->id, LBN_CLICKCHECKMARK);

					if (dwStyle & LBS_AUTOCHECK)
					{
						if (message == MSG_LBUTTONUP)
						{
							PLISTBOXITEM plbi;

							plbi = lstGetItem (pData, hit);

							switch (plbi->dwFlags & LBIF_CHECKMARKMASK)
							{
								case LBIF_CHECKED:
									plbi->dwFlags &= ~LBIF_CHECKMARKMASK;
									if (message == MSG_LBUTTONUP && dwStyle & LBS_NOTIFY)
										NotifyParent (hwnd, pCtrl->id, LBN_SELCANCEL);
									break;
								default:
									plbi->dwFlags &= ~LBIF_CHECKMARKMASK;
									plbi->dwFlags |= LBIF_CHECKED;
									break;
							}
						}

						lstInvalidateItem (hwnd, pData, hit);
					}
				}
			}

			GetClientRect (hwnd, &rcInv);
			if (oldSel >= 0)
			{
				if (oldSel >= pData->itemTop && (oldSel <= pData->itemTop + pData->itemVisibles))
				{
					lstGetItemsRect (pData, oldSel, oldSel, &rcInv);
					InvalidateRect (hwnd, &rcInv, TRUE);
				}
			}

			lstGetItemsRect (pData, pData->itemHilighted, pData->itemHilighted, &rcInv);
			InvalidateRect (hwnd, &rcInv, TRUE);
			pData->itemHilighted = hit;

			lstGetItemsRect (pData, hit, hit, &rcInv);
			InvalidateRect (hwnd, &rcInv, TRUE);

			lstSetScrollInfo (hwnd, pData, TRUE);

			if ( message == MSG_LBUTTONUP && (dwStyle & LBS_NOTIFY) )
			{
				if (dwStyle & LBS_MOUSEFOLLOW)
				{
					oldSel = pData->selItem;
					pData->selItem = hit;
				}

				if (oldSel != hit)
					NotifyParent (hwnd, pCtrl->id, LBN_SELCHANGE);
				if (!click_mark)
					NotifyParent (hwnd, pCtrl->id, LBN_CLICKED);
			}
			SendMessage(GetParent(hwnd), MSG_MLM_NEW_SELECTED, oldSel, hit);
		}
		return 0;

		case LB_GETITEMRECT:
		{
			RECT rcClient, *rcItem;
			int index;

			rcItem = (RECT*)lParam;
			index = (int)wParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (pData->itemCount == 0 || index > pData->itemCount)
				return LB_ERR;

			GetClientRect ( hwnd, &rcClient );
			rcItem->right = rcClient.right;
			rcItem->left = rcClient.left;
			rcItem->top = (index-pData->itemTop)*pData->itemHeight;
			rcItem->bottom = (index-pData->itemTop+1)*(pData->itemHeight);
			return LB_OKAY;
		}

	case MSG_MLM_NEW_KEY:
		{
			int oldSel = -1, newSel, newTop;
			RECT rcInv;
			PLISTBOXITEM pnewItem;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			if (pData->itemCount == 0)
				break;

			newTop = pData->itemTop;
			newSel = pData->itemHilighted;

			switch (LOWORD (wParam))
			{
				case CVR_KEY_RIGHT:
	                cvr_debug("******CVR_KEY_RIGHT********\n");
					newSel ++;
					if (newSel >= pData->itemCount)
					{
						newSel = 0;
						newTop = 0;
						break;
					}
					if (newSel > ITEM_BOTTOM (pData))
						newTop ++;
					break;
				case CVR_KEY_LEFT:
	                cvr_debug("******CVR_KEY_LEFT********\n");
					newSel --;
					if (newSel < 0)
					{
						newSel = pData->itemCount - 1;
						if(pData->itemCount > pData->itemVisibles)
						{
							newTop = pData->itemCount - pData->itemVisibles;
						}
						else
						{
							newTop = 0;
						}
						break;
					}
					if (newSel < pData->itemTop)
						newTop --;
					break;
				default:
					return 0;
			}
			GetClientRect (hwnd, &rcInv);
			cvr_debug("pData->itemHilighted = %d \n",pData->itemHilighted);
			cvr_debug("newSel = %d \n",newSel);
			cvr_debug("pData->itemTop = %d \n",pData->itemTop);
			cvr_debug("newTop = %d \n",newTop);
			if (pData->itemHilighted != newSel)
			{
				if (pData->itemTop != newTop)
				{
					pData->itemTop = newTop;
					pData->itemHilighted = newSel;
					pData->selItem = pData->itemHilighted;
					if (!(dwStyle & LBS_MULTIPLESEL))
					{
						pnewItem = lstGetItem(pData, newSel);
						if(pnewItem->dwFlags & LBIF_DISABLE)
						{
							lstCancelSelected(hwnd, pData);
						}
						else
						{
							oldSel = lstSelectItem (hwnd, pData, newSel);
							if ((dwStyle & LBS_NOTIFY) && (oldSel != newSel))
								NotifyParent (hwnd, pCtrl->id, LBN_SELCHANGE);
						}
					}

					InvalidateRect (hwnd, NULL, TRUE);
				}
				else
				{
					if (!(dwStyle & LBS_MULTIPLESEL))
					{
						pnewItem = lstGetItem(pData, newSel);
						if(pnewItem->dwFlags&LBIF_DISABLE)
						{
							cvr_debug("simon lstCancelSelected \n");
							lstCancelSelected(hwnd, pData);
							InvalidateRect(hwnd, NULL, TRUE);
						}
						else
						{
							oldSel = lstSelectItem (hwnd, pData, newSel);
							cvr_debug("simon NotifyParent oldSel=%d \n",oldSel);
							if ((dwStyle & LBS_NOTIFY) && (oldSel != newSel))
								NotifyParent (hwnd, pCtrl->id, LBN_SELCHANGE);
							if (oldSel >= 0) {
								if (oldSel >= pData->itemTop && oldSel <= (ITEM_BOTTOM (pData) + 1)) {
									lstGetItemsRect (pData, oldSel, oldSel, &rcInv);
									cvr_debug("simon NotifyParent InvalidateRect \n");
									InvalidateRect (hwnd, &rcInv, TRUE);
								}
							}
						}

						if (newSel < newTop)
						{
							pData->itemHilighted = newSel;
							pData->selItem = pData->itemHilighted;
							break;
						}

						pData->itemHilighted = newSel;
						pData->selItem = pData->itemHilighted;
						lstGetItemsRect (pData, newSel, newSel, &rcInv);
						InvalidateRect (hwnd, &rcInv, TRUE);
					}
					else
					{
						pnewItem = lstGetItem(pData, newSel);
						if(pnewItem->dwFlags&LBIF_DISABLE)
							break;
						lstGetItemsRect (pData, pData->itemHilighted, pData->itemHilighted, &rcInv);
						InvalidateRect (hwnd, &rcInv, TRUE);
						pData->itemHilighted = newSel;
						lstGetItemsRect (pData, newSel, newSel, &rcInv);
						InvalidateRect (hwnd, &rcInv, TRUE);
					}
				}
				lstSetScrollInfo (hwnd, pData, TRUE);
			}
			//SendMessage(GetParent(hwnd), MSG_MLM_NEW_SELECTED, oldSel, newSel);
		}
		break;

		case MSG_VSCROLL:
		{
			int newTop;
			int scrollHeight = 0;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			newTop = pData->itemTop;		/* get the top item's index */
			switch(wParam)
			{
				case SB_LINEDOWN:
					if (ITEM_BOTTOM (pData) < (pData->itemCount - 1)) {
						newTop ++;
						scrollHeight = -pData->itemHeight;
					}
					break;

				case SB_LINEUP:
					if (pData->itemTop > 0)
					{
						newTop --;
						scrollHeight = pData->itemHeight;
					}
					break;

				case SB_PAGEDOWN:
					if ((pData->itemTop + (pData->itemVisibles << 1)) <= pData->itemCount)
					{
						newTop += pData->itemVisibles;
					}
					else
					{
						newTop = pData->itemCount - pData->itemVisibles;
					}

					if (newTop < 0)
						return 0;

					scrollHeight = -(newTop - pData->itemTop)*pData->itemHeight;
					break;

				case SB_PAGEUP:
					if (pData->itemTop >= pData->itemVisibles)
					{
						newTop -= pData->itemVisibles;
					}
					else
					{
						newTop = 0;
					}

					scrollHeight = (pData->itemTop - newTop)*pData->itemHeight;
					break;

				case SB_THUMBTRACK:
					newTop = MIN ((int)lParam/pData->itemHeight,
							(pData->itemCount - pData->itemVisibles));
					scrollHeight = (pData->itemTop - newTop)*pData->itemHeight;
					break;

				default:
					break;
			}

			if (scrollHeight)
			{
				pData->itemTop = newTop;
				lstSetScrollInfo (hwnd, pData, TRUE);
				InvalidateRect (hwnd, NULL, TRUE);
				return 0;
			}
			break;
		}

		case MSG_HSCROLL:
		{
			int newLeft;
			int scrollWidth = 0;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			newLeft = pData->itemLeft;
			switch (wParam)
			{
				case SB_LINERIGHT:
					if (pData->itemMaxWidth > (pData->itemWidth + newLeft)) {
						newLeft++;
						scrollWidth = 1;
					}
					break;

				case SB_LINELEFT:
					if (newLeft > 0)
					{
						newLeft--;
						scrollWidth = 1;
					}
					break;

				case SB_PAGELEFT:
					if (newLeft >= pData->itemWidth)
					{
						newLeft -= pData->itemWidth;
						scrollWidth = 1;
					}
					else if (newLeft > 0)
					{
						newLeft = 0;
						scrollWidth = 1;
					}
					break;

				case SB_PAGERIGHT:
					if (newLeft + (pData->itemWidth << 1) <= pData->itemMaxWidth)
					{
						newLeft += pData->itemWidth;
						scrollWidth = 1;
					}
					else
					{
						newLeft = pData->itemMaxWidth - pData->itemWidth;
						scrollWidth = 1;
					}
					break;

				case SB_THUMBTRACK:
					newLeft = MIN((int)lParam, (pData->itemMaxWidth - pData->itemWidth));
					scrollWidth = (pData->itemLeft - newLeft);
					break;

				default:
					break;
			}

			if (scrollWidth)
			{
				pData->itemLeft = newLeft;
				lstSetScrollInfo (hwnd, pData, TRUE);
				InvalidateRect (hwnd, NULL, TRUE);
				return 0;
			}
			break;
		}

		case LB_SETITEMDISABLE:
		{
			PLISTBOXITEM item;
			int index = (int)wParam;
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (pData->itemCount <= 0 || index < 0 || index > pData->itemCount)
				return LB_ERR;

			item = lstGetItem (pData, index);
			if (lParam)
				item->dwFlags |= LBIF_DISABLE;
			else
				item->dwFlags &=~LBIF_DISABLE;

			return lstInvalidateUnderItem (hwnd, pData, index);
		}

		case LB_GETITEMDISABLE:
		{
			PLISTBOXITEM item;
			int index = (int)wParam;
			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if (pData->itemCount <= 0 || index < 0 || index > pData->itemCount)
				return LB_ERR;
			item = lstGetItem (pData, index);
			return item->dwFlags&LBIF_DISABLE;
			break;
		}

		case LB_SETITEMBOLD:
		{
			PLISTBOXITEM item;
			int index = (int) wParam;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;

			if(pData->lst_font == INV_LOGFONT)
				return LB_ERR;

			if (pData->itemCount <= 0 || index < 0 || index > pData->itemCount)
				return LB_ERR;

			item = lstGetItem (pData, index);

			if (lParam)
				item->dwFlags |= LBIF_BOLDSTYLE;
			else
				item->dwFlags &=~LBIF_BOLDSTYLE;

			return lstInvalidateUnderItem (hwnd, pData, index);
			break;
		}

		case LB_MULTIADDITEM:
		{
			char* string = NULL;
			PMENULISTITEMINFO pmlii = NULL;		/* menu list item info */
			char** stringarray = NULL;

			PLISTBOXITEM newItem;
			int itemWidth;
			int start, pos = -1;
			int i;
			unsigned int count;

			pData = (PLISTBOXDATA)pCtrl->dwAddData2;
			start = pData->itemCount;

			if (dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEBITMAP || dwStyle & LBS_HAVE_VALUE)
			{
				pmlii = (PMENULISTITEMINFO)lParam;
				if (!pmlii)
					return LB_ERR;
			}
			else
			{
				stringarray = (char* *)lParam;
				if (stringarray == NULL)
					return LB_ERR;
			}

			for (i = 0; i < (int)wParam; i++)
			{
				if (dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEBITMAP || dwStyle & LBS_HAVE_VALUE)
					string = pmlii[i].string;
				else
					string = stringarray[i];

				if(string == NULL || string[0] == '\0')
					break;

				newItem = lstAllocItem (pData);
				if (!newItem)
				{
					if (dwStyle & LBS_NOTIFY)
						NotifyParent (hwnd, pCtrl->id, LBN_ERRSPACE);
					return LB_ERRSPACE;
				}

				#if _USE_FIXSTR
				len = strlen (string);
				newItem->key = FixStrAlloc (len);
				if (!newItem->key)
				{
					lstFreeItem (pData, newItem);
					return LB_ERRSPACE;
				}
				if (len > 0)
				{
					strcpy (newItem->key, string);
				}
				#else
				newItem->key = strdup (string);
				if (!newItem->key)
				{
					lstFreeItem (pData, newItem);
					return LB_ERRSPACE;
				}
				#endif

				newItem->dwFlags = LBIF_NORMAL;
				newItem->imageFlag = LBIF_NORMAL;
				memset(&newItem->valueFlag, 0, sizeof(newItem->valueFlag));

				/*********************** use the menu list item info **********************/
				if (dwStyle & LBS_CHECKBOX || dwStyle & LBS_USEBITMAP || dwStyle & LBS_HAVE_VALUE)
				{
					/************** for check mask flags cmFlag *******************/
					if (pmlii[i].cmFlag & CMFLAG_CHECKED)
						newItem->dwFlags |= LBIF_CHECKED;
					else if (pmlii[i].cmFlag & CMFLAG_PARTCHECKED)
						newItem->dwFlags |= LBIF_PARTCHECKED;
					if (pmlii[i].cmFlag & LBIS_SIGNIFICANT)
						newItem->dwFlags |= LBIF_SIGNIFICANT;
					if (pmlii[i].cmFlag & CMFLAG_WITHCHECKBOX)
						newItem->dwFlags |= LBIF_WITHCHECKBOX;

					/*add and changed by caodao to  support display text*/
					/************ for item image *****************/
					if (dwStyle & LBS_USEBITMAP &&	pmlii[i].flagsEX.imageFlag & IMGFLAG_IMAGE)
					{
						newItem->dwImage = (DWORD)pmlii[i].hBmpImage;
						newItem->imageFlag |= LBIF_IMAGE;
                         //cvr_debug("newItem->dwImage =%x\n",newItem->dwImage);
					}
					else
					{
						newItem->dwImage = 0L;		/* display the item's image */
					}

					/****************** for item value *************************/
					newItem->valueCount = pmlii[i].flagsEX.valueCount;
					for(count = 0; count < pmlii[i].flagsEX.valueCount; count++)
					{
						newItem->dwValueImage[count] = 0L;
						newItem->dwValueString[count] = 0L;
						if(dwStyle & LBS_USEBITMAP && dwStyle & LBS_HAVE_VALUE && pmlii[i].flagsEX.valueFlag[count] & VMFLAG_IMAGE )
						{
							newItem->dwValueImage[count] = pmlii[i].hValue[count];
							newItem->valueFlag[count] |= LBIF_VALUE_IMAGE;
						}
						else if(dwStyle & LBS_USEBITMAP && dwStyle & LBS_HAVE_VALUE && pmlii[i].flagsEX.valueFlag[count] & VMFLAG_STRING)
						{
							newItem->dwValueString[count] = pmlii[i].hValue[count];
							newItem->valueFlag[count] |= LBIF_VALUE_STRING;
						}
					}
				}

				newItem->dwAddData = 0L;
				pos = lstAddNewItem (dwStyle, pData, newItem, -1);

				itemWidth = lstGetItemWidth (hwnd, newItem, pData->itemHeight);
				if (pData->itemMaxWidth < itemWidth)
					pData->itemMaxWidth = itemWidth;
			}

			lstInvalidateUnderMultiItem (hwnd, pData, start, pos);
			lstSetScrollInfo (hwnd, pData, TRUE);
			return pos;
		}
		break;

		case MSG_MLM_HILIGHTED_SPACE:
		{
			HILIGHT_LEFT = wParam;
			HILIGHT_RIGHT = lParam;
			break;
		}

		default:
			break;
	}

	return DefaultControlProc (hwnd, message, wParam, lParam);
}
