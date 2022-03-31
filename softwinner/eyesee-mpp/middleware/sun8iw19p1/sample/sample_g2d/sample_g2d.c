#define LOG_TAG "sample_g2d"
#include <utils/plat_log.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <asm-generic/int-l64.h>
#include <stdbool.h>
#include "linux/g2d_driver.h"

#include <confparser.h>
#include "sample_g2d.h"

#define DISP_MEM_REQUEST 0x2c0
#define DISP_MEM_RELEASE 0x2c1
#define DISP_MEM_GETADR 0x2c2

#define DISPALIGN(value, align) ((align==0)?(unsigned long)value:((((unsigned long)value) + ((align) - 1)) & ~((align) - 1)))

static int g_bSampleExit = 0;

static inline int LoadSampleG2dConfig(SampleG2dContext *pConfig, const char *conf_path)
{
    int iRet;

    CONFPARSER_S stConfParser;
    iRet = createConfParser(conf_path, &stConfParser);
    if(iRet < 0)
    {
        alogd("user not set config file. use default test parameter!");
        pConfig->iG2dMode = 0; //blt mode
        strncpy(pConfig->cstrSrcPath, "./desk_3840x2160.nv21", sizeof(pConfig->cstrSrcPath));
        pConfig->iSrcWidth = 3840;
        pConfig->iSrcHeight = 2160;
        pConfig->iSrcRectX = 0;
        pConfig->iSrcRectY = 0;
        pConfig->iSrcRectW = 3840;
        pConfig->iSrcRectH = 2160;
        strncpy(pConfig->cstrDstPath, "./bike_1280x720.nv12", sizeof(pConfig->cstrDstPath));
        pConfig->iDstWidth = 1280;
        pConfig->iDstHeight = 720;
        pConfig->iDstRectX = 0;
        pConfig->iDstRectY = 0;
        pConfig->iDstRectW = 1280;
        pConfig->iDstRectH = 720;
        strncpy(pConfig->cstrOutPath, "./G2dOut.nv21", sizeof(pConfig->cstrOutPath));
        goto use_default_conf;
    }

    if (pConfig->iG2dMode == -1) {
        pConfig->iG2dMode = GetConfParaInt(&stConfParser, SAMPLE_G2D_MODE, 0);
    }
    if (pConfig->iSrcWidth == -1) {
        pConfig->iSrcWidth = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_IMG_W, 0);
    }
    if (pConfig->iSrcHeight == -1) {
        pConfig->iSrcHeight = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_IMG_H, 0);
    }
    if (pConfig->iSrcRectX == -1) {
        pConfig->iSrcRectX = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_RECT_X, 0);
    }
    if (pConfig->iSrcRectY == -1) {
        pConfig->iSrcRectY = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_RECT_Y, 0);
    }
    if (pConfig->iSrcRectW == -1) {
        pConfig->iSrcRectW = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_RECT_W, 0);
    }
    if (pConfig->iSrcRectH == -1) {
        pConfig->iSrcRectH = GetConfParaInt(&stConfParser, SAMPLE_G2D_SRC_RECT_H, 0);
    }
    if (pConfig->iDstWidth == -1) {
        pConfig->iDstWidth = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_IMG_W, 0);
    }
    if (pConfig->iDstHeight == -1) {
        pConfig->iDstHeight = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_IMG_H, 0);
    }
    if (pConfig->iDstRectX == -1) {
        pConfig->iDstRectX = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_RECT_X, 0);
    }
    if (pConfig->iDstRectY == -1) {
        pConfig->iDstRectY = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_RECT_Y, 0);
    }
    if (pConfig->iDstRectW == -1) {
        pConfig->iDstRectW = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_RECT_W, 0);
    }
    if (pConfig->iDstRectH == -1) {
        pConfig->iDstRectH = GetConfParaInt(&stConfParser, SAMPLE_G2D_DST_RECT_H, 0);
    }

    char *pcTmpPtr;
    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_G2D_SRC_IMG, NULL);
    alogd("(%s)(%d)", pcTmpPtr, pConfig->cstrSrcPath[0]);
    if (NULL != pcTmpPtr && pConfig->cstrSrcPath[0] == 0xFF) {
        strncpy(pConfig->cstrSrcPath, pcTmpPtr, sizeof(pConfig->cstrSrcPath));
    }

    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_G2D_DST_IMG, NULL);
    alogd("(%s)(%d)", pcTmpPtr, pConfig->cstrDstPath[0]);
    if (NULL != pcTmpPtr && pConfig->cstrDstPath[0] == 0xFF) {
        strncpy(pConfig->cstrDstPath, pcTmpPtr, sizeof(pConfig->cstrDstPath));
    }

    pcTmpPtr = (char *)GetConfParaString(&stConfParser, SAMPLE_G2D_OUTPUT, NULL);
    alogd("(%s)(%d)", pcTmpPtr, pConfig->cstrOutPath[0]);
    if (NULL != pcTmpPtr && pConfig->cstrOutPath[0] == 0xFF) {
        strncpy(pConfig->cstrOutPath, pcTmpPtr, sizeof(pConfig->cstrOutPath));
    }

