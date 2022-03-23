
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : pixelformat.c
* Description :display engine 2.0 rotation processing base functions implement
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "vdecoder.h"
#include "log.h"

//* should include other  '*.h'  before CdcMalloc.h
#include "CdcMalloc.h"

#define ENABLE_ROTATE_BY_NEON (0)

/*******************************************************************************
Function name: map32x32_to_yuv_Y
Description:
    1. we should know : vdecbuf_uv is 32*32 align too
    2. must match gpuBuf_uv size.
    3. gpuBuf_uv size is half of gpuBuf_y size
    4. we guarantee: vdecbufSize>=gpuBufSize
    5. uv's macroblock is also 16*16. Vdec_macroblock is also twomb(32*32).
    6. if coded_width is stride/2, we can support gpu_buf_width 16byte align and 8byte align!
       But need outer set right value of coded_width, must meet gpu_uv_buf_width align's request!
Parameters:
    1. mode = 0:yv12, 1:thumb yuv420p
    2. coded_width and coded_height is uv size, already half of y_size
Return:

*******************************************************************************/
#ifdef CEDARX_DECODER_ARM32
void ConvertMb32420ToNv21Y(char* pSrc,char* pDst,int nWidth, int nHeight)
{
    int nMbWidth = 0;
    int nMbHeight = 0;
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char bufferU[32];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nLineStride = (nWidth + 15) &~15;
    nMbWidth = (nWidth+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;
    ptr = pSrc;

    nWidthMatchFlag = 0;
    nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*32 == nLineStride)
    {
        nWidthMatchFlag = 1;
        nCopyMbWidth = nMbWidth;

    }
    for(i=0; i<nMbHeight; i++)
    {
        char *dstAsm = NULL;
        char *srcAsm = NULL;
        CEDARC_UNUSE(dstAsm);
        CEDARC_UNUSE(srcAsm);
        for(j=0; j<nCopyMbWidth; j++)
        {
            for(m=0; m<32; m++)
            {
                if((i*32 + m) >= nHeight)
                  {
                    ptr += 32;
                    continue;
                  }
                srcAsm = ptr;
                lineNum = i*32 + m;           //line num
                offset =  lineNum*nLineStride + j*32;
                dstAsm = pDst+ offset;

                 asm volatile (
                                "vld1.8         {d0 - d3}, [%[srcAsm]]              \n\t"
                                "vst1.8         {d0 - d3}, [%[dstAsm]]              \n\t"
                                   : [dstAsm] "+r" (dstAsm), [srcAsm] "+r" (srcAsm)
                                   :  //[srcY] "r" (srcY)
                                   : "cc", "memory", "d0", "d1", "d2", "d3", \
                                    "d4", "d5", "d6", "d16", "d17", "d18",  \
                                    "d19", "d20", "d21", "d22", "d23",  \
                                    "d24", "d28", "d29", "d30", "d31");
                ptr += 32;
            }
        }

        if(nWidthMatchFlag == 1)
        {
            continue;
        }
        for(m=0; m<32; m++)
        {
            if((i*32 + m) >= nHeight)
            {
                ptr += 32;
                continue;
               }
            dstAsm = bufferU;
            srcAsm = ptr;
             lineNum = i*32 + m;           //line num
            offset =  lineNum*nLineStride + j*32;

                asm volatile (
                      "vld1.8         {d0 - d3}, [%[srcAsm]]              \n\t"
                      "vst1.8         {d0 - d3}, [%[dstAsm]]              \n\t"
                         : [dstAsm] "+r" (dstAsm), [srcAsm] "+r" (srcAsm)
                         :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", \
                            "d4", "d5", "d6", "d16", "d17", "d18", \
                            "d19", "d20", "d21", "d22", "d23", \
                            "d24", "d28", "d29", "d30", "d31");
               ptr += 32;
               for(k=0; k<32; k++)
               {
                   if((j*32+ k) >= nLineStride)
                      {
                       break;
                     }
                    pDst[offset+k] = bufferU[k];
               }
        }
    }
}

void ConvertMb32420ToNv21C(char* pSrc,char* pDst,int nPicWidth, int nPicHeight)
{
    int nMbWidth = 0;
    int nMbHeight = 0;
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char bufferV[16], bufferU[16];
    int nWidth = 0;
    int nHeight = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth*2 + 15) &~15;
    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
        char *dst0Asm = NULL;
        char *dst1Asm = NULL;
        char *srcAsm = NULL;
        CEDARC_UNUSE(dst0Asm);
        CEDARC_UNUSE(dst1Asm);
        CEDARC_UNUSE(srcAsm);
        for(j=0; j<nMbWidth; j++)
        {
            for(m=0; m<32; m++)
            {
                if((i*32 + m) >= nHeight)
                {
                    ptr += 32;
                    continue;
                }

                dst0Asm = bufferU;
                dst1Asm = bufferV;
                srcAsm = ptr;
                lineNum = i*32 + m;           //line num
                offset =  lineNum*nLineStride + j*32;

                asm volatile(
                        "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                        : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                        :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", \
                            "d4", "d5", "d6", "d16", "d17", "d18", \
                            "d19", "d20", "d21", "d22", "d23", "d24", \
                            "d28", "d29", "d30", "d31"
                     );
                ptr += 32;

                for(k=0; k<16; k++)
                {
                    if((j*32+ 2*k) >= nLineStride)
                    {
                        break;
                    }
                    pDst[offset+2*k]   = bufferV[k];
                       pDst[offset+2*k+1] = bufferU[k];
                }
            }
        }
    }
}

