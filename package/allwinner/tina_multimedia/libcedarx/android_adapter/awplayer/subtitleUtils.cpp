/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : subtitleUtils.cpp
* Description : subtitle util function, including charset convert
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : create first version
*
*/

#include "subtitleUtils.h"
//#include "media/mediaplayerinfo.h"
#include "unicode/ucnv.h"
#include "unicode/ustring.h"
#include "cdx_log.h"

namespace android {

//* the CHARSET_XXX strings is defined in "av/include/media/mediaplayerinfo.h",
//* and it should be the same as strings defined in "base/media/java/android/media/MediaPlayer.java"
#define CHARSET_UNKNOWN                        "UNKNOWN"
#define CHARSET_BIG5                           "Big5"
#define CHARSET_BIG5_HKSCS                     "Big5-HKSCS"
#define CHARSET_BOCU_1                         "BOCU-1"
#define CHARSET_CESU_8                         "CESU-8"
#define CHARSET_CP864                          "cp864"
#define CHARSET_EUC_JP                         "EUC-JP"
#define CHARSET_EUC_KR                         "EUC-KR"
#define CHARSET_GB18030                        "GB18030"
#define CHARSET_GBK                            "GBK"
#define CHARSET_HZ_GB_2312                     "HZ-GB-2312"
#define CHARSET_ISO_2022_CN                    "ISO-2022-CN"
#define CHARSET_ISO_2022_CN_EXT                "ISO-2022-CN-EXT"
#define CHARSET_ISO_2022_JP                    "ISO-2022-JP"
#define CHARSET_ISO_2022_KR                    "ISO-2022-KR"
#define CHARSET_ISO_8859_1                     "ISO-8859-1"
#define CHARSET_ISO_8859_10                    "ISO-8859-10"
#define CHARSET_ISO_8859_13                    "ISO-8859-13"
#define CHARSET_ISO_8859_14                    "ISO-8859-14"
#define CHARSET_ISO_8859_15                    "ISO-8859-15"
#define CHARSET_ISO_8859_16                    "ISO-8859-16"
#define CHARSET_ISO_8859_2                     "ISO-8859-2"
#define CHARSET_ISO_8859_3                     "ISO-8859-3"
#define CHARSET_ISO_8859_4                     "ISO-8859-4"
#define CHARSET_ISO_8859_5                     "ISO-8859-5"
#define CHARSET_ISO_8859_6                     "ISO-8859-6"
#define CHARSET_ISO_8859_7                     "ISO-8859-7"
#define CHARSET_ISO_8859_8                     "ISO-8859-8"
#define CHARSET_ISO_8859_9                     "ISO-8859-9"
#define CHARSET_KOI8_R                         "KOI8-R"
#define CHARSET_KOI8_U                         "KOI8-U"
#define CHARSET_MACINTOSH                      "macintosh"
#define CHARSET_SCSU                           "SCSU"
#define CHARSET_SHIFT_JIS                      "Shift_JIS"
#define CHARSET_TIS_620                        "TIS-620"
#define CHARSET_US_ASCII                       "US-ASCII"
#define CHARSET_UTF_16                         "UTF-16"
#define CHARSET_UTF_16BE                       "UTF-16BE"
#define CHARSET_UTF_16LE                       "UTF-16LE"
#define CHARSET_UTF_32                         "UTF-32"
#define CHARSET_UTF_32BE                       "UTF-32BE"
#define CHARSET_UTF_32LE                       "UTF-32LE"
#define CHARSET_UTF_7                          "UTF-7"
#define CHARSET_UTF_8                          "UTF-8"
#define CHARSET_WINDOWS_1250                   "windows-1250"
#define CHARSET_WINDOWS_1251                   "windows-1251"
#define CHARSET_WINDOWS_1252                   "windows-1252"
#define CHARSET_WINDOWS_1253                   "windows-1253"
#define CHARSET_WINDOWS_1254                   "windows-1254"
#define CHARSET_WINDOWS_1255                   "windows-1255"
#define CHARSET_WINDOWS_1256                   "windows-1256"
#define CHARSET_WINDOWS_1257                   "windows-1257"
#define CHARSET_WINDOWS_1258                   "windows-1258"
#define CHARSET_X_DOCOMO_SHIFT_JIS_2007        "x-docomo-shift_jis-2007"
#define CHARSET_X_GSM_03_38_2000               "x-gsm-03.38-2000"
#define CHARSET_X_IBM_1383_P110_1999           "x-ibm-1383_P110-1999"
#define CHARSET_X_IMAP_MAILBOX_NAME            "x-IMAP-mailbox-name"
#define CHARSET_X_ISCII_BE                     "x-iscii-be"
#define CHARSET_X_ISCII_DE                     "x-iscii-de"
#define CHARSET_X_ISCII_GU                     "x-iscii-gu"
#define CHARSET_X_ISCII_KA                     "x-iscii-ka"
#define CHARSET_X_ISCII_MA                     "x-iscii-ma"
#define CHARSET_X_ISCII_OR                     "x-iscii-or"
#define CHARSET_X_ISCII_PA                     "x-iscii-pa"
#define CHARSET_X_ISCII_TA                     "x-iscii-ta"
#define CHARSET_X_ISCII_TE                     "x-iscii-te"
#define CHARSET_X_ISO_8859_11_2001             "x-iso-8859_11-2001"
#define CHARSET_X_JAVAUNICODE                  "x-JavaUnicode"
#define CHARSET_X_KDDI_SHIFT_JIS_2007          "x-kddi-shift_jis-2007"
#define CHARSET_X_MAC_CYRILLIC                 "x-mac-cyrillic"
#define CHARSET_X_SOFTBANK_SHIFT_JIS_2007      "x-softbank-shift_jis-2007"
#define CHARSET_X_UNICODEBIG                   "x-UnicodeBig"
#define CHARSET_X_UTF_16LE_BOM                 "x-UTF-16LE-BOM"
#define CHARSET_X_UTF16_OPPOSITEENDIAN         "x-UTF16_OppositeEndian"
#define CHARSET_X_UTF16_PLATFORMENDIAN         "x-UTF16_PlatformEndian"
#define CHARSET_X_UTF32_OPPOSITEENDIAN         "x-UTF32_OppositeEndian"
#define CHARSET_X_UTF32_PLATFORMENDIAN         "x-UTF32_PlatformEndian"

const char* strTextCodecFormats[] =
{
    CHARSET_UNKNOWN     ,
    CHARSET_BIG5        ,
    CHARSET_BIG5_HKSCS  ,
    CHARSET_BOCU_1      ,
    CHARSET_CESU_8      ,
    CHARSET_CP864       ,
    CHARSET_EUC_JP      ,
    CHARSET_EUC_KR      ,
    CHARSET_GB18030     ,
    CHARSET_GBK         ,
    CHARSET_HZ_GB_2312  ,
    CHARSET_ISO_2022_CN ,
    CHARSET_ISO_2022_CN_EXT ,
    CHARSET_ISO_2022_JP ,
    CHARSET_ISO_2022_KR ,
    CHARSET_ISO_8859_1  ,
    CHARSET_ISO_8859_10 ,
    CHARSET_ISO_8859_13 ,
    CHARSET_ISO_8859_14 ,
    CHARSET_ISO_8859_15 ,
    CHARSET_ISO_8859_16 ,
    CHARSET_ISO_8859_2  ,
    CHARSET_ISO_8859_3  ,
    CHARSET_ISO_8859_4  ,
    CHARSET_ISO_8859_5  ,
    CHARSET_ISO_8859_6  ,
    CHARSET_ISO_8859_7  ,
    CHARSET_ISO_8859_8  ,
    CHARSET_ISO_8859_9  ,
    CHARSET_KOI8_R      ,
    CHARSET_KOI8_U      ,
    CHARSET_MACINTOSH   ,
    CHARSET_SCSU        ,
    CHARSET_SHIFT_JIS   ,
    CHARSET_TIS_620     ,
    CHARSET_US_ASCII    ,
    CHARSET_UTF_16      ,
    CHARSET_UTF_16BE    ,
    CHARSET_UTF_16LE    ,
    CHARSET_UTF_32      ,
    CHARSET_UTF_32BE    ,
    CHARSET_UTF_32LE    ,
    CHARSET_UTF_7       ,
    CHARSET_UTF_8       ,
    CHARSET_WINDOWS_1250 ,
    CHARSET_WINDOWS_1251 ,
    CHARSET_WINDOWS_1252 ,
    CHARSET_WINDOWS_1253 ,
    CHARSET_WINDOWS_1254 ,
    CHARSET_WINDOWS_1255 ,
    CHARSET_WINDOWS_1256 ,
    CHARSET_WINDOWS_1257 ,
    CHARSET_WINDOWS_1258 ,
    CHARSET_X_DOCOMO_SHIFT_JIS_2007 ,
    CHARSET_X_GSM_03_38_2000        ,
    CHARSET_X_IBM_1383_P110_1999    ,
    CHARSET_X_IMAP_MAILBOX_NAME     ,
    CHARSET_X_ISCII_BE  ,
    CHARSET_X_ISCII_DE  ,
    CHARSET_X_ISCII_GU  ,
    CHARSET_X_ISCII_KA  ,
    CHARSET_X_ISCII_MA  ,
    CHARSET_X_ISCII_OR  ,
    CHARSET_X_ISCII_PA  ,
    CHARSET_X_ISCII_TA  ,
    CHARSET_X_ISCII_TE  ,
    CHARSET_X_ISO_8859_11_2001 ,
    CHARSET_X_JAVAUNICODE      ,
    CHARSET_X_KDDI_SHIFT_JIS_2007     ,
    CHARSET_X_MAC_CYRILLIC            ,
    CHARSET_X_SOFTBANK_SHIFT_JIS_2007 ,
    CHARSET_X_UNICODEBIG              ,
    CHARSET_X_UTF_16LE_BOM            ,
    CHARSET_X_UTF16_OPPOSITEENDIAN    ,
    CHARSET_X_UTF16_PLATFORMENDIAN    ,
    CHARSET_X_UTF32_OPPOSITEENDIAN    ,
    CHARSET_X_UTF32_PLATFORMENDIAN    ,
    NULL
};


static int SubtitleUtilsConvertUnicode(char*               pOutBuf,
                                       int                 nOutBufSize,
                                       SubtitleItem*       pSubItem,
                                       const char*         pDefaultTextFormatName)
{
    const char *pInBuf = pSubItem->pText;
    ESubtitleTextFormat eInputTextFormat = pSubItem->eTextFormat;
    int len = pSubItem->nTextLength;

    int             charset;
    const char*  enc;;
    unsigned int subTextLen;
    UErrorCode   status;
    UConverter*  conv;
    UConverter*  utf8Conv;
    const char*  src;
    int          targetLength;
    char*        target;

    //*if subtitle decoder know charset,use it ,or use app's setting.
    switch(eInputTextFormat)
    {
        case SUBTITLE_TEXT_FORMAT_UTF8:
             enc = CHARSET_UTF_8;
             break;

        case SUBTITLE_TEXT_FORMAT_GB2312:
             enc = CHARSET_HZ_GB_2312;
             break;

        case SUBTITLE_TEXT_FORMAT_UTF16LE:
             enc = CHARSET_UTF_16LE;
             break;

        case SUBTITLE_TEXT_FORMAT_UTF16BE:
             enc = CHARSET_UTF_16BE;
             break;

        case SUBTITLE_TEXT_FORMAT_UTF32LE:
             enc = CHARSET_UTF_32LE;
             break;

        case SUBTITLE_TEXT_FORMAT_UTF32BE:
             enc = CHARSET_UTF_32BE;
             break;

        case SUBTITLE_TEXT_FORMAT_BIG5:
             enc = CHARSET_BIG5;
             break;

        case SUBTITLE_TEXT_FORMAT_GBK:
             enc = CHARSET_GBK;
             break;

        case SUBTITLE_TEXT_FORMAT_ANSI:  //can not match,set to "UTF-8"
             enc = CHARSET_UTF_8;
             break;

        default:
             enc = pDefaultTextFormatName;
             break;
    }

    status = U_ZERO_ERROR;

    conv = ucnv_open(enc, &status);
    if(U_FAILURE(status))
    {
        logw("could not create UConverter for %s\n", enc);
        memset(pOutBuf, 0, nOutBufSize);
        return -1;
    }

    utf8Conv = ucnv_open("UTF-8", &status);
    if (U_FAILURE(status))
    {
        logw("could not create UConverter for UTF-8\n");
        memset(pOutBuf, 0, nOutBufSize);
        ucnv_close(conv);
        return -1;
    }

    //*first we need to untangle the utf8 and convert it back to the original bytes
    // since we are reducing the length of the string, we can do this in place
    src = pInBuf;
    targetLength = len * 3 + 1;

    //*now convert from native encoding to UTF-8
    if(targetLength > nOutBufSize)
        targetLength = nOutBufSize;

    memset(pOutBuf, 0, nOutBufSize);
    target = &pOutBuf[0];

    ucnv_convertEx(utf8Conv,
                   conv,
                   &target,
                   (const char*)target + targetLength,
                   &src,
                   (const char*)src + len,
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   true,
                   true,
                   &status);

    if (U_FAILURE(status))
    {
        logw("ucnv_convertEx failed: %d\n", status);
        memset(pOutBuf, 0, nOutBufSize);
    }

    subTextLen = target - (char*)pOutBuf;
    logv("CedarXTimedText::convertUniCode src = %s, target = %s\n, subTextLen = %d",
        pInBuf, pOutBuf, subTextLen);

    ucnv_close(conv);
    ucnv_close(utf8Conv);

    return OK;
}


// Store the timing and text sample in a Parcel.
// The Parcel will be sent to MediaPlayer.java through event, and will be
// parsed in TimedText.java.
int SubtitleUtilsFillTextSubtitleToParcel(Parcel*       parcelDst,
                                          SubtitleItem* pSubItem,
                                          int           nSubtitleID,
                                          const char*   strDefaultTextFormatName,
                                          SubtitleDisPos* subDisPos)
{
    int          nFontStyleIdx;
    int          nSubStyleFlag;
    unsigned int nTextLen;
    int          nPts;

#define MAX_OUTPUT_TEXT_SIZE (1024)
    char         strOutText[MAX_OUTPUT_TEXT_SIZE];

    SubtitleUtilsConvertUnicode(strOutText,
                                MAX_OUTPUT_TEXT_SIZE,
                                pSubItem,
                                strDefaultTextFormatName);

    nFontStyleIdx = 0;
    nSubStyleFlag = 0;
    nTextLen      = strlen((char*)strOutText);

    parcelDst->writeInt32(KEY_LOCAL_SETTING);
    parcelDst->writeInt32(KEY_START_TIME);
    //* pts in unit of us, write out pts in unit of ms.
    parcelDst->writeInt32((int)(pSubItem->nPts/1000));

    parcelDst->writeInt32(KEY_STRUCT_TEXT);
    parcelDst->writeInt32(nTextLen);   //* write the size of the text sample
    parcelDst->writeInt32(nTextLen);   //* write the text sample as a byte array
    parcelDst->write((char*)strOutText, nTextLen);

    //*set subtitleID
    parcelDst->writeInt32(KEY_SUBTITLE_ID);
    parcelDst->writeInt32(nSubtitleID);

    if(pSubItem->eAlignment != 0)
    {
        parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_SUBDISPPOS);
        //*write text subtitle's position area.
        parcelDst->writeInt32(pSubItem->eAlignment);

        parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_SCREENRECT);
        //*write text subtitle's whole screen rect.
        parcelDst->writeInt32(0);
        parcelDst->writeInt32(0);
        parcelDst->writeInt32(pSubItem->nReferenceVideoHeight);
        parcelDst->writeInt32(pSubItem->nReferenceVideoWidth);

