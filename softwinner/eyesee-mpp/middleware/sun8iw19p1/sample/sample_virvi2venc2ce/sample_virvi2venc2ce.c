/******************************************************************************
  Copyright (C), 2001-2017, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : sample_virvi2venc2ce.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2017/1/5
  Last Modified :
  Description   : mpp component implement
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "sample_virvi2venc2ce"
#include <utils/plat_log.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/rsa.h>
#include <openssl/ossl_typ.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>



#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include "media/mpi_venc.h"
#include "media/mpi_sys.h"
#include "mm_common.h"
#include "mm_comm_venc.h"
#include "mm_comm_rc.h"
#include <mpi_videoformat_conversion.h>

#include <confparser.h>
#include "sample_virvi2venc2ce.h"
#include "sample_virvi2venc2ce_config.h"

//#define MAX_VIPP_DEV_NUM  2
//#define MAX_VIDEO_NUM         MAX_VIPP_DEV_NUM
//#define MAX_VIR_CHN_NUM   8

//VENC_CHN mVeChn;
//VI_DEV mViDev;
//VI_CHN mViChn;
//int AutoTestCount = 0,EncoderCount = 0;

//VI2Venc_Cap_S privCap[MAX_VIR_CHN_NUM][MAX_VIR_CHN_NUM];
//FILE* OutputFile_Fd;
#define AES_PER_BLK (1024)
#define AES128_KEY_LENGTH (16)
# define AES_KEY_SIZE_256	32

unsigned char g_key[AES_KEY_SIZE_256] = {
	0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
	0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};



SampleVirvi2VencConfparser *gpSampleVirvi2VencContext = NULL;

int hal_vipp_start(VI_DEV ViDev, VI_ATTR_S *pstAttr)
{
    AW_MPI_VI_CreateVipp(ViDev);
    AW_MPI_VI_SetVippAttr(ViDev, pstAttr);
    AW_MPI_VI_EnableVipp(ViDev);
    return 0;
}

int hal_vipp_end(VI_DEV ViDev)
{
    AW_MPI_VI_DisableVipp(ViDev);
    AW_MPI_VI_DestoryVipp(ViDev);
    return 0;
}

int hal_virvi_start(VI_DEV ViDev, VI_CHN ViCh, void *pAttr)
{
    int ret = -1;

    ret = AW_MPI_VI_CreateVirChn(ViDev, ViCh, pAttr);
    if(ret < 0)
    {
        aloge("Create VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    ret = AW_MPI_VI_SetVirChnAttr(ViDev, ViCh, pAttr);
    if(ret < 0)
    {
        aloge("Set VI ChnAttr failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    return 0;
}

int hal_virvi_end(VI_DEV ViDev, VI_CHN ViCh)
{
    int ret = -1;
#if 0
    /* better be invoked after AW_MPI_VENC_StopRecvPic */
    ret = AW_MPI_VI_DisableVirChn(ViDev, ViCh);
    if(ret < 0)
    {
        aloge("Disable VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
#endif
    ret = AW_MPI_VI_DestoryVirChn(ViDev, ViCh);
    if(ret < 0)
    {
        aloge("Destory VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    return 0;
}

static int ParseCmdLine(int argc, char **argv, SampleVirvi2VencCmdLineParam *pCmdLinePara)
{
    alogd("sample virvi2venc2ce path:[%s], arg number is [%d]", argv[0], argc);
    int ret = 0;
    int i=1;
    memset(pCmdLinePara, 0, sizeof(SampleVirvi2VencCmdLineParam));
    while(i < argc)
    {
        if(!strcmp(argv[i], "-path"))
        {
            if(++i >= argc)
            {
                aloge("fatal error! use -h to learn how to set parameter!!!");
                ret = -1;
                break;
            }
            if(strlen(argv[i]) >= MAX_FILE_PATH_SIZE)
            {
                aloge("fatal error! file path[%s] too long: [%d]>=[%d]!", argv[i], strlen(argv[i]), MAX_FILE_PATH_SIZE);
            }
            strncpy(pCmdLinePara->mConfigFilePath, argv[i], MAX_FILE_PATH_SIZE-1);
            pCmdLinePara->mConfigFilePath[MAX_FILE_PATH_SIZE-1] = '\0';
        }
        else if(!strcmp(argv[i], "-h"))
        {
            alogd("CmdLine param:\n"
                "\t-path /home/sample_virvi2venc2ce.conf\n");
            ret = 1;
            break;
        }
        else
        {
            alogd("ignore invalid CmdLine param:[%s], type -h to get how to set parameter!", argv[i]);
        }
        i++;
    }
    return ret;
}

static ERRORTYPE loadSampleVirvi2VencConfig(SampleVirvi2VencConfig *pConfig, const char *conf_path)
{
    int ret;
    char *ptr;
    CONFPARSER_S stConfParser;
    ret = createConfParser(conf_path, &stConfParser);
    if(ret < 0)
    {
        aloge("load conf fail");
        return FAILURE;
    }
    memset(pConfig, 0, sizeof(SampleVirvi2VencConfig));
    ptr = (char*)GetConfParaString(&stConfParser, SAMPLE_Virvi2Venc_Output_File_Path, NULL);
    strncpy(pConfig->OutputFilePath, ptr, MAX_FILE_PATH_SIZE-1);
    pConfig->OutputFilePath[MAX_FILE_PATH_SIZE-1] = '\0';
    pConfig->AutoTestCount = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Auto_Test_Count, 0);
    pConfig->EncoderCount = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Encoder_Count, 0);
    pConfig->DevNum = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Dev_Num, 0);
    pConfig->SrcFrameRate = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Src_Frame_Rate, 0);
    pConfig->SrcWidth = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Src_Width, 0);
    pConfig->SrcHeight = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Src_Height, 0);
    char *pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_Virvi2Venc_Dest_Format, NULL);
    if(!strcmp(pStrPixelFormat, "nv21"))
    {
        pConfig->DestPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    else
    {
        aloge("fatal error! conf file pic_format must be yuv420sp");
        pConfig->DestPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    char *EncoderType = (char*)GetConfParaString(&stConfParser, SAMPLE_Virvi2Venc_Dest_Encoder_Type, NULL);
    if(!strcmp(EncoderType, "H.264"))
    {
        pConfig->EncoderType = PT_H264;
    }
    else if(!strcmp(EncoderType, "H.265"))
    {
        pConfig->EncoderType = PT_H265;
    }
    else if(!strcmp(EncoderType, "MJPEG"))
    {
        pConfig->EncoderType = PT_MJPEG;
    }
    else
    {
        alogw("unsupported venc type:%p,encoder type turn to H.264!",EncoderType);
        pConfig->EncoderType = PT_H264;
    }
    pConfig->DestWidth = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Dest_Width, 0);
    pConfig->DestHeight = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Dest_Height, 0);
    pConfig->DestFrameRate = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Dest_Frame_Rate, 0);
    pConfig->DestBitRate = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Venc_Dest_Bit_Rate, 0);
    alogd("dev_num=%d, src_width=%d, src_height=%d, src_frame_rate=%d",
       pConfig->DevNum,pConfig->SrcWidth,pConfig->SrcHeight,pConfig->SrcFrameRate);
    alogd("dest_width=%d, dest_height=%d, dest_frame_rate=%d, dest_bit_rate=%d",
       pConfig->DestWidth,pConfig->DestHeight,pConfig->SrcFrameRate,pConfig->DestBitRate);
    destroyConfParser(&stConfParser);
    return SUCCESS;
}

