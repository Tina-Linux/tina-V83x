#ifndef _SAMPLE_G2D_H_
#define _SAMPLE_G2D_H_

#include "linux/g2d_driver.h"

#define MAX_FILE_PATH_SIZE (256)

#define SAMPLE_G2D_MODE            "g2d_mode"
#define SAMPLE_G2D_SRC_IMG         "src_img"
#define SAMPLE_G2D_SRC_IMG_W       "src_img_w"
#define SAMPLE_G2D_SRC_IMG_H       "src_img_h"
#define SAMPLE_G2D_SRC_RECT_X      "src_rect_x"
#define SAMPLE_G2D_SRC_RECT_Y      "src_rect_y"
#define SAMPLE_G2D_SRC_RECT_W      "src_rect_w"
#define SAMPLE_G2D_SRC_RECT_H      "src_rect_h"

#define SAMPLE_G2D_DST_IMG         "dst_img"
#define SAMPLE_G2D_DST_IMG_W       "dst_img_w"
#define SAMPLE_G2D_DST_IMG_H       "dst_img_h"
#define SAMPLE_G2D_DST_RECT_X      "dst_rect_x"
#define SAMPLE_G2D_DST_RECT_Y      "dst_rect_y"
#define SAMPLE_G2D_DST_RECT_W      "dst_rect_w"
#define SAMPLE_G2D_DST_RECT_H      "dst_rect_h"

#define SAMPLE_G2D_OUTPUT          "output"

#define SAMPLE_G2D_BLT_MODE 0
#define SAMPLE_G2D_FILLRECT_MODE 1
#define SAMPLE_G2D_STCHBLT_MODE 2
#define SAMPLE_G2D_BLTH_MODE 4
#define SAMPLE_G2D_BLDH_MODE 5
#define SAMPLE_G2D_MASK_MODE 6

typedef struct SampleCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_SIZE];

} SampleCmdLineParam;

typedef struct SampleG2dInfo {
    union {
        g2d_blt blt;
        g2d_fillrect fill_rect;
        g2d_stretchblt stchblt;
//        g2d_blt_h blt_h;
//        g2d_bld bld;
        g2d_maskblt mask;
    } g2d_mode;
    int iSrcIdx;
    void *pSrcPhyAddr;
    void *pSrcVirAddr;
    int iDstIdx;
    void *pDstPhyAddr;
    void *pDstVirAddr;
} SampleG2dInfo;

typedef struct SampleG2dContext
{
    int iG2dFd;
    int iG2dMode;
    int iG2dFlags;

    int iDispFd; // for memory alloc

    char cstrSrcPath[MAX_FILE_PATH_SIZE];
    int iSrcWidth;
    int iSrcHeight;
    int iSrcRectX;
    int iSrcRectY;
    int iSrcRectW;
    int iSrcRectH;
    char cstrDstPath[MAX_FILE_PATH_SIZE];
    int iDstWidth;
    int iDstHeight;
    int iDstRectX;
    int iDstRectY;
    int iDstRectW;
    int iDstRectH;

    char cstrOutPath[MAX_FILE_PATH_SIZE];

    SampleCmdLineParam mCmdLinePara;

    SampleG2dInfo mG2dInfo;
} SampleG2dContext;

#endif  /* _SAMPLE_G2D_H_ */

