#ifndef DO_AUDIORESAMPLE_H
#define DO_AUDIORESAMPLE_H
#include "aumixcom.h"

void init_audioresample(AudioMix** AMX);
int do_audioresample(AudioMix* AMX, int insrt,int inch,const char* inbuf,const int buflen,int outsrt,char *outbuf,int *outbuflen);
void destroy_audioresample(AudioMix** AMX);
#endif
