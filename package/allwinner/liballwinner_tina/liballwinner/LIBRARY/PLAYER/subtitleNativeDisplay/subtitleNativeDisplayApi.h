#ifndef _CDX_SUBRENDER_H__
#define _CDX_SUBRENDER_H__

#if 0
enum
{
	SUB_RENDER_ALIGN_NONE		= 0,
	SUB_RENDER_HALIGN_LEFT		= 1,
	SUB_RENDER_HALIGN_CENTER	= 2,
	SUB_RENDER_HALIGN_RIGHT		= 3,
	SUN_RENDER_HALIGN_MASK		= 0x0000000f,
	SUB_RENDER_VALIGN_TOP		= (1 << 4),
	SUB_RENDER_VALIGN_CENTER	= (2 << 4),
	SUB_RENDER_VALIGN_BOTTOM	= (3 << 4),
	SUN_RENDER_VALIGN_MASK		= 0x000000f0
};
#endif


enum
{
	SUB_RENDER_STYLE_NONE		    = 0,
	SUB_RENDER_STYLE_BOLD		    = 1 << 0,
	SUB_RENDER_STYLE_ITALIC         = 1 << 1,
	SUB_RENDER_STYLE_UNDERLINE      = 1 << 2,
	SUB_RENDER_STYLE_STRIKETHROUGH  = 1 << 3
};

