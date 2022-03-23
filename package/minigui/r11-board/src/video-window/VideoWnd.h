#ifndef VIDEOWND_H_
#define VIDEOWND_H_
#include <unistd.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "washing_res_cn.h"
#include "headbar_view.h"
#define MSG_PLAYVIDEO 305
HWND video_headbarwnd;
void media_notify_proc(HWND hwnd, int id, int nc, DWORD add_data);
HWND GetVideoWnd(void);
int VideoWnd(HWND hosting, int video_flag, HeadBarView *myHeadBarData);
#endif
