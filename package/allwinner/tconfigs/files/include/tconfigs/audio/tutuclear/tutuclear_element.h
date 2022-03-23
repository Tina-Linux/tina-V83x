#ifndef __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_ELEMENT_H__
#define __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_ELEMENT_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"
#include "tconfigs/common/signal_slot.h"
#include "tconfigs/audio/tutuclear/tutuclear_wrapper.h"

namespace tconfigs {
namespace audio {

class TutuclearElement : public Element,
                         public std::enable_shared_from_this<TutuclearElement> {
public:
    static std::shared_ptr<TutuclearElement> Create(const std::string& name,
            const rapidjson::Value& config);

    ~TutuclearElement(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    TutuclearElement(void) = delete;
    TutuclearElement(const std::string& name);

    bool Init(const rapidjson::Value& config);

    bool PropertyIsAppropriate(void);
    std::shared_ptr<Pad> CreatePadFromConfig(
            const rapidjson::Value& config, Pad::Direction dir,
            const std::string& pad_name);

    uint32_t GetSinkPadsTargetFlag(int num_pads);
    bool AllSinkPadsAreReady(int current_pad_index);

    TutuclearWrapper wrapper_;
    std::string prm_file_;
    std::string keyword_file_;

    BufferProperty::FormatType format_ = BufferProperty::FormatType::UNKNOWN;
    BufferProperty::StorageType storage_ = BufferProperty::StorageType::kUnknown;
    int rate_ = 0;
    int loop_frames_ = 0;

    std::shared_ptr<Pad> common_pad_ = nullptr;
    std::shared_ptr<Pad> reference_pad_ = nullptr;
    std::shared_ptr<Pad> output_pad_ = nullptr;

    // Bitmap that indicates which sink pad's Chain function has been called.
    // If called, the bit of the corresponding pad index will be set to 1.
    uint32_t sink_pads_current_flag_ = 0;
    uint32_t sink_pads_target_flag_ = 0;

    Message keyword_detected_message_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_TUTUCLEAR_TUTUCLEAR_ELEMENT_H__ */
