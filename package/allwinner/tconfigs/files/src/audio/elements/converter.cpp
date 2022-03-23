#include "tconfigs/audio/elements/converter.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"
#include "tconfigs/common/string.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(Converter)

std::shared_ptr<Converter> Converter::Create(const std::string& name,
            const rapidjson::Value& config)
{
    auto converter = std::shared_ptr<Converter>(new Converter(name));
    if (!converter || !converter->Init(config)) {
        return nullptr;
    }
    return converter;
}

Converter::Converter(const std::string& name)
    : Element(name)
{
}

bool Converter::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "Converter") {
        TCLOG_ERROR("\"%s\" is not a Converter", name().c_str());
        return false;
    }

    if (!AddPadsFromConfig(config, Pad::Direction::kSrc)) {
        TCLOG_ERROR("Converter \"%s\": error while adding src pads", name().c_str());
        return false;
    }
    if (!AddPadsFromConfig(config, Pad::Direction::kSink)) {
        TCLOG_ERROR("Converter \"%s\": error while adding sink pads", name().c_str());
        return false;
    }

    if (pads(Pad::Direction::kSrc)->size() > kPadsMaxNumber
            || pads(Pad::Direction::kSink)->size() > kPadsMaxNumber) {
        TCLOG_ERROR("Converter \"%s\": the number of neithor src nor sink pads "
                "can be greater than %d", name().c_str(), kPadsMaxNumber);
        return false;
    }

    const rapidjson::Value* map_config = nullptr;
    if (!json::pointer::GetObject(config, "/channel_map", &map_config)) {
        TCLOG_ERROR("Converter \"%s\": cannot get config \"channel_map\"", name().c_str());
        return false;
    }
    if (!GetChannelMapFromConfig(*map_config)) {
        TCLOG_ERROR("Converter \"%s\": error while getting channel map from config",
                name().c_str());
        return false;
    }

    UpdatePropertyMask();

    // TODO: add Pull-Push mode
    AddAvailableMode({Mode::kPush, Mode::kPull});

    return true;
}

