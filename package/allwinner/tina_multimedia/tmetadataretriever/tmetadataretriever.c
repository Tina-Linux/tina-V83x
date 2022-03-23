/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : tmetadatatriever.c
 * Description : tmetadatatriever for Linux,
 *               get the video thumbnails
 * History :
 *
 */

#define LOG_TAG "tmetadataretriever"
#include "tlog.h"
#include "tmetadataretriever.h"
#include <errno.h>

#include "memoryAdapter.h"
#include "iniparserapi.h"
#include "tina_multimedia_version.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>

#define SAVE_YUV 0
#define SAVE_RGB 0

#define MAX_PACKET_COUNT_TO_GET_A_FRAME 4096    /* process 4096 packets to get a frame at maximum. */
#define MAX_TIME_TO_GET_A_FRAME         5000000         /* use 5 seconds to get a frame at maximum. */

typedef struct TRetrieverContext
{
    CdxDataSourceT              mSource;
    CdxMediaInfoT                 mMediaInfo;
    CdxParserT*                    mParser;
    CdxStreamT*                  mStream;
    VideoDecoder*                mVideoDecoder;
    VideoFrame                     mVideoFrame;
    struct ScMemOpsS *      memops;
    int                                   mScaleDownEn;
    int                                   mHorizonScaleDownRatio;
    int                                   mVerticalScaleDownRatio;
    int                                   mDecodedPixFormat;
    int                                   mOutputDataType;
}TRetrieverContext;

static int64_t GetSysTime()
{
    int64_t time;
    struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t)t.tv_sec * 1000000;
    time += t.tv_usec;
    return time;
}


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

        TLOGD("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
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
    TLOGD("*****in save_bmp_rgb565:success=%d", success);
    return success;
}
#endif

static int saveYuvPic(TRetrieverContext* retrieverContext,VideoFrame* videoFrame){
    if(retrieverContext == NULL || videoFrame == NULL ||videoFrame->mYuvData== NULL){
        TLOGE("retrieverContext or imgFrame or imgFrame->mData is null,saveYuvPic err");
        return -1;
    }
    char path[50];
    char acturalWidth[10];
    char acturalHeight[10];
    sprintf(acturalWidth,"%u",videoFrame->mDisplayWidth);
    sprintf(acturalHeight,"%u",videoFrame->mDisplayHeight);
    TLOGD("acturalWidth = %s,height = %s",acturalWidth,acturalHeight);
    strcpy(path,"/tmp/save_");
    strcat(path,acturalWidth);
    strcat(path,"_");
    strcat(path,acturalHeight);
    switch (retrieverContext->mOutputDataType)
    {
        case TmetadataRetrieverOutputDataNV21:
            strcat(path,"_nv21.dat");
            break;
        case TmetadataRetrieverOutputDataYV12:
            strcat(path,"_yv12.dat");
            break;
        default:
            TLOGW("the retrieverContext->mOutputDataType = %d,which is not support,use the default:%d",retrieverContext->mOutputDataType,TmetadataRetrieverOutputDataNV21);
            strcat(path,"_nv21.dat");
            break;
    }
    FILE* savaYuvFd = fopen(path, "wb");
    if(savaYuvFd==NULL){
	TLOGE("fopen save.yuv fail****");
	TLOGE("err str: %s",strerror(errno));
        return -1;
    }else{
	fseek(savaYuvFd,0,SEEK_SET);
	int write_ret = fwrite(videoFrame->mYuvData, 1, videoFrame->mYuvSize, savaYuvFd);
	if(write_ret <= 0){
	    TLOGE("yuv write error,err str: %s",strerror(errno));
            fclose(savaYuvFd);
	    savaYuvFd = NULL;
            return -1;
	}
	fclose(savaYuvFd);
	savaYuvFd = NULL;
    }
    TLOGD("save yuv data successfully,path = %s",path);
    return 0;
}


