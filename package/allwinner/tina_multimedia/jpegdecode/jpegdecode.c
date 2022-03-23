/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : jpegdecode.c
 * Description : jpegdecode for Linux,
 *               decode jpeg image(base line format)
 * History :
 *
 */

#define LOG_TAG "jpegdecode"
#include "tlog.h"
#include "jpegdecode.h"
#include "tina_multimedia_version.h"
#include <errno.h>
#include <sys/time.h>

#define SAVE_YUV 0
#define SAVE_RGB 0
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

         TLOGD("BMP:rgb565:%dbit,%d*%d,%d", info->bmiHeader.biBitCount, w, h, head->bfSize);
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
    TLOGD("*****save_bmp_rgb565 success=%d", success);
    return success;
}
#endif

static char * readJpegData(char *path, int *pLen)
{
    FILE *fp = NULL;
    int ret = 0;
    char *data = NULL;

    fp = fopen(path, "rb");
    if(fp == NULL)
    {
        TLOGE("read jpeg file error, errno(%d)", errno);
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    *pLen = ftell(fp);
    rewind(fp);
    data = (char *) malloc (sizeof(char)*(*pLen));

    if(data == NULL)
      {
          TLOGE("malloc memory fail");
          fclose(fp);
          return NULL;
      }

    ret = fread (data,1,*pLen,fp);
    if (ret != *pLen)
    {
        TLOGE("read file fail");
        fclose(fp);
        free(data);
        return NULL;
    }

    if(fp != NULL)
    {
        fclose(fp);
    }
    return data;
}

static int saveYuvPic(JpegDecoderContext* decContext,ImgFrame* imgFrame){
    if(decContext == NULL || imgFrame == NULL ||imgFrame->mYuvData== NULL){
        TLOGE("decContext or imgFrame or imgFrame->mData is null,saveYuvPic err");
        return -1;
    }
    char path[50];
    char acturalWidth[10];
    char acturalHeight[10];
    sprintf(acturalWidth,"%u",imgFrame->mDisplayWidth);
    sprintf(acturalHeight,"%u",imgFrame->mDisplayHeight);
    TLOGD("acturalWidth = %s,height = %s",acturalWidth,acturalHeight);
    strcpy(path,"/tmp/save_");
    strcat(path,acturalWidth);
    strcat(path,"_");
    strcat(path,acturalHeight);
    switch (decContext->mOutputDataType)
    {
        case JpegDecodeOutputDataNV21:
            strcat(path,"_nv21.dat");
            break;
        case JpegDecodeOutputDataNV12:
            strcat(path,"_nv12.dat");
            break;
        case JpegDecodeOutputDataYU12:
            strcat(path,"_yu12.dat");
            break;
        case JpegDecodeOutputDataYV12:
            strcat(path,"_yv12.dat");
            break;
        default:
            TLOGW("the decContext->mOutputDataType = %d,which is not support,use the default:%d",decContext->mOutputDataType,JpegDecodeOutputDataNV21);
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
	int write_ret = fwrite(imgFrame->mYuvData, 1, imgFrame->mYuvSize, savaYuvFd);
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

static int transformYV12toRGB565(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 ImgFrame* pImgFrame)
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
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nWidth*pPicture->nHeight/4);
    CdcMemFlushCache(memOps, pPicture->pData2, pPicture->nWidth*pPicture->nHeight/4);

    //* set pointers.
    pDst  = (unsigned short*)pImgFrame->mRGB565Data;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    pSrcV = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    pSrcU = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    for(y = 0; y < (int)pImgFrame->mDisplayHeight; ++y)
    {
        for(x = 0; x < (int)pImgFrame->mDisplayWidth; x += 2)
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

        pSrcY += pImgFrame->mDisplayWidth;

        if(y & 1)
        {
            pSrcU += pImgFrame->mDisplayWidth / 2;
            pSrcV += pImgFrame->mDisplayWidth / 2;
        }

        pDst += pImgFrame->mDisplayWidth;
    }

    free(pClipTable);

#if SAVE_RGB
        FILE* outFp = fopen("/tmp/rgb.bmp", "wb");
        if(outFp != NULL)
        {
            TLOGD("************save_bmp_rgb565");
            save_bmp_rgb565(outFp, pImgFrame->mDisplayWidth, pImgFrame->mDisplayHeight, pImgFrame->mRGB565Data);
            fclose(outFp);
        }
#endif

    return 0;
}

static void setDecoderPara(JpegDecoderContext* p,VideoStreamInfo* vStreamInfo,VConfig* vConfig){
    vStreamInfo->eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
    vConfig->bDisable3D = 0;
    vConfig->bDispErrorFrame = 0;
    vConfig->bNoBFrames = 0;
    vConfig->bRotationEn = 0;
    vConfig->bScaleDownEn = p->mScaleDownEn;
    vConfig->nHorizonScaleDownRatio = p->mHorizonScaleDownRatio;
    vConfig->nVerticalScaleDownRatio = p->mVerticalScaleDownRatio;
    vConfig->eOutputPixelFormat = p->mDecodedPixFormat;
    vConfig->nDeInterlaceHoldingFrameBufferNum = 0;
    vConfig->nDisplayHoldingFrameBufferNum = 0;
    vConfig->nRotateHoldingFrameBufferNum = 0;
    vConfig->nDecodeSmoothFrameBufferNum = 0;
    vConfig->nVbvBufferSize = 2*1024*1024;
    vConfig->bThumbnailMode = 1;
    vConfig->memops = p->memops;
}

static long long GetNowMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static long long GetNowUs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

JpegDecoder* JpegDecoderCreate(){
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)malloc(sizeof(JpegDecoderContext));
    if(!p)
    {
        TLOGE("JpegDecoderContext malloc fail");
        return NULL;
    }
    LogTinaVersionInfo();
    memset(p, 0x00, sizeof(JpegDecoderContext));

    p->memops = MemAdapterGetOpsS();
    if(p->memops == NULL)
    {
        free(p);
        TLOGE("MemAdapterGetOpsS fail");
        return NULL;
    }
    CdcMemOpen(p->memops);
    TLOGD("AddVDPlugin first");
    AddVDPlugin();
    return (JpegDecoder*)p;
}
void  JpegDecoderDestory(JpegDecoder* v){
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    if(p->mVideoDecoder != NULL)
    {
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
    }
    if(p->mImgFrame.mYuvData)
    {
        free(p->mImgFrame.mYuvData);
        p->mImgFrame.mYuvData = NULL;
    }
    if(p->mImgFrame.mRGB565Data)
    {
        free(p->mImgFrame.mRGB565Data);
        p->mImgFrame.mRGB565Data = NULL;
    }
    memset(&p->mImgFrame, 0x00, sizeof(ImgFrame));
    if(p->memops){
        CdcMemClose(p->memops);
        p->memops = NULL;
    }
    free(p);
}
void JpegDecoderSetDataSource(JpegDecoder* v, const char* pUrl,JpegDecodeScaleDownRatio scaleRatio,JpegDecodeOutputDataType outputType){
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    p->mUrl = pUrl;
    p->mOutputDataType = outputType;
    switch (scaleRatio)
    {
        case JPEG_DECODE_SCALE_DOWN_1:
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
        case JPEG_DECODE_SCALE_DOWN_2:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 1;
            p->mVerticalScaleDownRatio = 1;
            break;
        case JPEG_DECODE_SCALE_DOWN_4:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 2;
            p->mVerticalScaleDownRatio = 2;
            break;
        case JPEG_DECODE_SCALE_DOWN_8:
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
    switch (outputType)
    {
        case JpegDecodeOutputDataNV21:
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
        case JpegDecodeOutputDataNV12:
            p->mDecodedPixFormat = PIXEL_FORMAT_NV12;
            break;
        case JpegDecodeOutputDataYU12:
            p->mDecodedPixFormat = PIXEL_FORMAT_YUV_PLANER_420;
            break;
        case JpegDecodeOutputDataYV12:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        case JpegDecodeOutputDataRGB565:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        default:
            TLOGW("the outputType = %d,which is not support,use the default:%d",outputType,JpegDecodeOutputDataNV21);
            p->mOutputDataType = JpegDecodeOutputDataNV21;
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
    }

}

void JpegDecoderSetDataSourceBuf(JpegDecoder* v, char* buffer,int bufLen,JpegDecodeScaleDownRatio scaleRatio,JpegDecodeOutputDataType outputType){
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    p->mSrcBuf = buffer;
    p->mSrcBufLen = bufLen;
    p->mOutputDataType = outputType;
    switch (scaleRatio)
    {
        case JPEG_DECODE_SCALE_DOWN_1:
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
        case JPEG_DECODE_SCALE_DOWN_2:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 1;
            p->mVerticalScaleDownRatio = 1;
            break;
        case JPEG_DECODE_SCALE_DOWN_4:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 2;
            p->mVerticalScaleDownRatio = 2;
            break;
        case JPEG_DECODE_SCALE_DOWN_8:
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
    switch (outputType)
    {
        case JpegDecodeOutputDataNV21:
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
        case JpegDecodeOutputDataNV12:
            p->mDecodedPixFormat = PIXEL_FORMAT_NV12;
            break;
        case JpegDecodeOutputDataYU12:
            p->mDecodedPixFormat = PIXEL_FORMAT_YUV_PLANER_420;
            break;
        case JpegDecodeOutputDataYV12:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        case JpegDecodeOutputDataRGB565:
            p->mDecodedPixFormat = PIXEL_FORMAT_YV12;
            break;
        default:
            TLOGW("the outputType = %d,which is not support,use the default:%d",outputType,JpegDecodeOutputDataNV21);
            p->mOutputDataType = JpegDecodeOutputDataNV21;
            p->mDecodedPixFormat = PIXEL_FORMAT_NV21;
            break;
    }
}

ImgFrame *JpegDecoderGetFrame(JpegDecoder* v){
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    VideoStreamInfo vStreamInfo;
    VConfig vConfig;
    char *jpegData = NULL;
    int dataLen =0;
    char *buf, *ringBuf;
    int buflen, ringBufLen;
    VideoPicture* videoPicture;

    /**read jpeg data from file**/
    if(p->mUrl){
        jpegData = readJpegData(p->mUrl, &dataLen);
        if (dataLen <= 0  || jpegData == NULL)
       {
            TLOGE("read file fail");
            return NULL;
        }
    }else if(p->mSrcBuf && p->mSrcBufLen){
        jpegData = p->mSrcBuf;
        dataLen = p->mSrcBufLen;
    }else{
        TLOGE("the input source has error");
        return NULL;
    }

    /**create the video decoder**/
    p->mVideoDecoder = CreateVideoDecoder();
    if(!p->mVideoDecoder){
        TLOGE("create video decoder failed\n");
        return NULL;
    }

    /**init the video decoder parameter**/
    memset(&vStreamInfo,0,sizeof(VideoStreamInfo));
    memset(&vConfig,0,sizeof(vConfig));
    setDecoderPara(p,&vStreamInfo,&vConfig);
    if ((InitializeVideoDecoder(p->mVideoDecoder, &vStreamInfo, &vConfig)) != 0)
    {
        TLOGE("InitializeVideoDecoder failed");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    /**request vbm buffer from video decoder**/
     if(RequestVideoStreamBuffer(p->mVideoDecoder,
                                dataLen,
                                (char**)&buf,
                                &buflen,
                                (char**)&ringBuf,
                                &ringBufLen,
                                0))
    {
        TLOGE("Request Video Stream Buffer failed");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    TLOGD("Request Video Stream Buffer ok");

    if(buflen + ringBufLen < dataLen)
    {
        TLOGE("#####Error: request buffer failed, buffer is not enough");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    /**copy and submit stream to video decoder SBM**/
    if(buflen >= dataLen)
    {
        memcpy(buf,jpegData,dataLen);
    }
    else
    {
        memcpy(buf,jpegData,buflen);
        memcpy(ringBuf,jpegData+buflen,dataLen-buflen);
    }
    TLOGD("Copy Video Stream Data ok!");

    VideoStreamDataInfo DataInfo;
    memset(&DataInfo, 0, sizeof(DataInfo));
    DataInfo.pData = buf;
    DataInfo.nLength = dataLen;
    DataInfo.bIsFirstPart = 1;
    DataInfo.bIsLastPart = 1;

    if (SubmitVideoStreamData(p->mVideoDecoder, &DataInfo, 0))
    {
        TLOGE("#####Error: Submit Video Stream Data failed!");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }
    TLOGD("Submit Video Stream Data ok!");

    /**decode stream**/
    int endofstream = 0;
    int dropBFrameifdelay = 0;
    int64_t currenttimeus = 0;
    int decodekeyframeonly = 0;
    long long startDecTime = GetNowUs();
    int ret = DecodeVideoStream(p->mVideoDecoder, endofstream, decodekeyframeonly,
                        dropBFrameifdelay, currenttimeus);
    long long finishDecTime = GetNowUs();
    printf("decode time = %lld us\n",finishDecTime-startDecTime);
    switch (ret)
    {
        case VDECODE_RESULT_KEYFRAME_DECODED:
        case VDECODE_RESULT_FRAME_DECODED:
        case VDECODE_RESULT_NO_FRAME_BUFFER:
        {
            ret = ValidPictureNum(p->mVideoDecoder, 0);
            if (ret>= 0)
            {
                videoPicture = RequestPicture(p->mVideoDecoder, 0);
                if (videoPicture == NULL){
                    TLOGE("RequestPicture fail");
                    DestroyVideoDecoder(p->mVideoDecoder);
                    p->mVideoDecoder = NULL;
                    return NULL;
                }
                TLOGD("decoder one pic...");
                TLOGD("pic nWidth is %d,nHeight is %d",videoPicture->nWidth,videoPicture->nHeight);
                TLOGD("videoPicture->nWidth = %d,videoPicture->nHeight = %d,videoPicture->nLineStride = %d",videoPicture->nWidth,videoPicture->nHeight,videoPicture->nLineStride);
                TLOGD("videoPicture->nTopOffset = %d,videoPicture->nLeftOffset = %d,videoPicture->nBottomOffset = %d,videoPicture->nRightOffset = %d",
                        videoPicture->nTopOffset,videoPicture->nLeftOffset,videoPicture->nBottomOffset,videoPicture->nRightOffset);
                if(p->mOutputDataType == JpegDecodeOutputDataNV21 || p->mOutputDataType == JpegDecodeOutputDataNV12
                    || p->mOutputDataType == JpegDecodeOutputDataYU12 || p->mOutputDataType == JpegDecodeOutputDataYV12){
                    if(p->mImgFrame.mYuvData != NULL){//if the mYuvData is not NULL,we should free the buffer which is malloc before
                        free(p->mImgFrame.mYuvData);
                        memset(&p->mImgFrame, 0x00, sizeof(ImgFrame));
                    }
                    p->mImgFrame.mWidth = videoPicture->nWidth;
                    p->mImgFrame.mHeight = videoPicture->nHeight;
                    p->mImgFrame.mDisplayWidth = videoPicture->nRightOffset-videoPicture->nLeftOffset;
                    p->mImgFrame.mDisplayHeight = videoPicture->nBottomOffset-videoPicture->nTopOffset;
                    p->mImgFrame.mYuvSize = p->mImgFrame.mDisplayWidth*p->mImgFrame.mDisplayHeight*3/2;
                    p->mImgFrame.mYuvData = (unsigned char*)malloc(p->mImgFrame.mYuvSize);
                    if(p->mImgFrame.mYuvData == NULL)
                    {
                        TLOGE("imgFrame.mYuvData malloc fail");
                        DestroyVideoDecoder(p->mVideoDecoder);
                        p->mVideoDecoder = NULL;
                        return NULL;
                    }
                    /* flush cache*/
                    if(p->mOutputDataType == JpegDecodeOutputDataYU12 || p->mOutputDataType == JpegDecodeOutputDataYV12){
                        CdcMemFlushCache(p->memops, videoPicture->pData0, videoPicture->nWidth*videoPicture->nHeight);
                        CdcMemFlushCache(p->memops, videoPicture->pData1, videoPicture->nHeight*videoPicture->nHeight/4);
                        CdcMemFlushCache(p->memops, videoPicture->pData2, videoPicture->nHeight*videoPicture->nHeight/4);
                        /**copy actural picture data to imgframe**/
                        memcpy(p->mImgFrame.mYuvData,videoPicture->pData0,p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight);
                        memcpy(p->mImgFrame.mYuvData+p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight,videoPicture->pData1,p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight/4);
                        memcpy(p->mImgFrame.mYuvData+p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight*5/4,videoPicture->pData2,p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight/4);
                    }else{
                        CdcMemFlushCache(p->memops, videoPicture->pData0, videoPicture->nWidth*videoPicture->nHeight);
                        CdcMemFlushCache(p->memops, videoPicture->pData1, videoPicture->nHeight*videoPicture->nHeight/2);
                        /**copy actural picture data to imgframe**/
                        memcpy(p->mImgFrame.mYuvData,videoPicture->pData0,p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight);
                        memcpy(p->mImgFrame.mYuvData+p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight,videoPicture->pData1,p->mImgFrame.mDisplayWidth *p->mImgFrame.mDisplayHeight/2);
                    }
#ifdef SAVE_YUV
                    saveYuvPic(p,&p->mImgFrame);
#endif
                }else if(p->mOutputDataType ==JpegDecodeOutputDataRGB565){
                    if(p->mImgFrame.mRGB565Data != NULL){//if the mRGB565Data is not NULL,we should free the buffer which is malloc before
                        free(p->mImgFrame.mRGB565Data);
                        memset(&p->mImgFrame, 0x00, sizeof(ImgFrame));
                    }
                    p->mImgFrame.mWidth = videoPicture->nWidth;
                    p->mImgFrame.mHeight = videoPicture->nHeight;
                    p->mImgFrame.mDisplayWidth = videoPicture->nRightOffset-videoPicture->nLeftOffset;
                    p->mImgFrame.mDisplayHeight = videoPicture->nBottomOffset-videoPicture->nTopOffset;
                    p->mImgFrame.mRGB565Size = p->mImgFrame.mDisplayWidth*p->mImgFrame.mDisplayHeight*2;
                    p->mImgFrame.mRGB565Data = (uint8_t*)malloc(p->mImgFrame.mRGB565Size);
                    if(p->mImgFrame.mRGB565Data == NULL)
                    {
                        TLOGE("imgFrame.mRGB565Data malloc fail");
                        DestroyVideoDecoder(p->mVideoDecoder);
                        p->mVideoDecoder = NULL;
                        return NULL;
                    }
                    transformYV12toRGB565(p->memops,videoPicture,&p->mImgFrame);
                }else{
                    TLOGE("fail:the outputType = %d,whic is not support",p->mOutputDataType);
                }

                /**return the picture buf to video decoder**/
                ReturnPicture(p->mVideoDecoder,videoPicture);

            }
            else
            {
               TLOGE("no ValidPictureNum ret is %d",ret);
            }
            break;
        }

        case VDECODE_RESULT_OK:
        case VDECODE_RESULT_CONTINUE:
        case VDECODE_RESULT_NO_BITSTREAM:
        case VDECODE_RESULT_RESOLUTION_CHANGE:
        case VDECODE_RESULT_UNSUPPORTED:
        default:
            TLOGE("video decode Error: %d ", ret);
            DestroyVideoDecoder(p->mVideoDecoder);
            p->mVideoDecoder = NULL;
            return NULL;
    }

    /**free the jpeg data**/
    if (jpegData != NULL)
    {
        TLOGD("free jpeg data");
        free(jpegData);
        jpegData = NULL;
    }

    return &p->mImgFrame;
}
