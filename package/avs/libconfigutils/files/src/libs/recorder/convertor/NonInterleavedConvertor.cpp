#include <string.h>

#include "recorder/convertor/NonInterleavedConvertor.h"

#define CHANNEL_BIT_CHANNGE(data, samples, in_type, out_type, direction, offset, factor) \
do {\
    int sample; \
    in_type *ogirin = (in_type *)data; \
    out_type *dest = (out_type *)m_channel_data; \
    for(sample = 0; sample < samples * m_in_channels; sample++) { \
        dest[sample] = (ogirin[sample] direction offset) & factor; \
    } \
}while(0)

namespace AW {

std::shared_ptr<ConvertorInterface> NonInterleavedConvertor::create(
                                    int convert_sample_block,
                                    int in_sample_rate,
                                    int in_channles,
                                    int in_sample_bits,
                                    int out_sample_rate,
                                    int out_sample_bits)
{
    return std::shared_ptr<NonInterleavedConvertor>(new NonInterleavedConvertor(
                                    convert_sample_block,
                                    in_sample_rate,
                                    in_channles,
                                    in_sample_bits,
                                    out_sample_rate,
                                    out_sample_bits));
}

NonInterleavedConvertor::~NonInterleavedConvertor()
{
    if(m_channel_data) {
        free(m_channel_data);
        m_channel_data = nullptr;
    }
}

int NonInterleavedConvertor::rate_convert(char *in, int samples, char **out)
{

    return samples;
}

int NonInterleavedConvertor::bits_convert(char *data, int samples)
{
    if(m_channel_data == nullptr) {
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

int NonInterleavedConvertor::convert(char *data, int samples)
{
    int ret = 0;
    if(data == nullptr || samples == 0) {
        printf("input param error, data: %p, samples: %d\n",data ,samples);
        return -1;
    }

    if(m_in_sample_rate == m_out_sample_rate && m_in_sample_bits == m_out_sample_bits) {
        m_channel_data = data;
        return samples;
    }

    if(samples > m_convert_sample_block){
        printf("samples is out of size\n");
        return -1;
    }
    if(m_in_sample_rate != m_out_sample_rate) {
        //do some resample
        printf("NonInterleavedConvertor is not support resample now!\n");
        return -1;
    }

    if(m_in_sample_bits != m_out_sample_bits) {
        ret = bits_convert(data, samples);
        //printf("NonInterleavedConvertor is not support bit convert now!\n");
        //return -1;
    }

    return ret;
}

int NonInterleavedConvertor::getChannelData(int channel, char **data)
{
    if(channel >= m_out_channels) {
        printf("getChannelData failed, total channel: %d, get channel: %d\n",m_out_channels, channel);
        return -1;
    }

    if(m_in_sample_rate == m_out_sample_rate && m_in_sample_bits == m_out_sample_bits) {
        *data = m_channel_data + channel * m_convert_sample_block * m_in_channels * m_out_sample_bits/8;
        return m_convert_sample_block;
    }

    if(m_channel_data == nullptr) return -1;


    *data = m_channel_data + channel * m_convert_sample_block * m_out_sample_bits/8;

    return m_convert_sample_block;
}

NonInterleavedConvertor::NonInterleavedConvertor(
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


    printf("NonInterleavedConvertor channels: (%d), rate:(%d->%d), bits:(%d->%d)\n", m_in_channels, m_in_sample_rate, m_out_sample_rate, m_in_sample_bits, m_out_sample_bits);
}

}
