#ifndef __TCONFIGS_AUDIO_COMMON_ELEMENT_FACTORY_H__
#define __TCONFIGS_AUDIO_COMMON_ELEMENT_FACTORY_H__

#include <map>
#include <functional>
#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

#define REGISTER_ELEMENT(element_type) \
    static std::shared_ptr<Element> Create##element_type(const std::string& name, \
            const rapidjson::Value& config) { \
        return element_type::Create(name, config); \
    } \
    class element_type##Registration { \
    public: \
        element_type##Registration() { \
            ElementFactory::RegisterElement(#element_type, Create##element_type); \
        } \
    }; \
    static const element_type##Registration k##element_type##Registration;

class ElementFactory {
public:
    static std::shared_ptr<Element> CreateElement(
            const std::string& element_type, const rapidjson::Value& element_config);

    static void RegisterElement(const std::string& element_type,
            std::function<std::shared_ptr<Element> (const std::string&, const rapidjson::Value&)>
                element_create_func);

private:
    static std::map<std::string,
            std::function<std::shared_ptr<Element> (const std::string&, const rapidjson::Value&)>>
        factory_map_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_ELEMENT_FACTORY_H__ */
