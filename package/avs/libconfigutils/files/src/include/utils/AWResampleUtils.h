#ifndef __AW_RESAMPLE_UTILS_H__
#define __AW_RESAMPLE_UTILS_H__

#include <memory>

#include <aumixcom.h>
#include <doResample.h>

namespace AW {

class AWResampleUtils
{
public:
    static std::shared_ptr<AWResampleUtils> create(int samples_block, int in_sample_bits, int in_channel, int in_sample_rate, int out_sample_rate);

    ~AWResampleUtils();

    int resample(char *in, int samples, char **out);

private:
    AWResampleUtils(int samples_block, int in_sample_bits, int in_channel, int in_sample_rate, int out_sample_rate);
    int init();
    Resampler *m_resample_res{nullptr};
    ResCfg m_resample_cfg;
    char *m_resample_buf{nullptr};

    int m_sample_block;
    int m_sample_bits;
};
}
#endif