static int transformPicture(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 VideoFrame* pVideoFrame)
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
        TLOGE("source pixel format is not YV12, quit.");
        return -1;
    }

    //* initialize the clip table.
    pClipTable = (unsigned char*)malloc(nClipMax - nClipMin + 1);
    if(pClipTable == NULL)
    {
        TLOGE("can not allocate memory for the clip table, quit.");
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
    pDst  = (unsigned short*)pVideoFrame->mRGB565Data;
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

    for(y = 0; y < (int)pVideoFrame->mDisplayHeight; ++y)
    {
        for(x = 0; x < (int)pVideoFrame->mDisplayWidth; x += 2)
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

        pSrcY += pVideoFrame->mDisplayWidth;

        if(y & 1)
        {
            pSrcU += pVideoFrame->mDisplayWidth / 2;
            pSrcV += pVideoFrame->mDisplayWidth / 2;
        }

        pDst += pVideoFrame->mDisplayWidth;
    }

    free(pClipTable);

#if SAVE_RGB
    FILE* outFp = fopen("/tmp/rgb.bmp", "wb");
    if(outFp != NULL)
    {
        TLOGD("************save_bmp_rgb565");
        save_bmp_rgb565(outFp, pVideoFrame->mDisplayWidth, pVideoFrame->mDisplayHeight, pVideoFrame->mRGB565Data);
        fclose(outFp);
    }
#endif

    return 0;
}

static void clear(TRetriever* v)
{
    TRetrieverContext* p;
    p = (TRetrieverContext*)v;

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

    if(p->mVideoFrame.mRGB565Data)
    {
        free(p->mVideoFrame.mRGB565Data);
        p->mVideoFrame.mRGB565Data = NULL;
    }
	if (p->mVideoFrame.mYuvData) {
		free(p->mVideoFrame.mYuvData);
		p->mVideoFrame.mYuvData = NULL;
	}
    memset(&p->mVideoFrame, 0x00, sizeof(VideoFrame));
}

TRetriever* TRetrieverCreate()
{
    TRetrieverContext* p;
    p = (TRetrieverContext*)malloc(sizeof(TRetrieverContext));
    if(!p)
    {
        TLOGE("TRetrieverCreate fail");
        return NULL;
    }
    LogTinaVersionInfo();
    memset(p, 0, sizeof(TRetrieverContext));

    memset(&p->mSource, 0, sizeof(CdxDataSourceT));

    p->memops = MemAdapterGetOpsS();
    if(p->memops == NULL)
    {
        TLOGE("MemAdapterGetOpsS fail");
        free(p);
        return NULL;
    }
    CdcMemOpen(p->memops);

    return (TRetriever*)p;
}

int  TRetrieverDestory(TRetriever* v)
{
    TRetrieverContext* p;
    p = (TRetrieverContext*)v;
    clear(v);
    CdcMemClose(p->memops);
    free(p);
    return 0;
}

static int retrieverSetDataSource(TRetrieverContext* p, const char* pUrl)
{
    if(strstr(pUrl, "://") != NULL)
    {
        p->mSource.uri = strdup(pUrl);
        if(p->mSource.uri == NULL)
        {
            TLOGE("can not dump string of uri.");
            return -1;
        }
    }
    else
    {
        p->mSource.uri  = (char*)malloc(strlen(pUrl)+8);
        if(p->mSource.uri == NULL)
        {
            TLOGE("can not dump string of uri.");
            return -1;
        }
        sprintf(p->mSource.uri, "file://%s", pUrl);
    }
    return 0;
}

