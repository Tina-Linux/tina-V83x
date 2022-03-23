#include "tconfigs/audio/elements/file_sink.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(FileSink)

const std::unordered_map<std::string, FileSink::Type> FileSink::kStringToTypeMap = {
    {"wav", Type::kWav},
    {"pcm", Type::kPcm},
};

FileSink::Type FileSink::StringToType(const std::string& s)
{
    auto itr = kStringToTypeMap.find(s);
    if (itr == kStringToTypeMap.end()) {
        return Type::kNumOfTypes;
    }
    return itr->second;
}

std::shared_ptr<FileSink> FileSink::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto file_sink = std::shared_ptr<FileSink>(new FileSink(name));
    if (!file_sink || !file_sink->Init(config)) {
        return nullptr;
    }
    return file_sink;
}

std::shared_ptr<FileSink> FileSink::Create(const std::string& name,
        const rapidjson::Value& config, const std::string& file_path)
{
    auto file_sink = std::shared_ptr<FileSink>(new FileSink(name, file_path));
    if (!file_sink || !file_sink->Init(config)) {
        return nullptr;
    }
    return file_sink;
}

FileSink::FileSink(const std::string& name)
    : Element(name)
{
}

FileSink::FileSink(const std::string& name, const std::string& file_path)
    : Element(name),
      path_(file_path)
{
}

bool FileSink::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    int content_int;

    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "FileSink") {
        TCLOG_ERROR("\"%s\" is not a FileSink", name().c_str());
        return false;
    }

    if (!json::pointer::GetString(config, "/type", &content_string)) {
        TCLOG_ERROR("FileSink \"%s\": cannot get config \"type\"", name().c_str());
        return false;
    }
    type_ = StringToType(content_string);
    if (type_ == Type::kNumOfTypes) {
        std::string available_types_string = "";
        for (auto itr = kStringToTypeMap.begin(); itr != kStringToTypeMap.end(); ++itr) {
            available_types_string.append(" \"" + itr->first + "\"");
        }
        TCLOG_ERROR("FileSink \"%s\": unrecognized type: \"%s\". Available types are:%s",
                name().c_str(), content_string, available_types_string.c_str());
        return false;
    }

    if (path_ == "") {
        if (!json::pointer::GetString(config, "/path", &content_string)) {
            TCLOG_ERROR("FileSink \"%s\": cannot get config \"path\"", name().c_str());
            return false;
        }
        path_ = content_string;
    }

    BufferProperty property;
    property.set_storage(BufferProperty::StorageType::kInterleaved);
    if (json::pointer::GetInt(config, "/loop_frames", &content_int)) {
        property.set_frames(content_int);
    }
    if (json::pointer::GetString(config, "/format", &content_string)) {
        BufferProperty::FormatType format =
            BufferProperty::StringToFormatType(content_string);
        if (format == BufferProperty::FormatType::UNKNOWN) {
            TCLOG_ERROR("FileSink \"%s\": Unrecognized format: \"%s\"",
                    name().c_str(), content_string);
            return false;
        }
        property.set_format(format);
    }
    if (json::pointer::GetInt(config, "/channels", &content_int)) {
        property.set_channels(content_int);
    }
    if (json::pointer::GetInt(config, "/rate", &content_int)) {
        property.set_rate(content_int);
    }

    const rapidjson::Value* sink_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/sink_pads", &sink_pads_config)) {
        TCLOG_ERROR("FileSink \"%s\": cannot get config \"sink_pads\"", name().c_str());
        return false;
    }
    // FileSink has only one sink pad
    if (sink_pads_config->Size() != 1) {
        TCLOG_ERROR("FileSink \"%s\": the number of sink pads in config is not equal to 1",
                name().c_str());
        return false;
    }
    std::string sink_pad_name = sink_pads_config->MemberBegin()->name.GetString();
    if (!AddPad(sink_pad_name, Pad::Direction::kSink, shared_from_this())) {
        TCLOG_ERROR("FileSink \"%s\": fail to add sink pad named \"%s\"",
                name().c_str(), sink_pad_name.c_str());
        return false;
    }
    sink_pad_ = FindPad(sink_pad_name, Pad::Direction::kSink);

    if (!property.IsEmpty()) {
        sink_pad_->set_property(property);
    }

    AddAvailableMode({Mode::kPush, Mode::kPull});

    return true;
}