use_default_conf:
    alogd("g2d_mode=(%d), output_file=(%s)", pConfig->iG2dMode, pConfig->cstrOutPath);
    alogd("src_img=%s,iSrcW=%d,iSrcH=%d,iSrcRectX=%d,iSrcRectY=%d,iSrcRectW=%d,iSrcRectH=%d",
        pConfig->cstrSrcPath, pConfig->iSrcWidth, pConfig->iSrcHeight,
        pConfig->iSrcRectX, pConfig->iSrcRectY, pConfig->iSrcRectW, pConfig->iSrcRectH);
    alogd("dst_img=%s,iDstW=%d,iDstH=%d,iDstRectX=%d,iDstRectY=%d,iDstRectW=%d,iDstRectH=%d",
        pConfig->cstrDstPath, pConfig->iDstWidth, pConfig->iDstHeight,
        pConfig->iDstRectX, pConfig->iDstRectY, pConfig->iDstRectW, pConfig->iDstRectH);

    destroyConfParser(&stConfParser);
    return SUCCESS;
}

static inline void usage(const char *argv0)
{
    printf(
        "\033[33m"
        "exec [-h|--help] [-p|--path]\n"
        "   <-h|--help>: print the help information\n"
        "   <-p|--path>       <args>: point to the configuration file path.\n"
        "   <-m|--mode>       <args>: indicate g2d mode. like blt and so on.\n"
        "   <-f|--flag>       <args>: blanding, mask flags.\n"
        "   <--simg>          <args>: source image should be process.\n"
        "   <--dimg>          <args>: destination image should be process.\n"
        "   <--simgw>         <args>: source image's width.\n"
        "   <--simgh>         <args>: source image's height.\n"
        "   <--srectx>        <args>: source image rectangle's x coordinate.\n"
        "   <--srecty>        <args>: source image rectangle's x coordinate.\n"
        "   <--srectw>        <args>: source image rectangle's width.\n"
        "   <--srecth>        <args>: source image rectangle's height.\n"
        "   <--dimgw>         <args>: destination image's width.\n"
        "   <--dimgh>         <args>: destination image's height.\n"
        "   <--drectx>        <args>: destination image rectangle's x coordinate.\n"
        "   <--drecty>        <args>: destination image rectangle's x coordinate.\n"
        "   <--drectw>        <args>: destination image rectangle's width.\n"
        "   <--drecth>        <args>: destination image rectangle's height.\n"
        "\033[0m\n");
}

