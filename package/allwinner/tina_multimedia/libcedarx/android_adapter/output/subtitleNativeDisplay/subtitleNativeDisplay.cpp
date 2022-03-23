/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleNativeDisplay.cpp
 * Description : subtitle NativeDisplay with skia
 * Author  : jiangyue <jiangyue@allwinnertech.com>
 * Date    : 2016/04/13
 * Comment : first version
 *
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "subtitleNativeDisplay"
#include <cdx_log.h>
#include "cdx_config.h"

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <utils/Errors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <binder/IInterface.h>
#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <ui/Region.h>
#include <ui/Rect.h>
#include <ui/PixelFormat.h>
#include <EGL/egl.h>
#include <SkCanvas.h>
#include <SkBitmap.h>
#include <SkImageEncoder.h>
#include <SkXfermode.h>
#include <SkRegion.h>
#include <SkTypeface.h>
#include <SkGlyphCache.h>
#include <SkUtils.h>
#include <SkAutoKern.h>
#include <cutils/properties.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include "unicode/ucnv.h"
#include "unicode/ustring.h"
#ifdef __cplusplus
}
#endif
#include "subtitleNativeDisplay.h"
#include "ui/DisplayInfo.h"
#include "native_window.h"
#include "SkDevice.h"
#include "sdecoder.h"
#include "subtitleControl.h"
#include "outputCtrl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  BASELAYER                  8  //5, 3, higher number layer is on topper.
#define  TOPBASELAYER            8  //5, 3
#define  BOTTOMBASELAYER         8
#define  LAYER_MULTIPLIER         10000
#define  LAYER_OFFSET            1000
#define  TRANSPARENT_COLOR         0x00050107
#define  MAX_TEXTLINE            3
#define  MAX_FONTHEIGHT            36
#define  SUB_DISPLAY            0
#define  FONT_SIZE_UNIT            16
#define  DEFAULT_TEXT_SIZE          24

#define  SUB_SHOW_NEW_VALID         1
#define  SUB_SHOW_OLD_VALID         2
#define  SUB_SHOW_INVALID           0
#define  SUB_HAS_NONE_COLOR         0
#define  SUB_HAS_DEFINED_COLOR      1
#define  SUB_HAS_DEFINED_FONTSIZE   1
#define  SUB_HAS_NONE_FONTSIZE      0
#define  SUB_HAS_DEFINED_STYLE      1
#define  SUB_HAS_NONE_STYLE         0
#define  SUB_INIT_SET_FONT_INFO     1
#define  SUB_USER_SET_FONT_INFO     2

#define PROP_SUBTITLE_MAXFONTSIZE_KEY  "media.stagefright.maxsubfont"
#define SAVE_BITMAP_TO_PNG 0

#if SAVE_BITMAP_TO_PNG
static bool save(const char filename[], SkBitmap &bitmap)
{
    if(!SkImageEncoder::EncodeFile(filename, bitmap, SkImageEncoder::kPNG_Type,
        SkImageEncoder::kDefaultQuality))
    {
        printf("save png %s failed \n",filename);
    }
    printf("%s saved ok\n",filename);
    return true;
}
#endif

namespace android
{

static SkBitmap::Config convertPixelFormat(PixelFormat ePixelFormat)
{
    switch (ePixelFormat)
    {
        case PIXEL_FORMAT_RGBX_8888:
        {
            return SkBitmap::kARGB_8888_Config;
        }
        case PIXEL_FORMAT_RGBA_8888:
        {
            return SkBitmap::kARGB_8888_Config;
        }
        case PIXEL_FORMAT_RGBA_4444:
        {
            return SkBitmap::kARGB_4444_Config;
        }
        case PIXEL_FORMAT_RGB_565:
        {
            return SkBitmap::kRGB_565_Config;
        }
        default:
        {
            return SkBitmap::kNo_Config;
        }
    }
}

static inline int is_ws_(int ch)
{
   return !((ch - 1) >> 5);
}

static inline int is_break(int c1,int c2)
{
    if(c1 == 0x0D && c2 == 0x0A)
    {
        return true;
    }

    return false;
}

static size_t subLineBreak(const char cText[], const char cStop[], const SkPaint& paint,
                       SkScalar margin,size_t *drawCount, int specialEffectFlag,
                       SkScalar* lineWidth)
{
    const char*       pStart =NULL;
#if ((CONF_ANDROID_MAJOR_VER == 4) && (CONF_ANDROID_SUB_VER == 2))
    SkAutoGlyphCache    ac(paint, NULL);
#else
    SkAutoGlyphCache    ac(paint, NULL, NULL);
#endif
    SkGlyphCache*       cache = NULL;
    SkFixed             w = 0;
    SkFixed             limit=0;
    SkAutoKern          autokern;
    size_t            count = 0;
    const char* pWord_start = NULL;
    int         prevWS       = true;
    int         isbreak      = false;
    int         forbidBreakFlag = 0;
    const char* pPrevText    = NULL;
    const char* breaktext   = NULL;
    SkUnichar   uni;
    int         currWS;

    SkUnichar   nextuni;

    pStart = cText;
    cache = ac.getCache();
    limit = SkScalarToFixed(margin);
    pWord_start = cText;

    forbidBreakFlag = ((specialEffectFlag==SUB_RENDER_EFFECT_BANNER_LTOR)||
                       (specialEffectFlag==SUB_RENDER_EFFECT_BANNER_RTOL)||
                       (specialEffectFlag==SUB_RENDER_EFFECT_KARAOKE));

    while (cText < cStop)
    {
        pPrevText    = cText;
        uni          = SkUTF8_NextUnichar(&cText);
        currWS       = is_ws_(uni);
        const SkGlyph&  mGlyph = cache->getUnicharMetrics(uni);

        if (!currWS && prevWS)
        {
            pWord_start = pPrevText;
        }

        prevWS = currWS;

        if(uni == 0x0D)
        {
            breaktext          = cText;
            nextuni = SkUTF8_NextUnichar(&breaktext);
            if(nextuni == 0x0A)
            {
                isbreak = true;
            }
            else
            {
                count += SkUTF8_CountUTF8Bytes(pPrevText);
            }
        }
        else
        {
            count += SkUTF8_CountUTF8Bytes(pPrevText);
        }

        w += autokern.adjust(mGlyph) + mGlyph.fAdvanceX;

        //if ((w > limit) || isbreak)
        if(isbreak ||((w>limit)&&(forbidBreakFlag==0)))
        {
            if (currWS) // eat the rest of the whitespace
            {
                *drawCount = count;
                while (cText < cStop && is_ws_(SkUTF8_ToUnichar(cText)))
                {
                    cText += SkUTF8_CountUTF8Bytes(cText);
                }
            }
            else    // backup until a whitespace (or 1 char)
            {
                if (pWord_start == pStart)
                {
                    if (pPrevText > pStart)
                    {
                        cText = pPrevText;
                    }
                }
                else
                {
                    cText = pWord_start;
                }

                *drawCount = cText - pStart;
            }

            break;
        }
    }

    *lineWidth = (w>limit)? ((SkScalar)(limit>>16)): ((SkScalar)(w>>16));

    if(cText >= cStop)
    {
        *drawCount = count;
    }
    return cText - pStart;
}

int CedarXSubTextBox::countLines(const char* pText, size_t len, const SkPaint& paint,
                            SkScalar mWidth, SkScalar* subTextWidth, int specialEffectFlag)
{
    const char* stop = pText + len;
    int         nCount = 0;
    size_t       charCount;
    SkScalar    lineWidth = 0;

    *subTextWidth = 0;

    if (mWidth > 0)
    {
        do
        {
            nCount += 1;
            pText += subLineBreak(pText, stop, paint, mWidth, &charCount,
                             specialEffectFlag, &lineWidth);
            if(*subTextWidth < lineWidth)
            {
                *subTextWidth = lineWidth;
            }
        } while (pText < stop);
    }
    return nCount;
}

//////////////////////////////////////////////////////////////////////////////

CedarXSubTextBox::CedarXSubTextBox()
{
    mFBox.setEmpty();
    mFSpacingMul = SK_Scalar1;
    mFSpacingAdd = 0;
    fMode = CedarXSubTextBoxLineBreak_Mode;
    fSpacingAlign = CedarXSubTextBoxEnd_SpacingAlign;
    mLastDispXPos = -1;
    mLastDispYPos = -1;
}

void CedarXSubTextBox::setMode(CedarXSubTextBoxMode mode)
{
    fMode = SkToU8(mode);
}

void CedarXSubTextBox::setSpacingAlign(CedarXSubTextBoxSpacingAlign align)
{
    fSpacingAlign = SkToU8(align);
}

void CedarXSubTextBox::subGetBox(SkRect* box) const
{
    if (box)
    {
        *box = mFBox;
    }
}

void CedarXSubTextBox::subSetBox(const SkRect& box)
{
    mFBox = box;
}

void CedarXSubTextBox::subSetBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    mFBox.set(left, top, right, bottom);
}

void CedarXSubTextBox::subGetSpacing(SkScalar* mul, SkScalar* add) const
{
    if (mul)
    {
        *mul = mFSpacingMul;
    }
    if (add)
    {
        *add = mFSpacingAdd;
    }
}

void CedarXSubTextBox::subSetSpacing(SkScalar mul, SkScalar add)
{
    mFSpacingMul = mul;
    mFSpacingAdd = add;
}

int CedarXSubTextBox::getLastXPos()
{
    return mLastDispXPos;
}

int CedarXSubTextBox::getLastYPos()
{
    return mLastDispYPos;
}

