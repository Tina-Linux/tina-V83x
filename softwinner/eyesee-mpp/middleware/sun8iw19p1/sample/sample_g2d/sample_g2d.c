
#define LOG_TAG "sample_g2d"
#include <utils/plat_log.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/g2d_driver.h>

#include <vo/hwdisplay.h>
#include <mpi_vo.h>
#include <media/mpi_sys.h>
#include <media/mm_comm_vi.h>
#include <mpi_videoformat_conversion.h>
#include <SystemBase.h>
#include <VideoFrameInfoNode.h>
#include "media/mpi_vi.h"
#include "media/mpi_isp.h"
#include <utils/PIXEL_FORMAT_E_g2d_format_convert.h>
#include <utils/VIDEO_FRAME_INFO_S.h>
#include <confparser.h>
#include "sample_g2d.h"
#include "sample_g2d_mem.h"
#include "sample_g2d_config.h"
#include <cdx_list.h>


static int ParseCmdLine(int argc, char **argv, SampleG2dCmdLineParam *pCmdLinePara)
{
    alogd("sample virvi path:[%s], arg number is [%d]", argv[0], argc);
    int ret = 0;
    int i=1;
    memset(pCmdLinePara, 0, sizeof(SampleG2dCmdLineParam));
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
                "\t-path /mnt/extsd/sample_vi_g2d.conf");
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



static ERRORTYPE loadSampleG2dConfig(SampleG2dConfig *pConfig, const char *pConfPath)
{
    int ret = SUCCESS;

    pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    pConfig->mSrcWidth = 1920;
    pConfig->mSrcHeight = 1080;
    pConfig->mSrcRectX = 0;
    pConfig->mSrcRectY = 0;
    pConfig->mSrcRectW = 1920;
    pConfig->mSrcRectH = 1080;
    pConfig->mDstRotate = 90;
    pConfig->mDstWidth = 1080;
    pConfig->mDstHeight = 1920;
    pConfig->mDstRectX = 0;
    pConfig->mDstRectY = 0;
    pConfig->mDstRectW = 1080;
    pConfig->mDstRectH = 1920;

    if(pConfPath != NULL)
    {
        CONFPARSER_S stConfParser;
        ret = createConfParser(pConfPath, &stConfParser);
        if(ret < 0)
        {
            aloge("load conf fail"); 
            return FAILURE;
        }
        char *pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_G2D_KEY_PIC_FORMAT, NULL);
        if(!strcmp(pStrPixelFormat, "nv21"))
        {
            pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        }
        else if(!strcmp(pStrPixelFormat, "nv12"))
        {
            pConfig->mPicFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        }
        else
        {
            aloge("fatal error! conf file pic_format[%s] is unsupported", pStrPixelFormat);
            pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        } 
        
        pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_G2D_DST_PIC_FORMAT, NULL);
        if(!strcmp(pStrPixelFormat, "nv21"))
        {
            pConfig->mDstPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        }
        else if(!strcmp(pStrPixelFormat, "nv12"))
        {
            pConfig->mDstPicFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        }
        else if(!strcmp(pStrPixelFormat, "rgb888"))
        {
            pConfig->mDstPicFormat = MM_PIXEL_FORMAT_RGB_888;
        }
        else
        {
            aloge("fatal error! conf dst pic_format[%s] is unsupported", pStrPixelFormat);
            pConfig->mDstPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        }
		
		aloge("config_pixel_fmt:src:%d,dst:%d",pConfig->mPicFormat,pConfig->mDstPicFormat);
        pConfig->mSrcWidth  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_WIDTH, 0);
        pConfig->mSrcHeight  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_HEIGHT, 0);
        pConfig->mSrcRectX  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_RECT_X, 0);
        pConfig->mSrcRectY  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_RECT_Y, 0);
        pConfig->mSrcRectW  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_RECT_W, 0);
        pConfig->mSrcRectH  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_SRC_RECT_H, 0);
        pConfig->mDstRotate = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_ROTATE, 0);
        pConfig->mDstWidth  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_WIDTH, 0);
        pConfig->mDstHeight = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_HEIGHT, 0);
        pConfig->mDstRectX  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_RECT_X, 0);
        pConfig->mDstRectY  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_RECT_Y, 0);
        pConfig->mDstRectW  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_RECT_W, 0);
        pConfig->mDstRectH  = GetConfParaInt(&stConfParser, SAMPLE_G2D_KEY_DST_RECT_H, 0);

        
        pConfig->g2d_mod  = GetConfParaInt(&stConfParser, SAMPLE_G2D_MODE, 0);  // 0:roate, 1: scale, 2:roate & scale

        char *tmp_ptr = (char*)GetConfParaString(&stConfParser, SAMPLE_G2D_SRC_FILE_STR, NULL);
        strcpy(pConfig->SrcFile,tmp_ptr);
        
        tmp_ptr = (char*)GetConfParaString(&stConfParser, SAMPLE_G2D_DST_FILE_STR, NULL);
        strcpy(pConfig->DstFile,tmp_ptr); 

        aloge("src_file:%s, dst_file:%s",pConfig->SrcFile,pConfig->DstFile);
        destroyConfParser(&stConfParser);
    }
    return ret;
}

