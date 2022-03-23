#ifndef __PLATFORM_RECORDER_H__
#define __PLATFORM_RECORDER_H__

#include <memory>
#include <string>
#include <json-c/json.h>

#include "recorder/ConvertorInterface.h"
#include "recorder/FilterInterface.h"
#include "recorder/RecorderInterface.h"

namespace AW {

class PlatformRecorder : public RecorderInterface
{
public:
    static std::shared_ptr<RecorderInterface> create(struct json_object *config, struct json_object *platform);
    ~PlatformRecorder();
    int init();
    int release();
    int fetch(std::shared_ptr<ConvertorInterface> &convertor, int samples);
    std::shared_ptr<ConvertorInterface> fetch(int samples);
    int start();
    int stop();

    std::shared_ptr<FilterInterface> getFilter() { return m_filter; }

private:
    PlatformRecorder(struct json_object *config, struct json_object *platform);

    int init_provider_by_name(const char *name, const std::string &create_timestamp);

    std::shared_ptr<FilterInterface> m_filter{nullptr};

    struct json_object *m_config_json{nullptr};
    struct json_object *m_platform_json{nullptr};

    std::shared_ptr<ConvertorInterface> m_output_convertor;
};

} // namespace AW

#endif /*__PLATFORM_RECORDER_H__*/
