#ifndef DEINTERLACE_HW_H
#define DEINTERLACE_HW_H

#include <deinterlace.h>

typedef enum
{
	DI_FORMAT_NV12      =0x00,
	DI_FORMAT_NV21      =0x01,
	DI_FORMAT_MB32_12   =0x02, //UV mapping like NV12
	DI_FORMAT_MB32_21   =0x03, //UV mapping like NV21

}DiPixelformatE;

typedef struct
{
	unsigned int nWidth;
	unsigned int nHeight;
}DiRectSizeT;

typedef struct
{
#if (DEINTERLACE_ADDR_64 == 1)
	unsigned long long         addr[2];  // the address of frame buffer
#else
	uintptr_t         addr[2];  // the address of frame buffer
#endif
	DiRectSizeT       mRectSize;
	DiPixelformatE    eFormat;
}DiFbT;

typedef struct
{
	DiFbT         mInputFb;	       //current frame fb
	DiFbT         mPreFb;          //previous frame fb
	DiRectSizeT   mSourceRegion;   //current frame and previous frame process region
	DiFbT         mOutputFb;       //output frame fb
	DiRectSizeT   mOutRegion;	   //output frame region
	unsigned int  nField;          //process field <0-top field ; 1-bottom field>
	unsigned int  bTopFieldFirst;  //video infomation <0-is not top_field_first; 1-is top_field_first>
}DiParaT;

class DeinterlaceHw : public Deinterlace
{
private:
    int fd;
    int64_t picCount;

public:
    DeinterlaceHw();

    ~DeinterlaceHw();

    int init();

    int reset();

    EPIXELFORMAT expectPixelFormat();

    int flag();

    int para(DiParaT *para);

    int dumpPara(DiParaT *para);

    int process(VideoPicture *pPrePicture,
                VideoPicture *pCurPicture,
                VideoPicture *pOutPicture,
                int nField);

};

#endif
