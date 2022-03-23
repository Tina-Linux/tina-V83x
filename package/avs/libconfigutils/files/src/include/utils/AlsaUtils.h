#ifndef __ALSA_UTILS_H__
#define __ALSA_UTILS_H__

#include <alsa/asoundlib.h>

namespace AW {

class AlsaUtils
{
public:
    enum class Type
    {
        PLAYBACK,
        CAPTURE
    };

    AlsaUtils(Type type = Type::CAPTURE);
    ~AlsaUtils();

    int init(const char *device, int sample_rate, int num_channels, int bits_per_sample, int period_size, int period);
    int release();
    int fetch(char *data, int samples);
    int play(char *data, int samples);
    int start();
    int stop();

private:
    char *m_device{nullptr};
    int m_alsa_can_pause{0};
    snd_pcm_t *m_snd_handler;

    Type m_type{Type::CAPTURE};
};

}
#endif /*__ALSA_UTILS_H__*/
