#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AUDIOCODEC_RECODRER_WRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AUDIOCODEC_RECODRER_WRAPPER_H_

#include "recorder/AlsaRecorder.h"
#include "utils/WavUtils.h"

namespace alexaClientSDK {
namespace sampleApp {

class AudioCodecRecorder : public AlsaRecorder
{
public:
    AudioCodecRecorder();
    virtual ~AudioCodecRecorder();

    virtual int init(std::shared_ptr<Application::ConfigUtils> config) override;
    virtual int release() override;
    virtual int start() override;
    virtual int stop() override;
    virtual int fetch(char *data, int samples) override;

private:
    AlsaRecorder m_aif2aec;

    int m_channel_use_to_detect;
    char *m_inputBuffer;

    WavUtils *m_wav_utils;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif /*ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_AUDIOCODEC_RECODRER_WRAPPER_H_*/
