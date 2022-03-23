#include "tconfigs/audio/elements/tee.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

std::shared_ptr<Tee> Tee::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto tee = std::shared_ptr<Tee>(new Tee(name));
    if (!tee || !tee->Init(config)) {
        return nullptr;
    }
    return tee;
}

std::shared_ptr<Element> Tee::CreateElement(const std::string& name,
        const rapidjson::Value& config)
{
    return Create(name, config);
}

Tee::Tee(const std::string& name)
    : Element(name)
{
}

bool Tee::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "Tee") {
        TCLOG_ERROR("\"%s\" is not a Tee", name().c_str());
        return false;
    }

    const rapidjson::Value* sink_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/sink_pads", &sink_pads_config)) {
        TCLOG_ERROR("Tee \"%s\": cannot get config \"sink_pads\"", name().c_str());
        return false;
    }
    // Tee has only one sink pad
    if (sink_pads_config->Size() != 1) {
        TCLOG_ERROR("Tee \"%s\": the number of sink pads in config is not equal to 1",
                name().c_str());
        return false;
    }
    std::string sink_pad_name = sink_pads_config->MemberBegin()->name.GetString();
    if (!AddPad(sink_pad_name, Pad::Direction::kSink, shared_from_this())) {
        TCLOG_ERROR("Tee \"%s\": fail to add sink pad named \"%s\"",
                name().c_str(), sink_pad_name.c_str());
        return false;
    }
    sink_pad_ = FindPad(sink_pad_name, Pad::Direction::kSink);

    const rapidjson::Value* src_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/src_pads", &src_pads_config)) {
        TCLOG_ERROR("Tee \"%s\": cannot get config \"src_pads\"", name().c_str());
        return false;
    }
    for (rapidjson::Value::ConstMemberIterator itr = src_pads_config->MemberBegin();
            itr != src_pads_config->MemberEnd(); ++itr) {
        if (!AddPad(itr->name.GetString(), Pad::Direction::kSrc, shared_from_this())) {
            TCLOG_ERROR("Tee \"%s\": fail to add src pad named \"%s\"",
                    name().c_str(), itr->name.GetString());
            return false;
        }
    }

    // TODO: add Pull & Pull-Push mode
    AddAvailableMode({Mode::kPush});

    return true;
}

bool Tee::Activate(Mode mode)
{
    // TODO: share buffer
    std::shared_ptr<BufferInterface> sink_buffer = sink_pad_->buffer();
    auto src_pads = pads(Pad::Direction::kSrc);
    for (auto itr = src_pads->begin(); itr != src_pads->end(); ++itr) {
        (*itr)->LinkBuffer(sink_buffer);
        auto peer = (*itr)->peer();
        if (!peer) {
            TCLOG_ERROR("Tee \"%s\": src pad \"%s\": has no peer",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }
        peer->LinkBuffer(sink_buffer);
    }

    return ActivateDefault(mode);
}

bool Tee::Deactivate(void)
{
    return DeactivateDefault();
}

int Tee::PushChain(std::shared_ptr<Pad> pad)
{
    auto src_pads = pads(Pad::Direction::kSrc);
    for (auto itr = src_pads->begin(); itr != src_pads->end(); ++itr) {
        // TODO: not complete
    }
}

} // namespace audio
} // namespace tconfigs
