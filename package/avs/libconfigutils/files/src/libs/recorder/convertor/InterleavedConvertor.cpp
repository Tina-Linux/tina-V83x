#include <string.h>

#include "recorder/convertor/InterleavedConvertor.h"

#define CHANNEL_BIT_CHANNGE(data, samples, in_type, out_type, direction, offset, factor) \
do {\
    int channel, sample; \
    in_type *ogirin = (in_type *)data; \
    out_type *dest = (out_type *)m_channel_data; \
    for (channel = 0; channel < m_in_channels; channel++) { \
        int map_channel = m_channel_map[channel];\
        if(map_channel == -1) continue; \
        for (sample = 0; sample < samples; ++sample) { \
            dest[samples * map_channel + sample] = (ogirin[sample * m_in_channels + channel] direction offset) & factor; \
        } \
    } \
}while(0)

namespace AW {

std::shared_ptr<ConvertorInterface> InterleavedConvertor::create(
                                    int convert_sample_block,
                                    int in_sample_rate,
                                    int in_channles,
                                    int in_sample_bits,
                                    int out_sample_rate,
                                    int out_sample_bits)
{
    return std::shared_ptr<InterleavedConvertor>(new InterleavedConvertor(
                                    convert_sample_block,
                                    in_sample_rate,
                                    in_channles,
                                    in_sample_bits,
                                    out_sample_rate,
                                    out_sample_bits));
}

InterleavedConvertor::~InterleavedConvertor()
{
    if(m_channel_data) {
        free(m_channel_data);
        m_channel_data = nullptr;
    }
}

int InterleavedConvertor::need_resample(char *in, int samples, char **out)
{
    //create resampler
    if(m_resample_utils == nullptr) {
        m_resample_utils = AWResampleUtils::create(m_convert_sample_block,
                                                   m_in_sample_bits,
                                                   m_in_channels,
                                                   m_in_sample_rate,
                                                   m_out_sample_rate);

        if(m_resample_utils == nullptr) return -1;
    }

    return m_resample_utils->resample(in, samples, out);
}

int InterleavedConvertor::need_separate_and_bits_channge(char *data, int samples)
{
    if(m_channel_data == nullptr) {
        //refrech map channel
        int out_channels = 0;
        for (int channel = 0; channel < m_in_channels; channel++) {
            if(m_channel_map[channel] != -1) out_channels++;
        }
        if(out_channels > 0) {
            m_out_channels = out_channels;
        }else{
            for (int channel = 0; channel < m_in_channels; channel++) {
                m_channel_map[channel] = channel;
            }
        }

        m_channel_data = (char*)malloc(m_convert_sample_block * m_out_channels * m_out_sample_bits/8);
    }

    if(m_in_sample_bits == 16 && m_out_sample_bits == 16) {
        //tested
        CHANNEL_BIT_CHANNGE(data, samples, signed short, signed short, <<, 0, 0xFFFFFFFF);
    }else if(m_in_sample_bits == 32 && m_out_sample_bits == 32) {
        //tested
        CHANNEL_BIT_CHANNGE(data, samples, int, int, <<, 0, 0xFFFFFFFF);
    }else if(m_in_sample_bits == 16 && m_out_sample_bits == 32) {
        //tested
        CHANNEL_BIT_CHANNGE(data, samples, signed short, int, <<, 16, 0xFFFF0000);
    }else if(m_in_sample_bits == 32 && m_out_sample_bits == 16) {
        CHANNEL_BIT_CHANNGE(data, samples, int, signed short, >>, 16, 0xFFFFFFFF);
    }else {
        printf("bits change(%d->%d) is not supported now\n", m_in_sample_bits, m_out_sample_bits);
        return -1;
    }
    return samples;
}

int InterleavedConvertor::need_bits_channge(char *data, int samples)
{
    if(m_in_sample_bits != m_out_sample_bits) {

        return 1;
    }
    return 0;
}

int InterleavedConvertor::convert(char *data, int samples)
{
    if(data == nullptr || samples == 0) {
        printf("input param error, data: %p, samples: %d\n",data ,samples);
        return -1;
    }
    if(m_in_sample_rate == m_out_sample_rate && samples > m_convert_sample_block) {
        printf("samples is out of size\n");
        return -1;
    }

    //need resample?
    int ret = samples;
    char *new_data = data;
    if(m_in_sample_rate != m_out_sample_rate) {
        ret = need_resample(data, samples, &new_data);
        if(ret == -1) return ret;
    }

    return need_separate_and_bits_channge(new_data, ret);;
}

int InterleavedConvertor::getChannelData(int channel, char **data)
{
    if(m_channel_data == nullptr) return -1;

    if(channel >= m_out_channels) {
        printf("getChannelData failed, total channel: %d, get channel: %d\n",m_out_channels, channel);
        return -1;
    }

    *data = m_channel_data + channel * m_convert_sample_block * m_out_sample_bits/8;

    return m_convert_sample_block;
}

int InterleavedConvertor::setChannelMap(int ogrigin_channel, int map_channel)
{
    if(ogrigin_channel >= m_in_channels || map_channel >= m_in_channels) {
        printf("setChannelMap channel %d->%d error\n", ogrigin_channel, map_channel);
        return -1;
    }
    m_channel_map[ogrigin_channel] = map_channel;
    //TODO::check the map_channel, request:0,1,2...  not 0,5,8...

    printf("InterleavedConvertor channel map (%d->%d)\n", ogrigin_channel, map_channel);
    return 0;
}

InterleavedConvertor::InterleavedConvertor(
                    int convert_sample_block,
                    int in_sample_rate,
                    int in_channles,
                    int in_sample_bits,
                    int out_sample_rate,
                    int out_sample_bits)
{
    m_convert_sample_block = convert_sample_block;
    m_in_sample_rate = in_sample_rate;
    m_in_sample_bits = in_sample_bits;
    m_in_channels = in_channles;
    m_out_sample_bits = out_sample_bits;
    m_out_sample_rate = out_sample_rate;
    m_out_channels = m_in_channels;

    //m_wav_utils.create("/tmp/resample.wav", "wb", m_in_sample_bits, m_in_channels, m_out_sample_rate);
    m_channel_map = (int *)malloc(sizeof(int) * m_in_channels);
    for(int i = 0; i < m_in_channels; i++)
        m_channel_map[i] = -1;

    printf("InterleavedConvertor channels: (%d), rate:(%d->%d), bits:(%d->%d)\n", m_in_channels, m_in_sample_rate, m_out_sample_rate, m_in_sample_bits, m_out_sample_bits);
}

}
