#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/minigui.h>

#include <mgncs4touch/listbox_view.h>
#include <mgncs4touch/item_set_view.h>

#include "test_list_res.h"
#define __ID_TIMER_SLIDER 100
#define LISTBOX_PNT_PATH "/usr/bin/mg-mgncs/res/listbox/"

ListboxView *listboxView = NULL;
ListboxView *listboxView_2 = NULL;
ListboxView *wifilistbox = NULL;

// DialogBox
PLOGFONT ID_FONT_SIMSUN_25;
PLOGFONT ID_FONT_SIMSUN_20;
static int itemForceIndex = 0;
void systeminit(void)
{
	ID_FONT_SIMSUN_20 = CreateLogFont("ttf", "simsun", "UTF-8",
		FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
		FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
		FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 20, 0);

	ID_FONT_SIMSUN_25 = CreateLogFont("ttf", "simsun", "UTF-8",
		FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
		FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
		FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 25, 0);
}
void ItemSetInfoInit(int index, ItemSetInfo *info, ListboxView *listboxView)
{
	int err_code;
	char pBuf[ITEM_MAX_NAME];

	info->x = 200;
	info->y = 120;
	info->w = 400;
	info->h = 200;

	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, VOLUME);
	memcpy(info->hdText, pBuf, sizeof(pBuf));
	printf("hdText=%s\n", info->hdText);
	info->hth = 70;
	info->hdFont = ID_FONT_SIMSUN_25;
	info->hTextColor = RGB2Pixel(HDC_SCREEN, 0, 0, 0);

	info->iy = 50;
	info->iSz = 3;
	info->iIndex = listboxView->itDt[index].btn->sltItem;
	info->iGap = 20;
	info->itemFont = ID_FONT_SIMSUN_20;
	info->iTextColor = RGB2Pixel(HDC_SCREEN, 0, 0, 0);

	info->lineColor = PIXEL_lightgray;

	err_code = LoadBitmapFromFile(HDC_SCREEN, &info->itemBmp, LISTBOX_PNT_PATH"choice_normal.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, HIGH);
	memcpy(info->item[0], pBuf, sizeof(pBuf));

	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, MIDDLE);
	memcpy(info->item[1], pBuf, sizeof(pBuf));

	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, LOW);
	memcpy(info->item[2], pBuf, sizeof(pBuf));

	info->h = info->hth + info->iy*info->iSz;
}

void ItemSetInfoUnInit(ItemSetInfo *info)
{
	UnloadBitmap(&info->itemBmp);
}

static ListboxView* ListboxDataInit_2(ListboxView *listboxView)
{
	int i;
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	listboxView = (ListboxView*) malloc(sizeof(ListboxView));
	if (NULL == listboxView) {
		printf("malloc ListboxView data error\n");
		return NULL;
	}
	memset((void *) listboxView, 0, sizeof(ListboxView));

	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->winBg, LISTBOX_PNT_PATH"set_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"set_bg.png");
	}
	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->ltbBg, LISTBOX_PNT_PATH"list_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"list_bg.png");
	}
	listboxView->lTextFt = ID_FONT_SIMSUN_25;
	listboxView->lTextColor =  RGB2Pixel(HDC_SCREEN, 255, 179, 9);
	listboxView->rTextFt = ID_FONT_SIMSUN_20;
	listboxView->rTextColor =  PIXEL_lightgray;
	listboxView->ht = 800;
	listboxView->ix = 750;
	listboxView->hfp = 25;
	listboxView->hbp = 25;

	listboxView->vt = 480;
	listboxView->iy = 50;
	listboxView->vfp = 105;
	listboxView->vbp = 30;
	listboxView->vgap = 25;

	listboxView->itType = ITEM_LINE | ITEM_BMP;

	listboxView->itSz = 15;
	listboxView->Index = 0;
	listboxView->showSz = 5;
	listboxView->lthGap = 25;
	listboxView->ltvGap = 10;
	listboxView->moveGap = 0;

	listboxView->itDt = (ItemData*) malloc(sizeof(ItemData)*listboxView->itSz);
	memset((void *) listboxView->itDt, 0, sizeof(ItemData)*listboxView->itSz);
	for(i=0; i<listboxView->itSz; i++)
	{
		listboxView->itDt[i].rType = RITEM_NONE;
		listboxView->itDt[i].rType |= RITEM_TEXT;

		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, "1%d", i);
		memcpy(listboxView->itDt[i].lT, pBuf, sizeof(pBuf));

		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, "Option");
		memcpy(listboxView->itDt[i].rT, pBuf, sizeof(pBuf));

	}

	listboxView->itDt[0].rType = RITEM_TEXT;

	listboxView->itDt[1].rType = RITEM_NONE;

	listboxView->itDt[2].rType = RITEM_BMP;
	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->itDt[2].rB, LISTBOX_PNT_PATH"wifi_status_0.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return listboxView;
}

