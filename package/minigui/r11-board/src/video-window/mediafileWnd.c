#include "mediafileWnd.h"
#include "itemtableviewpiece.h"
#include "videowindow.h"
#include "media_file_play.h"
#define SLIDE_OUT_SPEED 16
int native_total_num;
char *filepath_list[100];
char *filename_list[100];
char *urlname_list[100];
int videoflag;
static HWND mediafileWnd = 0;
static BOOL isFullOpen = FALSE;
static void openMenu(HWND hwnd) {

        isFullOpen = FALSE;

        MSG msg;
        HDC sec_dc_active;

        sec_dc_active = GetSecondaryDC(hwnd);
        SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DONOTHING);

        /* get the content of the active main window */
        ShowWindow(hwnd, SW_SHOWNORMAL);
        /* wait for MSG_IDLE */
        while (GetMessage(&msg, hwnd)) {
                if (msg.message == MSG_IDLE)
                        break;
                DispatchMessage(&msg);
        }
        int distance = 0;
        while (distance <= WIDTH) {
                BitBlt(sec_dc_active, 0, 0, WIDTH, 480, HDC_SCREEN, 800 - distance, 0,
                                0);
                usleep(10000);
                distance += 60;
                if (distance >= WIDTH) {
                        BitBlt(sec_dc_active, 0, 0, WIDTH, 480, HDC_SCREEN, LX, 0, 0);
                }
        }

        /* restore to default behavior */
        SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
        isFullOpen = TRUE;
}
ItemInfo iteminfo[MAX_TABLEVIEW_NUM] = {
        {"", "111", " ", 1},
};
SectionInfo mysectioninfo[] ={
        { "a", TABLESIZE(iteminfo), &iteminfo},
};

void setup_item_native_file(void)
{
	int i = 0;
	int ret = create_media_list_player();
	if(ret != 0){
		printf("create media list fail\n");
	}
	native_total_num = media_get_total_num();
	for(i = 0; i < native_total_num; ++i){
		filename_list[i] = (char *) malloc(64);
		if(filename_list[i] == NULL){
			printf("malloc error\n");
		}
		filepath_list[i] = (char *) malloc(64);
		if(filepath_list[i] == NULL){
                        printf("malloc error\n");
                }
		media_get_next_name_path(filename_list[i], filepath_list[i]);
		iteminfo[i].text = filename_list[i];
	}

}
void setup_item_url_file(void)
{
	int i = 0;
	int ret = create_media_list_player();
        if(ret != 0){
                printf("create media list fail\n");
        }
	getMediaFile();
	for(i = 0; i < mediaFileNum; ++i){
		urlname_list[i] = (char *)malloc(64);
		if(urlname_list == NULL){
			printf("malloc error\n");
		}
		char *p = strrchr(mediaFile[i], '/');

		urlname_list[i] = strtok(p, "/");
                iteminfo[i].text = urlname_list[i];
        }
}
static mHotPiece* create_item_userpiece(const char* text)
{
        mPanelPiece* panel = NEWPIECE(mPanelPiece);
        RECT rc = {0, 0, 250, 50};
        _c(panel)->setRect(panel, &rc);
        txt_piece = (mHotPiece*)NEWPIECE(mTextPiece);

        SetRect(&rc, 0, 0, 380, 50);
        _c(txt_piece)->setRect(txt_piece, &rc);
        _c((mTextPiece*)txt_piece)->setProperty((mTextPiece *)txt_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)getLogFont(ID_FONT_SIMSUN_20));
        _c((mTextPiece*)txt_piece)->setProperty ((mTextPiece*)txt_piece, NCSP_LABELPIECE_ALIGN, NCS_ALIGN_LEFT);
        gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
        _c((mTextPiece*)txt_piece)->setProperty ((mTextPiece*)txt_piece, NCSP_TEXTPIECE_TEXTCOLOR, (WORD)lanse);
        _c(txt_piece)->setProperty(txt_piece, NCSP_LABELPIECE_LABEL, (DWORD)text);
        /*addContent set position*/
        _c(panel)->addContent(panel, txt_piece, 20, 15);
        return (mHotPiece*)panel;
}
static void mItemTableView_construct(mItemTableView *self, DWORD add_data)
{
        Class(mTableViewPiece).construct((mTableViewPiece*)self, add_data);
        self->data = mysectioninfo;
}