static int FreeFrmBuff(SAMPLE_G2D_CTX *p_g2d_ctx)
{ 
    if(NULL != p_g2d_ctx->src_frm_info.p_vir_addr[0])
    {
        g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[0]);
    }
    
    if(NULL != p_g2d_ctx->src_frm_info.p_vir_addr[1])
    {
        g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[1]);
    }
    
    if(NULL != p_g2d_ctx->dst_frm_info.p_vir_addr[0])
    {
        g2d_freeMem(p_g2d_ctx->dst_frm_info.p_vir_addr[0]);
    }

    if(NULL != p_g2d_ctx->dst_frm_info.p_vir_addr[1])
    {
        g2d_freeMem(p_g2d_ctx->dst_frm_info.p_vir_addr[1]);
    } 

    return 0; 
} 

static int PrepareFrmBuff(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    SampleG2dConfig *pConfig = NULL;
    unsigned int size = 0;

    pConfig = &p_g2d_ctx->mConfigPara;
    
    p_g2d_ctx->src_frm_info.frm_width = pConfig->mSrcWidth;
    p_g2d_ctx->src_frm_info.frm_height =  pConfig->mSrcHeight;

    p_g2d_ctx->dst_frm_info.frm_width = pConfig->mDstWidth;
    p_g2d_ctx->dst_frm_info.frm_height = pConfig->mDstHeight;

    size = ALIGN(p_g2d_ctx->src_frm_info.frm_width, 16)*ALIGN(p_g2d_ctx->src_frm_info.frm_height, 16);

    p_g2d_ctx->src_frm_info.p_vir_addr[0] = (void *)g2d_allocMem(size);
    if(NULL == p_g2d_ctx->src_frm_info.p_vir_addr[0])
    {
        aloge("malloc_src_frm_y_mem_failed");
        return -1;
    }
    
    p_g2d_ctx->src_frm_info.p_vir_addr[1] = (void *)g2d_allocMem(size/2);
    if(NULL == p_g2d_ctx->src_frm_info.p_vir_addr[1])
    {
        g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[0]);
        aloge("malloc_src_frm_c_mem_failed");    
        return -1;
    } 
    
	if(pConfig->mDstPicFormat == MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420 ||
			pConfig->mDstPicFormat == MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420)
	{
		size = ALIGN(p_g2d_ctx->dst_frm_info.frm_width, 16)*ALIGN(p_g2d_ctx->dst_frm_info.frm_height, 16);
		p_g2d_ctx->dst_frm_info.p_vir_addr[0] = (void *)g2d_allocMem(size);
		if(NULL == p_g2d_ctx->dst_frm_info.p_vir_addr[0])
		{
			g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[0]); 
			g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[1]);
			aloge("malloc_dst_frm_y_mem_failed");
			return -1;
		}
		
		p_g2d_ctx->dst_frm_info.p_vir_addr[1] = (void *)g2d_allocMem(size/2);
		if(NULL == p_g2d_ctx->dst_frm_info.p_vir_addr[1])
		{
			g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[0]); 
			g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[1]); 
			g2d_freeMem(p_g2d_ctx->dst_frm_info.p_vir_addr[0]); 
			aloge("malloc_dst_frm_c_mem_failed");    
			return -1;
		} 
		p_g2d_ctx->dst_frm_info.p_phy_addr[0] = (void *)g2d_getPhyAddrByVirAddr(p_g2d_ctx->dst_frm_info.p_vir_addr[0]); 
		p_g2d_ctx->dst_frm_info.p_phy_addr[1] = (void *)g2d_getPhyAddrByVirAddr(p_g2d_ctx->dst_frm_info.p_vir_addr[1]); 
	}
	else if(pConfig->mDstPicFormat == MM_PIXEL_FORMAT_RGB_888)
	{
		size = p_g2d_ctx->dst_frm_info.frm_width * p_g2d_ctx->dst_frm_info.frm_height * 3;
		p_g2d_ctx->dst_frm_info.p_vir_addr[0] = (void *)g2d_allocMem(size);
		if(NULL == p_g2d_ctx->dst_frm_info.p_vir_addr[0])
		{
			if(p_g2d_ctx->src_frm_info.p_vir_addr[0] != NULL)
			{
				g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[0]); 
			}
			if(p_g2d_ctx->src_frm_info.p_vir_addr[1] != NULL)
			{
				g2d_freeMem(p_g2d_ctx->src_frm_info.p_vir_addr[1]);
			}
			aloge("malloc_dst_frm_y_mem_failed");
			return -1;
		}
		p_g2d_ctx->dst_frm_info.p_phy_addr[0] = (void *)g2d_getPhyAddrByVirAddr(p_g2d_ctx->dst_frm_info.p_vir_addr[0]); 
	}


    p_g2d_ctx->src_frm_info.p_phy_addr[0] = (void *)g2d_getPhyAddrByVirAddr(p_g2d_ctx->src_frm_info.p_vir_addr[0]); 
    p_g2d_ctx->src_frm_info.p_phy_addr[1] = (void *)g2d_getPhyAddrByVirAddr(p_g2d_ctx->src_frm_info.p_vir_addr[1]);


    return 0; 
}