bool Converter::AddPadsFromConfig(const rapidjson::Value& config, Pad::Direction dir)
{
    std::string token;
    switch (dir) {
    case Pad::Direction::kSrc:
        token = "/src_pads";
        break;
    case Pad::Direction::kSink:
        token = "/sink_pads";
        break;
    default:
        return false;
    }

    const rapidjson::Value* pads_config = nullptr;
    if (!json::pointer::GetObject(config, token.c_str(), &pads_config)) {
        TCLOG_ERROR("Converter \"%s\": cannot get config \"%s\"",
                name().c_str(), token.substr(1).c_str());
        return false;
    }
    for (rapidjson::Value::ConstMemberIterator itr = pads_config->MemberBegin();
            itr != pads_config->MemberEnd(); ++itr) {
        if (!AddPad(itr->name.GetString(), dir, shared_from_this())) {
            TCLOG_ERROR("Converter \"%s\": fail to add pad named \"%s\"",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        BufferProperty property;
        if (!GetPadPropertyFromConfig(itr, &property)) {
            return false;
        }
        auto pad = FindPad(itr->name.GetString(), dir);
        if (!pad) {
            TCLOG_ERROR("Converter \"%s\": error while finding pad \"%s\"",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        pad->set_property(property);
    }
    return true;
}

bool Converter::GetPadPropertyFromConfig(
        rapidjson::Value::ConstMemberIterator config, BufferProperty* property)
{
    const char* content_string = nullptr;
    int content_int = 0;

    if (json::pointer::GetString(config->value, "/format", &content_string)) {
        BufferProperty::FormatType format =
            BufferProperty::StringToFormatType(content_string);
        if (format == BufferProperty::FormatType::UNKNOWN) {
            TCLOG_ERROR("Converter \"%s\": Pad \"%s\": unrecognized format \"%s\"",
                    name().c_str(), config->name.GetString(), content_string);
            return false;
        }
        property->set_format(format);
    }

    if (json::pointer::GetInt(config->value, "/channels", &content_int)) {
        property->set_channels(content_int);
    }

    if (json::pointer::GetString(config->value, "/storage", &content_string)) {
        BufferProperty::StorageType storage =
            BufferProperty::StringToStorageType(content_string);
        if (storage == BufferProperty::StorageType::kUnknown) {
            TCLOG_ERROR("Converter \"%s\": Pad \"%s\": unrecognized storage \"%s\"",
                    name().c_str(), config->name.GetString(), content_string);
            return false;
        }
        property->set_storage(storage);
    }

    return true;
}

bool Converter::GetChannelMapFromConfig(const rapidjson::Value& config)
{
    for (rapidjson::Value::ConstMemberIterator itr = config.MemberBegin();
            itr != config.MemberEnd(); ++itr) {
        std::string sink_pad_name, src_pad_name;
        int sink_channel_index, src_channel_index;
        if (!ParseChannelConfig(itr->name.GetString(),
                    &src_pad_name, &src_channel_index)
                || !ParseChannelConfig(itr->value.GetString(),
                    &sink_pad_name, &sink_channel_index)) {
            return false;
        }
        auto src_pad = FindPad(src_pad_name, Pad::Direction::kSrc);
        if (!src_pad) {
            TCLOG_ERROR("Converter \"%s\": error while finding src pad \"%s\"",
                    name().c_str(), src_pad_name.c_str());
            return false;
        }
        auto sink_pad = FindPad(sink_pad_name, Pad::Direction::kSink);
        if (!sink_pad) {
            TCLOG_ERROR("Converter \"%s\": error while finding sink pad \"%s\"",
                    name().c_str(), sink_pad_name.c_str());
            return false;
        }

        channel_map_.insert({
                GetUniqueIndex(src_pad->index(), src_channel_index),
                GetUniqueIndex(sink_pad->index(), sink_channel_index)});
    }
    return true;
}

bool Converter::ParseChannelConfig(const std::string& channel_config,
        std::string* pad_name, int* channel_index)
{
    auto separate_pos = channel_config.rfind('.');
    if (separate_pos == std::string::npos
            || separate_pos >= channel_config.size() - 1) {
        TCLOG_ERROR("Converter \"%s\": incorrect channel config \"%s\": lack of \'.\'",
                name().c_str(), channel_config.c_str());
        return false;
    }

    *pad_name = channel_config.substr(0, separate_pos);
    *channel_index = common::StringToInt(channel_config.substr(separate_pos + 1));
    return true;
}

uint32_t Converter::GetUniqueIndex(uint16_t pad_index, uint16_t channel_index)
{
    return static_cast<uint32_t>(pad_index) << 16 | static_cast<uint32_t>(channel_index);
}

uint16_t Converter::GetPadIndex(uint32_t unique_index)
{
    return static_cast<uint16_t>(0xffff & (unique_index >> 16));
}

uint16_t Converter::GetChannelIndex(uint32_t unique_index)
{
    return static_cast<uint16_t>(0xffff & unique_index);
}

uint32_t Converter::GetPadsTargetFlag(int num_pads)
{
#if 0
    uint32_t result = 0;
    for (int i = 0; i < num_pads; ++i) {
        result |= (0x1 << i);
    }
    return result;
#endif
    return (0x1 << num_pads) - 1;
}

bool Converter::AllPadsAreReady(int current_pad_index)
{
    pads_current_flag_ |= 0x1 << current_pad_index;
    if (pads_current_flag_ == pads_target_flag_) {
        pads_current_flag_ = 0;
        return true;
    }
    return false;
}

#define CONVERT_DATA_FORMAT_SAME(out_type, in_data, out_data, shift_offset, mask) \
do { \
    out_data = in_data; \
} while (0)

#define CONVERT_DATA_FORMAT_UP(out_type, in_data, out_data, shift_offset, mask) \
do { \
    out_data = ((out_type)(in_data) << shift_offset) & mask; \
} while (0)

#define CONVERT_DATA_FORMAT_DOWN(out_type, in_data, out_data, shift_offset, mask) \
do { \
    out_data = (out_type)(((in_data) >> shift_offset) & mask); \
} while (0)

#define CONVERT_ONE_CHANNEL_DATA( \
        in_type, in_data, in_sample_pos, in_channel, in_channels, \
        out_type, out_data, out_sample_pos, out_channel, out_channels, \
        convert_func, shift_offset, mask) \
do { \
    in_type* input = (in_type*)in_data; \
    out_type* output = (out_type*)out_data; \
    for (int frame = 0; frame < loop_frames_; ++frame) { \
        int in_pos = (this->*in_sample_pos)(frame, in_channel, loop_frames_, in_channels); \
        int out_pos = (this->*out_sample_pos)(frame, out_channel, loop_frames_, out_channels); \
        convert_func(out_type, input[in_pos], output[out_pos], shift_offset, mask); \
    } \
} while (0)

#define CONVERT_ONE_CHANNEL_DATA_BRIEF( \
        in_type, out_type, convert_func, shift_offset, mask) \
do { \
    CONVERT_ONE_CHANNEL_DATA( \
            in_type, sink_data, sink_sample_pos, sink_channel_index, sink_channels, \
            out_type, src_data, src_sample_pos, src_channel_index, src_channels, \
            convert_func, shift_offset, mask); \
} while (0)

#define CONVERT_ERROR_PRINT(in_format, out_format) \
do { \
    TCLOG_ERROR("Converter \"%s\": not supported convertion: %s -> %s", \
            name().c_str(), \
            BufferProperty::FormatTypeToString(in_format).c_str(), \
            BufferProperty::FormatTypeToString(out_format).c_str()); \
} while (0)

bool Converter::ConvertOneChannelData(std::shared_ptr<Pad> src_pad, int src_channel_index)
{
    int src_pad_index = src_pad->index();
    int src_unique_index = GetUniqueIndex(src_pad_index, src_channel_index);
    auto itr = channel_map_.find(src_unique_index);
    if (itr == channel_map_.end()) {
        TCLOG_ERROR("Converter \"%s\": error while getting the corresponding sink "
                "pad unique index from src pad unique index: %d "
                "(pad index: %d, channel index: %d)", name().c_str(),
                src_unique_index, src_pad_index, src_channel_index);
        return false;
    }
    int sink_pad_index = GetPadIndex(itr->second);
    int sink_channel_index = GetChannelIndex(itr->second);

    auto sink_pad = pads(Pad::Direction::kSink)->at(sink_pad_index);

    int src_channels = src_pad->buffer()->property()->channels();
    int sink_channels = sink_pad->buffer()->property()->channels();

    auto src_format = src_pad->buffer()->property()->format();
    auto sink_format = sink_pad->buffer()->property()->format();

    SamplePosFunc src_sample_pos =
        src_pad->buffer()->property()->storage()
            == BufferProperty::StorageType::kInterleaved ?
        &Converter::InterleavedSamplePos : &Converter::NoninterleavedSamplePos;
    SamplePosFunc sink_sample_pos =
        sink_pad->buffer()->property()->storage()
            == BufferProperty::StorageType::kInterleaved ?
        &Converter::InterleavedSamplePos : &Converter::NoninterleavedSamplePos;

    void* src_data = src_pad->buffer()->data();
    void* sink_data = sink_pad->buffer()->data();

    switch (sink_format) {
    case BufferProperty::FormatType::S16_LE:
        switch (src_format) {
        case BufferProperty::FormatType::S16_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int16_t, int16_t, CONVERT_DATA_FORMAT_SAME, 0, 0xffff);
            break;
        case BufferProperty::FormatType::S24_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int16_t, int32_t, CONVERT_DATA_FORMAT_UP, 8, 0xffffff00);
            break;
        case BufferProperty::FormatType::S32_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int16_t, int32_t, CONVERT_DATA_FORMAT_UP, 16, 0xffffffff);
            break;
        default:
            CONVERT_ERROR_PRINT(sink_format, src_format);
            return false;
        }
        break;
    case BufferProperty::FormatType::S24_LE:
        switch (src_format) {
        case BufferProperty::FormatType::S16_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int16_t, CONVERT_DATA_FORMAT_DOWN, 8, 0x0000ffff);
            break;
        case BufferProperty::FormatType::S24_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int32_t, CONVERT_DATA_FORMAT_SAME, 0, 0xffffffff);
            break;
        case BufferProperty::FormatType::S32_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int32_t, CONVERT_DATA_FORMAT_UP, 8, 0xffffff00);
            break;
        default:
            CONVERT_ERROR_PRINT(sink_format, src_format);
            return false;
        }
        break;
    case BufferProperty::FormatType::S32_LE:
        switch (src_format) {
        case BufferProperty::FormatType::S16_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int16_t, CONVERT_DATA_FORMAT_DOWN, 16, 0x0000ffff);
            break;
        case BufferProperty::FormatType::S24_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int32_t, CONVERT_DATA_FORMAT_DOWN, 8, 0xffffffff);
            break;
        case BufferProperty::FormatType::S32_LE:
            CONVERT_ONE_CHANNEL_DATA_BRIEF(
                    int32_t, int32_t, CONVERT_DATA_FORMAT_SAME, 0, 0xffffffff);
            break;
        default:
            CONVERT_ERROR_PRINT(sink_format, src_format);
            return false;
        }
        break;
    default:
        CONVERT_ERROR_PRINT(sink_format, src_format);
        return false;
    }

    return true;
}