bool FileSink::Activate(Mode mode)
{
    if (!sink_pad_->peer()) {
        TCLOG_ERROR("FileSink \"%s\": sink pad peer not exists", name().c_str());
        return false;
    }

    auto property = sink_pad_->property();
    if (!property) {
        TCLOG_DEBUG("FileSink \"%s\": sink pad has no property", name().c_str());
        BufferProperty property_tmp;
        property_tmp.Complete(sink_pad_->buffer()->property(), nullptr);
        sink_pad_->set_property(property_tmp);
        property = sink_pad_->property();
    } else if (!property->IsComplete()) {
        TCLOG_DEBUG("FileSink \"%s\": sink pad's property is not complete", name().c_str());
        property->Complete(sink_pad_->buffer()->property(), nullptr);
    }

    int effective_bits, total_bits;
    if (!BufferProperty::FormatTypeToBits(property->format(), &effective_bits, &total_bits)) {
        TCLOG_ERROR("FileSink \"%s\": not supported format: %s", name().c_str(),
                BufferProperty::FormatTypeToString(property->format()).c_str());
        return false;
    }

    switch (type_) {
    case Type::kWav:
        wav_utils_ = std::shared_ptr<file::WavUtils>(new file::WavUtils);
        if (!wav_utils_ || 0 != wav_utils_->OpenWrite(path_,
                    property->channels(), property->rate(), effective_bits,
                    total_bits / 8 * property->channels())) {
            return false;
        }
        break;
    case Type::kPcm:
        file_utils_ = std::shared_ptr<file::FileUtils>(new file::FileUtils);
        if (!file_utils_ || 0 != file_utils_->OpenWrite(path_)) {
            return false;
        }
        break;
    default:
        TCLOG_ERROR("FileSink \"%s\": unrecognized type", name().c_str());
        return false;
    }

    return ActivateDefault(mode);
}

bool FileSink::Deactivate(void)
{
    switch (type_) {
    case Type::kWav:
        wav_utils_->Close();
        wav_utils_ = nullptr;
        break;
    case Type::kPcm:
        file_utils_->Close();
        file_utils_ = nullptr;
        break;
    default:
        DeactivateDefault();
        TCLOG_ERROR("FileSink \"%s\": unrecognized type", name().c_str());
        return false;
    }

    return DeactivateDefault();
}

int FileSink::WavWrite(void* data, int frames)
{
    return wav_utils_->Write(static_cast<char*>(data), frames);
}

int FileSink::PcmWrite(void* data, int frames)
{
    auto property = sink_pad_->buffer()->property();
    int effective_bits, total_bits;
    if (!BufferProperty::FormatTypeToBits(
                property->format(), &effective_bits, &total_bits)) {
        TCLOG_ERROR("FileSink \"%s\": not supported format: %s", name().c_str(),
                BufferProperty::FormatTypeToString(property->format()).c_str());
        return file::FileUtils::kErrorReturnValue;
    }
    int bytes = total_bits / 8 * property->channels() * frames;
    return file_utils_->Write(static_cast<char*>(data), bytes);
}

int FileSink::Loop(void)
{
    int ret = sink_pad_->Pull();
    if (ret < 0) {
        return ret;
    }
    int frames = sink_pad_->buffer()->property()->frames();
    return (this->*(kWriteFuncs[static_cast<int>(type_)]))(
            sink_pad_->buffer()->data(), frames);
}

int FileSink::PushChain(std::shared_ptr<Pad> pad)
{
    int frames = pad->buffer()->property()->frames();
    return (this->*(kWriteFuncs[static_cast<int>(type_)]))(
            pad->buffer()->data(), frames);
}

} // namespace audio
} // namespace tconfigs
