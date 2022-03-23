#include "TipLabel.h"
#include "CvrResource.h"

#define IDC_TITLE	100
#define IDC_TEXT	101
#define ONE_SHOT_TIMER	200
#define TIMEOUT_TIMER	201

#define SS_VCENTER		0x00000040
#define TIME_INFINITE 0xFFFFFF

typedef struct {
	enum LANG_STRINGS TitleCmd;
	enum LANG_STRINGS TextCmd;
}labelStringCmd_t;

labelStringCmd_t labelStringCmd[] = {
	{LANG_LABEL_TIPS,				LANG_LABEL_NO_TFCARD},
	{LANG_LABEL_WARNING,			LANG_LABEL_TFCARD_FULL},
	{LANG_LABEL_WARNING,			LANG_LABEL_TFCARD_FULL1},
	{LANG_LABEL_WARNING,			LANG_LABEL_PLAYBACK_FAIL},
	{LANG_LABEL_SHUTDOWN_TITLE,		LANG_LABEL_LOW_POWER_SHUTDOWN},
	{LANG_LABEL_SHUTDOWN_TITLE,		LANG_LABEL_10S_SHUTDOWN},
	{LANG_LABEL_SHUTDOWN_TITLE,		LANG_LABEL_SHUTDOWN_NOW},
	{LANG_LABEL_SHUTDOWN_TITLE,		LANG_LABEL_30S_NOWORK_SHUTDOWN},
	{LANG_LABEL_TIPS,				LANG_LABEL_FILELIST_EMPTY},
	{LANG_LABEL_TIPS,				LANG_LABEL_SUBMENU_FORMATTING_TEXT},
	{LANG_LABEL_IMPACT_NUM,			LANG_LABEL_IMPACT_NUM},
	{LANG_LABEL_TIPS,				LANG_LABEL_OTA_UPDATE},
	{LANG_LABEL_TIPS,				LANG_LABEL_INVAILD_TFCATD},
	{LANG_LABEL_TIPS,				LANG_LABEL_FORMATTING},
};

typedef struct {
	CvrRectType rect;
	unsigned int titleHeight;
	const char* title;
	const char* text;
	PLOGFONT pLogFont;
	gal_pixel bgc_widget;
	gal_pixel fgc_widget;
	gal_pixel linec_title;
	bool disableKeyEvent;
	void (*callback)(HWND hText, void* data);
	void* callbackData;
	/*
	 * false:  end the label when anykey down, if disableKeyEvent is not set
	 * true:   end the label when anykey up, if disableKeyEvent is not set
	 * */
	bool endLabelKeyUp;
	unsigned int timeoutMs;
}tipLabelData_t;

CTRLDATA ctrlData[2] =
{
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_CENTER | SS_VCENTER,
		0, 0, 0, 0,
		IDC_TITLE,
		"",
		0,
		WS_EX_NONE | WS_EX_TRANSPARENT,
		NULL, NULL
	},

	{
		CTRL_STATIC,
		WS_VISIBLE | SS_CENTER,
		0, 0, 0, 0,
		IDC_TEXT,
		"",
		0,
		WS_EX_NONE | WS_EX_TRANSPARENT,
		NULL, NULL
	}
};

bool bTipLabelEn = 0;
HWND tipLabelHwnd = NULL;

int CloseTipLabel(void)
{
	if(!bTipLabelEn)
	{
		//cvr_warn("tip label has aready disable\n");
		return -1;
	}
	bTipLabelEn = 0;

	SendNotifyMessage(tipLabelHwnd, MSG_DIALOG_CLOSE_TIP, 0, 0);
	return 0;
}

void TipLabelLine2 (HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveTo(hdc, x1, y1);
	LineTo(hdc, x2, y2);
}

static bool timerCallback(HWND hDlg, int id, DWORD data)
{
	tipLabelData_t* tipLabelData;

	tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);
	if(id == ONE_SHOT_TIMER)
	{
		if(tipLabelData->callback != NULL)
		{
			(*tipLabelData->callback)(hDlg, tipLabelData->callbackData);
		}
	}
	else if(id == TIMEOUT_TIMER)
	{
		CloseTipLabel();
	}
	cvr_debug("tiplabel timer end\n");

	return FALSE;
}

