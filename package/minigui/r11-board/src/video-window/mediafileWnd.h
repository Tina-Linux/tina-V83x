#ifndef MEDIA_FILE_H_
#define MEDIA_FILE_H_

#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mgncs/mgncs.h>
#include <mgncs/mobject.h>
#include <mgeff/mgeff.h>

#include <mgncs4touch/mgncs4touch.h>
#include <mgncs4touch/mtouchdebug.h>
#include "itemtableviewpiece.h"

#define LX 416
#define TY 0
#define RX 800
#define BY 480
#define WIDTH 384
#define MSG_MEDIA_ITEM 301
#define MAX_TABLEVIEW_NUM 128
#define ID_MEDIAITEM 304
static mItemTableView * table;
static mItemTableView *table_index;
mPanelPiece* panel;
mHotPiece *txt_piece;
mTableViewItemPiece* item;
mContainerCtrl* media_ctnr;
typedef struct _itemInfo{
    char *key;
    char *text;
    char *picture;
    int index;
}ItemInfo;

typedef struct _sectionInfo{
    char* index;
    int num;
    ItemInfo* rows;
}SectionInfo;
HWND GetMediafileHwnd();
int MediafilePopWin(HWND hosting, int video_flag);
#endif
