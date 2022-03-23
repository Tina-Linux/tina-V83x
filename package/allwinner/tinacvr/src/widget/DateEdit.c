#include "DateEdit.h"
#include "CvrResource.h"
#include "middle_ware.h"

#define SS_VCENTER		0x00000040

#define IDC_LABEL_START		200
#define IDC_LABEL_TITLE		200
#define IDC_LABEL_YEAR		201
#define IDC_LABEL_MONTH		202
#define IDC_LABEL_DAY		203
#define IDC_LABEL_HOUR		204
#define IDC_LABEL_MINUTE	205
#define IDC_LABEL_SECOND	206
#define IDC_START	207
#define IDC_YEAR	207
#define IDC_MONTH	208
#define IDC_DAY		209
#define IDC_HOUR	210
#define IDC_MINUTE	211
#define IDC_SECOND	212
#define IDC_END		212

#define INDEX_LABEL_TITLE	0
#define INDEX_LABEL_YEAR	1
#define INDEX_LABEL_MONTH	2
#define INDEX_LABEL_DAY		3
#define INDEX_LABEL_HOUR	4
#define INDEX_LABEL_MINUTE	5
#define INDEX_LABEL_SECOND	6
#define INDEX_YEAR			7
#define INDEX_MONTH			8
#define INDEX_DAY			9
#define INDEX_HOUR			10
#define INDEX_MINUTE		11
#define INDEX_SECOND		12

#define SELECT_BORDER	1
#define UNSELECT_BORDER 0

typedef struct {
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
}date_t;

void DataEditLine2 (HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveTo(hdc, x1, y1);
	LineTo(hdc, x2, y2);
}

static void getSystemDate(date_t* date)
{
	struct tm u_time;

	memset(&u_time, 0, sizeof(struct tm));
	get_local_time(&u_time);
	date->year = u_time.tm_year;
	date->month = u_time.tm_mon;
	date->day = u_time.tm_mday;
	date->hour = u_time.tm_hour;
	date->minute = u_time.tm_min;
	date->second = u_time.tm_sec;
}

static void setSystemDate(date_t* date)
{
	struct tm tm;

	tm.tm_year = date->year;
	tm.tm_mon = date->month;
	tm.tm_mday = date->day;
	tm.tm_hour = date->hour;
	tm.tm_min = date->minute;
	tm.tm_sec = date->second;
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = 0;

	set_local_time(&tm);
}

static int getMonthDays(int year, int month)
{
	int days;
	switch (month){
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		days = 31;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		days = 30;
		break;
	case 2:
		if ((year % 400 == 0) || ((year % 4 == 0 && year % 100 != 0)))
			days = 29;
		else
			days = 28;
		break;
	default:
		days = -1;
		break;
	}

	return days;
}

static void getCurrentSetDate(HWND hDlg, date_t* date)
{
	char timeStr[5] = {0};

	GetWindowText(GetDlgItem(hDlg, IDC_YEAR), timeStr, sizeof(timeStr));
	date->year = atoi(timeStr);
	bzero(timeStr, sizeof(timeStr));

	GetWindowText(GetDlgItem(hDlg, IDC_MONTH), timeStr, sizeof(timeStr));
	date->month = atoi(timeStr);

	GetWindowText(GetDlgItem(hDlg, IDC_DAY), timeStr, sizeof(timeStr));
	date->day = atoi(timeStr);

	GetWindowText(GetDlgItem(hDlg, IDC_HOUR), timeStr, sizeof(timeStr));
	date->hour = atoi(timeStr);

	GetWindowText(GetDlgItem(hDlg, IDC_MINUTE), timeStr, sizeof(timeStr));
	date->minute = atoi(timeStr);

	GetWindowText(GetDlgItem(hDlg, IDC_SECOND), timeStr, sizeof(timeStr));
	date->second = atoi(timeStr);

	cvr_debug("current set date is %04d-%02d-%02d--%02d-%02d-%02d\n", (int)date->year, (int)date->month,
		(int)date->day, (int)date->hour, (int)date->minute, (int)date->second);
}