void ConvertMb32422ToNv21C(char* pSrc,char* pDst,int nPicWidth, int nPicHeight)
{
    int nMbWidth = 0;
    int nMbHeight = 0;
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    char bufferV[16], bufferU[16];
    int nWidth = 0;
    int nHeight = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth*2 + 15) &~15;
    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight*2+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    for(i=0; i<nMbHeight; i++)
    {
        char *dst0Asm = NULL;
        char *dst1Asm = NULL;
        char *srcAsm = NULL;
        CEDARC_UNUSE(dst0Asm);
        CEDARC_UNUSE(dst1Asm);
        CEDARC_UNUSE(srcAsm);
        for(j=0; j<nMbWidth; j++)
        {
            for(m=0; m<16; m++)
            {
                if((i*16 + m) >= nHeight)
                {
                    ptr += 64;
                    continue;
                }

                dst0Asm = bufferU;
                dst1Asm = bufferV;
                srcAsm = ptr;
                lineNum = i*16 + m;           //line num
                offset =  lineNum*nLineStride + j*32;

                asm volatile(
                        "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                        : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                        :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", \
                            "d5", "d6", "d16", "d17", "d18", "d19", "d20", \
                            "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                     );
                ptr += 64;

                for(k=0; k<16; k++)
                {
                    if((j*32+ 2*k) >= nLineStride)
                    {
                        break;
                    }
                    pDst[offset+2*k]   = bufferV[k];
                       pDst[offset+2*k+1] = bufferU[k];
                }
            }
        }
    }

}

void ConvertMb32420ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
    int nMbWidth = 0;
    int nMbHeight = 0;
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;
    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    //nLineStride = ((nPicWidth+ 15) &~15)/2;
    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    nWidthMatchFlag = 0;
    nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*16 == nLineStride)
    {
        nWidthMatchFlag = 1;
        nCopyMbWidth = nMbWidth;
    }

    for(i=0; i<nMbHeight; i++)
    {
        char *dst0Asm = NULL;
        char *dst1Asm = NULL;
        char *srcAsm = NULL;
        CEDARC_UNUSE(dst0Asm);
        CEDARC_UNUSE(dst1Asm);
        CEDARC_UNUSE(srcAsm);
        for(j=0; j<nCopyMbWidth; j++)
        {
            for(m=0; m<32; m++)
            {
                if((i*32 + m) >= nHeight)
                {
                    ptr += 32;
                    continue;
                }

                srcAsm = ptr;
                lineNum = i*32 + m;           //line num
                offset =  lineNum*nLineStride + j*16;
                dst0Asm = pDstU+offset;
                dst1Asm = pDstV+offset;
                asm volatile(
                        "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                        : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                        :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", \
                            "d4", "d5", "d6", "d16", "d17", "d18", \
                            "d19", "d20", "d21", "d22", "d23", "d24",  \
                            "d28", "d29", "d30", "d31"
                     );
                ptr += 32;
            }
        }

        if(nWidthMatchFlag == 1)
        {
            continue;
        }
        for(m=0; m<32; m++)
        {
            if((i*32 + m) >= nHeight)
            {
                ptr += 32;
                continue;
            }

               srcAsm = ptr;
               lineNum = i*32 + m;           //line num
            offset =  lineNum*nLineStride + j*16;
               dst0Asm = bufferU;
            dst1Asm = bufferV;
               asm volatile(
                       "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                    "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                     "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                       : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                    :  //[srcY] "r" (srcY)
                    : "cc", "memory", "d0", "d1", "d2", "d3", "d4", \
                        "d5", "d6", "d16", "d17", "d18", "d19",  \
                        "d20", "d21", "d22", "d23", "d24", "d28", \
                        "d29", "d30", "d31"
                 );
               ptr += 32;

               for(k=0; k<16; k++)
               {
                   if((j*16+ k) >= nLineStride)
                   {
                       break;
                   }
                   pDstV[offset+k] = bufferV[k];
                   pDstU[offset+k] = bufferU[k];
               }
        }
    }
}

void ConvertMb32422ToYv12C(char* pSrc,char* pDstU, char*pDstV,int nPicWidth, int nPicHeight)
{
    int nMbWidth = 0;
    int nMbHeight = 0;
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 0;
    int k = 0;
    int nLineStride=0;
    int lineNum = 0;
    int offset = 0;
    int maxNum = 0;
    char* ptr = NULL;

    int nWidth = 0;
    int nHeight = 0;
    char bufferV[16], bufferU[16];
    int nWidthMatchFlag = 0;
    int nCopyMbWidth = 0;

    nWidth = (nPicWidth+1)/2;
    nHeight = (nPicHeight+1)/2;

    nLineStride = (nWidth+7)&~7;

    nMbWidth = (nWidth*2+31)&~31;
    nMbWidth /= 32;

    nMbHeight = (nHeight*2+31)&~31;
    nMbHeight /= 32;

    ptr = pSrc;

    nWidthMatchFlag = 0;
    nCopyMbWidth = nMbWidth-1;

    if(nMbWidth*16 == nLineStride)
    {
        nWidthMatchFlag = 1;
        nCopyMbWidth = nMbWidth;
    }

    for(i=0; i<nMbHeight; i++)
    {
        char *dst0Asm = NULL;
        char *dst1Asm = NULL;
        char *srcAsm = NULL;
        CEDARC_UNUSE(dst0Asm);
        CEDARC_UNUSE(dst1Asm);
        CEDARC_UNUSE(srcAsm);
        for(j=0; j<nCopyMbWidth; j++)
        {
            for(m=0; m<16; m++)
            {
                if((i*16 + m) >= nHeight)
                {
                    ptr += 64;
                    continue;
                }

                srcAsm = ptr;
                lineNum = i*16 + m;           //line num
                offset =  lineNum*nLineStride + j*16;
                dst0Asm = pDstU+offset;
                dst1Asm = pDstV+offset;

                asm volatile(
                        "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                        "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                        : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                        :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", \
                            "d5", "d6", "d16", "d17", "d18", "d19", "d20", \
                            "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                     );
                ptr += 64;
            }
        }

        if(nWidthMatchFlag==1)
        {
            continue;
        }
        for(m=0; m<16; m++)
        {
            if((i*16 + m) >= nHeight)
            {
                ptr += 64;
                continue;
            }

            dst0Asm = bufferU;
            dst1Asm = bufferV;
               srcAsm = ptr;
               lineNum = i*16 + m;           //line num
            offset =  lineNum*nLineStride + j*16;

               asm volatile(
                        "vld2.8         {d0-d3}, [%[srcAsm]]              \n\t"
                         "vst1.8         {d0,d1}, [%[dst0Asm]]              \n\t"
                        "vst1.8         {d2,d3}, [%[dst1Asm]]              \n\t"
                        : [dst0Asm] "+r" (dst0Asm), [dst1Asm] "+r" (dst1Asm), [srcAsm] "+r" (srcAsm)
                        :  //[srcY] "r" (srcY)
                        : "cc", "memory", "d0", "d1", "d2", "d3", "d4", \
                            "d5", "d6", "d16", "d17", "d18", "d19", "d20", \
                            "d21", "d22", "d23", "d24", "d28", "d29", "d30", "d31"
                        );
               ptr += 64;

               for(k=0; k<16; k++)
            {
                   if((j*16+ k) >= nLineStride)
                   {
                       break;
                   }
                pDstV[offset+k] = bufferV[k];
                      pDstU[offset+k] = bufferU[k];
            }
        }
    }
}
#endif

