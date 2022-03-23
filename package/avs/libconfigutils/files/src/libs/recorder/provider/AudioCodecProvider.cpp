#include <string.h>
#include <iostream>

#include "recorder/provider/AudioCodecProvider.h"
#include "utils/JsonUtils.h"

namespace AW {

std::shared_ptr<ProviderInterface> AudioCodecProvider::create()
{
    return std::shared_ptr<ProviderInterface>(new AudioCodecProvider());
}

AudioCodecProvider::AudioCodecProvider()
{
    m_sample_rate = 0;
    m_channels = 0;
    m_sample_bits = 0;
    m_period_size = 0;
    m_period = 0;
    m_device = nullptr;

    m_fetch_mode = FetchMode::FETCH_FILL_MEMORY;
}

AudioCodecProvider::~AudioCodecProvider()
{
    stop();
    release();
}

int AudioCodecProvider::init(struct json_object *config)
{
    struct json_object *j_value;

    if(!JsonUtils::json_object_object_get_ex(config, "name", &j_value)) return -1;
    m_name = JsonUtils::json_object_get_string(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "device", &j_value)) return -1;
    m_device = JsonUtils::json_object_get_string(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "channels", &j_value)) return -1;
    m_channels = JsonUtils::json_object_get_int(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "period-size", &j_value)) return -1;
    m_period_size = JsonUtils::json_object_get_int(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "period", &j_value)) return -1;
    m_period = JsonUtils::json_object_get_int(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "sample-bits", &j_value)) return -1;
    m_sample_bits = JsonUtils::json_object_get_int(j_value);

    if(!JsonUtils::json_object_object_get_ex(config, "sample-rate", &j_value)) return -1;
    m_sample_rate = JsonUtils::json_object_get_int(j_value);

    std::cout << "AudioCodec init device:   " << m_device << std::endl;
    std::cout << "                rate:     " << m_sample_rate << std::endl;
    std::cout << "                channels: " << m_channels << std::endl;
    std::cout << "                period_size: " << m_period_size << std::endl;
    std::cout << "                period:   " << m_period << std::endl;
    std::cout << "                sample_bits: " << m_sample_bits << std::endl;

    if(JsonUtils::json_object_object_get_ex(config, "output-data-file", &j_value)) {
        const char * dir = JsonUtils::json_object_get_string(j_value);
        if(strcmp(dir, "") != 0) enable_provider_data_saving(dir);
    }

    return m_alsa_utils.init(m_device, m_sample_rate, m_channels, m_sample_bits, m_period_size, m_period);;
}

int AudioCodecProvider::fetch(char *data, int samples)
{
    return m_alsa_utils.fetch(data, samples);;
}

int AudioCodecProvider::release()
{
    return m_alsa_utils.release();
}

int AudioCodecProvider::start()
{
    return m_alsa_utils.start();
}

int AudioCodecProvider::stop()
{
    return m_alsa_utils.stop();
}


} // namespace AW
