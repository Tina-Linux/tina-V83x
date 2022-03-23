#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <iostream>

#include <snsr.h>

#include "tutuClear.h"
#include "tutu_tool.h"

#define NO_MIC_INPUT             8
#define FRAMESZ_4_WAKEUP         160 // in sample, should be integer multiply of FRAMESZ_4_TUTU
#define FRAMESZ_4_TUTU           160 // in sample
#define TUTU_PRM_FNAME           "/etc/avs/tutuClearA1_ns4wakeup.prm"

struct wav_header {
    uint32_t riff_id;           /*00H ~ 03H*/   //"RIFF"
    uint32_t riff_sz;           /*04H ~ 07H*/
    uint32_t riff_fmt;          /*08H ~ 0BH*/   //"WAVE"
    uint32_t fmt_id;            /*0CH ~ 0FH*/   //"fmt "
    uint32_t fmt_sz;            /*10H ~ 13H*/   //PCM 16
    uint16_t audio_format;      /*14H ~ 15H*/   //PCM 1
    uint16_t num_channels;      /*16H ~ 17H*/   //PCM 1
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

void                    *pTUTUClearObject = NULL;
void                    *pExternallyAllocatedMem = NULL;
TUTUClearConfig_t       tTUTUClearConfig;
TUTUClearParam_t        tTUTUClearParam;
TUTUClearStat_t         tTUTUClearStat;

W32 atTSMPLAECRef[FRAMESZ_4_TUTU*2];
W32 atTSMPLMicSig[FRAMESZ_4_TUTU*NO_MIC_INPUT]; // up to 8 channels
W32 atTSMPLLOut[FRAMESZ_4_WAKEUP*3];;
short w16MICSelection0 = 0x0324;
short w16MICSelection1 = 0x1576;
int                     iMemSzInByte;

SnsrSession m_session;

int tutu_init()
{
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
        fprintf(stderr, "Software voice processor compiled on: " __DATE__ " " __TIME__ "\n");
        fprintf(stderr, "TUTUCLEAR Ver. %d.%d.%d Inside (0x%08x).\n", cMajor, cMinor, cRevision, uw32Config);
        fprintf(stderr, "Copyright (C) 2017, Spectimbre Inc.\n");
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
    memset(atTSMPLAECRef, 0, FRAMESZ_4_TUTU*sizeof(short));

    return 0;
}

int tutu_process(char *in, int in_samples, char *out)
{
    int k, l;
    for (k=0; k<NO_MIC_INPUT; k++)
    {
        for (l=0; l<in_samples; l++)
        {
            int     w32TargetChn;
            W32   w32SampleValue = ((W32 *)in)[l*NO_MIC_INPUT+k];

            // compensate channel shuffle
            w32TargetChn = k;
            w32SampleValue = w32SampleValue & (W32)0xFFFFFF00;

            if (w32TargetChn == 0) w32TargetChn = ((w16MICSelection0>>0) & 0xF); // U24
            else if (w32TargetChn == 1) w32TargetChn = ((w16MICSelection0>>4) & 0xF); // U26
            else if (w32TargetChn == 2) w32TargetChn = ((w16MICSelection0>>8) & 0xF); // AEC ref
            else if (w32TargetChn == 3) w32TargetChn = ((w16MICSelection0>>12) & 0xF); // AEC ref
            else if (w32TargetChn == 4) w32TargetChn = ((w16MICSelection1>>0) & 0xF); // U25 (no sound)
            else if (w32TargetChn == 5) w32TargetChn = ((w16MICSelection1>>4) & 0xF); // U27
            else if (w32TargetChn == 6) w32TargetChn = ((w16MICSelection1>>8) & 0xF); // U28
            else if (w32TargetChn == 7) w32TargetChn = ((w16MICSelection1>>12) & 0xF); // U29

            if (w32TargetChn < 6)
                atTSMPLMicSig[FRAMESZ_4_WAKEUP*w32TargetChn+l] = w32SampleValue;
            else
                atTSMPLAECRef[FRAMESZ_4_WAKEUP*(w32TargetChn-6)+l] = w32SampleValue;
        }
    }

    TUTUClear_OneFrame_32b(pTUTUClearObject,
                       &atTSMPLAECRef[0], // aec reference signal
                       &atTSMPLMicSig[0], // microphone signal
                       &atTSMPLLOut[0], // processsing result
                       &tTUTUClearStat);

    W16 *w16Out = (W16 *)out;
    for (l=0; l < FRAMESZ_4_TUTU; l++)
        w16Out[l] = (W16)(atTSMPLLOut[l] >> 16);

    static int count = 0;
    if (tTUTUClearStat.w16VtrgrFlg != 0) printf("wakeup_word:\"alexa\" detected. count: %d\n", count++);

    return FRAMESZ_4_TUTU;
}

int tutu_release()
{
    TUTUClear_Release(&pTUTUClearObject);
    if (pExternallyAllocatedMem != NULL) free(pExternallyAllocatedMem);
    printf("Release tutuClear okay.\n");
    return 0;
}

static std::string getSensoryDetails(SnsrSession session, SnsrRC result) {
    std::string message;
    // It is recommended by Sensory to prefer snsrErrorDetail() over snsrRCMessage() as it provides more details.
    if (session) {
        message = snsrErrorDetail(session);
    } else {
        message = snsrRCMessage(result);
    }
    if (message.empty()) {
        message = "Unrecognized error";
    }
    return message;
}

static SnsrRC keyWordDetectedCallback(SnsrSession s, const char* key, void* userData)
{
    static int count = 0;
    SnsrRC result;
    const char* keyword;
    double begin;
    double end;
    result = snsrGetDouble(s, SNSR_RES_BEGIN_SAMPLE, &begin);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed getbegin " << getSensoryDetails(s, result) << std::endl;
        return result;
    }