//****************************************************************//
//****************************************************************//

void ConvertPixelFormat(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
    int   nMemSizeY = 0;
    int   nMemSizeC = 0;
    int   nLineStride = 0;
    int   nHeight16Align = 0;
    //int   nHeight32Align = 0;
    //int   nHeight64Align = 0;
    int   nHeight = 0;
    int   i = 0;
    int      j = 0;

    nHeight        = pPictureIn->nHeight;
    nLineStride    = (pPictureIn->nWidth+15) &~15;
    nHeight16Align = (nHeight+15) & ~15;
    //nHeight32Align = (nHeight+31) & ~31;
    //nHeight64Align = (nHeight+63) & ~63;

    pPictureOut->nLineStride = (pPictureIn->nWidth+15) & ~15;
    pPictureOut->nHeight     = pPictureIn->nHeight;
    pPictureOut->nWidth      = pPictureIn->nWidth;
    pPictureOut->nTopOffset  = pPictureIn->nTopOffset;
    pPictureOut->nBottomOffset  = pPictureIn->nBottomOffset;
    pPictureOut->nLeftOffset  = pPictureIn->nLeftOffset;
    pPictureOut->nRightOffset  = pPictureIn->nRightOffset;

    if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
    {
#ifdef CEDARX_DECODER_ARM32
        if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            nMemSizeC = nMemSizeY>>2;

            ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, \
                    pPictureIn->nWidth, pPictureIn->nHeight);
            ConvertMb32420ToYv12C(pPictureIn->pData1, \
                    (char*)(pPictureOut->pData0+nMemSizeY+nMemSizeC), \
                    (char*)(pPictureOut->pData0+nMemSizeY), \
                    pPictureIn->nWidth, pPictureIn->nHeight);

        }
        else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            nMemSizeC = nMemSizeY>>2;
            ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, \
                    pPictureIn->nWidth, pPictureIn->nHeight);
            ConvertMb32422ToYv12C(pPictureIn->pData1, \
                    (char*)(pPictureOut->pData0+nMemSizeY+nMemSizeC), \
                    (char*)(pPictureOut->pData0+nMemSizeY), \
                    pPictureIn->nWidth, pPictureIn->nHeight);
        }
#endif

        if(pPictureIn->ePixelFormat == PIXEL_FORMAT_NV21)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;
            pPictureOut->pData2 = pPictureOut->pData1 + nMemSizeC;
            pPictureIn->pData1  = pPictureIn->pData0 + nMemSizeY;

            for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
            {
                for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
                {
                    pPictureOut->pData1[i*nLineStride/2+j] = \
                        pPictureIn->pData1[i*nLineStride+2*j];
                    pPictureOut->pData2[i*nLineStride/2+j] = \
                        pPictureIn->pData1[i*nLineStride+2*j+1];
                }
            }
        }
        else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
            nMemSizeC = nMemSizeY>>2;
            memcpy(pPictureOut->pData0+nMemSizeY, \
                pPictureIn->pData0+nMemSizeY+nMemSizeC, nMemSizeC);
            memcpy(pPictureOut->pData0+nMemSizeY+nMemSizeC, \
                pPictureIn->pData0+nMemSizeY, nMemSizeC);
        }
    }
    else if(pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)
    {
#ifdef CEDARX_DECODER_ARM32
        if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            //nMemSizeY = nLineStride*nHeight;
            nMemSizeC = nMemSizeY>>2;
            ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, \
                    pPictureIn->nWidth, pPictureIn->nHeight);
            ConvertMb32420ToNv21C(pPictureIn->pData1, \
                    pPictureOut->pData0+nMemSizeY, \
                    pPictureIn->nWidth, pPictureIn->nHeight);
        }
        else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            nMemSizeC = nMemSizeY>>2;
            ConvertMb32420ToNv21Y(pPictureIn->pData0,pPictureOut->pData0, \
                pPictureIn->nWidth, pPictureIn->nHeight);
            ConvertMb32422ToNv21C(pPictureIn->pData1, \
                (char*)(pPictureOut->pData0+nMemSizeY), \
                pPictureIn->nWidth, pPictureIn->nHeight);
        }
#endif
        if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YV12)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
            nMemSizeC = nMemSizeY>>2;
            pPictureIn->pData1 = pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2 = pPictureIn->pData1 + nMemSizeC;
            pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;

            for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
            {
                for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
                {
                    pPictureOut->pData1[i*nLineStride+2*j]   = \
                        pPictureIn->pData1[i*nLineStride/2+j];
                    pPictureOut->pData1[i*nLineStride+2*j+1] = \
                        pPictureIn->pData2[i*nLineStride/2+j];
                }
            }
        }
        else if(pPictureIn->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420)
        {
            nMemSizeY = nLineStride*nHeight16Align;
            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
            nMemSizeC = nMemSizeY>>2;
            pPictureIn->pData1 = pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2 = pPictureIn->pData1 + nMemSizeC;
            pPictureOut->pData1 = pPictureOut->pData0 + nMemSizeY;

            for(i=0; i<(pPictureIn->nHeight+1)/2; i++)
            {
                for(j=0; j<(pPictureIn->nWidth+1)/2; j++)
                {
                    pPictureOut->pData1[i*nLineStride+2*j]   = \
                        pPictureIn->pData2[i*nLineStride/2+j];
                    pPictureOut->pData1[i*nLineStride+2*j+1] = \
                        pPictureIn->pData1[i*nLineStride/2+j];
                 }
            }
        }
    }
    //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
}