//void generateKey() {
//    /* 生成公钥 */
//    RSA* rsa = RSA_generate_key(1024, 0x10001L, NULL, NULL);
//    BIO *bp = BIO_new(BIO_s_file());
//    BIO_write_filename(bp, "public.pem");
//    PEM_write_bio_RSAPublicKey(bp, rsa);
//    BIO_free_all(bp);
//    /* 生成私钥 */
//    bp = BIO_new_file("private.pem", "w+");
//    PEM_write_bio_RSAPrivateKey(bp, rsa, NULL, NULL, 4, NULL, NULL);
//    BIO_free_all(bp);
//    RSA_free(rsa);
//}

int openssl_encrypt(u8 *in, u8 *out, u8 *key, u32 key_len, u32 data_len)
{

    EVP_CIPHER_CTX *ctx;
	int outl=0,outltmp=0;
	int rest = data_len, per_len = 0;
	unsigned char *inp = in, *outp = out;
	int ret = 0;
    int block_size;
    ENGINE_load_builtin_engines();
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL){
        aloge("EVP_CIPHER_CTX_new: ctx == NULL ?");
        EVP_CIPHER_CTX_free(ctx);
    }

	EVP_CIPHER_CTX_init(ctx);
    
	if( key_len == AES128_KEY_LENGTH ){
		aloge("aes 128\n");
		ret = EVP_EncryptInit_ex(ctx,EVP_aes_128_cbc(),NULL,key,NULL);
		if( ret != 1 ){
			aloge(" Encryption init aes cbc 128 fail\n");
			return -1 ;
		}
	}else{
		ret = EVP_EncryptInit_ex(ctx,EVP_aes_128_cbc(),NULL,key,NULL);
		if( ret != 1 ){
			aloge(" Encryption init aes cbc 128 fail\n");
			return -1 ;
		}
	}

	EVP_CIPHER_CTX_set_padding(ctx, 1);

    block_size = EVP_CIPHER_CTX_block_size(ctx);
    ret = EVP_EncryptUpdate(ctx, outp, &outltmp, (const unsigned char *)inp, data_len);

    outl += outltmp;

	if(0 < EVP_EncryptFinal_ex(ctx, outp+outltmp,&outltmp)){
        outl += outltmp;

    }else{
        aloge("data not align!");
    }
    
    EVP_CIPHER_CTX_cleanup(ctx);

	aloge("enc ciphertext update length: %d, outltmp %d outl %d\n", data_len, outltmp, outl);
	return outl;
}