void CedarXSubTextBox::drawText(SkCanvas* canvas, const char *text, size_t len,
                               const SkPaint& mPaint, SkScalar textHeight,
                               int specialEffectFlag)
{
    size_t      drawCount;
    SkScalar    lineWidth;

    SkASSERT(canvas && &mPaint && (text || len == 0));

    SkScalar mMarginWidth = mFBox.width();

    logv("*&&&&CedarXSubTextBox::drawText len = %d,mMarginWidth = %f\n",len,mMarginWidth);
    if (mMarginWidth <= 0 || len == 0)
    {
       return;
    }

    const char* textStop = text + len;

    SkScalar   mSkScalarX=0, mSkScalarY=0, mScaledSpacing, height, mFontHeight;
    SkPaint::FontMetrics    mMetrics;

    switch (mPaint.getTextAlign())
    {
        case SkPaint::kLeft_Align:
            mSkScalarX = 0;
            break;
        case SkPaint::kCenter_Align:
            mSkScalarX = SkScalarHalf(mMarginWidth);
            break;
        default:
            mSkScalarX = mMarginWidth;
            break;
    }
    mSkScalarX += mFBox.fLeft;
    mFontHeight = mPaint.getFontMetrics(&mMetrics);
    mScaledSpacing = SkScalarMul(mFontHeight, mFSpacingMul) + mFSpacingAdd;
    height = mFBox.height();

    //  compute mSkScalarY position for first line
    {
        switch (fSpacingAlign)
        {
            case CedarXSubTextBoxStart_SpacingAlign:
                mSkScalarY = 0;
                break;
            case CedarXSubTextBoxCenter_SpacingAlign:
                mSkScalarY = SkScalarHalf(height - textHeight);
                break;
            default:
                mSkScalarY = height - textHeight;
                break;
        }

       mSkScalarY += mFBox.fTop - mMetrics.fAscent;
      /* logd("CedarXSubTextBox::drawText mSkScalarY = %f,mFBox.fTop=%f,\
          mMetrics.fAscent = %f,textHeight = %f,height = %f\n",mSkScalarY,
          mFBox.fTop,mMetrics.fAscent,textHeight,height);*/
    }

    mLastDispXPos = (int)mSkScalarX;
    mLastDispYPos = (int)(mSkScalarY-mScaledSpacing);

    for (;;)
    {
       len = subLineBreak(text, textStop, mPaint, mMarginWidth,&drawCount,
                       specialEffectFlag, &lineWidth);
       if (mSkScalarY + mMetrics.fDescent + mMetrics.fLeading > 0)
       {
           canvas->drawText(text, drawCount, mSkScalarX, mSkScalarY, mPaint);
           logv("canvas->drawText!text = %s,drawCount = %d\n",text,drawCount);
       }
       text += len;
       if (text >= textStop)
       {
           break;
       }
       mSkScalarY += mScaledSpacing;
       if (mSkScalarY + mMetrics.fAscent >= height)
       {
           break;
       }
    }
}


