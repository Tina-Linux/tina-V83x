/*
 * phraseSpotR16.c
 * yanchen.lu@gmems.com
 *
 */
#define __TUTUCLEAR_20171019__

#define THROW(a) { ewhere=(a); goto error; }
#define THROW2(a,b) {ewhere=(a); ewhat=(b); goto error; }

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <trulyhandsfree.h>
#include <sys/types.h>
#include <unistd.h>
#include "console.h"
#include "audio.h"
#include "datalocations.h"
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "asoundlib.h"

#ifdef __TUTUCLEAR_20171019__
#   include "tutuClear.h"
#   include "tutu_tool.h"
#   define NO_MIC_INPUT             8 // cannot be 6
#   define FRAMESZ_4_WAKEUP         160 // in sample, should be integer multiply of FRAMESZ_4_TUTU
#   define FRAMESZ_4_TUTU           160 // in sample
#   define TUTU_PRM_FNAME           "tutuClearA1_ns4wakeup.prm"
#endif // __TUTUCLEAR_20171019__

#define HBG_NETFILE                 "sensory_demo_hbg_en_us_sfs_delivery04_pruned_search_am.raw"
#define HBG_SEARCHFILE              "sensory_demo_hbg_en_us_sfs_delivery04_pruned_search_9.raw" // pre-built search
#define HBG_OUTFILE                 "myhbg.raw"             /* hbg output search */
#define NBEST                       (1)                     /* Number of results */
#define PHRASESPOT_DELAY            PHRASESPOT_DELAY_ASAP  /* Phrasespotting Delay */
#define MAXSTR                      (512)                   /* Output string size */

// ----------------------------------------------------------------------------
long long   allProfile[64]; // dynamic lib needs it
clock_t     tClkR; // dynamic lib needs it
int         should_stop = 0;

// ----------------------------------------------------------------------------
static void when_signal(int sig)
{
    switch(sig)
    {
    case SIGINT:
    {
        printf("SIGINT coming, stop the capture\n");
        should_stop = 1;
        break;
    }
    case SIGQUIT:
    {
        printf("SIGQUIT coming, stop the capture\n");
        break;
    }
    case SIGHUP:
    {
        printf("SIGHUP coming, stop the capture\n");
        should_stop = 1;
        break;
    }
    case SIGPIPE:
    {
        //When the client is closed after start scaning and parsing,
        //this signal will come, ignore it!
        printf("do nothings for PIPE signal\n");
        break;
    }
    }
}

// ----------------------------------------------------------------------------
char *formatExpirationDate(time_t expiration)
{
    static char expdate[33];
    if (!expiration) return "never";
    strftime(expdate, 32, "%m/%d/%Y 00:00:00 GMT", gmtime(&expiration));
    return expdate;
}

// ----------------------------------------------------------------------------
void TUTUClear_ParsePRMFile_QACT(const char         *pszCtrlFileName,
                                 TUTUClearConfig_t   *ptTUTUClearConfig,
                                 TUTUClearParam_t    *ptTUTUClearParam);

// ----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // wakeup IP variables ----------------------------------------------------
    const char              *rres;
    thf_t                   *ses=NULL;
    recog_t                 *r=NULL;
    searchs_t               *s=NULL;
    pronuns_t               *pp=NULL;
    char                    str[MAXSTR];
    const char              *ewhere, *ewhat=NULL;
    float                   score, delay;
    unsigned short          status, done=0;
    audiol_t                *p;
    unsigned short          i=0;
    int                     k, l;
    void                    *cons=NULL;
    FILE                    *fp=NULL;

    // audio driver variables -------------------------------------------------
    struct pcm_config       config;
    struct pcm              *pcm;
    unsigned int            uw32FrameSzInSampleForWakeUp = FRAMESZ_4_WAKEUP;
    char                    *pw8CapBuf;

