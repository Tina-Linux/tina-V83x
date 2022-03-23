
#ifndef _VIDEO_DECODER_H
#define _VIDEO_DECODER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vdecoder.h"
#include "memoryAdapter.h"

typedef struct _decoder
{
    int enable;
	int index;
	struct ScMemOpsS* memops;
	VideoDecoder *pVideo;
	VideoPicture *videoPicture;
	char *pCodecSpecificData;
	int nCodecSpecificDataLen;
	int Input_Fmt;
	int Output_Fmt;
	int ratio;
	unsigned char *Vir_Y_Addr;
	unsigned char *Vir_U_Addr;
	unsigned char *Phy_Y_Addr;
	unsigned char *Phy_C_Addr;
	unsigned int mWidth;
	unsigned int mHeight;
	int mYuvSize;
	int64_t nPts;
	int bufId;
}decoder_handle;

int DecoderInit(void *decoder);
int DecoderDequeue(void *decoder, unsigned char *start, int64_t pts, int length);
int DecoderaEnqueue(void *decoder, VideoPicture *videoPicture);
int DecoderRelease(void *decoder);
#endif