enum
{
	SUB_CHARSET_BITMAP              = -2,
	SUB_CHARSET_UNKNOWN			= -1,
	SUB_CHARSET_BIG5                = 0,
	SUB_CHARSET_BIG5_HKSCS          = 1,
	SUB_CHARSET_BOCU_1              = 2,
	SUB_CHARSET_CESU_8              = 3,
	SUB_CHARSET_CP864               = 4,
	SUB_CHARSET_EUC_JP              = 5,
	SUB_CHARSET_EUC_KR              = 6,
	SUB_CHARSET_GB18030             = 7,
	SUB_CHARSET_GBK                 = 8,
	SUB_CHARSET_HZ_GB_2312          = 9,
	SUB_CHARSET_ISO_2022_CN         = 10,
	SUB_CHARSET_ISO_2022_CN_EXT     = 11,
	SUB_CHARSET_ISO_2022_JP         = 12,
	SUB_CHARSET_ISO_2022_KR         = 13,
	SUB_CHARSET_ISO_8859_1          = 14,
	SUB_CHARSET_ISO_8859_10         = 15,
	SUB_CHARSET_ISO_8859_13         = 16,
	SUB_CHARSET_ISO_8859_14         = 17,
	SUB_CHARSET_ISO_8859_15         = 18,
	SUB_CHARSET_ISO_8859_16         = 19,
	SUB_CHARSET_ISO_8859_2          = 20,
	SUB_CHARSET_ISO_8859_3          = 21,
	SUB_CHARSET_ISO_8859_4          = 22,
	SUB_CHARSET_ISO_8859_5          = 23,
	SUB_CHARSET_ISO_8859_6          = 24,
	SUB_CHARSET_ISO_8859_7          = 25,
	SUB_CHARSET_ISO_8859_8          = 26,
	SUB_CHARSET_ISO_8859_9          = 27,
	SUB_CHARSET_KOI8_R              = 28,
	SUB_CHARSET_KOI8_U              = 29,
	SUB_CHARSET_MACINTOSH           = 30,
	SUB_CHARSET_SCSU                = 31,
	SUB_CHARSET_SHIFT_JIS           = 32,
	SUB_CHARSET_TIS_620             = 33,
	SUB_CHARSET_US_ASCII            = 34,
	SUB_CHARSET_UTF_16              = 35,
	SUB_CHARSET_UTF_16BE            = 36,
	SUB_CHARSET_UTF_16LE            = 37,
	SUB_CHARSET_UTF_32              = 38,
	SUB_CHARSET_UTF_32BE            = 39,
	SUB_CHARSET_UTF_32LE            = 40,
	SUB_CHARSET_UTF_7               = 41,
	SUB_CHARSET_UTF_8               = 42,
	SUB_CHARSET_WINDOWS_1250        = 43,
	SUB_CHARSET_WINDOWS_1251        = 44,
	SUB_CHARSET_WINDOWS_1252        = 45,
	SUB_CHARSET_WINDOWS_1253        = 46,
	SUB_CHARSET_WINDOWS_1254        = 47,
	SUB_CHARSET_WINDOWS_1255        = 48,
	SUB_CHARSET_WINDOWS_1256        = 49,
	SUB_CHARSET_WINDOWS_1257        = 50,
	SUB_CHARSET_WINDOWS_1258        = 51,
	SUB_CHARSET_X_DOCOMO_SHIFT_JIS_2007        = 52,
	SUB_CHARSET_X_GSM_03_38_2000               = 53,
	SUB_CHARSET_X_IBM_1383_P110_1999           = 54,
	SUB_CHARSET_X_IMAP_MAILBOX_NAME            = 55,
	SUB_CHARSET_X_ISCII_BE                     = 56,
	SUB_CHARSET_X_ISCII_DE                     = 57,
	SUB_CHARSET_X_ISCII_GU                     = 58,
	SUB_CHARSET_X_ISCII_KA                     = 59,
	SUB_CHARSET_X_ISCII_MA                     = 60,
	SUB_CHARSET_X_ISCII_OR                     = 61,
	SUB_CHARSET_X_ISCII_PA                     = 62,
	SUB_CHARSET_X_ISCII_TA                     = 63,
	SUB_CHARSET_X_ISCII_TE                     = 64,
	SUB_CHARSET_X_ISO_8859_11_2001             = 65,
	SUB_CHARSET_X_JAVAUNICODE                  = 66,
	SUB_CHARSET_X_KDDI_SHIFT_JIS_2007          = 67,
	SUB_CHARSET_X_MAC_CYRILLIC                 = 68,
	SUB_CHARSET_X_SOFTBANK_SHIFT_JIS_2007      = 69,
	SUB_CHARSET_X_UNICODEBIG                   = 70,
	SUB_CHARSET_X_UTF_16LE_BOM                 = 71,
	SUB_CHARSET_X_UTF16_OPPOSITEENDIAN         = 72,
	SUB_CHARSET_X_UTF16_PLATFORMENDIAN         = 73,
	SUB_CHARSET_X_UTF32_OPPOSITEENDIAN         = 74,
	SUB_CHARSET_X_UTF32_PLATFORMENDIAN         = 75
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
enum
{
   SUB_RENDER_EFFECT_NONE          = 0,
   SUB_RENDER_EFFECT_SCROLL_UP     = 1,
   SUB_RENDER_EFFECT_SCROLL_DOWN   = 2,
   SUB_RENDER_EFFECT_BANNER_LTOR   = 3,
   SUB_RENDER_EFFECT_BANNER_RTOL   = 4,
   SUB_RENDER_EFFECT_MOVE          = 5,
   SUB_RENDER_EFFECT_KARAOKE       = 6,

};

enum
{
	SUB_RENDER_ALIGN_NONE		= 0,
	SUB_RENDER_HALIGN_LEFT		= 1,
	SUB_RENDER_HALIGN_CENTER	= 2,
	SUB_RENDER_HALIGN_RIGHT		= 3,
	SUN_RENDER_HALIGN_MASK		= 0x0000000f,
	SUB_RENDER_VALIGN_TOP		= (1 << 4),
	SUB_RENDER_VALIGN_CENTER	= (2 << 4),
	SUB_RENDER_VALIGN_BOTTOM	= (3 << 4),
	SUN_RENDER_VALIGN_MASK		= 0x000000f0
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

enum SUB_ENCODING_TYPE
{
    SUB_ENCODING_UNKNOWN = SUB_CHARSET_UNKNOWN,  // unknown subtitle encode type
    SUB_ENCODING_NONE    = SUB_CHARSET_UNKNOWN,  // none subtitle bitstream
    SUB_ENCODING_BITMAP  = SUB_CHARSET_BITMAP,    // subtitle encode type is bitmap, 4 colors supported.
    SUB_ENCODING_UTF8    = SUB_CHARSET_UTF_8,      // subtitle encode type is UTF8
    SUB_ENCODING_GB2312  = SUB_CHARSET_HZ_GB_2312,    // subtitle encode type is GB2312
    SUB_ENCODING_UTF16LE = SUB_CHARSET_UTF_16LE,   // subtitle encode type is UTF16-LE
    SUB_ENCODING_UTF16BE = SUB_CHARSET_UTF_16BE,   // subtitle encode type is UTF16-BE
    SUB_ENCODING_UTF32LE = SUB_CHARSET_UTF_32LE,   // subtitle encode type is UTF32-LE
    SUB_ENCODING_UTF32BE = SUB_CHARSET_UTF_32BE,   // subtitle encode type is UTF32-BE
    SUB_ENCODING_BIG5    = SUB_CHARSET_BIG5,      // subtitle encode type is BIG5
    SUB_ENCODING_GBK     = SUB_CHARSET_GBK,       // subtitle encode type is GBK
    SUB_ENCODING_ANSI    = SUB_CHARSET_UTF_8,      // subtitle encode type is text, unknown character encode type
    SUB_ENCODING_
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

int	SubRenderCreate();
int	SubRenderDestory();
int	SubRenderDraw(sub_item_inf *sub_info);
int	SubRenderShow();
int	SubRenderHide(unsigned    int  systemTime, int* hasSubShowFlag);

int		SubRenderSetZorderTop();
int     SubRenderSetZorderBottom();

#endif