int CedarXSubTextBox::getTextVerInf(const char *text, size_t len,
    const SkPaint& paint,SkScalar* subTextHeight,SkScalar* subTextWidth,SkScalar* textBoxHeight,
    int textBoxStartx,int textBoxEndx, int textBoxStarty, int textBoxEndy, int specialEffectFlag)
{
    int nCount = 0;
    SkScalar mTextHeight = 0;
    const char* textStop;
    SkScalar mScaledSpacing, height, mFontHeight;
    SkPaint::FontMetrics    metrics;
    SkScalar marginWidth;

    marginWidth = textBoxEndx - textBoxStartx + 1;
    mFontHeight = paint.getFontMetrics(&metrics);
    mScaledSpacing = SkScalarMul(mFontHeight, mFSpacingMul) + mFSpacingAdd;
    height = textBoxEndy - textBoxStarty + 1;
    textStop = text + len;
    mTextHeight = mFontHeight;

    if(fMode == CedarXSubTextBoxLineBreak_Mode)
    {
        nCount = countLines(text, textStop - text, paint, marginWidth,
                    subTextWidth, specialEffectFlag);
        SkASSERT(nCount > 0);
        mTextHeight += mScaledSpacing * (nCount - 1);
    }
    *subTextHeight = mTextHeight;
    *textBoxHeight = (SkScalar)(textBoxEndy-textBoxStarty);
    return NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int CedarXSub::Show()
{
    if(mShow == false)
    {
        mSurfaceControl->show();
        mShow   = true;
    }

    return  NO_ERROR;
}

int CedarXSub::Hide()
{
    if(mShow == true)
    {
        mSurfaceControl->hide();
        mShow   = false;
    }

    return  NO_ERROR;
}

int CedarXSub::setZorderBottom()
{
    mSurfaceControl->setLayer(mBottomBaseLayer);

    return  NO_ERROR;
}

int CedarXSub::setZorderTop()
{
    mSurfaceControl->setLayer(mTopBaseLayer);

    return  NO_ERROR;
}

int   CedarXSub::setLayer(int layer)
{
    if(mShow == true)
    {
        SurfaceComposerClient::openGlobalTransaction();
        mSurfaceControl->setLayer(layer);
        SurfaceComposerClient::closeGlobalTransaction();
    }

    mLayer   = layer;

    return  NO_ERROR;
}

int   CedarXSub::getLayer()
{
    return mLayer;
}

int CedarXSub::setBackColor(int color)
{
    if(mBackColor != color)
    {
        mBackColor = color;

        if(mShow == true)
        {
            startRender();
            render();
            endRender();
        }
    }

    return NO_ERROR;
}

int CedarXSub::getBackColor()
{
    return mBackColor;
}

int   CedarXSub::getBitmapFormat()
{
    int   format;

    //format = (int)get_bitmap_format();

    return PIXEL_FORMAT_RGBA_8888;
}

int CedarXSub::convertUniCode(sub_item_inf *sub_info)
{
    int         charset;
    const char* enc = NULL;

    //logd("******************CedarXSub::convertUniCode sub_info->encodingType = %d mCharset=%d"
    //,sub_info->encodingType,mCharset);
    charset   = sub_info->encodingType;
    if(charset == SUBTITLE_TEXT_FORMAT_UNKNOWN)
    {
        charset = mCharset;
    }

    switch(charset)
    {
        case SUBTITLE_TEXT_FORMAT_UTF8:
            enc = "UTF-8";
            break;

        case SUBTITLE_TEXT_FORMAT_GB2312:
            enc = "HZ-GB-2312" ;
            break;

        case SUBTITLE_TEXT_FORMAT_UTF16LE:
            enc = "UTF-16LE";
            break;

        case SUBTITLE_TEXT_FORMAT_UTF16BE:
            enc = "UTF-16BE";
            break;

        case SUBTITLE_TEXT_FORMAT_UTF32LE:
            enc = "UTF-32LE";
            break;

        case SUBTITLE_TEXT_FORMAT_UTF32BE:
            enc = "UTF-32BE";
            break;

        case SUBTITLE_TEXT_FORMAT_BIG5:
            enc = "Big5";
            break;

        case SUBTITLE_TEXT_FORMAT_GBK:
            enc = "GBK";
            break;

        case SUBTITLE_TEXT_FORMAT_ANSI:  //can not match,set to "UTF-8"
            enc = "UTF-8";
            break;

        default:
            enc = "UTF-8";
            break;
    }

    if (enc)
    {
        UErrorCode status = U_ZERO_ERROR;

        UConverter *conv = ucnv_open(enc, &status);
        if (U_FAILURE(status))
        {
           logw("could not create UConverter for %s\n", enc);

           return -1;
        }

        UConverter *utf8Conv = ucnv_open("UTF-8", &status);
        if (U_FAILURE(status))
        {
            logw("could not create UConverter for UTF-8\n");

            ucnv_close(conv);
            return -1;
        }

        // first we need to untangle the utf8 and convert it back to the original bytes
        // since we are reducing the length of the string, we can do this in place
        const char* src = (const char*)sub_info->subTextBuf;
        int len = sub_info->subTextLen;
        // now convert from native encoding to UTF-8
        int targetLength = len * 3 + 1;
        memset(mText,0,MAX_SUBLENGTH);
        char* target = &mText[0];
        ucnv_convertEx(utf8Conv, conv, &target, (const char*)target + targetLength,&src,
                  (const char*)src + len, NULL, NULL, NULL, NULL, true, true, &status);
        if (U_FAILURE(status))
        {
            logw("ucnv_convertEx failed: %d\n", status);

            memset(mText,0,MAX_SUBLENGTH);
        }

        logv("CedarXSub::convertUniCode src = %s, mText: %s\n",sub_info->subTextBuf, mText);

        ucnv_close(conv);
        ucnv_close(utf8Conv);
    }
    return NO_ERROR;
}


//*********************************************************//
//**********************************************************//
int CedarXSub::setPosition(int posX,int posY)
{
    if(true)
    {
        size_t      count;
        int         posx;
        int         posy;

        mPosX   = posX;
        mPosY   = posY;
        if(mPosX + mMaxWidth > mScreenWidth)
        {
            //mPosX = mScreenWidth - mMaxWidth;
            mMaxWidth = mScreenWidth - mPosX;
        }

        if(mPosY + mMaxHeight > mScreenHeight)
        {
            //mPosY = mScreenHeight - mMaxHeight;
            mMaxHeight  = mScreenHeight - mPosY;
        }

        SurfaceComposerClient::openGlobalTransaction();
        mSurfaceControl->setPosition(mPosX,mPosY);

        SurfaceComposerClient::closeGlobalTransaction();
    }

    return  NO_ERROR;
}

int CedarXSub::setPositionYPercent(int percent)
{
    int maxYPos = 0;

    if(mSubMode & PIC_SUBTITLE)
    {
        mPosY   = (mScreenHeight - mMaxHeight)* (100 - percent) / 100;
        if(mPosY + mMaxHeight > mScreenHeight)
        {
            mPosY = mScreenHeight - mMaxHeight;
        }
        logd("2@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d %d", mPosX, mPosY);
        mSurfaceControl->setPosition(mPosX,mPosY);
    }
    else if((mShow == true) && (mSubMode & TEXT_SUBTITLE))
    {
        maxYPos = mScreenHeight*(100-percent)/100;
        if(mEndy > maxYPos)
        {
            mEndy = maxYPos;
        }
        if((mEndy-mStarty)<mTextHeight)
        {
            mStarty = mEndy-mTextHeight;
        }
        if(mStarty < 0)
        {
            mStarty = 0;
        }
        setPosition(mStartx,mStarty);
        mTextBox->subSetBox(0,0,SkIntToScalar(mEndx-mStartx),SkIntToScalar(mEndy-mStarty));
    }

    return  NO_ERROR;
}

int CedarXSub::getPositionX()
{
    return mPosX;
}

int CedarXSub::getPositionY()
{
    return mPosY;
}

int CedarXSub::getWidth()
{
    return mWidth;
}

int CedarXSub::getHeight()
{
    return mHeight;
}

int CedarXSub::setFontSize(int fontsize)
{
    int      len;
    SkScalar textBoxHeight;

    if(mSubMode & PIC_SUBTITLE)
    {
        return NO_ERROR;
    }
    if(fontsize != mFontSize)
    {
        mFontSize = fontsize;
        mPaint.setTextSize(fontsize * mFontScaleRatio / FONT_SIZE_UNIT);
        if(mShow == true)
        {
            len   = strlen((char*)mText);
            mTextBox->getTextVerInf(mText,len,mPaint,&mTextHeight,&mTextWidth,
                           &textBoxHeight, mStartx, mEndx, mStarty, mEndy,
                           mDispSubInfo->subEffectFlag);
            needModifyBoxInf(textBoxHeight);
            setPosition(mStartx,mStarty);
            mTextBox->subSetBox(0,0,SkIntToScalar(mEndx-mStartx),SkIntToScalar(mEndy-mStarty));
            startRender();
            render();
            endRender();
        }
    }

  return NO_ERROR;
}

int CedarXSub::getFontSize()
{
    return mFontSize;
}
int CedarXSub::setTextColor(int color)
{
    if(mTextColor != (unsigned int)color)
    {
        mTextColor = (unsigned int)color;

        mPaint.setColor(mTextColor);
        if(mShow == true)
        {
            startRender();
            render();
            endRender();
        }
    }

    return NO_ERROR;
}

int CedarXSub::getTextColor()
{
    return mTextColor;
}

int CedarXSub::setCharset(int Charset)
{
    mCharset = Charset;

    return NO_ERROR;
}

int CedarXSub::getCharset()
{
    return mCharset;
}

int CedarXSub::setTextAlign(int align)
{
    if(mTextAlign != align)
    {
        mTextAlign = align;

        if((align & SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_LEFT)
        {
            mPaint.setTextAlign(SkPaint::kLeft_Align);
        }
        else if((align & SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_CENTER)
        {
            mPaint.setTextAlign(SkPaint::kCenter_Align);
        }
        else if((align & SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_RIGHT)
        {
            mPaint.setTextAlign(SkPaint::kRight_Align);
        }
        else
        {
            mPaint.setTextAlign(SkPaint::kLeft_Align);
        }

        if((align & SUN_RENDER_VALIGN_MASK) == SUB_RENDER_VALIGN_TOP)
        {
            if(mTextBox != NULL)
            {
                mTextBox->setSpacingAlign(CedarXSubTextBox::CedarXSubTextBoxStart_SpacingAlign);
            }
        }
        else if((align & SUN_RENDER_VALIGN_MASK) == SUB_RENDER_VALIGN_CENTER)
        {
            if(mTextBox != NULL)
            {
                mTextBox->setSpacingAlign(CedarXSubTextBox::CedarXSubTextBoxCenter_SpacingAlign);
            }
        }
        else if((align & SUN_RENDER_VALIGN_MASK) == SUB_RENDER_VALIGN_BOTTOM)
        {
            if(mTextBox != NULL)
            {
                mTextBox->setSpacingAlign(CedarXSubTextBox::CedarXSubTextBoxEnd_SpacingAlign);
            }
        }
        else
        {
            if(mTextBox != NULL)
            {
                mTextBox->setSpacingAlign(CedarXSubTextBox::CedarXSubTextBoxStart_SpacingAlign);
            }
        }

        if(mShow == true)
        {
            startRender();
            render();
            endRender();
        }
    }

    return NO_ERROR;
}

int CedarXSub::getTextAlign()
{
    return mTextAlign;
}

int CedarXSub::setFontStyle(int style)
{
    if(mFontStyle != style)
    {
        mFontStyle = style;

        if((style & SUB_RENDER_STYLE_BOLD) == SUB_RENDER_STYLE_BOLD)
        {
            mPaint.setFakeBoldText(true);
        }
        else
        {
            mPaint.setFakeBoldText(false);
        }

        if((style & SUB_RENDER_STYLE_ITALIC) == SUB_RENDER_STYLE_ITALIC)
        {
            mPaint.setLinearText(true);
        }
        else
        {
            mPaint.setLinearText(false);
        }

        if((style & SUB_RENDER_STYLE_UNDERLINE) == SUB_RENDER_STYLE_UNDERLINE)
        {
            mPaint.setUnderlineText(true);
        }
        else
        {
            mPaint.setUnderlineText(false);
        }
        if((style & SUB_RENDER_STYLE_STRIKETHROUGH) == SUB_RENDER_STYLE_STRIKETHROUGH)
        {
            mPaint.setStrikeThruText(true);
        }
        else
        {
            mPaint.setStrikeThruText(false);
        }

        if(mShow == true)
        {
            startRender();
            render();
            endRender();
        }
    }

    return NO_ERROR;
}

int CedarXSub::getFontStyle()
{
    return mFontStyle;
}

//******************************************************************//
//******************************************************************//
CedarXSubRender::CedarXSubRender()
{
    sp<CedarXSub>     cedarXSub;

    logd("CedarXSubRender::CedarXSubRender xxxxxxxxxxxxxxxxxxxxxxxx1");
    mUserSetFontColor = 0;
    mUserSetFontSize  = 20;
    mUserSetFontStyle = 0;
    mUserSetYPercent  = 10;
    sub_pre           = NULL;
    mInitSetFontColor = 0;
    mInitSetFontSize  = 0;
    mInitSetFontStyle = 0;
    mDisplay          = 0;
    mPid              = 0;
    mBaseLayer        = 0;

    cedarXSub = new CedarXSub(0, mUserSetFontColor, mUserSetFontSize,
                            mUserSetFontStyle, mUserSetYPercent);
    logd("CedarXSubRender::CedarXSubRender xxxxxxxxxxxxxxxxxxxxxxx2");

    /*create main cedarXSub*/
    mCedarXSubs.add(cedarXSub);
}

CedarXSubRender::~CedarXSubRender()
{
    logd("CedarXSubRender::~CedarXSubRender!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
}

int CedarXSubRender::cedarxSubShow()
{
    size_t      count;
    logv("CedarXSubRender::cedarxSubShow!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    count   = mCedarXSubs.size();

    SurfaceComposerClient::openGlobalTransaction();

    for(size_t i = 0;i < count;i++)
    {
        if(mCedarXSubs[i]->getSubShowFlag() == SUB_SHOW_NEW_VALID)
        {
            mCedarXSubs[i]->Show();
        }
    }

    SurfaceComposerClient::closeGlobalTransaction();

    return NO_ERROR;
}

int CedarXSubRender::cedarxSubHide(unsigned int systemTime, unsigned int* hasSubShowFlag)
{
    size_t      count;
    unsigned  int remainSubShowFlag;
    logv("CedarXSubRender::cedarxSubHide!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    count   = mCedarXSubs.size();

    SurfaceComposerClient::openGlobalTransaction();
    remainSubShowFlag = 0;

    if(systemTime == 0xFFFFFFFF)
    {
        for(size_t i = 0;i < count;i++)
        {
            mCedarXSubs[i]->setSubShowFlag(SUB_SHOW_INVALID);
            mCedarXSubs[i]->Hide();
        }
    }
    else
    {
        for(size_t i = 0;i < count;i++)
        {
            if(mCedarXSubs[i]->getSubShowFlag()!=SUB_SHOW_INVALID)
            {
                if((mCedarXSubs[i]->getSubShowEndTime())<=systemTime)
                {
                mCedarXSubs[i]->setSubShowFlag(SUB_SHOW_INVALID);
                mCedarXSubs[i]->Hide();
                }
                else
                {
                remainSubShowFlag = 1;
                mCedarXSubs[i]->setSubShowFlag(SUB_SHOW_OLD_VALID);

                }
            }
        }
    }

    if(hasSubShowFlag != NULL)
    {
        *hasSubShowFlag = remainSubShowFlag;
    }

    SurfaceComposerClient::closeGlobalTransaction();
    if(remainSubShowFlag == 1)
    {
        for(size_t i = 0;i < count;i++)
        {
            if(mCedarXSubs[i]->getSubShowFlag()!=SUB_SHOW_INVALID)
            {
                mCedarXSubs[i]->processSpecialEffect(systemTime);
            }
        }
    }
    return  NO_ERROR;
}

int CedarXSubRender::cedarxSubSetZorderTop()
{
    size_t      count;
    logd("CedarXSubRender::cedarxSubSetZorderTop!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    count   = mCedarXSubs.size();

    SurfaceComposerClient::openGlobalTransaction();

    for(size_t i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setZorderTop();
    }

    SurfaceComposerClient::closeGlobalTransaction();

    return  NO_ERROR;
}

int CedarXSubRender::cedarxSubSetZorderBottom()
{
    size_t      count;
    logd("CedarXSubRender::cedarxSubSetZorderBottom!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    count   = mCedarXSubs.size();

    SurfaceComposerClient::openGlobalTransaction();

    for(size_t i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setZorderBottom();
    }

    SurfaceComposerClient::closeGlobalTransaction();

    return  NO_ERROR;
}


int CedarXSubRender::cedarxSubSetPosition(int index,int positionx,int positiony)
{
    size_t         count;

    count = mCedarXSubs.size();

    if(index > (int)count)
    {
         logw("Invalid index value!\n");

         return -1;
    }
    logd("???????????????????1");
    return  mCedarXSubs[index]->setPosition(positionx,positiony);
}

int CedarXSubRender::cedarxSubSetYPercent(int index,int percent)
{
    size_t         count;
    int          i;

    count = mCedarXSubs.size();
    if(index > (int)count)
    {
        logw("Invalid index value!\n");

        return -1;
    }

    SurfaceComposerClient::openGlobalTransaction();


    mUserSetYPercent  = percent;
    if(mCedarXSubs[0]->mShow == true)
    {
        cedarxSubHide(0xFFFFFFFF,NULL);
        if(sub_pre!=NULL)
            updateSubPara(sub_pre);
        cedarxSubShow();
    }
    SurfaceComposerClient::closeGlobalTransaction();

    return  0;
}

int CedarXSubRender::cedarxSubGetPositionX(int index)
{
    size_t         count;

    count = mCedarXSubs.size();

    if(index > (int)count)
    {
    logw("Invalid index value!\n");

    return -1;
    }

    return  mCedarXSubs[index]->getPositionX();
}

int CedarXSubRender::cedarxSubGetPositionY(int index)
{
    size_t         count;

    count = mCedarXSubs.size();

    if(index > (int)count)
    {
        logw("Invalid index value!\n");

        return -1;
    }

    return  mCedarXSubs[index]->getPositionY();
}


int CedarXSubRender::cedarxSubGetHeight(int index)
{
    size_t         count;

    count = mCedarXSubs.size();

    if(index > (int)count)
    {
        logw("Invalid index value!\n");

        return -1;
    }

    return  mCedarXSubs[index]->getHeight();
}

int CedarXSubRender::cedarxSubGetWidth(int index)
{
    size_t         count;

    count = mCedarXSubs.size();

    if(index > (int)count)
    {
        logw("Invalid index value!\n");

        return -1;
    }

    return  mCedarXSubs[index]->getWidth();
}

int CedarXSubRender::cedarxSubSetFontColor(int color)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setTextColor(color);
    }

    mInitSetFontColor =
           (mUserSetFontColor ==0)? SUB_INIT_SET_FONT_INFO : SUB_USER_SET_FONT_INFO;
    mUserSetFontColor = color;
    return NO_ERROR;
}

int CedarXSubRender::cedarxSubGetFontColor()
{
    return mCedarXSubs[0]->getTextColor();
}

int CedarXSubRender::cedarxSubSetFontSize(int size)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t i = 0;i < count;i++)
    {
        mInitSetFontSize =
            (mUserSetFontSize ==0)? SUB_INIT_SET_FONT_INFO : SUB_USER_SET_FONT_INFO;
        mCedarXSubs[i]->setFontSize(size);
    }
    mUserSetFontSize = size;
    return NO_ERROR;
}

int CedarXSubRender::cedarxSubGetFontSize()
{
    return mCedarXSubs[0]->getFontSize();
}

int CedarXSubRender::cedarxSubSetCharset(int charset)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setCharset(charset);
    }

    return NO_ERROR;
}

int CedarXSubRender::cedarxSubGetCharset()
{
    return mCedarXSubs[0]->getCharset();
}

int CedarXSubRender::cedarxSubSetBackColor(int color)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t   i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setBackColor(color);
    }

    return  NO_ERROR;
}

int CedarXSubRender::cedarxSubGetBackColor()
{
    return mCedarXSubs[0]->getBackColor();
}

int CedarXSubRender::cedarxSubSetAlign(int align)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t   i = 0;i < count;i++)
    {
        mCedarXSubs[i]->setTextAlign(align);
    }
    return  NO_ERROR;
}

