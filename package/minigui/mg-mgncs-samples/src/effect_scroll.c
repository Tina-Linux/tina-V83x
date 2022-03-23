#include <stdio.h>
#include <mgncs4touch/picture-preview.h>
#include <mgncs/mgncs.h>
#include <mgeff/mgeff.h>

#include <mgncs4touch/mgncs4touch.h>
#include <mgncs4touch/mtouchdebug.h>

static PicPreview *picpreview;
RECT rect={0,0,400,200};
static const char* src_picture[] =
{
    "./res/1.jpg",
    "./res/2.jpg",
    "./res/3.jpg",
    "./res/4.jpg",
    "./res/5.jpg",
};

static HDC createDCByPicture (HDC hdc, const char *path, int w, int h)
{
    HDC dc;
    BITMAP bmp;

    if (LoadBitmap (hdc, &bmp, path))
        return HDC_INVALID;

    if (w == 0)
        w = bmp.bmWidth;
    if (h == 0)
        h = bmp.bmHeight;

    dc = CreateCompatibleDCEx (hdc, w, h);

    FillBoxWithBitmap (dc, 0, 0, w, h, &bmp);

    UnloadBitmap (&bmp);

    return dc;
}

static PicPreview *PicturePreviewInit(PicPreview *picpreview, HDC hdc)
{
	int i = 0;
	picpreview  = (PicPreview *)malloc(sizeof(PicPreview));
	if(NULL == picpreview){
		printf("malloc picpreview data error\n");
	}
	memset((void *)picpreview, 0, sizeof(PicPreview));
	picpreview->picture_num = 5;
	picpreview->animation_type = MGEFF_EFFECTOR_SCROLL;
	picpreview->duration = 1000;
	for (i = 0; i < picpreview->picture_num; ++i) {
		picpreview->s_pic_paths[i] = src_picture[i];
        	picpreview->g_hdc_pic[i] = createDCByPicture (hdc, picpreview->s_pic_paths[i], rect.right, rect.bottom);
        	if (picpreview->g_hdc_pic[i] == HDC_INVALID) {
			free(picpreview);
			picpreview = NULL;
            		return NULL;
        	}
    	}
	return picpreview;
}
static int MainWindowProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (message) {
	case MSG_CREATE: {
		hdc = GetDC (hWnd);
		picpreview = PicturePreviewInit(picpreview, hdc);
		ReleaseDC(hdc);
		CreateWindowEx(PICPREVIEW, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 0, 0, 0, 0, rect.right,
                                rect.bottom, hWnd, (DWORD) picpreview);
		ReleaseDC (hdc);
		break;
	}
	case MSG_DESTROY:{
		if(picpreview != NULL){
			free(picpreview);
			picpreview = NULL;
		}
                DestroyAllControls(hWnd);
                return 0;
        }
	case MSG_CLOSE:{
		DestroyMainWindow(hWnd);
                PostQuitMessage(0);
		break;
	}
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}
int MiniGUIMain(int argc, const char* argv[])
{
        MSG Msg;
        MAINWINCREATE CreateInfo;
	HWND hMainwnd;
#ifdef _MGRM_PROCESSES
        JoinLayer(NAME_DEF_LAYER , "" , 0 , 0);
#endif
	ncsInitialize();
        ncs4TouchInitialize();
	mGEffInit();
	RegisterPicPreview();
	CreateInfo.dwStyle = WS_NONE;
        CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
        CreateInfo.spCaption = "";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = 0; /*GetSystemCursor (0);*/
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = MainWindowProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = 800;
        CreateInfo.by = 480;
        CreateInfo.iBkColor = PIXEL_lightwhite;
        CreateInfo.hHosting = HWND_DESKTOP;
        CreateInfo.dwAddData = 0;
	hMainwnd = CreateMainWindow(&CreateInfo);
        if (hMainwnd == HWND_INVALID)
                return -1;
        ShowWindow(hMainwnd, SW_SHOWNORMAL);
        while (GetMessage(&Msg, hMainwnd)) {
                DispatchMessage(&Msg);
        }
	ncs4TouchUninitialize();
        ncsUninitialize();
	UnregisterPicPreview();
        MainWindowThreadCleanup(hMainwnd);
	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