static struct option pstLongOptions[] = {
   {"help",        no_argument,       0, 'h'},
   {"mode",        required_argument, 0, 'm'},
   {"path",        required_argument, 0, 'p'},
   {"simg",        required_argument, 0, 256},
   {"dimg",        required_argument, 0, 257},
   {"simgw",       required_argument, 0, 258},
   {"simgh",       required_argument, 0, 259},
   {"srectx",      required_argument, 0, 260},
   {"srecty",      required_argument, 0, 261},
   {"srectw",      required_argument, 0, 262},
   {"srecth",      required_argument, 0, 263},
   {"dimgw",       required_argument, 0, 264},
   {"dimgh",       required_argument, 0, 265},
   {"drectx",      required_argument, 0, 266},
   {"drecty",      required_argument, 0, 267},
   {"drectw",      required_argument, 0, 268},
   {"drecth",      required_argument, 0, 269},
   {"flag",        required_argument, 0, 'f'},
   {0,             0,                 0,  0 }
};

static int ParseCmdLine(int argc, char **argv, SampleG2dContext *pstContext)
{
    int mRet;
    int iOptIndex = 0;

    memset(pstContext, -1, sizeof(SampleG2dContext));
    pstContext->mCmdLinePara.mConfigFilePath[0] = 0;
    while (1) {
        mRet = getopt_long(argc, argv, ":p:f:m:h", pstLongOptions, &iOptIndex);
        if (mRet == -1) {
            break;
        }

        switch (mRet) {
            /* let the "sampleXXX -path sampleXXX.conf" command to be compatible with
             * "sampleXXX -p sampleXXX.conf"
             */
            case 'p':
                if (strcmp("ath", optarg) == 0) {
                    if (NULL == argv[optind]) {
                        usage(argv[0]);
                        goto opt_need_arg;
                    }
                    alogd("path is [%s]\n", argv[optind]);
                    strncpy(pstContext->mCmdLinePara.mConfigFilePath,
                        argv[optind], sizeof(pstContext->mCmdLinePara.mConfigFilePath));
                } else {
                    alogd("path is [%s]\n", optarg);
                    strncpy(pstContext->mCmdLinePara.mConfigFilePath,
                        optarg, sizeof(pstContext->mCmdLinePara.mConfigFilePath));
                }
                break;
            case 'm':
                alogd("g2d mode is [%d]\n", atoi(optarg));
                pstContext->iG2dMode = atoi(optarg);
                break;
            case 'f':
                alogd("g2d flags is [%d]\n", atoi(optarg));
                pstContext->iG2dFlags = atoi(optarg);
                break;
            case 256:
                alogd("source image path is [%s]\n", optarg);
                strncpy(pstContext->cstrSrcPath, optarg, sizeof(pstContext->cstrSrcPath));
                break;
            case 257:
                alogd("destination image path is [%s]\n", optarg);
                strncpy(pstContext->cstrDstPath, optarg, sizeof(pstContext->cstrDstPath));
                break;
            case 258:
                alogd("source image's width [%d]\n", atoi(optarg));
                pstContext->iSrcWidth = atoi(optarg);
                break;
            case 259:
                alogd("source image's height [%d]\n", atoi(optarg));
                pstContext->iSrcHeight = atoi(optarg);
                break;
            case 260:
                alogd("source image rectangle's x coordinate [%d]\n", atoi(optarg));
                pstContext->iSrcRectX= atoi(optarg);
                break;
            case 261:
                alogd("source image rectangle's y coordinate [%d]\n", atoi(optarg));
                pstContext->iSrcRectY= atoi(optarg);
                break;
            case 262:
                alogd("source image rectangle's width [%d]\n", atoi(optarg));
                pstContext->iSrcRectW= atoi(optarg);
                break;
            case 263:
                alogd("source image rectangle's height [%d]\n", atoi(optarg));
                pstContext->iSrcRectH= atoi(optarg);
                break;
            case 264:
                alogd("dstination image's width [%d]\n", atoi(optarg));
                pstContext->iDstWidth = atoi(optarg);
                break;
            case 265:
                alogd("dstination image's height [%d]\n", atoi(optarg));
                pstContext->iDstHeight = atoi(optarg);
                break;
            case 266:
                alogd("dstination image rectangle's x coordinate [%d]\n", atoi(optarg));
                pstContext->iDstRectX = atoi(optarg);
                break;
            case 267:
                alogd("dstination image rectangle's y coordinate [%d]\n", atoi(optarg));
                pstContext->iDstRectY= atoi(optarg);
                break;
            case 268:
                alogd("dstination image rectangle's width [%d]\n", atoi(optarg));
                pstContext->iDstRectW = atoi(optarg);
                break;
            case 269:
                alogd("dstination image rectangle's height [%d]\n", atoi(optarg));
                pstContext->iDstRectH = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                goto print_help_exit;
                break;
            case ':':
                aloge("option \"%s\" need <arg>\n", argv[optind - 1]);
                goto opt_need_arg;
                break;
            case '?':
                if (optind > 2) {
                    break;
                }
                aloge("unknow option \"%s\"\n", argv[optind - 1]);
                usage(argv[0]);
                goto unknow_option;
                break;
            default:
                printf("?? why getopt_long returned character code 0%o ??\n", mRet);
                break;
        }
    }

    return 0;
opt_need_arg:
unknow_option:
print_help_exit:
    return -1;
}