int CedarXSubRender::cedarxSubGetAlign()
{
    return mCedarXSubs[0]->getTextAlign();
}

//**************************************************************************//
//**************************************************************************//
int CedarXSubRender::cedarxSubSetFontStyle(int style)
{
    size_t      count;

    count   = mCedarXSubs.size();

    for(size_t   i = 0;i < count;i++)
    {
        mInitSetFontStyle =
            (mUserSetFontStyle ==0)? SUB_INIT_SET_FONT_INFO : SUB_USER_SET_FONT_INFO;
        mCedarXSubs[i]->setFontStyle(style);
    }
    mUserSetFontStyle = style;
    return  NO_ERROR;
}

int CedarXSubRender::cedarxSubGetFontStyle()
{
    return mCedarXSubs[0]->getFontStyle();
}

//*************************************************************************//
//***************************************************************************//
int CedarXSub::setSubInf(sub_item_inf *sub_info,int startx, int starty, int endx,
                      int endy, int lastDispx, int lastDispy,
                      int newShowSubFlag, int yPercent)
{
    mSubShowFlag       = SUB_SHOW_NEW_VALID;
    mDispSubInfo       = sub_info;
    logv("setSubInf start");
    if(sub_info->subMode != 0)
    {
        return 0;
    }
    setTextBox(sub_info, startx, starty, endx, endy, lastDispx,
            lastDispy, newShowSubFlag, yPercent);

    switch(sub_info->subEffectFlag)
    {
        case SUB_RENDER_EFFECT_NONE:
        {
            break;
        }
        case SUB_RENDER_EFFECT_SCROLL_UP:
        {
            mAlignment &= SUN_RENDER_HALIGN_MASK;
            mAlignment |= SUB_RENDER_VALIGN_TOP;
            mEndy   = mDispSubInfo->effectEndyPos;
            mStarty = mEndy - mMaxHeight;
            if(mStarty < 0)
            {
                mStarty = 0;
            }
            break;
        }
        case SUB_RENDER_EFFECT_SCROLL_DOWN:
        {
            mAlignment &= SUN_RENDER_HALIGN_MASK;
            mAlignment |= SUB_RENDER_VALIGN_BOTTOM;
            mStarty   = mDispSubInfo->effectStartyPos;
            mEndy   = mStarty + mMaxHeight;
            if(mEndy > mScreenHeight)
            {
                mEndy = mScreenHeight;
            }
            break;
        }
        case SUB_RENDER_EFFECT_BANNER_LTOR:
        {
            mAlignment &= SUN_RENDER_VALIGN_MASK;
            mAlignment |= SUB_RENDER_HALIGN_RIGHT;
            mStartx = 0;
            mEndx   = mScreenWidth;
            break;
        }
        case SUB_RENDER_EFFECT_BANNER_RTOL:
        {
            mAlignment &= SUN_RENDER_VALIGN_MASK;
            mAlignment |= SUB_RENDER_HALIGN_LEFT;
            mStartx = 0;
            mEndx   = mScreenWidth;
            break;
        }
        case SUB_RENDER_EFFECT_MOVE:
        {
            mAlignment &= SUN_RENDER_HALIGN_MASK;
            mAlignment &= SUN_RENDER_VALIGN_MASK;
            mAlignment |= SUB_RENDER_VALIGN_TOP;
            mAlignment |= SUB_RENDER_HALIGN_LEFT;
            mStartx = 0;
            mEndx   = mScreenWidth;
            mEndy   = mScreenHeight;
            mStarty = mScreenHeight -mMaxHeight;
            break;
        }
        case SUB_RENDER_EFFECT_KARAOKE:
        {
            break;
        }
    }
    logv("setSubInf finish");
    return NO_ERROR;
}

int CedarXSub::setSubShowFlag(int subShowFlag)
{
    mSubShowFlag = subShowFlag;
    return NO_ERROR;
}
int CedarXSub::getSubShowFlag()
{
    return mSubShowFlag;
}

unsigned int CedarXSub::getSubShowEndTime()
{
    return mDispSubInfo->endTime;
}

int  CedarXSub::getTextColorFlag()
{
    if(mDispSubInfo == NULL)
    {
        return SUB_HAS_NONE_COLOR;
    }
    return (mDispSubInfo->primaryColor>0)? SUB_HAS_DEFINED_COLOR: SUB_HAS_NONE_COLOR;
}
int  CedarXSub::getTextFontSizeFlag()
{
    if(mDispSubInfo == NULL)
    {
        return SUB_HAS_NONE_STYLE;
    }
    return (mDispSubInfo->fontSize>0)? SUB_HAS_DEFINED_FONTSIZE: SUB_HAS_NONE_FONTSIZE;
}

