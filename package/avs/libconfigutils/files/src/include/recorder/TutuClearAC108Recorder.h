#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_TUTUCLEAR_AC108_RECODRER_WRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_TUTUCLEAR_AC108_RECODRER_WRAPPER_H_

#include "recorder/RecorderInterface.h"
#include "utils/WavUtils.h"

#include <tutu/tutuClear.h>
#include <tutu/tutu_tool.h>
#define NO_MIC_INPUT                4
#define FRAMESZ_4_WAKEUP         160 // in sample, should be integer multiply of FRAMESZ_4_TUTU
#define FRAMESZ_4_TUTU           160 // in sample
#define TUTU_PRM_FNAME           "/etc/avs/tutuClearA1_ns4wakeup.prm"

namespace alexaClientSDK {
namespace sampleApp {

class TutuClearAC108Recorder : public RecorderInterface
{
public:
    TutuClearAC108Recorder(AC108Recorder::AC108Mode mode);
    ~TutuClearAC108Recorder();

    int init(std::shared_ptr<Application::ConfigUtils> config) override;
    int release() override;
    int fetch(char *data, int samples) override;

private:
    int tutu_init();
    int tutu_process_encode(char *in, int in_samples, char *out);
    int tutu_process_normal(char *in, int in_samples, char *out);
private:
    //tutu
    void                    *pTUTUClearObject = nullptr;
    void                    *pExternallyAllocatedMem = nullptr;
    TUTUClearConfig_t       tTUTUClearConfig;
    TUTUClearParam_t        tTUTUClearParam;
    TUTUClearStat_t         tTUTUClearStat;

    W32 atTSMPLAECRef[FRAMESZ_4_TUTU*2];
    W32 atTSMPLMicSig[FRAMESZ_4_TUTU*NO_MIC_INPUT]; // up to 8 channels
    W32 atTSMPLLOut[FRAMESZ_4_WAKEUP];
    short w16MICSelection0 = 0x0324;
    short w16MICSelection1 = 0x1576;
/*
    W32 *atTSMPLAECRef;
    W32 *atTSMPLMicSig;
    W32 *atTSMPLLOut;
*/
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif /*ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_TUTUCLEAR_AC108_RECODRER_WRAPPER_H_*/