static int DialogProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	int nCount = 0 ;
	static int tickCount = 0;

	switch(message)
	{
		case MSG_INITDIALOG:
			{
				tipLabelData_t* tipLabelData;

				tickCount = GetTickCount();
				tipLabelData = (tipLabelData_t*)lParam;
				if(!tipLabelData) {
					cvr_error("invalid tipLabelData\n");
					return -1;
				}

				tipLabelHwnd = hDlg;
				SetWindowAdditionalData(hDlg, (DWORD)tipLabelData);

				if(tipLabelData->pLogFont) {
					SetWindowFont(hDlg, tipLabelData->pLogFont);
				}

				SetWindowBkColor(hDlg, tipLabelData->bgc_widget);
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TITLE), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, tipLabelData->fgc_widget) );
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TEXT), WE_FGC_WINDOW,  PIXEL2DWORD(HDC_SCREEN, tipLabelData->fgc_widget) );

				if(tipLabelData->callback != NULL) {
					cvr_debug("start timer\n");
					SetTimerEx(hDlg, ONE_SHOT_TIMER, 5, timerCallback);
				}
				if(tipLabelData->timeoutMs != TIME_INFINITE) {
					cvr_debug("set timeOut %d ms\n", (int)tipLabelData->timeoutMs);
					SetTimerEx(hDlg, TIMEOUT_TIMER, tipLabelData->timeoutMs / 10, timerCallback);
				}

				break;
			}

		case MSG_PAINT:
			{
				RECT rect;
				HDC hdc;
				tipLabelData_t* tipLabelData;

				hdc = BeginPaint(hDlg);

				tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);
				GetClientRect(GetDlgItem(hDlg, IDC_TITLE), &rect);
				SetPenColor(hdc, tipLabelData->linec_title );
				TipLabelLine2(hdc, 0, RECTH(rect) + 2, RECTW(rect), RECTH(rect) + 2);

				EndPaint(hDlg, hdc);
				break;
			}

		case MSG_FONTCHANGED:
			{
				PLOGFONT pLogFont;
				pLogFont = GetWindowFont(hDlg);
				if(pLogFont) {
					SetWindowFont(GetDlgItem(hDlg, IDC_TITLE), pLogFont);
					SetWindowFont(GetDlgItem(hDlg, IDC_TEXT), pLogFont);
				}
				break;
			}

		case MSG_KEYUP:
			{
				tipLabelData_t* tipLabelData;

				tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);
				if(tipLabelData->disableKeyEvent) {
					cvr_debug("disable key event\n");
					break;
				}

				nCount = GetTickCount()-tickCount;
				cvr_debug("nCount=%d\n", nCount);
				if(tipLabelData->endLabelKeyUp && nCount>=100)
				{
					CloseTipLabel();
				}
				break;
			}

		case MSG_KEYDOWN:
			{
				break;
			}

		case MSG_DIALOG_CLOSE_TIP:
			{
				if(IsTimerInstalled(hDlg, ONE_SHOT_TIMER))
				{
					KillTimer(hDlg, ONE_SHOT_TIMER);
				}
				if(IsTimerInstalled(hDlg, TIMEOUT_TIMER))
				{
					KillTimer(hDlg, TIMEOUT_TIMER);
				}

				EndDialog(hDlg, 0);
				break;
		   }
		default:
		   {
				break;
		   }
	}

	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

int showTipLabel(HWND hParent, tipLabelData_t* info)
{
	CvrRectType rect;
	DLGTEMPLATE dlg;

	if(info == NULL) {
		cvr_error("invalid info\n");
		return -1;
	}
	memset(&dlg, 0, sizeof(dlg));

	rect = info->rect;

	ctrlData[0].x = 0;
	ctrlData[0].y = 0;
	ctrlData[0].w = rect.w;
	ctrlData[0].h = info->titleHeight;
	ctrlData[0].caption = info->title;

	ctrlData[1].x = 10;
	ctrlData[1].y = ctrlData[0].h +30;
	ctrlData[1].w = rect.w - 20;
	ctrlData[1].h = rect.h - 10 - ctrlData[1].y;
	ctrlData[1].caption = info->text;

	dlg.dwStyle = WS_VISIBLE;
	dlg.dwExStyle = WS_EX_NONE;
	dlg.x = rect.x;
	dlg.y = rect.y;
	dlg.w = rect.w;
	dlg.h =	rect.h;
	dlg.caption = "";
	dlg.hIcon = 0;
	dlg.hMenu = 0;
	dlg.controlnr = 2;
	dlg.controls = ctrlData;
	dlg.dwAddData = 0;

	return DialogBoxIndirectParam(&dlg, hParent, DialogProc, (LPARAM)info);
}

int getTipLabelData(tipLabelData_t* tipLabelData)
{
	int retval;

	retval = getResRect(ID_TIPLABEL, &tipLabelData->rect);
	if(retval < 0) {
		cvr_error("get tiplabel rect failed\n");
		return -1;
	}
	retval = getResIntValue(ID_TIPLABEL, INTVAL_TITLEHEIGHT);
	if(retval < 0) {
		cvr_error("get tiplabel rect failed\n");
		return -1;
	}
	tipLabelData->titleHeight = retval;
	cvr_debug("titleHeight is %d\n", (int)tipLabelData->titleHeight);

	tipLabelData->bgc_widget = getResColor(ID_TIPLABEL, COLOR_BGC);
	tipLabelData->fgc_widget = getResColor(ID_TIPLABEL, COLOR_FGC);
	tipLabelData->linec_title = getResColor(ID_TIPLABEL, COLOR_LINEC_TITLE);

	tipLabelData->pLogFont = getLogFont();

	return 0;
}

int ShowTipLabel(HWND hParent, enum labelIndex index, bool endLabelKeyUp, unsigned int timeoutMs)
{
	tipLabelData_t tipLabelData;

	if(bTipLabelEn)
	{
		cvr_warn("tip label has aready en\n");
		return -1;
	}
	bTipLabelEn = 1;

	if(index >= TABLESIZE(labelStringCmd)) {
		cvr_error("invalid index: %d\n", index);
		return -1;
	}

	memset(&tipLabelData, 0, sizeof(tipLabelData) );

	if(getTipLabelData(&tipLabelData) < 0) {
		cvr_error("get tipLabelData failed\n");
		return -1;
	}

	tipLabelData.title = getLabel(labelStringCmd[index].TitleCmd);
	if(!tipLabelData.title) {
		cvr_error("get LANG_LABEL_LOW_POWER_TITLE failed\n");
		return -1;
	}

	tipLabelData.text = getLabel(labelStringCmd[index].TextCmd);
	tipLabelData.timeoutMs = timeoutMs;

	if(!tipLabelData.text)
	{
		cvr_error("get LANG_LABEL_LOW_POWER_TEXT failed\n");
		return -1;
	}

	tipLabelData.endLabelKeyUp = endLabelKeyUp;

	return showTipLabel(hParent, &tipLabelData);
}
