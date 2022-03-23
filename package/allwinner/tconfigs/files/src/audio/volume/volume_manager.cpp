#include "tconfigs/audio/volume/volume_manager.h"

#include "tconfigs/log/logging.h"
#include "tconfigs/common/string.h"

namespace tconfigs {
namespace audio {

std::shared_ptr<VolumeManager> VolumeManager::Create(const rapidjson::Value& config)
{
    auto volume_manager = std::shared_ptr<VolumeManager>(new VolumeManager());
    if (!volume_manager || !volume_manager->Init(config)) {
        TCLOG_ERROR("Fail to create VolumeManager");
        return nullptr;
    }
    return volume_manager;
}

bool VolumeManager::Init(const rapidjson::Value& config)
{
    for (rapidjson::Value::ConstMemberIterator itr = config.MemberBegin();
            itr != config.MemberEnd(); ++itr) {
        const char* type_name = nullptr;
        if (!json::pointer::GetString(itr->value, "/type", &type_name)) {
            TCLOG_ERROR("\"%s\": cannot get config \"type\"", itr->name.GetString());
            return false;
        }
        std::string type_string(type_name);
        if (type_string == "MuteSwitch") {
            auto mute_switch = CreateMuteSwitch(itr->value);
            if (!mute_switch) {
                TCLOG_ERROR("Fail to create MuteSwitch \"%s\"", itr->name.GetString());
                return false;
            }
            mute_switches_.insert({itr->name.GetString(), mute_switch});
        } else if (type_string == "VolumeControl") {
            auto volume_control = CreateVolumeControl(itr->value);
            if (!volume_control) {
                TCLOG_ERROR("Fail to create VolumeControl \"%s\"", itr->name.GetString());
                return false;
            }
            volume_controls_.insert({itr->name.GetString(), volume_control});
        } else {
            TCLOG_ERROR("Invalid type: %s", type_string.c_str());
            return false;
        }
    }
    return true;
}

std::shared_ptr<VolumeManager::MuteSwitch> VolumeManager::CreateMuteSwitch(
        const rapidjson::Value& config)
{
    const char* device = nullptr;
    if (!json::pointer::GetString(config, "/device", &device)) {
        TCLOG_ERROR("Cannot get config \"device\"");
        return nullptr;
    }

    std::shared_ptr<MuteSwitch> mute_switch = nullptr;
    int numid;
    if (json::pointer::GetInt(config, "/numid", &numid)) {
        mute_switch = std::shared_ptr<MuteSwitch>(new MuteSwitch(device, numid));
        if (!mute_switch) {
            TCLOG_ERROR("Fail to create MuteSwitch");
            return nullptr;
        }
    } else {
        const char* iface = nullptr;
        const char* name = nullptr;
        if (!json::pointer::GetString(config, "/iface", &iface)
                || !json::pointer::GetString(config, "/name", &name)) {
            TCLOG_ERROR("Cannot get config \"numid\" or \"iface\", \"name\"");
            return nullptr;
        }
        mute_switch = std::shared_ptr<MuteSwitch>(new MuteSwitch(device, iface, name));
        if (!mute_switch) {
            TCLOG_ERROR("Fail to create MuteSwitch");
            return nullptr;
        }
    }

    int mute_value;
    if (!json::pointer::GetInt(config, "/mute_value", &mute_value)) {
        TCLOG_ERROR("Cannot get config \"mute_value\"");
        return nullptr;
    }
    int unmute_value;
    if (!json::pointer::GetInt(config, "/unmute_value", &unmute_value)) {
        TCLOG_ERROR("Cannot get config \"unmute_value\"");
        return nullptr;
    }
    mute_switch->InitMuteValue(mute_value, unmute_value);

    const char* default_value = nullptr;
    if (json::pointer::GetString(config, "/default", &default_value)) {
        std::string default_string(default_value);
        if (default_string == "mute") {
            if (!mute_switch->SetMute(true)) {
                TCLOG_WARNING("Fail to set default value: %s", default_string.c_str());
            }
        } else if (default_string == "unmute") {
            if (!mute_switch->SetMute(false)) {
                TCLOG_WARNING("Fail to set default value: %s", default_string.c_str());
            }
        } else {
            TCLOG_WARNING("Invalid default value: %s (only \"mute\" or \"unmute\" is valid)",
                    default_string.c_str());
        }
    }

    return mute_switch;
}

std::shared_ptr<VolumeManager::VolumeControl> VolumeManager::CreateVolumeControl(
        const rapidjson::Value& config)
{
    const char* device = nullptr;
    if (!json::pointer::GetString(config, "/device", &device)) {
        TCLOG_ERROR("Cannot get config \"device\"");
        return nullptr;
    }

    std::shared_ptr<VolumeControl> volume_control = nullptr;
    int numid;
    if (json::pointer::GetInt(config, "/numid", &numid)) {
        volume_control = std::shared_ptr<VolumeControl>(new VolumeControl(
                    device, numid));
        if (!volume_control) {
            TCLOG_ERROR("Fail to create VolumeControl");
            return nullptr;
        }
    } else {
        const char* iface = nullptr;
        const char* name = nullptr;
        if (!json::pointer::GetString(config, "/iface", &iface)
                || !json::pointer::GetString(config, "/name", &name)) {
            TCLOG_ERROR("Cannot get config \"numid\" or \"iface\", \"name\"");
            return nullptr;
        }
        volume_control = std::shared_ptr<VolumeControl>(new VolumeControl(
                    device, iface, name));
        if (!volume_control) {
            TCLOG_ERROR("Fail to create MuteSwitch");
            return nullptr;
        }
    }

    int valid_min;
    if (!json::pointer::GetInt(config, "/valid_min", &valid_min)) {
        TCLOG_ERROR("Cannot get config \"valid_min\"");
        return nullptr;
    }
    int valid_max;
    if (!json::pointer::GetInt(config, "/valid_max", &valid_max)) {
        TCLOG_ERROR("Cannot get config \"valid_max\"");
        return nullptr;
    }
    if (!volume_control->InitValidRange(valid_min, valid_max)) {
        TCLOG_ERROR("Fail to init valid range: min: %d, max: %d", valid_min, valid_max);
        return nullptr;
    }

    if (!volume_control->InitActualRange()) {
        TCLOG_WARNING("Fail to init actual range");
    }

    int default_volume;
    if (json::pointer::GetInt(config, "/default", &default_volume)) {
        if (!volume_control->SetVolume(default_volume)) {
            TCLOG_WARNING("Fail to set default volume: %d", default_volume);
        }
    }

    return volume_control;
}

std::shared_ptr<VolumeManager::MuteSwitch> VolumeManager::FindMuteSwitch(
        const std::string& name)
{
    auto itr = mute_switches_.find(name);
    if (itr == mute_switches_.end()) {
        return nullptr;
    }
    return itr->second;
}

std::shared_ptr<VolumeManager::VolumeControl> VolumeManager::FindVolumeControl(
        const std::string& name)
{
    auto itr = volume_controls_.find(name);
    if (itr == volume_controls_.end()) {
        return nullptr;
    }
    return itr->second;
}

// MuteSwitch ==================================================================
VolumeManager::MuteSwitch::MuteSwitch(const std::string& device, int numid)
    : control_(device, numid)
{
}

VolumeManager::MuteSwitch::MuteSwitch(const std::string& device,
        const std::string& iface, const std::string& element_name)
    : control_(device, iface, element_name)
{
}

bool VolumeManager::MuteSwitch::SetMute(bool mute)
{
    const std::string& value = mute ?
        common::IntToString(mute_value_) : common::IntToString(unmute_value_);
    return control_.SetValue(value);
}

bool VolumeManager::MuteSwitch::IsMute(void)
{
    int value;
    if (!control_.GetValue(&value)) {
        TCLOG_ERROR("Cannot get the value to determine whether it is mute");
        return false;
    }
    return value == mute_value_ ? true : false;
}

// VolumeControl ===============================================================
VolumeManager::VolumeControl::VolumeControl(const std::string& device, int numid)
    : control_(device, numid)
{
}

VolumeManager::VolumeControl::VolumeControl(const std::string& device,
        const std::string& iface, const std::string& element_name)
    : control_(device, iface, element_name)
{
}

bool VolumeManager::VolumeControl::InitActualRange(void)
{
    int volume, actual_min, actual_max;
    if (!control_.GetValueWithRange(&volume, &actual_min, &actual_max)) {
        TCLOG_WARNING("Fail to get actual volume range");
        return false;
    }
    actual_min_ = actual_min;
    actual_max_ = actual_max;
    actual_range_has_got_ = true;
    return true;
}

bool VolumeManager::VolumeControl::SetVolume(int volume)
{
    if (volume < valid_min_) {
        volume = valid_min_;
    } else if (volume > valid_max_) {
        volume = valid_max_;
    }

    if (actual_range_has_got_) {
        volume = volume * actual_max_ / valid_max_;
    } else {
        int volume_got, actual_min, actual_max;
        if (control_.GetValueWithRange(&volume_got, &actual_min, &actual_max)) {
            actual_min_ = actual_min;
            actual_max_ = actual_max;
            actual_range_has_got_ = true;
            volume = volume * actual_max_ / valid_max_;
        } else {
            TCLOG_DEBUG("Actual range not got, set original volume");
        }
    }
    return control_.SetValue(common::IntToString(volume));
}

bool VolumeManager::VolumeControl::GetVolume(int* volume)
{
    if (actual_range_has_got_) {
        return control_.GetValue(volume);
    } else {
        int actual_min, actual_max;
        if (!control_.GetValueWithRange(volume, &actual_min, &actual_max)) {
            TCLOG_DEBUG("Fail to get value with range");
            return false;
        }
        actual_min_ = actual_min;
        actual_max_ = actual_max;
        actual_range_has_got_ = true;
    }
    return true;
}

} // namespace audio
} // namespace tconfigs