static void mItemTableView_destroy(mItemTableView *self)
{
        Class(mTableViewPiece).destroy((mTableViewPiece*)self);
}
static mTableViewItemPiece* mItemTableView_createItemForRow(mItemTableView* self, const mIndexPath* indexpath)
{
        RECT rect;
        RECT view_rc;
        ItemInfo* rowinfo = mysectioninfo[indexpath->section].rows + indexpath->row;
        mHotPiece* piece = create_item_userpiece(rowinfo->text);
        item = NEWPIECEEX(mTableViewItemPiece, 0);
        _c(self)->getRect(self, &view_rc);
        _c(piece)->getRect(piece, &rect);
        _c(item)->setUserPiece(item, piece);
        rect.right = RECTW(view_rc);
        _c(item)->setRect(item, &rect);
        return item;
}
static int mItemTableView_numberOfSections(mItemTableView* self)
{
        printf("TABLESIZE(mysectioninfo) = %d\n", TABLESIZE(mysectioninfo));
        return TABLESIZE(mysectioninfo);
}
static int mItemTableView_numberOfRowsInSection(mItemTableView* self, int section)
{
	if(videoflag == NATIVE_VIDEO){
        	return media_get_total_num();
	}else if(videoflag == URL_VIDEO){
		return mediaFileNum;
	}
}
static int mItemTableView_rowDidSelectAtIndexPath(mItemTableView *self, const mIndexPath* indexpath)
{
        printf("indexpath.section =%d, index =%d\n", indexpath->section, indexpath->row);
	SendMessage(GetVideoWnd(), MSG_PLAYVIDEO, NULL, indexpath->row);
	SendMessage(GetMediafileHwnd(), MSG_CLOSE, NULL, NULL);
}
BEGIN_MINI_CLASS(mItemTableView, mTableViewPiece)
        CLASS_METHOD_MAP(mItemTableView, construct)
        CLASS_METHOD_MAP(mItemTableView, createItemForRow)
        CLASS_METHOD_MAP(mItemTableView, numberOfSections)
        CLASS_METHOD_MAP(mItemTableView, numberOfRowsInSection)
        CLASS_METHOD_MAP(mItemTableView, rowDidSelectAtIndexPath)
        CLASS_METHOD_MAP(mItemTableView, destroy)
END_MINI_CLASS
static BOOL create_mediafile_item(HWND hwnd, mContainerCtrl *media_ctnr)
{
	printf("create_mediafile_item\n");
	int i = 0;
	RECT rc = {0, 0, 380, 460};
	table_index = table = (mItemTableView*)NEWPIECEEX(mItemTableView, NCS_TABLEVIEW_INDEXLOCATE_STYLE);
	_c(table)->setRect(table, &rc);
	if(videoflag == NATIVE_VIDEO){
		setup_item_native_file();
	}else if(videoflag == URL_VIDEO){
		setup_item_url_file();
	}
	_c(table)->reloadData(table);
	panel = NEWPIECE(mPanelPiece);
                _c(panel)->addContent(panel, (mHotPiece*)table, 0, 0);
                _c(panel)->setRect(panel, &rc);
                _c(media_ctnr)->setBody(media_ctnr, (mHotPiece*)panel);
	return TRUE;
}
static int mediafileProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	switch (message){
	case MSG_CREATE:{
		SendMessage(hwnd, MSG_MEDIA_ITEM, NULL, NULL);
		break;
	}
	case MSG_MEDIA_ITEM:{
		media_ctnr = (mContainerCtrl*)ncsCreateWindow(NCSCTRL_CONTAINERCTRL,
                "ContainerCtrl",
                WS_VISIBLE, 0, ID_MEDIAITEM,
                10, 10, 380, 460,
                hwnd,
                NULL, NULL, NULL, (DWORD)NULL);
		create_mediafile_item(hwnd, media_ctnr);
		//InvalidateRect(GetMediafileHwnd(), NULL, TRUE);
		break;
	}
	case MSG_DESTROY:{
                DestroyAllControls(hwnd);
                return 0;
        }
        case MSG_CLOSE: {
		if (isFullOpen){
			mediafileWnd = 0;
                	DestroyMainWindow(hwnd);
                	PostQuitMessage(0);
		}
                return 0;
        }
	}
	return DefaultMainWinProc(hwnd, message, wParam, lParam);
}
HWND GetMediafileHwnd() {
        return mediafileWnd;
}
int MediafilePopWin(HWND hosting, int video_flag) {
        if (mediafileWnd != 0)
                return -1;
        MSG Msg;
        MAINWINCREATE CreateInfo;
	videoflag = video_flag;
	MGNCS_INIT_CLASS(mItemTableView);
        MGNCS_INIT_CLASS(mIndexLocatePiece);
        CreateInfo.dwStyle = WS_NONE;
        CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;
        CreateInfo.spCaption = "";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = mediafileProc;
        CreateInfo.lx = LX;
        CreateInfo.ty = TY;
        CreateInfo.rx = RX;
        CreateInfo.by = BY;
        CreateInfo.iBkColor = PIXEL_lightgray;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hosting;
        mediafileWnd = CreateMainWindow(&CreateInfo);
        if (mediafileWnd == HWND_INVALID)
                return -1;
	//ShowWindow(mediafileWnd, SW_SHOWNORMAL);
        openMenu(mediafileWnd);
        while (GetMessage(&Msg, mediafileWnd)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
        }
	mGEffDeinit();
        MainWindowThreadCleanup(mediafileWnd);
        return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
