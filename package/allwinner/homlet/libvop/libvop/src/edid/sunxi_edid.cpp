
#include "edid.h"
#include "sunxi_edid.h"
#include "cea_vic.h"


SunxiEdidHandle SunxiEdidLoad(const char *path)
{
    EdidParser *edid = new EdidParser(path);
    edid->reload();
    edid->getVideoFormatSupported();
    return edid;
}

void SunxiEdidReload(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    edid->reload();
    edid->getVideoFormatSupported();
}

void SunxiEdidUnload(SunxiEdidHandle handle)
{
    delete (EdidParser *)handle;
}

const char * SunxiEdidGetMonitorName(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mMonitorName;
}

unsigned short SunxiEdidGetDispSizeWidth(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mDisplaySizeH * 10;
}

unsigned short SunxiEdidGetDispSizeHeight(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mDisplaySizeV * 10;
}

int SunxiEdidGetSinkType(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mSinkType;
}

int SunxiEdidSupportedRGBOnly(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mRGBOnly;
}

bool SunxiEdidIsSupportY420Sampling(SunxiEdidHandle handle, int mode)
{
    EdidParser *edid = (EdidParser *)handle;
    int i;

    int vic = mode;
    for (i = 0; i < edid->mSupportedVIC.size(); i++) {
        if ((edid->mSupportedVIC[i]->vic == vic) &&
                edid->mSupportedVIC[i]->ycbcr420_sampling)
            return true;
    }
    for (i = 0; i < edid->mY420VIC.size(); i++) {
        if (edid->mY420VIC[i]->vic == vic)
            return true;
    }
    return false;
}

bool SunxiEdidIsSupportRegularSampling(SunxiEdidHandle handle, int mode)
{
    EdidParser *edid = (EdidParser *)handle;
    int i;

    int vic = mode;
    for (i = 0; i < edid->mSupportedVIC.size(); i++) {
        if ((edid->mSupportedVIC[i]->vic == vic) &&
                edid->mSupportedVIC[i]->regular_sampling)
            return true;
    }
    for (i = 0; i < edid->mY420VIC.size(); i++) {
        if (edid->mY420VIC[i]->regular_sampling)
            return true;
    }
    return false;
}


int * SunxiEdidGetSupportedVideoFormat(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->VideoFormatSupported;
}

int SunxiEdidGetMaxAudioPcmChs(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mMaxAudioPcmChs;
}

int SunxiEdidGetPreferredVideoFormat(SunxiEdidHandle handle)
{
    EdidParser *edid = (EdidParser *)handle;
    return edid->mPreferredVideoFormat;
}



