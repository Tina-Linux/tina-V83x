#include "tconfigs/audio/tutuclear/tutuclear_element.h"

#include <limits>
#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(TutuclearElement)

std::shared_ptr<TutuclearElement> TutuclearElement::Create(
        const std::string& name, const rapidjson::Value& config)
{
    auto tutuclear = std::shared_ptr<TutuclearElement>(new TutuclearElement(name));
    if (!tutuclear || !tutuclear->Init(config)) {
        return nullptr;
    }
    return tutuclear;
}

TutuclearElement::TutuclearElement(const std::string& name)
    : Element(name)
{
}

bool TutuclearElement::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    int content_int = 0;
    const rapidjson::Value* content_object = nullptr;

    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "TutuclearElement") {
        TCLOG_ERROR("\"%s\" is not a TutuclearElement", name().c_str());
        return false;
    }

    if (!json::pointer::GetString(config, "/prm_file", &content_string)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"prm_file\"",
                name().c_str());
        return false;
    }
    prm_file_ = content_string;

    if (!json::pointer::GetString(config, "/keyword_file", &content_string)) {
        TCLOG_DEBUG("TutuclearElement \"%s\": no \"keyword_file\" specified in config, "
                "not to detect keyword", name().c_str());
    } else {
        TCLOG_DEBUG("TutuclearElement \"%s\": \"keyword_file\" specified: %s",
                name().c_str(), content_string);
        keyword_file_ = content_string;
    }

    if (!json::pointer::GetString(config, "/format", &content_string)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"format\"",
                name().c_str());
        return false;
    }
    format_ = BufferProperty::StringToFormatType(content_string);

    if (!json::pointer::GetString(config, "/storage", &content_string)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"storage\"",
                name().c_str());
        return false;
    }
    storage_ = BufferProperty::StringToStorageType(content_string);

    if (!json::pointer::GetInt(config, "/rate", &content_int)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"rate\"",
                name().c_str());
        return false;
    }
    rate_ = content_int;

    if (!json::pointer::GetInt(config, "/loop_frames", &content_int)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"loop_frames\"",
                name().c_str());
        return false;
    }
    loop_frames_ = content_int;

    if (!PropertyIsAppropriate()) {
        TCLOG_ERROR("TutuclearElement \"%s\": property not appropriate "
                "(format: %s, storage: %s, rate: %d, loop_frames: %d)",
                name().c_str(), BufferProperty::FormatTypeToString(format_).c_str(),
                BufferProperty::StorageTypeToString(storage_).c_str(), rate_, loop_frames_);
        return false;
    }

    common_pad_ = CreatePadFromConfig(config, Pad::Direction::kSink, "common");
    if (!common_pad_) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to create pad \"common\" from config",
                name().c_str());
        return false;
    }
    reference_pad_ = CreatePadFromConfig(config, Pad::Direction::kSink, "reference");
    if (!reference_pad_) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to create pad \"reference\" from config",
                name().c_str());
        return false;
    }
    output_pad_ = CreatePadFromConfig(config, Pad::Direction::kSrc, "output");
    if (!output_pad_) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to create pad \"output\" from config",
                name().c_str());
        return false;
    }

    if (json::pointer::GetObject(config, "/keyword_detected_message", &content_object)) {
        Message::UpdateFromConfig(&keyword_detected_message_, *content_object);
    }

    if (!CreateExternalMessageSender()) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to create external message sender",
                name().c_str());
        return false;
    }

    UpdatePropertyMask();

    // TODO: add Pull & Pull-Push mode
    AddAvailableMode({Mode::kPush});

    const char* keyword_file_string = keyword_file_.empty() ? nullptr : keyword_file_.c_str();
    if (0 != wrapper_.Init(prm_file_.c_str(), keyword_file_string)) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to init tutuclear", name().c_str());
        return false;
    }

    return true;
}

bool TutuclearElement::PropertyIsAppropriate(void)
{
    return (rate_ == 16000
            && loop_frames_ == 160
            && (format_ == BufferProperty::FormatType::S16_LE
                || format_ == BufferProperty::FormatType::S32_LE)
            && storage_ == BufferProperty::StorageType::kNoninterleaved);
}