void SignalHandle(int iArg)
{
    alogd("receive exit signal. \n");
    g_bSampleExit = 1;
}

static inline int InitG2dDevice(SampleG2dContext *pstContext)
{
    int iRet = 0;
	unsigned long dwaDispArg[6];

    pstContext->iG2dFd = open("/dev/g2d", O_RDWR);
    if (pstContext->iG2dFd < 0) {
        iRet = -1;
        aloge("open device /dev/g2d failed!!");
        goto open_g2d_dev_err;
    }
    pstContext->iDispFd = open("/dev/disp", O_RDWR);
    if (pstContext->iDispFd < 0) {
        iRet = -1;
        aloge("open device /dev/disp failed!!");
        goto open_disp_dev_err;
    }

    unsigned long dwSrcSize = pstContext->iSrcWidth * pstContext->iSrcHeight * 3 / 2;
    unsigned long dwDstSize = pstContext->iDstWidth * pstContext->iDstHeight * 3 / 2;
	dwaDispArg[0] = 0;
	dwaDispArg[1] = dwSrcSize;
    iRet = ioctl(pstContext->iDispFd, DISP_MEM_REQUEST, (void *)dwaDispArg);
    if (iRet < 0) {
        aloge("alloc memory for source image failed!!");
        goto alloc_src_err;
    }
    pstContext->mG2dInfo.iSrcIdx = 0;
    dwaDispArg[0] = 0;
    pstContext->mG2dInfo.pSrcPhyAddr = (void*)ioctl(pstContext->iDispFd,
        DISP_MEM_GETADR, (void *)dwaDispArg);
    pstContext->mG2dInfo.pSrcPhyAddr = (void*)DISPALIGN(pstContext->mG2dInfo.pSrcPhyAddr, 4);
	pstContext->mG2dInfo.pSrcVirAddr = (void*)mmap(NULL, dwSrcSize,
	    PROT_READ | PROT_WRITE, MAP_SHARED, pstContext->iDispFd, (unsigned long)pstContext->mG2dInfo.pSrcPhyAddr);
	if (NULL == pstContext->mG2dInfo.pSrcVirAddr) {
        aloge("map memory for source image failed!!");
        iRet = -1;
        goto map_src_err;
	}

    FILE *pSrcFp;
    pSrcFp = fopen(pstContext->cstrSrcPath, "rb+");
    if (pSrcFp == NULL) {
        aloge("open source image (%s) failed!!", pstContext->cstrSrcPath);
        iRet = -1;
        goto open_src_err;
    }
    fread(pstContext->mG2dInfo.pSrcVirAddr, dwSrcSize, 1, pSrcFp);
    fclose(pSrcFp);

	dwaDispArg[0] = 1;
	dwaDispArg[1] = dwDstSize;
    iRet = ioctl(pstContext->iDispFd, DISP_MEM_REQUEST, (void *)dwaDispArg);
    if (iRet < 0) {
        aloge("alloc memory for destination image failed!!");
        goto alloc_dst_err;
    }
    pstContext->mG2dInfo.iDstIdx = 1;
    dwaDispArg[0] = 1;
    pstContext->mG2dInfo.pDstPhyAddr = (void*)ioctl(pstContext->iDispFd,
        DISP_MEM_GETADR, (void *)dwaDispArg);
    pstContext->mG2dInfo.pDstPhyAddr = (void*)DISPALIGN(pstContext->mG2dInfo.pDstPhyAddr, 4);
	pstContext->mG2dInfo.pDstVirAddr = (void*)mmap(NULL, dwDstSize,
	    PROT_READ | PROT_WRITE, MAP_SHARED, pstContext->iDispFd, (unsigned long)pstContext->mG2dInfo.pDstPhyAddr);
	if (NULL == pstContext->mG2dInfo.pSrcVirAddr) {
        aloge("map memory for source image failed!!");
        iRet = -1;
        goto map_dst_err;
	}

    FILE *pDstFp;
    pDstFp = fopen(pstContext->cstrDstPath, "rb+");
    if (pDstFp == NULL) {
        aloge("open destination image (%s) failed!!", pstContext->cstrDstPath);
        iRet = -1;
        goto open_dst_err;
    }
    fread(pstContext->mG2dInfo.pDstVirAddr, dwDstSize, 1, pDstFp);
    fclose(pDstFp);

    return 0;

open_dst_err:
    munmap(pstContext->mG2dInfo.pDstVirAddr, dwDstSize);
map_dst_err:
	dwaDispArg[0] = pstContext->mG2dInfo.iDstIdx;
    ioctl(pstContext->iDispFd, DISP_MEM_RELEASE, (void *)dwaDispArg);
alloc_dst_err:
open_src_err:
    munmap(pstContext->mG2dInfo.pSrcVirAddr, dwSrcSize);
map_src_err:
	dwaDispArg[0] = pstContext->mG2dInfo.iSrcIdx;
    ioctl(pstContext->iDispFd, DISP_MEM_RELEASE, (void *)dwaDispArg);
alloc_src_err:
    close(pstContext->iDispFd);
open_disp_dev_err:
    close(pstContext->iG2dFd);
open_g2d_dev_err:
    return iRet;
}