int RotatePicture0Degree(VideoPicture* pPictureIn,
            VideoPicture* pPictureOut,
            int nGpuYAlign,
            int nGpuCAlign)
{
    int nMemSizeY = 0;
    int nMemSizeC = 0;
    int nLineStride = 0;
    int ePixelFormat = 0;
    int nHeight16Align = 0;
    int nHeight32Align = 0;
    int nHeight64Align = 0;

    nLineStride    = pPictureIn->nLineStride;
    ePixelFormat   = pPictureIn->ePixelFormat;
    nHeight16Align = (pPictureIn->nHeight+15) &~15;
    nHeight32Align = (pPictureIn->nHeight+31) &~31;
    nHeight64Align = (pPictureIn->nHeight+63) &~63;

    switch(pPictureOut->ePixelFormat)
    {
        case PIXEL_FORMAT_YUV_PLANER_420:
        case PIXEL_FORMAT_YUV_PLANER_422:
        case PIXEL_FORMAT_YUV_PLANER_444:
        case PIXEL_FORMAT_YV12:
        case PIXEL_FORMAT_NV21:
            //* for decoder,
            //* height of Y component is required to be 16 aligned,
            //* for example, 1080 becomes to 1088.
            //* width and height of U or V component are both required to be 8 aligned.

            //* nLineStride should be 16 aligned.
            nMemSizeY = nLineStride*nHeight16Align;
            if(ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 ||
                    ePixelFormat == PIXEL_FORMAT_YV12 ||
                    ePixelFormat == PIXEL_FORMAT_NV21)
                nMemSizeC = nMemSizeY>>2;
            else if(ePixelFormat == PIXEL_FORMAT_YUV_PLANER_422)
                nMemSizeC = nMemSizeY>>1;
            else
                nMemSizeC = nMemSizeY;  //* PIXEL_FORMAT_YUV_PLANER_444

            //* copy relay on gpuYAlign and gpuCAlign if the format is YV12
            if(pPictureOut->ePixelFormat == PIXEL_FORMAT_YV12)
            {
                //* we can memcpy directly if gpuYAlign is 16 and gpuCAlign is 8
                if(nGpuYAlign == 16 && nGpuCAlign == 8)
                {
                    memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY+nMemSizeC*2);
                }
                else if(nGpuYAlign == 16 && nGpuCAlign == 16)
                {
                    int i;
                    int nCpyWidthSize = pPictureOut->nLineStride;
                    char* pDstData = pPictureOut->pData0;
                    char* pSrcData = pPictureIn->pData0;
                    //* cpy y
                    for(i = 0;i < pPictureIn->nHeight;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride;
                    }
                    //* cpy v
                    pDstData = pPictureOut->pData0 + pPictureOut->nLineStride*pPictureOut->nHeight;
                    pSrcData = pPictureIn->pData0 + nMemSizeY;
                    nCpyWidthSize = (nCpyWidthSize/2 + 15) & ~15;

                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                    //* cpy u
                    int nCSize = nCpyWidthSize*pPictureOut->nHeight/2;
                    pDstData = pPictureOut->pData0 + \
                            pPictureOut->nLineStride*pPictureOut->nHeight + nCSize;
                    pSrcData = pPictureIn->pData0 + nMemSizeY+nMemSizeC;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                }
                else if(nGpuYAlign == 32 && nGpuCAlign == 16)
                {
                    int i;
                    int nCpyWidthSize = pPictureOut->nLineStride;
                    char* pDstData = pPictureOut->pData0;
                    char* pSrcData = pPictureIn->pData0;
                    //* cpy y
                    for(i = 0;i < pPictureIn->nHeight;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth);
                        pDstData += nCpyWidthSize;
                        pSrcData += pPictureIn->nLineStride;
                    }
                    //* cpy v
                    pDstData = pPictureOut->pData0 + \
                            pPictureOut->nLineStride*pPictureOut->nHeight;
                    pSrcData = pPictureIn->pData0 + nMemSizeY;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize/2;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                    //* cpy u
                    pDstData = pPictureOut->pData0 + \
                            pPictureOut->nLineStride*pPictureOut->nHeight*5/4;
                    pSrcData = pPictureIn->pData0 + nMemSizeY+nMemSizeC;
                    for(i = 0;i < pPictureIn->nHeight/2;i++)
                    {
                        memcpy(pDstData,pSrcData,pPictureIn->nWidth/2);
                        pDstData += nCpyWidthSize/2;
                        pSrcData += pPictureIn->nLineStride/2;
                    }
                }
                else
                {
                    loge("the nGpuYAlign[%d] and nGpuCAlign[%d] is not surpport!", \
                        nGpuYAlign,nGpuCAlign);
                    return -1;
                }
            }
            else
            {
                memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY+nMemSizeC*2);
            }
            break;

        case PIXEL_FORMAT_YUV_MB32_420:
        case PIXEL_FORMAT_YUV_MB32_422:
        case PIXEL_FORMAT_YUV_MB32_444:
            //* for decoder,
            //* height of Y component is required to be 32 aligned.
            //* height of UV component are both required to be 32 aligned.
            //* nLineStride should be 32 aligned.
            nMemSizeY = nLineStride*nHeight32Align;
            if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
                nMemSizeC = nLineStride*nHeight64Align/4;
            else if(ePixelFormat == PIXEL_FORMAT_YUV_MB32_422)
                nMemSizeC = nLineStride*nHeight64Align/2;
            else
                nMemSizeC = nLineStride*nHeight64Align;

            memcpy(pPictureOut->pData0, pPictureIn->pData0, nMemSizeY);
            memcpy(pPictureOut->pData1, pPictureIn->pData1, nMemSizeC*2);
            //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY);
            //AdapterMemFlushCache((void*)pPictureOut->pData1, 2*nMemSizeC);
            break;

        default:
            loge("pixel format incorrect, ePixelFormat=%d", ePixelFormat);
            return -1;
    }
    return 0;
}

