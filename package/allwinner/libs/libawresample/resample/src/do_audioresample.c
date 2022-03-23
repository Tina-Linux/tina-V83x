#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<assert.h>
#include <stdlib.h>

#include "do_audioresample.h"

int do_audioresample(AudioMix* AMX, int insrt,int inch,const char* inbuf,const int buflen,int outsrt,char *outbuf,int *outbuflen)
{
	PcmInfo AmxInputA;
	PcmInfo AmxInputB;
	PcmInfo AmxOutput;

	int templen = 0,temptotallen = 0,outlen = 0;
	//short   pTempResampleBuffer[64*1024] = {0};

	AMX->InputB = &AmxInputB;
	AMX->InputA = &AmxInputA;
	AMX->Output = &AmxOutput;
	temptotallen = templen = 0;

	AMX->InputB->SampleRate = insrt;
	AMX->InputB->Chan       = inch;
	AMX->InputB->PCMPtr     = (short*)inbuf;
	AMX->InputB->PcmLen     = buflen / 2 / inch;
	AMX->InputA->PCMPtr = (short*)outbuf;//pTempResampleBuffer;//(pAudioDecoder->ADCedarCtx.pTempResampleBuffer + (inch==1?TEMP_RESAMPLE_BUFFER_SIZE/2:0));
	AMX->InputA->SampleRate = outsrt;
	AMX->InputA->Chan = AMX->InputB->Chan;
	AMX->InputA->PcmLen = (outsrt * AMX->InputB->PcmLen)/insrt + 1;
	AMX->ByPassflag = 2;
	while(AMX->InputB->PcmLen)
	{
		do_AuMIX(AMX);
		templen = AMX->MixLen * 2 * AMX->Mixch;
		temptotallen += templen;
		AMX->InputA->PCMPtr += AMX->MixLen * AMX->Mixch;
	}
	//memcpy(inbuf,pTempResampleBuffer,temptotallen);
	*outbuflen = temptotallen;
	return 0;

}

void init_audioresample(AudioMix** AMX)
{
    *AMX = (AudioMix*)malloc(sizeof(AudioMix));
    memset(*AMX, 0x00, sizeof(AudioMix));
    (*AMX)->RESI = Init_ResampleInfo();
}

void destroy_audioresample(AudioMix** AMX)
{
    Destroy_ResampleInfo((*AMX)->RESI);
    free(*AMX);
    *AMX = NULL;
}