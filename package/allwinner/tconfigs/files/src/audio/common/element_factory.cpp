#include "tconfigs/audio/common/element_factory.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

std::shared_ptr<Element> ElementFactory::CreateElement(
        const std::string& element_name, const rapidjson::Value& element_config)
{
    const char* type_string = nullptr;
    if (!json::pointer::GetString(element_config, "/element_type", &type_string)) {
        TCLOG_ERROR("Cannot get the type of element \"%s\" in config",
                element_name.c_str());
        return nullptr;
    }

    std::string type_name(type_string);
    auto itr = factory_map_.find(type_name);
    if (itr == factory_map_.end()) {
        TCLOG_ERROR("Not support creating element \"%s\" with ElementFactory",
                type_name.c_str());
        return nullptr;
    }
    return itr->second(element_name, element_config);
}

void ElementFactory::RegisterElement(const std::string& element_name,
        std::function<std::shared_ptr<Element> (const std::string&, const rapidjson::Value&)>
            element_create_func)
{
    factory_map_.insert({element_name, element_create_func});
}

std::map<std::string,
        std::function<std::shared_ptr<Element> (const std::string&, const rapidjson::Value&)>>
    ElementFactory::factory_map_;

} // namespace audio
} // namespace tconfigs
