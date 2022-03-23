#ifndef _BOTTOMMENU_H_
#define _BOTTOMMENU_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "resource.h"

#include "button_view.h"
#include <unistd.h>
#include <string.h>
#include "sunxi_display_v2.h"
#include "mid_list.h"
#define MSG_PLAYER_PAINT (MSG_USER + 5)
#define _ID_SEEKBAR_TIMER 104
typedef struct {
	HWND BottomMenuHwnd;
	HWND BottomFatherHwnd;
	HWND startMenuHwnd;
	HWND volumeMenuHwnd;
	HWND volumebarMenuHwnd;
	HWND musicLeftHwnd;
	HWND musicMiddleHwnd;
	HWND musicRightHwnd;
	HWND pictureLeftHwnd;
	HWND pictureMiddleHwnd;
	HWND pictureRightHwnd;
	CvrRectType mainSize;
	CvrRectType startMenuSize;
	CvrRectType volumeMenuSize;
	CvrRectType volumeBarSize;
	CvrRectType musicLeftSize;
	CvrRectType musicMiddleSize;
	CvrRectType musicRightSize;
	CvrRectType pictureLeftSize;
	CvrRectType pictureRightSize;
	BITMAP volumeBmp;
	BITMAP musicLeftBmp;
	BITMAP musicMiddleBmp;
	BITMAP musicMiddleStartBmp;
	BITMAP musicRightBmp;
	int addID;
	int media_number;
	BOOL isfirstFile;
	BOOL islastFile;
	/* Indicates whether the video is turned on */
	/* to determine if tplayer_exit () */
	BOOL tplayerOpen;
	/* Indicates whether the video is playing, */
	/* to determine the video stop, hang and other status */
	BOOL isPlaying;
	int voice_number;
	BOOL voiceMax;
	int speed; /* Said seekbar sliding speed */
	BOOL onceflash;
} BottomMenuDataType;
#if 1
int pathFindFilename(char *path[64]);
int InitDisplayRect(int rectx, int recty, int rectw, int recth);
int startPlayer();
void pausePlayer();
void stopPlayer();
void playerseekto(int videoCurTime);
static void voice_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
static void music_start_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
static void musicleft_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
static void musicright_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
void startPicture();
static void picture_notif_proc(HWND hwnd, int id, int nc, DWORD add_data);
static void CreateBottomMenuBtn(HWND hWnd);
static void CreateBottomMusic(HWND hWnd);
static void CreateBottomPicture(HWND hwnd);
void InitAllResource();
int jpeg_dispClose();
#endif
#endif

