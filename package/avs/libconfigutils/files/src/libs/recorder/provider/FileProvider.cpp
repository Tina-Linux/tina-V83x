#include <string.h>
#include <iostream>

#include "recorder/provider/FileProvider.h"
#include "utils/JsonUtils.h"

namespace AW {

std::shared_ptr<ProviderInterface> FileProvider::create()
{
    return std::shared_ptr<ProviderInterface>(new FileProvider());
}

FileProvider::FileProvider()
{
    m_fetch_mode = FetchMode::FETCH_FILL_MEMORY;
}

FileProvider::~FileProvider()
{
}

int FileProvider::init(struct json_object *config)
{
    struct json_object *j_value;

    if(!JsonUtils::json_object_object_get_ex(config, "format", &j_value)) return -1;
    m_format = JsonUtils::json_object_get_string(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "path", &j_value)) return -1;
    m_path = JsonUtils::json_object_get_string(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "name", &j_value)) return -1;
    m_name = JsonUtils::json_object_get_string(j_value);

    if(strcmp(m_format, "wav")) {
        printf("No wav file is not supported now!\n");
        return -1;
    }

    m_wav_utils.create(m_path, "rb");

    struct wav_header &header =  m_wav_utils.getWavHeader();

    m_sample_rate = header.sample_rate;
    m_channels = header.num_channels;
    m_sample_bits = header.bits_per_sample;
    m_total_samples = header.data_sz/(m_channels* m_sample_bits/8);
    printf("FileProvider, %s, sample_rate: %d, channels: %d, sample_bits:%d\n", m_path, m_sample_rate, m_channels, m_sample_bits);
    return 0;
}

int FileProvider::fetch(char *data, int samples)
{
    int fetch_sample = m_wav_utils.read(data, samples);
    if(fetch_sample > 0){
        m_total_fetch_samples += fetch_sample;
        printf("\r[%0.2f%%](%d/%d) ", (float)m_total_fetch_samples*100/(float)m_total_samples,m_total_fetch_samples,m_total_samples);
        fflush(stdout);
    }else if(fetch_sample == 0){
        printf("FileProvider Reach the end of the file!\n");
    }else {
        printf("FileProvider fetch error, ret : %d\n", fetch_sample);
    }
    if(fetch_sample < samples) fetch_sample = 0;
    return fetch_sample;
}

int FileProvider::release()
{
    m_wav_utils.release();
    return 0;
}


} // namespace AW