int TRetrieverSetDataSource(TRetriever* v, const char* pUrl,TmetadataRetrieverScaleDownRatio scaleRatio,TmetadataRetrieverOutputDataType outputType)
{
    int ret;
    TRetrieverContext* p;
    p = (TRetrieverContext*)v;
    clear(v);

    //1. set the datasource object.
    ret = retrieverSetDataSource(p, pUrl);
    if(ret < 0)
    {
        TLOGE("set datasource fail.");
        return -1;
    }

    TLOGD("uri: %s", p->mSource.uri);

    //2.create a stream
    p->mStream = CdxStreamCreate(&p->mSource);
    if(!p->mStream)
    {
        TLOGE("CdxStreamCreate fail.");
        return -1;
    }
    ret = CdxStreamConnect(p->mStream);
    if(ret < 0)
    {
        TLOGE("CdxStreamConnect fail ,ret = %d",ret);
        CdxStreamClose(p->mStream);
        p->mStream = NULL;
        return -1;
    }

    //3.create a parser
    p->mParser = CdxParserCreate(p->mStream, MUTIL_AUDIO);
    if(!p->mParser)
    {
        TLOGE("parser creat fail.");
        CdxStreamClose(p->mStream);
        p->mStream = NULL;
        return -1;
    }
    ret = CdxParserInit(p->mParser);
    if(ret < 0)
    {
        TLOGE("CdxParserInit fail ,ret = %d",ret);
        CdxParserClose(p->mParser);
        p->mParser = NULL;
        p->mStream = NULL;
        return -1;
    }

    //4. get media info.
    memset(&p->mMediaInfo, 0, sizeof(CdxMediaInfoT));
    if(CdxParserGetMediaInfo(p->mParser, &p->mMediaInfo) == 0)
    {
        if (p->mParser->type == CDX_PARSER_TS ||
            p->mParser->type == CDX_PARSER_BD ||
            p->mParser->type == CDX_PARSER_HLS)
        {
            p->mMediaInfo.program[0].video[0].bIsFramePackage = 0; /* stream package,this will set to video decoder */
        }
        else
        {
            p->mMediaInfo.program[0].video[0].bIsFramePackage = 1; /* frame package,this will set to video decoder*/
        }
    }

    //5.store the scale ratio
    switch (scaleRatio)
    {
        case TMETADATA_RETRIEVER_SCALE_DOWN_1:
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
        case TMETADATA_RETRIEVER_SCALE_DOWN_2:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 1;
            p->mVerticalScaleDownRatio = 1;
            break;
        case TMETADATA_RETRIEVER_SCALE_DOWN_4:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 2;
            p->mVerticalScaleDownRatio = 2;
            break;
        case TMETADATA_RETRIEVER_SCALE_DOWN_8:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 3;
            p->mVerticalScaleDownRatio = 3;
            break;
        default:
            TLOGW("the scalRatio = %d,which is not support,use the default",scaleRatio);
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
    }

    //store the output data type and set the decoded pixformat
    p->mOutputDataType = outputType;
    switch (outputType)
    {
        case TmetadataRetrieverOutputDataNV21:
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
        case TmetadataRetrieverOutputDataYV12:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        case TmetadataRetrieverOutputDataRGB565:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        default:
            TLOGW("the outputType = %d,which is not support,use the default:%d",outputType,TmetadataRetrieverOutputDataNV21);
            p->mOutputDataType = TmetadataRetrieverOutputDataNV21;
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
    }
    return 0;
}

