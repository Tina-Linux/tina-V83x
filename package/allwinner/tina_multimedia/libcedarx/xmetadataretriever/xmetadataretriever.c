/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : xmetadataretriever.c
 * Description : xmetadataretriever for Linux,
 *               get mediainfo and thumbnail picture
 * History :
 *
 */


#define LOG_TAG "xmetadataretriever"
#include "cdx_log.h"

#include "xmetadataretriever.h"
#include "memoryAdapter.h"
#include "iniparserapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>

#define SAVE_BITSTREAM 0
#define SAVE_RGB 0

#if SAVE_BITSTREAM
const char* bitstreamPath = "/data/camera/out.h264";
static FILE* fph264 = NULL;
#endif

/* process 1024 packets to get a frame at maximum. */
#define MAX_PACKET_COUNT_TO_GET_A_FRAME 4096
#define MAX_TIME_TO_GET_A_FRAME         5000000 /* use 5 seconds to get a frame at maximum. */
#define MAX_TIME_TO_GET_A_STREAM        10000000 /* use 10 seconds to get a stream at maximum. */
#define MAX_OUTPUT_STREAM_SIZE          (1024*1024)

typedef struct XRetrieverContext
{
    CdxDataSourceT              mSource;
    CdxMediaInfoT               mMediaInfo;
    CdxParserT*                 mParser;
    CdxStreamT*                 mStream;
    VideoDecoder*               mVideoDecoder;
    XVideoFrame                 mVideoFrame;
    XVideoStream                mVideoStream;
    int                         mCancelPrepareFlag;
    struct ScMemOpsS *memops;
}XRetrieverContext;

static int64_t GetSysTime();

#if SAVE_RGB
//-------------------------------------------------------------------
 /*
　　位图文件的组成
          结构名称 符 号
     位图文件头 (bitmap-file header) BITMAPFILEHEADER bmfh
    位图信息头 (bitmap-information header) BITMAPINFOHEADER bmih
    彩色表　(color table) RGBQUAD aColors[]
    图象数据阵列字节 BYTE aBitmapBits[]
  */
typedef struct bmp_header
{
    short twobyte           ;//两个字节，用来保证下面成员紧凑排列，这两个字符不能写到文件中
         //14B
    char bfType[2]          ;//!文件的类型,该值必需是0x4D42，也就是字符'BM'
    unsigned int bfSize     ;//!说明文件的大小，用字节为单位
    unsigned int bfReserved1;//保留，必须设置为0
    unsigned int bfOffBits  ;//!说明从文件头开始到实际的图象数据之间的字节的偏移量，这里为14B+sizeof(BMPINFO)
}BMPHEADER;

typedef struct bmp_info
{
         //40B
     unsigned int biSize         ;//!BMPINFO结构所需要的字数
     int biWidth                 ;//!图象的宽度，以象素为单位
     int biHeight                ;//!图象的宽度，以象素为单位,如果该值是正数，
                                  //说明图像是倒向的，如果该值是负数，则是正向的
     unsigned short biPlanes     ;//!目标设备说明位面数，其值将总是被设为1
     unsigned short biBitCount   ;//!比特数/象素，其值为1、4、8、16、24、或32
     unsigned int biCompression  ;//说明图象数据压缩的类型
     #define BI_RGB        0L    //没有压缩
     #define BI_RLE8       1L    //每个象素8比特的RLE压缩编码，压缩格式由2字节组成（重复象素计数和颜色索引）；
     #define BI_RLE4       2L    //每个象素4比特的RLE压缩编码，压缩格式由2字节组成
     #define BI_BITFIELDS  3L    //每个象素的比特由指定的掩码决定。
     unsigned int biSizeImage    ;//图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0
     int biXPelsPerMeter         ;//水平分辨率，用象素/米表示
     int biYPelsPerMeter         ;//垂直分辨率，用象素/米表示
     unsigned int biClrUsed      ;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
     unsigned int biClrImportant ;//对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。
}BMPINFO;

