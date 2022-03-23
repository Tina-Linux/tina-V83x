#ifndef VIDEO_WINDOW_H_
#define VIDEO_WINDOW_H_
#include <unistd.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "washing_res_cn.h"
#include "VideoWnd.h"
#define ID_NATIVE 301
#define ID_URL 302
#define NATIVE_VIDEO 0
#define URL_VIDEO 1
static BITMAP native_video;
static BITMAP url_video;
int VIDEO_FLAG;
#endif
