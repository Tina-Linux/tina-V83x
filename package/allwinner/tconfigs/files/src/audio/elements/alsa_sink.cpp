#include "tconfigs/audio/elements/alsa_sink.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(AlsaSink)

std::shared_ptr<AlsaSink> AlsaSink::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto alsa_sink = std::shared_ptr<AlsaSink>(new AlsaSink(name));
    if (!alsa_sink || !alsa_sink->Init(config)) {
        return nullptr;
    }
    return alsa_sink;
}

AlsaSink::AlsaSink(const std::string& name)
    : AlsaElement(name)
{
}

bool AlsaSink::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "AlsaSink") {
        TCLOG_ERROR("\"%s\" is not a AlsaSink", name().c_str());
        return false;
    }

    const rapidjson::Value* devices_config = nullptr;
    if (!json::pointer::GetObject(config, "/devices", &devices_config)) {
        TCLOG_ERROR("AlsaSink \"%s\": cannot get config \"devices\"", name().c_str());
        return false;
    }

    for (rapidjson::Value::ConstMemberIterator itr = devices_config->MemberBegin();
            itr != devices_config->MemberEnd(); ++itr) {
        AlsaUtils::Parameters parameters;
        parameters.stream = AlsaUtils::StreamType::PLAYBACK;

        auto device = CreateAlsaDeviceFromConfig(devices_.size(), itr, &parameters);
        if (!device) {
            TCLOG_ERROR("AlsaSink \"%s\": device \"%s\": fail to create AlsaDevice",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        devices_.push_back(device);

        if (!SetSoftwareParametersFromConfig(device, itr)) {
            TCLOG_ERROR("AlsaSink \"%s\": device \"%s\": fail to set software parameters",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        if (!SetMessagesFromConfig(device, itr)) {
            TCLOG_ERROR("AlsaSink \"%s\": device \"%s\": fail to set messages",
                    name().c_str(), itr->name.GetString());
            return false;
        }
    }

    // TODO: maybe more than one devices are needed?
    if (devices_.size() != 1) {
        TCLOG_ERROR("AlsaSink \"%s\": number of devices is not equal to 1",
                name().c_str());
        return false;
    }

    const rapidjson::Value* pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/sink_pads", &pads_config)) {
        TCLOG_ERROR("AlsaSink \"%s\": cannot get config \"sink_pads\"", name().c_str());
        return false;
    }
    if (pads_config->Size() != devices_.size()) {
        TCLOG_ERROR("AlsaSink \"%s\": number of pads (%d) and devices (%lu) not matching",
                name().c_str(), pads_config->Size(), devices_.size());
        return false;
    }

    for (auto itr = devices_.begin(); itr != devices_.end(); ++itr) {
        BufferProperty property;
        if (!GetPropertyFromAlsaDevice(*itr, &property)) {
            TCLOG_ERROR("AlsaSink \"%s\": fail to get property from device \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }

        Pad::Direction dir = Pad::Direction::kSink;
        if (!AddPad((*itr)->name(), dir, shared_from_this())) {
            TCLOG_ERROR("AlsaSink \"%s\": fail to add sink pad named \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }
        auto pad = FindPad((*itr)->name(), dir);
        if (!pad) {
            TCLOG_ERROR("AlsaSink \"%s\": error while finding sink pad \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }
        pad->set_property(property);
    }

    if (!CreateExternalMessageSender()) {
        TCLOG_ERROR("AlsaSink \"%s\": fail to create external message sender",
                name().c_str());
        return false;
    }

    AddStateChangeMessageCallback(StateChangeDirection::kUp, State::kPaused, [&] {
        for (auto itr = devices_.begin(); itr != devices_.end(); ++itr) {
            (*itr)->utils()->Start();
        }
    });
    AddStateChangeMessageCallback(StateChangeDirection::kDown, State::kPlaying, [&] {
        for (auto itr = devices_.begin(); itr != devices_.end(); ++itr) {
            (*itr)->utils()->Pause();
        }
    });

    // TODO: add pull mode
    AddAvailableMode({Mode::kPush});

    return true;
}

bool AlsaSink::Activate(Mode mode)
{
    return ActivateDefault(mode);
}

bool AlsaSink::Deactivate(void)
{
    return DeactivateDefault();
}

int AlsaSink::PushChain(std::shared_ptr<Pad> pad)
{
    if (pad->index() != 0) {
        TCLOG_ERROR("AlsaSink \"%s\": incorrect pad index", name().c_str());
        return -1;
    }
    std::shared_ptr<AlsaDevice> device = devices_.front();
    std::shared_ptr<AlsaUtils> utils = device->utils();
    int ret = utils->WriteInterleaved(
            static_cast<char*>(pad->buffer()->data()),
            pad->buffer()->property()->frames());
    if (ret == -EPIPE) {
        TCLOG_WARNING("AlsaSink \"%s\": PCM \"%s\": underrun",
                name().c_str(), utils->device().c_str());
        PrintXrunInformation(utils);
        if (!device->xrun_message().content().empty()) {
            SendExternalMessage(device->xrun_message());
        }
        utils->Prepare();
    } else if (ret == -ESTRPIPE) {
        TCLOG_DEBUG("AlsaSink \"%s\": PCM \"%s\": Suspended. Trying resume.",
                name().c_str(), utils->device().c_str());
        while ((ret = utils->Resume()) == -EAGAIN) {
            TCLOG_DEBUG("AlsaSink \"%s\": PCM \"%s\": resume cannot be proceed "
                    "immediately. Try again", name().c_str(), utils->device().c_str());
            sleep(1);
        }
        if (ret < 0) {
            ret = utils->Prepare();
        }
        return ret;
    } else if (ret < 0) {
        TCLOG_ERROR("AlsaSink \"%s\": PCM \"%s\": write error (%s)",
                name().c_str(), utils->device().c_str(), AlsaUtils::ErrorString(ret));
        utils->Prepare();

        // TODO: whether return the negative number?
        return ret;
    }
    return 0;
}

} // namespace audio
} // namespace tconfigs