static void add(HWND hDlg, unsigned int IDCSelected, int addEnd)
{
	int value;
	char timeStr[5] = {0};

	GetWindowText(GetDlgItem(hDlg, IDCSelected), timeStr, sizeof(timeStr));
	value = atoi(timeStr);

	value += addEnd;
	switch(IDCSelected) {
	case IDC_YEAR:
		if(value < 1970)
			value = 1970;
		break;
	case IDC_MONTH:
		if(value > 12)
			value = 1;
		else if(value < 1)
			value = 12;
		break;
	case IDC_DAY:
		{
			int year, month, days;
			GetWindowText(GetDlgItem(hDlg, IDC_YEAR), timeStr, sizeof(timeStr));
			year = atoi(timeStr);
			GetWindowText(GetDlgItem(hDlg, IDC_MONTH), timeStr, sizeof(timeStr));
			month = atoi(timeStr);
			days = getMonthDays(year, month);
			cvr_debug("year-%d-month-%d have %d days\n", year, month, days);
			if(value > days)
				value = 1;
			else if(value < 1)
				value = days;
		}
		break;
	case IDC_HOUR:
		if(value > 23)
			value = 0;
		else if(value < 0)
			value = 23;
		break;
	case IDC_MINUTE:
	case IDC_SECOND:
		if(value > 59)
			value = 0;
		else if(value < 0)
			value = 59;
		break;
	}

	if(IDCSelected == IDC_YEAR)
		sprintf(timeStr, "%04d", value);
	else
		sprintf(timeStr, "%02d", value);

	SetWindowText(GetDlgItem(hDlg, IDCSelected), timeStr);
}

static void SetBorderColor(HWND hDlg, unsigned int IDC, unsigned int select_flg)
{
	RECT rect;
	dateSettingData_t* configData;
	HWND hCtrl;

	configData = (dateSettingData_t*)GetWindowAdditionalData(hDlg);
	hCtrl = GetDlgItem(hDlg, IDC);

	HDC hdc = GetDC(hCtrl);

	GetClientRect(hCtrl, &rect);
	if (select_flg == SELECT_BORDER){
		SetPenColor(hdc, configData->borderc_selected );
	}
	else if (select_flg == UNSELECT_BORDER){
		SetPenColor(hdc, configData->borderc_normal );
	}

	Rectangle (hdc, rect.left, rect.top, rect.right-1, rect.bottom-1);

	EndPaint(hCtrl, hdc);
}

static int dateSettingProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	static unsigned int IDCSelected = IDC_YEAR;
	static unsigned long long tickCnt;
	dateSettingData_t* configData;
	#ifdef USE_IPS_SCREEN
    static int eraseNum = 0;
	#endif

	//cvr_debug("dateSettingProc is 0x%x, 0x%x, 0x%x, 0x%x\n", hDlg, message, wParam, lParam);
	switch(message) {
	case MSG_INITDIALOG:
		{
			configData = (dateSettingData_t*)lParam;
			if(configData == NULL) {
				cvr_debug("configData is NULL\n");
				EndDialog(hDlg, -1);
			}
			SetWindowAdditionalData(hDlg, (DWORD)configData);

			for (unsigned int i = 0; i < 7; i++){
				SetWindowElementAttr(GetDlgItem (hDlg, IDC_LABEL_START + i ), WE_FGC_WINDOW , PIXEL2DWORD(HDC_SCREEN, configData->fgc_label) ); //0xFFDCDCDC
			}
			for (unsigned int i = 0; i < 6; i++){
				SetWindowElementAttr(GetDlgItem (hDlg, IDC_START + i), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, configData->fgc_number) );//0xFF919191
			}

			SetWindowBkColor(hDlg, configData->bgc_widget);
			SetWindowFont(hDlg, configData->pLogFont);
		}
		break;
	case MSG_FONTCHANGED:
		{
			PLOGFONT pLogFont;
			pLogFont = GetWindowFont(hDlg);
			for(unsigned int i = 0; i < 13; i++) {
				SetWindowFont(GetDlgItem(hDlg, IDC_LABEL_START + i), pLogFont);
			}
		}
		break;
	case MSG_PAINT:
		{
			RECT rect;
			HDC hdc = BeginPaint(hDlg);

			configData = (dateSettingData_t*)GetWindowAdditionalData(hDlg);
			GetClientRect(GetDlgItem(hDlg, IDC_LABEL_TITLE), &rect);
			SetPenColor(hdc, configData->linec_title);

			DataEditLine2(hdc, 0, RECTH(rect), RECTW(rect), RECTH(rect));
			SetBorderColor(hDlg, IDCSelected, SELECT_BORDER);
			for(unsigned int i = 0; i < 6; i++) {
				if(IDC_START + i != IDCSelected)
					SetBorderColor(hDlg, IDC_START + i, UNSELECT_BORDER);
			}
			EndPaint(hDlg, hdc);
		}
		break;
	case MSG_KEYDOWN:
		{
#ifdef USE_IPS_SCREEN
            if(eraseNum <= 2)
                return 0;
#endif
			if(wParam == CVR_KEY_MODE) {
				tickCnt = GetTickCount();
			}
		}
		break;
	case MSG_KEYUP:
		{
#ifdef USE_IPS_SCREEN
            if(eraseNum <= 2)
                return 0;
#endif
			switch(wParam) {
			case CVR_KEY_OK:
				IDCSelected++;
				if(IDCSelected > IDC_END)
					IDCSelected = IDC_START;
				cvr_debug("IDCSelected is %d\n", (int)IDCSelected);
				InvalidateRect(hDlg, NULL, FALSE);
				break;
			case CVR_KEY_LEFT:
				add(hDlg, IDCSelected, -1);
				break;
			case CVR_KEY_RIGHT:
				add(hDlg, IDCSelected, 1);
				break;
			case CVR_KEY_MODE:
				if((GetTickCount() - tickCnt ) < 50)
				{
					date_t date;
					getCurrentSetDate(hDlg, &date);
					setSystemDate(&date);
				}
				EndDialog(hDlg, 0);
				break;
			}
		}
		break;
	case MSG_ERASEBKGND:
#ifdef USE_IPS_SCREEN
        {
            //to avoid the extra erase message the first time
            if(eraseNum == 0 || eraseNum == 1)
                eraseNum ++;
            if(eraseNum ==2) {
                eraseNum = 3;
                return 0;
             }
        }
#endif
		break;
        case MSG_DESTROY:
            {
#ifdef USE_IPS_SCREEN
               eraseNum = 0;
#endif
            }
            break;
		default:
			break;
	}
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int DateEditDialog(HWND hParent, dateSettingData_t* configData)
{
	CTRLDATA ctrlData[13];
	DLGTEMPLATE dlgDateSetting;
	unsigned int intval;
	memset(ctrlData, 0, sizeof(ctrlData));

	intval = configData->rect.h - configData->titleRectH - configData->boxH;
	intval = configData->titleRectH + intval / 3;

	for(unsigned int i = 0; i < 13; i++) {
		ctrlData[i].class_name = CTRL_STATIC;
		ctrlData[i].dwStyle = SS_CENTER | SS_VCENTER | WS_VISIBLE;
		ctrlData[i].caption = "";
		ctrlData[i].dwExStyle = WS_EX_TRANSPARENT;
		ctrlData[i].werdr_name = NULL;
		ctrlData[i].we_attrs = NULL;
		ctrlData[i].id	= IDC_LABEL_START + i;

		ctrlData[i].y = intval;
		ctrlData[i].h = configData->boxH;
	}
	ctrlData[INDEX_LABEL_TITLE].caption = configData->title;
	ctrlData[INDEX_LABEL_YEAR].caption = configData->year;
	ctrlData[INDEX_LABEL_MONTH].caption = configData->month;
	ctrlData[INDEX_LABEL_DAY].caption = configData->day;
	ctrlData[INDEX_LABEL_HOUR].caption = configData->hour;
	ctrlData[INDEX_LABEL_MINUTE].caption = configData->minute;
	ctrlData[INDEX_LABEL_SECOND].caption = configData->second;

	cvr_debug("(%s, %s, %s)\n", configData->title, configData->year, configData->day);

	ctrlData[INDEX_LABEL_TITLE].x = 0;
	ctrlData[INDEX_LABEL_TITLE].y = 0;
	ctrlData[INDEX_LABEL_TITLE].w = configData->rect.w;
	ctrlData[INDEX_LABEL_TITLE].h = configData->titleRectH;

	ctrlData[INDEX_YEAR].x = configData->hBorder;
	ctrlData[INDEX_YEAR].w = configData->yearW;
	ctrlData[INDEX_LABEL_YEAR].x = ctrlData[INDEX_YEAR].x +  ctrlData[INDEX_YEAR].w + 2;
	ctrlData[INDEX_LABEL_YEAR].w = configData->dateLabelW;

	ctrlData[INDEX_MONTH].x = ctrlData[INDEX_LABEL_YEAR].x +  ctrlData[INDEX_LABEL_YEAR].w + 2;
	ctrlData[INDEX_MONTH].w = configData->numberW;
	ctrlData[INDEX_LABEL_MONTH].x = ctrlData[INDEX_MONTH].x +  ctrlData[INDEX_MONTH].w + 2;
	ctrlData[INDEX_LABEL_MONTH].w = configData->dateLabelW;

	ctrlData[INDEX_DAY].x = ctrlData[INDEX_LABEL_MONTH].x +  ctrlData[INDEX_LABEL_MONTH].w + 2;
	ctrlData[INDEX_DAY].w = configData->numberW;
	ctrlData[INDEX_LABEL_DAY].x = ctrlData[INDEX_DAY].x +  ctrlData[INDEX_DAY].w + 2;
	ctrlData[INDEX_LABEL_DAY].w = configData->dateLabelW;

	ctrlData[INDEX_HOUR].x = ctrlData[INDEX_LABEL_DAY].x +  ctrlData[INDEX_LABEL_DAY].w + 2;
	ctrlData[INDEX_HOUR].w = configData->numberW;
	ctrlData[INDEX_LABEL_HOUR].x = ctrlData[INDEX_HOUR].x +  ctrlData[INDEX_HOUR].w + 2;
	ctrlData[INDEX_LABEL_HOUR].w = configData->dateLabelW;

	ctrlData[INDEX_MINUTE].x = ctrlData[INDEX_LABEL_HOUR].x +  ctrlData[INDEX_LABEL_HOUR].w + 2;
	ctrlData[INDEX_MINUTE].w = configData->numberW;
	ctrlData[INDEX_LABEL_MINUTE].x = ctrlData[INDEX_MINUTE].x +  ctrlData[INDEX_MINUTE].w + 2;
	ctrlData[INDEX_LABEL_MINUTE].w = configData->dateLabelW;

	ctrlData[INDEX_SECOND].x = ctrlData[INDEX_LABEL_MINUTE].x +  ctrlData[INDEX_LABEL_MINUTE].w + 2;
	ctrlData[INDEX_SECOND].w = configData->numberW;
	ctrlData[INDEX_LABEL_SECOND].x = ctrlData[INDEX_SECOND].x +  ctrlData[INDEX_SECOND].w + 2;
	ctrlData[INDEX_LABEL_SECOND].w = configData->dateLabelW;

	date_t date;
	char dateString[6][5];
	getSystemDate(&date);
	sprintf(dateString[0],"%04u", date.year);
	sprintf(dateString[1],"%02u", date.month);
	sprintf(dateString[2],"%02u", date.day);
	sprintf(dateString[3],"%02u", date.hour);
	sprintf(dateString[4],"%02u", date.minute);
	sprintf(dateString[5],"%02u", date.second);
	ctrlData[INDEX_YEAR].caption = dateString[0];
	ctrlData[INDEX_MONTH].caption = dateString[1];
	ctrlData[INDEX_DAY].caption = dateString[2];
	ctrlData[INDEX_HOUR].caption = dateString[3];
	ctrlData[INDEX_MINUTE].caption = dateString[4];
	ctrlData[INDEX_SECOND].caption = dateString[5];

	memset(&dlgDateSetting, 0, sizeof(dlgDateSetting));
	dlgDateSetting.dwStyle = WS_VISIBLE;
	dlgDateSetting.dwExStyle = WS_EX_NONE;
	dlgDateSetting.x = configData->rect.x;
	dlgDateSetting.y = configData->rect.y;
	dlgDateSetting.w = configData->rect.w;
	dlgDateSetting.h = configData->rect.h;
	dlgDateSetting.caption = "";
	dlgDateSetting.controlnr = TABLESIZE(ctrlData);
	dlgDateSetting.controls = ctrlData;

	return DialogBoxIndirectParam (&dlgDateSetting, hParent, dateSettingProc, (LPARAM)configData);
}