#ifdef __TUTUCLEAR_20171019__
    void                    *pTUTUClearObject = NULL;
    void                    *pExternallyAllocatedMem = NULL;
    TUTUClearConfig_t       tTUTUClearConfig;
    TUTUClearParam_t        tTUTUClearParam;
    TUTUClearStat_t         tTUTUClearStat;

    int                     iMemSzInByte;
    short                   w16MICSelection0 = 0x0324;
    short                   w16MICSelection1 = 0x1576;
    short                   aw16AECRef[FRAMESZ_4_TUTU*2];
    short                   aw16MicSig[FRAMESZ_4_TUTU*NO_MIC_INPUT]; // up to 8 channels
    short                   aw16LOut[FRAMESZ_4_WAKEUP];
    FILE                    *pfLout = NULL;
    int                     iSzOfSmpl = sizeof(int);
#endif // __TUTUCLEAR_20171019__

    /* Draw console */
    if (!(cons=initConsole(NULL))) return 1;
    printf("iSzOfSmpl = %d\n", iSzOfSmpl);
    for (i = 0; i < argc; ++i ) printf( "argv[ %d ] = %s\n", i, argv[ i ] );

#ifdef __TUTUCLEAR_20171019__
    // read configuration
    tTUTUClearConfig.uw32Version = TUTUCLEAR_VERSION;
    printf("Parsing %s\n", TUTU_PRM_FNAME);
    TUTUClear_ParsePRMFile_QACT(TUTU_PRM_FNAME,
                                &tTUTUClearConfig,
                                &tTUTUClearParam);

    printf("uw16FrameSz = %d\n", (int)tTUTUClearConfig.uw16FrameSz);
    printf("uw16MaxNumOfMic = %d\n", (int)tTUTUClearConfig.uw16MaxNumOfMic);
    printf("uw16MaxTailLength = %d\n", (int)tTUTUClearConfig.uw16MaxTailLength);
    printf("uw16SamplingFreq = %d\n", (int)tTUTUClearConfig.uw16SamplingFreq);
    w16MICSelection0 = tTUTUClearParam.uw16Resv2;
    printf("w16MICSelection0 = 0x%04X\n", w16MICSelection0);
    w16MICSelection1 = tTUTUClearParam.uw16Resv3;
    printf("w16MICSelection1 = 0x%04X\n", w16MICSelection1);

    if (tTUTUClearConfig.uw16SamplingFreq*tTUTUClearConfig.uw16FrameSz > FRAMESZ_4_TUTU*1000)
    {
        printf("Illegal tTUTUClearConfig.\n");
        return 1;
    }

    // memeory allocation
    iMemSzInByte = TUTUClear_QueryMemSz(&tTUTUClearConfig);
    printf("tutuClear DM usage = %d bytes\n", iMemSzInByte);
    pExternallyAllocatedMem = malloc(iMemSzInByte);

    // version check
    {
        W8 cMajor, cMinor, cRevision;
        UW32 uw32Config = TUTUClear_GetVerNum(&cMajor, &cMinor, &cRevision);
        fprintf(stderr,"Software voice processor compiled on: " __DATE__ " " __TIME__ "\n");
        fprintf(stderr,"TUTUCLEAR Ver. %d.%d.%d Inside (0x%08x).\n", cMajor, cMinor, cRevision, uw32Config);
        fprintf(stderr,"Copyright (C) 2017, Spectimbre Inc.\n");
    }

    // init
    printf("uw32OpMode = %08X\n", tTUTUClearParam.uw32OpMode);
    printf("uw32FuncMode = %08X\n", tTUTUClearParam.uw32FuncMode);
    printf("uw16NumOfMic = %d\n", tTUTUClearParam.uw16NumOfMic);
    printf("uw16ECTailLengthInMs = %d\n", tTUTUClearParam.uw16ECTailLengthInMs);
    if ((TUTUClear_Init(&tTUTUClearConfig,
                        pExternallyAllocatedMem,
                        &pTUTUClearObject)) != TUTU_OK)
    {
        printf("Fail to do TUTUClear_Init.\n");
        return 1;
    }
    else
        printf("TUTUClear_Init okay.\n");

    // set parameter
    if ((TUTUClear_SetParams(pTUTUClearObject, &tTUTUClearParam)) != TUTU_OK)
    {
        printf("Fail to do TUTUClear_SetParams.\n");
        return 1;
    }
    else
        printf("TUTUClear_SetParams okay.\n");

    // reset AEC reference buffer
    memset(aw16AECRef, 0, FRAMESZ_4_TUTU*sizeof(short));

