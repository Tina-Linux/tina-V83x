#include "tconfigs/audio/tutuclear/tutuclear_wrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include <tutu/tutu_typedef.h>
#include <tutu/tutu_tool.h>

namespace tconfigs {
namespace audio {

namespace {
const char *formatExpirationDate(time_t expiration)
{
    static char expdate[33];
    if (!expiration) return "never";
    strftime(expdate, 32, "%m/%d/%Y 00:00:00 GMT", gmtime(&expiration));
    return expdate;
}
} // namespace

TutuclearWrapper::~TutuclearWrapper(void)
{
    Release();
}

int TutuclearWrapper::Init(const char* prm_file, const char* keyword_file)
{
    //begin to init tutuClear
    tTUTUClearConfig.uw32Version = TUTUCLEAR_VERSION;
    printf("Parsing %s\n", prm_file);
    TUTUClear_ParsePRMFile_QACT(prm_file,
                                &tTUTUClearConfig,
                                &tTUTUClearParam);

    printf("uw16FrameSz = 0x%x\n", (int)tTUTUClearConfig.uw16FrameSz);
    printf("uw16MaxNumOfMic = 0x%x\n", (int)tTUTUClearConfig.uw16MaxNumOfMic);
    printf("uw16MaxTailLength = 0x%x\n", (int)tTUTUClearConfig.uw16MaxTailLength);
    printf("uw16SamplingFreq = 0x%x\n", (int)tTUTUClearConfig.uw16SamplingFreq);
    printf("KewWordGap = 0x%x, 0x%x\n",tTUTUClearParam.tTUTUDOAParam.auw16Resrv[0], tTUTUClearParam.tTUTUDOAParam.auw16Resrv[1]);

    // memeory allocation
    UW32 iMemSzInByte = TUTUClear_QueryMemSz(&tTUTUClearConfig);
    printf("tutuClear DM usage = %d bytes\n", iMemSzInByte);
    pExternallyAllocatedMem = malloc(iMemSzInByte);

    // version check
    {
        W8 cMajor, cMinor, cRevision;
        UW32 uw32Config = TUTUClear_GetVerNum(&cMajor, &cMinor, &cRevision);
        fprintf(stderr,"Software voice processor compiled on: " __DATE__ " " __TIME__"\n");
        fprintf(stderr,"TUTUCLEAR Ver. %d.%d.%d Inside (0x%08x).\n", cMajor, cMinor, cRevision, uw32Config);
    }

    // init
    printf("uw32OpMode = %08X\n", tTUTUClearParam.uw32OpMode);
    printf("uw32FuncMode = %08X\n", tTUTUClearParam.uw32FuncMode);
    printf("uw16NumOfMic = %d\n", tTUTUClearParam.uw16NumOfMic);
    printf("uw16ECTailLengthInMs = %d\n", tTUTUClearParam.uw16ECTailLengthInMs);
    if (TUTU_OK != TUTUClear_Init(&tTUTUClearConfig,
                                  pExternallyAllocatedMem,
                                  &pTUTUClearObject)) {
        printf("Fail to do TUTUClear_Init.\n");
        return -1;
    } else {
        printf("TUTUClear_Init okay.\n");
    }

    // set parameter
    if (TUTU_OK != TUTUClear_SetParams(pTUTUClearObject, &tTUTUClearParam)) {
        printf("Fail to do TUTUClear_SetParams.\n");
        return -1;
    } else {
        printf("TUTUClear_SetParams okay.\n");
    }

    if (keyword_file) {
        if (TUTU_OK != TUTUClear_ImportKeywordData(
                    pTUTUClearObject, (const W8 *)keyword_file)) {
            printf("Fail to do TUTUClear_ImportKeywordData.\n");
            return -1;
        }
    }
    //-----tutuClear Init End----

    printf("TUTUClear can be evaluated for %d minutes.\n",
            TUTUClear_ResetSCFlg(pTUTUClearObject,0));
    printf("TUTUClear can be evaluated till %s.\n",
            formatExpirationDate((time_t)TUTUClear_QueryLicenseExpiration()));

    return 0;
}

void TutuclearWrapper::Release(void)
{
    if (pTUTUClearObject) {
        TUTUClear_Release(&pTUTUClearObject);
        pTUTUClearObject = nullptr;
    }
}


int TutuclearWrapper::ProcessOneFrame(int16_t* common_signal_buf,
        int16_t* reference_signal_buf, int16_t* output_signal_buf)
{
    return TUTUClear_OneFrame(pTUTUClearObject, (W16*)reference_signal_buf,
            (W16*)common_signal_buf, (W16*)output_signal_buf, &tTUTUClearStat);
}

int TutuclearWrapper::ProcessOneFrame(int32_t* common_signal_buf,
        int32_t* reference_signal_buf, int32_t* output_signal_buf)
{
    return TUTUClear_OneFrame_32b(pTUTUClearObject, (W32*)reference_signal_buf,
            (W32*)common_signal_buf, (W32*)output_signal_buf, &tTUTUClearStat);
}

bool TutuclearWrapper::KeywordIsDetected(void)
{
    return (tTUTUClearStat.w16VtrgrFlg >> 8 != 0);
}

} // namespace audio
} // namespace tconfigs
