
#ifndef _sunxi_edid_h_
#define _sunxi_edid_h_

#include "../sunxi_display2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * SunxiEdidHandle;

SunxiEdidHandle SunxiEdidLoad(const char *path);
void SunxiEdidReload(SunxiEdidHandle handle);
void SunxiEdidUnload(SunxiEdidHandle handle);
const char *    SunxiEdidGetMonitorName(SunxiEdidHandle handle);
unsigned short SunxiEdidGetDispSizeWidth(SunxiEdidHandle handle);
unsigned short SunxiEdidGetDispSizeHeight(SunxiEdidHandle handle);
int SunxiEdidGetSinkType(SunxiEdidHandle handle);
int SunxiEdidSupportedRGBOnly(SunxiEdidHandle handle);
bool SunxiEdidIsSupportY420Sampling(SunxiEdidHandle handle, int mode);
bool SunxiEdidIsSupportRegularSampling(SunxiEdidHandle handle, int mode);
int * SunxiEdidGetSupportedVideoFormat(SunxiEdidHandle handle);
int SunxiEdidGetMaxAudioPcmChs(SunxiEdidHandle handle);
int SunxiEdidGetPreferredVideoFormat(SunxiEdidHandle handle);

#ifdef __cplusplus
}
#endif


#endif
