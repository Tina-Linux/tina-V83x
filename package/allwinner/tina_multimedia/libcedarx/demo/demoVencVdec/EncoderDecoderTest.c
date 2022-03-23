/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : EncoderDecoderTest.c
 * Description : EncoderDecoderTest
 * History :
 *
 */

#include <cdx_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include <sys/time.h>
#include <time.h>
#include <memoryAdapter.h>
#include "vdecoder.h"
#include <math.h>

//#define USE_SVC
//#define USE_VIDEO_SIGNAL
//#define USE_ASPECT_RATIO
//#define USE_SUPER_FRAME

#define RESULT_DECODE_FINISH (2)

typedef struct {
    char             intput_file_path[256];

    unsigned int  encode_frame_num;
    unsigned int  encode_format;

    unsigned int src_size;
    unsigned int dst_size;

    unsigned int src_width;
    unsigned int src_height;
    unsigned int dst_width;
    unsigned int dst_height;

    int bit_rate;
    int frame_rate;
    int maxKeyFrame;
}encode_param_t;

typedef enum {
    INPUT,
    HELP,
    ENCODE_FRAME_NUM,
    ENCODE_FORMAT,
    OUTPUT,
    SRC_SIZE,
    DST_SIZE,
    COMPARE_FILE,
    INVALID
}ARGUMENT_T;

typedef struct {
    char Short[8];
    char Name[128];
    ARGUMENT_T argument;
    char Description[512];
}argument_t;


typedef struct VencoderContext
{
    VencH264Param h264Param;
    VencBaseConfig baseConfig;
    VideoEncoder* pVideoEnc;
    unsigned char *uv_tmp_buffer;
    encode_param_t    encode_param;
    FILE *in_file;
    int  bEncoderFinishFlag;
    long long nTotalEncodeTime;

#ifdef USE_SUPER_FRAME
    VencSuperFrameConfig     sSuperFrameCfg;
#endif
}VencoderContext;

typedef struct VdecoderContext
{
    VideoDecoder* pVideoDec;
    VConfig       videoConfig;
    VideoStreamInfo videoStreamInfo;
    int nRequestPicCount;
    char  encode_file_path[256];
    FILE* encode_file;
    char* pEncoderPicData;
    unsigned int   nEncoderPicDataSize;
    int   nBitstreamTotalSize;
    double max_psnr;
    double min_psnr;
    double total_psnr;
    double avr_psnr;
    double num_psnr;
}VdecoderContext;


typedef struct MainContext
{
    VencoderContext vencoderCxt;
    VdecoderContext vdecoderCxt;
}MainContext;

static const argument_t ArgumentMapping[] =
{
    { "-h",  "--help",    HELP,
        "Print this help" },
    { "-i",  "--input",   INPUT,
        "Input file path" },
    { "-n",  "--encode_frame_num",   ENCODE_FRAME_NUM,
        "After encoder n frames, encoder stop" },
    { "-f",  "--encode_format",  ENCODE_FORMAT,
        "0:h264 encoder, 1:jpeg_encoder" },
    { "-o",  "--output",  OUTPUT,
        "output file path" },
    { "-s",  "--srcsize",  SRC_SIZE,
        "src_size,can be 1080,720,480" },
    { "-d",  "--dstsize",  DST_SIZE,
        "dst_size,can be 1080,720,480" },
    { "-c",  "--compare",  COMPARE_FILE,
        "compare file:reference file path" },
};

//extern int gettimeofday(struct timeval *tv, struct timezone *tz);
static long long GetNowUs()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

int yu12ToNv12(unsigned int width, unsigned int height, unsigned char *addr_uv,
          unsigned char *addr_tmp_uv)
{
    unsigned int i, chroma_bytes;
    unsigned char *u_addr = NULL;
    unsigned char *v_addr = NULL;
    unsigned char *tmp_addr = NULL;

    chroma_bytes = width*height/4;

    u_addr = addr_uv;
    v_addr = addr_uv + chroma_bytes;
    tmp_addr = addr_tmp_uv;

    for(i=0; i<chroma_bytes; i++)
    {
        *(tmp_addr++) = *(u_addr++);
        *(tmp_addr++) = *(v_addr++);
    }

    memcpy(addr_uv, addr_tmp_uv, chroma_bytes*2);

    return 0;
}

ARGUMENT_T GetArgument(char *name)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    while(i < num)
    {
        if((0 == strcmp(ArgumentMapping[i].Name, name)) ||
            ((0 == strcmp(ArgumentMapping[i].Short, name)) &&
             (0 != strcmp(ArgumentMapping[i].Short, "--"))))
        {
            return ArgumentMapping[i].argument;
        }
        i++;
    }
    return INVALID;
}

static void PrintDemoUsage(void)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    printf("Usage:");
    while(i < num)
    {
        printf("%s %-32s %s", ArgumentMapping[i].Short, ArgumentMapping[i].Name,
                ArgumentMapping[i].Description);
        printf("\n");
        i++;
    }
}

