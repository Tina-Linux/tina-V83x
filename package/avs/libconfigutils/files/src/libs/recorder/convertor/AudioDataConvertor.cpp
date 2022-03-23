#include <string.h>

#include "recorder/convertor/AudioDataConvertor.h"

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

std::shared_ptr<AudioDataConvertor> AudioDataConvertor::create(
                                    int convert_sample_block,
                                    int in_sample_rate,
                                    int in_channles,
                                    int in_sample_bits,
                                    int out_sample_rate,
                                    ChannelConvertType type,
                                    int out_sample_bits)
{
    return std::shared_ptr<AudioDataConvertor>(new AudioDataConvertor(
                                    convert_sample_block,
                                    in_sample_rate,
                                    in_channles,
                                    in_sample_bits,
                                    out_sample_rate,
                                    type,
                                    out_sample_bits));
}

AudioDataConvertor::~AudioDataConvertor()
{
    if(m_channel_data) {
        free(m_channel_data);
        m_channel_data = nullptr;
    }
    if(m_resample_res){
        Destroy_Resampler(m_resample_res);
        m_resample_res = nullptr;
    }
    if(m_resample_buf) {
        free(m_resample_buf);
        m_resample_buf = nullptr;
    }



    //m_wav_utils.release();
}

int AudioDataConvertor::need_resample(char *data, int samples)
{
    //create resampler
    if(m_resample_res == nullptr) {
        m_resample_res = Creat_Resampler();
        if(m_resample_res == nullptr) {
            printf("Creat_Resampler failed!\n");
            exit(-1);
        }
        //check channel
        if(m_in_channels != 2) {
            printf("resample channel: %d, is not supported for the resampler", m_in_channels);
            return -1;
        }
        //check bits
        if(m_in_sample_bits != 16) {
            printf("resample sample bits: %d, is not supported for the resampler", m_in_sample_bits);
            return -1;
        }
        m_resample_cfg.inch   = m_in_channels;
        m_resample_cfg.insrt  = m_in_sample_rate;
        m_resample_cfg.outsrt = m_out_sample_rate;
        m_resample_buf = (char*)malloc(m_resample_cfg.inch * m_convert_sample_block * m_in_sample_bits/8);
        if(m_resample_buf == nullptr) {
            printf("malloc buffer failed!\n");
            exit(-1);
        }
    }

    //do resample
    m_resample_cfg.samples = samples;
    m_resample_cfg.inbuf   = data;
    m_resample_res->prepare(m_resample_res, &m_resample_cfg);
    int out_samples = m_resample_res->process(m_resample_res);
    //printf("%s %d, in_samples:%d, out_samples: %d\n", __func__,__LINE__,in_samples, out_samples);
    int offset = 0;
    if(out_samples < m_convert_sample_block) {
        printf("in samples: %d, resample(%d->%d), request out samples: %d, but now: %d, fill 0!\n",
                    samples, m_in_sample_rate, m_out_sample_rate, m_convert_sample_block, out_samples);
        int fill_sample = m_convert_sample_block - out_samples;
        offset = fill_sample * m_resample_cfg.inch * m_in_sample_bits/8;
        memset(m_resample_buf, 0, offset);
    }
    m_resample_res->getData(m_resample_res, m_resample_buf+offset, out_samples*m_resample_cfg.inch*m_in_sample_bits/8);

    //m_wav_utils.write(m_resample_buf, m_convert_sample_block);

    return m_convert_sample_block;
}