int openssl_decrypt(u8 *in, u8 *out, u8 *key, u32 key_len, u32 data_len)
{

	EVP_CIPHER_CTX *ctx;

	int outl=0,outltmp=0;;
	int ret = 0 ;
	int rest = data_len, per_len = 0;
	unsigned char *inp = in, *outp = out;
    int block_size;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL){
        aloge("EVP_CIPHER_CTX_new: ctx == NULL ?");
        EVP_CIPHER_CTX_free(ctx);
    }

	EVP_CIPHER_CTX_init(ctx);
	if( key_len == AES128_KEY_LENGTH ){
		aloge("aes 128\n");
		ret = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(),NULL,key,NULL);
		if( ret != 1 ){
			aloge(" Encryption init aes cbc 128 fail\n");
			return -1 ;
		}
	}else{
		ret = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(),NULL,key,NULL);
		if( ret != 1 ){
			aloge(" Encryption init aes cbc 128 fail\n");
			return -1 ;
		}
	}

	EVP_CIPHER_CTX_set_padding(ctx, 1);

    block_size = EVP_CIPHER_block_size(ctx);


	ret = EVP_DecryptUpdate(ctx, outp, &outltmp, (const unsigned char *)inp, data_len);
	if( ret != 1 ) {
			aloge(" Encryption update fail\n");
			return -1 ;
	} else {
            outl += outltmp;
	}
	

	if(0 < EVP_DecryptFinal_ex(ctx, outp +outl, &outltmp)){
        outl += outltmp;
    }else
        aloge("data not align!");
        
	EVP_CIPHER_CTX_cleanup(ctx);    
	aloge("dec ciphertext update length: %d, outltmp %d outl %d\n", data_len, outltmp, outl);
	return outl;
}

