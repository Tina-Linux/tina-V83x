#ifndef __AUDIO_DATA_CONVERTOR_H__
#define __AUDIO_DATA_CONVERTOR_H__

#include <memory>

#include <aumixcom.h>
#include <doResample.h>

#include "utils/WavUtils.h"

namespace AW {

class AudioDataConvertor
{
public:
    enum class ChannelConvertType
    {
        CHANNEL_UNCHANGE,
        CHANNEL_SEPARATE
    };
    static std::shared_ptr<AudioDataConvertor> create(int convert_sample_block,
                                                      int in_sample_rate,
                                                      int in_channles,
                                                      int in_sample_bits,
                                                      int out_sample_rate,
                                                      ChannelConvertType type,
                                                      int out_sample_bits);
    ~AudioDataConvertor();

    int convert(char *data, int samples);
    int setChannelMap(int ogrigin_channel, int map_channel);
    int getChannelData(int channel, char **data);

private:
    AudioDataConvertor(int convert_sample_block,
                       int in_sample_rate,
                       int in_channles,
                       int in_sample_bits,
                       int out_sample_rate,
                       ChannelConvertType type,
                       int out_sample_bits);

    int need_resample(char *data, int samples);
    int need_separate_and_bits_channge(char *data, int samples);
    int need_bits_channge(char *data, int samples);

    char *m_channel_data{nullptr};

    int m_convert_sample_block;
    int m_in_sample_rate;
    int m_in_sample_bits;
    int m_in_channels;
    int m_out_sample_bits;
    int m_out_sample_rate;
    int m_out_channels;

    ChannelConvertType m_type;

    Resampler *m_resample_res{nullptr};
    ResCfg m_resample_cfg;
    char *m_resample_buf{nullptr};

    int *m_channel_map{nullptr};

    WavUtils m_wav_utils;
};

}

#endif