typedef struct tagRGBQUAD
{
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO
{
    BMPINFO    bmiHeader;
    //RGBQUAD    bmiColors[1];
    unsigned int rgb[3];
} BITMAPINFO;

static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)
{
    int size = 0;
    if (head && info)
    {
        size = w * h * 2;
        memset(head, 0, sizeof(* head));
        memset(info, 0, sizeof(* info));
         head->bfType[0] = 'B';
         head->bfType[1] = 'M';
         head->bfOffBits = 14 + sizeof(* info);
         head->bfSize = head->bfOffBits + size;
         head->bfSize = (head->bfSize + 3) & ~3;
         size = head->bfSize - head->bfOffBits;

         info->bmiHeader.biSize = sizeof(info->bmiHeader);
         info->bmiHeader.biWidth = w;
         info->bmiHeader.biHeight = -h;
         info->bmiHeader.biPlanes = 1;
         info->bmiHeader.biBitCount = 16;
         info->bmiHeader.biCompression = BI_BITFIELDS;
         info->bmiHeader.biSizeImage = size;

         info->rgb[0] = 0xF800;
         info->rgb[1] = 0x07E0;
         info->rgb[2] = 0x001F;

         logd("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
     }
     return size;
 }

static int save_bmp_rgb565(FILE* fp, int width, int height, unsigned char* pData)
{
    int success = 0;
    int size = 0;
    BMPHEADER head;
    BITMAPINFO info;
    size = get_rgb565_header(width, height, &head, &info);
    if(size > 0)
    {
        fwrite(head.bfType,1,2,fp);
        fwrite(&head.bfSize,1,4,fp);
        fwrite(&head.bfReserved1,1,4,fp);
        fwrite(&head.bfOffBits,1,4,fp);

        fwrite(&info,1,sizeof(info), fp);
        fwrite(pData,1,size, fp);
        success = 1;
    }
    logd("*****success=%d\n", success);
    return success;
}
#endif

// B = 1.164 * (Y - 16) + 2.018 * (U - 128)
// G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128)
// R = 1.164 * (Y - 16) + 1.596 * (V - 128)

// B = 298/256 * (Y - 16) + 517/256 * (U - 128)
// G = .................. - 208/256 * (V - 128) - 100/256 * (U - 128)
// R = .................. + 409/256 * (V - 128)

// min_B = (298 * (- 16) + 517 * (- 128)) / 256 = -277
// min_G = (298 * (- 16) - 208 * (255 - 128) - 100 * (255 - 128)) / 256 = -172
// min_R = (298 * (- 16) + 409 * (- 128)) / 256 = -223

// max_B = (298 * (255 - 16) + 517 * (255 - 128)) / 256 = 534
// max_G = (298 * (255 - 16) - 208 * (- 128) - 100 * (- 128)) / 256 = 432
// max_R = (298 * (255 - 16) + 409 * (255 - 128)) / 256 = 481

// clip range -278 .. 535

typedef struct {
    int nPicWidth;
    int nPicHeight;
    unsigned char* pData;
    unsigned int  nDataSize;
}RgbVideoFrame;

static int transformPictureNV21ToRGB565(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 XVideoFrame* pVideoFrame)
{
    unsigned short*  pDst;
    unsigned char*   pSrcY;
    unsigned char*   pSrcU;
    int              y;
    int              x;
    unsigned char*   pClipTable;
    unsigned char*   pClip;

    static const int nClipMin = -278;
    static const int nClipMax = 535;

    if((pPicture->ePixelFormat!= PIXEL_FORMAT_NV12) &&
            (pPicture->ePixelFormat!= PIXEL_FORMAT_NV21))
    {
        loge("source pixel format is not NV12, quit.");
        return -1;
    }

    //* initialize the clip table.
    pClipTable = (unsigned char*)malloc(nClipMax - nClipMin + 1);
    if(pClipTable == NULL)
    {
        loge("can not allocate memory for the clip table, quit.");
        return -1;
    }
    for(x=nClipMin; x<=nClipMax; x++)
    {
        pClipTable[x-nClipMin] = (x<0) ? 0 : (x>255) ? 255 : x;
    }
    pClip = &pClipTable[-nClipMin];

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nLineStride*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nLineStride*pPicture->nHeight/2);

    //* set pointers.
    pDst  = (unsigned short*)pVideoFrame->mData;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    pSrcU = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                    (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    signed y1, y2;
    signed u,  v;
    for(y = 0; y < (int)pVideoFrame->mHeight; ++y)
    {
        for(x = 0; x < (int)pVideoFrame->mWidth; x += 2)
        {
            y1 = (signed)pSrcY[x] - 16;
            y2 = (signed)pSrcY[x + 1] - 16;

            if(pPicture->ePixelFormat == PIXEL_FORMAT_NV12)
            {
                u = (signed)pSrcU[x] - 128;
                v = (signed)pSrcU[x + 1] - 128;
            }
            else
            {
                v = (signed)pSrcU[x] - 128;
                u = (signed)pSrcU[x + 1] - 128;
            }

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            unsigned int rgb1 = ((pClip[r1] >> 3) << 11) |
                                ((pClip[g1] >> 2) << 5)  |
                                (pClip[b1] >> 3);

            unsigned int rgb2 = ((pClip[r2] >> 3) << 11) |
                                ((pClip[g2] >> 2) << 5)  |
                                (pClip[b2] >> 3);

            *(unsigned int *)(&pDst[x]) = (rgb2 << 16) | rgb1;
        }

        pSrcY += pPicture->nLineStride;

        if(y & 1)
        {
            pSrcU += pPicture->nLineStride;
        }

        pDst += pVideoFrame->mWidth;
    }

    free(pClipTable);

    return 0;
}

static int transformPictureMb32ToRGB(struct ScMemOpsS *memOps,
                                                  VideoPicture * pPicture,
                                                  XVideoFrame* pVideoFrame)
{
    unsigned char*   pClipTable;
    unsigned char*   pClip;
    static const int nClipMin = -278;
    static const int nClipMax = 535;

    unsigned short*  pDst = NULL;
    unsigned char*   pSrcY = NULL;
    unsigned char*   pSrcVU = NULL;

    int x = 0;
    int y = 0;
    int nMbWidth = 0;
    int nMbHeight = 0;
    int nVMb = 0;
    int nHMb = 0;
    int yPos = 0;
    int pos = 0;
    int uvPos = 0;

  // 可以将pClipTable 以参数的形式传入transformPictureMb32ToRGB
    //* initialize the clip table.
    pClipTable = (unsigned char*)malloc(nClipMax - nClipMin + 1);
    if(pClipTable == NULL)
    {
        loge("can not allocate memory for the clip table, quit.");
        return -1;
    }
    for(x=nClipMin; x<=nClipMax; x++)
    {
        pClipTable[x-nClipMin] = (x<0) ? 0 : (x>255) ? 255 : x;
    }
    pClip = &pClipTable[-nClipMin];

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nHeight*pPicture->nHeight/2);

    pDst  = (unsigned short*)pVideoFrame->mData;
    pSrcY = (unsigned char*)pPicture->pData0;
    pSrcVU  = (unsigned char*)pPicture->pData1;

    nMbWidth = pPicture->nWidth/32;
    nMbHeight = pPicture->nHeight/32;

    for(nVMb=0; nVMb<nMbHeight;nVMb++)
    {
        for(nHMb=0; nHMb<nMbWidth; nHMb++)
        {
            pos = nVMb*pPicture->nWidth*32+nHMb*32;
            for(y=0; y<32; y++)
            {
                yPos = (nVMb*nMbWidth+nHMb)*1024+y*32;
                uvPos = ((nVMb/2)*nMbWidth*1024)+nHMb*1024+(y/2)*32+ (((nVMb%2)==1) ? 512 : 0);
                for(x=0; x<32; x+=2)
                {
                    signed y1 = (signed)pSrcY[yPos+x+0] - 16;
                    signed y2 = (signed)pSrcY[yPos+x+1] - 16;
                    signed u  = (signed)pSrcVU[uvPos+x+0] - 128;
                    signed v  = (signed)pSrcVU[uvPos+x+1] - 128;
                    signed u_b = u * 517;
                    signed u_g = -u * 100;
                    signed v_g = -v * 208;
                    signed v_r = v * 409;
                    signed tmp1 = y1 * 298;
                    signed b1 = (tmp1 + u_b) / 256;
                    signed g1 = (tmp1 + v_g + u_g) / 256;
                    signed r1 = (tmp1 + v_r) / 256;
                    signed tmp2 = y2 * 298;
                    signed b2 = (tmp2 + u_b) / 256;
                    signed g2 = (tmp2 + v_g + u_g) / 256;
                    signed r2 = (tmp2 + v_r) / 256;
                    unsigned int rgb1 = ((pClip[r1] >> 3) << 11) |
                                        ((pClip[g1] >> 2) << 5)  |
                                        (pClip[b1] >> 3);

                    unsigned int rgb2 = ((pClip[r2] >> 3) << 11) |
                                        ((pClip[g2] >> 2) << 5)  |
                                        (pClip[b2] >> 3);
                    *(unsigned int *)(&pDst[pos]) = (rgb2 << 16) | rgb1;
                    pos += 2;
                }
                pos += (nMbWidth-1)*32;
            }
        }
    }

    pDst  = (unsigned short*)pVideoFrame->mData;
    for(y=0; y<pPicture->nTopOffset; y++)
    {
        memset(pDst+y*pVideoFrame->mWidth, 0, 2*pVideoFrame->mWidth);
    }
    for(y=pPicture->nBottomOffset; y<(int)pVideoFrame->mHeight; y++)
    {
        memset(pDst+y*pVideoFrame->mWidth, 0, 2*pVideoFrame->mWidth);
    }

    for(y=pPicture->nTopOffset; y<pPicture->nBottomOffset; y++)
    {
        memset(pDst+y*pVideoFrame->mWidth, 0, 2*pPicture->nLeftOffset);
        memset(pDst+y*pVideoFrame->mWidth+pPicture->nRightOffset, 0,
                2*(pVideoFrame->mWidth-pPicture->nRightOffset));
    }

    free(pClipTable);

#if SAVE_RGB
    FILE* outFp = fopen("/data/UDISK/rgb.bmp", "wb");
    if(outFp != NULL)
    {
        logd("************save_bmp_rgb565\n");
        save_bmp_rgb565(outFp, pVideoFrame->mWidth, pVideoFrame->mHeight, pVideoFrame->mData);
        fclose(outFp);
    }
#endif

    return 0;
}

static int transformPicture(struct ScMemOpsS *memOps,
                                  VideoPicture * pPicture,
                                  XVideoFrame* pVideoFrame);

static void clear(XRetriever* v)
{
    XRetrieverContext* p;
    p = (XRetrieverContext*)v;

    if(p->mParser)
    {
        CdxParserForceStop(p->mParser);    //* to prevend parser from blocking at a network io.
        CdxParserClose(p->mParser);
        p->mParser = NULL;
        p->mStream = NULL;
    }
    else if(p->mStream)
    {
        CdxStreamForceStop(p->mStream);
        CdxStreamClose(p->mStream);
        p->mStream = NULL;
    }

    if(p->mVideoDecoder != NULL)
    {
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
    }

    if(p->mSource.uri)
    {
        free(p->mSource.uri);
    }
    memset(&p->mMediaInfo, 0, sizeof(CdxMediaInfoT));

    if(p->mVideoFrame.mData)
    {
        free(p->mVideoFrame.mData);
        p->mVideoFrame.mData = NULL;
    }
    memset(&p->mVideoFrame, 0x00, sizeof(XVideoFrame));

    if(p->mVideoStream.mBuf)
    {
        free(p->mVideoStream.mBuf);
        p->mVideoStream.mBuf = NULL;
    }
    memset(&p->mVideoStream, 0x00, sizeof(XVideoStream));

    return;
}