static void *GetEncoderFrameThread(void *pArg)
{
    int ret = 0;
    int count = 0;

    VI2Venc_Cap_S *pCap = (VI2Venc_Cap_S *)pArg;
    VI_DEV nViDev = pCap->Dev;
    VI_CHN nViChn = pCap->Chn;
    VENC_CHN nVencChn = pCap->mVencChn;
    VENC_STREAM_S VencFrame;
    VENC_PACK_S venc_pack;
    VencFrame.mPackCount = 1;
    VencFrame.mpPack = &venc_pack;
    alogd("Cap threadid=0x%lx, ViDev = %d, ViCh = %d\n", pCap->thid, nViDev, nViChn);

    if (nVencChn >= 0 && nViChn >= 0) 
    {
        MPP_CHN_S ViChn = {MOD_ID_VIU, nViDev, nViChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, nVencChn};
        ret = AW_MPI_SYS_Bind(&ViChn,&VeChn);
        if(ret !=SUCCESS)
        {
            alogd("error!!! vi can not bind venc!!!\n");
            return (void*)FAILURE;
        }
    }
    //alogd("start start recv success!\n");
    ret = AW_MPI_VI_EnableVirChn(nViDev, nViChn);
    if (ret != SUCCESS) 
    {
        alogd("VI enable error!");
        return (void*)FAILURE;
    }
    ret = AW_MPI_VENC_StartRecvPic(nVencChn);
    if (ret != SUCCESS) 
    {
        alogd("VENC Start RecvPic error!");
        return (void*)FAILURE;
    }
    int clen,plen,num;

    while(count != gpSampleVirvi2VencContext->mConfigPara.EncoderCount)
    {
        count++;
        if((ret = AW_MPI_VENC_GetStream(nVencChn, &VencFrame, 4000)) < 0) //6000(25fps) 4000(30fps)
        {
            alogd("get first frmae failed!\n");
            continue;
        }
        else
        {
            if(VencFrame.mpPack != NULL && VencFrame.mpPack->mLen0)
            {
//                unsigned char in[17] = "ahsdiuwhqhwdhadsh";
//                unsigned char enout[64+EVP_MAX_BLOCK_LENGTH];
//                unsigned char decout[32];
//                aloge("in:%s,src_len:%d",in,strlen(in));
//                int enc_len = openssl_encrypt(in, enout, g_key, AES_KEY_SIZE_256, 17);
//                aloge("enout:%s,enc_len:%d",enout,enc_len);
//                int dec_len = openssl_decrypt(enout, decout, g_key, AES_KEY_SIZE_256, enc_len);
//                decout[17] = '\0';
//                aloge("decout:%s,dec_len:%d",decout,dec_len);

                int venc_frame_len = VencFrame.mpPack->mLen0;
                unsigned char *encout = malloc(venc_frame_len + EVP_MAX_BLOCK_LENGTH);
                unsigned char *decout = malloc(venc_frame_len + EVP_MAX_BLOCK_LENGTH);                
                int encout_len = 0;
                int decout_len = 0;
                if (encout != NULL && decout != NULL) {
                    encout_len = openssl_encrypt(VencFrame.mpPack->mpAddr0, encout, g_key, AES_KEY_SIZE_256, venc_frame_len);
                    if(encout_len > 0){
                        decout_len = openssl_decrypt(encout, decout, g_key, AES_KEY_SIZE_256, encout_len);                    
                        if(decout_len <= 0){
                            aloge("openssl dec faile!!!");
                        }
                    }else{
                        aloge("openssl enc faile!!!");
                    }
     
                    if(decout_len == venc_frame_len){
                        aloge("openssl enc and dec test succese!");
                    }
                    free(encout);
                    free(decout);
                }
                //fwrite(VencFrame.mpPack->mpAddr0,1,VencFrame.mpPack->mLen0, gpSampleVirvi2VencContext->mOutputFileFp);
            }
            if(VencFrame.mpPack != NULL && VencFrame.mpPack->mLen1)
            {                
                int venc_frame_len = VencFrame.mpPack->mLen1;
                unsigned char *encout = malloc(venc_frame_len + EVP_MAX_BLOCK_LENGTH);
                unsigned char *decout = malloc(venc_frame_len + EVP_MAX_BLOCK_LENGTH);                
                int encout_len = 0;
                int decout_len = 0;
                if (encout != NULL && decout != NULL) {
                    encout_len = openssl_encrypt(VencFrame.mpPack->mpAddr1, encout, g_key, AES_KEY_SIZE_256, venc_frame_len);
                    if(encout_len > 0){
                        decout_len = openssl_decrypt(encout, decout, g_key, AES_KEY_SIZE_256, encout_len);                    
                        if(decout_len <= 0){
                            aloge("openssl dec faile!!!");
                        }
                    }else{
                        aloge("openssl enc faile!!!");
                    }
     
                    if(decout_len == venc_frame_len){
                        aloge("openssl enc and dec test succese!");
                    }
                    free(encout);
                    free(decout);
                }
                //fwrite(VencFrame.mpPack->mpAddr1,1,VencFrame.mpPack->mLen1, gpSampleVirvi2VencContext->mOutputFileFp);
            }
            ret = AW_MPI_VENC_ReleaseStream(nVencChn, &VencFrame);
            if(ret < 0)
            {
                alogd("falied error,release failed!!!\n");
            }
         }
    }
    return NULL;
}