int RotatePicture90Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
    int nLeftOffset = 0;
    int nRightOffset = 0;
    int nBottomOffset = 0;
    int nTopOffset = 0;

    pPictureOut->nLineStride = (pPictureIn->nHeight+15) &~15;
    pPictureOut->nHeight = pPictureIn->nWidth;
    pPictureOut->nWidth = pPictureIn->nHeight;

    if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))
    {
        int i;
        int j;
        for(j=0; j<pPictureIn->nHeight; j++)
        {
            for(i=0; i<pPictureIn->nWidth; i++)
            {
                pPictureOut->pData0[i*pPictureOut->nLineStride+pPictureIn->nHeight-j-1] = \
                    pPictureIn->pData0[j*pPictureIn->nLineStride+i];
            }
        }

        int nHeight           = pPictureIn->nHeight/2;
        int nWidth            = pPictureIn->nWidth/2;
        int nLineStride       = pPictureIn->nLineStride/2;
        int nRotateLineStride = pPictureOut->nLineStride/2;

        int nMemSizeY = 0;
        int nMemSizeC = 0;
        if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
        {
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
            pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;

            for(j=0; j<nHeight; j++)
            {
                for(i=0; i<nWidth; i++)
                {
                    pPictureOut->pData1[i*nRotateLineStride+nHeight-j-1] = \
                        pPictureIn->pData1[j*nLineStride+i];
                    pPictureOut->pData2[i*nRotateLineStride+nHeight-j-1] = \
                        pPictureIn->pData2[j*nLineStride+i];
                }
            }
        }
        else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)|| \
            (pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
        {
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            CEDARC_UNUSE(nMemSizeC);
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;

            for(j=0; j<nHeight; j++)
            {
                for(i=0; i<nWidth; i++)
                {
                    pPictureOut->pData1[2*i*nRotateLineStride+2*nHeight-(2*j+1)-1] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i];
                    pPictureOut->pData1[2*i*nRotateLineStride+2*nHeight-(2*j)-1] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i+1];
                }
            }
        }
    }

    //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);

    nLeftOffset = pPictureIn->nLeftOffset;
    nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
    nTopOffset = pPictureIn->nTopOffset;
    nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

    pPictureOut->nLeftOffset   = nBottomOffset;
    pPictureOut->nBottomOffset = nRightOffset;
    pPictureOut->nRightOffset  = nTopOffset;
    pPictureOut->nTopOffset    = nLeftOffset;
    pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
    pPictureOut->nBottomOffset  = pPictureOut->nHeight -pPictureOut->nBottomOffset;
    return 0;
}

int RotatePicture180Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
    //int nRotateLineStride = 0;
    int nLeftOffset = 0;
    int nRightOffset = 0;
    int nBottomOffset = 0;
    int nTopOffset = 0;

    pPictureOut->nLineStride = pPictureIn->nLineStride;

    if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))
    {

        int i;
        int j;
        for(j=0; j<pPictureIn->nHeight; j++)
        {
            for(i=0; i<pPictureIn->nWidth; i++)
            {
                int index = (pPictureIn->nHeight-1-j)*pPictureIn->nLineStride+ \
                    (pPictureIn->nWidth-1-i);
                pPictureOut->pData0[index] = pPictureIn->pData0[j*pPictureIn->nLineStride+i];
            }
        }

        int nHeight           = pPictureIn->nHeight/2;
        int nWidth            = pPictureIn->nWidth/2;
        int nLineStride       = pPictureIn->nLineStride/2;

        int nMemSizeY;
        int nMemSizeC;
        if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
        {
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
            pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

            for(j=0; j<nHeight; j++)
            {
                for(i=0; i<nWidth; i++)
                {
                    pPictureOut->pData1[(nHeight-1-j)*nLineStride+(nWidth-1-i)] = \
                        pPictureIn->pData1[j*nLineStride+i];
                    pPictureOut->pData2[(nHeight-1-j)*nLineStride+(nWidth-1-i)] =  \
                        pPictureIn->pData2[j*nLineStride+i];
                }
            }
        }
        else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21)|| \
            (pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
        {
             nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
             nMemSizeC = nMemSizeY>>2;
             CEDARC_UNUSE(nMemSizeC);

             pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
             pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
             for(j=0; j<nHeight; j++)
             {
                 for(i=0; i<nWidth; i++)
                 {
                     pPictureOut->pData1[2*(nHeight-1-j)*nLineStride+2*(nWidth-1-i)] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i];
                     pPictureOut->pData1[2*(nHeight-1-j)*nLineStride+2*(nWidth-1-i)+1] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i+1];
                 }
             }
        }
    }

    //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
    pPictureOut->nHeight = pPictureIn->nHeight;
    pPictureOut->nWidth = pPictureIn->nWidth;

    nLeftOffset = pPictureIn->nLeftOffset;
    nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
    nTopOffset = pPictureIn->nTopOffset;
    nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

    pPictureOut->nLeftOffset   = nRightOffset;
    pPictureOut->nBottomOffset = nTopOffset;
    pPictureOut->nRightOffset  = nLeftOffset;
    pPictureOut->nTopOffset    = nBottomOffset;

    pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
    pPictureOut->nBottomOffset = pPictureOut->nHeight -pPictureOut->nBottomOffset;
    return 0;
}

