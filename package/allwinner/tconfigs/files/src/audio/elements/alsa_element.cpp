#include "tconfigs/audio/elements/alsa_element.h"

#include <sys/time.h>
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

const std::unordered_map<std::string, AlsaUtils::AccessType>
AlsaElement::kStringToAlsaAccessMap = {
    {"RW_INTERLEAVED", AlsaUtils::AccessType::RW_INTERLEAVED},
    {"MMAP_INTERLEAVED", AlsaUtils::AccessType::MMAP_INTERLEAVED},
};

const std::unordered_map<std::string, AlsaUtils::FormatType>
AlsaElement::kStringToAlsaFormatMap = {
    {"S16_LE", AlsaUtils::FormatType::S16_LE},
    {"S24_LE", AlsaUtils::FormatType::S24_LE},
    {"S32_LE", AlsaUtils::FormatType::S32_LE},
};

const std::unordered_map<std::string, AlsaUtils::TimestampMode>
AlsaElement::kStringToAlsaTimestampModeMap = {
    {"NONE", AlsaUtils::TimestampMode::NONE},
    {"ENABLE", AlsaUtils::TimestampMode::ENABLE},
    {"MMAP", AlsaUtils::TimestampMode::MMAP},
};

const std::unordered_map<std::string, AlsaUtils::TimestampType>
AlsaElement::kStringToAlsaTimestampTypeMap = {
    {"GETTIMEOFDAY", AlsaUtils::TimestampType::GETTIMEOFDAY},
    {"MONOTONIC", AlsaUtils::TimestampType::MONOTONIC},
    {"MONOTONIC_RAW", AlsaUtils::TimestampType::MONOTONIC_RAW},
};

const std::unordered_map<AlsaUtils::FormatType, BufferProperty::FormatType>
AlsaElement::kAlsaFormatToPropertyFormatMap = {
    {AlsaUtils::FormatType::S16_LE, BufferProperty::FormatType::S16_LE},
    {AlsaUtils::FormatType::S24_LE, BufferProperty::FormatType::S24_LE},
    {AlsaUtils::FormatType::S32_LE, BufferProperty::FormatType::S32_LE},
};

const std::unordered_map<AlsaUtils::AccessType, BufferProperty::StorageType>
AlsaElement::kAlsaAccessToPropertyStorageMap = {
    {AlsaUtils::AccessType::RW_INTERLEAVED, BufferProperty::StorageType::kInterleaved},
    {AlsaUtils::AccessType::MMAP_INTERLEAVED, BufferProperty::StorageType::kInterleaved},
};

AlsaUtils::AccessType AlsaElement::StringToAlsaAccess(const std::string& access_string)
{
    auto itr = kStringToAlsaAccessMap.find(access_string);
    if (itr == kStringToAlsaAccessMap.end()) {
        return AlsaUtils::AccessType::UNKNOWN;
    }
    return itr->second;
}

AlsaUtils::FormatType AlsaElement::StringToAlsaFormat(const std::string& format_string)
{
    auto itr = kStringToAlsaFormatMap.find(format_string);
    if (itr == kStringToAlsaFormatMap.end()) {
        return AlsaUtils::FormatType::UNKNOWN;
    }
    return itr->second;
}

BufferProperty::FormatType AlsaElement::AlsaFormatToPropertyFormat(AlsaUtils::FormatType format)
{
    auto itr = kAlsaFormatToPropertyFormatMap.find(format);
    if (itr == kAlsaFormatToPropertyFormatMap.end()) {
        return BufferProperty::FormatType::UNKNOWN;
    }
    return itr->second;
}

BufferProperty::StorageType AlsaElement::AlsaAccessToPropertyStorage(AlsaUtils::AccessType access)
{
    auto itr = kAlsaAccessToPropertyStorageMap.find(access);
    if (itr == kAlsaAccessToPropertyStorageMap.end()) {
        return BufferProperty::StorageType::kUnknown;
    }
    return itr->second;
}

AlsaUtils::TimestampMode AlsaElement::StringToAlsaTimestampMode(const std::string& ts_mode_string)
{
    auto itr = kStringToAlsaTimestampModeMap.find(ts_mode_string);
    if (itr == kStringToAlsaTimestampModeMap.end()) {
        return AlsaUtils::TimestampMode::UNKNOWN;
    }
    return itr->second;
}

AlsaUtils::TimestampType AlsaElement::StringToAlsaTimestampType(const std::string& ts_type_string)
{
    auto itr = kStringToAlsaTimestampTypeMap.find(ts_type_string);
    if (itr == kStringToAlsaTimestampTypeMap.end()) {
        return AlsaUtils::TimestampType::UNKNOWN;
    }
    return itr->second;
}

std::shared_ptr<AlsaElement::AlsaDevice> AlsaElement::AlsaDevice::Create(
        const std::string& name, int index, int loop_frames,
        const char* device, const AlsaUtils::Parameters* params)
{
    auto alsa_device = std::shared_ptr<AlsaDevice>(
            new AlsaDevice(name, index, loop_frames));
    if (!alsa_device || !alsa_device->Init(device, params)) {
        return nullptr;
    }
    return alsa_device;
}

