#ifndef __RECORDER_FILE_PROVIDER_H__
#define __RECORDER_FILE_PROVIDER_H__

#include <memory>
#include <json-c/json.h>
#include "utils/WavUtils.h"
#include "recorder/ProviderInterface.h"

namespace AW {

class FileProvider : public ProviderInterface
{
public:
    static std::shared_ptr<ProviderInterface> create();

    ~FileProvider();

    int init(struct json_object *config);
    int release();
    int start(){ return 0; };
    int stop(){ return 0; };
    int fetch(char **data, int samples) { return -1; };
    int fetch(char *data, int samples);
private:
    FileProvider();

private:
    WavUtils m_wav_utils;

    const char *m_format{nullptr};
    const char *m_path{nullptr};

    struct wav_header m_header;

    int m_total_fetch_samples{0};
    int m_total_samples{0};
};
} // namespace AW

#endif /*__RECORDER_FILE_PROVIDER_H__*/
