#ifndef __TCONFIGS_AUDIO_ELEMENTS_TEE_H__
#define __TCONFIGS_AUDIO_ELEMENTS_TEE_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class Tee : public Element,
            public std::enable_shared_from_this<Tee> {

    // TODO:
    //  Currently the sink pad and all the src pads in Tee use the same buffer.
    //  Need to add buffer data copying? Or use Converter alternatively?

public:
    static std::shared_ptr<Tee> Create(const std::string& name,
            const rapidjson::Value& config);
    static std::shared_ptr<Element> CreateElement(const std::string& name,
            const rapidjson::Value& config);
    ~Tee(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    Tee(void) = delete;
    Tee(const std::string& name);

    bool Init(const rapidjson::Value& config);

    std::shared_ptr<Pad> sink_pad_ = nullptr;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_TEE_H__ */
