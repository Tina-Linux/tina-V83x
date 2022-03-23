#include "WifiWindow.h"
#include <mgncs/mgncs.h>
#include "resource.h"
#include "washing_machine.h"
#include "headbar_view.h"
#include "bottombar_view.h"
#include "wifiactivity.h"
#include <mgncs4touch/mtouchdebug.h>
#include <mgncs/mgncs.h>
#include "light_menu.h"
#include "msimpletableviewpiece.h"

static int ID = -1;
int HOSTING = -1;
HWND WifiWnd = 0;

HDC hdc;
int status_wifilist = STATUS_OFF;
int password_error_status = FALSE;
int connect_error = FALSE;
extern pthread_stop_refreash;
extern HWND button;
static void Register_res(void)
{
        LoadBitmapFromFile(HDC_SCREEN, &wifi_wait, "/usr/res/menu/wifi_wait.png");
	LoadBitmap(HDC_SCREEN, &bmp_left, "/usr/res/menu/wifi_signal_strength3.png");
}
ItemInfo listinfo[MAX_TABLEVIEW_NUM] = {
	{"", "", "/usr/res/menu/wifi_signal_strength3.png", 1},
};
SectionInfo sectioninfo[] ={
        { "a", TABLESIZE(listinfo), &listinfo},
};
int get_wifi_list_num()
{
	return scan_number;
}
void setup_item_wifi_name_nofirst(){
	int i = 0;
	if(bool_connect == 0){
                listinfo[0].text = scan_info_arry[listinfo_index].ssid;
                listinfo[0].picture = "/usr/res/menu/wifi_signal_strength3.png";
                listinfo[0].index = 0;
                for(i = 0; i < listinfo_index; ++i){
                        listinfo[i+1].text = scan_info_arry[i].ssid;
                        listinfo[i+1].picture = "/usr/res/menu/wifi_signal_strength3.png";
                        listinfo[i+1].index = i;
                }
                for(i = listinfo_index+1; i < wifi_list_num; ++i){
                        listinfo[i].text = scan_info_arry[i].ssid;
                        listinfo[i].picture = "/usr/res/menu/wifi_signal_strength3.png";
                        listinfo[i].index = i;
                }
                strcpy(connect_wifiname, scan_info_arry[listinfo_index].ssid);
                strcat(connect_wifiname, "(已连接)");
                listinfo[0].text = connect_wifiname;

        }
	else if(bool_connect != 0){
		int wifi_list_num = get_wifi_list_num();
		for(i = 0; i < wifi_list_num; ++i){
                listinfo[i].text = scan_info_arry[i].ssid;
                listinfo[i].picture = "/usr/res/menu/wifi_signal_strength3.png";
                listinfo[i].index = i;
		}
        }
	if(text_connecting == STATUS_ON){
                listinfo[0].text = scan_info_arry[listinfo_index].ssid;
                listinfo[0].picture = "/usr/res/menu/wifi_signal_strength3.png";
                listinfo[0].index = 0;
                for(i = 0; i < listinfo_index; ++i){
                        listinfo[i+1].text = scan_info_arry[i].ssid;
                        listinfo[i+1].picture = "/usr/res/menu/wifi_signal_strength3.png";
                        listinfo[i+1].index = i;
                }
                for(i = listinfo_index+1; i < wifi_list_num; ++i){
                        listinfo[i].text = scan_info_arry[i].ssid;
                        listinfo[i].picture = "/usr/res/menu/wifi_signal_strength3.png";
                        listinfo[i].index = i;
                }
                strcpy(connect_wifiname, scan_info_arry[listinfo_index].ssid);
                strcat(connect_wifiname, "(正在连接中...)");
                listinfo[0].text = connect_wifiname;
        }
}
void setup_item_wifi_name()
{
	int i = 0;
	wifi_list_num = get_wifi_list_num();
	for(i = 0; i < wifi_list_num; ++i){
		listinfo[i].text = scan_info_arry[i].ssid;
		listinfo[i].picture = "/usr/res/menu/wifi_signal_strength3.png";
		listinfo[i].index = i;
	}
}
char *get_select_wifiname(int select_index)
{
	return listinfo[select_index].text;
}
static mHotPiece* create_item_userpiece(const char* text, const char* picture)
{
	/* image piece */
	mHotPiece *img_piece = (mHotPiece*)NEWPIECE(mImagePiece);
	/* userPiece */
	mPanelPiece* panel = NEWPIECE(mPanelPiece);
	RECT rc = {0, 0, ITEM_W, ITEM_H};
	_c(panel)->setRect(panel, &rc);
	/*write content to the box*/
	rc.right = bmp_left.bmWidth;
	rc.bottom = bmp_left.bmHeight;
	_c(img_piece)->setRect(img_piece, &rc);
	_c(img_piece)->setProperty(img_piece, NCSP_IMAGEPIECE_IMAGE, (DWORD)&bmp_left);
	/* label piece */
	txt_piece = (mHotPiece*)NEWPIECE(mTextPiece);
	SetRect(&rc, 0, 0, ITEM_W, ITEM_H);
	_c(txt_piece)->setRect(txt_piece, &rc);
	_c((mTextPiece*)txt_piece)->setProperty((mTextPiece *)txt_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)getLogFont(ID_FONT_SIMSUN_20));
	_c((mTextPiece*)txt_piece)->setProperty ((mTextPiece*)txt_piece, NCSP_LABELPIECE_ALIGN, NCS_ALIGN_LEFT);
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	_c((mTextPiece*)txt_piece)->setProperty ((mTextPiece*)txt_piece, NCSP_TEXTPIECE_TEXTCOLOR, (WORD)lanse);
	_c(txt_piece)->setProperty(txt_piece, NCSP_LABELPIECE_LABEL, (DWORD)text);
	/*addContent set position*/
	_c(panel)->addContent(panel, img_piece, 600, -2);
	_c(panel)->addContent(panel, txt_piece, 5, 15);
	return (mHotPiece*)panel;
}
static void mSimpleTableView_construct(mSimpleTableView *self, DWORD add_data)
{
        Class(mTableViewPiece).construct((mTableViewPiece*)self, add_data);
	self->data = sectioninfo;
}

