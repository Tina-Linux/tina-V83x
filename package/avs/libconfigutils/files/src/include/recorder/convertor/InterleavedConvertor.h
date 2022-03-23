#ifndef __INTERLEAVED_CONVERTOR_H__
#define __INTERLEAVED_CONVERTOR_H__

#include <memory>
#include "utils/AWResampleUtils.h"
#include "recorder/ConvertorInterface.h"

namespace AW {

class InterleavedConvertor : public ConvertorInterface
{
public:
    static std::shared_ptr<ConvertorInterface> create(int convert_sample_block,
                                               int in_sample_rate,
                                               int in_channles,
                                               int in_sample_bits,
                                               int out_sample_rate,
                                               int out_sample_bits);
    ~InterleavedConvertor();

    int convert(char *data, int samples);
    int setChannelMap(int ogrigin_channel, int map_channel);
    int getChannelData(int channel, char **data);

private:
    InterleavedConvertor(int convert_sample_block,
                         int in_sample_rate,
                         int in_channles,
                         int in_sample_bits,
                         int out_sample_rate,
                         int out_sample_bits);

    int need_resample(char *in, int samples, char **out);
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

    int *m_channel_map{nullptr};

    std::shared_ptr<AWResampleUtils> m_resample_utils{nullptr};
};
}
#endif
