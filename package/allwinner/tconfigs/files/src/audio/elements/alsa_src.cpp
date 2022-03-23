#include "tconfigs/audio/elements/alsa_src.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(AlsaSrc)

std::shared_ptr<AlsaSrc> AlsaSrc::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto alsa_src = std::shared_ptr<AlsaSrc>(new AlsaSrc(name));
    if (!alsa_src || !alsa_src->Init(config)) {
        return nullptr;
    }
    return alsa_src;
}

AlsaSrc::AlsaSrc(const std::string& name)
    : AlsaElement(name)
{
}

bool AlsaSrc::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "AlsaSrc") {
        TCLOG_ERROR("\"%s\" is not a AlsaSrc", name().c_str());
        return false;
    }

    const rapidjson::Value* devices_config = nullptr;
    if (!json::pointer::GetObject(config, "/devices", &devices_config)) {
        TCLOG_ERROR("AlsaSrc \"%s\": cannot get config \"devices\"", name().c_str());
        return false;
    }

    for (rapidjson::Value::ConstMemberIterator itr = devices_config->MemberBegin();
            itr != devices_config->MemberEnd(); ++itr) {
        AlsaUtils::Parameters parameters;
        parameters.stream = AlsaUtils::StreamType::CAPTURE;

        auto device = CreateAlsaDeviceFromConfig(devices_.size(), itr, &parameters);
        if (!device) {
            TCLOG_ERROR("AlsaSrc \"%s\": device \"%s\": fail to create AlsaDevice",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        devices_.push_back(device);

        if (!SetSoftwareParametersFromConfig(device, itr)) {
            TCLOG_ERROR("AlsaSrc \"%s\": device \"%s\": fail to set software parameters",
                    name().c_str(), itr->name.GetString());
            return false;
        }
        if (!SetMessagesFromConfig(device, itr)) {
            TCLOG_ERROR("AlsaSrc \"%s\": device \"%s\": fail to set messages",
                    name().c_str(), itr->name.GetString());
            return false;
        }
    }

    const rapidjson::Value* pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/src_pads", &pads_config)) {
        TCLOG_ERROR("AlsaSrc \"%s\": cannot get config \"src_pads\"", name().c_str());
        return false;
    }
    if (pads_config->Size() != devices_.size()) {
        TCLOG_ERROR("AlsaSrc \"%s\": number of pads (%d) and devices (%lu) not matching",
                name().c_str(), pads_config->Size(), devices_.size());
        return false;
    }

    for (auto itr = devices_.begin(); itr != devices_.end(); ++itr) {
        BufferProperty property;
        if (!GetPropertyFromAlsaDevice(*itr, &property)) {
            TCLOG_ERROR("AlsaSrc \"%s\": fail to get property from device \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }

        Pad::Direction dir = Pad::Direction::kSrc;
        if (!AddPad((*itr)->name(), dir, shared_from_this())) {
            TCLOG_ERROR("AlsaSrc \"%s\": fail to add src pad named \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }
        auto pad = FindPad((*itr)->name(), dir);
        if (!pad) {
            TCLOG_ERROR("AlsaSrc \"%s\": error while finding src pad \"%s\"",
                    name().c_str(), (*itr)->name().c_str());
            return false;
        }
        pad->set_property(property);
    }

    if (!CreateExternalMessageSender()) {
        TCLOG_ERROR("AlsaSrc \"%s\": fail to create external message sender",
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

    // TODO: add Pull mode
    AddAvailableMode({Mode::kPush});

    return true;
}

bool AlsaSrc::Activate(Mode mode)
{
    return ActivateDefault(mode);
}

bool AlsaSrc::Deactivate(void)
{
    return DeactivateDefault();
}

int AlsaSrc::LoopOneDevice(void)
{
    std::shared_ptr<AlsaDevice> device = devices_.front();
    std::shared_ptr<AlsaUtils> utils = device->utils();
    std::shared_ptr<Pad> pad = pads(Pad::Direction::kSrc)->front();
    int ret = utils->ReadInterleaved(
            static_cast<char*>(pad->buffer()->data()),
            pad->buffer()->property()->frames());
    if (ret == -EPIPE) {
        TCLOG_WARNING("AlsaSrc \"%s\": PCM \"%s\": overrun",
                name().c_str(), utils->device().c_str());
        PrintXrunInformation(utils);
        if (!device->xrun_message().content().empty()) {
            SendExternalMessage(device->xrun_message());
        }
        utils->Prepare();
    } else if (ret == -ESTRPIPE) {
        TCLOG_WARNING("AlsaSrc \"%s\": PCM \"%s\": Suspended. Trying resume.",
                name().c_str(), utils->device().c_str());
        while ((ret = utils->Resume()) == -EAGAIN) {
            TCLOG_DEBUG("AlsaSrc \"%s\": PCM \"%s\": resume cannot be proceed "
                    "immediately. Try again", name().c_str(), utils->device().c_str());
            sleep(1);
        }
        if (ret < 0) {
            ret = utils->Prepare();
        }
        return ret;
    } else if (ret < 0) {
        TCLOG_ERROR("AlsaSrc \"%s\": PCM \"%s\": read error (%s)",
                name().c_str(), utils->device().c_str(), AlsaUtils::ErrorString(ret));
        utils->Prepare();
        // TODO: whether return the negative number?
        return ret;
    }

    return pad->Push();
}

int AlsaSrc::Loop(void)
{
    if (devices_.size() == 1) {
        return LoopOneDevice();
    } else if (devices_.size() > 1) {
        // TODO: support multi devices
        TCLOG_ERROR("AlsaSrc \"%s\": not support multi devices", name().c_str());
        return -1;
    } else {
        TCLOG_ERROR("AlsaSrc \"%s\": number of devices (%lu) is not valid",
                name().c_str(), devices_.size());
        return -1;
    }
}

} // namespace audio
} // namespace tconfigs
