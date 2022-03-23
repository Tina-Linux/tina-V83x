#ifndef __RECORDER_AUDIO_CODEC_PROVIDER_H__
#define __RECORDER_AUDIO_CODEC_PROVIDER_H__

#include <memory>
#include <json-c/json.h>
#include "utils/AlsaUtils.h"
#include "recorder/ProviderInterface.h"

namespace AW {

class AudioCodecProvider : public ProviderInterface
{
public:
    static std::shared_ptr<ProviderInterface> create();

    ~AudioCodecProvider();

    int init(struct json_object *config);
    int release();
    int start();
    int stop();
    int fetch(char **data, int samples) { return -1; };
    int fetch(char *data, int samples);
private:
    AudioCodecProvider();

private:
    int m_period_size;
    int m_period;
    const char *m_device;

    AlsaUtils m_alsa_utils;
};
} // namespace AW

#endif /*__RECORDER_AUDIO_CODEC_PROVIDER_H__*/