static void mSimpleTableView_destroy(mSimpleTableView *self)
{
	//UnloadBitmap(&bmp_left);
	Class(mTableViewPiece).destroy((mTableViewPiece*)self);
}

static mTableViewItemPiece* mSimpleTableView_createItemForRow(mSimpleTableView* self, const mIndexPath* indexpath)
{
	RECT rect;
	RECT view_rc;
	ItemInfo* rowinfo = sectioninfo[indexpath->section].rows + indexpath->row;
	mHotPiece* piece = create_item_userpiece(rowinfo->text, rowinfo->picture);
	item = NEWPIECEEX(mTableViewItemPiece, 0);
	_c(self)->getRect(self, &view_rc);
	_c(piece)->getRect(piece, &rect);
	_c(item)->setUserPiece(item, piece);
	rect.right = RECTW(view_rc);
	_c(item)->setRect(item, &rect);
	return item;
}
static int mSimpleTableView_numberOfSections(mSimpleTableView* self)
{
	//printf("TABLESIZE(sectioninfo) = %d\n", TABLESIZE(sectioninfo));
	return TABLESIZE(sectioninfo);
}
static int mSimpleTableView_numberOfRowsInSection(mSimpleTableView* self, int section)
{
	//printf("sectioninfo[section].num =%d\n", sectioninfo[section].num);
	return get_wifi_list_num();
}

static int mSimpleTableView_rowDidSelectAtIndexPath(mSimpleTableView *self, const mIndexPath* indexpath)
{
	printf("indexpath.section =%d, index =%d\n", indexpath->section, indexpath->row);
	wifiinfo.selectindex = indexpath->row;
	wifiselectindex = indexpath->row;
	wifiinfo.wifi_name = get_select_wifiname(wifiinfo.selectindex);
	listinfo_index = listinfo[wifiselectindex].index;
	//wifiinfo.wifi_password = get_select_wifipassword(wifiinfo.selectindex);
	pthread_stop_refreash = STATUS_ON;
	SendMessage(WifiWnd, MSG_WIFI_PASSWORD, 0, 0);
}
static void mSimpleTableView_onCommitInsertRowAtIndexPath(mTableViewPiece* self, const mIndexPath* indexpath)
{
	_c(self)->insertRowAtIndexPath(self, indexpath, NCS_TABLEVIEW_ANIMATIONZOOM);
}

