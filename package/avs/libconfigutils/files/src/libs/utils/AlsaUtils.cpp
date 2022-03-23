#include <string.h>
#include <iostream>

#include "utils/AlsaUtils.h"

static void printf_alsa_state(const char *str, _snd_pcm_state state) {
    switch(state) {
    case SND_PCM_STATE_OPEN :
        printf("%s, alsa state: SND_PCM_STATE_OPEN\n", str);
        break;
    case SND_PCM_STATE_SETUP :
        printf("%s, alsa state: SND_PCM_STATE_SETUP\n", str);
        break;
    case SND_PCM_STATE_PREPARED :
        printf("%s, alsa state: SND_PCM_STATE_PREPARED\n", str);
        break;
    case SND_PCM_STATE_RUNNING :
        printf("%s, alsa state: SND_PCM_STATE_RUNNING\n", str);
        break;
    case SND_PCM_STATE_XRUN :
        printf("%s, alsa state: SND_PCM_STATE_XRUN\n", str);
        break;
    case SND_PCM_STATE_DRAINING :
        printf("%s, alsa state: SND_PCM_STATE_DRAINING\n", str);
        break;
    case SND_PCM_STATE_PAUSED :
        printf("%s, alsa state: SND_PCM_STATE_PAUSED\n", str);
        break;
    case SND_PCM_STATE_SUSPENDED :
        printf("%s, alsa state: SND_PCM_STATE_SUSPENDED\n", str);
        break;
    default:
        printf("%s, alsa state :%d is not string printf\n", str, state);
    }
}
namespace AW {

AlsaUtils::AlsaUtils(Type type)
{
    m_type = type;
    m_snd_handler = NULL;
}

AlsaUtils::~AlsaUtils()
{
    stop();
    release();
}

int AlsaUtils::init(const char *device, int sample_rate, int num_channels, int bits_per_sample, int period_size, int period)
{
    int rc;
    snd_pcm_hw_params_t *params;
    snd_pcm_stream_t stream_type;

    if(m_type == Type::CAPTURE) stream_type = SND_PCM_STREAM_CAPTURE;
    else stream_type = SND_PCM_STREAM_PLAYBACK;

    rc = snd_pcm_open(&m_snd_handler, device, stream_type, 0);
    if (rc < 0) {
        printf("%s: unable to open pcm device: %s\n", device, snd_strerror(rc));
        return rc;
    }

    m_device = (char*)malloc(strlen(device)+1);
    bzero(m_device, strlen(device)+1);
    strcpy(m_device, device);

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(m_snd_handler, params);
    snd_pcm_hw_params_set_access(m_snd_handler, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    if(bits_per_sample == 16)
       snd_pcm_hw_params_set_format(m_snd_handler, params, SND_PCM_FORMAT_S16_LE);
    else if(bits_per_sample == 24)
       snd_pcm_hw_params_set_format(m_snd_handler, params, SND_PCM_FORMAT_S24_LE);
    else if(bits_per_sample == 32)
       snd_pcm_hw_params_set_format(m_snd_handler, params, SND_PCM_FORMAT_S32_LE);

    snd_pcm_hw_params_set_channels(m_snd_handler, params, num_channels);
    snd_pcm_hw_params_set_rate(m_snd_handler, params, sample_rate, 0);

    snd_pcm_uframes_t tmp_period_size = period_size;
    unsigned int tmp_period = period;
    snd_pcm_hw_params_set_period_size_near(m_snd_handler, params, &tmp_period_size, NULL);
    snd_pcm_hw_params_set_periods_near(m_snd_handler, params, &tmp_period, NULL);

    rc = snd_pcm_hw_params(m_snd_handler, params);/* Write the parameters to the driver */
    if (rc < 0) {
        printf("%s: unable to set hw parameters: %s\n",device, snd_strerror(rc));
        return rc;
    }
    m_alsa_can_pause = snd_pcm_hw_params_can_pause(params);
    printf("%s, alsa can pause: %d\n", device, m_alsa_can_pause);

    printf_alsa_state(m_device, snd_pcm_state(m_snd_handler));

    if(m_type == Type::PLAYBACK){
        snd_pcm_sw_params_t *alsa_swparams;
        snd_pcm_sw_params_alloca(&alsa_swparams);
        snd_pcm_sw_params_current(m_snd_handler, alsa_swparams);
        snd_pcm_uframes_t val;
        snd_pcm_sw_params_get_start_threshold(alsa_swparams, &val);
        printf("%s, snd_pcm_sw_params_get_start_threshold: %ld\n", device, val);
        snd_pcm_sw_params_get_stop_threshold(alsa_swparams, &val);
        printf("%s, snd_pcm_sw_params_get_stop_threshold: %ld\n", device, val);
        snd_pcm_sw_params(m_snd_handler, alsa_swparams);
        snd_pcm_sw_params_set_start_threshold(m_snd_handler, alsa_swparams, period_size);
        //snd_pcm_sw_params_set_stop_threshold(m_snd_handler, alsa_swparams, period_size);
        snd_pcm_sw_params(m_snd_handler, alsa_swparams);
        snd_pcm_sw_params_get_start_threshold(alsa_swparams, &val);
        printf("%s, snd_pcm_sw_params_get_start_threshold: %ld\n", device, val);
        snd_pcm_sw_params_get_stop_threshold(alsa_swparams, &val);
        printf("%s, snd_pcm_sw_params_get_stop_threshold: %ld\n", device, val);
    }
    return 0;
}

int AlsaUtils::release()
{
    if(m_snd_handler != nullptr){
        snd_pcm_close(m_snd_handler);
        m_snd_handler = nullptr;
    }
    if(m_device != nullptr){
        free(m_device);
        m_device = nullptr;
    }
}

int AlsaUtils::fetch(char *data, int samples)
{
    if(m_type == Type::PLAYBACK) return -1;

    int ret = snd_pcm_readi(m_snd_handler, data, samples);

    if (ret == -EPIPE) {
        /* EPIPE means overrun */
        printf("%s, overrun occurred\n", m_device);
        snd_pcm_prepare(m_snd_handler);
        ret = 0;
    } else if (ret < 0) {
        printf("%s, error from read: %s\n", m_device, snd_strerror(ret));
    }

    return ret;
}

int AlsaUtils::play(char *data, int samples)
{
    if(m_type == Type::CAPTURE) return -1;

    int ret = snd_pcm_writei(m_snd_handler, data, samples);

    if (ret == -EAGAIN || ret == -EINTR){
        ret = 0;
    } else if (ret == -EPIPE) {
        /* EPIPE means overrun */
        printf("%s, underrun occurred\n", m_device);
        snd_pcm_prepare(m_snd_handler);
        ret = 0;
    } else if (ret < 0) {
        printf("%s, error from read: %s\n", m_device, snd_strerror(ret));
    }

    return ret;
}

int AlsaUtils::start()
{
    if(m_type == Type::PLAYBACK) return 0;

    if(m_snd_handler == NULL) return -1;
    int rc;
    if(snd_pcm_state(m_snd_handler) == SND_PCM_STATE_PREPARED) {
        rc = snd_pcm_start(m_snd_handler);
    }else if(snd_pcm_state(m_snd_handler) == SND_PCM_STATE_RUNNING) {
        rc = 0;
    }else if(snd_pcm_state(m_snd_handler) == SND_PCM_STATE_PAUSED){
        if(m_alsa_can_pause){
            rc = snd_pcm_pause(m_snd_handler, 0);
        }else {
            rc = snd_pcm_prepare(m_snd_handler);
        }
    }else {
        printf_alsa_state(m_device, snd_pcm_state(m_snd_handler));
        rc = -1;
    }

    if (rc < 0) {
        printf("%s, AlsaUtils unable to start: %s\n",m_device, snd_strerror(rc));
        return rc;
    }

    return rc;
}

int AlsaUtils::stop()
{
    if(m_type == Type::PLAYBACK) return 0;

    if(m_snd_handler == NULL) return -1;

    if(snd_pcm_state(m_snd_handler) == SND_PCM_STATE_PAUSED) return 0;
    if(snd_pcm_state(m_snd_handler) != SND_PCM_STATE_RUNNING) {
        printf_alsa_state(m_device, snd_pcm_state(m_snd_handler));
        return -1;
    }

    int rc;
    if(m_alsa_can_pause){
        rc = snd_pcm_pause(m_snd_handler, 1);
    }else {
        rc = snd_pcm_drop(m_snd_handler);
    }

    if (rc < 0) {
        printf("%s, AlsaUtils unable to stop: %s\n", m_device, snd_strerror(rc));
        return rc;
    }

    return rc; //TODO ??
}

}