        if(pSubItem->nStartX != subDisPos->nStartX ||
            pSubItem->nStartY != subDisPos->nStartY ||
            pSubItem->nEndY != subDisPos->nEndY ||
            pSubItem->nEndX != subDisPos->nEndX)
        {
                parcelDst->writeInt32(KEY_STRUCT_TEXT_POS);
                //*write text subtitle's position Rect.
                parcelDst->writeInt32(pSubItem->nStartY);
                parcelDst->writeInt32(pSubItem->nStartX);
                parcelDst->writeInt32(pSubItem->nEndY);
                parcelDst->writeInt32(pSubItem->nEndX);
                subDisPos->nStartX = pSubItem->nStartX;
                subDisPos->nStartY = pSubItem->nStartY;
                subDisPos->nEndX = pSubItem->nEndX;
                subDisPos->nEndY = pSubItem->nEndY;
        }
    }

    if(pSubItem->nFontSize!=0 && pSubItem->nPrimaryColor!=0)
    {
        //*convert strFontName,bBold,bItalic,bUnderlined
        if(!strcmp(pSubItem->strFontName, SUBTITLE_FONT_NAME_EPILOG))
            nFontStyleIdx = 0;
        else if(!strcmp(pSubItem->strFontName, SUBTITLE_FONT_NAME_VERDANA))
            nFontStyleIdx = 1;
        else if(!strcmp(pSubItem->strFontName, SUBTITLE_FONT_NAME_GEORGIA))
            nFontStyleIdx = 2;
        else if(!strcmp(pSubItem->strFontName, SUBTITLE_FONT_NAME_ARIAL))
            nFontStyleIdx = 3;
        else if(!strcmp(pSubItem->strFontName, SUBTITLE_FONT_NAME_TIMES_NEW_ROMAN))
            nFontStyleIdx = 4;
        else
            nFontStyleIdx = -1;

        //* In the absence of any bits set in flags, the text
        //* is plain. Otherwise, 1: bold, 2: italic, 4: underline
        if(pSubItem->bBold)
            nSubStyleFlag |= 1<<0;
        if(pSubItem->bItalic)
            nSubStyleFlag |= 1<<1;
        if(pSubItem->bUnderlined)
            nSubStyleFlag |= 1<<2;

        parcelDst->writeInt32(KEY_STRUCT_STYLE_LIST);
        parcelDst->writeInt32(KEY_FONT_ID);
        parcelDst->writeInt32(nFontStyleIdx);
        parcelDst->writeInt32(KEY_FONT_SIZE);
        parcelDst->writeInt32((int)pSubItem->nFontSize);
        parcelDst->writeInt32(KEY_TEXT_COLOR_RGBA);
        parcelDst->writeInt32((int)pSubItem->nPrimaryColor);
        parcelDst->writeInt32(KEY_STYLE_FLAGS);
        parcelDst->writeInt32(nSubStyleFlag);
    }

    return 0;
}