inline int Converter::InterleavedSamplePos(
        int frame_index, int channel_index, int frames, int channels)
{
    return frame_index * channels + channel_index;
}

inline int Converter::NoninterleavedSamplePos(
        int frame_index, int channel_index, int frames, int channels)
{
    return channel_index * frames + frame_index;
}

bool Converter::Activate(Mode mode)
{
    int num_pads = 0;
    switch (mode) {
    case Mode::kPush:
        num_pads = pads(Pad::Direction::kSink)->size();
        break;
    case Mode::kPull:
        num_pads = pads(Pad::Direction::kSrc)->size();
        break;
    default:
        TCLOG_ERROR("Converter \"%s\": not supported mode", name().c_str());
        return false;
    }
    pads_target_flag_ = GetPadsTargetFlag(num_pads);
    pads_current_flag_ = 0;

    auto src_pads = pads(Pad::Direction::kSrc);
    if (src_pads->size() <= 0) {
        TCLOG_ERROR("Converter \"%s\": has no src pads", name().c_str());
        return false;
    }
    loop_frames_ = src_pads->front()->buffer()->property()->frames();
    rate_ = src_pads->front()->buffer()->property()->rate();

    return ActivateDefault(mode);
}

bool Converter::Deactivate(void)
{
    return DeactivateDefault();
}

