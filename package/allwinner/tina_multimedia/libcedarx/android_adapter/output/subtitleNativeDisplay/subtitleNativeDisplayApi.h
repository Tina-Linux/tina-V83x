/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleNativeDisplayApi.h
 * Description : NativeDisplay
 * Author  : jiangyue <jiangyue@allwinnertech.com>
 * Date    : 2016/04/13
 * Comment : first version
 *
 */

#ifndef _CDX_SUBRENDER_H__
#define _CDX_SUBRENDER_H__

enum {
   SUB_RENDER_STYLE_NONE          = 0,
   SUB_RENDER_STYLE_BOLD          = 1 << 0,
   SUB_RENDER_STYLE_ITALIC         = 1 << 1,
   SUB_RENDER_STYLE_UNDERLINE      = 1 << 2,
   SUB_RENDER_STYLE_STRIKETHROUGH  = 1 << 3
};

enum SUB_ALIGNMENT
{
    SUB_ALIGNMENT_UNKNOWN = -1,
    SUB_ALIGNMENT_MIDDLE  = 0,
    SUB_ALIGNMENT_LEFT    = 1,
    SUB_ALIGNMENT_RIGHT   = 2,
    SUB_ALIGNMENT_
};

enum SUB_FONTSTYLE
{
    SUB_FONT_UNKNOWN         = -1,
    SUB_FONT_EPILOG          = 0,
    SUB_FONT_VERDANA         = 1,
    SUB_FONT_GEORGIA         = 2,
    SUB_FONT_ARIAL           = 3,
    SUB_FONT_TIMES_NEW_ROMAN = 4,
    SUB_FONT_
};

enum {
   SUB_RENDER_EFFECT_NONE          = 0,
   SUB_RENDER_EFFECT_SCROLL_UP     = 1,
   SUB_RENDER_EFFECT_SCROLL_DOWN   = 2,
   SUB_RENDER_EFFECT_BANNER_LTOR   = 3,
   SUB_RENDER_EFFECT_BANNER_RTOL   = 4,
   SUB_RENDER_EFFECT_MOVE          = 5,
   SUB_RENDER_EFFECT_KARAOKE       = 6,

};

enum {
   SUB_RENDER_ALIGN_NONE        = 0,
   SUB_RENDER_HALIGN_LEFT       = 1,
   SUB_RENDER_HALIGN_CENTER    = 2,
   SUB_RENDER_HALIGN_RIGHT    = 3,
   SUN_RENDER_HALIGN_MASK      = 0x0000000f,
   SUB_RENDER_VALIGN_TOP      = (1 << 4),
   SUB_RENDER_VALIGN_CENTER   = (2 << 4),
   SUB_RENDER_VALIGN_BOTTOM   = (3 << 4),
   SUN_RENDER_VALIGN_MASK      = 0x000000f0
};


enum SUBTITLE_DISP_POSITION
{
    SUB_DISPPOS_DEFAULT   = 0,
    SUB_DISPPOS_BOT_LEFT  = SUB_RENDER_VALIGN_BOTTOM+SUB_RENDER_HALIGN_LEFT,
    SUB_DISPPOS_BOT_MID   = SUB_RENDER_VALIGN_BOTTOM+SUB_RENDER_HALIGN_CENTER,
    SUB_DISPPOS_BOT_RIGHT = SUB_RENDER_VALIGN_BOTTOM+SUB_RENDER_HALIGN_RIGHT,
    SUB_DISPPOS_MID_LEFT  = SUB_RENDER_VALIGN_CENTER+SUB_RENDER_HALIGN_LEFT,
    SUB_DISPPOS_MID_MID   = SUB_RENDER_VALIGN_CENTER+SUB_RENDER_HALIGN_CENTER,
    SUB_DISPPOS_MID_RIGHT = SUB_RENDER_VALIGN_CENTER+SUB_RENDER_HALIGN_RIGHT,
    SUB_DISPPOS_TOP_LEFT  = SUB_RENDER_VALIGN_TOP   +SUB_RENDER_HALIGN_LEFT,
    SUB_DISPPOS_TOP_MID   = SUB_RENDER_VALIGN_TOP   +SUB_RENDER_HALIGN_CENTER,
    SUB_DISPPOS_TOP_RIGHT = SUB_RENDER_VALIGN_TOP   +SUB_RENDER_HALIGN_RIGHT,
    SUB_DISPPOS_
};