int  CedarXSub::getTextFontStyleFlag()
{
    if(mDispSubInfo == NULL)
    {
        return SUB_HAS_NONE_STYLE;
    }
    return (mDispSubInfo->subStyle==
         SUB_RENDER_STYLE_NONE)? SUB_HAS_NONE_STYLE:SUB_HAS_DEFINED_STYLE;
}

//************************************************************************//
//************************************************************************//

int CedarXSub::processSpecialEffect(unsigned int systemTime)
{
    int xPos = 0;
    int yPos = 0;
    int startx = 0;
    int endx = 0;
    int curSubSectionIdx = 0;
    int newAlignment = 0;

    if((mSubShowFlag==SUB_SHOW_INVALID) ||(mDispSubInfo->subEffectFlag==SUB_RENDER_EFFECT_NONE))
    {
        return NO_ERROR;
    }

    switch(mDispSubInfo->subEffectFlag)
    {
        case SUB_RENDER_EFFECT_SCROLL_UP:
        {
            xPos = getPositionX();
            yPos = mDispSubInfo->effectEndyPos -
             (systemTime-mDispSubInfo->startTime)/mDispSubInfo->effectTimeDelay;
            setPosition(xPos, yPos);
            break;
        }
        case SUB_RENDER_EFFECT_SCROLL_DOWN:
        {
            xPos = getPositionX();
            yPos = mDispSubInfo->effectStartyPos+
             (systemTime-mDispSubInfo->startTime)/mDispSubInfo->effectTimeDelay;
            setPosition(xPos, yPos);
            break;
        }
        case SUB_RENDER_EFFECT_BANNER_LTOR:
        {
            xPos = mDispSubInfo->effectStartxPos+
              (systemTime-mDispSubInfo->startTime)/mDispSubInfo->effectTimeDelay;
            mTextBox->subSetBox(0,0,SkIntToScalar(xPos),SkIntToScalar(mEndy-mStarty));
            break;
        }
        case SUB_RENDER_EFFECT_BANNER_RTOL:
        {
            xPos = mDispSubInfo->effectEndxPos-
             (systemTime-mDispSubInfo->startTime)/mDispSubInfo->effectTimeDelay;
            mTextBox->subSetBox(xPos,0,SkIntToScalar(mEndx-mStartx),
                           SkIntToScalar(mEndy-mStarty));
            break;
        }
        case SUB_RENDER_EFFECT_MOVE:
        {
            if(mDispSubInfo->effectStartyPos < mDispSubInfo->effectEndyPos)
            {
                yPos = mDispSubInfo->effectStartyPos +
                      ((mDispSubInfo->effectEndyPos-mDispSubInfo->effectStartyPos)*
                      (systemTime-mDispSubInfo->startTime))/
                      (mDispSubInfo->endTime-mDispSubInfo->startTime);
            }
            else
            {
                yPos = mDispSubInfo->effectStartyPos -
                       ((mDispSubInfo->effectStartyPos-mDispSubInfo->effectEndyPos)*
                       (systemTime-mDispSubInfo->startTime))/
                       (mDispSubInfo->endTime-mDispSubInfo->startTime);
            }
            if(mDispSubInfo->effectStartxPos < mDispSubInfo->effectEndxPos)
            {
                xPos = mDispSubInfo->effectStartxPos +
                      ((mDispSubInfo->effectEndxPos-mDispSubInfo->effectStartxPos)*
                      (systemTime-mDispSubInfo->startTime))/
                      (mDispSubInfo->endTime-mDispSubInfo->startTime);
            }
            else
            {
                xPos = mDispSubInfo->effectStartxPos -
                       ((mDispSubInfo->effectStartxPos-mDispSubInfo->effectEndxPos)*
                       (systemTime-mDispSubInfo->startTime))/
                       (mDispSubInfo->endTime-mDispSubInfo->startTime);
            }
            setPosition(xPos, yPos);
            break;
        }
        #if 0
        case SUB_RENDER_EFFECT_KARAOKE:
        {
             if((systemTime-mDispSubInfo->startTime)> mDispSubInfo->effectTimeDelay)
             {
                return NO_ERROR;
             }
             if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_LEFT)
             {
                startx = mStartx;
             }
             else if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_CENTER)
             {
                startx = mStartx+(mEndx-mStartx-(int)mTextWidth)/2;
             }
             else if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_RIGHT)
             {
                startx = mEndx-(int)mTextWidth;
             }
             endx = startx+mTextWidth*(systemTime-mDispSubInfo->startTime)/
                mDispSubInfo->effectTimeDelay;

             setPosition(startx, mStarty);

             mTextBox->subSetBox(0,0,SkIntToScalar(endx-startx),SkIntToScalar(mEndy-mStarty));
             newAlignment = mAlignment;
             newAlignment &= SUN_RENDER_VALIGN_MASK;
             newAlignment |= SUB_RENDER_HALIGN_LEFT;

             setTextAlign(newAlignment);
             SurfaceComposerClient::openGlobalTransaction();

             if(mTextColor != mDispSubInfo->primaryColor)
             {
                mTextColor = mDispSubInfo->primaryColor;
             mPaint.setColor(mTextColor);
             }
             startRenderRegion(startx, mStarty, endx, mEndy);
             render(0);
             endRender();
             SurfaceComposerClient::closeGlobalTransaction();
             return NO_ERROR;
        }
        #endif
       #if 0
        case SUB_RENDER_EFFECT_KARAOKE:
        {
             if((systemTime-mDispSubInfo->startTime)> mDispSubInfo->effectTimeDelay)
             {
                return NO_ERROR;
             }
             mDispSubInfo->subKarakoEffectInf->karaKoSectionLen[0] = 0;
             mDispSubInfo->subKarakoEffectInf->karaKoSectionLen[68] = 0;
             mDispSubInfo->subKarakoEffectInf->karaKoSectionLen[126] = 0;

             for(curSubSectionIdx=0;
             curSubSectionIdx<mDispSubInfo->subKarakoEffectInf->karakoSectionNum;
             curSubSectionIdx++)
             {
                 baseTime1 = mDispSubInfo->subKarakoEffectInf
                        ->karaKoSectionStartTime[curSubSectionIdx];
                 baseTime2 = mDispSubInfo->subKarakoEffectInf
                        ->karaKoSectionStartTime[curSubSectionIdx+1];
                 logd("***********baseTime1=%d, baseTime2=%d, diffTime=%d\n",
                 baseTime1, baseTime2, (systemTime-mDispSubInfo->startTime));
                if(((systemTime-mDispSubInfo->startTime)>=baseTime1)&&
                    ((systemTime-mDispSubInfo->startTime)<baseTime2))
                {
                    break;
                }
            }
             if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_LEFT)
             {
                startx = mStartx;
             }
             else if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_CENTER)
             {
                startx = mStartx+(mEndx-mStartx-(int)mTextWidth)/2;
             }
             else if((mAlignment&SUN_RENDER_HALIGN_MASK) == SUB_RENDER_HALIGN_RIGHT)
             {
                startx = mEndx-(int)mTextWidth;
             }


             len1      = mDispSubInfo->subKarakoEffectInf->karaKoSectionLen[curSubSectionIdx];
             len2      = mDispSubInfo->subKarakoEffectInf->karaKoSectionLen[curSubSectionIdx+1];

             logd("***********len1=%d, len2=%d, curSubSectionIdx=%d\n",
            len1, len2, curSubSectionIdx);

             startx += len1;

             endx  = startx +(systemTime-mDispSubInfo->startTime-baseTime1)*
               (len2-len1)/(baseTime2-baseTime1);

             //endx = startx+mTextWidth*(systemTime-mDispSubInfo->startTime)
                  /mDispSubInfo->effectTimeDelay;

             setPosition(startx, mStarty);

             mTextBox->subSetBox(0,0,SkIntToScalar(endx-startx),SkIntToScalar(mEndy-mStarty));
             newAlignment = mAlignment;
             newAlignment &= SUN_RENDER_VALIGN_MASK;
             newAlignment |= SUB_RENDER_HALIGN_LEFT;

             setTextAlign(newAlignment);
             SurfaceComposerClient::openGlobalTransaction();

             if(mTextColor != mDispSubInfo->primaryColor)
             {
                mTextColor = mDispSubInfo->primaryColor;
             mPaint.setColor(mTextColor);
             }
             startRenderRegion(startx, mStarty, endx, mEndy);
             render(0);
             endRender();
             SurfaceComposerClient::closeGlobalTransaction();
             return NO_ERROR;

        }
        #endif
        default:
        {
            return NO_ERROR;
        }
    }

    SurfaceComposerClient::openGlobalTransaction();
    startRender();
    render();
    endRender();
    SurfaceComposerClient::closeGlobalTransaction();
    return NO_ERROR;
}

//************************************************************************//
//*********************************************************************** //
int CedarXSub::getTextBox(int* startx, int *starty, int* endx, int* endy,
                        int *lastDispx, int *lastDispy)
{
    *startx     = mStartx;
    *starty     = mStarty;
    *endx       = mEndx;
    *endy       = mEndy;
    *lastDispx  = mStartDispx;
    *lastDispy  = mStartDispy;
    return NO_ERROR;
}
//*********************************************************************//
//******************************************************************** //

