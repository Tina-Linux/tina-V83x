#ifndef __TCONFIGS_AUDIO_ELEMENTS_COMMON_SINK_H__
#define __TCONFIGS_AUDIO_ELEMENTS_COMMON_SINK_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class CommonSink : public Element,
                   public std::enable_shared_from_this<CommonSink> {
public:
    enum class Result {
        kNormal,
        kError,
    };

    static std::shared_ptr<CommonSink> Create(const std::string& name,
            const rapidjson::Value& config);

    ~CommonSink(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    CommonSink(void) = delete;
    CommonSink(const std::string& name);

    bool Init(const rapidjson::Value& config);

    std::shared_ptr<common::Signal<void*, const BufferProperty*, Result*>>
        data_got_signal_ = nullptr;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_COMMON_SINK_H__ */