    result = snsrGetDouble(s, SNSR_RES_END_SAMPLE, &end);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed getend " << getSensoryDetails(s, result) << std::endl;
        return result;
    }

    result = snsrGetString(s, SNSR_RES_TEXT, &keyword);
    if (result != SNSR_RC_OK) {
        std::cout << "keyWordDetectedCallbackFailed keywordRetrievalFailure " << getSensoryDetails(s, result) << std::endl;
        return result;
    }
    std::cout << "keyWordDetected! " << count++ << std::endl;
    return SNSR_RC_OK;
}
int snsr_release()
{
    snsrRelease(m_session);
    return 0;
}
int snsr_init(const char *model, int point)
{

    // Allocate the Sensory library handle
    SnsrRC result = snsrNew(&m_session);
    if (result != SNSR_RC_OK) {
    std::cout << "initFailed: snsrNew " << getSensoryDetails(m_session, result) << std::endl;
    exit(-1);
    }

    // Get the expiration date of the library
    const char* info = nullptr;
    result = snsrGetString(m_session, SNSR_LICENSE_EXPIRES, &info);
    if (result == SNSR_RC_OK && info) {
        // Will print "License expires on <date>"
        std::cout << info << std::endl;
    } else {
        std::cout << "Sensory library license does not expire." << std::endl;
    }

    // Check if the expiration date is near, then we should display a warning
    result = snsrGetString(m_session, SNSR_LICENSE_WARNING, &info);
    if (result == SNSR_RC_OK && info) {
        // Will print "License will expire in <days-until-expiration> days."
        std::cout << info << std::endl;
    } else {
        std::cout << "Sensory library license does not expire for at least 60 more days." << std::endl;
    }

    result = snsrLoad(m_session, snsrStreamFromFileName(model, "r"));
    if (result != SNSR_RC_OK) {
        std::cout << "initFailed: snsrLoad " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }

    if(point > 0){
        int target;
        result = snsrGetInt(m_session, SNSR_OPERATING_POINT, &target);
        std::cout << "Sensory model default operating point " << std::to_string(target) << std::endl;

        result = snsrSet(m_session, ("operating-point=" + std::to_string(point)).c_str());
        if (result != SNSR_RC_OK) {
            std::cout <<"error" << getSensoryDetails(m_session, result) << std::endl;
        }

        result = snsrGetInt(m_session, SNSR_OPERATING_POINT, &target);
        std::cout << "Sensory model new operating point " << std::to_string(target) << std::endl;
    }

    // Setting the callback handler
    result = snsrSetHandler(
            m_session,
            SNSR_RESULT_EVENT,
            snsrCallback(keyWordDetectedCallback, nullptr, nullptr));

    if (result != SNSR_RC_OK) {
    std::cout << "setUpRuntimeSettingsFailed: setKeywordDetectionHandlerFailure " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }

    /*
     * Turns off automatic pipeline flushing that happens when the end of the input stream is reached. This is an
     * internal setting recommended by Sensory when audio is presented to Sensory in small chunks.
     */
    result = snsrSetInt(m_session, SNSR_AUTO_FLUSH, 0);
    if (result != SNSR_RC_OK) {
    std::cout << "setUpRuntimeSettingsFailed: disableAutoPipelineFlushingFailed " << getSensoryDetails(m_session, result) << std::endl;
        exit(-1);
    }
    return 0;
}