int RotatePicture270Degree(VideoPicture* pPictureIn, VideoPicture* pPictureOut)
{
    int nLeftOffset = 0;
    int nRightOffset = 0;
    int nBottomOffset = 0;
    int nTopOffset = 0;

    pPictureOut->nLineStride = (pPictureIn->nHeight+15) &~15;
    pPictureOut->nHeight = pPictureIn->nWidth;
    pPictureOut->nWidth = pPictureIn->nHeight;

    if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV21)
            ||(pPictureOut->ePixelFormat == PIXEL_FORMAT_NV12))

    {
        int i = 0;
        int j = 0;
        for(j=0; j<pPictureIn->nHeight; j++)
        {
            for(i=0; i<pPictureIn->nWidth; i++)
            {
                pPictureOut->pData0[(pPictureIn->nWidth-1-i)*pPictureOut->nLineStride+j] = \
                    pPictureIn->pData0[j*pPictureIn->nLineStride+i];
            }
        }

        int nHeight           = pPictureIn->nHeight/2;
        int nWidth            = pPictureIn->nWidth/2;
        int nLineStride       = pPictureIn->nLineStride/2;
        int nRotateLineStride = pPictureOut->nLineStride/2;

        int nMemSizeY = 0;
        int nMemSizeC = 0;
        if(pPictureOut->ePixelFormat==PIXEL_FORMAT_YV12)
        {
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;

            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;
            pPictureIn->pData2  =  pPictureIn->pData1 + nMemSizeC;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            pPictureOut->pData1 =  pPictureOut->pData0+ nMemSizeY;
            pPictureOut->pData2 =  pPictureOut->pData1+ nMemSizeC;

            for(j=0; j<nHeight; j++)
            {
                for(i=0; i<nWidth; i++)
                {
                    pPictureOut->pData1[(nWidth-1-i)*nRotateLineStride+j] = \
                        pPictureIn->pData1[j*nLineStride+i];
                    pPictureOut->pData2[(nWidth-1-i)*nRotateLineStride+j] = \
                        pPictureIn->pData2[j*nLineStride+i];
                }
            }
        }
        else if((pPictureOut->ePixelFormat==PIXEL_FORMAT_NV21) || \
            (pPictureOut->ePixelFormat==PIXEL_FORMAT_NV12))
        {
            nMemSizeY = pPictureIn->nLineStride*((pPictureIn->nHeight+15)&~15);
            pPictureIn->pData1  =  pPictureIn->pData0 + nMemSizeY;

            nMemSizeY = pPictureOut->nLineStride*((pPictureOut->nHeight+15)&~15);
            nMemSizeC = nMemSizeY>>2;
            CEDARC_UNUSE(nMemSizeC);

            for(j=0; j<nHeight; j++)
            {
                for(i=0; i<nWidth; i++)
                {
                    pPictureOut->pData1[2*(nWidth-1-i)*nRotateLineStride+2*j] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i];
                    pPictureOut->pData1[2*(nWidth-1-i)*nRotateLineStride+2*j+1] = \
                        pPictureIn->pData1[2*j*nLineStride+2*i+1];
                }
            }
        }
    }

    //AdapterMemFlushCache((void*)pPictureOut->pData0, nMemSizeY+2*nMemSizeC);
    nLeftOffset = pPictureIn->nLeftOffset;
    nRightOffset = pPictureIn->nWidth - pPictureIn->nRightOffset;
    nTopOffset = pPictureIn->nTopOffset;
    nBottomOffset = pPictureIn->nHeight - pPictureIn->nTopOffset;

    pPictureOut->nLeftOffset   = nTopOffset;
    pPictureOut->nBottomOffset = nLeftOffset;
    pPictureOut->nRightOffset  = nBottomOffset;
    pPictureOut->nTopOffset    = nRightOffset;

    pPictureOut->nRightOffset  = pPictureOut->nWidth - pPictureOut->nRightOffset;
    pPictureOut->nBottomOffset = pPictureOut->nHeight -pPictureOut->nBottomOffset;
    return 0;
}