XRetriever* XRetrieverCreate()
{
    XRetrieverContext* p;
    p = (XRetrieverContext*)malloc(sizeof(XRetrieverContext));
    if(!p)
    {
        return NULL;
    }
    memset(p, 0x00, sizeof(XRetrieverContext));

    memset(&p->mSource, 0, sizeof(CdxDataSourceT));

    p->memops = MemAdapterGetOpsS();
    if(p->memops == NULL)
    {
        free(p);
        return NULL;
    }
    CdcMemOpen(p->memops);

    return (XRetriever*)p;
}

int  XRetrieverDestory(XRetriever* v)
{
    XRetrieverContext* p;
    p = (XRetrieverContext*)v;

    clear(v);
    CdcMemClose(p->memops);
    free(p);
    return 0;
}

static int retrieverSetDataSource(XRetrieverContext* p, const char* pUrl,
                                 const CdxKeyedVectorT* pHeaders)
{

    if(strstr(pUrl, "://") != NULL)
    {
        p->mSource.uri = strdup(pUrl);
        if(p->mSource.uri == NULL)
        {
            loge("can not dump string of uri.");
            return -1;
        }
    }
    else
    {
        p->mSource.uri  = (char*)malloc(strlen(pUrl)+8);
        if(p->mSource.uri == NULL)
        {
            loge("can not dump string of uri.");
            return -1;
        }
        sprintf(p->mSource.uri, "file://%s", pUrl);
    }

    if(pHeaders != NULL &&
        (!strncasecmp("http://", pUrl, 7) || !strncasecmp("https://", pUrl, 8)))
    {
        //todo: set headers
        int nHeaderSize = CdxKeyedVectorGetSize((CdxKeyedVectorT*)pHeaders);
        if(nHeaderSize > 0)
        {
            p->mSource.extraDataType = EXTRA_DATA_HTTP_HEADER;
        }
    }
    return 0;
}

int XRetrieverSetDataSource(XRetriever* v, const char* pUrl, const CdxKeyedVectorT* pHeaders)
{
    int ret;
    XRetrieverContext* p;
    p = (XRetrieverContext*)v;
    clear(v);

    //* 1. set the datasource object.
    ret = retrieverSetDataSource(p, pUrl, pHeaders);
    if(ret < 0)
    {
        loge("set datasource fail.");
        return -1;
    }

    logd("uri: %s", p->mSource.uri);

    //* 2. create a parser.
    p->mStream = CdxStreamCreate(&p->mSource);
    if(!p->mStream)
    {
        loge("stream creat fail.");
        return -1;
    }
    ret = CdxStreamConnect(p->mStream);
    if(ret < 0)
    {
        return -1;
    }

    p->mParser = CdxParserCreate(p->mStream, MUTIL_AUDIO);
    if(!p->mParser)
    {
        loge("parser creat fail.");
        return -1;
    }
    ret = CdxParserInit(p->mParser);
    if(ret < 0)
    {
        return -1;
    }

    //* 3. get media info.
    memset(&p->mMediaInfo, 0, sizeof(CdxMediaInfoT));
    if(CdxParserGetMediaInfo(p->mParser, &p->mMediaInfo) == 0)
    {
        if (p->mParser->type == CDX_PARSER_TS ||
            p->mParser->type == CDX_PARSER_BD ||
            p->mParser->type == CDX_PARSER_HLS)
        {
            p->mMediaInfo.program[0].video[0].bIsFramePackage = 0; /* stream package */
        }
        else
        {
            p->mMediaInfo.program[0].video[0].bIsFramePackage = 1; /* frame package */
        }
    }

    return 0;
}

static void setScaleRatio(VConfig *vconfig, VideoStreamInfo *videoInfo,
        int *hScaleRatio, int *vScaleRatio)
{
    int wider = 640, widest = 1280;
    int higher = 480, highest = 960;
    if (vconfig->nRotateDegree == 1 || vconfig->nRotateDegree == 3)
    {
        wider = 480;
        widest = 960;
        higher = 640;
        highest = 1280;
    }

    if(videoInfo->nWidth > widest || videoInfo->nWidth  == 0)
        *hScaleRatio = 2; //* scale down to 1/4 the original width;
    else if(videoInfo->nWidth > wider)
        *hScaleRatio = 1; //* scale down to 1/2 the original width;

    if(videoInfo->nHeight > highest || videoInfo->nHeight == 0)
        *vScaleRatio = 2; //* scale down to 1/4 the original height;
    else if(videoInfo->nHeight > higher)
        *vScaleRatio = 1; //* scale down to 1/2 the original height;
}