VideoFrame *TRetrieverGetFrameAtTime(TRetriever* v, int64_t timeUs)
{
    VideoPicture*     pPicture;
    int               bDone;
    int               bSuccess;
    int               bHasVideo;

    VConfig           vconfig;
    CdxPacketT        packet;
    int               ret;
    int               nPacketCount;
    int64_t           nStartTime;
    int64_t           nTimePassed;

    TRetrieverContext* p;
    p = (TRetrieverContext*)v;

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
        TLOGE("media file do not contain a video stream, getFrameAtTime() return fail.");
        return NULL;
    }

    //* 2. create a video decoder.
    if(p->mVideoDecoder == NULL)
    {
        p->mVideoDecoder = CreateVideoDecoder();
        //* use decoder to capture thumbnail, decoder use less memory in this mode.
        vconfig.bThumbnailMode      = 1;
        //set the decoded output pixel format
        vconfig.eOutputPixelFormat = p->mDecodedPixFormat;
        //* no need to decode two picture when decoding a thumbnail picture.
        vconfig.bDisable3D          = 1;
        //* set align stride to 16 as defualt
        vconfig.nAlignStride        = 16;
        //* set this flag when the parser can give this info,
        //* mov files recorded by iphone or android phone
        //* conains this info.

        //* set the rotation
        int nRotateDegree;
        int nRotation = atoi((const char*)p->mMediaInfo.rotate);
        TLOGD("nRotation = %d",nRotation);

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

        VideoStreamInfo *videoInfo =
                        p->mMediaInfo.program[p->mMediaInfo.programIndex].video;
        vconfig.bScaleDownEn = p->mScaleDownEn;
        vconfig.nHorizonScaleDownRatio = p->mHorizonScaleDownRatio;
        vconfig.nVerticalScaleDownRatio = p->mVerticalScaleDownRatio;

        //* set the vbv buffer size
        vconfig.nVbvBufferSize = 4*1024*1024;

        vconfig.nDeInterlaceHoldingFrameBufferNum = 0;//GetConfigParamterInt("pic_4di_num", 2);
        vconfig.nDisplayHoldingFrameBufferNum = 0;
        vconfig.nRotateHoldingFrameBufferNum = GetConfigParamterInt("pic_4rotate_num", 0);
        vconfig.nDecodeSmoothFrameBufferNum = GetConfigParamterInt("pic_4smooth_num", 3);

        vconfig.memops = p->memops;
        if(InitializeVideoDecoder(p->mVideoDecoder,
            videoInfo, &vconfig) != 0)
        {
            TLOGE("initialize video decoder fail.");
            return NULL;
        }
    }
     if(timeUs < 0 || p->mMediaInfo.program[p->mMediaInfo.programIndex].duration < timeUs/1000)
     {
         timeUs = (p->mMediaInfo.program[p->mMediaInfo.programIndex].duration >> 1) * 1000;
     }
    //* 3. seek parser to the specific position.
    if(p->mMediaInfo.bSeekable && timeUs > 0 &&
        timeUs < ((int64_t)p->mMediaInfo.program[p->mMediaInfo.programIndex].duration*1000))
    {
        if(CdxParserSeekTo(p->mParser, timeUs,0) != 0)
        {
            TLOGE("can not seek media file to the specific time %" PRId64 " us.", timeUs);
            return NULL;
        }
    }
    else
    {
        TLOGW("media file do not support seek operation, get frame from the begin.");
    }

    //* 4. loop to decode a picture.
    nPacketCount = 0;
    nStartTime   = GetSysTime();
    do
    {
        //* 4.1 prefetch packet type and packet data size.
        packet.flags = 0;
        if(CdxParserPrefetch(p->mParser, &packet) != 0)
        {
            //* prefetch fail, may be file end reached.
            TLOGE("prefetch fail,may be file end reached");
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
                    TLOGE("read packet from parser fail.");
                    bDone = 1;
                    bSuccess = 0;
                    break;
                }
            }
            else
            {
                //* no buffer, may be the decoder is full of stream.
                TLOGW("waiting for stream buffer.");
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
                    TLOGE("read packet from parser fail.");
                    bDone = 1;
                    bSuccess = 0;
                    break;
                }
            }
            else
            {
                TLOGE("can not allocate buffer for none video packet.");
                bDone = 1;
                bSuccess = 0;
                break;
            }
        }

        //* 4.3 decode stream.
        ret = DecodeVideoStream(p->mVideoDecoder, 0 /*eos*/,
                0/*key frame only*/, 0/*drop b frame*/, 0/*current time*/);
        TLOGD("DecodeVideoStream return value = %d",ret);
        if(ret < 0)
        {
            TLOGE("decode stream return fail.");
            bDone = 1;
            bSuccess = 0;
            break;
        }
        else if(ret == VDECODE_RESULT_RESOLUTION_CHANGE)
        {
            TLOGD("video resolution changed , ReopenVideoEngine");
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
                bDone = 1;
                bSuccess = 1;
                break;
            }
        }

        //* check whether cost too long time or process too much packets.
        nTimePassed = GetSysTime() - nStartTime;
        if(nTimePassed > MAX_TIME_TO_GET_A_FRAME || nTimePassed < 0)
        {
            TLOGW("cost more than %d us but can not get a picture, quit.", MAX_TIME_TO_GET_A_FRAME);
            bDone = 1;
            bSuccess = 0;
            break;
        }
        if(nPacketCount > MAX_PACKET_COUNT_TO_GET_A_FRAME)
        {
            TLOGW("process more than %d packets but can not get a picture, quit.",
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
        p->mVideoFrame.mRotationAngle = 0;

        if(p->mOutputDataType == TmetadataRetrieverOutputDataNV21 || p->mOutputDataType == TmetadataRetrieverOutputDataYV12){
            if(p->mVideoFrame.mYuvData != NULL){//if the mYuvData is not NULL,we should free the buffer which is malloc before
                free(p->mVideoFrame.mYuvData);
                memset(&p->mVideoFrame, 0x00, sizeof(VideoFrame));
            }
            p->mVideoFrame.mWidth = pPicture->nWidth;
            p->mVideoFrame.mHeight = pPicture->nHeight;
            p->mVideoFrame.mDisplayWidth = pPicture->nRightOffset-pPicture->nLeftOffset;
            p->mVideoFrame.mDisplayHeight = pPicture->nBottomOffset-pPicture->nTopOffset;
            p->mVideoFrame.mYuvSize = p->mVideoFrame.mDisplayWidth*p->mVideoFrame.mDisplayHeight*3/2;
            p->mVideoFrame.mYuvData = (unsigned char*)malloc(p->mVideoFrame.mYuvSize);
            if(p->mVideoFrame.mYuvData == NULL)
            {
                TLOGE("mVideoFrame.mYuvData malloc fail");
                return NULL;
            }
            /* flush cache*/
            if(p->mOutputDataType == TmetadataRetrieverOutputDataYV12){
                CdcMemFlushCache(p->memops, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
                CdcMemFlushCache(p->memops, pPicture->pData1, pPicture->nHeight*pPicture->nHeight/4);
                CdcMemFlushCache(p->memops, pPicture->pData2, pPicture->nHeight*pPicture->nHeight/4);
                /**copy actural picture data to imgframe**/
                memcpy(p->mVideoFrame.mYuvData,pPicture->pData0,p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight);
                memcpy(p->mVideoFrame.mYuvData+p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight,pPicture->pData1,p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight/4);
                memcpy(p->mVideoFrame.mYuvData+p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight*5/4,pPicture->pData2,p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight/4);
            }else{
                CdcMemFlushCache(p->memops, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
                CdcMemFlushCache(p->memops, pPicture->pData1, pPicture->nHeight*pPicture->nHeight/2);
                /**copy actural picture data to imgframe**/
                memcpy(p->mVideoFrame.mYuvData,pPicture->pData0,p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight);
                memcpy(p->mVideoFrame.mYuvData+p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight,pPicture->pData1,p->mVideoFrame.mDisplayWidth *p->mVideoFrame.mDisplayHeight/2);
            }
#if SAVE_YUV
            saveYuvPic(p,&p->mVideoFrame);
#endif
        }else if( p->mOutputDataType == TmetadataRetrieverOutputDataRGB565){
            if(p->mVideoFrame.mRGB565Data != NULL){//if the mRGB565Data is not NULL,we should free the buffer which is malloc before
                free(p->mVideoFrame.mRGB565Data);
                memset(&p->mVideoFrame, 0x00, sizeof(VideoFrame));
            }
            p->mVideoFrame.mWidth         = pPicture->nWidth;
            p->mVideoFrame.mHeight        = pPicture->nHeight;
            p->mVideoFrame.mDisplayWidth  = pPicture->nRightOffset - pPicture->nLeftOffset;
            p->mVideoFrame.mDisplayHeight = pPicture->nBottomOffset - pPicture->nTopOffset;
            p->mVideoFrame.mRGB565Size = p->mVideoFrame.mDisplayWidth * p->mVideoFrame.mDisplayHeight * 2;
            p->mVideoFrame.mRGB565Data= (unsigned char*)malloc(p->mVideoFrame.mRGB565Size);
            if(p->mVideoFrame.mRGB565Data == NULL)
            {
                TLOGE("can not allocate memory for video frame.");
                return NULL;
            }
            transformPicture(p->memops, pPicture, &p->mVideoFrame);
        }else{
            TLOGE("fail:the outputType = %d,whic is not support",p->mOutputDataType);
        }
        /**return the picture buf to video decoder**/
        ReturnPicture(p->mVideoDecoder,pPicture);
        return &p->mVideoFrame;
    }
    else
    {
        TLOGE("cannot decode a picture.");
        return NULL;
    }
}

int TRetrieverGetMetaData(TRetriever* v, TMetaDataType type, void* pVal)
{
    TRetrieverContext* p;
    p = (TRetrieverContext*)v;
    int ret = 0;
    switch(type)
    {
        case TMETADATA_VIDEO_WIDTH:
        {
            int *w = (int*)pVal;
            TLOGD("video width : %d", p->mMediaInfo.program[0].video[0].nWidth);
            *w = p->mMediaInfo.program[0].video[0].nWidth;
            break;
        }

        case TMETADATA_VIDEO_HEIGHT:
        {
            int *h = (int*)pVal;
            TLOGD("hvideo height : %d", p->mMediaInfo.program[0].video[0].nHeight);
            *h = p->mMediaInfo.program[0].video[0].nHeight;
            break;
        }

        case TMETADATA_DURATION:
        {
            int *duration = (int*)pVal;
            *duration = p->mMediaInfo.program[0].duration;
            break;
        }
        case TMETADATA_MEDIAINFO:
        {
            CdxMediaInfoT *mi = (CdxMediaInfoT *)pVal;
            *mi = p->mMediaInfo;
            break;
        }
        case TMETADATA_PARSER_TYPE:
        {
            int *type = (int*)pVal;
            *type = p->mParser->type;
            break;
        }
        default:
        {
            TLOGW("unknown type: %d", type);
            ret = -1;
            break;
        }
    }
    return ret;
}