static inline void DestroyD2dDevice(SampleG2dContext *pstContext)
{
	unsigned long dwaDispArg[6];
    unsigned long dwSrcSize = pstContext->iSrcWidth * pstContext->iSrcHeight * 3 / 2;
    unsigned long dwDstSize = pstContext->iDstWidth * pstContext->iDstHeight * 3 / 2;

    munmap(pstContext->mG2dInfo.pDstVirAddr, dwDstSize);
	dwaDispArg[0] = pstContext->mG2dInfo.iDstIdx;
    ioctl(pstContext->iDispFd, DISP_MEM_RELEASE, (void *)dwaDispArg);
    munmap(pstContext->mG2dInfo.pSrcVirAddr, dwSrcSize);
	dwaDispArg[0] = pstContext->mG2dInfo.iSrcIdx;
    ioctl(pstContext->iDispFd, DISP_MEM_RELEASE, (void *)dwaDispArg);
    close(pstContext->iDispFd);
    close(pstContext->iG2dFd);
}

static inline int DoG2dBltModeProc(SampleG2dContext *pstContext)
{
    int iRet = 0;
    g2d_blt *pstBlt = &pstContext->mG2dInfo.g2d_mode.blt;

    pstBlt->flag = G2D_BLT_ROTATE90;
    pstBlt->color = 0xee8899;
    pstBlt->alpha = 0x36;
    pstBlt->src_image.addr[0] = (unsigned int)pstContext->mG2dInfo.pSrcPhyAddr;
    pstBlt->src_image.addr[1] = (unsigned int)pstContext->mG2dInfo.pSrcPhyAddr + pstContext->iSrcWidth * pstContext->iSrcHeight;
    pstBlt->src_image.w = pstContext->iSrcWidth;
    pstBlt->src_image.h = pstContext->iSrcHeight;
    pstBlt->src_image.format = G2D_FMT_PYUV420UVC;
    pstBlt->src_image.pixel_seq = G2D_SEQ_VUVU;
    pstBlt->src_rect.x = pstContext->iSrcRectX;
    pstBlt->src_rect.y = pstContext->iSrcRectY;
    pstBlt->src_rect.w = pstContext->iSrcRectW;
    pstBlt->src_rect.h = pstContext->iSrcRectH;

    pstBlt->dst_image.addr[0] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr;
    pstBlt->dst_image.addr[1] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr + pstContext->iDstWidth * pstContext->iDstHeight;
    pstBlt->dst_image.w = pstContext->iDstWidth;
    pstBlt->dst_image.h = pstContext->iDstHeight;
    pstBlt->dst_image.format = G2D_FMT_PYUV420UVC;
    pstBlt->dst_image.pixel_seq = G2D_SEQ_VUVU;
    pstBlt->dst_x = pstContext->iDstRectX;
    pstBlt->dst_y = pstContext->iDstRectY;
#if 1
    iRet = ioctl(pstContext->iG2dFd, G2D_CMD_BITBLT, pstBlt);
    if (iRet < 0) {
        aloge("do G2D_CMD_BITBLT failed!!");
        goto blt_err;
    }
#endif
blt_err:
    return iRet;
}