static void mSimpleTableView_onCommitDeleteRowAtIndexPath(mTableViewPiece* self, const mIndexPath* indexpath)
{
	_c(self)->deleteRowAtIndexPath(self, indexpath, NCS_TABLEVIEW_ANIMATIONZOOM);
}
BEGIN_MINI_CLASS(mSimpleTableView, mTableViewPiece)
        CLASS_METHOD_MAP(mSimpleTableView, construct)
        CLASS_METHOD_MAP(mSimpleTableView, createItemForRow)
        CLASS_METHOD_MAP(mSimpleTableView, numberOfSections)
        CLASS_METHOD_MAP(mSimpleTableView, numberOfRowsInSection)
	CLASS_METHOD_MAP(mSimpleTableView, rowDidSelectAtIndexPath)
	CLASS_METHOD_MAP(mSimpleTableView, onCommitInsertRowAtIndexPath)
	CLASS_METHOD_MAP(mSimpleTableView, onCommitDeleteRowAtIndexPath)
        CLASS_METHOD_MAP(mSimpleTableView, destroy)
END_MINI_CLASS
static BOOL create_wifilist_item(HWND hwnd, mContainerCtrl *ctnr)
{
	mTextPiece* text;
	if(status_switchbutton == STATUS_ON){
		RECT rc = {0, 0, ITEM_W, 350};
		table_index = table = (mSimpleTableView*)NEWPIECEEX(mSimpleTableView, NCS_TABLEVIEW_INDEXLOCATE_STYLE);
		_c(table)->setRect(table, &rc);
		setup_item_wifi_name();
		//_c(table)->setSeparatorColor(table, 0xFFFF00FF);
		_c(table)->reloadData(table);
		KillTimer(GetWifiWnd(), TIMER_ID);
		panel = NEWPIECE(mPanelPiece);
		_c(panel)->addContent(panel, (mHotPiece*)table, 0, 0);
		_c(panel)->setRect(panel, &rc);
		_c(ctnr)->setBody(ctnr, (mHotPiece*)panel);
	}else if(status_switchbutton == STATUS_OFF){
	}
	return TRUE;
}

static void *wifi_search_list(HWND hwnd)
{
	EnableWindow(switch_button, FALSE);
//	pthread_mutex_lock(&mutex);
        while(1){
        if(pthread_stop_refreash == STATUS_ON){
		pthread_cond_wait(&cond, &mutex);
        }else if(pthread_stop_refreash == STATUS_OFF){
	        if(listinfo != NULL){
	        	KillTimer(GetWifiWnd(), TIMER_ID);
	        	HWND parent = GetParent(hwnd);
						scan_waln(wifi_interface);
		if(first_scan == TRUE){
			SendMessage(GetWifiWnd(), MSG_CREATE_ITEM, NULL, NULL);
			first_scan = FALSE;
		}else{
			SendMessage(GetWifiWnd(), MSG_REFASH_ITEM, NULL, NULL);
		}
	        InvalidateRect(GetWifiWnd(), NULL, TRUE);
//		pthread_mutex_unlock(&mutex);
		EnableWindow(switch_button, TRUE);
	        sleep(15);
	        }
        }
        }
	pthread_exit(NULL);
}
static void *wifi_disconnect_ap(void)
{
	SendMessage(GetWifiWnd(), MSG_DISCONNECT_AP, NULL, NULL);
	pthread_exit(NULL);
}
void switchbutton_notif_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
	HWND parent = GetParent(hwnd);
	if(nc == NCSN_SWB_STATUSCHANGED){
		if(id == ID_SWBUTTON){
                        if(add_data == 0){
	                       	/*off button，close the window for searching wifi*/
				status_switchbutton = STATUS_OFF;
				status_wifilist = STATUS_OFF;
				pthread_stop_refreash = STATUS_ON;
				KillTimer(GetWifiWnd(), TIMER_ID);
				KillTimer(headbarwnd, rotate_TIMERID);
				ctnr_wnd_close = STATUS_OFF;
				ShowWindow(ctnr->hwnd, SW_HIDE);
				if(bool_connect == 0){
					bool_connect = -1;
					pthread_create(&pthread_disconnect_ap, NULL, &wifi_disconnect_ap, NULL);
				}
				InvalidateRect(headbarwnd, NULL, TRUE);
				InvalidateRect(GetWifiWnd(), NULL, TRUE);
                        }else{
				switch_on_num++;
                                /*on button, open the window to searching wifi*/
				status_switchbutton = STATUS_ON;
				status_wifilist = STATUS_OFF;
				SetTimer(GetWifiWnd(), TIMER_ID, 1);
				pthread_stop_refreash = STATUS_OFF;
				wifi_list_num = get_wifi_list_num();
				if(switch_on_num == 1){
					pthread_create(&pthread_ID, NULL, &wifi_search_list, hwnd);
				 }else{
					if(bool_connect != 0){
						SendMessage(GetWifiWnd(), MSG_CREATE_ITEM, NULL, NULL);
					}
					ShowWindow(ctnr->hwnd, SW_SHOW);
				}
                        }
                }
        }
}