void ParseArgument(encode_param_t *encode_param, char *argument, char *value)
{
    ARGUMENT_T arg;

    arg = GetArgument(argument);

    switch(arg)
    {
        case HELP:
            PrintDemoUsage();
            exit(-1);
        case INPUT:
            memset(encode_param->intput_file_path, 0, sizeof(encode_param->intput_file_path));
            sscanf(value, "%255s", encode_param->intput_file_path);
            logd(" get input file: %s ", encode_param->intput_file_path);
            break;
        case ENCODE_FRAME_NUM:
            sscanf(value, "%u", &encode_param->encode_frame_num);
            break;
        case ENCODE_FORMAT:
            sscanf(value, "%u", &encode_param->encode_format);
            break;
        case OUTPUT:
            break;
        case SRC_SIZE:
            sscanf(value, "%u", &encode_param->src_size);
            logd(" get src_size: %dp ", encode_param->src_size);
            if(encode_param->src_size == 1080)
            {
                encode_param->src_width = 1920;
                encode_param->src_height = 1080;
            }
            else if(encode_param->src_size == 720)
            {
                encode_param->src_width = 1280;
                encode_param->src_height = 720;
            }
            else if(encode_param->src_size == 480)
            {
                encode_param->src_width = 640;
                encode_param->src_height = 480;
            }
            else
            {
                encode_param->src_width = 1280;
                encode_param->src_height = 720;
                logw("encoder demo only support the size 1080p,720p,480p, \
                 now use the default size 720p\n");
            }
            break;
        case DST_SIZE:
            sscanf(value, "%u", &encode_param->dst_size);
            logd(" get dst_size: %dp ", encode_param->dst_size);
            if(encode_param->dst_size == 1080)
            {
                encode_param->dst_width = 1920;
                encode_param->dst_height = 1080;
            }
            else if(encode_param->dst_size == 720)
            {
                encode_param->dst_width = 1280;
                encode_param->dst_height = 720;
            }
            else if(encode_param->dst_size == 480)
            {
                encode_param->dst_width = 640;
                encode_param->dst_height = 480;
            }
            else
            {
                encode_param->dst_width = 1280;
                encode_param->dst_height = 720;
                logw("encoder demo only support the size 1080p,720p,480p,\
                 now use the default size 720p\n");
            }
            break;
            case COMPARE_FILE:
            break;
        case INVALID:
        default:
            logd("unknowed argument :  %s", argument);
            break;
    }
}

void DemoHelpInfo(void)
{
    logd(" ==== CedarX2.0 encoder demo help start ===== ");
    logd(" -h or --help to show the demo usage");
    logd(" demo created by yangcaoyuan, allwinnertech/AL3 ");
    logd(" email: yangcaoyuan@allwinnertech.com ");
    logd(" ===== CedarX2.0 encoder demo help end ====== ");
}

int SeekPrefixNAL(char* begin)
{
    unsigned int i;
    char* pchar = begin;
    char TemporalID = 0;
    char isPrefixNAL = 1;
    char NAL[4] = {0x00, 0x00, 0x00, 0x01};
    char PrefixNAL[3] = {0x6e, 0x4e, 0x0e};

    if(!pchar)
    {
        return -1;
    }

    for(i=0; i<4; i++)
    {
        if(pchar[i] != NAL[i])
        {
            isPrefixNAL = 0;
            break;
        }
    }
    if(isPrefixNAL == 1)
    {
        isPrefixNAL = 0;
        for(i=0; i<3; i++)
        {
            if(pchar[4] == PrefixNAL[i])
            {
                isPrefixNAL = 1;
                break;
            }
        }
    }
    // read temporal_id
    if(isPrefixNAL == 1)
    {
        TemporalID = pchar[7];
        TemporalID >>= 5;
        return TemporalID;
    }

    return -1;
}

void InitJpegExif(EXIFInfo *exifinfo)
{
    exifinfo->ThumbWidth = 640;
    exifinfo->ThumbHeight = 480;

    strcpy((char*)exifinfo->CameraMake,        "allwinner make test");
    strcpy((char*)exifinfo->CameraModel,        "allwinner model test");
    strcpy((char*)exifinfo->DateTime,         "2014:02:21 10:54:05");
    strcpy((char*)exifinfo->gpsProcessingMethod,  "allwinner gps");

    exifinfo->Orientation = 0;

    exifinfo->ExposureTime.num = 2;
    exifinfo->ExposureTime.den = 1000;

    exifinfo->FNumber.num = 20;
    exifinfo->FNumber.den = 10;
    exifinfo->ISOSpeed = 50;

    exifinfo->ExposureBiasValue.num= -4;
    exifinfo->ExposureBiasValue.den= 1;

    exifinfo->MeteringMode = 1;
    exifinfo->FlashUsed = 0;

    exifinfo->FocalLength.num = 1400;
    exifinfo->FocalLength.den = 100;

    exifinfo->DigitalZoomRatio.num = 4;
    exifinfo->DigitalZoomRatio.den = 1;

    exifinfo->WhiteBalance = 1;
    exifinfo->ExposureMode = 1;

    exifinfo->enableGpsInfo = 1;

    exifinfo->gps_latitude = 23.2368;
    exifinfo->gps_longitude = 24.3244;
    exifinfo->gps_altitude = 1234.5;

    exifinfo->gps_timestamp = (long)time(NULL);

    strcpy((char*)exifinfo->CameraSerialNum,  "123456789");
    strcpy((char*)exifinfo->ImageName,  "exif-name-test");
    strcpy((char*)exifinfo->ImageDescription,  "exif-descriptor-test");
}