//* the next two rotate function with neon work ok, we can use it if necessary
#if ENABLE_ROTATE_BY_NEON
void NV21Rotate90DegreeByNeon(unsigned char* dst_addr, unsigned char* src_addr,
      unsigned int width, unsigned int height)
{
#if 1
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;

    unsigned int row_bytes = width;
    unsigned int col_bytes = height;

    unsigned int row_bytes_7 = width*7;
    unsigned int row_bytes_3 = width*3;

    unsigned char* y_src_line0 = src_addr;
    unsigned char* y_src_line1 = y_src_line0 + row_bytes;
    unsigned char* y_src_line2 = y_src_line1 + row_bytes;
    unsigned char* y_src_line3 = y_src_line2 + row_bytes;
    unsigned char* y_src_line4 = y_src_line3 + row_bytes;
    unsigned char* y_src_line5 = y_src_line4 + row_bytes;
    unsigned char* y_src_line6 = y_src_line5 + row_bytes;
    unsigned char* y_src_line7 = y_src_line6 + row_bytes;

    unsigned char* uv_src_line0 = src_addr + width*height;
    unsigned char* uv_src_line1 = uv_src_line0 + row_bytes;
    unsigned char* uv_src_line2 = uv_src_line1 + row_bytes;
    unsigned char* uv_src_line3 = uv_src_line2 + row_bytes;

    unsigned char* y_dst_line0 = dst_addr + col_bytes - 8;
    unsigned char* uv_dst_line0 = dst_addr + width*height + col_bytes - 8;

    unsigned char* y_dst_line0_static = y_dst_line0;
    unsigned char* uv_dst_line0_static = uv_dst_line0;

    for(i=0; i<height; i+=8)
    {
        for(j=0; j<width; j+=8)
        {
            asm volatile
            (
                "mov             r9,    %9          \n\t"

                "pld             [%0,#64]               \n\t"
                "pld             [%1,#64]               \n\t"
                "pld             [%2,#64]               \n\t"
                "pld             [%3,#64]               \n\t"
                "pld             [%4,#64]               \n\t"
                "pld             [%5,#64]               \n\t"
                "pld             [%6,#64]               \n\t"
                "pld             [%7,#64]               \n\t"

                "vld1.u8         {d0},  [%0]!           \n\t"
                "vld1.u8         {d1},  [%1]!           \n\t"
                "vld1.u8         {d2},  [%2]!           \n\t"
                "vld1.u8         {d3},  [%3]!           \n\t"
                "vld1.u8         {d4},  [%4]!           \n\t"
                "vld1.u8         {d5},  [%5]!           \n\t"
                "vld1.u8         {d6},  [%6]!           \n\t"
                "vld1.u8         {d7},  [%7]!           \n\t"

                "vtrn.8          d1,  d0                            \n\t"
                "vtrn.8          d3,  d2                            \n\t"
                "vtrn.8          d5,  d4                            \n\t"
                "vtrn.8          d7,  d6                            \n\t"

                "vtrn.16         d3,  d1                            \n\t"
                "vtrn.16         d2,  d0                            \n\t"
                "vtrn.16         d7,  d5                            \n\t"
                "vtrn.16         d6,  d4                            \n\t"

                "vtrn.32         d7,  d3                            \n\t"
                "vtrn.32         d5,  d1                            \n\t"
                "vtrn.32         d6,  d2                            \n\t"
                "vtrn.32         d4,  d0                            \n\t"

                "vst1.u8         {d7},  [%8], r9            \n\t"
                "vst1.u8         {d6},  [%8], r9            \n\t"
                "vst1.u8         {d5},  [%8], r9            \n\t"
                "vst1.u8         {d4},  [%8], r9            \n\t"
                "vst1.u8         {d3},  [%8], r9        \n\t"
                "vst1.u8         {d2},  [%8], r9            \n\t"
                "vst1.u8         {d1},  [%8], r9            \n\t"
                "vst1.u8         {d0},  [%8], r9        \n\t"

                : "+r" (y_src_line0),  // %0
                  "+r" (y_src_line1),  // %1
                  "+r" (y_src_line2),  // %2
                  "+r" (y_src_line3),  // %3
                  "+r" (y_src_line4),  // %4
                  "+r" (y_src_line5),  // %5
                  "+r" (y_src_line6),  // %6
                  "+r" (y_src_line7),  // %7
                  "+r" (y_dst_line0),  // %8
                  "+r" (col_bytes)     // %9
                :
                : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "r9"
            );

            asm volatile
            (
                "mov             r8,    %5          \n\t"

                "pld             [%0,#64]               \n\t"
                "pld             [%1,#64]               \n\t"
                "pld             [%2,#64]               \n\t"
                "pld             [%3,#64]               \n\t"

                "vld1.u8         {d8},  [%0]!           \n\t"
                "vld1.u8         {d9},  [%1]!           \n\t"
                "vld1.u8         {d10}, [%2]!           \n\t"
                "vld1.u8         {d11}, [%3]!           \n\t"

                "vtrn.16         d9,  d8                            \n\t"
                "vtrn.16         d11,  d10                          \n\t"

                "vtrn.32         d11,  d9                           \n\t"
                "vtrn.32         d10,  d8                           \n\t"

                "vst1.u8         {d11}, [%4], r8            \n\t"
                "vst1.u8         {d10}, [%4], r8            \n\t"
                "vst1.u8         {d9},  [%4], r8            \n\t"
                "vst1.u8         {d8},  [%4], r8            \n\t"

                : "+r" (uv_src_line0),  // %0
                  "+r" (uv_src_line1),  // %1
                  "+r" (uv_src_line2),  // %2
                  "+r" (uv_src_line3),  // %3
                  "+r" (uv_dst_line0),  // %4
                  "+r" (col_bytes)      // %5
                :
                : "cc", "memory", "d8", "d9", "d10", "d11", "r8"
            );
        }

        y_src_line0 = y_src_line7;
        y_src_line1 += row_bytes_7;
        y_src_line2 += row_bytes_7;
        y_src_line3 += row_bytes_7;
        y_src_line4 += row_bytes_7;
        y_src_line5 += row_bytes_7;
        y_src_line6 += row_bytes_7;
        y_src_line7 += row_bytes_7;

        uv_src_line0 = uv_src_line3;
        uv_src_line1 += row_bytes_3;
        uv_src_line2 += row_bytes_3;
        uv_src_line3 += row_bytes_3;

        y_dst_line0 = y_dst_line0_static - i - 8;
        uv_dst_line0 = uv_dst_line0_static - i - 8;
    }
#else
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned char *src_t = src_addr+ width * (height - 1);
    unsigned char *dst_t = dst_addr;
    unsigned char *tmp;
    for (i = 0; i < width; ++i)
    {
        tmp = src_t + i;
        for (j = 0; j < height; ++j)
        {
            *dst_t++ = *tmp;
            tmp -= width;
        }
    }

    unsigned int uv_h = height >> 1;
    src_t = src_addr + width * height + width * (uv_h - 1);
    for(i = 0; i < width; i += 2)
    {
        tmp = src_t + i;
        for(j = 0; j < uv_h; ++j)
        {
            *dst_t++ = *tmp;
            *dst_t++ = *(tmp + 1);
            tmp -= width;
        }
    }
#endif
}


