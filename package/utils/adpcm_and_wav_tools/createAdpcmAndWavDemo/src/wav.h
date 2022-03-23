#ifndef __WAV_H
#define __WAV_H
typedef struct wavObject{//8 bytes
    int id;
    int size;
}wavObjectS;

typedef struct RIFFChunk{//12 bytes
    wavObjectS head;
    int   type;
}RIFFChunkS;

typedef struct FormatChunk{//24 bytes
    wavObjectS head;
    short int formatType;
    short int channel;
    int sampleRate;
    int avgBytePerSec;
    short int blockAlign;
    short int bitPerSample;
}FormatChunkS;

typedef struct WavHeadObject{
    RIFFChunkS riffChunk;
    FormatChunkS formatChunk;
    wavObjectS dataHead;
}WavHeadObjectS;

typedef enum FORMAT_TYPE{
    PCM_TYPE = 1,
    ADPCM_TYPE =2,
}FORMAT_TYPE;

#ifdef  __cplusplus
extern "C" {
#endif
    int ParseWavFile(FILE* inputFp,WavHeadObjectS* wavHeadP);
    int MuxWavHead(FILE* outputFp,int channel,int sampleRate,int bitPerSample,FORMAT_TYPE formatType,int totalDataLen);
    int MuxWavData(unsigned char* inputData,int inputDataLen,FILE* outputFp);
#ifdef  __cplusplus
}
#endif

#endif
