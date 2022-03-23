/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : subtitleUtils.h
* Description : subtitle util function, including charset convert
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first version
*
*/

#ifndef __SUBTITLE_UTILS_H
#define __SUBTITLE_UTILS_H
#include <binder/Parcel.h>

#include "player.h"

namespace android {

enum {
    // These keys must be in sync with the keys in TimedText.java
    KEY_DISPLAY_FLAGS                 = 1, // int32_t
    KEY_STYLE_FLAGS                   = 2, // int32_t
    KEY_BACKGROUND_COLOR_RGBA         = 3, // int32_t
    KEY_HIGHLIGHT_COLOR_RGBA          = 4, // int32_t
    KEY_SCROLL_DELAY                  = 5, // int32_t
    KEY_WRAP_TEXT                     = 6, // int32_t
    KEY_START_TIME                    = 7, // int32_t
    KEY_STRUCT_BLINKING_TEXT_LIST     = 8, // List<CharPos>
    KEY_STRUCT_FONT_LIST              = 9, // List<Font>
    KEY_STRUCT_HIGHLIGHT_LIST         = 10, // List<CharPos>
    KEY_STRUCT_HYPER_TEXT_LIST        = 11, // List<HyperText>
    KEY_STRUCT_KARAOKE_LIST           = 12, // List<Karaoke>
    KEY_STRUCT_STYLE_LIST             = 13, // List<Style>
    KEY_STRUCT_TEXT_POS               = 14, // TextPos
    KEY_STRUCT_JUSTIFICATION          = 15, // Justification
    KEY_STRUCT_TEXT                   = 16, // Text

    KEY_SUBTITLE_ID                   = 17, //subtitle id

    KEY_STRUCT_AWEXTEND_BMP           = 50, // bmp subtitle such as idxsub and pgs.
    KEY_STRUCT_AWEXTEND_PIXEL_FORMAT  = 51,  //PIXEL_FORMAT_RGBA_8888
    KEY_STRUCT_AWEXTEND_PICWIDTH      = 52, // bmp subtitle item's width
    KEY_STRUCT_AWEXTEND_PICHEIGHT     = 53, // bmp subtitle item's height
    KEY_STRUCT_AWEXTEND_SUBDISPPOS    = 54, // text subtitle's position, SUB_DISPPOS_BOT_LEFT
    KEY_STRUCT_AWEXTEND_SCREENRECT    = 55, // text subtitle's position need a whole area as a ref.
    // when multi subtitle show the same time, such as ssa,
    //we need to tell app which subtitle need to hide.
    KEY_STRUCT_AWEXTEND_HIDESUB       = 56,
    KEY_STRUCT_AWEXTEND_REFERENCE_VIDEO_WIDTH  = 57, // bmp subtitle reference video width
    KEY_STRUCT_AWEXTEND_REFERENCE_VIDEO_HEIGHT = 58, // bmp subtitle reference video height

    KEY_GLOBAL_SETTING                = 101,
    KEY_LOCAL_SETTING                 = 102,
    KEY_START_CHAR                    = 103,
    KEY_END_CHAR                      = 104,
    KEY_FONT_ID                       = 105,
    KEY_FONT_SIZE                     = 106,
    KEY_TEXT_COLOR_RGBA               = 107,
};

extern const char* strTextCodecFormats[];

typedef struct SubtitleDisPos
{
    int                  nStartX;
    int                  nStartY;
    int                  nEndX;
    int                  nEndY;
}SubtitleDisPos;

// Store the timing and text sample in a Parcel.
// The Parcel will be sent to MediaPlayer.java through event, and will be
// parsed in TimedText.java.
int SubtitleUtilsFillTextSubtitleToParcel(Parcel*       parcelDst,
                                          SubtitleItem* pSubItem,
                                          int           nSubtitleID,
                                          const char*   strDefaultTextFormatName,
                                          SubtitleDisPos* subDisPos);

// Store the timing and text sample in a Parcel.
// The Parcel will be sent to MediaPlayer.java through event, and will be
// parsed in TimedText.java.
int SubtitleUtilsFillBitmapSubtitleToParcel(Parcel* parcelDst,
                                                    SubtitleItem* pSubItem,
                                                    int nSubtitleID);

};

#endif