#endif // __TUTUCLEAR_20171019__

    // begin of wakeup IP init ------------------------------------------------
    if(!(ses=thfSessionCreate()))
    {
        panic(cons, "thfSessionCreate", thfGetLastError(NULL), 0);
        return 1;
    }

    /* Display TrulyHandsfree SDK library version number */
    dispv(cons, "TrulyHandsfree SDK Library version: %s\n", thfVersion());
    dispv(cons, "Expiration date: %s\n\n", formatExpirationDate(thfGetLicenseExpiration()));

    /* Create recognizer */
    /* NOTE: HBG_NETFILE is a custom model for 'hello blue genie'. Use a generic acoustic model if you change vocabulary or contact Sensory for assistance in developing a custom acoustic model specific to your vocabulary. */
    disp(cons,"Loading recognizer: "  HBG_NETFILE);
    if(!(r=thfRecogCreateFromFile(ses, HBG_NETFILE,(unsigned short)(AUDIO_BUFFERSZ/1000.f*SAMPLERATE),-1,NO_SDET)))
        THROW("thfRecogCreateFromFile");

    disp(cons,"Loading custom HBG search: " HBG_SEARCHFILE);
    if(!(s=thfSearchCreateFromFile(ses,r, HBG_SEARCHFILE,NBEST)))
        THROW("thfSearchCreateFromFile");

    /* Configure parameters - only DELAY... others are saved in search already */
    disp(cons,"Setting parameters...");
    delay=PHRASESPOT_DELAY;
    if (!thfPhrasespotConfigSet(ses,r,s,PS_DELAY,delay))
        THROW("thfPhrasespotConfigSet: delay");

    if (thfRecogGetSampleRate(ses,r) != SAMPLERATE)
        THROW("Acoustic model is incompatible with audio samplerate");

    /* Initialize recognizer */
    disp(cons,"Initializing recognizer...");

    if(!thfRecogInit(ses,r,s,RECOG_KEEP_NONE))
        THROW("thfRecogInit");
    // end of wakeup IP init --------------------------------------------------

    // begin audio i/o init ---------------------------------------------------
    config.channels = NO_MIC_INPUT;
    config.rate = 16000;
    config.period_size = 1024;
    config.period_count = 4;
    config.format = PCM_FORMAT_S24_LE; // 16
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pw8CapBuf = (char *)malloc(uw32FrameSzInSampleForWakeUp*NO_MIC_INPUT*iSzOfSmpl);
    if (!pw8CapBuf)
    {
        printf("Unable to allocate %d bytes\n", uw32FrameSzInSampleForWakeUp*NO_MIC_INPUT*iSzOfSmpl);
        free(pw8CapBuf);
    }

    p = NULL;
    p = (audiol_t *)memset(malloc(sizeof(audiol_t)), 0, sizeof(audiol_t));
    p->len = uw32FrameSzInSampleForWakeUp;
    p->audio = (short int *)malloc(p->len*sizeof(short));
    p->flags = 0;

    pcm = pcm_open(1, 0, PCM_IN, &config);
    // end of audio i/o init --------------------------------------------------

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    sprintf(str,"Starting recognition loop..\n"); disp(cons,str);
    i = 0;
    while (!should_stop)
    {
        i ++;
        done = 0;
        while (!done)
        {

            // take data from audio i/o
            pcm_read(pcm, pw8CapBuf, uw32FrameSzInSampleForWakeUp*NO_MIC_INPUT*iSzOfSmpl);

#ifdef __TUTUCLEAR_20171019__
            // convert data from interleaved format to non-interleaved format
            // TUTUClear_OneFrame requires buffer with non-interleaved data
            // frame size needs to be 10 ms

            for (k=0; k<NO_MIC_INPUT; k++)
            {
                for (l=0; l<FRAMESZ_4_TUTU; l++)
                {
                    int     w32TargetChn;
                    int     w32SampleValue = ((W32 *)pw8CapBuf)[l*NO_MIC_INPUT+k];
                    short   w16SampleValue;

                    // compensate channel shuffle
                    w32TargetChn = (w32SampleValue>>8) & (W32)0x0000000F;
                    w16SampleValue = (W16)(w32SampleValue>>16);

                    if (w32TargetChn > 7)
                    {
                        printf("[ERR] illegal w32TargetChn = %d!\n", w32TargetChn);
                        w32TargetChn = 3;
                    }

                    if (w32TargetChn == 0) w32TargetChn = ((w16MICSelection0>>0) & 0xF); // U24
                    else if (w32TargetChn == 1) w32TargetChn = ((w16MICSelection0>>4) & 0xF); // U26
                    else if (w32TargetChn == 2) w32TargetChn = ((w16MICSelection0>>8) & 0xF); // AEC ref
                    else if (w32TargetChn == 3) w32TargetChn = ((w16MICSelection0>>12) & 0xF); // AEC ref
                    else if (w32TargetChn == 4) w32TargetChn = ((w16MICSelection1>>0) & 0xF); // U25 (no sound)
                    else if (w32TargetChn == 5) w32TargetChn = ((w16MICSelection1>>4) & 0xF); // U27
                    else if (w32TargetChn == 6) w32TargetChn = ((w16MICSelection1>>8) & 0xF); // U28
                    else if (w32TargetChn == 7) w32TargetChn = ((w16MICSelection1>>12) & 0xF); // U29

                    if (w32TargetChn < 6)
                        aw16MicSig[FRAMESZ_4_TUTU*w32TargetChn+l] = w16SampleValue;
                    else
                        aw16AECRef[FRAMESZ_4_TUTU*(w32TargetChn-6)+l] = w16SampleValue;
                }
            }

            // --------------------------------------------------------
            TUTUClear_OneFrame(pTUTUClearObject,
                               &aw16AECRef[0], // aec reference signal
                               &aw16MicSig[0], // microphone signal
                               &aw16LOut[0], // processsing result
                               &tTUTUClearStat);

            // place NS result to wakeup input
            memcpy(p->audio, aw16LOut, uw32FrameSzInSampleForWakeUp*sizeof(short));


#endif // __TUTUCLEAR_20171019__

            // invoke wakeup IP

            if (!thfRecogPipe(ses, r, p->len, p->audio, RECOG_ONLY, &status))
                THROW("recogPipe");

            if ((status == RECOG_DONE) || (should_stop!=0))
                done = 1;

        } // while (!done)

        /* Report N-best recognition result */
        rres = NULL;
        if (status==RECOG_DONE)
        {
            score=0;
            if (!thfRecogResult(ses,r,&score,&rres,NULL,NULL,NULL,NULL,NULL,NULL))
                THROW("thfRecogResult");

            {
                struct timeval  tv;
                struct timezone tz;
                struct tm       *p;

                gettimeofday(&tv, &tz);
                p = localtime(&tv.tv_sec);

                sprintf(str, "[%02d-%02d-%02d-%02d-%02d-%02d-%06ld] \"%s\" keyword detected (%6.1f), count: %6d",
                        p->tm_year + 1900,
                        1+p->tm_mon,
                        p->tm_mday,
                        p->tm_hour,
                        p->tm_min,
                        p->tm_sec,
                        tv.tv_usec,
                        rres,
                        score,
                        i);
                disp(cons,str);
            }
        }
        else
        {
            disp(cons,"Skipping recognition loop.");
        } // if (status==RECOG_DONE)

        thfRecogReset(ses,r);

    } // while loop

    /* Clean up */
