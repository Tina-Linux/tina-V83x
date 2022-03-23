#include "tconfigs/audio/elements/common_sink.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(CommonSink)

std::shared_ptr<CommonSink> CommonSink::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto common_sink = std::shared_ptr<CommonSink>(new CommonSink(name));
    if (!common_sink|| !common_sink->Init(config)) {
        return nullptr;
    }
    return common_sink;
}

CommonSink::CommonSink(const std::string& name)
    : Element(name)
{
}

bool CommonSink::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;

    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "CommonSink") {
        TCLOG_ERROR("\"%s\" is not a CommonSink", name().c_str());
        return false;
    }

    const rapidjson::Value* pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/sink_pads", &pads_config)) {
        TCLOG_ERROR("CommonSink \"%s\": cannot get config \"sink_pads\"",
                name().c_str());
        return false;
    }
    for (rapidjson::Value::ConstMemberIterator itr = pads_config->MemberBegin();
            itr != pads_config->MemberEnd(); ++itr) {
        if (!AddPad(itr->name.GetString(), Pad::Direction::kSink, shared_from_this())) {
            TCLOG_ERROR("CommonSink \"%s\": fail to add pad named \"%s\"",
                    name().c_str(), itr->name.GetString());
            return false;
        }
    }

    if (json::pointer::GetString(config, "/data_got_signal", &content_string)) {
        AddSignal<void*, const BufferProperty*, Result*>(content_string);
        data_got_signal_ =
            FindSignal<void*, const BufferProperty*, Result*>(content_string);
        if (!data_got_signal_) {
            TCLOG_ERROR("CommonSink \"%s\": error while finding signal \"%s\"",
                    name().c_str(), content_string);
            return false;
        }
    }

    AddAvailableMode({Mode::kPush});

    return true;
}

bool CommonSink::Activate(Mode mode)
{
    return ActivateDefault(mode);
}

bool CommonSink::Deactivate(void)
{
    return DeactivateDefault();
}

int CommonSink::PushChain(std::shared_ptr<Pad> pad)
{
    Result result = Result::kNormal;
    if (data_got_signal_) {
        data_got_signal_->Emit(
                pad->buffer()->data(), pad->buffer()->property(), &result);
    }
    return result == Result::kNormal ? 0 : -1;
}

} // namespace audio
} // namespace tconfigs
