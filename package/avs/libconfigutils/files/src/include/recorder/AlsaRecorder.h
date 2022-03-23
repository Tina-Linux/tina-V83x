#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_RECODRER_WRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_RECODRER_WRAPPER_H_

#include <alsa/asoundlib.h>
#include <ConfigUtils.h>

#include "utils/WavUtils.h"

std::string getTimeStamp();

namespace alexaClientSDK {
namespace sampleApp {

class AlsaRecorder
{
public:
    AlsaRecorder();
    virtual ~AlsaRecorder();
    virtual int init(const char *device, int sample_rate, int num_channels, int bits_per_sample, int period_size, int period);
    virtual int init(std::shared_ptr<Application::ConfigUtils> config){return 0;};
    virtual int release();
    virtual int fetch(char *data, int samples);
    virtual int start();
    virtual int stop();

protected:
    std::shared_ptr<Application::ConfigUtils> m_configUtils;
    int m_sample_rate;
    int m_channels;
    int m_sample_bits;
    int m_period_size;
    int m_period;
    const char *m_device;
private:
    snd_pcm_t *m_snd_handler;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif /*ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_RECODRER_WRAPPER_H_*/