static int WifiWinProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		HWND wifioff_prompt;
		HWND switch_wifioff;
        case MSG_CREATE: {
		SetTimer(hwnd, TIMER_ID, 1);
		return 0;
	}
	case MSG_CREATE_ITEM:{
		ctnr = (mContainerCtrl*)ncsCreateWindow(NCSCTRL_CONTAINERCTRL,
                "ContainerCtrl",
                WS_VISIBLE, 0, ID_WIFILIST,
                30, 100, ITEM_W, 350,
                hwnd,
                NULL, NULL, NULL, (DWORD)NULL);
		if(status_switchbutton == STATUS_ON){
			create_wifilist_item(hwnd, ctnr);
		}
		return 0;
	}
	case MSG_REFASH_ITEM:{
		RECT rc = {0, 0, ITEM_W, 350};
		//setup_item_wifi_name();
		setup_item_wifi_name_nofirst();
		_c(table)->reloadData(table);
		KillTimer(GetWifiWnd(), TIMER_ID);
		panel = NEWPIECE(mPanelPiece);
		_c(panel)->addContent(panel, (mHotPiece*)table, 0, 0);
		_c(panel)->setRect(panel, &rc);
		_c(ctnr)->setBody(ctnr, (mHotPiece*)panel);
		return 0;
	}
	case MSG_MOVE_TOP:{
		_c(table)->moveViewport(table, 0, 0);
                return 0;
        }
	case MSG_DISCONNECT_AP:{
		wlan_disconnect(wifi_interface, event_label);
		return 0;
	}
	case MSG_TIMER:{
		if(status_switchbutton == STATUS_ON){
	 		if(password_error_status == TRUE){
	 			KillTimer(hwnd, TIMER_PASSWORD_ERROR);
	 			ShowWindow(password_errorwnd, SW_HIDE);
				password_error_status = FALSE;
				EnableWindow(GetContainerWnd(), TRUE);
			}else if(connect_error == TRUE){
				KillTimer(hwnd, TIMER_PASSWORD_ERROR);
				ShowWindow(connect_errorwnd, SW_HIDE);
				connect_error = FALSE;
				EnableWindow(GetContainerWnd(), TRUE);
			}else{
				if(ctnr_wnd_close == STATUS_OFF){

				}else if(ctnr_wnd_close == STATUS_ON){
	         		angle += SPINNING_SPEED;
	                 	if (angle >= 46080) {
	                         	angle = 0;
	                	}
				InvalidateRect(hwnd, NULL, TRUE);
				}
	 		}
		}else{
			KillTimer(hwnd, TIMER_ID);
			wifi_icon_change = TRUE;
			headBarData->winDes = WIN_DES_WIFIWND;
			headbarwnd = HeadBarViewInit(hwnd, 0, headBarData);
			SendMessage(headbarwnd, MSG_UPDATETIME, NULL, NULL);
			SendMessage(headbarwnd, MSG_CHAGE_HEADBAR, NULL, NULL);
		}
		return 0;
	}
	case MSG_PAINT: {
		hdc = BeginPaint(hwnd);
		if(status_switchbutton == STATUS_ON){
			if(status_wifilist == STATUS_OFF){
        			RotateBitmap(hdc, &wifi_wait, 350, 250, angle);
			}
			else if(status_wifilist == STATUS_ON){
			}
		}
		EndPaint(hwnd, hdc);
		return 0;
	}

	case MSG_WIFI_PASSWORD:{
		light_menu_flag = TRUE;
		first_connect = 0;
		light_menu(hwnd, 2);
		return 0;
	}
	case MSG_PASSWORD_ERROR:{
		password_error_status = TRUE;
		SetTimer(hwnd, TIMER_PASSWORD_ERROR, 100);
		password_errorwnd = CreateWindowEx(CTRL_STATIC, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTER | SS_NOTIFY, NULL,
                ID_WIFIBACK, 250, 200, 350, 50,
                                 hwnd, (DWORD) 0);
		SetWindowFont(password_errorwnd, getLogFont(ID_FONT_SIMSUN_30));
		SetWindowText (password_errorwnd, WIFI_PASSWORD_ERROR);
		return 0;
	}
	case MSG_CONNECT_ERROR:{
		connect_error = TRUE;
                SetTimer(hwnd, TIMER_PASSWORD_ERROR, 100);

                connect_errorwnd = CreateWindowEx(CTRL_STATIC, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTER | SS_NOTIFY, NULL,
                ID_WIFIBACK, 250, 200, 350, 50,
                                 hwnd, (DWORD) 0);
                SetWindowFont(connect_errorwnd, getLogFont(ID_FONT_SIMSUN_30));
                SetWindowText (connect_errorwnd, WIFI_CONNECT_ERROR);
                return 0;
	}
	case MSG_WIFI_READON:{
		EnableWindow(switch_button, FALSE);
		wifi_readonwnd = 0;
		wifi_readonwnd = CreateWindowEx(CTRL_STATIC, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTER | SS_NOTIFY, NULL,
                ID_WIFIBACK, 250, 200, 350, 50,
                                 hwnd, (DWORD) 0);
		SetWindowFont(wifi_readonwnd, getLogFont(ID_FONT_SIMSUN_30));
                SetWindowText (wifi_readonwnd, WIFI_READON);
		return 0;
	}
	/*add speed meter*/
	case MSG_MOUSEMOVE:
	case MSG_LBUTTONUP:
	case MSG_LBUTTONDOWN:
		SpeedMeterMessageHandler(NULL, message, LOSWORD(lParam), HISWORD(lParam), (DWORD)wParam);
		break;
	case MSG_DESTROY:{
		unloadBitMap(&wifi_wait);
		unloadBitMap(&bmp_left);
		DestroyAllControls(hwnd);
		return 0;
	}
	case MSG_CLOSE: {
		DestroyMainWindow(hwnd);
        	PostQuitMessage(0);
		return 0;
	}
	}
	 return DefaultMainWinProc(hwnd, message, wParam, lParam);
}
HWND GetContainerWnd()
{
	return ctnr->hwnd;
}
HWND GetWifiWnd()
{
	return WifiWnd;
}

