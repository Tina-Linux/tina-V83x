#ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_SINK_H__
#define __TCONFIGS_AUDIO_ELEMENTS_ALSA_SINK_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/utils/alsa_utils.h"
#include "tconfigs/audio/elements/alsa_element.h"

namespace tconfigs {
namespace audio {

class AlsaSink : public AlsaElement,
                 public std::enable_shared_from_this<AlsaSink> {
public:
    static std::shared_ptr<AlsaSink> Create(const std::string& name,
            const rapidjson::Value& config);

    ~AlsaSink(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

//    int Loop(void) override;
    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    AlsaSink(void) = delete;
    AlsaSink(const std::string& name);

    bool Init(const rapidjson::Value& config);

    std::vector<std::shared_ptr<AlsaDevice>> devices_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_ALSA_SINK_H__ */