static inline int DoG2dStchBltModeProc(SampleG2dContext *pstContext)
{
    int iRet = 0;
    g2d_stretchblt *pstStchBlt = &pstContext->mG2dInfo.g2d_mode.stchblt;

    pstStchBlt->flag = G2D_BLT_NONE;
    pstStchBlt->color = 0xee8899;
    pstStchBlt->alpha = 0x73;
    pstStchBlt->src_image.addr[0] = (unsigned int)pstContext->mG2dInfo.pSrcPhyAddr;
    pstStchBlt->src_image.addr[1] = (unsigned int)pstContext->mG2dInfo.pSrcPhyAddr + pstContext->iSrcWidth * pstContext->iSrcHeight;
    pstStchBlt->src_image.format = G2D_FMT_PYUV420UVC;
    pstStchBlt->src_image.w = pstContext->iSrcWidth;
    pstStchBlt->src_image.h = pstContext->iSrcHeight;
    pstStchBlt->src_image.pixel_seq = G2D_SEQ_VUVU;
    pstStchBlt->src_rect.x = pstContext->iSrcRectX;
    pstStchBlt->src_rect.y = pstContext->iSrcRectY;
    pstStchBlt->src_rect.w = pstContext->iSrcRectW;
    pstStchBlt->src_rect.h = pstContext->iSrcRectH;

    pstStchBlt->dst_image.addr[0] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr;
    pstStchBlt->dst_image.addr[1] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr + pstContext->iDstWidth * pstContext->iDstHeight;
    pstStchBlt->dst_image.format = G2D_FMT_PYUV420UVC;
    pstStchBlt->dst_image.w = pstContext->iDstWidth;
    pstStchBlt->dst_image.h = pstContext->iDstHeight;
    pstStchBlt->dst_image.pixel_seq = G2D_SEQ_VUVU;
    pstStchBlt->dst_rect.x = pstContext->iDstRectX;
    pstStchBlt->dst_rect.y = pstContext->iDstRectY;
    pstStchBlt->dst_rect.w = pstContext->iDstRectW;
    pstStchBlt->dst_rect.h = pstContext->iDstRectH;
#if 1
    iRet = ioctl(pstContext->iG2dFd, G2D_CMD_STRETCHBLT, pstStchBlt);
    if (iRet < 0) {
        aloge("do G2D_CMD_STRETCHBLT failed!!");
        goto stchblt_err;
    }
#endif
stchblt_err:
    return iRet;
}

