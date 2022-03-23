#include <string.h>
#include <iostream>

#include "recorder/provider/AC108Provider.h"
#include "utils/JsonUtils.h"


static int check_channel(const char *buffer, int pos) {

    int channels[8];
    int i;
    for(i = 0; i < 8; i++){
        channels[i] = (buffer[pos + i*4] & 0x0F);
        printf("channels[%d] = %d ", pos + i*4, channels[i]);
    }
    printf("\n");

    if(channels[0] == 0 &&
        channels[1] == 1 &&
        channels[2] == 2 &&
        channels[3] == 3 &&
        channels[4] == 4 &&
        channels[5] == 5 &&
        channels[6] == 6 &&
        channels[7] == 7)
        return 0;

    return -1;
}

static int get_channel_and_skip_dirty(const char *buffer, int *channel) {
    int start_pos = 0;
    while(1){
        *channel = check_channel(buffer, start_pos);
        if(*channel != -1)  break;
        start_pos++;
    }
    start_pos -= 1;
    printf("start_pos %d, start channel %d\n", start_pos, *channel);
    return start_pos;
}
/*
static int handle_record_buffer(const char *buffer_in, int size_in, char *buffer_out, int size_out) {
    int start_pos = 0;
    int real_out_size = 0;
    while(start_pos < size_in && real_out_size < size_out) {
        buffer_out[real_out_size++] = buffer_in[start_pos+2];
        buffer_out[real_out_size++] = buffer_in[start_pos+3];
        start_pos += 4;
    }
    return real_out_size;
}
*/
namespace AW {

std::shared_ptr<ProviderInterface> AC108Provider::create()
{
    return std::shared_ptr<ProviderInterface>(new AC108Provider());
}

AC108Provider::AC108Provider()
{
    m_sample_rate = 0;
    m_channels = 0;
    m_sample_bits = 0;
    m_period_size = 0;
    m_period = 0;
    m_device = nullptr;

    m_recorde_buf = nullptr;
    m_recorde_buf_start = nullptr;
    m_sample_size = 0;
    m_start_channel = 0;
    m_offset = 0;
    m_status = 0;
    m_fetch_samples = 0;

    m_fetch_mode = FetchMode::FETCH_GET_MEMORY;
}

AC108Provider::~AC108Provider()
{
    stop();
    release();
}

int AC108Provider::init(struct json_object *config)
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

    if(!JsonUtils::json_object_object_get_ex(config, "mode", &j_value)) return -1;
    if(strcmp(JsonUtils::json_object_get_string(j_value), "encode") == 0) m_mode = AC108Mode::ENCODE;

    std::cout << "AC108 init, mode:     " << (m_mode == AC108Mode::ENCODE ? "encode":"normal") << std::endl;
    std::cout << "            device:   " << m_device << std::endl;
    std::cout << "            rate:     " << m_sample_rate << std::endl;
    std::cout << "            channels: " << m_channels << std::endl;
    std::cout << "            period_size: " << m_period_size << std::endl;
    std::cout << "            period:   " << m_period << std::endl;
    std::cout << "            sample_bits: " << m_sample_bits << std::endl;

    int ret = m_alsa_utils.init(m_device, m_sample_rate, m_channels, m_sample_bits, m_period_size, m_period);
    if(ret < 0) return ret;

    ret = mode_init();
    if(ret < 0) {
        //printf failed
    }

    if(JsonUtils::json_object_object_get_ex(config, "output-data-file", &j_value)) {
        const char * dir = JsonUtils::json_object_get_string(j_value);
        if(strcmp(dir, "") != 0) enable_provider_data_saving(dir);
    }

    return ret;
}
int AC108Provider::normal_fetch(char **data, int samples)
{
    m_fetch_samples = m_alsa_utils.fetch(m_recorde_buf, samples);

    *data = m_recorde_buf;
    return m_fetch_samples;
}

int AC108Provider::encode_fetch(char **data, int samples)
{
    //ENCODE mode
    if(m_offset > 0 && m_status == 1) {
        m_offset = m_sample_size - m_offset % m_sample_size;

        memcpy(m_recorde_buf, m_recorde_buf_start + m_fetch_samples*m_sample_size, m_offset);
        for(int i = 0; i < m_offset; i++){
            printf("[%d] = %02x ", i, m_recorde_buf[i]);
        }
        printf("\n");

        m_recorde_buf_start = m_recorde_buf;
        std::cout << "2nd buf_start:" << m_offset << std::endl;
        m_status = 2;
    }
    else if(m_offset > 0 && m_status == 2) {
        memcpy(m_recorde_buf, m_recorde_buf + m_fetch_samples*m_sample_size, m_offset); //??
        /*
        for(int i = 0; i < m_offset; i++){
            printf("[%d] = %02x ", i, m_recorde_buf[i]);
        }
        printf("\n");
        */
    }

    m_fetch_samples = m_alsa_utils.fetch(m_recorde_buf + m_offset, samples);
    if(m_fetch_samples <= 0){
        return m_fetch_samples;
    }

    if(m_status == 0){
        m_offset = get_channel_and_skip_dirty(m_recorde_buf, &m_start_channel);

        m_fetch_samples = (m_fetch_samples*m_sample_size - m_offset) / m_sample_size; //??

        std::cout << "1st samples:" << m_fetch_samples << " offset: " << m_offset << std::endl;
        m_status = 1;
        m_recorde_buf_start = m_recorde_buf + m_offset;
    }

    *data = m_recorde_buf_start;

    return m_fetch_samples;
}

int AC108Provider::fetch(char **data, int samples)
{
    //NORMAL mode
    if(m_mode == AC108Mode::NORMAL) {
        normal_fetch(data, samples);
    }else{
        encode_fetch(data, samples);
    }

    return m_fetch_samples;
}

int AC108Provider::release()
{
    mode_release();
    return m_alsa_utils.release();
}

int AC108Provider::start()
{
    m_status = 0;
    m_start_channel = 0;
    m_offset = 0;
    m_status = 0;
    m_fetch_samples = 0;

    return m_alsa_utils.start();
}

int AC108Provider::stop()
{
    return m_alsa_utils.stop();
}

int AC108Provider::mode_init()
{
    //ENCODE
    if(m_sample_bits == 24) m_sample_bits =32;

    //NORMAL m_sample_bits=16 do nothings

    m_sample_size = m_channels*(m_sample_bits/8);
    m_recorde_buf = (char*)malloc(m_sample_size*(m_period_size + 1));
    if(m_recorde_buf == nullptr) return -1;

    return 0;
}

void AC108Provider::mode_release()
{
    if(m_recorde_buf) {
        free(m_recorde_buf);
        m_recorde_buf = nullptr;
    }
}

} // namespace AW