void usage(const char *name)
{
    printf("usage:\n");
    printf("%s target-wav-file [retest-times] [sensory-model] [sensory-model-operating-point]\n", name);
}

int main(int argc, char *argv[])
{
    const char *file = nullptr;
    const char *model = nullptr;
    int point = 0;

    int re_test = 1;

    if(argc < 4) {
        usage(argv[0]);
        exit(-1);
    }

    file = argv[1];
    re_test = atoi(argv[2]);
    model = argv[3];
    point = atoi(argv[4]);

    tutu_init();
    snsr_init(model, point);

    //File
    printf("open: %s, re_test: %d\n", file, re_test);
    FILE *fp = fopen(file, "r");
    if(fp == NULL){
        printf("fopen error %s\n",strerror(errno));
        return -1;
    }

    struct wav_header header;
    int bytes = fread((void*)&header, 1, sizeof(struct wav_header), fp);
    printf("num_channels = %d\n", header.num_channels);
    printf("bits_per_sample = %d\n", header.bits_per_sample);
    printf("sample_rate = %d\n", header.sample_rate);
    printf("total samples = %d\n", header.data_sz/(header.num_channels*header.bits_per_sample/8));

    char data[FRAMESZ_4_TUTU*8*4];
    FILE *fout = fopen("/tmp/tutuclear_test_out.wav", "wb");
    struct wav_header out_header;
    memcpy(&out_header, &header, sizeof(header));
    out_header.num_channels = 1;
    out_header.sample_rate = 16;
    //out_header.data_sz =
    while(re_test-- > 0) {
        printf("#%d\n", re_test);

        fseek(fp, sizeof(struct wav_header), SEEK_SET);

        int should_break = 1;
        while(should_break) {
            int bytes = fread(data, 1, FRAMESZ_4_TUTU*8*4, fp);
            if(bytes < 0) {
                printf("fread error %s\n",strerror(errno));
                exit(-1);
            }
            if(bytes < FRAMESZ_4_TUTU*8*4){
                printf("Reach the end of the file? fread bytes %d\n",bytes);
                should_break = 0;
                continue;
            }
            int samples = tutu_process(data, FRAMESZ_4_TUTU, (char*)atTSMPLLOut);
            fwrite(atTSMPLLOut, 1, samples*sizeof(int16_t), fout);
            snsrSetStream(
                    m_session,
                    SNSR_SOURCE_AUDIO_PCM,
                    snsrStreamFromMemory(atTSMPLLOut, samples * sizeof(int16_t), SNSR_ST_MODE_READ));
            SnsrRC result = snsrRun(m_session);
            switch (result) {
                case SNSR_RC_STREAM_END:
                    // Reached end of buffer without any keyword detections
                    break;
                case SNSR_RC_OK:
                    std::cout << "SNSR_RC_OK" << std::endl;
                    break;
                default:
                    // A different return from the callback function that indicates some sort of error
                std::cout << "detect: unexpectedReturn " << getSensoryDetails(m_session, result) << std::endl;
                exit(-1);
            }
            snsrClearRC(m_session);
        }
    }
    fclose(fout);
    fclose(fp);
    snsr_release();
    tutu_release();
    return 0;
}
