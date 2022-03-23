#ifndef AUMIXCOMMON
#define AUMIXCOMMON

typedef struct __PcmInfo__{
//input infor
    unsigned int SampleRate;
    short*       PCMPtr;
    unsigned int PcmLen;//sample num;
    unsigned int Chan;
    int          preamp;//-20 -- 20 db
}PcmInfo;

typedef struct ___AudioMix__{
//input para
    PcmInfo    *InputA;
    PcmInfo    *InputB;
    PcmInfo    *TempBuf;
    PcmInfo    *Output;
    unsigned int    ByPassflag; //0: need mix A+B ==¡·A  1£ºbypass   A==>A
//output para
    short*    MixbufPtr;
    unsigned int    MixLen;
    unsigned int    Mixch;
    void*           RESI;
}AudioMix;
int   do_AuMIX(AudioMix    *AMX);
void* Init_ResampleInfo();
void  Destroy_ResampleInfo(void * ri);

#endif