void Virvi2Venc_HELP()
{
    alogd("Run CSI0/CSI1+Venc command: ./sample_virvi2venc2ce -path ./sample_virvi2venc2ce.conf\r\n");
}

int main(int argc, char *argv[])
{
    int ret, count = 0,result = 0;
    //int vipp_dev;
    int virvi_chn;
    //int isp_dev;

    printf("sample_virvi2venc2ce buile time = %s, %s.\r\n", __DATE__, __TIME__);
    if (argc != 3)
    {
        Virvi2Venc_HELP();
        exit(0);
    }
    SampleVirvi2VencConfparser *pContext = (SampleVirvi2VencConfparser*)malloc(sizeof(SampleVirvi2VencConfparser));
    gpSampleVirvi2VencContext = pContext;
    memset(pContext, 0, sizeof(SampleVirvi2VencConfparser));
    /* parse command line param,read sample_virvi2venc2ce.conf */
    if(ParseCmdLine(argc, argv, &pContext->mCmdLinePara) != 0)
    {
        aloge("fatal error! command line param is wrong, exit!");
        result = -1;
        goto _exit;
    }
    char *pConfigFilePath;
    if(strlen(pContext->mCmdLinePara.mConfigFilePath) > 0)
    {
        pConfigFilePath = pContext->mCmdLinePara.mConfigFilePath;
    }
    else
    {
        pConfigFilePath = DEFAULT_SAMPLE_VIPP2VENC_CONF_PATH;
    }
    /* parse config file. */
    if(loadSampleVirvi2VencConfig(&pContext->mConfigPara, pConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }
    
    while(count != pContext->mConfigPara.AutoTestCount)
    {
        alogd("======================================.\r\n");
        alogd("Auto Test count start: %d. (MaxCount==1000).\r\n", count);
        system("cat /proc/meminfo | grep Committed_AS");
        alogd("======================================.\r\n");    
        MPP_SYS_CONF_S mSysConf;
        memset(&mSysConf, 0, sizeof(MPP_SYS_CONF_S));
        mSysConf.nAlignWidth = 32;
        AW_MPI_SYS_SetConf(&mSysConf);
        ret = AW_MPI_SYS_Init();
        if (ret < 0)
        {
            aloge("sys Init failed!");
            return -1;
        }
        pContext->mViDev = pContext->mConfigPara.DevNum;
        /* dev:0, chn:0,1,2,3,4...16 */
        /* dev:1, chn:0,1,2,3,4...16 */
        /* dev:2, chn:0,1,2,3,4...16 */
        /* dev:3, chn:0,1,2,3,4...16 */
        /*Set VI Channel Attribute*/
        memset(&pContext->mViAttr, 0, sizeof(VI_ATTR_S));
        pContext->mViAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        pContext->mViAttr.memtype = V4L2_MEMORY_MMAP;
        pContext->mViAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pContext->mConfigPara.DestPicFormat);
        pContext->mViAttr.format.field = V4L2_FIELD_NONE;
        pContext->mViAttr.format.width = pContext->mConfigPara.SrcWidth;
        pContext->mViAttr.format.height = pContext->mConfigPara.SrcHeight;
        pContext->mViAttr.fps = pContext->mConfigPara.SrcFrameRate;
        /* update configuration anyway, do not use current configuration */
        pContext->mViAttr.use_current_win = 0;
        pContext->mViAttr.nbufs = 5;
        pContext->mViAttr.nplanes = 2;

        /* MPP components */
        pContext->mVeChn = 0;

        /* venc chn attr */
        memset(&pContext->mVEncChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
        //SIZE_S wantedVideoSize = {pContext->mConfigPara.DestWidth, pContext->mConfigPara.DestHeight};
        //int wantedFrameRate = pContext->mConfigPara.DestFrameRate;
        pContext->mVEncChnAttr.VeAttr.Type = pContext->mConfigPara.EncoderType;
        pContext->mVEncChnAttr.VeAttr.MaxKeyInterval = pContext->mConfigPara.DestFrameRate;
        pContext->mVEncChnAttr.VeAttr.SrcPicWidth = pContext->mConfigPara.SrcWidth;
        pContext->mVEncChnAttr.VeAttr.SrcPicHeight = pContext->mConfigPara.SrcHeight;
        pContext->mVEncChnAttr.VeAttr.Field = VIDEO_FIELD_FRAME;
        pContext->mVEncChnAttr.VeAttr.PixelFormat = pContext->mConfigPara.DestPicFormat;
        //int wantedVideoBitRate = pContext->mConfigPara.DestBitRate;
        if(PT_H264 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            pContext->mVEncChnAttr.VeAttr.AttrH264e.Profile = 1;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.PicWidth = pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.PicHeight = pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mBitRate = pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mMinQp = 1;
        }
        else if(PT_H265 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mProfile = 0;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mPicWidth = pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mPicHeight = pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mbPIntraEnable = TRUE;
            pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pContext->mConfigPara.DestBitRate;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mMinQp = 1;
        }
        else if(PT_MJPEG == pContext->mVEncChnAttr.VeAttr.Type)
        {
            pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
            pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mPicWidth= pContext->mConfigPara.DestWidth;
            pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mPicHeight = pContext->mConfigPara.DestHeight;
            pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pContext->mVEncChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pContext->mConfigPara.DestBitRate;
        }
        pContext->mVencFrameRateConfig.SrcFrmRate = pContext->mConfigPara.SrcFrameRate;
        pContext->mVencFrameRateConfig.DstFrmRate = pContext->mConfigPara.DestFrameRate;
#if 0
        /* has invoked in AW_MPI_SYS_Init() */
        result = VENC_Construct();
        if (result != SUCCESS)
        {
            alogd("VENC Construct error!");
            result = -1;
            goto _exit;
        }
#endif
        hal_vipp_start(pContext->mViDev, &pContext->mViAttr);
        //AW_MPI_ISP_Init();
        AW_MPI_ISP_Run(pContext->mIspDev);
        // for (virvi_chn = 0; virvi_chn < MAX_VIR_CHN_NUM; virvi_chn++)
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            memset(&pContext->privCap[pContext->mViDev][virvi_chn], 0, sizeof(VI2Venc_Cap_S));
            pContext->privCap[pContext->mViDev][virvi_chn].Dev = pContext->mViDev;
            pContext->privCap[pContext->mViDev][virvi_chn].Chn = virvi_chn;
            pContext->privCap[pContext->mViDev][virvi_chn].s32MilliSec = 5000;  // 2000;
            pContext->privCap[pContext->mViDev][virvi_chn].EncoderType = pContext->mVEncChnAttr.VeAttr.Type;
            if (0 == virvi_chn) /* H264, H265, MJPG, Preview(LCD or HDMI), VDA, ISE, AIE, CVBS */
            {
                /* open isp */
                if (pContext->mViDev == 0 || pContext->mViDev == 1) 
                {
                    pContext->mIspDev = 0;
                } 
                else if (pContext->mViDev == 2 || pContext->mViDev == 3) 
                {
                    pContext->mIspDev = 1;
                }

                result = hal_virvi_start(pContext->mViDev, virvi_chn, NULL);
                if(result < 0)
                {
                    alogd("VI start failed!\n");
                    result = -1;
                    goto _exit;
                }
                pContext->privCap[pContext->mViDev][virvi_chn].thid = 0;
                result = AW_MPI_VENC_CreateChn(pContext->mVeChn, &pContext->mVEncChnAttr);
                if(result < 0)
                {
                    alogd("create venc channel[%d] falied!\n", pContext->mVeChn);
                    result = -1;
                    goto _exit;
                }
                pContext->privCap[pContext->mViDev][virvi_chn].mVencChn = pContext->mVeChn;
                AW_MPI_VENC_SetFrameRate(pContext->mVeChn, &pContext->mVencFrameRateConfig);
                VencHeaderData vencheader;
                //open output file
                pContext->mOutputFileFp = fopen(pContext->mConfigPara.OutputFilePath, "wb+");
                if(!pContext->mOutputFileFp)
                {
                    aloge("fatal error! can't open file[%s]", pContext->mConfigPara.OutputFilePath);
                    result = -1;
                    //goto _exit;
                }
                if(PT_H264 == pContext->mVEncChnAttr.VeAttr.Type)
                {
                    AW_MPI_VENC_GetH264SpsPpsInfo(pContext->mVeChn, &vencheader);
                    if(vencheader.nLength)
                    {
                        fwrite(vencheader.pBuffer,vencheader.nLength,1,pContext->mOutputFileFp);
                    }
                }
                else if(PT_H265 == pContext->mVEncChnAttr.VeAttr.Type)
                {
                    AW_MPI_VENC_GetH265SpsPpsInfo(pContext->mVeChn, &vencheader);
                    if(vencheader.nLength)
                    {
                        fwrite(vencheader.pBuffer,vencheader.nLength, 1, pContext->mOutputFileFp);
                    }
                }
                result = pthread_create(&pContext->privCap[pContext->mViDev][virvi_chn].thid, NULL, GetEncoderFrameThread, (void *)&pContext->privCap[pContext->mViDev][virvi_chn]);
                if (result < 0)
                {
                    alogd("pthread_create failed, Dev[%d], Chn[%d].\n", pContext->privCap[pContext->mViDev][virvi_chn].Dev, pContext->privCap[pContext->mViDev][virvi_chn].Chn);
                    continue;
                }
            }
        }
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            int eError = 0;
            alogd("wait get encoder frame thread exit!");
            pthread_join(pContext->privCap[pContext->mViDev][virvi_chn].thid, (void*)&eError);
            alogd("get encoder frame thread exit done!");
        }

        result = AW_MPI_VENC_StopRecvPic(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Stop Receive Picture error!");
            result = -1;
            goto _exit;
        }
#if 1
        /* better call AW_MPI_VI_DisableVirChn immediately after AW_MPI_VENC_StopRecvPic was invoked */
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            ret = AW_MPI_VI_DisableVirChn(pContext->mViDev, virvi_chn);
            if(ret < 0)
            {
                aloge("Disable VI Chn failed,VIDev = %d,VIChn = %d", pContext->mViDev, virvi_chn);
                return ret ;
            }
        }
