/*************************************************************************
    > File Name: encodertest.c
    > Author:
    > Mail:
    > Created Time: 2017/10/14 14:51:17
 ************************************************************************/

#include <stdio.h>
#include <cdx_log.h>
#include <stdlib.h>
#include <string.h>
#include "vencoder.h"
#include <sys/time.h>
#include <time.h>
#include <memoryAdapter.h>

#define ALIGN_16B(x) (((x) + (15)) & ~(15))

typedef struct {
    char        intput_file[256];
    char        output_file[256];

    VENC_PIXEL_FMT input_format;
    unsigned int encode_frame_num;
    unsigned int encode_format;

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

static long long GetNowUs()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

long long time1=0;
long long time2=0;
long long time3=0;

void show_help(void)
{
    printf("*******************************************************************************\n");
    printf(" please enter following instructoins!\n");
    printf(" encodertest argv[1] argv[2] argv[3] argv[4] argv[5] argv[6] argv[7] argv[8] argv[9]\n");
    printf(" argv[1]: encode format---0:H264 1:JPEG\n");
    printf(" argv[2]: input format---YV12 YU12 NV12 NV21 YUV420 YUYV\n");
    printf(" argv[3]: input data width\n");
    printf(" argv[4]: input data height\n");
    printf(" argv[5]: output data width\n");
    printf(" argv[6]: output data height\n");
    printf(" argv[7]: encode frame number\n");
    printf(" argv[8]: input data path\n");
    printf(" argv[9]: out data path\n");
    printf("*******************************************************************************\n");
    printf(" for instance : encodertest 0 NV21 640 480 1280 720 10 /tmp/source_data.yuv /tmp\n");
    printf("*******************************************************************************\n");
}

int return_format(char *short_name)
{
    if (strcmp(short_name, "YU12") == 0) {
        return VENC_PIXEL_YUV420P;
    } else if (strcmp(short_name, "YV12") == 0) {
        return VENC_PIXEL_YVU420P;
    } else if (strcmp(short_name, "YUV420") == 0) {
        return VENC_PIXEL_YUV420P;
    } else if (strcmp(short_name, "NV12") == 0) {
        return VENC_PIXEL_YUV420SP;
    } else if (strcmp(short_name, "NV21") == 0) {
        return VENC_PIXEL_YVU420SP;
    } else if (strcmp(short_name, "YUYV") == 0) {
        return VENC_PIXEL_YUYV422;
    } else {
        return -1;
    }
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

int main(int argc, char **argv)
{
    VencBaseConfig baseConfig;
    VencAllocateBufferParam bufferParam;
    VideoEncoder *pVideoEnc = NULL;
    VencHeaderData sps_pps_data;
    VencH264Param h264Param;
    VencInputBuffer inputBuffer;
    VencOutputBuffer outputBuffer;
    EXIFInfo exifinfo;

    int result = 0;
    FILE *in_file = NULL;
    FILE *out_file = NULL;

    char *input_path = NULL;
    char *output_path = NULL;

    /* set the default encode param */
    encode_param_t encode_param;
    memset(&encode_param, 0, sizeof(encode_param));

    encode_param.bit_rate = 2*1024*1024;
    encode_param.frame_rate = 30;
    encode_param.maxKeyFrame = 30;

    /* user setting */
    if(argc != 10){
        show_help();
        return -1;
    }

    if(atoi(argv[1]) == 0)
        encode_param.encode_format = VENC_CODEC_H264;
    else
        encode_param.encode_format = VENC_CODEC_JPEG;

    encode_param.input_format = return_format(argv[2]);
    if(encode_param.input_format == -1){
        printf("*************************************************\n");
        printf(" Does not support %s\n", argv[2]);
        printf("*************************************************\n");
        return -1;
    }

    encode_param.src_width = atoi(argv[3]);
    encode_param.src_height = atoi(argv[4]);
    encode_param.dst_width = atoi(argv[5]);
    encode_param.dst_height = atoi(argv[6]);

    encode_param.encode_frame_num = atoi(argv[7]);

    strcpy((char*)encode_param.intput_file, argv[8]);
    if(encode_param.encode_format == VENC_CODEC_H264)
        sprintf((char *)encode_param.output_file, "%s/%u_%u.264", argv[9], encode_param.dst_width, encode_param.dst_height);
    else
        strcpy((char *)encode_param.output_file, argv[9]);

    /* h264 param */
    memset(&h264Param, 0, sizeof(h264Param));
    h264Param.bEntropyCodingCABAC = 1;
    h264Param.nBitrate = encode_param.bit_rate;
    h264Param.nFramerate = encode_param.frame_rate;
    h264Param.nCodingMode = VENC_FRAME_CODING;

    h264Param.nMaxKeyInterval = encode_param.maxKeyFrame;
    h264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
    h264Param.sProfileLevel.nLevel = VENC_H264Level31;
    h264Param.sQPRange.nMinqp = 10;
    h264Param.sQPRange.nMaxqp = 40;

    InitJpegExif(&exifinfo);

    input_path = encode_param.intput_file;
    output_path = encode_param.output_file;

    in_file = fopen(input_path, "r");
    if(in_file == NULL){
        printf(" open %s fail\n", input_path);
        return -1;
    }

    /* if encode format is h264,outputfile is the same file */
    if(encode_param.encode_format == VENC_CODEC_H264){
        out_file = fopen(output_path, "wb");
        if(out_file == NULL){
            printf(" open %s fail\n", output_path);
            fclose(in_file);
            return -1;
        }
    }

    /* create encoder */
    pVideoEnc = VideoEncCreate(encode_param.encode_format);
    if(!pVideoEnc){
        printf(" create encoder failed\n");
	fclose(in_file);
        if(out_file){
            fclose(out_file);
            out_file = NULL;
        }
        return -1;
    }

    /* set param */
    int vbvSize = 2*1024*1024;
    VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetVbvSize, &vbvSize);
    if(encode_param.encode_format == VENC_CODEC_JPEG){
        int quality = 90;
        int jpeg_mode = 1;
        VencJpegVideoSignal vs;
        vs.src_colour_primaries = VENC_YCC;
        vs.dst_colour_primaries = VENC_BT601;
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegExifInfo, &exifinfo);
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegQuality, &quality);
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegEncMode, &jpeg_mode);
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamJpegVideoSignal, &vs);

        if(1 == jpeg_mode){
            int jpeg_biteRate = 12*1024*1024;
            int jpeg_frameRate = 30;
            VencBitRateRange bitRateRange;
            bitRateRange.bitRateMax = 14*1024*1024;
            bitRateRange.bitRateMin = 10*1024*1024;

            VideoEncSetParameter(pVideoEnc, VENC_IndexParamBitrate, &jpeg_biteRate);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamFramerate, &jpeg_frameRate);
            VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetBitRateRange, &bitRateRange);
        }
    }else if(encode_param.encode_format == VENC_CODEC_H264){
        int value;

        VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &h264Param);

        value = 0;
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamIfilter, &value);

        value = 0; //degree
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamRotation, &value);

        value = 0;
        VideoEncSetParameter(pVideoEnc, VENC_IndexParamSetPSkip, &value);
    }

    /* encoder init */
    memset(&baseConfig, 0, sizeof(VencBaseConfig));
    memset(&bufferParam, 0 ,sizeof(VencAllocateBufferParam));

    baseConfig.memops = MemAdapterGetOpsS();
    if(baseConfig.memops == NULL){
        printf(" MemAdapterGetOpsS failed\n");
        if(pVideoEnc)
            VideoEncDestroy(pVideoEnc);
        fclose(in_file);
        if(out_file){
            fclose(out_file);
            out_file = NULL;
        }
        return -1;
    }
    CdcMemOpen(baseConfig.memops);
    baseConfig.nInputWidth= encode_param.src_width;
    baseConfig.nInputHeight = encode_param.src_height;
    baseConfig.nStride = ALIGN_16B(encode_param.src_width);

    baseConfig.nDstWidth = encode_param.dst_width;
    baseConfig.nDstHeight = encode_param.dst_height;

    baseConfig.eInputFormat = encode_param.input_format;

	if(encode_param.input_format == VENC_PIXEL_YUYV422 || encode_param.input_format == VENC_PIXEL_YUV422SP){
        bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight*2;
        bufferParam.nBufferNum = 4;
    }else{
	    bufferParam.nSizeY = baseConfig.nInputWidth*baseConfig.nInputHeight;
	    bufferParam.nSizeC = baseConfig.nInputWidth*baseConfig.nInputHeight/2;
	    bufferParam.nBufferNum = 4;
    }

    if(VideoEncInit(pVideoEnc, &baseConfig) != 0){
        printf(" VideoEncInit failed\n");
        if(pVideoEnc)
            VideoEncDestroy(pVideoEnc);
        fclose(in_file);
        if(out_file){
            fclose(out_file);
            out_file = NULL;
        }
        return -1;
    }

    if(encode_param.encode_format == VENC_CODEC_H264){
        unsigned int head_num = 0;
        VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &sps_pps_data);
        fwrite(sps_pps_data.pBuffer, 1, sps_pps_data.nLength, out_file);
        printf(" sps_pps_data.nLength: %d\n", sps_pps_data.nLength);
        for(head_num=0; head_num<sps_pps_data.nLength; head_num++)
            printf(" the sps_pps :%02x\n", *(sps_pps_data.pBuffer+head_num));
    }

    /* alloc input buffer */
    AllocInputBuffer(pVideoEnc, &bufferParam);

    unsigned int testNumber = 0;

    /* encode */
    while(testNumber < encode_param.encode_frame_num)
    {
        /* get one input buffer */
        GetOneAllocInputBuffer(pVideoEnc, &inputBuffer);
        {
            if(encode_param.input_format == VENC_PIXEL_YUYV422 || encode_param.input_format == VENC_PIXEL_YUV422SP){
                unsigned int sizeY;

                sizeY = fread(inputBuffer.pAddrVirY, 1,
                                    baseConfig.nInputWidth*baseConfig.nInputHeight*2, in_file);

                if(sizeY != baseConfig.nInputWidth*baseConfig.nInputHeight*2){
                    fseek(in_file, 0L, SEEK_SET);
                    sizeY = fread(inputBuffer.pAddrVirY, 1,
                    baseConfig.nInputWidth*baseConfig.nInputHeight*2, in_file);
                }
            }else{
                unsigned int sizeY,sizeC;

	            sizeY = fread(inputBuffer.pAddrVirY, 1,
	                                baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
	            sizeC = fread(inputBuffer.pAddrVirC, 1,
	                                baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);

	            if((sizeY != baseConfig.nInputWidth*baseConfig.nInputHeight)
	                || (sizeC != baseConfig.nInputWidth*baseConfig.nInputHeight/2)){
	                fseek(in_file, 0L, SEEK_SET);
	                sizeY = fread(inputBuffer.pAddrVirY, 1,
	                                baseConfig.nInputWidth*baseConfig.nInputHeight, in_file);
	                sizeC = fread(inputBuffer.pAddrVirC, 1,
	                                baseConfig.nInputWidth*baseConfig.nInputHeight/2, in_file);
	            }
            }
        }

        inputBuffer.bEnableCorp = 0;
        inputBuffer.sCropInfo.nLeft =  240;
        inputBuffer.sCropInfo.nTop =  240;
        inputBuffer.sCropInfo.nWidth =  240;
        inputBuffer.sCropInfo.nHeight =  240;

        FlushCacheAllocInputBuffer(pVideoEnc, &inputBuffer);

        /* encode */
        AddOneInputBuffer(pVideoEnc, &inputBuffer);
        time1 = GetNowUs();
        VideoEncodeOneFrame(pVideoEnc);
        time2 = GetNowUs();
        printf(" encode frame %u use time is %lldus...\n",testNumber,(time2-time1));
        time3 += time2-time1;

        /* return inputbuffer to encoder */
        AlreadyUsedInputBuffer(pVideoEnc,&inputBuffer);
        ReturnOneAllocInputBuffer(pVideoEnc, &inputBuffer);

        /* get encode data */
        result = GetOneBitstreamFrame(pVideoEnc, &outputBuffer);
        if(result == -1){
            goto out;
        }

        /* H264 */
        if(encode_param.encode_format == VENC_CODEC_H264){
            /* write output data */
            fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

            if(outputBuffer.nSize1){
                fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);
            }
        }else{/* JPEG */
            char jpeg_path[256];
            sprintf(jpeg_path, "%s/%u_%u_%u.jpeg", encode_param.output_file, encode_param.dst_width,
                                                    encode_param.dst_height, testNumber);
            out_file = fopen(jpeg_path, "wb");
            if(out_file == NULL){
                printf(" open %s fail\n", jpeg_path);
                fclose(in_file);
                return -1;
            }

            /* write output data */
            fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

            if(outputBuffer.nSize1)
                fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);

            fclose(out_file);
            out_file = NULL;
        }

        /* return outbuffer */
        FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);

        if(h264Param.nCodingMode==VENC_FIELD_CODING && encode_param.encode_format==VENC_CODEC_H264)
        {
            GetOneBitstreamFrame(pVideoEnc, &outputBuffer);

            fwrite(outputBuffer.pData0, 1, outputBuffer.nSize0, out_file);

            if(outputBuffer.nSize1)
                fwrite(outputBuffer.pData1, 1, outputBuffer.nSize1, out_file);

            FreeOneBitStreamFrame(pVideoEnc, &outputBuffer);
        }

        testNumber++;
    }

    printf(" the average encode time is %lldus...\n",time3/testNumber);
out:
    /* free buffers */
    ReleaseAllocInputBuffer(pVideoEnc);

    if(pVideoEnc)
        VideoEncDestroy(pVideoEnc);

    pVideoEnc = NULL;
    printf(" output file is saved:%s\n",encode_param.output_file);

    if(out_file)
        fclose(out_file);
    if(in_file)
        fclose(in_file);
    if(baseConfig.memops)
        CdcMemClose(baseConfig.memops);

    return 0;
}
