#ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_SRC_H__
#define __TCONFIGS_AUDIO_ELEMENTS_ALSA_SRC_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/utils/alsa_utils.h"
#include "tconfigs/audio/elements/alsa_element.h"

namespace tconfigs {
namespace audio {

class AlsaSrc : public AlsaElement,
                public std::enable_shared_from_this<AlsaSrc> {
public:
    static std::shared_ptr<AlsaSrc> Create(const std::string& name,
            const rapidjson::Value& config);

    ~AlsaSrc(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int Loop(void) override;

private:
    AlsaSrc(void) = delete;
    AlsaSrc(const std::string& name);

    bool Init(const rapidjson::Value& config);

    int LoopOneDevice(void);

    std::vector<std::shared_ptr<AlsaDevice>> devices_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_SRC_H__ */
