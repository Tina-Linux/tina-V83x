#ifndef DORESAMPLE_H
#define DORESAMPLE_H

#include "aumixcom.h"

typedef struct _Resampler Resampler;
typedef struct _ResCfg    ResCfg;

struct _Resampler
{
    AudioMix  AMX;
    void*     tempResampleBuffer;
    int       (*prepare)(Resampler *, ResCfg*);
    int       (*process)(Resampler *);
};

struct _ResCfg
{
    int   insrt;
    int   inch;
    int   outsrt;
    char* inbuf;
    char* outbuf;
    int   samples;
};


Resampler* Create_Resampler();
int Destroy_Resampler(Resampler * res);

#endif