static ListboxView* ListboxDataUninit_2(ListboxView *listboxView)
{

	UnloadBitmap(&listboxView->itDt[2].rB);

	if(listboxView->itDt)
	{
		free(listboxView->itDt);
		listboxView->itDt = NULL;
	}

	UnloadBitmap(&listboxView->winBg);
	UnloadBitmap(&listboxView->ltbBg);
	if(listboxView)
	{
		free(listboxView);
		listboxView = NULL;
	}

	return listboxView;
}

void ButtonItemFunction(int index, DWORD param)
{
	int ret;
	ItemSetInfo info;
	char pBuf[NAME_MAX_SIZE];

	ListboxView *listboxView = (ListboxView *)param;
	printf("index=%d, itSz=%d\n", index, listboxView->itSz);

	if(index == 4)
	{
		ItemSetInfoInit(index, &info, listboxView);
		listboxView->itDt[index].btn->sltItem = ShowItemSetDialog(listboxView->sfHwnd, &info);
		printf("itemForceIndex=%d\n", listboxView->itDt[index].btn->sltItem);
		ItemSetInfoUnInit(&info);

		if(listboxView->itDt[index].btn->sltItem == 0)
		{
			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, HIGH);
			memcpy(listboxView->itDt[index].btn->text, pBuf, sizeof(pBuf));
		}
		else if(listboxView->itDt[index].btn->sltItem == 1)
		{
			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, MIDDLE);
			memcpy(listboxView->itDt[index].btn->text, pBuf, sizeof(pBuf));
		}
		else if(listboxView->itDt[index].btn->sltItem == 2)
		{
			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, LOW);
			memcpy(listboxView->itDt[index].btn->text, pBuf, sizeof(pBuf));
		}
	}
	else if(index == 5)
	{
		ShowWindow(listboxView->sfHwnd, SW_HIDE);

		listboxView_2 = ListboxDataInit_2(listboxView_2);
		if (listboxView_2)
		{
			listboxView_2->parentHwnd = listboxView->parentHwnd;
			listboxView_2->sfHwnd= CreateWindowEx(LISTBOX_VIEW, "",
				WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
				1, 0, listboxView_2->vfp, listboxView_2->ht, listboxView_2->vt - listboxView_2->vfp,
				listboxView->parentHwnd, (DWORD)listboxView_2);
			SetWindowFont(listboxView_2->sfHwnd, ID_FONT_SIMSUN_25);
		}
	}
}

void WifiButtonItemFunction(int index, DWORD param)
{
	int ret;
	ItemSetInfo info;
	char pBuf[NAME_MAX_SIZE];

	ListboxView *listboxView = (ListboxView *)param;
	printf("index=%d, itSz=%d\n", index, listboxView->itSz);

	if(index == 0)
	{
		if(listboxView->itDt[index].btn->onStus)
		{
			listboxView->itSz = 15;
			listboxView->showSz = 5;
		}
		else
		{
			listboxView->itSz = 1;
			listboxView->showSz = 1;
		}
	}
	else
	{
		;
	}
}