static double computePSNR(char* pEncoderData, char* pDecoderData, int nWidht, int nHeight)
{
    unsigned int y_size, u_size, v_size;
    unsigned char* y_enc_data;
    unsigned char* u_enc_data;
    unsigned char* v_enc_data;
    unsigned char* y_dec_data;
    unsigned char* u_dec_data;
    unsigned char* v_dec_data;
    unsigned int i;
    double mse = 0;
    double psnr = 0;
    double div = 0;

    y_size = nWidht*nHeight;
    u_size = nWidht*nHeight/4;
    v_size = nWidht*nHeight/4;

    logv("pEncoderData = %p, pDecoderData = %p, w = %d, h = %d",pEncoderData, pDecoderData,
          nWidht, nHeight);
    logv("y_size = %d, u_size = %d, v_size = %d",y_size, u_size, v_size);

    y_enc_data = (unsigned char*)pEncoderData;
    u_enc_data = (unsigned char*)(pEncoderData + y_size);
    v_enc_data = (unsigned char*)(pEncoderData + y_size + u_size);

    y_dec_data = (unsigned char*)pDecoderData;
    u_dec_data = (unsigned char*)(pDecoderData + y_size + v_size);
    v_dec_data = (unsigned char*)(pDecoderData + y_size);

    for(i=0; i< y_size; i++)
    {
        div = y_dec_data[i] - y_enc_data[i];
        logv(" y: div = %f, div*div = %f, i = %d",div, div*div,i );
        mse += div*div;
    }

    for(i=0; i<u_size; i++)
    {
        div = u_dec_data[i] - u_enc_data[i];
        logv(" u: div = %f, div*div = %f, i = %d",div, div*div,i );
        mse += div*div;
    }
    for(i=0; i<v_size; i++)
    {
        div = v_dec_data[i] - v_enc_data[i];
        logv(" v: div = %f, div*div = %f, i = %d",div, div*div,i );
        mse += div*div;
    }

    logv("*** mse_01 = %f",mse);
    mse = mse/(nWidht*nHeight);
    psnr = 10 * log10(255 * 255 / mse);

    logv("*** mse = %f, psnr = %f", mse, psnr);
    //double    mse = mse / (width*height);
    //double psnr = 10 * log10(255 * 255 / mse);
    return psnr;

}