int  CedarXSub::setTextBox(sub_item_inf *sub_info,int startx, int starty, int endx,
                        int endy, int lastDispx, int lastDispy, int newShowSubFlag,
                        int yPercent)
{
    int align = 0;
    int newStartx = 0;
    int newStarty = 0;
    int newEndx = 0;
    int newEndy = 0;
    int maxYPos = 0;
    int minYPos = 0;

    mStartx     = startx;
    mStarty     = starty;
    mEndx       = endx;
    mEndy       = endy;
    mStartDispx = lastDispx;
    mStartDispy = lastDispy;

    mMaxWidth       = mScreenWidth;
    //mMaxHeight      = mMaxTextLine*mMaxFontHeight;
    mMaxHeight = mScreenHeight;
    mPosX = 0;
    mPosY = 0;

    maxYPos = mScreenHeight*(100-yPercent)/100;
    minYPos = yPercent*mScreenHeight/100;

    switch(sub_info->subDispPos)
    {
        case SUB_DISPPOS_TOP_LEFT:
             align |= SUB_RENDER_HALIGN_LEFT;
             align |= SUB_RENDER_VALIGN_TOP;
             newStartx  = sub_info->startx;
             newStarty  = sub_info->starty;
             newEndx    = mScreenWidth;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
            if(newStarty < minYPos)
            {
                newStarty = minYPos;
            }
             break;
        case SUB_DISPPOS_TOP_MID:
             align |= SUB_RENDER_HALIGN_CENTER;
             align |= SUB_RENDER_VALIGN_TOP;

             newStartx  = 0;
             newStarty  = sub_info->starty;
             newEndx    = mScreenWidth;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
            if(newStarty < minYPos)
            {
                newStarty = minYPos;
            }
             break;
        case SUB_DISPPOS_TOP_RIGHT:
             align |= SUB_RENDER_HALIGN_RIGHT;
             align |= SUB_RENDER_VALIGN_TOP;
             newStartx  = 0;
             newStarty  = sub_info->starty;
             newEndx    = sub_info->endx;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
            if(newStarty < minYPos)
            {
                newStarty = minYPos;
            }
             break;
        case SUB_DISPPOS_MID_LEFT:
             align |= SUB_RENDER_HALIGN_LEFT;
             align |= SUB_RENDER_VALIGN_CENTER;
             newStartx  = sub_info->startx;
             newStarty  = (mScreenHeight-mMaxHeight)>>1;
             newEndx    = mScreenWidth;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
             break;
        case SUB_DISPPOS_MID_MID:
             align |= SUB_RENDER_HALIGN_CENTER;
             align |= SUB_RENDER_VALIGN_CENTER;
             newStartx  = 0;
             newStarty  = (mScreenHeight-mMaxHeight)>>1;
             newEndx    = mScreenWidth;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
             break;
        case SUB_DISPPOS_MID_RIGHT:
             align |= SUB_RENDER_HALIGN_RIGHT;
             align |= SUB_RENDER_VALIGN_CENTER;
             newStartx  = 0;
             newStarty  = (mScreenHeight-mMaxHeight)>>1;
             newEndx    = sub_info->endx;
             newEndy    = newStarty+mMaxHeight;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
             break;
        case SUB_DISPPOS_BOT_LEFT:
             align |= SUB_RENDER_HALIGN_LEFT;
             align |= SUB_RENDER_VALIGN_BOTTOM;
             newStartx  = sub_info->startx;
             newEndy    = sub_info->endy;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
             newEndx    = mScreenWidth;
             newStarty  = newEndy-mMaxHeight;
             if(newStarty < 0)
             {
                newStarty = 0;
             }
             break;
        case SUB_DISPPOS_BOT_MID:
             align |= SUB_RENDER_HALIGN_CENTER;
             align |= SUB_RENDER_VALIGN_BOTTOM;
             newStartx  = 0;
             newEndx    = mScreenWidth;
             newEndy    = sub_info->endy;
             if((newEndy+10) > maxYPos)
             {
                newEndy = maxYPos;
             }
             newStarty  = newEndy-mMaxHeight;
             if(newStarty < 0)
             {
                newStarty = 0;
             }
             break;
        case SUB_DISPPOS_BOT_RIGHT:
             align |= SUB_RENDER_HALIGN_RIGHT;
             align |= SUB_RENDER_VALIGN_BOTTOM;
             newStartx  = 0;
             newEndx    = sub_info->endx;
             newEndy    = sub_info->endy;
             if(newEndy > maxYPos)
             {
                newEndy = maxYPos;
             }
             newStarty  = newEndy-mMaxHeight;
             if(newStarty < 0)
             {
                newStarty = 0;
             }
             break;
        default:
             align |= SUB_RENDER_HALIGN_CENTER;
             align |= SUB_RENDER_VALIGN_BOTTOM;
             newStartx  = mPosX;
             newEndx    = mPosX+mWidth;
             newEndy    = maxYPos;
             newStarty  = newEndy - mMaxHeight;
             if(newStarty < 0)
             {
                newStarty = 0;
             }
             break;
      }
     if((newShowSubFlag==0)&&(newStartx==mStartx)&&(newStarty==mStarty)&&
      (newEndx==mEndx)&&(newEndy==mEndy))
     {
        mStartx = newStartx;
        mEndx   = newEndx;
        mEndy   = mStartDispy+mStarty;
        mStarty = mEndy-mMaxHeight;
        if(mStarty <  0)
        {
            mStarty = 0;
        }
    }
    else
    {
        mStartx = newStartx;
        mStarty = newStarty;
        mEndx   = newEndx;;
        mEndy   = newEndy;
    }
    mAlignment = align;
    logv("******rendere:align=%x,subDispPos=%d,startx=%d,starty=%d,endx=%d,endy=%d,\
     orgStartx=%d,orgStarty=%d,orgEndx=%d,orgEndy=%d\n",align,sub_info->subDispPos,
     mStartx,mStarty, mEndx, mEndy,sub_info->startx,sub_info->starty,sub_info->endx,
     sub_info->endy);
    return  NO_ERROR;
}
//***********************************************************************//
//********************************************************************** //
int CedarXSub::generateBitmap(sub_item_inf *sub_info)
{
  ssize_t bpr = sub_info->subPicWidth * bytesPerPixel(PIXEL_FORMAT_RGBA_8888);
   mBitmap.setConfig(convertPixelFormat(PIXEL_FORMAT_RGBA_8888),
                  sub_info->subPicWidth, sub_info->subPicHeight, bpr);
   mBitmap.setIsOpaque(true);

   if (sub_info->subPicWidth > 0 && sub_info->subPicHeight > 0)
   {
        //set R of bitmap(RGBA) to 0
        for (int i = 0 ; i < (int)(sub_info->subPicWidth * sub_info->subPicHeight); i++ )
        {
            sub_info->subBitmapBuf[4*i] = 0;
        }
       mBitmap.setPixels((void *)sub_info->subBitmapBuf);
   }
   else
   {
       // be safe with an empty bitmap.
       mBitmap.setPixels(NULL);
   }

  return NO_ERROR;
}


//*********************************************************************//
//********************************************************************//


//***********************************************************************//
//********************************************************************** //
int CedarXSub::render()
{
    if(mCanvas == NULL)
  {
     logd("shit!!!!!!!!!");
  }
  mCanvas->drawColor(mBackColor, SkXfermode::kSrc_Mode);
  if(mSubMode & TEXT_SUBTITLE)
  {
        int     len;
      SkScalar textHeight;
        SkScalar textBoxHeight;
        len = strlen(mText);
        mTextBox->drawText(mCanvas, mText, len, mPaint, mTextHeight,
                      mDispSubInfo->subEffectFlag);
        mStartDispx = mTextBox->getLastXPos();
        mStartDispy = mTextBox->getLastYPos();
  }
  else if(mSubMode & PIC_SUBTITLE)
  {
        int posx    = 0;
        int posy    = 0;
        int width  = mBitmap.width();
        int height  = mBitmap.height();

        if((mDispSubInfo->subScaleWidth==0)||(mDispSubInfo->subScaleHeight==0))
        {
            posx = (mWidth - width)>>1;
            posy = (mHeight - height);
            posx = (posx<0)? 0 : posx;
            posy = (posy<0)? 0 : posy;
            #if SAVE_BITMAP_TO_PNG
            save("/data/local/tmp/subBitmapS011",mBitmap);
            #endif
         mCanvas->drawBitmap(mBitmap,posx,posy,&mPaint);
            #if SAVE_BITMAP_TO_PNG
            SkBitmap BitmapIncanvas = mCanvas->getDevice()->accessBitmap(false);
            save("/data/local/tmp/subBitmapS011-output",BitmapIncanvas);
            #endif
        }
        else
        {
            SkIRect src;
            SkRect  dst;
            src.set(0,0,width, height);

            posx = (mWidth - mDispSubInfo->subScaleWidth)>>1;
            posy = (mHeight - mDispSubInfo->subScaleHeight);

            posx = (posx<0)? 0 : posx;
            posy = (posy<0)? 0 : posy;

            dst.set(posx,posy,posx+mDispSubInfo->subScaleWidth,
               posy+mDispSubInfo->subScaleHeight);
            mCanvas->drawBitmapRect(mBitmap, &src, dst,&mPaint);
        }
  }
  return NO_ERROR;
}