static int SampleG2d_G2dOpen(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    int ret = 0;
    p_g2d_ctx->mG2dFd = open("/dev/g2d", O_RDWR, 0);
    if (p_g2d_ctx->mG2dFd < 0)
    {
        aloge("fatal error! open /dev/g2d failed");
        ret = -1;
    }
    return ret;
}

static int SampleG2d_G2dClose(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    if(p_g2d_ctx->mG2dFd >= 0)
    {
        close(p_g2d_ctx->mG2dFd);
        p_g2d_ctx->mG2dFd = -1;
    }
    return 0;
}

static int SampleG2d_G2dConvert_rotate(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    int ret = 0;
    g2d_blt_h blit;
    g2d_fmt_enh eSrcFormat, eDstFormat; 
    SampleG2dConfig *pConfig = NULL;

    pConfig = &p_g2d_ctx->mConfigPara;

    
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(pConfig->mPicFormat, &eSrcFormat);
    if(ret!=SUCCESS)
    {
        aloge("fatal error! src pixel format[0x%x] is invalid!", pConfig->mPicFormat);
        return -1;
    }
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(pConfig->mDstPicFormat, &eDstFormat);
    if(ret!=SUCCESS)
    {
        aloge("fatal error! dst pixel format[0x%x] is invalid!", pConfig->mPicFormat);
        return -1;
    }

    //config blit
    memset(&blit, 0, sizeof(g2d_blt_h));
    switch(pConfig->mDstRotate)
    {
        case 0:
            blit.flag_h = G2D_BLT_NONE_H;   //G2D_ROT_0, G2D_BLT_NONE_H
            break;
        case 90:
            blit.flag_h = G2D_ROT_90;
            break;
        case 180:
            blit.flag_h = G2D_ROT_180;
            break;
        case 270:
            blit.flag_h = G2D_ROT_270;
            break;
        default:
            aloge("fatal error! rotation[%d] is invalid!", pConfig->mDstRotate);
            blit.flag_h = G2D_BLT_NONE_H;
            break;
    }

    if(p_g2d_ctx->dst_frm_info.frm_width != p_g2d_ctx->src_frm_info.frm_width || 
        p_g2d_ctx->dst_frm_info.frm_height != p_g2d_ctx->src_frm_info.frm_height )
    {
        aloge("fatal_error: the scale can't be perormed when do rotation,size_info:src:%u-%u,dst:%u-%u",
            p_g2d_ctx->src_frm_info.frm_width,p_g2d_ctx->src_frm_info.frm_height,
            p_g2d_ctx->dst_frm_info.frm_width,p_g2d_ctx->dst_frm_info.frm_height);
    }

    
    //blit.src_image_h.bbuff = 1;
    //blit.src_image_h.color = 0xff;
    blit.src_image_h.format = eSrcFormat;
    blit.src_image_h.laddr[0] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[0];
    blit.src_image_h.laddr[1] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[1];
    blit.src_image_h.laddr[2] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = p_g2d_ctx->src_frm_info.frm_width;
    blit.src_image_h.height = p_g2d_ctx->src_frm_info.frm_height;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = pConfig->mSrcRectX;
    blit.src_image_h.clip_rect.y = pConfig->mSrcRectY;
    blit.src_image_h.clip_rect.w = pConfig->mSrcRectW;
    blit.src_image_h.clip_rect.h = pConfig->mSrcRectH;
    blit.src_image_h.gamut = G2D_BT601;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

    //blit.dst_image_h.bbuff = 1;
    //blit.dst_image_h.color = 0xff;
    blit.dst_image_h.format = eDstFormat;
    blit.dst_image_h.laddr[0] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[0];
    blit.dst_image_h.laddr[1] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[1];
    blit.dst_image_h.laddr[2] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = p_g2d_ctx->dst_frm_info.frm_width;
    blit.dst_image_h.height = p_g2d_ctx->dst_frm_info.frm_height;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = pConfig->mDstRectX;
    blit.dst_image_h.clip_rect.y = pConfig->mDstRectY;
    blit.dst_image_h.clip_rect.w = pConfig->mDstRectW;
    blit.dst_image_h.clip_rect.h = pConfig->mDstRectH;
    blit.dst_image_h.gamut = G2D_BT601;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    ret = ioctl(p_g2d_ctx->mG2dFd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        aloge("fatal error! bit-block(image) transfer failed[%d]", ret);
        system("cd /sys/class/sunxi_dump;echo 0x14A8000,0x14A8100 > dump;cat dump");
    }

    return ret;
} 