int AudioDataConvertor::need_separate_and_bits_channge(char *data, int samples)
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
        /*
        int channel, sample;
        signed short *ogirin = (signed short *)data;
        signed short *dest = (signed short *)m_channel_data;
        for (channel = 0; channel < m_in_channels; channel++) {
            for (sample = 0; sample < samples; sample++) {
                dest[samples * channel + sample] = ogirin[sample * m_in_channels + channel];
            }
        }
        */
    }else if(m_in_sample_bits == 32 && m_out_sample_bits == 32) {
        //tested
        CHANNEL_BIT_CHANNGE(data, samples, int, int, <<, 0, 0xFFFFFFFF);
        /*
        int channel, sample;
        int *ogirin = (int *)data;
        int *dest = (int *)m_channel_data;
        for (channel = 0; channel < m_in_channels; channel++) {
            int map_channel = m_channel_map[channel];
            printf("%s %d channel: %d,map_channel:%d\n", __func__,__LINE__,channel,map_channel);
            if(map_channel == -1) continue;
            for (sample = 0; sample < samples; sample++) {
                dest[samples * map_channel + sample] = ogirin[sample * m_in_channels + channel];
            }
        }
        */
    }else if(m_in_sample_bits == 16 && m_out_sample_bits == 32) {
        //tested
        CHANNEL_BIT_CHANNGE(data, samples, signed short, int, <<, 16, 0xFFFF0000);
        /*
        int channel, sample;
        signed short *ogirin = (signed short *)data;
        int *dest = (int *)m_channel_data;
        for (channel = 0; channel < m_in_channels; channel++) {
            for (sample = 0; sample < samples; ++sample) {
                dest[samples * channel + sample] = (ogirin[sample * m_in_channels + channel] << 16) & 0xFFFF0000;
            }
        }
        */
    }else if(m_in_sample_bits == 32 && m_out_sample_bits == 16) {
        CHANNEL_BIT_CHANNGE(data, samples, int, signed short, >>, 16, 0xFFFFFFFF);
        /*
        int channel, sample;
        int *ogirin = (int *)data;
        signed short *dest = (signed short *)m_channel_data;

        for (channel = 0; channel < m_in_channels; channel++) {
            for (sample = 0; sample < samples; ++sample) {
                dest[samples * channel + sample] = ogirin[sample * m_in_channels + channel] >> 16;
            }
        }
        */
    }else {
        printf("bits change(%d->%d) is not supported now\n", m_in_sample_bits, m_out_sample_bits);
        return -1;
    }
    return samples;
}

int AudioDataConvertor::need_bits_channge(char *data, int samples)
{
    if(m_in_sample_bits != m_out_sample_bits) {

        return 1;
    }
    return 0;
}

int AudioDataConvertor::convert(char *data, int samples)
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
        ret = need_resample(data, samples);
        if(ret == -1) return ret;
        new_data = m_resample_buf;
    }

    //need channel-separate and sample-bit change?
    if(m_type == ChannelConvertType::CHANNEL_SEPARATE)
        ret = need_separate_and_bits_channge(new_data, ret);
    else if(m_type == ChannelConvertType::CHANNEL_UNCHANGE && m_in_sample_bits != m_out_sample_bits){
        //only need change sample-bit
        printf("channel unchange and sample bits(%d->%d) is not supported now\n", m_in_sample_bits, m_out_sample_bits);
        return -1;
    }

    return ret;
}

int AudioDataConvertor::getChannelData(int channel, char **data)
{
    if(m_type == ChannelConvertType::CHANNEL_UNCHANGE || m_channel_data == nullptr) {
        return -1;
    }

    if(channel >= m_out_channels) {
        printf("getChannelData failed, total channel: %d, get channel: %d\n",m_out_channels, channel);
        return -1;
    }

    *data = m_channel_data + channel * m_convert_sample_block * m_out_sample_bits/8;

    return m_convert_sample_block;
}

int AudioDataConvertor::setChannelMap(int ogrigin_channel, int map_channel)
{
    if(ogrigin_channel >= m_in_channels || map_channel >= m_in_channels) {
        printf("setChannelMap channel %d->%d error\n", ogrigin_channel, map_channel);
        return -1;
    }
    m_channel_map[ogrigin_channel] = map_channel;
    //TODO::check the map_channel, request:0,1,2...  not 0,5,8...

    printf("AudioDataConvertor channel map (%d->%d)\n", ogrigin_channel, map_channel);
    return 0;
}

AudioDataConvertor::AudioDataConvertor(
                    int convert_sample_block,
                    int in_sample_rate,
                    int in_channles,
                    int in_sample_bits,
                    int out_sample_rate,
                    ChannelConvertType type,
                    int out_sample_bits)
{
    m_convert_sample_block = convert_sample_block;
    m_in_sample_rate = in_sample_rate;
    m_in_sample_bits = in_sample_bits;
    m_in_channels = in_channles;
    m_out_sample_bits = out_sample_bits;
    m_out_sample_rate = out_sample_rate;
    m_out_channels = m_in_channels;

    m_type = type;

    //m_wav_utils.create("/tmp/resample.wav", "wb", m_in_sample_bits, m_in_channels, m_out_sample_rate);
    m_channel_map = (int *)malloc(sizeof(int) * m_in_channels);
    for(int i = 0; i < m_in_channels; i++)
        m_channel_map[i] = -1;

    printf("AudioDataConvertor channels: (%d), rate:(%d->%d), bits:(%d->%d)\n", m_in_channels, m_in_sample_rate, m_out_sample_rate, m_in_sample_bits, m_out_sample_bits);
}

}
