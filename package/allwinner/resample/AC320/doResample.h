#ifndef DORESAMPLE_H
#define DORESAMPLE_H

#include "aumixcom.h"
#define TEMP_RESAMPLE_BUFFER_SIZE (256*1024)

typedef struct _Resampler Resampler;
typedef struct _ResCfg    ResCfg;

struct _Resampler
{
	AudioMix  AMX;
	void*     tempResampleBuffer;
	int       (*prepare)(Resampler *, ResCfg*);
	int       (*process)(Resampler *);
	int       (*getData)(Resampler *, void*, unsigned int);
};

struct _ResCfg
{
	int       insrt;
	int       inch;

	int       outsrt;

	char*     inbuf;
	int       samples;
};

#ifdef __cplusplus
extern"C"{
#endif
Resampler* Creat_Resampler();
int Destroy_Resampler(Resampler * res);
#ifdef __cplusplus
}
#endif
#endif
