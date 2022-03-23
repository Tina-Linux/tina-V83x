#ifndef OMX_CODEC_H_
#define OMX_CODEC_H_

#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "sem.h"
#include "async_queue.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void* OmxCodecCreate(char* component_name, char* component_role);

void OmxCodecDestroy(void* omx_codec);

int OmxCodecConfigure(void* omx_codec, OMX_BOOL isEncoder, OMX_U32 nFrameWidth, OMX_U32 nFrameHeight,
                             OMX_COLOR_FORMATTYPE color_format);

void OmxCodecStart(void* omx_codec);

void OmxCodecStop(void* omx_codec);

OMX_BUFFERHEADERTYPE* dequeneInputBuffer(void* omx_codec);

int queneInputBuffer(void* omx_codec, OMX_BUFFERHEADERTYPE* pBuffer);

OMX_BUFFERHEADERTYPE* dequeneOutputBuffer(void* omx_codec);

int queneOutputBuffer(void* omx_codec, OMX_BUFFERHEADERTYPE* pBuffer);

//void* convertAddressVir2Phy(void* omx_codec, void* pAddress);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //OMX_CODEC_H_