int WifiWindow(HWND hosting, int id)
{
#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER, NULL, 0, 0);
#endif
	HOSTING = hosting;
	ID = id;
	MSG Msg;
        MAINWINCREATE CreateInfo;
	WifiWnd = 0;
	CreateInfo.dwStyle = WS_NONE;
        CreateInfo.dwExStyle = WS_EX_TROUNDCNS | WS_EX_BROUNDCNS | WS_EX_TOPMOST
                        | WS_EX_AUTOSECONDARYDC;
        CreateInfo.spCaption = "";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = 0; /*GetSystemCursor(0);*/
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = WifiWinProc;
	CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = 800;
        CreateInfo.by = 480;
	CreateInfo.iBkColor = PIXEL_lightgray;
        CreateInfo.dwAddData = 0;
       	CreateInfo.hHosting = hosting;
	Register_res();
	MGNCS_INIT_CLASS(mSimpleTableView);
	MGNCS_INIT_CLASS(mIndexLocatePiece);
	WifiWnd = CreateMainWindow(&CreateInfo);
	if (WifiWnd == HWND_INVALID){
                return -1;
	}
	ShowWindow(WifiWnd, SW_SHOWNORMAL);
	HWND have_focus = GetFocusChild(WifiWnd);
	while (GetMessage(&Msg, WifiWnd)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
        }
	mGEffDeinit();
        MainWindowThreadCleanup(WifiWnd);
        return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