static int initVencoder(VencoderContext* pVencoderCxt)
{
    logd("**** initVencoder ****");
    VencH264FixQP fixQP;
    VencCyclicIntraRefresh sIntraRefresh;
    EXIFInfo exifinfo;
    VencAllocateBufferParam bufferParam;
#ifdef USE_SVC
    VencH264SVCSkip         SVCSkip; // set SVC and skip_frame
#endif
#ifdef USE_ASPECT_RATIO
    VencH264AspectRatio        sAspectRatio;
#endif
#ifdef USE_VIDEO_SIGNAL
    VencH264VideoSignal        sVideoSignal;
#endif
    VencBaseConfig* pBaseConfig = &pVencoderCxt->baseConfig;
    encode_param_t* pEncode_param = &pVencoderCxt->encode_param;
    VencH264Param*  pH264Param = &pVencoderCxt->h264Param;

 //set roi
    VencROIConfig sRoiConfig[4];

    sRoiConfig[0].bEnable = 1;
    sRoiConfig[0].index = 0;
    sRoiConfig[0].nQPoffset = 10;
    sRoiConfig[0].sRect.nLeft = 320;
    sRoiConfig[0].sRect.nTop = 180;
    sRoiConfig[0].sRect.nWidth = 320;
    sRoiConfig[0].sRect.nHeight = 180;

    sRoiConfig[1].bEnable = 1;
    sRoiConfig[1].index = 1;
    sRoiConfig[1].nQPoffset = 10;
    sRoiConfig[1].sRect.nLeft = 320;
    sRoiConfig[1].sRect.nTop = 180;
    sRoiConfig[1].sRect.nWidth = 320;
    sRoiConfig[1].sRect.nHeight = 180;

    sRoiConfig[2].bEnable = 1;
    sRoiConfig[2].index = 2;
    sRoiConfig[2].nQPoffset = 10;
    sRoiConfig[2].sRect.nLeft = 320;
    sRoiConfig[2].sRect.nTop = 180;
    sRoiConfig[2].sRect.nWidth = 320;
    sRoiConfig[2].sRect.nHeight = 180;

    sRoiConfig[3].bEnable = 1;
    sRoiConfig[3].index = 3;
    sRoiConfig[3].nQPoffset = 10;
    sRoiConfig[3].sRect.nLeft = 320;
    sRoiConfig[3].sRect.nTop = 180;
    sRoiConfig[3].sRect.nWidth = 320;
    sRoiConfig[3].sRect.nHeight = 180;

    //intraRefresh
    sIntraRefresh.bEnable = 1;
    sIntraRefresh.nBlockNumber = 10;

    //fix qp mode
    fixQP.bEnable = 1;
    fixQP.nIQp = 23;
    fixQP.nPQp = 30;

    //* h264 param
    pH264Param->bEntropyCodingCABAC = 1;
    pH264Param->nBitrate = pEncode_param->bit_rate;
    pH264Param->nFramerate = pEncode_param->frame_rate;
    pH264Param->nCodingMode = VENC_FRAME_CODING;
    //pH264Param->nCodingMode = VENC_FIELD_CODING;

    pH264Param->nMaxKeyInterval = pEncode_param->maxKeyFrame;
    pH264Param->sProfileLevel.nProfile = VENC_H264ProfileMain;
    pH264Param->sProfileLevel.nLevel = VENC_H264Level31;
    pH264Param->sQPRange.nMinqp = 10;
    pH264Param->sQPRange.nMaxqp = 40;
    InitJpegExif(&exifinfo);

    pVencoderCxt->in_file = fopen(pEncode_param->intput_file_path, "r");

    if(pVencoderCxt->in_file == NULL)
    {
        loge("open in_file fail\n");
        return -1;
    }

    memset(pBaseConfig, 0 ,sizeof(VencBaseConfig));
    memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));

    pBaseConfig->memops = MemAdapterGetOpsS();
    if (pBaseConfig->memops == NULL)
    {
        printf("MemAdapterGetOpsS failed\n");
        return -1;
    }
    CdcMemOpen(pBaseConfig->memops);
    pBaseConfig->nInputWidth= pEncode_param->src_width;
    pBaseConfig->nInputHeight = pEncode_param->src_height;
    pBaseConfig->nStride = pEncode_param->src_width;

    pBaseConfig->nDstWidth = pEncode_param->dst_width;
    pBaseConfig->nDstHeight = pEncode_param->dst_height;
    //the format of yuv file is yuv420p,
    //but the old ic only support the yuv420sp,
    //so use the func yu12Tonv12() to config all the format.
    pBaseConfig->eInputFormat = VENC_PIXEL_YUV420SP;

    bufferParam.nSizeY = pBaseConfig->nInputWidth*pBaseConfig->nInputHeight;
    bufferParam.nSizeC = pBaseConfig->nInputWidth*pBaseConfig->nInputHeight/2;
    bufferParam.nBufferNum = 4;
    pVencoderCxt->pVideoEnc = VideoEncCreate(pEncode_param->encode_format);

    if(pEncode_param->encode_format == VENC_CODEC_JPEG)
    {
        int quality = 90;
        int jpeg_mode = 1;
        VencJpegVideoSignal vs;
        vs.src_colour_primaries = VENC_YCC;
        vs.dst_colour_primaries = VENC_BT601;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamJpegQuality, &quality);
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamJpegVideoSignal, &vs);

        if(1 == jpeg_mode)
        {
            int jpeg_biteRate = 12*1024*1024;
            int jpeg_frameRate = 30;
            VencBitRateRange bitRateRange;
            bitRateRange.bitRateMax = 12*1024*1024;
            bitRateRange.bitRateMin = 12*1024*1024;

            VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamBitrate, &jpeg_biteRate);
            VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamFramerate,
                                 &jpeg_frameRate);
            VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamSetBitRateRange,
                                 &bitRateRange);
        }
    }
    else if(pEncode_param->encode_format == VENC_CODEC_H264)
    {
        int value;

        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264Param, pH264Param);

#ifdef USE_SVC
        // Add for  Temporal SVC and Skip_Frame
        SVCSkip.nTemporalSVC = T_LAYER_4;
        switch(SVCSkip.nTemporalSVC)
        {
            case T_LAYER_4:
                SVCSkip.nSkipFrame = SKIP_8;
                break;
            case T_LAYER_3:
                SVCSkip.nSkipFrame = SKIP_4;
                break;
            case T_LAYER_2:
                SVCSkip.nSkipFrame = SKIP_2;
                break;
            default:
                SVCSkip.nSkipFrame = NO_SKIP;
                break;
        }

        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264SVCSkip, &SVCSkip);
#endif

        value = 0;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamIfilter, &value);

        value = 0; //degree
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamRotation, &value);

        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264FixQP, &fixQP);

        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264CyclicIntraRefresh,
        //                     &sIntraRefresh);

        //value = 720/4;
        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamSliceHeight, &value);

        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[0]);
        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[1]);
        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[2]);
        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamROIConfig, &sRoiConfig[3]);
        value = 0;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamSetPSkip, &value);
#ifdef USE_ASPECT_RATIO
        sAspectRatio.aspect_ratio_idc = 255;
        sAspectRatio.sar_width = 4;
        sAspectRatio.sar_height = 3;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264AspectRatio, &sAspectRatio);
#endif
        //value = 1;
        //VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264FastEnc, &value);

#ifdef USE_VIDEO_SIGNAL
        sVideoSignal.video_format = 5;
        sVideoSignal.src_colour_primaries = 0;
        sVideoSignal.dst_colour_primaries = 1;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264VideoSignal,
                             &sVideoSignal);
#endif

#ifdef USE_SUPER_FRAME
        VencSuperFrameConfig* pSuperFrameCfg = &pVencoderCxt->sSuperFrameCfg;
        //sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_REENCODE;
        pSuperFrameCfg->eSuperFrameMode = VENC_SUPERFRAME_NONE;
        pSuperFrameCfg->nMaxIFrameBits = 30000*8;
        pSuperFrameCfg->nMaxPFrameBits = 15000*8;
        VideoEncSetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamSuperFrameConfig,
                             pSuperFrameCfg);