//* option = 0 means seek to the sync frame privious to the timeUs.
//* option = 1 means seek to the sync frame next to the timeUs.
//* option = 2 means seek to the sync frame closest to the timeUs.
//* option = 3 means seek to the closest frame to the timeUs.
XVideoFrame *XRetrieverGetFrameAtTime(XRetriever* v, int64_t timeUs, int option)
{
    VideoPicture*     pPicture;
    int               bDone;
    int               bSuccess;
    int               bHasVideo;
    int               nHorizonScaleRatio;
    int               nVerticalScaleRatio;

    VConfig           vconfig;
    CdxPacketT        packet;
    int               ret;
    int               nPacketCount;
    int64_t           nStartTime;
    int64_t           nTimePassed;
    XRetrieverContext* p;

    p = (XRetrieverContext*)v;

    //* FIXME:
    //* if it is a media file with drm protection, we should not return an album art picture.

    bDone = 0;
    bSuccess = 0;
    bHasVideo = 0;
    memset(&vconfig, 0, sizeof(VConfig));

    //* 1. check whether there is a video stream.
    if(p->mMediaInfo.programIndex >= 0 && p->mMediaInfo.programNum >= p->mMediaInfo.programIndex)
    {
        if(p->mMediaInfo.program[p->mMediaInfo.programIndex].videoNum > 0)
            bHasVideo = 1;
    }
    if(!bHasVideo)
    {
        logw("media file do not contain a video stream, getFrameAtTime() return fail.");
        return NULL;
    }

    //* 2. create a video decoder.
    if(p->mVideoDecoder == NULL)
    {
        p->mVideoDecoder = CreateVideoDecoder();
        //* use decoder to capture thumbnail, decoder use less memory in this mode.
        if(option == 3)
        {
            vconfig.bThumbnailMode      = 0;
        }
        else
        {
            vconfig.bThumbnailMode      = 1;
        }
        //* all decoder support YV12 format.
        // some chip not support yv12, we should set PIXEL_FORMAT_YUV_MB32_420;
        const char *vd_outfmt = GetConfigParamterString("vd_output_fmt", NULL);
        CDX_LOG_CHECK(vd_outfmt, "vd_output_fmt NULL, pls check your config.");
        if (strcmp(vd_outfmt, "mb32") == 0)
        {
            vconfig.eOutputPixelFormat = PIXEL_FORMAT_YUV_MB32_420;
        }
        else
        {
            logd("set default format: yv12");
            vconfig.eOutputPixelFormat = PIXEL_FORMAT_YV12;
        }

        //* no need to decode two picture when decoding a thumbnail picture.
        vconfig.bDisable3D          = 1;

        vconfig.nAlignStride        = 16;//* set align stride to 16 as defualt
#if 1
        //* set this flag when the parser can give this info,
        //* mov files recorded by iphone or android phone
        //* conains this info.

        //* set the rotation
        int nRotateDegree;
        int nRotation = atoi((const char*)p->mMediaInfo.rotate);

        if(nRotation == 0)
           nRotateDegree = 0;
        else if(nRotation == 90)
           nRotateDegree = 1;
        else if(nRotation == 180)
           nRotateDegree = 2;
        else if(nRotation == 270)
           nRotateDegree = 3;
        else
           nRotateDegree = 0;

        if(nRotateDegree != 0)
        {
            vconfig.bRotationEn         = 1;
            vconfig.nRotateDegree       = nRotateDegree;
        }
        else
        {
            vconfig.bRotationEn         = 0;
            vconfig.nRotateDegree       = 0;
        }

#endif
        //* set the picture scale down ratio,
        //* we generate a picture with pixel size less than 640x480.
        nHorizonScaleRatio  = 0;
        nVerticalScaleRatio = 0;
        VideoStreamInfo *videoInfo =
                        p->mMediaInfo.program[p->mMediaInfo.programIndex].video;
        setScaleRatio(&vconfig, videoInfo, &nHorizonScaleRatio, &nVerticalScaleRatio);
#if 0
        if(vconfig.nRotateDegree == 1 || vconfig.nRotateDegree == 3)
        {
            /* nWidth=0 treated as nWidth=1920 */
            if(videoInfo.nWidth > 960 ||
                videoInfo.nWidth  == 0)
                nHorizonScaleRatio = 2; //* scale down to 1/4 the original width;
            else if(videoInfo.nWidth > 480)
                nHorizonScaleRatio = 1; //* scale down to 1/2 the original width;

            /* nHeight=0 treated as nHeight=1080 */
            if(videoInfo.nHeight > 1280 ||
                videoInfo.nHeight == 0)
                nVerticalScaleRatio = 2; //* scale down to 1/4 the original height;
            else if(videoInfo.nHeight > 640)
                nVerticalScaleRatio = 1; //* scale down to 1/2 the original height;
        }
        else
        {
            /* nWidth=0 treated as nWidth=1920 */
            if(videoInfo.nWidth > 1280 ||
                videoInfo.nWidth  == 0)
                nHorizonScaleRatio = 2; //* scale down to 1/4 the original width;
            else if(videoInfo.nWidth > 640)
                nHorizonScaleRatio = 1; //* scale down to 1/2 the original width;

             /* nHeight=0 treated as nHeight=1080 */
            if(videoInfo.nHeight > 960 ||
                videoInfo.nHeight == 0)
                nVerticalScaleRatio = 2; //* scale down to 1/4 the original height;
            else if(videoInfo.nHeight > 480)
                nVerticalScaleRatio = 1; //* scale down to 1/2 the original height;
        }
#endif
        //* set to the same scale ratio.
        if(nVerticalScaleRatio > nHorizonScaleRatio)
            nHorizonScaleRatio = nVerticalScaleRatio;
        else
            nVerticalScaleRatio = nHorizonScaleRatio;

        //* set scale ratio to vconfig.
        if(nHorizonScaleRatio || nVerticalScaleRatio)
        {
            vconfig.bScaleDownEn            = 1;
            vconfig.nHorizonScaleDownRatio  = nHorizonScaleRatio;
            vconfig.nVerticalScaleDownRatio = nVerticalScaleRatio;
        }

        //* initialize the decoder.
        vconfig.nVbvBufferSize = 2*1024*1024;
        if(videoInfo->nWidth > 3800)
            vconfig.nVbvBufferSize = 4*1024*1024;
        //###check
        vconfig.nDeInterlaceHoldingFrameBufferNum = 0;//GetConfigParamterInt("pic_4di_num", 2);
        vconfig.nDisplayHoldingFrameBufferNum = 0;
        vconfig.nRotateHoldingFrameBufferNum = GetConfigParamterInt("pic_4rotate_num", 0);
        vconfig.nDecodeSmoothFrameBufferNum = GetConfigParamterInt("pic_4smooth_num", 3);

        vconfig.bConvertVp910bitTo8bit = 1;
        vconfig.memops = p->memops;
        if(InitializeVideoDecoder(p->mVideoDecoder,
            videoInfo, &vconfig) != 0)
        {
            loge("initialize video decoder fail.");
            return NULL;
        }
    }

    if(timeUs < 0)
    {
        //The key frame of MKV always at the end of file, need reset to 0,
        //otherwise will return mix thumbnail
        if(p->mParser->type == CDX_PARSER_MKV)
        {
            timeUs = 0;
        }
        else
        {
            timeUs = ((int64_t)p->mMediaInfo.program[p->mMediaInfo.programIndex].duration*1000/2);
        }
    }

     if(p->mMediaInfo.program[p->mMediaInfo.programIndex].duration < 20000)
     {
         timeUs = 0;
     }

    //* 3. seek parser to the specific position.
    if(p->mMediaInfo.bSeekable && timeUs > 0 &&
        timeUs < ((int64_t)p->mMediaInfo.program[p->mMediaInfo.programIndex].duration*1000))
    {
        SeekModeType nSeekModeType;
        nSeekModeType = AW_SEEK_THUMBNAIL;
        if(option == 2)
        {
            nSeekModeType = AW_SEEK_CLOSEST_SYNC;
        }
        if(CdxParserSeekTo(p->mParser, timeUs, nSeekModeType) != 0)
        {
            logw("seek media file error at the specific time %" PRId64 " us.", timeUs);
            if(CdxParserSeekTo(p->mParser, 0, nSeekModeType) != 0)
            {
                loge("seek media file error at the time 0");
                return NULL;
            }
        }
    }
    else
    {
        logw("media file do not support seek operation, get frame from the begin.");
    }

    //* 4. loop to decode a picture.
    nPacketCount = 0;
    nStartTime   = GetSysTime();
    int64_t first_pts;
    int first_flag = 0;

    do
    {
        //* 4.1 prefetch packet type and packet data size.
        packet.flags = 0;
        if(CdxParserPrefetch(p->mParser, &packet) != 0)
        {
            //* prefetch fail, may be file end reached.
            bDone = 1;
            bSuccess = 0;
            break;
        }

        //* 4.2 feed data to the video decoder.
        if(packet.type == CDX_MEDIA_VIDEO && (packet.flags&MINOR_STREAM)==0)
        {
            ret = RequestVideoStreamBuffer(p->mVideoDecoder,
                                           packet.length,
                                           (char**)&packet.buf,
                                           &packet.buflen,
                                           (char**)&packet.ringBuf,
                                           &packet.ringBufLen,
                                           0);

            if(ret==0 && (packet.buflen+packet.ringBufLen)>=packet.length)
            {
                if(first_flag == 0)
                {
                    first_pts = packet.pts;
                    first_flag = 1;
                }
                nPacketCount++;
                if(CdxParserRead(p->mParser, &packet) == 0)
                {
                    VideoStreamDataInfo dataInfo;
                    dataInfo.pData        = (char*)packet.buf;
                    dataInfo.nLength      = packet.length;
                    dataInfo.nPts         = packet.pts;
                    dataInfo.nPcr         = -1;
                    dataInfo.bIsFirstPart = 1;
                    dataInfo.bIsLastPart  = 1;
                    dataInfo.nStreamIndex = 0;
                    SubmitVideoStreamData(p->mVideoDecoder, &dataInfo, 0);
                }
                else
                {
                    //* read data fail, may be data error.
                    loge("read packet from parser fail.");
                    bDone = 1;
                    bSuccess = 0;
                    break;
                }
            }
            else
            {
                //* no buffer, may be the decoder is full of stream.
                logw("waiting for stream buffer.");
            }
        }
        else
        {
            //* only process the major video stream.
            //* allocate a buffer to read uncare media data and skip it.
            packet.buf = malloc(packet.length);
            if(packet.buf != NULL)
            {
                nPacketCount++;
                packet.buflen     = packet.length;
                packet.ringBuf    = NULL;
                packet.ringBufLen = 0;
                if(CdxParserRead(p->mParser, &packet) == 0)
                {
                    free(packet.buf);
                    continue;
                }
                else
                {
                    free(packet.buf);

                    //* read data fail, may be data error.
                    loge("read packet from parser fail.");
                    bDone = 1;
                    bSuccess = 0;
                    break;
                }
            }
            else
            {
                loge("can not allocate buffer for none video packet.");
                bDone = 1;
                bSuccess = 0;
                break;
            }
        }

        //* 4.3 decode stream.
        ret = DecodeVideoStream(p->mVideoDecoder, 0 /*eos*/,
                0/*key frame only*/, 0/*drop b frame*/, 0/*current time*/);

        if(ret < 0)
        {
            loge("decode stream return fail.");
            bDone = 1;
            bSuccess = 0;
            break;
        }
        else if(ret == VDECODE_RESULT_RESOLUTION_CHANGE)
        {
            logi("video resolution changed.");
            ReopenVideoEngine(p->mVideoDecoder, &vconfig,
                    &p->mMediaInfo.program[p->mMediaInfo.programIndex].video[0]);
            continue;
        }

        //* 4.4 try to get a picture from the decoder.
        if(ret == VDECODE_RESULT_FRAME_DECODED || ret == VDECODE_RESULT_KEYFRAME_DECODED)
        {
            pPicture = RequestPicture(p->mVideoDecoder, 0/*the major stream*/);
            if(pPicture != NULL)
            {
                if(ret == VDECODE_RESULT_KEYFRAME_DECODED)
                {
                    if(option == 3)
                    {
                        if(timeUs - pPicture->nPts < 16666)
                        {
                           bDone = 1;
                           bSuccess = 1;
                           break;
                        }
                        else
                        {
                            ReturnPicture(p->mVideoDecoder,pPicture);
                            pPicture = NULL;
                        }
                    }
                    else
                    {
                        if(option == 1)
                        {
                            if(pPicture->nPts >= timeUs)
                            {
                                bDone = 1;
                                bSuccess = 1;
                                break;
                            }
                            else
                            {
                                ReturnPicture(p->mVideoDecoder,pPicture);
                                pPicture = NULL;
                            }
                        }
                        if(option == 0 || option == 2)
                        {
                            bDone = 1;
                            bSuccess = 1;
                            break;
                        }
                    }
                }
                else
                {
                    if(option == 3)
                    {
                       if(timeUs - pPicture->nPts < 16666)
                       {
                           bDone = 1;
                           bSuccess = 1;
                           break;
                       }
                       else
                       {
                           ReturnPicture(p->mVideoDecoder,pPicture);
                           pPicture = NULL;
                       }
                    }
                    else
                    {
                        ReturnPicture(p->mVideoDecoder,pPicture);
                        pPicture = NULL;
                    }
                }
            }
        }

        //* check whether cost too long time or process too much packets.
        nTimePassed = GetSysTime() - nStartTime;
        if(nTimePassed > MAX_TIME_TO_GET_A_FRAME || nTimePassed < 0)
        {
            logw("cost more than %d us but can not get a picture, quit.", MAX_TIME_TO_GET_A_FRAME);
            bDone = 1;
            bSuccess = 0;
            break;
        }
        if(nPacketCount > MAX_PACKET_COUNT_TO_GET_A_FRAME)
        {
            logw("process more than %d packets but can not get a picture, quit.",
                            MAX_PACKET_COUNT_TO_GET_A_FRAME);
            bDone = 1;
            bSuccess = 0;
            break;
        }
    }while(!bDone);

    //* 5. transform the picture if suceess to get a picture.
    if(bSuccess)
    {
        //* let the width and height is multiple of 2, for convinient of yuv to rgb565 converting.
        if(pPicture->nLeftOffset & 1)
            pPicture->nLeftOffset += 1;
        if(pPicture->nRightOffset & 1)
            pPicture->nRightOffset -= 1;
        if(pPicture->nTopOffset & 1)
            pPicture->nTopOffset += 1;
        if(pPicture->nBottomOffset & 1)
            pPicture->nBottomOffset -= 1;

        //* I find that the mpeg2 decoder output the original picture's crop params,
        //* it is bigger than the scaledown picture's size.
        if((pPicture->nBottomOffset != 0 || pPicture->nRightOffset != 0) &&
            pPicture->nRightOffset <= pPicture->nLineStride)
        {
            p->mVideoFrame.mDisplayWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
            p->mVideoFrame.mDisplayHeight = pPicture->nBottomOffset - pPicture->nTopOffset;
            p->mVideoFrame.mWidth         = p->mVideoFrame.mDisplayWidth;
            p->mVideoFrame.mHeight        = p->mVideoFrame.mDisplayHeight;
        }
        else
        {
            p->mVideoFrame.mDisplayWidth  = pPicture->nWidth;
            p->mVideoFrame.mDisplayHeight = pPicture->nHeight;
            p->mVideoFrame.mWidth         = pPicture->nWidth;
            p->mVideoFrame.mHeight        = pPicture->nHeight;
        }

        if(pPicture->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
        {
            // for 1663, mb32
            p->mVideoFrame.mDisplayWidth  = pPicture->nWidth;
            p->mVideoFrame.mDisplayHeight = pPicture->nHeight;
            p->mVideoFrame.mWidth         = pPicture->nWidth;
            p->mVideoFrame.mHeight        = pPicture->nHeight;
            p->mVideoFrame.mSize = p->mVideoFrame.mWidth * p->mVideoFrame.mHeight * 2;
            p->mVideoFrame.mData = (unsigned char*)malloc(p->mVideoFrame.mSize);
            if(p->mVideoFrame.mData == NULL)
            {
                loge("can not allocate memory for video frame.");
                return NULL;
            }
        }
        else
        {
            //* for RGB565 pixel format.
            p->mVideoFrame.mSize = p->mVideoFrame.mWidth * p->mVideoFrame.mHeight * 2;
            p->mVideoFrame.mData = (unsigned char*)malloc(p->mVideoFrame.mSize);
            if(p->mVideoFrame.mData == NULL)
            {
                loge("can not allocate memory for video frame.");
                return NULL;
            }
        }

        p->mVideoFrame.mRotationAngle = 0;

        if(pPicture->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420)
        {

            //* convert pixel format.
            if(transformPictureMb32ToRGB(p->memops, pPicture, &p->mVideoFrame) < 0)
            {
                return NULL;
            }
        }
        else if(pPicture->ePixelFormat == PIXEL_FORMAT_NV12
            || pPicture->ePixelFormat == PIXEL_FORMAT_NV21)
        {
            if(transformPictureNV21ToRGB565(p->memops, pPicture, &p->mVideoFrame) < 0)
            {
                return NULL;
            }
        }
        else
        {
            if(transformPicture(p->memops, pPicture, &p->mVideoFrame) < 0)
            {
                return NULL;
            }
        }
        ReturnPicture(p->mVideoDecoder,pPicture);
        pPicture = NULL;
        ResetVideoDecoder(p->mVideoDecoder);
        return &p->mVideoFrame;
    }
    else
    {
        loge("cannot decode a picture.");
        return NULL;
    }
}

XVideoStream *XRetrieverGetStreamAtTime(XRetriever * v,int64_t timeUs)
{
    VideoPicture*      pPicture;
    int               bDone;
    int               bHasVideo;
    int               nHorizonScaleRatio;
    int               nVerticalScaleRatio;
    enum EPIXELFORMAT ePixelFormat;
    VConfig           vconfig;
    CdxPacketT          packet;
    int               ret;
    int64_t           nStartTime;
    int64_t           nTimePassed;

    //* for encoder
    VencBaseConfig    sBaseEncConfig;
    VencInputBuffer   sInputBuffer;
    VencOutputBuffer  sOutputBuffer;
    VencHeaderData    spsppsInfo;
    VencH264Param     h264Param;
    VideoEncoder*     pVideoEncoder    = NULL;
    int               bEncoderInited   = 0;
    unsigned char*    pOutBuf          = NULL;
    int               bBitStreamLength = 0;
    int               bEncodeAllFrames = 0;
    int               bFrameCounter    = 0;

    int               videoCodec;

    XRetrieverContext* p;
    int               bErr = 0;

    p = (XRetrieverContext*)v;

#if SAVE_BITSTREAM
    fph264 = fopen(bitstreamPath, "wb");
#endif

    //* FIXME:
    //* if it is a media file with drm protection, we should not return an album art picture.

    bDone = 0;
    bHasVideo = 0;
    memset(&vconfig, 0, sizeof(VConfig));

    //* 1. check whether there is a video stream.
    if(p->mMediaInfo.programIndex >= 0 &&
            p->mMediaInfo.programNum >= p->mMediaInfo.programIndex)
    {
        if(p->mMediaInfo.program[p->mMediaInfo.programIndex].videoNum > 0)
            bHasVideo = 1;
    }
    if(!bHasVideo)
    {
        logw("media file do not contain a video stream, getFrameAtTime() return fail.");
        bErr = 1;
        goto NEED_EXIT;
    }

    videoCodec =
        p->mMediaInfo.program[p->mMediaInfo.programIndex].video[0].eCodecFormat;
    if(videoCodec == VIDEO_CODEC_FORMAT_WMV1 ||
        videoCodec == VIDEO_CODEC_FORMAT_WMV2 ||
        videoCodec == VIDEO_CODEC_FORMAT_VP6 ||
        videoCodec == VIDEO_CODEC_FORMAT_H265 ||
        videoCodec == VIDEO_CODEC_FORMAT_VP9)
    {
        logw("Soft decoder thumb disable!");
        bErr = 1;
        goto NEED_EXIT;
    }

    //* 2. create a video encoder
    pVideoEncoder = VideoEncCreate(VENC_CODEC_H264);
    if(pVideoEncoder == NULL)
    {
        bErr = 1;
        goto NEED_EXIT;
    }

    pOutBuf = (unsigned char*)malloc(MAX_OUTPUT_STREAM_SIZE);
    if(pOutBuf == NULL)
    {
        bErr = 1;
        goto NEED_EXIT;
    }

    //* 3. create a video decoder.
    if(p->mVideoDecoder == NULL)
    {
        p->mVideoDecoder = CreateVideoDecoder();
        //* use decoder to capture thumbnail, decoder use less memory in this mode.
        vconfig.bThumbnailMode        = 0;

        //* all decoder support YUV_MB32_420 format.
        vconfig.eOutputPixelFormat    = PIXEL_FORMAT_NV21;
        //* no need to decode two picture when decoding a thumbnail picture.
        vconfig.bDisable3D            = 1;

        vconfig.nAlignStride        = 16;//* set align stride to 16 as default
#if 1
        //* set this flag when the parser can give this info, mov files recorded by
        //* iphone or android phone conains this info.

        //* set the rotation
        int nRotateDegree;
        int nRotation = atoi((const char*)p->mMediaInfo.rotate);

        if(nRotation == 0)
           nRotateDegree = 0;
        else if(nRotation == 90)
           nRotateDegree = 1;
        else if(nRotation == 180)
           nRotateDegree = 2;
        else if(nRotation == 270)
           nRotateDegree = 3;
        else
           nRotateDegree = 0;

        if(nRotateDegree != 0)
        {
            vconfig.bRotationEn         = 1;
            vconfig.nRotateDegree       = nRotateDegree;
        }else
        {
            vconfig.bRotationEn         = 0;
            vconfig.nRotateDegree       = 0;
        }

#endif
        /* set the picture scale down ratio, we generate a picture with pixel
         * size less than 640x480.
         */
        nHorizonScaleRatio = 0;
        nVerticalScaleRatio = 0;

        VideoStreamInfo *videoInfo =
                p->mMediaInfo.program[p->mMediaInfo.programIndex].video;
        setScaleRatio(&vconfig, videoInfo, &nHorizonScaleRatio, &nVerticalScaleRatio);

        //* set to the same scale ratio.
        if(nVerticalScaleRatio > nHorizonScaleRatio)
            nHorizonScaleRatio = nVerticalScaleRatio;
        else
            nVerticalScaleRatio = nHorizonScaleRatio;

        //* set scale ratio to vconfig.
        if(nHorizonScaleRatio || nVerticalScaleRatio)
        {
            vconfig.bScaleDownEn            = 1;
            vconfig.nHorizonScaleDownRatio    = nHorizonScaleRatio;
            vconfig.nVerticalScaleDownRatio = nVerticalScaleRatio;
        }
        vconfig.memops = MemAdapterGetOpsS();

        //* initialize the decoder.
        vconfig.nDeInterlaceHoldingFrameBufferNum = 0;//*not use deinterlace
        vconfig.nDisplayHoldingFrameBufferNum     = 0;//*gpu and decoder not share buffer
        vconfig.nRotateHoldingFrameBufferNum      = GetConfigParamterInt("pic_4rotate_num", 0);
        vconfig.nDecodeSmoothFrameBufferNum       = GetConfigParamterInt("pic_4smooth_num", 3);
        if(InitializeVideoDecoder(p->mVideoDecoder, videoInfo, &vconfig) != 0)
        {
            loge("initialize video decoder fail.");
            bErr = 1;
            goto NEED_EXIT;
        }
    }

    //* 4. seek parser to the specific position.
    if(p->mMediaInfo.bSeekable && timeUs != 0 && timeUs <
        ((int64_t)p->mMediaInfo.program[p->mMediaInfo.programIndex].duration*1000))
    {
        //* FIXME.
        //* we should seek to a position according to the 'option' param.
        //* option = 0 means seek to the sync frame privious to the timeUs.
        //* option = 1 means seek to the sync frame next to the timeUs.
        //* option = 2 means seek to the sync frame closest to the timeUs.
        //* option = 3 means seek to the closest frame to the timeUs.
        //* here we process all case as option = 0.
        if(CdxParserSeekTo(p->mParser, timeUs, 0) != 0)
        {
            loge("can not seek media file to the specific time %" PRId64 " us.", timeUs);
            bErr = 1;
            goto NEED_EXIT;
        }
    }
    else
    {
        logw("media file do not support seek operation, get frame from the begin.");
    }

    //* 5. loop to get bitstream.
    nStartTime     = GetSysTime();
    do
    {
        while(VideoStreamFrameNum(p->mVideoDecoder, 0) < 4)
        {
            //* 5.1 prefetch packet type and packet data size.
            if(CdxParserPrefetch(p->mParser, &packet) != 0)
            {
                //* prefetch fail, may be file end reached.
                bDone = 1;
                goto EOS_EXIT;
            }

            //* 5.2 feed data to the video decoder.
            if(packet.type == CDX_MEDIA_VIDEO && (packet.flags&MINOR_STREAM)==0)
            {
                ret = RequestVideoStreamBuffer(p->mVideoDecoder,
                                               packet.length,
                                               (char**)&packet.buf,
                                               &packet.buflen,
                                               (char**)&packet.ringBuf,
                                               &packet.ringBufLen,
                                               0);

                if(ret==0 && (packet.buflen+packet.ringBufLen)>=packet.length)
                {
                    if(CdxParserRead(p->mParser, &packet) == 0)
                    {
                        VideoStreamDataInfo dataInfo;
                        dataInfo.pData          = (char*)packet.buf;
                        dataInfo.nLength      = packet.length;
                        dataInfo.nPts          = packet.pts;
                        dataInfo.nPcr          = -1;
                        dataInfo.bIsFirstPart = 1;
                        dataInfo.bIsLastPart  = 1;
                        dataInfo.nStreamIndex = 0;
                        SubmitVideoStreamData(p->mVideoDecoder, &dataInfo, 0);
                    }
                    else
                    {
                        //* read data fail, may be data error.
                        loge("read packet from parser fail.");
                        bDone = 1;
                        goto EOS_EXIT;
                    }
                }
                else
                {
                    //* no buffer, may be the decoder is full of stream.
                    logw("waiting for stream buffer.");

                    bDone = 1;
                    goto EOS_EXIT;
                }
            }
            else
            {
                //* only process the major video stream.
                //* allocate a buffer to read uncare media data and skip it.
                packet.buf = malloc(packet.length);
                if(packet.buf != NULL)
                {
                    packet.buflen      = packet.length;
                    packet.ringBuf      = NULL;
                    packet.ringBufLen = 0;
                    if(CdxParserRead(p->mParser, &packet) == 0)
                    {
                        free(packet.buf);
                        continue;
                    }
                    else
                    {
                        free(packet.buf);
                        //* read data fail, may be data error.
                        loge("read packet from parser fail.");
                        bDone = 1;
                        goto EOS_EXIT;
                    }
                }
                else
                {
                    loge("can not allocate buffer for none video packet.");
                    bDone = 1;
                    goto EOS_EXIT;
                }
            }
        }

        logv("stream number: %d", VideoStreamFrameNum(p->mVideoDecoder, 0));

        //* 5.3 decode stream.
        ret = DecodeVideoStream(p->mVideoDecoder, 0 /*eos*/, 0/*key frame only*/,
                0/*drop b frame*/, 0/*current time*/);

        if(ret < 0)
        {
            loge("decode stream return fail.");
            bDone = 1;
            break;
        }
        else if(ret == VDECODE_RESULT_RESOLUTION_CHANGE)
        {
            logi("video resolution changed.");
            bDone = 1;
            break;
        }

        //* 5.4 try to get a picture from the decoder and encode it.
        if(ret == VDECODE_RESULT_FRAME_DECODED || ret == VDECODE_RESULT_KEYFRAME_DECODED)
        {
            pPicture = RequestPicture(p->mVideoDecoder, 0/*the major stream*/);
            if(pPicture != NULL)
            {
                bFrameCounter++;
                int enc_width  = 0;
                int enc_height = 0;
                if(bEncoderInited == 0)
                {

                    //* h264 param
                    memset(&h264Param, 0, sizeof(VencH264Param));
                    h264Param.bEntropyCodingCABAC = 1;
                    h264Param.nBitrate = 150000; /* bps */
                    h264Param.nCodingMode = VENC_FRAME_CODING;
                    h264Param.nMaxKeyInterval = 60;
                    h264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
                    h264Param.sProfileLevel.nLevel = VENC_H264Level31;
                    h264Param.sQPRange.nMinqp = 20;
                    h264Param.sQPRange.nMaxqp = 40;

                    VideoEncSetParameter(pVideoEncoder, VENC_IndexParamH264Param, &h264Param);

                    if(pPicture->nFrameRate > 20000)
                    {
                        h264Param.nFramerate = (pPicture->nFrameRate/1000)>>1;
                        bEncodeAllFrames = 0;
                    }
                    else if(pPicture->nFrameRate > 0)
                    {
                        h264Param.nFramerate = pPicture->nFrameRate/1000;
                        bEncodeAllFrames = 1;
                    }
                    else
                    {
                        h264Param.nFramerate = 15;  /* fps */
                        bEncodeAllFrames = 0;
                    }

                    //* init encoder config info
                    memset(&sBaseEncConfig, 0 ,sizeof(VencBaseConfig));

                    if((pPicture->nBottomOffset != 0 || pPicture->nRightOffset != 0) &&
                        pPicture->nRightOffset <= pPicture->nLineStride)
                    {
                        enc_width  = pPicture->nRightOffset - pPicture->nLeftOffset;
                        enc_height = pPicture->nBottomOffset - pPicture->nTopOffset;
                    }
                    else
                    {
                        enc_width  = pPicture->nWidth;
                        enc_height = pPicture->nHeight;
                    }

                    sBaseEncConfig.nInputWidth= (pPicture->nWidth);
                    sBaseEncConfig.nInputHeight = (pPicture->nHeight);
                    sBaseEncConfig.nStride = pPicture->nLineStride;

                    sBaseEncConfig.nDstWidth = 256;
                    sBaseEncConfig.nDstHeight = 144;
                    sBaseEncConfig.eInputFormat = VENC_PIXEL_YVU420SP;

                    logd("w:%d, h:%d, stride:%d", sBaseEncConfig.nInputWidth,
                            sBaseEncConfig.nInputHeight, pPicture->nLineStride);
                    logd("nRightOffset:%d, nLeftOffset:%d, nBottomOffset:%d, nTopOffset:%d",
                            pPicture->nRightOffset, pPicture->nLeftOffset,
                            pPicture->nBottomOffset, pPicture->nTopOffset);

                    if(VideoEncInit(pVideoEncoder, &sBaseEncConfig)!=0)
                    {
                        loge("VideoEncInit failed");
                        bErr = 1;
                        goto NEED_EXIT;
                    }

                    VideoEncGetParameter(pVideoEncoder, VENC_IndexParamH264SPSPPS, &spsppsInfo);

                    if(spsppsInfo.pBuffer != 0 &&
                            (spsppsInfo.nLength + 8 <= MAX_OUTPUT_STREAM_SIZE))
                    {
                        int frameRate =  h264Param.nFramerate*1000;
                        pOutBuf[bBitStreamLength++] = 't';
                        pOutBuf[bBitStreamLength++] = 'h';
                        pOutBuf[bBitStreamLength++] = 'u';
                        pOutBuf[bBitStreamLength++] = 'm';

                        pOutBuf[bBitStreamLength++]    = frameRate        & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameRate>>8) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameRate>>16) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameRate>>24) & 0xff;
                        pOutBuf[bBitStreamLength++]    = spsppsInfo.nLength     & 0xff;
                        pOutBuf[bBitStreamLength++]    = (spsppsInfo.nLength>>8) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (spsppsInfo.nLength>>16) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (spsppsInfo.nLength>>24) & 0xff;
                        memcpy(pOutBuf + bBitStreamLength, (unsigned char*)spsppsInfo.pBuffer,
                                (int)spsppsInfo.nLength);
                        bBitStreamLength += spsppsInfo.nLength;

#if SAVE_BITSTREAM
                        fwrite(spsppsInfo.pBuffer, 1, spsppsInfo.nLength, fph264);
#endif

                    }
                    else
                    {
                        loge("can not get sps and pps from encoder.");
                        bErr = 1;
                        goto NEED_EXIT;
                    }

                    bEncoderInited = 1;
                }

                if(bEncodeAllFrames || (bFrameCounter & 1))
                {
                    if((pPicture->nBottomOffset != 0 || pPicture->nRightOffset != 0) &&
                        pPicture->nRightOffset <= pPicture->nLineStride)
                    {
                        enc_width  = pPicture->nRightOffset - pPicture->nLeftOffset;
                        enc_height = pPicture->nBottomOffset - pPicture->nTopOffset;
                    }
                    else
                    {
                        enc_width  = pPicture->nWidth;
                        enc_height = pPicture->nHeight;
                    }

                    memset(&sInputBuffer, 0, sizeof(VencInputBuffer));
                    sInputBuffer.pAddrPhyY =
                        (unsigned char*)CdcMemGetPhysicAddressCpu(p->memops,
                                pPicture->pData0);
                    sInputBuffer.pAddrPhyC =
                        (unsigned char*)CdcMemGetPhysicAddressCpu(p->memops,
                                pPicture->pData1);
                    if((enc_width <= pPicture->nWidth) || (enc_height <= pPicture->nHeight))
                    {
                        sInputBuffer.bEnableCorp = 1;
                        sInputBuffer.sCropInfo.nWidth = enc_width;
                        sInputBuffer.sCropInfo.nHeight= enc_height;
                        sInputBuffer.sCropInfo.nLeft  = pPicture->nLeftOffset;
                        sInputBuffer.sCropInfo.nTop   = pPicture->nTopOffset ;
                    }

                    AddOneInputBuffer(pVideoEncoder, &sInputBuffer);
                    ret = VideoEncodeOneFrame(pVideoEncoder);
                    if(ret != VENC_RESULT_OK)
                    {
                        loge("encoder error");
                        bErr = 1;
                        goto NEED_EXIT;
                    }
                    AlreadyUsedInputBuffer(pVideoEncoder,&sInputBuffer);

                    memset(&sOutputBuffer, 0, sizeof(VencOutputBuffer));
                    GetOneBitstreamFrame(pVideoEncoder, &sOutputBuffer);
                    if((sOutputBuffer.nSize0 + sOutputBuffer.nSize1 + 4) <= MAX_OUTPUT_STREAM_SIZE)
                    {
                        int frameSize = sOutputBuffer.nSize0 + sOutputBuffer.nSize1;

                        pOutBuf[bBitStreamLength++]    = frameSize & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameSize>>8) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameSize>>16) & 0xff;
                        pOutBuf[bBitStreamLength++]    = (frameSize>>24) & 0xff;
                        memcpy(pOutBuf + bBitStreamLength, sOutputBuffer.pData0,
                                sOutputBuffer.nSize0);

                        bBitStreamLength += sOutputBuffer.nSize0;
                        if(sOutputBuffer.pData1 != NULL && sOutputBuffer.nSize1 > 0)
                        {
                            memcpy(pOutBuf + bBitStreamLength, sOutputBuffer.pData1,
                                    sOutputBuffer.nSize1);
                            bBitStreamLength += sOutputBuffer.nSize1;
                        }
                    }

#if SAVE_BITSTREAM
                    fwrite(sOutputBuffer.pData0, 1, sOutputBuffer.nSize0, fph264);
                    if(sOutputBuffer.pData1 != NULL && sOutputBuffer.nSize1 > 0)
                    {
                        fwrite(sOutputBuffer.pData1, 1, sOutputBuffer.nSize1, fph264);
                    }
#endif
                    FreeOneBitStreamFrame(pVideoEncoder, &sOutputBuffer);
                }

                ReturnPicture(p->mVideoDecoder, pPicture);
            }
        }

        //* check whether cost too long time or process too much packets.
        nTimePassed = GetSysTime() - nStartTime;
        if(nTimePassed > MAX_TIME_TO_GET_A_STREAM)
        {
            logw("cost more than %d us to get a steam, quit.", MAX_TIME_TO_GET_A_FRAME);
            bDone = 1;
            break;
        }

        if(bFrameCounter >= 150)
        {
            bDone = 1;
            break;
        }

    }while(!bDone);

