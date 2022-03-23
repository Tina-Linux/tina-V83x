#ifndef __RECORDER_PROVIDER_INTERFACE_H__
#define __RECORDER_PROVIDER_INTERFACE_H__

#include <json-c/json.h>

#include "recorder/ConvertorInterface.h"
#include "utils/WavUtils.h"

namespace AW {

class ProviderInterface
{
public:
    enum class  FetchMode
    {
        FETCH_GET_MEMORY,
        FETCH_FILL_MEMORY
    };

    virtual ~ProviderInterface(){
        if(getFetchMode() == FetchMode::FETCH_FILL_MEMORY && m_fetch_data != nullptr){
            free(m_fetch_data);
            m_fetch_data = nullptr;
        }

        if(m_wav_utils != nullptr) {
            m_wav_utils->release();
            delete m_wav_utils;
        }
    };

    virtual int init(struct json_object *config) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int release() = 0;

    virtual int fetch(char *data, int samples) = 0;
    virtual int fetch(char **data, int samples) = 0;

    int fetch(std::shared_ptr<ConvertorInterface> converter, int samples){
        int get_samples;
        if(getFetchMode() == FetchMode::FETCH_GET_MEMORY) {
            get_samples = fetch(&m_fetch_data, samples);
            if(get_samples <= 0) return get_samples;
        }else if(getFetchMode() == FetchMode::FETCH_FILL_MEMORY){
            if(m_fetch_data == nullptr) {
                m_fetch_data_samples = samples;
                m_fetch_data = (char*)malloc(m_fetch_data_samples * m_channels * m_sample_bits / 8);
                if(m_fetch_data == nullptr) return -1;
            }
            if(samples > m_fetch_data_samples) {
                return -1;
            }
            get_samples = fetch(m_fetch_data, samples);
        }

        if(m_wav_utils != nullptr) m_wav_utils->write(m_fetch_data, get_samples);

        return converter->convert(m_fetch_data, get_samples);
    };

    FetchMode getFetchMode() { return m_fetch_mode; };
    int getSampleRate(){ return m_sample_rate; };
    int getChannels(){ return m_channels; };
    int getSampleBits() { return m_sample_bits; };
    const char *getName() { return m_name; };

    void setCreateTimestamp(const std::string &timestamp) { m_create_timestamp = timestamp; };
protected:

    void enable_provider_data_saving(std::string file_dir){
        if(!m_create_timestamp.empty())
            file_dir = file_dir + "-" + m_create_timestamp;
        file_dir = file_dir + ".wav";
        printf("ProviderInterface create provider data file: %s\n", file_dir.c_str());
        m_wav_utils = new WavUtils();
        m_wav_utils->create(file_dir, "wb", m_sample_bits, m_channels, m_sample_rate);
    };

    FetchMode m_fetch_mode;

    int m_sample_rate;
    int m_channels;
    int m_sample_bits;

    const char *m_name{nullptr};

    std::string m_create_timestamp;
private:
    char *m_fetch_data{nullptr};
    int m_fetch_data_samples{0};

    WavUtils *m_wav_utils{nullptr};
};

}
#endif /*__RECORDER_PROVIDER_INTERFACE_H__*/