#endif
    }

    VideoEncInit(pVencoderCxt->pVideoEnc, pBaseConfig);

    if(pEncode_param->encode_format == VENC_CODEC_H264)
    {
        unsigned int head_num = 0;
        VencHeaderData sps_pps_data;
        VideoEncGetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
        logd("sps_pps_data.nLength: %d", sps_pps_data.nLength);
        for(head_num=0; head_num<sps_pps_data.nLength; head_num++)
            logd("the sps_pps :%02x\n", *(sps_pps_data.pBuffer+head_num));
    }

    AllocInputBuffer(pVencoderCxt->pVideoEnc, &bufferParam);

    if(pBaseConfig->eInputFormat == VENC_PIXEL_YUV420SP)
    {
        int uv_tmp_buffer_size = pBaseConfig->nInputWidth*pBaseConfig->nInputHeight/2;
        pVencoderCxt->uv_tmp_buffer = (unsigned char*)malloc(uv_tmp_buffer_size);
        if(pVencoderCxt->uv_tmp_buffer == NULL)
        {
            loge("malloc uv_tmp_buffer fail\n");
            fclose(pVencoderCxt->in_file);
            return -1;
        }
    }

    logd("**** initVencoder ok****");
    return 0;
}

static int initVdecoder(MainContext* pMainCxt)
{
    VencoderContext* pVencoderCxt = &pMainCxt->vencoderCxt;
    VdecoderContext* pVdecoderCxt = &pMainCxt->vdecoderCxt;
    VideoStreamInfo* pVideoStreamInfo = &pVdecoderCxt->videoStreamInfo;
    VConfig*         pVConfig = &pVdecoderCxt->videoConfig;
    int nRet = -1;
    encode_param_t*  pEncode_param    = &pVencoderCxt->encode_param;

    pVdecoderCxt->encode_file = fopen(pVdecoderCxt->encode_file_path, "r");
    if(pVdecoderCxt->encode_file == NULL)
    {
        loge("*** open encode file failed");
        return -1;
    }

    pVdecoderCxt->nEncoderPicDataSize = pEncode_param->dst_width*pEncode_param->dst_height*3/2;
    pVdecoderCxt->pEncoderPicData = (char*)malloc(pVdecoderCxt->nEncoderPicDataSize);
    if(pVdecoderCxt->pEncoderPicData == NULL)
    {
        loge(" malloc for pEncoderPicData failed, size = %d", pVdecoderCxt->nEncoderPicDataSize);
        return -1;
    }

    memset(pVdecoderCxt->pEncoderPicData, 0, pVdecoderCxt->nEncoderPicDataSize);

    memset(pVideoStreamInfo, 0, sizeof(VideoStreamInfo));
    memset(pVConfig, 0, sizeof(VConfig));

    AddVDPlugin();
    pVdecoderCxt->pVideoDec = CreateVideoDecoder();
    if(pVdecoderCxt->pVideoDec == NULL)
    {
        loge(" create video decoder failed");
        return -1;
    }

    if(pEncode_param->encode_format == VENC_CODEC_H264)
        pVideoStreamInfo->eCodecFormat = VIDEO_CODEC_FORMAT_H264;
    else if(pEncode_param->encode_format == VENC_CODEC_JPEG)
        pVideoStreamInfo->eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
    else if(pEncode_param->encode_format == VENC_CODEC_VP8)
        pVideoStreamInfo->eCodecFormat = VIDEO_CODEC_FORMAT_VP8;
    else
    {
        loge(" vdecoder not support the codec format = %d",
             pVencoderCxt->encode_param.encode_format);
        return -1;
    }

    pVideoStreamInfo->nWidth  = pEncode_param->dst_width;
    pVideoStreamInfo->nHeight = pEncode_param->dst_height;
    pVideoStreamInfo->nFrameRate = pEncode_param->frame_rate;

    if(pEncode_param->encode_format == VENC_CODEC_H264)
    {
        unsigned int head_num = 0;
        VencHeaderData sps_pps_data;
        VideoEncGetParameter(pVencoderCxt->pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
        logd("init decoder -- sps_pps_data.nLength: %d", sps_pps_data.nLength);
        for(head_num=0; head_num<sps_pps_data.nLength; head_num++)
        logd("init decoder -- the sps_pps :%02x\n", *(sps_pps_data.pBuffer+head_num));

        if(pVideoStreamInfo->pCodecSpecificData)
            free(pVideoStreamInfo->pCodecSpecificData);

        pVideoStreamInfo->pCodecSpecificData = (char*)malloc(sps_pps_data.nLength);
        if(pVideoStreamInfo->pCodecSpecificData == NULL)
        {
            loge("malloc for codec specific data failed");
            return -1;
        }
        memcpy(pVideoStreamInfo->pCodecSpecificData,sps_pps_data.pBuffer, sps_pps_data.nLength);
        pVideoStreamInfo->nCodecSpecificDataLen = sps_pps_data.nLength;
    }
    pVConfig->bGpuBufValid = 0;
    pVConfig->nAlignStride = 16;
    pVConfig->nDecodeSmoothFrameBufferNum = 3;
    pVConfig->eOutputPixelFormat = PIXEL_FORMAT_YV12;

    pVConfig->memops = MemAdapterGetOpsS();
    if (pVConfig->memops == NULL)
    {
        loge("MemAdapterGetOpsS failed\n");
        return -1;
    }
    CdcMemOpen(pVConfig->memops);

    nRet = InitializeVideoDecoder(pVdecoderCxt->pVideoDec, pVideoStreamInfo, pVConfig);
    if(nRet < 0)
    {
        loge(" init video decoder failed");
        return -1;
    }
    return 0;
}

static int doVdecoder(MainContext* pMainCxt)
{
    int nRet = -1;
    VencoderContext* pVencoderCxt = &pMainCxt->vencoderCxt;
    VdecoderContext* pVdecoderCxt = &pMainCxt->vdecoderCxt;
    VideoEncoder* pVideoEnc = pVencoderCxt->pVideoEnc;
    VideoDecoder* pVideoDec = pVdecoderCxt->pVideoDec;
    VencOutputBuffer outputBuffer;
    VideoPicture* pVideoPic = NULL;
    int bVideoStreamEosFlag = 0;
    int   nStreamBufIndex = 0;
    int bEvenHadBistreamFlag = 1;

    //1, get bit stream frome encoder
    nRet = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
    logv("** get bit stream, ret = %d",nRet);
    if(nRet == -1)
    {
        bEvenHadBistreamFlag = 0;
        if(pVencoderCxt->bEncoderFinishFlag == 1)
        {
            logw("setup bVideoStreamEosFlag to 1");
            bVideoStreamEosFlag = 1;
        }
        usleep(20*1000);
        logv("*** GetOneBitstreamFrame failed ***");
        goto decode_video_stream;
    }
    //2, reqeust buffer from decoder

    int nRequireSize = outputBuffer.nSize0 + outputBuffer.nSize1;
    char* pBuf = NULL;
    char* pRingBuf = NULL;
    int   nBufSize = 0;
    int   nRingBufSize = 0;

    do
    {
        nRet = RequestVideoStreamBuffer(pVideoDec, nRequireSize, &pBuf, &nBufSize,
                                        &pRingBuf, &nRingBufSize, nStreamBufIndex);
        logv("reqeust stream buffer, ret = %d, nRequireSize = %d",nRet, nRequireSize);
        if(nRet < 0 || (nBufSize + nRingBufSize) < nRequireSize)
        {
            logw(" reqeust video stream buffer failed: nRet[%d], reqeustSize[%d], realSize[%d]",
                 nRet, nRequireSize, nBufSize + nRingBufSize);
        }
    }while(nRet < 0 || (nBufSize + nRingBufSize) < nRequireSize);

    //3, submit buffer
    if(outputBuffer.nSize1 == 0)
    {
        memcpy(pBuf, outputBuffer.pData0, nBufSize);
        if(pRingBuf)
        {
           memcpy(pRingBuf, outputBuffer.pData0 + nBufSize, nRingBufSize);
        }
    }
    else
    {
        char* tmpBuf = (char*)malloc(nRequireSize);
        if(tmpBuf == NULL)
        {
            loge("malloc for tmpBuf failed");
        }
        memcpy(tmpBuf, outputBuffer.pData0, outputBuffer.nSize0);
        memcpy(tmpBuf + outputBuffer.nSize0, outputBuffer.pData1, outputBuffer.nSize1);

        memcpy(pBuf, tmpBuf, nBufSize);
        if(pRingBuf)
        {
           memcpy(pRingBuf, tmpBuf + nBufSize, nRingBufSize);
        }
        free(tmpBuf);
    }

    VideoStreamDataInfo dataInfo;
    memset(&dataInfo, 0, sizeof(VideoStreamDataInfo));
    dataInfo.nPts = outputBuffer.nPts;
    dataInfo.pData = pBuf;
    dataInfo.nLength = nBufSize + nRingBufSize;
    dataInfo.nStreamIndex = 0;
    dataInfo.bIsFirstPart = 1;
    dataInfo.bIsLastPart  = 1;
    dataInfo.bValid       = 1;

    pVdecoderCxt->nBitstreamTotalSize += dataInfo.nLength;

    nRet = SubmitVideoStreamData(pVideoDec, &dataInfo, nStreamBufIndex);

    FreeOneBitStreamFrame(pVencoderCxt->pVideoEnc, &outputBuffer);

decode_video_stream:

    nRet = DecodeVideoStream(pVideoDec, bVideoStreamEosFlag, 0, 0, 0);
    logv(" decode nRet = %d, nBitstreamTotalSize = %d",nRet, pVdecoderCxt->nBitstreamTotalSize);
    //4, request picture an return

    pVideoPic =  RequestPicture(pVideoDec, nStreamBufIndex);
    logv(" get pic = %p",pVideoPic);
    if(pVideoPic)
    {
        pVdecoderCxt->nRequestPicCount++;

        unsigned int decPicSize = (unsigned int)pVideoPic->nWidth*pVideoPic->nHeight*3/2;
        if(decPicSize != pVdecoderCxt->nEncoderPicDataSize)
        {
            loge("picSize of encoder and decoder not match: %d, %d",
                decPicSize, pVdecoderCxt->nEncoderPicDataSize);
            abort();
        }
        unsigned int read_size = 0;
        read_size = fread(pVdecoderCxt->pEncoderPicData, 1,
                          decPicSize, pVdecoderCxt->encode_file);
        if(read_size != decPicSize)
        {
            loge("read size not right: %d, %d",read_size, decPicSize);
            abort();
        }

        CdcMemFlushCache(pVdecoderCxt->videoConfig.memops, pVideoPic->pData0, decPicSize);

        double psnr = computePSNR(pVdecoderCxt->pEncoderPicData, pVideoPic->pData0,
                    pVideoPic->nWidth, pVideoPic->nHeight);
        logv("** format = %d, psnr = %f, decCount = %d",pVencoderCxt->encode_param.encode_format,
              psnr, pVdecoderCxt->nRequestPicCount);
        pVdecoderCxt->num_psnr++;
        pVdecoderCxt->total_psnr += psnr;
        pVdecoderCxt->avr_psnr = pVdecoderCxt->total_psnr/pVdecoderCxt->num_psnr;

        if(psnr < pVdecoderCxt->min_psnr || pVdecoderCxt->min_psnr == 0)
            pVdecoderCxt->min_psnr = psnr;

        if(psnr > pVdecoderCxt->max_psnr || pVdecoderCxt->max_psnr == 0)
            pVdecoderCxt->max_psnr = psnr;

        //logd("*** had reqeust a picture ***");
        ReturnPicture(pVideoDec, pVideoPic);
        pVideoPic = NULL;
    }
    else if(nRet == VDECODE_RESULT_NO_BITSTREAM && bVideoStreamEosFlag == 1)
        return RESULT_DECODE_FINISH;

    return 0;
}


static void* encodeDecodeThread(void* param)
{
    MainContext* pMainCxt = (MainContext*)param;
    VencoderContext* pVencoderCxt = &pMainCxt->vencoderCxt;
    VdecoderContext* pVdecoderCxt = &pMainCxt->vdecoderCxt;
    encode_param_t*  pEncode_param = &pVencoderCxt->encode_param;
    VencBaseConfig*  pBaseConfig = &pVencoderCxt->baseConfig;
    VencInputBuffer  inputBuffer;
    VencOutputBuffer outputBuffer;
    int result = 0;

#ifdef USE_SVC
    // used for throw frame test with SVC
    int TemporalLayer = -1;
    char p9bytes[9] = {0};
#endif

    unsigned int nEncodeNum = 0;
    int i = 0;

    while(1)
    {
        if(pVencoderCxt->bEncoderFinishFlag == 0)
        {
            //* get in buffer and copy yuv data
            GetOneAllocInputBuffer(pVencoderCxt->pVideoEnc, &inputBuffer);
            unsigned int size1, size2;
            unsigned int copy_size;

            size1 = fread(inputBuffer.pAddrVirY, 1,
                    pBaseConfig->nInputWidth*pBaseConfig->nInputHeight, pVencoderCxt->in_file);
            size2 = fread(inputBuffer.pAddrVirC, 1,
                      pBaseConfig->nInputWidth*pBaseConfig->nInputHeight/2, pVencoderCxt->in_file);

            if((size1!= pBaseConfig->nInputWidth*pBaseConfig->nInputHeight)
             || (size2!= pBaseConfig->nInputWidth*pBaseConfig->nInputHeight/2))
            {
                pVencoderCxt->bEncoderFinishFlag = 1;
                continue;
            }

            if(pBaseConfig->eInputFormat == VENC_PIXEL_YUV420SP)
            {
                yu12ToNv12(pBaseConfig->nInputWidth, pBaseConfig->nInputHeight,
                     inputBuffer.pAddrVirC, pVencoderCxt->uv_tmp_buffer);
            }

            inputBuffer.bEnableCorp = 0;
            inputBuffer.sCropInfo.nLeft =  240;
            inputBuffer.sCropInfo.nTop  =  240;
            inputBuffer.sCropInfo.nWidth  =  240;
            inputBuffer.sCropInfo.nHeight =  240;

            FlushCacheAllocInputBuffer(pVencoderCxt->pVideoEnc, &inputBuffer);

            //* add in buffer to encoder and engine encoder
            AddOneInputBuffer(pVencoderCxt->pVideoEnc, &inputBuffer);
            long long time1 = GetNowUs();
            result =  VideoEncodeOneFrame(pVencoderCxt->pVideoEnc);
            logv("*** encode result = %d",result);
            long long time2 = GetNowUs();
            logv("encode frame %d use time is %lldus..\n",nEncodeNum,(time2-time1));
            pVencoderCxt->nTotalEncodeTime += time2-time1;

            AlreadyUsedInputBuffer(pVencoderCxt->pVideoEnc,&inputBuffer);
            ReturnOneAllocInputBuffer(pVencoderCxt->pVideoEnc, &inputBuffer);
             nEncodeNum++;
        }


        //* get the bit stream
        result = doVdecoder(pMainCxt);
        if(pVencoderCxt->bEncoderFinishFlag && result == RESULT_DECODE_FINISH)
            break;

    }


    double bitStreamSizeMB = ((double)pVdecoderCxt->nBitstreamTotalSize)/(1024*1024);

    double totalTime = ((double)nEncodeNum)/30;
    logd("*** Video Time Len = %f",totalTime);

    double bitRate = (((double)pVdecoderCxt->nBitstreamTotalSize)/totalTime)*8;
    double bitRateMB = bitRate/(1024*1024);


    pVencoderCxt->bEncoderFinishFlag = 1;

    logd("***finish nEncodeNum = %d, bitStreamSizeMB = %f, %d, bitRateMb = %f***",
         nEncodeNum, bitStreamSizeMB, pVdecoderCxt->nBitstreamTotalSize, bitRateMB);

    logd("***finish: avr_psnr = %f, max_psnr = %f, min_psnr = %f",
         pVdecoderCxt->avr_psnr, pVdecoderCxt->max_psnr, pVdecoderCxt->min_psnr);

    logd("the average encode time is %lldus...\n",pVencoderCxt->nTotalEncodeTime/nEncodeNum);

    pthread_exit(NULL);
    return NULL;

error_exit:
    pVencoderCxt->bEncoderFinishFlag = 1;
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char** argv)
{
    log_set_level(LOG_LEVEL_DEBUG);
    MainContext mainContext;
    pthread_t vencoder_thread;
    pthread_t vdecoder_thread;
    VencoderContext* pVencoderCxt = &mainContext.vencoderCxt;
    VdecoderContext* pVdecoderCxt = &mainContext.vdecoderCxt;
    int nRet = -1;
    memset(&mainContext, 0, sizeof(MainContext));

    int i = 0;

    //set the default encode param
    encode_param_t*   pEncode_param = &mainContext.vencoderCxt.encode_param;

    pEncode_param->src_width = 1280;
    pEncode_param->src_height = 720;
    pEncode_param->dst_width = 1280;
    pEncode_param->dst_height = 720;

    pEncode_param->bit_rate = 3*1024*1024;
    pEncode_param->frame_rate = 30;
    pEncode_param->maxKeyFrame = 1/*30*/;

    pEncode_param->encode_format = VENC_CODEC_H264;
    pEncode_param->encode_frame_num = 200;

    strcpy((char*)pEncode_param->intput_file_path,   "/data/camera/720p-30zhen_1.yuv");
    strcpy((char*)pVdecoderCxt->encode_file_path,    "/data/camera/720p-30zhen_2.yuv");
//strcpy((char*)pEncode_param->intput_file_path, "/data/camera/720p-1-isp_1.yuv");
//strcpy((char*)pVdecoderCxt->encode_file_path, "/data/camera/720p-1-isp_2.yuv");
//strcpy((char*)pEncode_param->intput_file_path,"/mnt/usbhost/Storage01/720p50_mobcal_ter_1.yuv");
//strcpy((char*)pVdecoderCxt->encode_file_path,"/mnt/usbhost/Storage01/720p50_mobcal_ter_2.yuv");

    //parse the config paramter
    if(argc >= 2)
    {
        for(i = 1; i < (int)argc; i += 2)
        {
            ParseArgument(pEncode_param, argv[i], argv[i + 1]);
        }
    }
    else
    {
        printf(" we need more arguments ");
        PrintDemoUsage();
        return 0;
    }

    nRet = initVencoder(&mainContext.vencoderCxt);
    if(nRet < 0)
    {
        loge("*** init vencoder failed ***");
        goto out;
    }

    nRet = initVdecoder(&mainContext);
    if(nRet < 0)
    {
        loge("*** init vdecoder failed ***");
        goto out;
    }
    pthread_create(&vencoder_thread, NULL, encodeDecodeThread, (void*)(&mainContext));

    pthread_join(vencoder_thread, (void**)&nRet);
out:

    if(pVdecoderCxt->encode_file)
        fclose(pVdecoderCxt->encode_file);

    if(pVdecoderCxt->pEncoderPicData)
        free(pVdecoderCxt->pEncoderPicData);

    if(pVdecoderCxt->pVideoDec)
    {
        DestroyVideoDecoder(pVdecoderCxt->pVideoDec);
    }
    pVdecoderCxt->pVideoDec = NULL;

    if(pVdecoderCxt->videoConfig.memops)
    {
        CdcMemClose(pVdecoderCxt->videoConfig.memops);
    }

    if(pVdecoderCxt->videoStreamInfo.pCodecSpecificData)
    {
        free(pVdecoderCxt->videoStreamInfo.pCodecSpecificData);
    }

    if(pVencoderCxt->pVideoEnc)
    {
        VideoEncDestroy(pVencoderCxt->pVideoEnc);
    }
    pVencoderCxt->pVideoEnc = NULL;

    if(pVencoderCxt->in_file)
        fclose(pVencoderCxt->in_file);
    if(pVencoderCxt->uv_tmp_buffer)
        free(pVencoderCxt->uv_tmp_buffer);
    if(pVencoderCxt->baseConfig.memops)
    {
        CdcMemClose(pVencoderCxt->baseConfig.memops);
    }

    return 0;
}