int Converter::PushChain(std::shared_ptr<Pad> pad)
{
    if (!AllPadsAreReady(pad->index())) {
        return 0;
    }

    auto src_pads = pads(Pad::Direction::kSrc);
    for (auto itr = src_pads->begin(); itr != src_pads->end(); ++itr) {
        int channels = (*itr)->buffer()->property()->channels();
        for (int i = 0; i < channels; ++i) {
            if (!ConvertOneChannelData(*itr, i)) {
                TCLOG_ERROR("Converter \"%s\": pad \"%s\": channel %d: error while"
                        "converting data", name().c_str(), (*itr)->name().c_str(), i);
                return -1;
            }
        }
        int ret = (*itr)->Push();
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

int Converter::PullChain(std::shared_ptr<Pad> pad)
{
    if (!AllPadsAreReady(pad->index())) {
        return 0;
    }

    auto sink_pads = pads(Pad::Direction::kSink);
    for (auto itr = sink_pads->begin(); itr != sink_pads->end(); ++itr) {
        int ret = (*itr)->Pull();
        if (ret < 0) {
            return -1;
        }
    }
    auto src_pads = pads(Pad::Direction::kSrc);
    for (auto itr = src_pads->begin(); itr != src_pads->end(); ++itr) {
        int channels = (*itr)->buffer()->property()->channels();
        for (int i = 0; i < channels; ++i) {
            if (!ConvertOneChannelData(*itr, i)) {
                TCLOG_ERROR("Converter \"%s\": pad \"%s\": channel %d: error while"
                        "converting data", name().c_str(), (*itr)->name().c_str(), i);
                return -1;
            }
        }
    }

    return 0;
}

} // namespace audio
} // namespace tconfigs {