// Store the timing and text sample in a Parcel.
// The Parcel will be sent to MediaPlayer.java through event, and will be
// parsed in TimedText.java.
int SubtitleUtilsFillBitmapSubtitleToParcel(Parcel* parcelDst,
                                SubtitleItem* pSubItem, int nSubtitleID)
{
    int nBufSize;

    parcelDst->writeInt32(KEY_LOCAL_SETTING);
    parcelDst->writeInt32(KEY_START_TIME);
    //* pts in unit of us, write out pts in unit of ms.
    parcelDst->writeInt32((int)(pSubItem->nPts/1000));

    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_BMP);
    //* write the pixel format of bmp subtitle
    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_PIXEL_FORMAT);
#if 0
    parcelDst->writeInt32(pSubItem->ePixelFormat); //* 0: ARGB, 1: YUV, defined in sdecoder.h
                                                //* but currently only support ARGB.
#else
    //* application only support PIXEL_FORMAT_RGBA_8888 pixel format, this value will be ignored
    //* at frameworks/base/media/java/android/media/TimeText.java::parseParcel().
    //* so we write zero here.
    if(pSubItem->ePixelFormat != SUBTITLE_PIXEL_FORMAT_ARGB)
    {
        loge("subtitle pixel format is not ARGB, can not handle.");
        abort();
    }
    parcelDst->writeInt32(0);
