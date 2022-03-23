#ifndef __TCONFIGS_AUDIO_ELEMENTS_CONVERTER_H__
#define __TCONFIGS_AUDIO_ELEMENTS_CONVERTER_H__

#include <functional>
#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class Converter : public Element,
                  public std::enable_shared_from_this<Converter> {
public:
    static std::shared_ptr<Converter> Create(const std::string& name,
            const rapidjson::Value& config);

    ~Converter(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int PushChain(std::shared_ptr<Pad> pad) override;
    int PullChain(std::shared_ptr<Pad> pad) override;

private:
    typedef int (Converter::*SamplePosFunc)(
            int frame_index, int channel_index, int frames, int channels);

    Converter(void) = delete;
    Converter(const std::string& name);

    bool Init(const rapidjson::Value& config);

    bool AddPadsFromConfig(const rapidjson::Value& config, Pad::Direction dir);
    bool GetPadPropertyFromConfig(
            rapidjson::Value::ConstMemberIterator config, BufferProperty* property);
    bool GetChannelMapFromConfig(const rapidjson::Value& config);
    bool ParseChannelConfig(const std::string& channel_config,
            std::string* pad_name, int* channel_index);

    uint32_t GetUniqueIndex(uint16_t pad_index, uint16_t channel_index);
    uint16_t GetPadIndex(uint32_t unique_index);
    uint16_t GetChannelIndex(uint32_t unique_index);

    uint32_t GetPadsTargetFlag(int num_pads);
    bool AllPadsAreReady(int current_pad_index);

    bool ConvertOneChannelData(std::shared_ptr<Pad> src_pad, int src_channel_index);

    int InterleavedSamplePos(
            int frame_index, int channel_index, int frames, int channels);
    int NoninterleavedSamplePos(
            int frame_index, int channel_index, int frames, int channels);

    int loop_frames_ = 0;
    int rate_ = 0;

    // key: unique channel index in src pads
    // value: unique channel index in sink pads
    std::unordered_map<uint32_t, uint32_t> channel_map_;

    // Bitmap that indicates which src/sink pad's Chain function has been called.
    // If called, the bit of the corresponding pad index will be set to 1.
    uint32_t pads_current_flag_ = 0;
    uint32_t pads_target_flag_ = 0;

    static const int kPadsMaxNumber = 32;
};

} // namespace audio
} // namespace tconfigs {

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_CONVERTER_H__ */