AlsaElement::AlsaDevice::AlsaDevice(const std::string& name, int index, int loop_frames)
    : name_(name),
      index_(index),
      loop_frames_(loop_frames)
{
}

bool AlsaElement::AlsaDevice::Init(const char* device, const AlsaUtils::Parameters* params)
{
    auto utils = AlsaUtils::Create(device, params);
    if (!utils) {
        TCLOG_ERROR("AlsaElement \"%s\": Fail to create AlsaUtils for \"%s\"",
                name().c_str(), device);
        return false;
    }
    utils_ = utils;
    return true;
}

AlsaElement::AlsaElement(const std::string& name)
    : Element(name)
{
}

std::shared_ptr<AlsaElement::AlsaDevice> AlsaElement::CreateAlsaDeviceFromConfig(
        int device_index,
        rapidjson::Value::ConstMemberIterator config,
        AlsaUtils::Parameters* params)
{
    const char* device_string = nullptr;
    if (!json::pointer::GetString(config->value, "/device", &device_string)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"device\"",
                name().c_str(), config->name.GetString());
        return nullptr;
    }

    int loop_frames = 0;
    if (!json::pointer::GetInt(config->value, "/loop_frames", &loop_frames)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"loop_frames\"",
                name().c_str(), config->name.GetString());
        return nullptr;
    }

    if (!GetDeviceParameters(config, params)) {
        TCLOG_ERROR("AlsaElement \"%s\": error while getting parameters of device \"%s\"",
                name().c_str(), config->name.GetString());
        return nullptr;
    }

    return AlsaDevice::Create(config->name.GetString(), device_index,
            loop_frames, device_string, params);
}

bool AlsaElement::GetDeviceParameters(
        rapidjson::Value::ConstMemberIterator device_config,
        AlsaUtils::Parameters* params)
{
    const char* content_string = nullptr;
    int content_int = 0;

    if (!json::pointer::GetString(device_config->value, "/access", &content_string)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"access\"",
                name().c_str(), device_config->name.GetString());
        return false;
    }
    params->access = StringToAlsaAccess(content_string);
    if (params->access == AlsaUtils::AccessType::UNKNOWN) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized access type: \"%s\"",
                name().c_str(), device_config->name.GetString(), content_string);
        return false;
    }

    if (!json::pointer::GetString(device_config->value, "/format", &content_string)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"format\"",
                name().c_str(), device_config->name.GetString());
        return false;
    }
    params->format = StringToAlsaFormat(content_string);
    if (params->format == AlsaUtils::FormatType::UNKNOWN) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized format type: \"%s\"",
                name().c_str(), device_config->name.GetString(), content_string);
        return false;
    }

    if (!json::pointer::GetInt(device_config->value, "/rate", &content_int)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"rate\"",
                name().c_str(), device_config->name.GetString());
        return false;
    }
    params->rate = content_int;

    if (!json::pointer::GetInt(device_config->value, "/channels", &content_int)) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": cannot get config \"channels\"",
                name().c_str(), device_config->name.GetString());
        return false;
    }
    params->channels = content_int;

    if (!json::pointer::GetInt(device_config->value, "/period_frames", &content_int)) {
        params->period_frames = params->rate / 8;
        TCLOG_INFO("AlsaElement \"%s\": device \"%s\": not config \"period_frames\", "
                "use default value: %d", name().c_str(),
                device_config->name.GetString(), params->period_frames);
    } else {
        params->period_frames = content_int;
    }

    if (!json::pointer::GetInt(device_config->value, "/periods", &content_int)) {
        params->periods = 4;
        TCLOG_INFO("AlsaElement \"%s\": device \"%s\": not config \"periods\", "
                "use default value: %d", name().c_str(),
                device_config->name.GetString(), params->periods);
    } else {
        params->periods = content_int;
    }

    return true;
}