static int SampleG2d_G2dConvert_scale(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    int ret = 0;
    g2d_blt_h blit;
    g2d_fmt_enh eSrcFormat, eDstFormat; 
    SampleG2dConfig *pConfig = NULL;

    pConfig = &p_g2d_ctx->mConfigPara;

    
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(pConfig->mPicFormat, &eSrcFormat);
    if(ret!=SUCCESS)
    {
        aloge("fatal error! src pixel format[0x%x] is invalid!", pConfig->mPicFormat);
        return -1;
    }
    ret = convert_PIXEL_FORMAT_E_to_g2d_fmt_enh(pConfig->mDstPicFormat, &eDstFormat);
    if(ret!=SUCCESS)
    {
        aloge("fatal error! dst pixel format[0x%x] is invalid!", pConfig->mPicFormat);
        return -1;
    }

    //config blit
    memset(&blit, 0, sizeof(g2d_blt_h));

    if(0 != pConfig->mDstRotate)
    {
        aloge("fatal_err: rotation can't be performed when do scaling");
    }

    blit.flag_h = G2D_BLT_NONE_H;       // angle rotation used
//    switch(pConfig->mDstRotate)
//    {
//        case 0:
//            blit.flag_h = G2D_BLT_NONE_H;   //G2D_ROT_0, G2D_BLT_NONE_H
//            break;
//        case 90:
//            blit.flag_h = G2D_ROT_90;
//            break;
//        case 180:
//            blit.flag_h = G2D_ROT_180;
//            break;
//        case 270:
//            blit.flag_h = G2D_ROT_270;
//            break;
//        default:
//            aloge("fatal error! rotation[%d] is invalid!", pConfig->mDstRotate);
//            blit.flag_h = G2D_BLT_NONE_H;
//            break;
//    }
    //blit.src_image_h.bbuff = 1;
    //blit.src_image_h.color = 0xff;
    blit.src_image_h.format = eSrcFormat;
    blit.src_image_h.laddr[0] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[0];
    blit.src_image_h.laddr[1] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[1];
    blit.src_image_h.laddr[2] = (unsigned int)p_g2d_ctx->src_frm_info.p_phy_addr[2];
    //blit.src_image_h.haddr[] = 
    blit.src_image_h.width = p_g2d_ctx->src_frm_info.frm_width;
    blit.src_image_h.height = p_g2d_ctx->src_frm_info.frm_height;
    blit.src_image_h.align[0] = 0;
    blit.src_image_h.align[1] = 0;
    blit.src_image_h.align[2] = 0;
    blit.src_image_h.clip_rect.x = pConfig->mSrcRectX;
    blit.src_image_h.clip_rect.y = pConfig->mSrcRectY;
    blit.src_image_h.clip_rect.w = pConfig->mSrcRectW;
    blit.src_image_h.clip_rect.h = pConfig->mSrcRectH;
    blit.src_image_h.gamut = G2D_BT601;
    blit.src_image_h.bpremul = 0;
    //blit.src_image_h.alpha = 0xff;
    blit.src_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.src_image_h.fd = -1;
    blit.src_image_h.use_phy_addr = 1;

	aloge("g2d_dst_fmt:%d",eDstFormat);

    //blit.dst_image_h.bbuff = 1;
    //blit.dst_image_h.color = 0xff;
    blit.dst_image_h.format = eDstFormat;
    blit.dst_image_h.laddr[0] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[0];
    blit.dst_image_h.laddr[1] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[1];
    blit.dst_image_h.laddr[2] = (unsigned int)p_g2d_ctx->dst_frm_info.p_phy_addr[2];
    //blit.dst_image_h.haddr[] = 
    blit.dst_image_h.width = p_g2d_ctx->dst_frm_info.frm_width;
    blit.dst_image_h.height = p_g2d_ctx->dst_frm_info.frm_height;
    blit.dst_image_h.align[0] = 0;
    blit.dst_image_h.align[1] = 0;
    blit.dst_image_h.align[2] = 0;
    blit.dst_image_h.clip_rect.x = pConfig->mDstRectX;
    blit.dst_image_h.clip_rect.y = pConfig->mDstRectY;
    blit.dst_image_h.clip_rect.w = pConfig->mDstRectW;
    blit.dst_image_h.clip_rect.h = pConfig->mDstRectH;
    blit.dst_image_h.gamut = G2D_BT601;
    blit.dst_image_h.bpremul = 0;
    //blit.dst_image_h.alpha = 0xff;
    blit.dst_image_h.mode = G2D_PIXEL_ALPHA;   //G2D_PIXEL_ALPHA, G2D_GLOBAL_ALPHA
    blit.dst_image_h.fd = -1;
    blit.dst_image_h.use_phy_addr = 1;

    ret = ioctl(p_g2d_ctx->mG2dFd, G2D_CMD_BITBLT_H, (unsigned long)&blit);
    if(ret < 0)
    {
        aloge("fatal error! bit-block(image) transfer failed[%d]", ret);
        system("cd /sys/class/sunxi_dump;echo 0x14A8000,0x14A8100 > dump;cat dump");
    }

    return ret;
} 