//***************************************************************************//
//************************************************************************** //
CedarXSub::CedarXSub(int index, int userSetFontColor, int userSetFontSize,
                   int userSetFontStyle, int userSetYpercent)
{
    status_t err;
    sp<Surface> mSurface;
    char prop_value[4];
    int int_val;
    DisplayInfo mDisplayInfo;
    logd("CedarXSub::CedarXSub #####################\n");
    mDisplay = SUB_DISPLAY;
    mMaxTextLine = MAX_TEXTLINE;
    mFontScaleRatio = FONT_SIZE_UNIT;
    mMaxFontHeight   = MAX_FONTHEIGHT;
    mPid         = IPCThreadState::self()->getCallingPid();
    mShow         = false;

    sp<SurfaceComposerClient> mClient = new SurfaceComposerClient;
    mSurfaceClient = mClient;

    sp<IBinder> dtoken(
        SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
    SurfaceComposerClient::getDisplayInfo(dtoken, &mDisplayInfo);

    mScreenWidth   = mDisplayInfo.w;
    mScreenHeight  = mDisplayInfo.h;

    mMaxWidth      = mScreenWidth;

    if(mScreenHeight >= 1080)
    {
        mFontScaleRatio = FONT_SIZE_UNIT * 3; //X3
        mMaxFontHeight  = mMaxFontHeight * 3;
    }
    else if (mScreenHeight >= 720)
    {
        mFontScaleRatio = FONT_SIZE_UNIT * 3 / 2; //X1.5
        mMaxFontHeight  = mMaxFontHeight * 3 / 2;
    }
    else if (mScreenHeight >= 600)
    {
        mFontScaleRatio = FONT_SIZE_UNIT * 5 / 4; //X1.25
        mMaxFontHeight  = mMaxFontHeight * 5 / 4;
    }

    mMaxHeight  = mScreenHeight;

    if(mScreenHeight >= 1080)
    {
        if(mMaxHeight <= 300)
        {
            mMaxHeight = 300;
        }
    }
    else
    {
        if(mMaxHeight <= 300)
        {
            mMaxHeight = 300;
        }
    }

    if(mMaxWidth >= mScreenWidth)
    {
        mPosX = 0;
    }
    else
    {
        mPosX = (mScreenWidth - mMaxWidth)>>1;
    }

    if(mMaxHeight >= mScreenHeight)
    {
        mPosY = 0;
    }
    else
    {
        mPosY = (mScreenHeight - mMaxHeight)* (100 - userSetYpercent) / 100;
    }
    String8 name;
    const size_t SIZE = 128;
    char buffer[SIZE];
    snprintf(buffer, SIZE, "<pid_%d>", getpid());
    name.append(buffer);
    mSurfaceControl = mClient->createSurface(name, mMaxWidth, mMaxHeight,
                   PIXEL_FORMAT_TRANSPARENT, ISurfaceComposerClient::eFXSurfaceNormal);

    if(mSurfaceControl != NULL)
    {
        SurfaceComposerClient::openGlobalTransaction();
        mTopBaseLayer    = TOPBASELAYER * LAYER_MULTIPLIER + LAYER_OFFSET + 2 + index;
        mBottomBaseLayer= BOTTOMBASELAYER * LAYER_MULTIPLIER + LAYER_OFFSET  - 10 + index;
        mLayer         = mTopBaseLayer;
        logd("3@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d %d", mPosX, mPosY);
        mSurfaceControl->setPosition(mPosX,mPosY);
        //mSurfaceControl->setLayer(mLayer);
        mSurfaceControl->setLayer(99999);
        mSurfaceControl->hide();
        SurfaceComposerClient::closeGlobalTransaction();

        mCanvas      = new SkCanvas;

        mWidth   = mMaxWidth;
        mHeight = mMaxHeight;

        mPaint.setAntiAlias(true);
        logd("=**********************8 mFontScaleRatio = %d", mFontScaleRatio);
        if(userSetFontSize == 0)
        {
            mPaint.setTextSize(DEFAULT_TEXT_SIZE * mFontScaleRatio / FONT_SIZE_UNIT);
        }
        else
        {
            mPaint.setTextSize(userSetFontSize * mFontScaleRatio / FONT_SIZE_UNIT);
        }

        if(userSetFontColor == 0)
        {
            mPaint.setColor(0xFFFFFFFF);
        }
        else
        {
            mPaint.setColor(userSetFontColor);
        }
        if(userSetFontStyle == 0)
        {
            mFontStyle = SUB_RENDER_STYLE_NONE;
            setFontStyle(SUB_RENDER_STYLE_NONE);
        }
        else
        {
            setFontStyle(userSetFontStyle);
        }
        mPaint.setTextAlign(SkPaint::kCenter_Align);
        mBackColor = TRANSPARENT_COLOR;
#if (CONF_ANDROID_MAJOR_VER >= 5)
        mTypeface  =
        SkTypeface::CreateFromFile("/system/fonts/DroidSansFallbackAndroid44.ttf");
#else
        mTypeface  = SkTypeface::CreateFromFile("/system/fonts/DroidSansFallback.ttf");
#endif
        mPaint.setTypeface(mTypeface);
        mCharset     = SUBTITLE_TEXT_FORMAT_GBK;
        mTextBox     = NULL;
        mSubMode     = 0;
        mDispSubInfo = NULL;
        mSubShowFlag = SUB_SHOW_INVALID;
    }
}
//*************************************************************************//
//************************************************************************ //
CedarXSub::~CedarXSub()
{
    if(mTypeface != NULL)
    {
        mTypeface->unref();
    }

    logd("CedarXSub::~CedarXSub1!\n");

    if(mCanvas != NULL)
    {
        mCanvas->unref();
    }

    logd("CedarXSub::~CedarXSub2!\n");

    if(mTextBox != NULL)
    {
        delete mTextBox;
    }

    logd("CedarXSub::~CedarXSub3!\n");

    if (SurfaceControl::isValid(mSurfaceControl))
    {
        mSurfaceControl->clear();
    }

    logd("CedarXSub::~CedarXSub4!\n");
}
//***************************************************************************//
//************************************************************************** //

int CedarXSub::startRender()
{
#if ((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER == 2))
    Surface::SurfaceInfo    info;
    status_t             err;
    SkBitmap             mBitmap;
    sp<Surface>            mSurface;

    mSurface   = mSurfaceControl->getSurface();
    err = mSurface->lock(&info, NULL);
    if (err < 0)
    {
       logw("get surface information failed!\n");

       return  -1;
    }

    ssize_t bpr = info.s * bytesPerPixel(info.format);
    mBitmap.setConfig(convertPixelFormat(info.format), info.w, info.h, bpr);
    if (info.format == PIXEL_FORMAT_RGBX_8888)
    {
        mBitmap.setIsOpaque(true);
    }

    if (info.w > 0 && info.h > 0)
    {
        //bitmap.setPixels(info.bits+4*(mStarty*mScreenWidth+mStartx));
        mBitmap.setPixels(info.bits);
    }
    else
    {
        // be safe with an empty bitmap.
        mBitmap.setPixels(NULL);
    }

    mCanvas->setBitmapDevice(mBitmap);

    mSaveCount = mCanvas->save();

    return  NO_ERROR;
#else
    ANativeWindow_Buffer nativeWindowBuffer;
    status_t             err;
    SkBitmap             mBitmap;
    sp<Surface>            mSurface;
    mSurface   = mSurfaceControl->getSurface();
    err = mSurface->lock(&nativeWindowBuffer, NULL/*false*/);
    if (err < 0)
    {
        logw("get surface information failed!\n");

        return  -1;
    }
    ssize_t bpr = nativeWindowBuffer.width * bytesPerPixel(nativeWindowBuffer.format);
    mBitmap.setConfig(convertPixelFormat(nativeWindowBuffer.format),
                                         nativeWindowBuffer.width,
                                         nativeWindowBuffer.height, bpr);

    if (nativeWindowBuffer.format == PIXEL_FORMAT_RGBX_8888)
    {
        mBitmap.setIsOpaque(true);
    }

    if (nativeWindowBuffer.width > 0 && nativeWindowBuffer.height > 0)
    {
        //bitmap.setPixels(info.bits+4*(mStarty*mScreenWidth+mStartx));
        mBitmap.setPixels(nativeWindowBuffer.bits);
    }
    else
    {
        // be safe with an empty bitmap.
        mBitmap.setPixels(NULL);
    }

    mCanvas->setBitmapDevice(mBitmap);

    mSaveCount = mCanvas->save();
    return  NO_ERROR;
#endif
}

int CedarXSub::endRender()
{
    status_t             err;
    sp<Surface>            mSurface;

    mSurface   = mSurfaceControl->getSurface();

    // detach the canvas from the surface
    mCanvas->restoreToCount(mSaveCount);
    mSaveCount = 0;

    // unlock surface
    err = mSurface->unlockAndPost();
    if (err < 0)
    {
        logw("surface unlockAndPost Failed!\n");

        return -1;
    }

    return NO_ERROR;
}
//************************************************************//
//*********************************************************** //

//************************************************************************//
//*********************************************************************** //
int CedarXSub::needModifyBoxInf(SkScalar textBoxHeight)
{
    int aglignMent= 0;
    int offset = 0;

    if(mTextHeight <= textBoxHeight)
    {
        return NO_ERROR;             // not need modify text box vertical info
    }
    else
    {
        aglignMent = (mAlignment & 0xf0);
        if(aglignMent == SUB_RENDER_VALIGN_TOP)
        {
            if((SkScalar)(mScreenHeight-mStarty) > mTextHeight)
            {
                mEndy = mStarty + mTextHeight;
                if(mEndy >= mScreenHeight)
                {
                    mEndy = mScreenHeight;
                }
            }
            else
            {
                mEndy = mScreenHeight;
                mStarty = mEndy - mTextHeight;
                if(mStarty < 0)
                {
                    mStarty = 0;
                }
            }
        }
        else if(aglignMent == SUB_RENDER_VALIGN_CENTER)
        {
            offset = (mTextHeight - textBoxHeight)/2;
            mStarty -= offset;
            mEndy += offset;
            if(mStarty < 0)
            {
                mStarty = 0;
            }
            if(mEndy > mScreenHeight)
            {
               mEndy = mScreenHeight;
            }
        }
        else if(aglignMent == SUB_RENDER_VALIGN_BOTTOM)
        {
            if((SkScalar)mEndy > mTextHeight)
            {
                mStarty = mEndy - mTextHeight;
                if(mStarty < 0)
                {
                    mStarty = 0;
                }
            }
            else
            {
                mStarty = 0;
                mEndy = mStarty + mTextHeight;
                if(mEndy > mScreenHeight)
                {
                    mEndy = mScreenHeight;
                }
            }
        }
    }
    return NO_ERROR;
}

//*************************************************************//
//************************************************************ //
int CedarXSub::updatePara(sub_item_inf *sub_info, int initSetFontColor,
                        int initSetFontSize, int initSetFontStyle, int Ypercent)
{
    int     len = 0;
    int   fontIndex = 0;
    SkScalar textBoxHeight;

    if(sub_info == NULL)
    {
        logw("input para error!\n");

        return    -1;
    }

    mSubMode = 0;

    if(sub_info->subMode == 0)
    {
        mSubMode |= TEXT_SUBTITLE;
        convertUniCode(sub_info);
    }
    else
    {
        mSubMode |= PIC_SUBTITLE;

        generateBitmap(sub_info);
    }

    if(mSubMode & TEXT_SUBTITLE)
    {
        logv("CedarXSub::updatePara mText = %s\n",mText);
        if(mTextBox == NULL)
        {
            mTextBox = new CedarXSubTextBox;
            mTextAlign |= SUB_RENDER_HALIGN_CENTER;
            mTextAlign |= SUB_RENDER_VALIGN_BOTTOM;
        }
        if(sub_info->subHasFontInfFlag == 1)
        {
            if((initSetFontSize==SUB_INIT_SET_FONT_INFO)&&(sub_info->fontSize>0))
            {
                #if 1
                fontIndex = (sub_info->fontSize>>2)-2;
                fontIndex = (fontIndex<0)? 0:fontIndex;
                fontIndex = (fontIndex>16)? 16:fontIndex;
                fontIndex += 16;
                fontIndex = (fontIndex<25)? 25: fontIndex;
                #else
                fontIndex = sub_info->fontSize;
                #endif

                setFontSize(fontIndex);
            }

            if(mDispSubInfo->subEffectFlag==SUB_RENDER_EFFECT_KARAOKE)
            {
                if(sub_info->secondaryColor > 0)
                {
                    setTextColor(sub_info->secondaryColor);
                }
            }
            else
            {
                if((initSetFontColor==SUB_INIT_SET_FONT_INFO)&&(sub_info->primaryColor>0))
                {
                    setTextColor(sub_info->primaryColor);
                }
                if((initSetFontStyle==SUB_INIT_SET_FONT_INFO)
                    &&(sub_info->subStyle != SUB_RENDER_STYLE_NONE))
                {
                    setFontStyle(sub_info->subStyle);
                }
            }
        }

        len   = strlen((char*)mText);

        mTextBox->getTextVerInf(mText,len,mPaint,&mTextHeight,&mTextWidth, &textBoxHeight,
                              mStartx, mEndx, mStarty, mEndy, mDispSubInfo->subEffectFlag);
        needModifyBoxInf(textBoxHeight);
        setPosition(mStartx,mStarty -50);
        mTextBox->subSetBox(0,0,SkIntToScalar(mEndx-mStartx),SkIntToScalar(mEndy-mStarty));
        setTextAlign(mAlignment);
    }
    else
    {
        mPosY   = (mScreenHeight-mMaxFontHeight)* (Ypercent) / 100;
        logd("~~ mPosY = %d, mScreenHeight = %d, mMaxFontHeight = %d",
           mPosY, mScreenHeight, mMaxFontHeight);

        mSurfaceControl->setPosition(mPosX,-mPosY);
    }
    if((mDispSubInfo->subEffectFlag==SUB_RENDER_EFFECT_NONE)
        ||(mDispSubInfo->subEffectFlag==SUB_RENDER_EFFECT_KARAOKE))
    {
        startRender();
        render();
        endRender();
    }
    return NO_ERROR;
}

//*********************************************************************//
//********************************************************************* //
int   CedarXSubRender::updateSubPara(sub_item_inf *sub_info)
{
    sub_pre = sub_info;
    int              count = 0;
    size_t         size;
    sub_item_inf    *current;
    sp<CedarXSub>   cedarXSub;
    int             dispStartx = 0;
    int             dispStarty = 0;
    int             dispHeight = 0;
    int             align      = 0;
    int             lastPosValidFlag = 0;
    int             startx = 0;
    int             starty = 0;
    int             endx   = 0;
    int             endy   = 0;
    int             lastDispx = 0;
    int             lastDispy = 0;
    int             newShowSubFlag = 0;

    if(sub_info == NULL)
    {
        logw("Invalid sub information!\n");

        return  -1;
    }

    current = sub_info;

    size   = mCedarXSubs.size();
    newShowSubFlag = 1;

    while(current != NULL)
    {
        if(count > 0)
        {
            mCedarXSubs[count-1]->getTextBox(&startx, &starty, &endx, &endy,
                                    &lastDispx, &lastDispy);
        }

        if(count >= (int)size)
        {
            cedarXSub = new CedarXSub(0, mUserSetFontColor, mUserSetFontSize,
                                mUserSetFontStyle, mUserSetYPercent);

            /*create main cedarXSub*/
            mCedarXSubs.add(cedarXSub);
        }

        if(mCedarXSubs[count]->getSubShowFlag() == SUB_SHOW_INVALID)
        {
            if(current == NULL)
            {
               logd("***********current is null **********");
            }
            logv("@@startx = %d, starty = %d, endx = %d, \
                    endx = %d,lastDispx = %d lastDispy = %d, newShowSubFlag = %d",
                    startx, starty, endx, endy, lastDispx, lastDispy, newShowSubFlag);
            mCedarXSubs[count]->setSubInf(current, startx, starty, endx, endy, lastDispx,
                                    lastDispy, newShowSubFlag, mUserSetYPercent);
            mCedarXSubs[count]->updatePara(current, mInitSetFontColor, mInitSetFontSize,
                                     mInitSetFontStyle, mUserSetYPercent);
            current = (sub_item_inf *)current->nextSubItem;
            newShowSubFlag = 0;
        }
        count++;
    }

    return NO_ERROR;
}


//**********************************************************//
//**********************************************************//

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SubCtrlContext
{
    SubCtrl               base;
    CedarXSubRender*      pCedarXSubRender;
}SubCtrlContext;


static int __Destory(SubCtrl* s)
{
    SubCtrlContext* sc = (SubCtrlContext*)s;

    delete sc->pCedarXSubRender;

    sc->pCedarXSubRender = NULL;

    free(sc);

    return 0;
}

static int __Show(SubCtrl* s, SubtitleItem *pItem, unsigned int id)
{
    SubCtrlContext* sc = (SubCtrlContext*)s;
    (void)id;

    sub_item_inf dispSubItem ;
    memset(&dispSubItem, 0, sizeof(sub_item_inf));

    dispSubItem.subMode = 0;
    dispSubItem.startx  = pItem->nStartX; // the invalid value is -1
    dispSubItem.starty  = pItem->nStartY; // the invalid value is -1
    dispSubItem.endx    = pItem->nEndX;   // the invalid value is -1
    dispSubItem.endy    = pItem->nEndY;   // the invalid value is -1
    dispSubItem.subTextBuf = (unsigned char*)pItem->pText;// the data buffer of the text subtitle
    dispSubItem.subTextLen = (unsigned int)pItem->nTextLength;// the length of the text subtitle
    dispSubItem.encodingType = pItem->eTextFormat;// the encoding tyle of the text subtitle

    dispSubItem.subBitmapBuf = (unsigned char*)pItem->pBitmapData;

    if(dispSubItem.subTextBuf)
    {
        dispSubItem.subMode = 0;
    }
    else if(dispSubItem.subBitmapBuf)
    {
        dispSubItem.subMode = 1;
    }
    dispSubItem.subPicWidth = pItem->nBitmapWidth;
    dispSubItem.subPicHeight = pItem->nBitmapHeight;
    dispSubItem.subDispPos = pItem->eAlignment;

    sc->pCedarXSubRender->updateSubPara(&dispSubItem);
    return sc->pCedarXSubRender->cedarxSubShow();
}


static int __Hide(SubCtrl* s, unsigned int id)
{
    unsigned int systemTime = 0xffffffff;
    unsigned int *hasSubShowFlag = NULL;
    SubCtrlContext* sc = (SubCtrlContext*)s;
    (void)id;

    return sc->pCedarXSubRender->cedarxSubHide(systemTime, hasSubShowFlag);
}

static SubControlOpsT mSubCtrl =
{
    .destory = __Destory,
    .show    = __Show,
    .hide    = __Hide,
};

SubCtrl* SubNativeRenderCreate()
{
    logd("****************SubRenderCreate!****************\n");
    SubCtrlContext* sc = (SubCtrlContext*)malloc(sizeof(SubCtrlContext));
    if(sc == NULL)
    {
        return NULL;
    }
    memset(sc, 0x00, sizeof(SubCtrlContext));

    sc->pCedarXSubRender = new CedarXSubRender();
    sc->base.ops = &mSubCtrl;

    return &sc->base;
}

#ifdef __cplusplus
}
#endif
} // namespace android