#endif

    //* write the width of bmp subtitle
    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_PICWIDTH);
    parcelDst->writeInt32(pSubItem->nBitmapWidth);

    //* write the height of bmp subtitle
    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_PICHEIGHT);
    parcelDst->writeInt32(pSubItem->nBitmapHeight);

    //* write the reference video width of bmp subtitle
    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_REFERENCE_VIDEO_WIDTH);
    parcelDst->writeInt32(pSubItem->nReferenceVideoWidth);

    //* write the reference video height of bmp subtitle
    parcelDst->writeInt32(KEY_STRUCT_AWEXTEND_REFERENCE_VIDEO_HEIGHT);
    parcelDst->writeInt32(pSubItem->nReferenceVideoHeight);

    //* write the size of the text sample, we only process PIXEL_FORMAT_RGBA_8888 format currently.
    nBufSize = pSubItem->nBitmapWidth*pSubItem->nBitmapHeight*4;
    parcelDst->writeInt32(nBufSize);

    //* write the argb buffer as an int32_t array
    parcelDst->writeInt32(nBufSize/4);
    parcelDst->write(pSubItem->pBitmapData, nBufSize);

    //* set subtitleID
    parcelDst->writeInt32(KEY_SUBTITLE_ID);
    parcelDst->writeInt32(nSubtitleID);

    return 0;
}

};