static void SampleG2d_G2dConvert(SAMPLE_G2D_CTX *p_g2d_ctx)
{
    SampleG2dConfig *pConfig = &p_g2d_ctx->mConfigPara;
    if(0 == pConfig->g2d_mod)
    {
        (void)SampleG2d_G2dConvert_rotate(p_g2d_ctx);
    }
    else if(1 == pConfig->g2d_mod)
    {
        (void)SampleG2d_G2dConvert_scale(p_g2d_ctx);
    }
}


int main(int argc, char *argv[])
{
    int ret = 0;
    unsigned int size1 = 0;
    unsigned int size2 = 0; 
    unsigned int read_len = 0;
    unsigned int out_len = 0;
    SAMPLE_G2D_CTX g2d_ctx;
    SAMPLE_G2D_CTX *p_g2d_ctx = &g2d_ctx;

    memset(p_g2d_ctx,0,sizeof(SAMPLE_G2D_CTX)); 
    
    SampleG2dConfig *pConfig = &p_g2d_ctx->mConfigPara;
    GLogConfig stGLogConfig = 
    {
        .FLAGS_logtostderr = 0,
        .FLAGS_colorlogtostderr = 1,
        .FLAGS_stderrthreshold = _GLOG_INFO,
        .FLAGS_minloglevel = _GLOG_INFO,
        .FLAGS_logbuflevel = -1,
        .FLAGS_logbufsecs = 0,
        .FLAGS_max_log_size = 1,
        .FLAGS_stop_logging_if_full_disk = 1,
    };
    strcpy(stGLogConfig.LogDir, "/tmp/log");
    strcpy(stGLogConfig.InfoLogFileNameBase, "LOG-");
    strcpy(stGLogConfig.LogFileNameExtension, "IPC-");
    log_init(argv[0], &stGLogConfig);


    if(ParseCmdLine(argc, argv, &p_g2d_ctx->mCmdLinePara) != 0)
    {
        ret = -1;
        return ret;
    }
    char *pConfigFilePath = NULL; 
    if(strlen(p_g2d_ctx->mCmdLinePara.mConfigFilePath) > 0)
    {
        pConfigFilePath = p_g2d_ctx->mCmdLinePara.mConfigFilePath;
    }
    else
    {
        pConfigFilePath = NULL;
    }

    if(loadSampleG2dConfig(&p_g2d_ctx->mConfigPara, pConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        ret = -1;
        return ret;
    }

    memset(&p_g2d_ctx->mSysConf, 0, sizeof(MPP_SYS_CONF_S));
    p_g2d_ctx->mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&p_g2d_ctx->mSysConf);
    ret = AW_MPI_SYS_Init();
    if (ret < 0)
    {
        aloge("sys Init failed!");
        ret = -1;
        return ret;
    }

    g2d_MemOpen();

    ret = PrepareFrmBuff(p_g2d_ctx);
    if(0 != ret)
    {
        aloge("malloc frm buffer failed"); 
        goto _err1;
    }

    p_g2d_ctx->fd_in = fopen(p_g2d_ctx->mConfigPara.SrcFile,"r");
    if(NULL == p_g2d_ctx->fd_in)
    {
        aloge("open src file failed");
        goto _err2;
    }
    fseek(p_g2d_ctx->fd_in, 0, SEEK_SET); 
    p_g2d_ctx->fd_out = fopen(p_g2d_ctx->mConfigPara.DstFile, "wb");
    if (NULL == p_g2d_ctx->fd_out)
    {
        aloge("open out file failed");
        goto _err2;
    }
    fseek(p_g2d_ctx->fd_out, 0, SEEK_SET); 

    read_len = p_g2d_ctx->src_frm_info.frm_width * p_g2d_ctx->src_frm_info.frm_height;
    
    size1 = fread(p_g2d_ctx->src_frm_info.p_vir_addr[0] , 1, read_len, p_g2d_ctx->fd_in);
    if(size1 != read_len)
    {
        aloge("read_y_data_frm_src_file_invalid");
    }
    size2 = fread(p_g2d_ctx->src_frm_info.p_vir_addr[1], 1, read_len /2, p_g2d_ctx->fd_in);
    if(size2 != read_len/2)
    {
        aloge("read_c_data_frm_src_file_invalid");
    } 
    
    fclose(p_g2d_ctx->fd_in);
    
    g2d_flushCache((void *)p_g2d_ctx->src_frm_info.p_vir_addr[0], read_len);
    g2d_flushCache((void *)p_g2d_ctx->src_frm_info.p_vir_addr[1], read_len/2);

    // g2d related operations start ->
    SampleG2d_G2dOpen(p_g2d_ctx);
    SampleG2d_G2dConvert(p_g2d_ctx); 
    SampleG2d_G2dClose(p_g2d_ctx);
    // g2d related operations end <-
	if(pConfig->mDstPicFormat == MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420 ||
			pConfig->mDstPicFormat == MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420)
	{
		out_len = p_g2d_ctx->dst_frm_info.frm_width * p_g2d_ctx->dst_frm_info.frm_height;
		g2d_flushCache((void *)p_g2d_ctx->dst_frm_info.p_vir_addr[0], out_len);
		g2d_flushCache((void *)p_g2d_ctx->dst_frm_info.p_vir_addr[1], out_len/2);
		
		fwrite(p_g2d_ctx->dst_frm_info.p_vir_addr[0], 1, out_len, p_g2d_ctx->fd_out);
		fwrite(p_g2d_ctx->dst_frm_info.p_vir_addr[1], 1, out_len/2, p_g2d_ctx->fd_out);
	}
	else if(pConfig->mDstPicFormat == MM_PIXEL_FORMAT_RGB_888)
	{
		out_len = p_g2d_ctx->dst_frm_info.frm_width * p_g2d_ctx->dst_frm_info.frm_height *3;
		g2d_flushCache((void *)p_g2d_ctx->dst_frm_info.p_vir_addr[0], out_len);
		
		fwrite(p_g2d_ctx->dst_frm_info.p_vir_addr[0], 1, out_len, p_g2d_ctx->fd_out);
	}
	
    
    fclose(p_g2d_ctx->fd_out);

_err2:
    FreeFrmBuff(p_g2d_ctx);
 _err1: 
    g2d_MemClose();
    AW_MPI_SYS_Exit(); 

    return 0; 
}