#ifdef __TUTUCLEAR_20171019__
    TUTUClear_Release(&pTUTUClearObject);
    if (pExternallyAllocatedMem != NULL) free(pExternallyAllocatedMem);
    printf("Release tutuClear okay.\n");
#endif // __TUTUCLEAR_20171019__

    pcm_close(pcm);
    thfRecogDestroy(r);
    r=NULL;
    thfSearchDestroy(s);
    s=NULL;
    thfPronunDestroy(pp);
    pp=NULL;
    thfSessionDestroy(ses);
    ses=NULL;
    disp(cons,"Done test.");

    return 0;

    /* Async error handling, see console.h for panic */
error:
//  killAudio();
    if(!ewhat && ses) ewhat=thfGetLastError(ses);
    if(fp) fclose(fp);
    if(r) thfRecogDestroy(r);
    if(s) thfSearchDestroy(s);
    if(pp) thfPronunDestroy(pp);
    panic(cons,ewhere,ewhat,0);
    if(ses) thfSessionDestroy(ses);
    return 1;
}

#ifdef __TUTUCLEAR_20171019__
//---------------------------------------------------------------------------
// Following 8 functions are for parsing parameter file
TIniFileInfo *OpenIniFile(const char *pszFileName)
{
    FILE            *fptr;
    TIniFileInfo    *pIniFileInfo;
    int             i, iLineCount;
    char            buf[256];
    char            *pBuf;
    char            *pTemp;

    fptr = fopen(pszFileName, "r");
    if (!fptr)
        return NULL;

    pIniFileInfo = (TIniFileInfo *)malloc(sizeof(TIniFileInfo));
    if (!pIniFileInfo)
        return NULL;
    pIniFileInfo->fptr = fptr;

    for (iLineCount=0; fgets(buf, 255, fptr); iLineCount++);

    pIniFileInfo->pIdentValueList = (TIdentValue *)malloc(sizeof(TIdentValue) * iLineCount);

    fseek(fptr, 0, SEEK_SET);
    pIniFileInfo->iLineCount = iLineCount;

    for (i=0; i<iLineCount; i++)
    {
        fgets(buf, 255, fptr);
        pTemp = pIniFileInfo->pIdentValueList[i].PszIdent;

        //printf("%2d-th line(i): %s", i, buf);

        // skip while space
        pBuf = buf;
        while ((*pBuf==' ') || (*pBuf=='\t'))
        {
            pBuf ++;
        }

        // --------------------------------------------------------------------
        // fill ident
        while (((*pBuf)!=' ') &&
                ((*pBuf)!='\t') &&
                (*pBuf))
        {
            *pTemp++ = *pBuf++;
        }
        *pTemp = 0;
        //printf("%2d-th line(t): %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        // skip while space
        while ((*pBuf==' ') || (*pBuf=='\t'))
        {
            pBuf ++;
        }

        // --------------------------------------------------------------------
        // fill value
        pTemp = pIniFileInfo->pIdentValueList[i].PszValue;

        while (((*pBuf)!=' ') &&
                ((*pBuf)!='\n') &&
                ((*pBuf)!='\r') &&
                ((*pBuf)!='\t') &&
                ((*pBuf)!=';') &&
                (*pBuf))
        {
            *pTemp++ = *pBuf++;
        }

        *pTemp = 0;
        //printf("%2d-th line(v): %s\n", i, pIniFileInfo->pIdentValueList[i].PszValue);

    }
    fseek(fptr, 0, SEEK_SET);
    return pIniFileInfo;
}
//---------------------------------------------------------------------------
int CloseIniFile(TIniFileInfo *pIniFileInfo)
{
    fclose(pIniFileInfo->fptr);
    free(pIniFileInfo->pIdentValueList);
    free(pIniFileInfo);

    return 1;
}
//---------------------------------------------------------------------------
int ReadUWord16(TIniFileInfo *pIniFileInfo,
                const char   *pszIdent,
                UW16         *puw16,
                UW16         uw16Default)
{
    int         i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *puw16 = uw16Default;
    //printf("%s>>\n", pszIdent);
    for (i=0; i<pIniFileInfo->iLineCount; i++)
    {
        //printf("%2d-th line: %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        if (strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *puw16 = (UW16)strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, uw16Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadWord16(TIniFileInfo *pIniFileInfo,
               const char   *pszIdent,
               W16          *pw16,
               W16          w16Default)
{
    int         i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *pw16 = w16Default;
    //printf("%s>>\n", pszIdent);
    for (i=0; i<pIniFileInfo->iLineCount; i++)
    {
        //printf("%2d-th line: %s\n", i, pIniFileInfo->pIdentValueList[i].PszIdent);

        if (strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *pw16 = (W16)strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, w16Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, W32 *pw32Value, W32 w32Default)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *pw32Value = w32Default;
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *pw32Value = strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, w32Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadUWord32(TIniFileInfo *pIniFileInfo, const char *pszIdent, UW32 *puw32Value, UW32 uw32Default)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    *puw32Value = uw32Default;
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            *puw32Value = strtol(pIdentValue[i].PszValue, NULL, 0);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %d (presse any key to continue).\n", pszIdent, uw32Default);
        //getche();
    }

    return 1;
}
//---------------------------------------------------------------------------
int ReadString(TIniFileInfo *pIniFileInfo, char *pszIdent, char *pszValue, char *pszDefault)
{
    int i;
    TIdentValue *pIdentValue = pIniFileInfo->pIdentValueList;

    strcpy(pszValue, pszDefault);
    for(i = 0; i < pIniFileInfo->iLineCount; i++)
    {
        if(strcmp(pIdentValue[i].PszIdent, pszIdent) == 0)
        {
            strcpy(pszValue, pIdentValue[i].PszValue);
            break;
        }
    }

    if (i == pIniFileInfo->iLineCount)
    {
        printf("Cannot find \"%s\", set as %s (presse any key to continue).\n", pszIdent, pszDefault);
        //getche();
    }

    return 1;
}

void TUTUClear_ParsePRMFile_QACT(const char            *pszCtrlFileName,
                                 TUTUClearConfig_t     *ptTUTUClearConfig,
                                 TUTUClearParam_t      *ptTUTUClearParam)
{
    TIniFileInfo    *pIniFileInfo;

    if ((pIniFileInfo=OpenIniFile(pszCtrlFileName)) == NULL) return;

    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_SAMPLING_FREQ", &ptTUTUClearConfig->uw16SamplingFreq, 0);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_FRAME_SZ_IN_MS", &ptTUTUClearConfig->uw16FrameSz, 10);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_MAX_NUM_MIC", &ptTUTUClearConfig->uw16MaxNumOfMic, 0);
    ReadUWord16(pIniFileInfo, "TUTUCLEARCONFIG_MAX_TAIL_LENGTH_IN_MS", &ptTUTUClearConfig->uw16MaxTailLength, 0);

    /* Params to TUTUClear_SetParams() ----------------------------------------- */
    ReadUWord32(pIniFileInfo, "TUTU_PARAM_SYS.uw32OpMode", &ptTUTUClearParam->uw32OpMode, 0);
    ReadUWord32(pIniFileInfo, "TUTU_PARAM_SYS.uw32FuncMode", &ptTUTUClearParam->uw32FuncMode, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16NumOfMic", &ptTUTUClearParam->uw16NumOfMic, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16ECTailLengthInMs", &ptTUTUClearParam->uw16ECTailLengthInMs, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv0", &ptTUTUClearParam->uw16Resv0, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv1", &ptTUTUClearParam->uw16Resv1, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv2", &ptTUTUClearParam->uw16Resv2, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_SYS.uw16Resv3", &ptTUTUClearParam->uw16Resv3, 0);

    /* Params to AEC ----------------------------------------------------------- */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[0]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[1]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[2]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[3]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[4]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[5]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[6]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[7]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[8]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[9]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[10]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[11]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[12]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[13]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[14]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AEC.auw16ParamAEC[15]", &ptTUTUClearParam->tTUTUAECParam.auw16ParamAEC[15], 0);
    /* Params to NS ------------------------------------------------------------ */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16Lambda", &ptTUTUClearParam->tTUTUNSParam.uw16Lambda, 32113);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16SupressionLevel", &ptTUTUClearParam->tTUTUNSParam.uw16SupressionLevel, 1);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16BGSimplicity", &ptTUTUClearParam->tTUTUNSParam.uw16BGSimplicity, 4096);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.uw16D", &ptTUTUClearParam->tTUTUNSParam.uw16D, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[0]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[1]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[2]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[3]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[4]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[5]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[6]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[7]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[8]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[9]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[10]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[11]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[12]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[13]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[14]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[15]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[15], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[16]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[16], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[17]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[17], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[18]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[18], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[19]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[19], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[20]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[20], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[21]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[21], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[22]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[22], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[23]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[23], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[24]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[24], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[25]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[25], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[26]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[26], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_NS.auw16Resrv[27]", &ptTUTUClearParam->tTUTUNSParam.auw16Resrv[27], 0);
    /* Params to EQ ------------------------------------------------------------ */
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.uw16EQPartitionBegin", &ptTUTUClearParam->tTUTUEQParam.uw16EQPartitionBegin, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.uw16Resrv0", &ptTUTUClearParam->tTUTUEQParam.uw16Resrv0, 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[0]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[0], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[1]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[1], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[2]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[2], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[3]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[4]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[4], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[5]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[5], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[6]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[6], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[7]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[7], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[8]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[8], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[9]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[9], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[10]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[10], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[11]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[11], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[12]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[12], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[13]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[13], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[14]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[14], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_EQ.auw16EQPartitionWidth[15]", &ptTUTUClearParam->tTUTUEQParam.auw16EQPartitionWidth[15], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[0]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[1]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[2]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[3]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[4]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[4], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[5]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[5], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[6]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[6], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[7]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[7], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[8]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[8], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[9]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[9], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[10]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[10], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[11]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[11], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[12]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[12], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[13]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[13], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[14]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[14], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_EQ.aw16EQGain[15]", &ptTUTUClearParam->tTUTUEQParam.aw16EQGain[15], 0);
    /* Params to DRC ----------------------------------------------------------- */
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16ExpanderInputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16ExpanderInputLevels[0], -96);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16ExpanderInputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16ExpanderInputLevels[1], -96);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16NumCompressorKnees", &ptTUTUClearParam->tTUTUDRCParam.uw16NumCompressorKnees, 3);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16Resrv0", &ptTUTUClearParam->tTUTUDRCParam.uw16Resrv0, 3);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[2]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeInputLevels[3]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeInputLevels[3], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[0]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[0], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[1]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[1], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[2]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[2], 0);
    ReadWord16(pIniFileInfo, "TUTU_PARAM_DRC.aw16CompressorKneeOutputLevels[3]", &ptTUTUClearParam->tTUTUDRCParam.aw16CompressorKneeOutputLevels[3], 0);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16AttackTime", &ptTUTUClearParam->tTUTUDRCParam.uw16AttackTime, 5);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16ReleaseTime", &ptTUTUClearParam->tTUTUDRCParam.uw16ReleaseTime, 10);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16MakeupGain", &ptTUTUClearParam->tTUTUDRCParam.uw16MakeupGain, 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.uw16Resrv1", &ptTUTUClearParam->tTUTUDRCParam.uw16Resrv1, 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.auw16Resrv[0]", &ptTUTUClearParam->tTUTUDRCParam.auw16Resrv[0], 0x0100);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_DRC.auw16Resrv[1]", &ptTUTUClearParam->tTUTUDRCParam.auw16Resrv[1], 0x0100);
    /* Params to AGC ----------------------------------------------------------- */
    ReadWord16(pIniFileInfo, "TUTU_PARAM_AGC.w16TargetOutputLevel", &ptTUTUClearParam->tTUTUAGCParam.w16TargetOutputLevel, -9);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16MaxGain", &ptTUTUClearParam->tTUTUAGCParam.uw16MaxGain, 2048);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16MinGain", &ptTUTUClearParam->tTUTUAGCParam.uw16MinGain, 32);
    ReadUWord16(pIniFileInfo, "TUTU_PARAM_AGC.uw16RespRate", &ptTUTUClearParam->tTUTUAGCParam.uw16RespRate, 16384);

    CloseIniFile(pIniFileInfo);
    return;
}
#endif // __TUTUCLEAR_20171019__