static inline int DoG2dFillRectModeProc(SampleG2dContext *pstContext)
{
    int iRet = 0;
    g2d_fillrect *pstFillRect = &pstContext->mG2dInfo.g2d_mode.fill_rect;

    pstFillRect->flag = G2D_BLT_PLANE_ALPHA;
    pstFillRect->color = 0xFF003500;
    pstFillRect->alpha = 0x40;

    pstFillRect->dst_image.addr[0] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr;
    pstFillRect->dst_image.addr[1] = (unsigned int)pstContext->mG2dInfo.pDstPhyAddr + pstContext->iDstWidth * pstContext->iDstHeight;
    pstFillRect->dst_image.format = G2D_FMT_PYUV420UVC;
    pstFillRect->dst_image.w = pstContext->iDstWidth;
    pstFillRect->dst_image.h = pstContext->iDstHeight;
    pstFillRect->dst_image.pixel_seq = G2D_SEQ_VUVU;
    pstFillRect->dst_rect.x = pstContext->iDstRectX;
    pstFillRect->dst_rect.y = pstContext->iDstRectY;
    pstFillRect->dst_rect.w = pstContext->iDstRectW;
    pstFillRect->dst_rect.h = pstContext->iDstRectH;
#if 1
    iRet = ioctl(pstContext->iG2dFd, G2D_CMD_FILLRECT, pstFillRect);
    if (iRet < 0) {
        aloge("do G2D_CMD_STRETCHBLT failed!!");
        goto stchblt_err;
    }
#endif
stchblt_err:
    return iRet;
}

static inline int DoG2dProcess(SampleG2dContext *pstContext)
{
    int iRet = 0;

    switch (pstContext->iG2dMode) {
        case SAMPLE_G2D_BLT_MODE: {
            iRet = DoG2dBltModeProc(pstContext);
        } break;
        case SAMPLE_G2D_FILLRECT_MODE: {
            iRet = DoG2dFillRectModeProc(pstContext);
        } break;
        case SAMPLE_G2D_STCHBLT_MODE: {
            iRet = DoG2dStchBltModeProc(pstContext);
        } break;
        case SAMPLE_G2D_BLTH_MODE: {

        } break;
        case SAMPLE_G2D_BLDH_MODE: {

        } break;
        case SAMPLE_G2D_MASK_MODE: {

        } break;
        default: {
            aloge("unknow G2D mode!!");
            return -1;
        } break;
    }

    if (iRet < 0) {
        return iRet;
    }

    FILE *pOutFp;
    unsigned long dwOutSize = pstContext->iDstWidth * pstContext->iDstHeight * 3 / 2;

    pOutFp = fopen(pstContext->cstrOutPath, "wb+");
    if (pOutFp == NULL) {
        aloge("open source image (%s) failed!!", pstContext->cstrOutPath);
        iRet = -1;
        goto open_out_err;
    }
    fwrite(pstContext->mG2dInfo.pDstVirAddr, dwOutSize, 1, pOutFp);
    fclose(pOutFp);

open_out_err:
    return iRet;
}

int main(int argc, char **argv)
{
    int iRet;

    alogd("sample_g2d running!\n");
    SampleG2dContext stContext;
    memset(&stContext, 0, sizeof(SampleG2dContext));

    iRet = ParseCmdLine(argc, argv, &stContext);
    if (iRet < 0) {
        return -1;
    }

    /* parse config file. */
    if(LoadSampleG2dConfig(&stContext , stContext.mCmdLinePara.mConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        iRet = -1;
        goto load_conf_err;
    }

    iRet = InitG2dDevice(&stContext);
    if (iRet < 0) {
        aloge("InitG2dDevice failed!!");
        goto init_dev_err;
    }
#if 1
    iRet = DoG2dProcess(&stContext);
    if (iRet < 0) {
        aloge("DoG2dProcess failed!!");
        goto g2d_proc_err;
    }
#endif
g2d_proc_err:
    DestroyD2dDevice(&stContext);
init_dev_err:
load_conf_err:
    if (iRet == 0) {
        alogd("sample_g2d exit!\n");
    }
    return iRet;
}