void NV21Rotate270DegreeByNeon(unsigned char* dst_addr, unsigned char* src_addr,
      unsigned int width, unsigned int height)
{
    unsigned int i = 0;
    unsigned int j = 0;

    unsigned int k = 0;

    unsigned int row_bytes = width;
    unsigned int col_bytes = height;

    unsigned int row_bytes_7 = width*7;
    unsigned int row_bytes_3 = width*3;

    unsigned char* y_src_line0 = src_addr;
    unsigned char* y_src_line1 = y_src_line0 + row_bytes;
    unsigned char* y_src_line2 = y_src_line1 + row_bytes;
    unsigned char* y_src_line3 = y_src_line2 + row_bytes;
    unsigned char* y_src_line4 = y_src_line3 + row_bytes;
    unsigned char* y_src_line5 = y_src_line4 + row_bytes;
    unsigned char* y_src_line6 = y_src_line5 + row_bytes;
    unsigned char* y_src_line7 = y_src_line6 + row_bytes;

    unsigned char* uv_src_line0 = src_addr + width*height;
    unsigned char* uv_src_line1 = uv_src_line0 + row_bytes;
    unsigned char* uv_src_line2 = uv_src_line1 + row_bytes;
    unsigned char* uv_src_line3 = uv_src_line2 + row_bytes;

    unsigned char* y_dst_line0 = dst_addr +  (row_bytes - 8)*col_bytes;
    unsigned char* uv_dst_line0 = dst_addr + width*height + (row_bytes - 8)*col_bytes/2;
    unsigned char* y_dst_line0_tmp = y_dst_line0;
    unsigned char* uv_dst_line0_tmp    = uv_dst_line0;

    unsigned char* y_dst_line0_static = y_dst_line0;
    unsigned char* uv_dst_line0_static = uv_dst_line0;

    for(i=0; i<height; i+=8)
    {
        for(j=0; j<width; j+=8)
        {
            asm volatile
            (
                "mov             r9,    %9          \n\t"

                "pld             [%0,#64]               \n\t"
                "pld             [%1,#64]               \n\t"
                "pld             [%2,#64]               \n\t"
                "pld             [%3,#64]               \n\t"
                "pld             [%4,#64]               \n\t"
                "pld             [%5,#64]               \n\t"
                "pld             [%6,#64]               \n\t"
                "pld             [%7,#64]               \n\t"

                "vld1.u8         {d0},  [%0]!           \n\t"
                "vld1.u8         {d1},  [%1]!           \n\t"
                "vld1.u8         {d2},  [%2]!           \n\t"
                "vld1.u8         {d3},  [%3]!           \n\t"
                "vld1.u8         {d4},  [%4]!           \n\t"
                "vld1.u8         {d5},  [%5]!           \n\t"
                "vld1.u8         {d6},  [%6]!           \n\t"
                "vld1.u8         {d7},  [%7]!           \n\t"

                "vtrn.8          d0,  d1                            \n\t"
                "vtrn.8          d2,  d3                            \n\t"
                "vtrn.8          d4,  d5                            \n\t"
                "vtrn.8          d6,  d7                            \n\t"

                "vtrn.16         d1,  d3                            \n\t"
                "vtrn.16         d0,  d2                            \n\t"
                "vtrn.16         d5,  d7                            \n\t"
                "vtrn.16         d4,  d6                            \n\t"

                "vtrn.32         d3,  d7                            \n\t"
                "vtrn.32         d1,  d5                            \n\t"
                "vtrn.32         d2,  d6                            \n\t"
                "vtrn.32         d0,  d4                            \n\t"

                "vst1.u8         {d7},  [%8], r9            \n\t"
                "vst1.u8         {d6},  [%8], r9            \n\t"
                "vst1.u8         {d5},  [%8], r9            \n\t"
                "vst1.u8         {d4},  [%8], r9            \n\t"
                "vst1.u8         {d3},  [%8], r9        \n\t"
                "vst1.u8         {d2},  [%8], r9            \n\t"
                "vst1.u8         {d1},  [%8], r9            \n\t"
                "vst1.u8         {d0},  [%8], r9        \n\t"

                : "+r" (y_src_line0),  // %0
                  "+r" (y_src_line1),  // %1
                  "+r" (y_src_line2),  // %2
                  "+r" (y_src_line3),  // %3
                  "+r" (y_src_line4),  // %4
                  "+r" (y_src_line5),  // %5
                  "+r" (y_src_line6),  // %6
                  "+r" (y_src_line7),  // %7
                  "+r" (y_dst_line0),  // %8
                  "+r" (col_bytes)     // %9
                :
                : "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "r9"
            );

            asm volatile
            (
                "mov             r8,    %5          \n\t"

                "pld             [%0,#64]               \n\t"
                "pld             [%1,#64]               \n\t"
                "pld             [%2,#64]               \n\t"
                "pld             [%3,#64]               \n\t"

                "vld1.u8         {d8},  [%0]!           \n\t"
                "vld1.u8         {d9},  [%1]!           \n\t"
                "vld1.u8         {d10}, [%2]!           \n\t"
                "vld1.u8         {d11}, [%3]!           \n\t"

                "vtrn.16         d8,  d9                            \n\t"
                "vtrn.16         d10,  d11                          \n\t"

                "vtrn.32         d9,  d11                           \n\t"
                "vtrn.32         d8,  d10                           \n\t"

                "vst1.u8         {d11}, [%4], r8            \n\t"
                "vst1.u8         {d10}, [%4], r8            \n\t"
                "vst1.u8         {d9},  [%4], r8            \n\t"
                "vst1.u8         {d8},  [%4], r8            \n\t"

                : "+r" (uv_src_line0),  // %0
                  "+r" (uv_src_line1),  // %1
                  "+r" (uv_src_line2),  // %2
                  "+r" (uv_src_line3),  // %3
                  "+r" (uv_dst_line0),  // %4
                  "+r" (col_bytes)      // %5
                :
                : "cc", "memory", "d8", "d9", "d10", "d11", "r8"
            );

            y_dst_line0_tmp -= (col_bytes*8);
            uv_dst_line0_tmp -= (col_bytes*4);

            y_dst_line0 = y_dst_line0_tmp;
            uv_dst_line0 = uv_dst_line0_tmp;
        }

        y_src_line0 = y_src_line7;
        y_src_line1 += row_bytes_7;
        y_src_line2 += row_bytes_7;
        y_src_line3 += row_bytes_7;
        y_src_line4 += row_bytes_7;
        y_src_line5 += row_bytes_7;
        y_src_line6 += row_bytes_7;
        y_src_line7 += row_bytes_7;

        uv_src_line0 = uv_src_line3;
        uv_src_line1 += row_bytes_3;
        uv_src_line2 += row_bytes_3;
        uv_src_line3 += row_bytes_3;

        y_dst_line0 = y_dst_line0_static + i + 8;
        uv_dst_line0 = uv_dst_line0_static + i + 8;

        y_dst_line0_tmp = y_dst_line0;
        uv_dst_line0_tmp = uv_dst_line0;
    }
}

#endif