std::shared_ptr<Pad> TutuclearElement::CreatePadFromConfig(
        const rapidjson::Value& config, Pad::Direction dir,
        const std::string& pad_name)
{
    std::string token;
    switch (dir) {
    case Pad::Direction::kSrc:
        token = "/src_pads/" + pad_name;
        break;
    case Pad::Direction::kSink:
        token = "/sink_pads/" + pad_name;
        break;
    default:
        return nullptr;
    }

    const rapidjson::Value* pad_config = nullptr;
    if (!json::pointer::GetObject(config, token.c_str(), &pad_config)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"%s\"",
                name().c_str(), token.c_str());
        return nullptr;
    }

    if (!AddPad(pad_name, dir, shared_from_this())) {
        TCLOG_ERROR("TutuclearElement \"%s\": fail to add pad named \"%s\"",
                name().c_str(), pad_name.c_str());
        return nullptr;
    }

    BufferProperty property;
    int content_int = 0;
    if (!json::pointer::GetInt(*pad_config, "/channels", &content_int)) {
        TCLOG_ERROR("TutuclearElement \"%s\": cannot get config \"channels\" "
                "from pad \"%s\"", name().c_str(), pad_name.c_str());
        return nullptr;
    }
    property.set_channels(content_int);
    property.set_format(format_);
    property.set_storage(storage_);
    property.set_rate(rate_);
    property.set_frames(loop_frames_);
    auto pad = FindPad(pad_name, dir);
    if (!pad) {
        TCLOG_ERROR("TutuclearElement \"%s\": error while finding pad \"%s\"",
                name().c_str(), pad_name.c_str());
        return nullptr;
    }
    pad->set_property(property);

    return pad;
}

uint32_t TutuclearElement::GetSinkPadsTargetFlag(int num_pads)
{
    uint32_t result = 0;
    for (int i = 0; i < num_pads; ++i) {
        result |= (0x1 << i);
    }
    return result;
}

bool TutuclearElement::AllSinkPadsAreReady(int current_pad_index)
{
    sink_pads_current_flag_ |= 0x1 << current_pad_index;
    if (sink_pads_current_flag_ == sink_pads_target_flag_) {
        sink_pads_current_flag_ = 0;
        return true;
    }
    return false;
}

bool TutuclearElement::Activate(Mode mode)
{
    switch (mode) {
    case Mode::kPush:
        sink_pads_target_flag_ = GetSinkPadsTargetFlag(
                pads(Pad::Direction::kSink)->size());
        sink_pads_current_flag_ = 0;
        break;
    default:
        TCLOG_ERROR("TutuclearElement \"%s\": not supported mode", name().c_str());
        return false;
    }

    return ActivateDefault(mode);
}

bool TutuclearElement::Deactivate(void)
{
    return DeactivateDefault();
}

#define TUTUCLEAR_PROCESS_ONE_FRAME(type) \
do { \
    wrapper_.ProcessOneFrame((type*)(common_pad_->buffer()->data()), \
            (type*)(reference_pad_->buffer()->data()), \
            (type*)(output_pad_->buffer()->data())); \
} while (0)

int TutuclearElement::PushChain(std::shared_ptr<Pad> pad)
{
    if (!AllSinkPadsAreReady(pad->index())) {
        return 0;
    }

    switch (format_) {
    case BufferProperty::FormatType::S16_LE:
        TUTUCLEAR_PROCESS_ONE_FRAME(int16_t);
        break;
    case BufferProperty::FormatType::S32_LE:
        TUTUCLEAR_PROCESS_ONE_FRAME(int32_t);
        break;
    default:
        TCLOG_ERROR("TutuclearElement \"%s\": unrecognized format: %s",
                name().c_str(), BufferProperty::FormatTypeToString(format_).c_str());
        return -1;
    }

    if (wrapper_.KeywordIsDetected()) {
        TCLOG_INFO("TutuclearElement \"%s\": keyword detected", name().c_str());
        SendExternalMessage(keyword_detected_message_);
    }

    return output_pad_->Push();
}

} // namespace audio
} // namespace tconfigs