enum SUB_MODE
{
    SUB_MODE_TEXT   = 0,
    SUB_MODE_BITMAP = 1,
    SUB_MODE_
};

#if 1
#define SUB_MAX_KARAOKE_EFFECT_NUM 16

typedef struct SUBTITLE_KARAKO_EFFECT_INF
{
    unsigned int  karakoSectionNum;
    unsigned int  karaKoSectionStartTime[SUB_MAX_KARAOKE_EFFECT_NUM];
    unsigned int  karaKoSectionLen[SUB_MAX_KARAOKE_EFFECT_NUM];
    unsigned int  karaKoSectionColor[SUB_MAX_KARAOKE_EFFECT_NUM];
}sub_karako_effect_inf;
#endif
typedef struct SUBTITLE_ITEM_INF       //the information of each subItem
{
    unsigned char   subMode;            //0: SUB_MODE_TEXT; 1: SUB_MODE_BITMAP
    int  startx;             // the invalid value is -1
    int  starty;             // the invalid value is -1
    int  endx;               // the invalid value is -1
    int  endy;               // the invalid value is -1
    int  subDispPos;         // the disply position of the subItem
    int  startTime;          // the start display time of the subItem
    int  endTime;            // the end display time of the subItem
    unsigned char*  subTextBuf;         // the data buffer of the text subtitle
    unsigned char*  subBitmapBuf;       // the data buffer of the bitmap subtitle
    unsigned int  subTextLen;         // the length of the text subtitle
    unsigned int  subPicWidth;        // the width of the bitmap subtitle
    unsigned int  subPicHeight;       // the height of the bitmap subtitle
    unsigned char   alignment;          // the alignment of the subtitle
    signed char   encodingType;       // the encoding tyle of the text subtitle
    void*    nextSubItem;        // the information of the next subItem
    unsigned char   dispBufIdx;         // the diplay index of the sub

    unsigned int  subScaleWidth;      // the scaler width of the bitmap subtitle
    unsigned int  subScaleHeight;     // the scaler height of the bitmap subtitle

    unsigned char   subHasFontInfFlag;
    unsigned char   fontStyle;          // the font style of the text subtitle
    signed char   fontSize;           // the font size of the text subtile
    unsigned int  primaryColor;
    unsigned int  secondaryColor;
  //unsigned int  outlineColour;
  //unsigned int  backColour;
    unsigned int  subStyle;          // the bold flag,the italic flag, or the underline flag
    int  subEffectFlag;
    unsigned int  effectStartxPos;
    unsigned int  effectEndxPos;
    unsigned int  effectStartyPos;
    unsigned int  effectEndyPos;
    unsigned int  effectTimeDelay;
    sub_karako_effect_inf *subKarakoEffectInf;
}sub_item_inf;


enum SUB_DATA_STRUCT
{
    SUB_DATA_STRUCT_ARGB = 0,
    SUB_DATA_STRUCT_RGBA = 1,
    SUB_DATA_STRUCT_BGRA = 2,
    SUB_DATA_STRUCT_ABGR = 3,
    SUB_DATA_STRUCT_
};

int    SubRenderCreate();
int    SubRenderDestory();
int    SubRenderDraw(sub_item_inf *sub_info);
int    SubRenderShow();
int    SubRenderHide(unsigned    int  systemTime, int* hasSubShowFlag);

int      SubRenderSetZorderTop();
int     SubRenderSetZorderBottom();

#endif
