#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sine_detector.h"

/*********************noise reduction estimate*****************************/
int NF_init(nf_ref_t *nf, int sampleRate, float freq, float threshold)
{
	//** input:
	//** nf : pointer of the struct  ....read file -- sine_detector.h
	//** sampleRate : frequency rate  .... 48000 44100 32000 16000 8000...
	//** freq : frequecy of the sine signal
	//** threshold: signal to noise ratio
	//**
	//** output
	//** 1 : memory is set
	//** 0 : else

	if (!nf)
		return 0;

//	memset(nf, 0, sizeof(nf_ref_t));

	nf->sampleRate = sampleRate;
	nf->freq = freq;
	nf->sin_flag = 1;
	nf->var_flag = 1;
	nf->xbuf[0] = 0;
	nf->xbuf[1] = 0;
	nf->ybuf[0] = 0;
	nf->ybuf[1] = 0;
	nf->rr = 0.999;
	nf->aa = cos(2*PI*freq/sampleRate);
	nf->xPow = 0;
	nf->yPow = 0;
	nf->eerNum = 1;
	nf->startTime = (20*sampleRate/1000);
	nf->frameNum = 0;
	nf->threshold = threshold;

	return 1;
}

/*************************************************/
int NF_end(nf_ref_t *nf)
{
	//** input:
	//** nf : pointer of the struct  ....read file -- sine_detector.h
	//**
	//** output
	//** nf->sin_flag : result of the sine signal detection
	float end, tail;

	tail = (float)((float)nf->frameNum - (float)nf->eerNum) / (float)nf->sampleRate;
	end  = 0.1; //100ms

	if(nf->sin_flag == 0)
		if (tail < end)
			nf->sin_flag = 1;

	return nf->sin_flag;
}

/**********************************************************************/
int notch_filter(nf_ref_t *nf, short *buffer, short *output, int DataNum)
{
	//** input:
	//** nf : pointer of the struct  ....read file -- sine_detector.h
	//** buffer : input buffer
	//** output : output buffer
	//** DataNum : length of the buffer
	//**
	//** output
	//** nf->sin_flag : result of the sine signal detection   1:sine signal ; 0 : no a sine signal
	//**
	int i;
	float tmp1 = 0;
	float tmp2 = 0;
	float tmp3 = 0;

	for (i=0; i<DataNum; i++) {
		tmp1 = (float)buffer[i] - 2*(nf->aa)*(nf->xbuf[0]) + nf->xbuf[1];
		tmp2 = 2*(nf->rr)*(nf->aa)*(nf->ybuf[0]) - (nf->rr)*(nf->rr)*(nf->ybuf[0]);

		tmp3 = tmp1 + tmp2;

		output[i] = (short)SATURATE(tmp3, MAX_16);

		nf->xbuf[1] = nf->xbuf[0];
		nf->xbuf[0] = buffer[i];
		nf->ybuf[1] = nf->ybuf[0];
		nf->ybuf[0] = output[i];

		nf->frameNum += 1;

		nf->xPow = ALPHA*nf->xPow + (1-ALPHA)*buffer[i]*buffer[i];
		nf->yPow = ALPHA*nf->yPow + (1-ALPHA)*output[i]*output[i];

		tmp1 = nf->xPow / (nf->yPow + 1);

		tmp2 = 10*log10(tmp1);

		if ((nf->frameNum) > (nf->startTime)) {
			if (tmp2 < nf->threshold) {
				nf->sin_flag = 0;

				if (nf->var_flag) {
					nf->eerNum = nf->frameNum;
					nf->var_flag = 0;
				}
			}
		}
	}
	return nf->sin_flag;
}