bool AlsaElement::SetSoftwareParametersFromConfig(std::shared_ptr<AlsaDevice> alsa_device,
        rapidjson::Value::ConstMemberIterator device_config)
{
    AlsaUtils::SoftwareParameters sw_params;
    uint32_t sw_params_flag = 0;

    const char* content_string = nullptr;
    int content_int = 0;

    if (json::pointer::GetString(device_config->value, "/timestamp_mode", &content_string)) {
        sw_params.timestamp_mode = StringToAlsaTimestampMode(content_string);
        if (sw_params.timestamp_mode == AlsaUtils::TimestampMode::UNKNOWN) {
            TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized timestamp mode: \"%s\"",
                    name().c_str(), device_config->name.GetString(), content_string);
            return false;
        }
        sw_params_flag |= AlsaUtils::kTimestampModeFlag;
    }
    if (json::pointer::GetString(device_config->value, "/timestamp_type", &content_string)) {
        sw_params.timestamp_type= StringToAlsaTimestampType(content_string);
        if (sw_params.timestamp_type== AlsaUtils::TimestampType::UNKNOWN) {
            TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized timestamp type: \"%s\"",
                    name().c_str(), device_config->name.GetString(), content_string);
            return false;
        }
        sw_params_flag |= AlsaUtils::kTimestampTypeFlag;
    }
    if (json::pointer::GetInt(device_config->value, "/start_threshold", &content_int)) {
        sw_params.start_threshold = content_int;
        if (sw_params.start_threshold < 0) {
            TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": invalid start threshold: \"%d\"",
                    name().c_str(), device_config->name.GetString(), content_int);
            return false;
        }
        sw_params_flag |= AlsaUtils::kStartThresholdFlag;
    }
    if (json::pointer::GetInt(device_config->value, "/stop_threshold", &content_int)) {
        sw_params.stop_threshold = content_int;
        if (sw_params.stop_threshold < 0) {
            TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": invalid stop threshold: \"%d\"",
                    name().c_str(), device_config->name.GetString(), content_int);
            return false;
        }
        sw_params_flag |= AlsaUtils::kStopThresholdFlag;
    }
    if (json::pointer::GetInt(device_config->value, "/silence_threshold", &content_int)) {
        sw_params.silence_threshold = content_int;
        if (sw_params.silence_threshold < 0) {
            TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": invalid silence threshold: \"%d\"",
                    name().c_str(), device_config->name.GetString(), content_int);
            return false;
        }
        sw_params_flag |= AlsaUtils::kSilenceThresholdFlag;
    }

    int ret = alsa_device->utils()->SetSoftwareParameters(sw_params_flag, &sw_params);
    if (ret != 0) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": fail to set software parameters",
                name().c_str(), device_config->name.GetString());
        return false;
    }

    return true;
}

bool AlsaElement::SetMessagesFromConfig(std::shared_ptr<AlsaDevice> alsa_device,
        rapidjson::Value::ConstMemberIterator device_config)
{
    const rapidjson::Value* content_object = nullptr;

    const char* xrun_message_token =
        alsa_device->utils()->params()->stream == AlsaUtils::StreamType::PLAYBACK ?
        "/underrun_message" : "/overrun_message";
    if (json::pointer::GetObject(device_config->value, xrun_message_token, &content_object)) {
        Message xrun_message;
        Message::UpdateFromConfig(&xrun_message, *content_object);
        alsa_device->set_xrun_message(xrun_message);
    }

    return true;
}

bool AlsaElement::GetPropertyFromAlsaDevice(
        std::shared_ptr<AlsaDevice> device, BufferProperty* property)
{
    BufferProperty::FormatType format =
        AlsaFormatToPropertyFormat(device->utils()->params()->format);
    if (format == BufferProperty::FormatType::UNKNOWN) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized property format type",
                name().c_str(), device->name().c_str());
        return false;
    }
    BufferProperty::StorageType storage =
        AlsaAccessToPropertyStorage(device->utils()->params()->access);
    if (storage == BufferProperty::StorageType::kUnknown) {
        TCLOG_ERROR("AlsaElement \"%s\": device \"%s\": unrecognized property storage type",
                name().c_str(), device->name().c_str());
        return false;
    }
    property->set_storage(storage);
    property->set_format(format);
    property->set_rate(device->utils()->params()->rate);
    property->set_channels(device->utils()->params()->channels);
    property->set_frames(device->loop_frames());
    return true;
}

void AlsaElement::PrintXrunInformation(std::shared_ptr<AlsaUtils> utils)
{
    AlsaUtils::Status status;
    uint32_t status_flag = AlsaUtils::kStateFlag
        | AlsaUtils::kTriggerTimestampFlag
        | AlsaUtils::kAvailableFramesFlag;
    if (0 != utils->GetStatus(status_flag, &status)) {
        TCLOG_ERROR("AlsaElement \"%s\": PCM \"%s\": fail to get status",
                name().c_str(), utils->device().c_str());
        return;
    }

    AlsaUtils::SoftwareParameters sw_params;
    uint32_t sw_params_flag = AlsaUtils::kTimestampTypeFlag;
    if (0 != utils->GetSoftwareParameters(sw_params_flag, &sw_params)) {
        TCLOG_ERROR("AlsaElement \"%s\": PCM \"%s\": fail to get software parameters",
                name().c_str(), utils->device().c_str());
        return;
    }

    // TODO: Adapt it for the timestamp type besides GETTIMEOFDAY.
    // If the timestamp_type in sw_params is not GETTIMEOFDAY, the difference
    // of "now" and "trigger_timestamp" may be incorrect.
    if (status.state == AlsaUtils::State::XRUN
            && sw_params.timestamp_type == AlsaUtils::TimestampType::GETTIMEOFDAY) {
        struct timeval now, diff;
        gettimeofday(&now, 0);
        timersub(&now, &status.trigger_timestamp, &diff);
        TCLOG_INFO("AlsaElement \"%s\": PCM \"%s\": xrun Information:",
                name().c_str(), utils->device().c_str());
        TCLOG_INFO("\tavail %d frames in buffer", status.available_frames);
        TCLOG_INFO("\txrun duration: at least %.3f ms long",
                diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
    }
}

} // namespace audio
} // namespace tconfigs
