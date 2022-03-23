#include "ProgressBar.h"
#include "CvrResource.h"
#include <string.h>
#include "ctrlclass.h"

#define IDC_LABEL_LEFT	601
#define IDC_LABEL_RIGHT	602
#define IDC_PROGRESSBAR	603

typedef  struct tagPROGRESSDATA
{
	int nMin;
	int nMax;
	int nPos;
	int nStepInc;
}PROGRESSDATA;
typedef PROGRESSDATA* PPROGRESSDATA;

static WNDPROC oldProc;

static void my_draw_progress (HWND hWnd, HDC hdc, int nMax, int nMin, int nPos)
{
	RECT    rcClient;
	int     x, y, w, h;
	ldiv_t   ndiv_progress;
	unsigned int     nAllPart;
	unsigned int     nNowPart;
	int     whOne, nRem;
	int     ix;
	unsigned int     i;
	int     step;
	int pbar_border = 0;
	gal_pixel old_color;

	if (nMax == nMin)
		return;

	if ((nMax - nMin) > 5)
		step = 1;
	else
		step = 1;

	GetClientRect (hWnd, &rcClient);

	x = rcClient.left + pbar_border;
	y = rcClient.top + pbar_border;
	w = RECTW (rcClient) - (pbar_border << 1);
	h = RECTH (rcClient) - (pbar_border << 1);


	SetWindowBkColor(hWnd, RGBA2Pixel(hdc, 0xaf, 0xaf, 0xaf, 0xff));
	if (hWnd != HWND_NULL)
		old_color = SetBrushColor (hdc, GetWindowBkColor (hWnd));
	else
		old_color = SetBrushColor (hdc, GetWindowElementPixel (HWND_DESKTOP, WE_BGC_DESKTOP));

	//draw the erase background
	FillBox (hdc, rcClient.left + 1, rcClient.top, RECTW (rcClient), RECTH (rcClient) - 2);
	SetPenColor(hdc, old_color);

	ndiv_progress = ldiv (nMax - nMin, step);
	nAllPart = ndiv_progress.quot;

	ndiv_progress = ldiv (nPos - nMin, step);
	nNowPart = ndiv_progress.quot;

	ndiv_progress = ldiv (w, nAllPart);		/* calculate the with for each step*/

	whOne = ndiv_progress.quot;
	nRem = ndiv_progress.rem;

	SetBrushColor(hdc, RGB2Pixel(hdc, 0x42, 0xce, 0xff));		/* set the fill color */

	/* dislay the % */
	if (whOne >= 4) {
		for (i = 0, ix = x + 1; i < nNowPart; ++i) {
			if ((ix + whOne) > (x + w))
				whOne = x + w - ix;

			FillBox (hdc, ix, y + 1, whOne, h - 2);
			ix += whOne;
/*
			if(nRem > 0) {
				ix ++;
				nRem --;
			}
*/
		}
	} else {
		int prog = w * nNowPart/nAllPart;

		FillBox (hdc, x, y + 1, prog, h - 2);
	}
}


static int NewProgressBarProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC           hdc;
	PCONTROL      pCtrl;

	pCtrl = gui_Control (hWnd);

	switch(message) {
	case MSG_CREATE:
		{
		}
		break;
	case MSG_PAINT:
		{
			PROGRESSDATA *data = (PROGRESSDATA *)pCtrl->dwAddData2;
			hdc = BeginPaint (hWnd);

			my_draw_progress (hWnd, hdc, data->nMax, data->nMin, data->nPos);

			EndPaint (hWnd, hdc);
			return 0;
		}

	default:
		break;
	}

	return (*oldProc) (hWnd, message, wParam, lParam);
}

int CdrProgressBarProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case MSG_CREATE:
		{
			int char_len;
			int label_w;
			int progressbar_w;
			HWND retWnd;
			RECT rect;
			int x, y, w, h;
			ProgressBarData_t* PGBData;

			if(!lParam) {
				cvr_error("invalid lParam\n");
				return -1;
			}
			PGBData = (ProgressBarData_t*)lParam;

			GetClientRect(hWnd, &rect);
			x = rect.left;
			y = rect.top;
			w = rect.right;
			h = rect.bottom;

			cvr_debug("x = %d\n", x);
			cvr_debug("y = %d\n", y);
			cvr_debug("w = %d\n", w);
			cvr_debug("h = %d\n", h);

			char_len = strlen("00:00");
			label_w = 8*char_len;
			progressbar_w = w-(2*label_w+12);

			retWnd = CreateWindowEx(CTRL_STATIC, NULL,
					WS_VISIBLE,
					WS_EX_NONE | WS_EX_TRANSPARENT | SS_RIGHT,
					IDC_LABEL_LEFT,
					x + 1, y + 3 , label_w, h,
					hWnd, 0);
			if(retWnd == HWND_INVALID) {
				cvr_error("create playback progress bar label left failed\n");
				break;
			}

			retWnd = CreateWindowEx(CTRL_STATIC, NULL,
					WS_VISIBLE,
					WS_EX_NONE | WS_EX_TRANSPARENT | SS_LEFT,
					IDC_LABEL_RIGHT,
					x+1+label_w+5+progressbar_w+5, y + 3, label_w, h,
					hWnd, 0);
			if(retWnd == HWND_INVALID) {
				cvr_error("create playback progress bar label right failed\n");
				break;
			}

			retWnd = CreateWindowEx(CTRL_PROGRESSBAR, NULL,
					WS_VISIBLE,
					WS_EX_NONE,
					IDC_PROGRESSBAR,
					x+1+label_w+5, h/3 , progressbar_w, h/3,
					hWnd, 0);
			if(retWnd == HWND_INVALID) {
				cvr_error("create playback progress bar label right failed\n");
				break;
			}
			oldProc = SetWindowCallbackProc(retWnd, NewProgressBarProc);

			SetWindowBkColor(hWnd, PGBData->bgcWidget);
			SetWindowElementAttr(GetDlgItem(hWnd, IDC_LABEL_LEFT), WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, PGBData->fgcWidget));
			SetWindowElementAttr(GetDlgItem(hWnd, IDC_LABEL_RIGHT), WE_FGC_WINDOW, Pixel2DWORD(HDC_SCREEN, PGBData->fgcWidget));
		}
		break;
	case PGBM_SETTIME_RANGE:
		{
			PGBTime_t *start_time, *end_time;
			unsigned int pgb_max, pgb_min;
			char buf_left[20] = {0}, buf_right[20] = {0};

			start_time   = (PGBTime_t*)wParam;
			end_time = (PGBTime_t*)lParam;

			if(end_time == NULL)
				break;

			if(start_time == NULL) {
				pgb_min = 0;
				sprintf(buf_left, "%02d:%02d", 0, 0);
			} else {
				if(start_time->min > 59)
					start_time->min = 59;
				if(start_time->sec > 59)
					start_time->sec = 59;
				pgb_min = start_time->sec + start_time->min * 60;
				sprintf(buf_left, "%02d:%02d", start_time->min, start_time->sec);
			}

			if(end_time->min > 59)
				end_time->min = 59;
			if(end_time->sec > 59)
				end_time->sec = 59;
			pgb_max = end_time->sec + end_time->min * 60;
			sprintf(buf_right, "%02d:%02d", end_time->min, end_time->sec);

			SetWindowText(GetDlgItem(hWnd, IDC_LABEL_LEFT), buf_left);
			SetWindowText(GetDlgItem(hWnd, IDC_LABEL_RIGHT), buf_right);
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESSBAR), PBM_SETRANGE, pgb_min, pgb_max-1);
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESSBAR), PBM_SETPOS, 0, 0);
		}
		break;
	case PGBM_SETCURTIME:
		{
			unsigned int pgb_cur;
			char buf_cur[20] = {0};
			PGBTime_t* cur_time = (PGBTime_t*)wParam;

			if(cur_time == NULL) {
				pgb_cur = 0;
				sprintf(buf_cur, "%02d:%02d", 0, 0);
			} else {//todo
				if(cur_time->min > 59)
					cur_time->min = 59;
				if(cur_time->sec > 59)
					cur_time->sec = 59;

				pgb_cur = cur_time->sec + cur_time->min * 60;
				sprintf(buf_cur, "%02d:%02d", cur_time->min, cur_time->sec);
			}
			SetWindowText(GetDlgItem(hWnd, IDC_LABEL_LEFT), buf_cur);
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESSBAR), PBM_SETPOS, pgb_cur, 0);
		}
		break;

	case MSG_CLOSE:
		{
			DestroyAllControls(hWnd);
			DestroyWindow(hWnd);
		}
		break;

	case MSG_DESTROY:
		{
			;
		}
		break;

	default:
		break;
	}

	return DefaultControlProc (hWnd, message, wParam, lParam);
}

int RegisterPGBControls(void)
{
	WNDCLASS WndClass;

	WndClass.spClassName = CTRL_CDRPROGRESSBAR;
	WndClass.dwStyle     = WS_NONE;
	WndClass.dwExStyle   = WS_EX_NONE;
	WndClass.hCursor     = 0;/*GetSystemCursor (0);*/
	WndClass.iBkColor    = RGBA2Pixel(HDC_SCREEN, 0x00, 0x00, 0xff, 0x10);
	WndClass.WinProc = CdrProgressBarProc;

	if(RegisterWindowClass(&WndClass) == FALSE) {
		cvr_error("register MenuList failed\n");
		return -1;
	}

	return 0;
}

void UnRegisterPGBControls(void)
{
	UnregisterWindowClass(CTRL_CDRPROGRESSBAR);
}
