#ifndef DEINTERLACE_H
#define DEINTERLACE_H

#include <cdx_config.h>
#include <vdecoder.h>

enum DE_INTERLACE_FLAG
{
	DE_INTERLACE_NONE,
	DE_INTERLACE_HW,
	DE_INTERLACE_SW
};

class Deinterlace
{
public:
//    Deinterlace(){};

    virtual ~Deinterlace(){};

    virtual int init() = 0;

    virtual int reset() = 0;

    virtual EPIXELFORMAT expectPixelFormat() = 0;

    virtual int flag() = 0;

    virtual int process(VideoPicture *pPrePicture,
                        VideoPicture *pCurPicture,
                        VideoPicture *pOutPicture,
                        int nField) = 0;

};

Deinterlace *DeinterlaceCreate();

#endif
