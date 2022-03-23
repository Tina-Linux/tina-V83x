#ifndef __NON_INTERLEAVED_CONVERTOR_H__
#define __NON_INTERLEAVED_CONVERTOR_H__

#include <memory>
#include "utils/AWResampleUtils.h"
#include "recorder/ConvertorInterface.h"

namespace AW {

class NonInterleavedConvertor : public ConvertorInterface
{
public:
    static std::shared_ptr<ConvertorInterface> create(int convert_sample_block,
                                               int in_sample_rate,
                                               int in_channles,
                                               int in_sample_bits,
                                               int out_sample_rate,
                                               int out_sample_bits);
    ~NonInterleavedConvertor();

    int convert(char *data, int samples);
    int setChannelMap(int ogrigin_channel, int map_channel) { return -1; };
    int getChannelData(int channel, char **data);

private:
    NonInterleavedConvertor(int convert_sample_block,
                         int in_sample_rate,
                         int in_channles,
                         int in_sample_bits,
                         int out_sample_rate,
                         int out_sample_bits);

    int rate_convert(char *in, int samples, char **out);
    int bits_convert(char *data, int samples);

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
