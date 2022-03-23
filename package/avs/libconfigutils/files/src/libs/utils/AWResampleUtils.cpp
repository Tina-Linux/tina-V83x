#include <string.h>

#include "utils/AWResampleUtils.h"

namespace AW {

std::shared_ptr<AWResampleUtils> AWResampleUtils::create(int samples_block,
                                                         int in_sample_bits,
                                                         int in_channel,
                                                         int in_sample_rate,
                                                         int out_sample_rate)
{
    auto utils = std::shared_ptr<AWResampleUtils>(new AWResampleUtils(samples_block,
                                                                      in_sample_bits,
                                                                      in_channel,
                                                                      in_sample_rate,
                                                                      out_sample_rate));

    if(utils->init() < 0) return nullptr;

    return utils;
}

AWResampleUtils::AWResampleUtils(int sample_block,
                                 int in_sample_bits,
                                 int in_channel,
                                 int in_sample_rate,
                                 int out_sample_rate)
{
    m_sample_block = sample_block;
    m_sample_bits = in_sample_bits;

    m_resample_cfg.insrt  = in_sample_rate;
    m_resample_cfg.inch   = in_channel;
    m_resample_cfg.outsrt = out_sample_rate;
}

AWResampleUtils::~AWResampleUtils()
{
    if(m_resample_buf) {
        free(m_resample_buf);
        m_resample_buf = nullptr;
    }
    if(m_resample_res){
        Destroy_Resampler(m_resample_res);
        m_resample_res = nullptr;
    }
}

int AWResampleUtils::init()
{
    //create resampler
    if(m_resample_res == nullptr) {
        m_resample_res = Creat_Resampler();
        if(m_resample_res == nullptr) {
            printf("Creat_Resampler failed!\n");
            return -1;
        }
        //check channel
        if(m_resample_cfg.inch != 2) {
            printf("resample channel: %d, is not supported for the resampler", m_resample_cfg.inch);
            return -1;
        }
        //check bits
        if(m_sample_bits != 16) {
            printf("resample sample bits: %d, is not supported for the resampler", m_sample_bits);
            return -1;
        }

        m_resample_buf = (char*)malloc(m_resample_cfg.inch * m_sample_block * m_sample_bits/8);
        if(m_resample_buf == nullptr) {
            printf("malloc buffer failed!\n");
            return -1;
        }
    }
    return 0;
}

int AWResampleUtils::resample(char *in, int samples, char **out)
{
    int request_samples = ((float)m_resample_cfg.outsrt / (float)m_resample_cfg.insrt) * samples;

    /*
    if(m_sample_block < samples) {
        //do some remalloc
    }
    */

    //do resample
    m_resample_cfg.samples = samples;
    m_resample_cfg.inbuf   = in;
    m_resample_res->prepare(m_resample_res, &m_resample_cfg);
    int out_samples = m_resample_res->process(m_resample_res);

    int offset = 0;
    if(out_samples < request_samples) {
        printf("in samples: %d, resample(%d->%d), request out samples: %d, but now: %d, fill 0!\n",
                    samples, m_resample_cfg.insrt, m_resample_cfg.outsrt, request_samples, out_samples);
        int fill_sample = request_samples - out_samples;
        offset = fill_sample * m_resample_cfg.inch * m_sample_bits/8;
        memset(m_resample_buf, 0, offset);
    }
    m_resample_res->getData(m_resample_res, m_resample_buf+offset, out_samples*m_resample_cfg.inch*m_sample_bits/8);

    *out = m_resample_buf;

    return request_samples;
}

}
