#ifndef __RECORDER_AC108_PROVIDER_H__
#define __RECORDER_AC108_PROVIDER_H__

#include <memory>
#include <json-c/json.h>
#include "utils/AlsaUtils.h"
#include "recorder/ProviderInterface.h"

namespace AW {

class AC108Provider : public ProviderInterface
{
public:
    enum class AC108Mode
    {
        ENCODE,
        NORMAL
    };
    static std::shared_ptr<ProviderInterface> create();

    ~AC108Provider();

    int init(struct json_object *config);
    int release();
    int start();
    int stop();
    int fetch(char **data, int samples);
    int fetch(char *data, int samples) { return -1; };
private:
    AC108Provider();
    int mode_init();
    void mode_release();

    int normal_fetch(char **data, int samples);
    int encode_fetch(char **data, int samples);
protected:
    AC108Mode m_mode{AC108Mode::NORMAL};
private:
    int m_period_size;
    int m_period;
    const char *m_device;

    char *m_recorde_buf;
    char *m_recorde_buf_start;
    int m_sample_size;
    int m_start_channel;
    int m_offset;
    int m_status;
    int m_fetch_samples;

    AlsaUtils m_alsa_utils;


};
} // namespace AW

#endif /*__RECORDER_AC108_PROVIDER_H__*/
