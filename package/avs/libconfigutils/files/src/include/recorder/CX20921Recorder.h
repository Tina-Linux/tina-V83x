#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_CX20921_RECODRER_WRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_CX20921_RECODRER_WRAPPER_H_

#include "recorder/AudioCodecRecorder.h"
#include "utils/WavUtils.h"

namespace alexaClientSDK {
namespace sampleApp {

class CX20921Recorder : public AudioCodecRecorder
{
public:
    CX20921Recorder();
    ~CX20921Recorder();

    int init(std::shared_ptr<Application::ConfigUtils> config) override;
    int release() override;
    int start() override;
    int stop() override;

private:
    AlsaRecorder m_aif2aec;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif /*ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_CX20921_RECODRER_WRAPPER_H_*/