#endif
        result = AW_MPI_VENC_ResetChn(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Reset Chn error!");
            result = -1;
            goto _exit;
        }
        AW_MPI_VENC_DestroyChn(pContext->mVeChn);
        if (result != SUCCESS)
        {
            alogd("VENC Destroy Chn error!");
            result = -1;
            goto _exit;
        }
        for (virvi_chn = 0; virvi_chn < 1; virvi_chn++)
        {
            result = hal_virvi_end(pContext->mViDev, virvi_chn);
            if(result < 0)
            {
                alogd("VI end failed!\n");
                result = -1;
                goto _exit;
            }
        }

        AW_MPI_ISP_Stop(pContext->mIspDev);
        AW_MPI_ISP_Exit();

        hal_vipp_end(pContext->mViDev);
        /* exit mpp systerm */
        ret = AW_MPI_SYS_Exit();
        if (ret < 0)
        {
            aloge("sys exit failed!");
            return -1;
        }
        fclose(pContext->mOutputFileFp);
        pContext->mOutputFileFp = NULL;
        alogd("======================================.\r\n");
        alogd("Auto Test count end: %d. (MaxCount==1000).\r\n", count);
        alogd("======================================.\r\n");
        count++;
    }
    if(pContext!=NULL)
    {
        free(pContext);
        pContext = NULL;
    }
    gpSampleVirvi2VencContext = NULL;
    printf("sample_virvi2venc2ce exit!\n");
    return 0;
_exit:
    if(pContext!=NULL)
    {
        free(pContext);
        pContext = NULL;
    }
    gpSampleVirvi2VencContext = NULL;
    return result;
}