static ButtonData* ListboxButtonInit(ButtonData *buttonData)
{
	int err_code;
	buttonData = (ButtonData*) malloc(sizeof(ButtonData));
	if (NULL == buttonData)
	{
		printf("malloc ButtonData data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonData));

	buttonData->onStus = 1;
	buttonData->fun = ButtonItemFunction;
	buttonData->sltItem = -1;
	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->onBmp, LISTBOX_PNT_PATH"choice_normal.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->offBmp, LISTBOX_PNT_PATH"choice_noun_normal.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return buttonData;
}

static ButtonData* ListboxButtonUninit(ButtonData *buttonData)
{
	UnloadBitmap(&buttonData->onBmp);
	UnloadBitmap(&buttonData->offBmp);
	if(buttonData)
	{
		free(buttonData);
		buttonData = NULL;
	}

	return buttonData;
}

static ButtonData* ListboxButtonInit_2(ButtonData *buttonData)
{
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	buttonData = (ButtonData*) malloc(sizeof(ButtonData));
	if (NULL == buttonData)
	{
		printf("malloc ButtonData data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonData));

	buttonData->onStus = 1;
	buttonData->fun = ButtonItemFunction;

	buttonData->sltItem = 0;
	if(buttonData->sltItem == 0)
	{
		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, HIGH);
		memcpy(buttonData->text, pBuf, sizeof(pBuf));
	}
	else if(buttonData->sltItem == 1)
	{
		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, MIDDLE);
		memcpy(buttonData->text, pBuf, sizeof(pBuf));
	}
	else if(buttonData->sltItem == 2)
	{
		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, LOW);
		memcpy(buttonData->text, pBuf, sizeof(pBuf));
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->onBmp, LISTBOX_PNT_PATH"unfold_press.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->offBmp, LISTBOX_PNT_PATH"unfold_press.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return buttonData;
}

static ButtonData* ListboxButtonUninit_2(ButtonData *buttonData)
{
	UnloadBitmap(&buttonData->onBmp);
	UnloadBitmap(&buttonData->offBmp);
	if(buttonData)
	{
		free(buttonData);
		buttonData = NULL;
	}

	return buttonData;
}

static ButtonData* ListboxButtonInit_3(ButtonData *buttonData)
{
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	buttonData = (ButtonData*) malloc(sizeof(ButtonData));
	if (NULL == buttonData)
	{
		printf("malloc ButtonData data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonData));

	buttonData->onStus = 1;
	buttonData->fun = ButtonItemFunction;

	buttonData->sltItem = -1;

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->onBmp, LISTBOX_PNT_PATH"unfold_press.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->offBmp, LISTBOX_PNT_PATH"unfold_press.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return buttonData;
}

static ButtonData* ListboxButtonUninit_3(ButtonData *buttonData)
{
	UnloadBitmap(&buttonData->onBmp);
	UnloadBitmap(&buttonData->offBmp);
	if(buttonData)
	{
		free(buttonData);
		buttonData = NULL;
	}

	return buttonData;
}

static ButtonData* WifiListboxButtonInit_2(ButtonData *buttonData)
{
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	buttonData = (ButtonData*) malloc(sizeof(ButtonData));
	if (NULL == buttonData)
	{
		printf("malloc ButtonData data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonData));

	buttonData->onStus = 1;
	buttonData->fun = WifiButtonItemFunction;

	buttonData->sltItem = -1;

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->onBmp, LISTBOX_PNT_PATH"wifi_status_0.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->offBmp, LISTBOX_PNT_PATH"wifi_status_0.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return buttonData;
}

static ButtonData* WifiListboxButtonUninit_2(ButtonData *buttonData)
{
	UnloadBitmap(&buttonData->onBmp);
	UnloadBitmap(&buttonData->offBmp);
	if(buttonData)
	{
		free(buttonData);
		buttonData = NULL;
	}

	return buttonData;
}

static ButtonData* WifiListboxButtonInit(ButtonData *buttonData)
{
	int err_code;
	buttonData = (ButtonData*) malloc(sizeof(ButtonData));
	if (NULL == buttonData)
	{
		printf("malloc ButtonData  data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonData));

	buttonData->onStus = 1;
	buttonData->fun = WifiButtonItemFunction;
	buttonData->sltItem = -1;

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->onBmp, LISTBOX_PNT_PATH"choice_normal.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->offBmp, LISTBOX_PNT_PATH"choice_noun_normal.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	return buttonData;
}

static ButtonData* WifiListboxButtonUninit(ButtonData *buttonData)
{
	UnloadBitmap(&buttonData->onBmp);
	UnloadBitmap(&buttonData->offBmp);
	if(buttonData)
	{
		free(buttonData);
		buttonData = NULL;
	}

	return buttonData;
}

static ListboxView* ListboxDataInit(ListboxView *listboxView)
{
	int i = 0;
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	listboxView = (ListboxView*) malloc(sizeof(ListboxView));
	if (NULL == listboxView) {
		printf("malloc ListboxView data error\n");
		return NULL;
	}
	memset((void *) listboxView, 0, sizeof(ListboxView));

	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->winBg, LISTBOX_PNT_PATH"set_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"set_bg.png");
	}
	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->ltbBg, LISTBOX_PNT_PATH"list_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"list_bg.png");
	}
	listboxView->lTextFt = ID_FONT_SIMSUN_25;
	listboxView->lTextColor =  RGB2Pixel(HDC_SCREEN, 255, 179, 9);
	listboxView->rTextFt = ID_FONT_SIMSUN_20;
	listboxView->rTextColor =  PIXEL_lightgray;
	listboxView->ht = 800;
	listboxView->ix = 750;
	listboxView->hfp = 25;
	listboxView->hbp = 25;

	listboxView->vt = 480;
	listboxView->iy = 50;
	listboxView->vfp = 105;
	listboxView->vbp = 30;
	listboxView->vgap = 25;

	listboxView->itType = ITEM_LINE | ITEM_BMP;
	listboxView->lineColor = PIXEL_lightgray;
	listboxView->itSz = 15;
	listboxView->Index = 0;
	listboxView->showSz = 5;
	listboxView->lthGap = 25;
	listboxView->ltvGap = 40;
	listboxView->moveGap = 10;

	listboxView->itDt = (ItemData*) malloc(sizeof(ItemData)*listboxView->itSz);
	memset((void *) listboxView->itDt, 0, sizeof(ItemData)*listboxView->itSz);
	for(i = 0; i<listboxView->itSz; i++){
		listboxView->itDt[i].rType = RITEM_NONE;
		listboxView->itDt[i].rType |= RITEM_TEXT;
	}
	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, "%s", "Option");
	memcpy(listboxView->itDt[0].lT, pBuf, sizeof(pBuf));
	sprintf(pBuf, "%s", "Bluetooth");
	memcpy(listboxView->itDt[1].lT, pBuf, sizeof(pBuf));
	sprintf(pBuf, "%s", "WLAN");
	memcpy(listboxView->itDt[2].lT, pBuf, sizeof(pBuf));
	sprintf(pBuf, "%s", "Display");
	memcpy(listboxView->itDt[3].lT, pBuf, sizeof(pBuf));
	sprintf(pBuf, "%s", "Volume");
	memcpy(listboxView->itDt[4].lT, pBuf, sizeof(pBuf));
	memset(pBuf, 0, sizeof(pBuf));
	for(i=5; i<listboxView->itSz; i++){
	sprintf(pBuf, "a");
	memcpy(listboxView->itDt[i].rT, pBuf, sizeof(pBuf));
	}

	listboxView->itDt[0].rType = RITEM_TEXT;

	listboxView->itDt[1].rType = RITEM_NONE;

	listboxView->itDt[2].rType = RITEM_BMP;
	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->itDt[2].rB, LISTBOX_PNT_PATH"wifi_status_0.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load bitmap failed\n");
	}

	listboxView->itDt[3].rType = RITEM_BUTTON;
	listboxView->itDt[3].btn = ListboxButtonInit(listboxView->itDt[3].btn);

	listboxView->itDt[4].rType = RITEM_BUTTON;
	listboxView->itDt[4].btn = ListboxButtonInit_2(listboxView->itDt[4].btn);

	listboxView->itDt[5].rType = RITEM_BUTTON;
	listboxView->itDt[5].btn = ListboxButtonInit_3(listboxView->itDt[5].btn);

	return listboxView;
}

static ListboxView* ListboxDataUninit(ListboxView *listboxView)
{
	listboxView->itDt[4].btn = ListboxButtonUninit_2(listboxView->itDt[4].btn);
	listboxView->itDt[3].btn = ListboxButtonUninit(listboxView->itDt[3].btn);

	UnloadBitmap(&listboxView->itDt[2].rB);

	if(listboxView->itDt)
	{
		free(listboxView->itDt);
		listboxView->itDt = NULL;
	}

	UnloadBitmap(&listboxView->winBg);
	UnloadBitmap(&listboxView->ltbBg);
	if(listboxView)
	{
		free(listboxView);
		listboxView = NULL;
	}

	return listboxView;
}

static ListboxView* WifiListboxDataInit(ListboxView *listboxView)
{
	int i;
	int err_code;
	char pBuf[NAME_MAX_SIZE];

	listboxView = (ListboxView*) malloc(sizeof(ListboxView));
	if (NULL == listboxView) {
		printf("malloc ListboxView data error\n");
		return NULL;
	}
	memset((void *) listboxView, 0, sizeof(ListboxView));

	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->winBg, LISTBOX_PNT_PATH"set_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"set_bg.png");
	}

	err_code = LoadBitmapFromFile(HDC_SCREEN, &listboxView->ltbBg, LISTBOX_PNT_PATH"list_bg.png");
	if (err_code != ERR_BMP_OK)
	{
		printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"list_bg.png");
	}

	listboxView->lTextFt = ID_FONT_SIMSUN_25;
	listboxView->lTextColor =  RGB2Pixel(HDC_SCREEN, 255, 179, 9);
	listboxView->rTextFt = ID_FONT_SIMSUN_20;
	listboxView->rTextColor =  PIXEL_lightgray;
	listboxView->ht = 800;
	listboxView->ix = 750;
	listboxView->hfp = 25;
	listboxView->hbp = 25;

	listboxView->vt = 480;
	listboxView->iy = 50;
	listboxView->vfp = 105;
	listboxView->vbp = 30;
	listboxView->vgap = 25;

	listboxView->itSz = 15;
	listboxView->Index = 0;
	listboxView->showSz = 5;
	listboxView->lthGap = 25;
	listboxView->ltvGap = 10;
	listboxView->moveGap = 0;

	listboxView->itDt = (ItemData*) malloc(sizeof(ItemData)*listboxView->itSz);
	memset((void *) listboxView->itDt, 0, sizeof(ItemData)*listboxView->itSz);
	for(i=1; i<listboxView->itSz; i++)
	{
		listboxView->itDt[i].rType = RITEM_BUTTON;
		memset(pBuf, 0, sizeof(pBuf));
		sprintf(pBuf, "%d", i);
		memcpy(listboxView->itDt[i].lT, pBuf, sizeof(pBuf));

		listboxView->itDt[i].btn = WifiListboxButtonInit_2(listboxView->itDt[i].btn);
	}

	listboxView->itDt[0].rType = RITEM_BUTTON;
	memset(pBuf, 0, sizeof(pBuf));
	sprintf(pBuf, "wifi");
	memcpy(listboxView->itDt[0].lT, pBuf, sizeof(pBuf));
	listboxView->itDt[0].btn = WifiListboxButtonInit(listboxView->itDt[0].btn);

	return listboxView;
}

static int ActivityListProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int err_code;
	static BITMAP bmpBg;

	switch (message)
	{
	case MSG_TIMER:
	{
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		if (listboxView)
		{
			listboxView->parentHwnd = hWnd;
			listboxView->sfHwnd= CreateWindowEx(LISTBOX_VIEW, "",
				WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
				0, 0, listboxView->vfp, listboxView->ht, listboxView->vt-listboxView->vfp,
				hWnd, (DWORD)listboxView);
			SetWindowFont(listboxView->sfHwnd, ID_FONT_SIMSUN_25);
		}

		break;
	}
	case MSG_CREATE:
	{
		err_code = LoadBitmapFromFile(HDC_SCREEN, &bmpBg, LISTBOX_PNT_PATH"set_bg.png");
		if (err_code != ERR_BMP_OK)
		{
			printf("load %s bitmap failed\n", LISTBOX_PNT_PATH"set_bg.png");
		}


		listboxView = ListboxDataInit(listboxView);
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
		return 0;
	}
	case MSG_PAINT: {
		break;
	}
	case MSG_ERASEBKGND:
	{
		HDC hdc = (HDC) wParam;
		const RECT* clip = (const RECT*) lParam;
		BOOL fGetDC = FALSE;
		RECT rcTemp;

		if (hdc == 0) {
			hdc = GetClientDC(hWnd);
			fGetDC = TRUE;
		}
		if (clip) {
			rcTemp = *clip;
			ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
			ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
			IncludeClipRect(hdc, &rcTemp);
		}
		FillBoxWithBitmap(hdc, 0, 0, 0, 0, &bmpBg);
		if (fGetDC)
			ReleaseDC(hdc);
		return 0;
	}
	case MSG_LBUTTONDOWN: {
		break;
	}
	case MSG_LBUTTONUP: {
		break;
	}
	case MSG_DESTROY:
		if(listboxView_2)
		{
			listboxView_2 = ListboxDataUninit_2(listboxView_2);
		}

		if(listboxView)
		{
			listboxView = ListboxDataUninit(listboxView);
		}

		UnloadBitmap(&bmpBg);
		DestroyAllControls(hWnd);
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		break;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivityListbox(HWND hosting)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	RegisterListboxView();
	systeminit();
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityListProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = PIXEL_lightgray;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;


	hMainWnd = CreateMainWindow(&CreateInfo);

	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	UnregisterListboxView();

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