EOS_EXIT:
    if (pOutBuf && bBitStreamLength)
    {
        p->mVideoStream.mBuf = pOutBuf;
        p->mVideoStream.mSize = bBitStreamLength;
    }

    logv("bBitStreamLength: %p,%d", pOutBuf, bBitStreamLength);

NEED_EXIT:
    if(pVideoEncoder)
    {
        VideoEncUnInit(pVideoEncoder);
        VideoEncDestroy(pVideoEncoder);
        pVideoEncoder = NULL;
    }

    if(pOutBuf)
    {
        free(pOutBuf);
        pOutBuf = NULL;
    }

    if(p->mVideoDecoder)
    {
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
    }

#if SAVE_BITSTREAM
    fclose(fph264);
    fph264 = NULL;
#endif

    if(bErr == 1)
        return NULL;
    else
        return &p->mVideoStream;
}


int XRetrieverGetMetaData(XRetriever* v, int type, void* pVal)
{
    XRetrieverContext* p;
    p = (XRetrieverContext*)v;

    switch(type)
    {
        case METADATA_VIDEO_WIDTH:
        {
            int *w = (int*)pVal;
            logd("w: %d", p->mMediaInfo.program[0].video[0].nWidth);
            *w = p->mMediaInfo.program[0].video[0].nWidth;
            break;
        }

        case METADATA_VIDEO_HEIGHT:
        {
            int *h = (int*)pVal;
            logd("h: %d", p->mMediaInfo.program[0].video[0].nHeight);
            *h = p->mMediaInfo.program[0].video[0].nHeight;
            break;
        }

        case METADATA_DURATION:
        {
            int *duration = (int*)pVal;
            *duration = p->mMediaInfo.program[0].duration;
            break;
        }
        case METADATA_MEDIAINFO:
        {
            CdxMediaInfoT *mi = (CdxMediaInfoT *)pVal;
            *mi = p->mMediaInfo;
            break;
        }
        case METADATA_PARSER_TYPE:
        {
            int *type = (int*)pVal;
            *type = p->mParser->type;
            break;
        }
        default:
        {
            logw("unknown type: %d", type);
            return -1;
        }
    }

    return 0;
}

static int64_t GetSysTime()
{
    int64_t time;
    struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t)t.tv_sec * 1000000;
    time += t.tv_usec;
    return time;
}

static int transformPicture(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 XVideoFrame* pVideoFrame)
{
    unsigned short*  pDst;
    unsigned char*   pSrcY;
    unsigned char*   pSrcU;
    unsigned char*   pSrcV;
    int              y;
    int              x;
    unsigned char*   pClipTable;
    unsigned char*   pClip;

    static const int nClipMin = -278;
    static const int nClipMax = 535;

    if((pPicture->ePixelFormat!= PIXEL_FORMAT_YV12) &&
            (pPicture->ePixelFormat!= PIXEL_FORMAT_YUV_PLANER_420))
    {
        loge("source pixel format is not YV12, quit.");
        return -1;
    }

    //* initialize the clip table.
    pClipTable = (unsigned char*)malloc(nClipMax - nClipMin + 1);
    if(pClipTable == NULL)
    {
        loge("can not allocate memory for the clip table, quit.");
        return -1;
    }
    for(x=nClipMin; x<=nClipMax; x++)
    {
        pClipTable[x-nClipMin] = (x<0) ? 0 : (x>255) ? 255 : x;
    }
    pClip = &pClipTable[-nClipMin];

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nLineStride*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nLineStride*pPicture->nHeight/4);
    CdcMemFlushCache(memOps, pPicture->pData2, pPicture->nLineStride*pPicture->nHeight/4);

    //* set pointers.
    pDst  = (unsigned short*)pVideoFrame->mData;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    if(pPicture->ePixelFormat== PIXEL_FORMAT_YV12)
    {
        pSrcV = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;
        pSrcU = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;
    }
    else
    {
        pSrcU = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                    (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;
        pSrcV = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                    (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;
    }

    for(y = 0; y < (int)pVideoFrame->mHeight; ++y)
    {
        for(x = 0; x < (int)pVideoFrame->mWidth; x += 2)
        {
            // B = 1.164 * (Y - 16) + 2.018 * (U - 128)
            // G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128)
            // R = 1.164 * (Y - 16) + 1.596 * (V - 128)

            // B = 298/256 * (Y - 16) + 517/256 * (U - 128)
            // G = .................. - 208/256 * (V - 128) - 100/256 * (U - 128)
            // R = .................. + 409/256 * (V - 128)

            // min_B = (298 * (- 16) + 517 * (- 128)) / 256 = -277
            // min_G = (298 * (- 16) - 208 * (255 - 128) - 100 * (255 - 128)) / 256 = -172
            // min_R = (298 * (- 16) + 409 * (- 128)) / 256 = -223

            // max_B = (298 * (255 - 16) + 517 * (255 - 128)) / 256 = 534
            // max_G = (298 * (255 - 16) - 208 * (- 128) - 100 * (- 128)) / 256 = 432
            // max_R = (298 * (255 - 16) + 409 * (255 - 128)) / 256 = 481

            // clip range -278 .. 535

            signed y1 = (signed)pSrcY[x] - 16;
            signed y2 = (signed)pSrcY[x + 1] - 16;

            signed u = (signed)pSrcU[x / 2] - 128;
            signed v = (signed)pSrcV[x / 2] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            unsigned int rgb1 = ((pClip[r1] >> 3) << 11) |
                                ((pClip[g1] >> 2) << 5)  |
                                (pClip[b1] >> 3);

            unsigned int rgb2 = ((pClip[r2] >> 3) << 11) |
                                ((pClip[g2] >> 2) << 5)  |
                                (pClip[b2] >> 3);

            *(unsigned int *)(&pDst[x]) = (rgb2 << 16) | rgb1;
        }

        pSrcY += pPicture->nLineStride;

        if(y & 1)
        {
            pSrcU += pPicture->nLineStride / 2;
            pSrcV += pPicture->nLineStride / 2;
        }

        pDst += pVideoFrame->mWidth;
    }

    free(pClipTable);

    return 0;
}
