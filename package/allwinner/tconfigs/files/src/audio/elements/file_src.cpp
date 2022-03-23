#include "tconfigs/audio/elements/file_src.h"

#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(FileSrc)

const std::unordered_map<std::string, FileSrc::Type> FileSrc::kStringToTypeMap = {
    {"wav", Type::kWav},
    {"pcm", Type::kPcm},
};

FileSrc::Type FileSrc::StringToType(const std::string& s)
{
    auto itr = kStringToTypeMap.find(s);
    if (itr == kStringToTypeMap.end()) {
        return Type::kNumOfTypes;
    }
    return itr->second;
}

std::shared_ptr<FileSrc> FileSrc::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto file_src = std::shared_ptr<FileSrc>(new FileSrc(name));
    if (!file_src || !file_src->Init(config)) {
        return nullptr;
    }
    return file_src;
}

std::shared_ptr<FileSrc> FileSrc::Create(const std::string& name,
        const rapidjson::Value& config, const std::string& file_path)
{
    auto file_src = std::shared_ptr<FileSrc>(new FileSrc(name, file_path));
    if (!file_src || !file_src->Init(config)) {
        return nullptr;
    }
    return file_src;
}

FileSrc::FileSrc(const std::string& name)
    : Element(name)
{
}

FileSrc::FileSrc(const std::string& name, const std::string& file_path)
    : Element(name),
      path_(file_path)
{
}

bool FileSrc::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;

    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "FileSrc") {
        TCLOG_ERROR("\"%s\" is not a FileSrc", name().c_str());
        return false;
    }

    if (!json::pointer::GetString(config, "/type", &content_string)) {
        TCLOG_ERROR("FileSrc \"%s\": cannot get config \"type\"", name().c_str());
        return false;
    }
    type_ = StringToType(content_string);
    if (type_ == Type::kNumOfTypes) {
        std::string available_types_string = "";
        for (auto itr = kStringToTypeMap.begin(); itr != kStringToTypeMap.end(); ++itr) {
            available_types_string.append(" \"" + itr->first + "\"");
        }
        TCLOG_ERROR("FileSrc \"%s\": unrecognized type \"%s\". Available types are:%s",
                name().c_str(), content_string, available_types_string.c_str());
        return false;
    }

    if (path_ == "") {
        if (!json::pointer::GetString(config, "/path", &content_string)) {
            TCLOG_ERROR("FileSrc \"%s\": cannot get config \"path\"", name().c_str());
            return false;
        }
        path_ = content_string;
    }

    BufferProperty property;
    switch (type_) {
    case Type::kWav:
        wav_utils_ = std::shared_ptr<file::WavUtils>(new file::WavUtils);
        if (!wav_utils_ || 0 != wav_utils_->OpenRead(path_)) {
            return false;
        }
        if (!GetWavParameters(wav_utils_, &property)) {
            return false;
        }
        break;
    case Type::kPcm:
        file_utils_ = std::shared_ptr<file::FileUtils>(new file::FileUtils);
        if (!file_utils_ || 0 != file_utils_->OpenRead(path_)) {
            return false;
        }
        if (!GetPcmParameters(config, &property)) {
            return false;
        }
        break;
    default:
        TCLOG_ERROR("FileSrc \"%s\": unrecognized type", name().c_str());
        return false;
    }

    int content_int = 0;
    if (json::pointer::GetInt(config, "/loop_frames", &content_int)) {
        property.set_frames(content_int);
    } else {
        TCLOG_DEBUG("FileSrc \"%s\": not get config \"loop_frames\"", name().c_str());
    }

    bool content_bool = false;
    if (json::pointer::GetBool(config, "/repeated", &content_bool)) {
        repeated_ = content_bool;
    } else {
        TCLOG_DEBUG("FileSrc \"%s\": not get config \"repeated\"", name().c_str());
    }

    const rapidjson::Value* src_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/src_pads", &src_pads_config)) {
        TCLOG_ERROR("FileSrc \"%s\": cannot get config \"src_pads\"", name().c_str());
        return false;
    }
    // FileSrc has only one src pad
    if (src_pads_config->Size() != 1) {
        TCLOG_ERROR("FileSrc \"%s\": the number of src pads in config is not equal to 1",
                name().c_str());
        return false;
    }
    std::string src_pad_name = src_pads_config->MemberBegin()->name.GetString();
    if (!AddPad(src_pad_name, Pad::Direction::kSrc, shared_from_this())) {
        TCLOG_ERROR("FileSrc \"%s\": fail to add src pad named \"%s\"",
                name().c_str(), src_pad_name.c_str());
        return false;
    }
    src_pad_ = FindPad(src_pad_name, Pad::Direction::kSrc);
    src_pad_->set_property(property);

    AddAvailableMode({Mode::kPush, Mode::kPull});

    return true;
}

bool FileSrc::GetWavParameters(std::shared_ptr<file::WavUtils> wav_utils,
        BufferProperty* property)
{
    const file::WavUtils::Header* header = wav_utils->header();
    BufferProperty::FormatType format = BufferProperty::BitsToFormatType(
            header->bits_per_sample, header->block_align / header->channels * 8);
    if (format == BufferProperty::FormatType::UNKNOWN) {
        TCLOG_ERROR("Unrecognized format of wav file: \"%s\"",
                wav_utils->file_name().c_str());
        return false;
    }
    property->set_format(format);
    property->set_channels(header->channels);
    property->set_rate(header->sample_rate);
    property->set_storage(BufferProperty::StorageType::kInterleaved);
    return true;
}

bool FileSrc::GetPcmParameters(const rapidjson::Value& config,
            BufferProperty* property)
{
    const char* content_string = nullptr;
    int content_int = 0;

    if (!json::pointer::GetString(config, "/format", &content_string)) {
        TCLOG_ERROR("FileSrc \"%s\": cannot get config \"format\"", name().c_str());
        return false;
    }
    BufferProperty::FormatType format =
        BufferProperty::StringToFormatType(content_string);
    if (format == BufferProperty::FormatType::UNKNOWN) {
        TCLOG_ERROR("Unrecognized format of pcm file config: \"%s\"", content_string);
        return false;
    }
    property->set_format(format);

    if (!json::pointer::GetInt(config, "/channels", &content_int)) {
        TCLOG_ERROR("FileSrc \"%s\": cannot get config \"channels\"", name().c_str());
        return false;
    }
    property->set_channels(content_int);

    if (!json::pointer::GetInt(config, "/rate", &content_int)) {
        TCLOG_ERROR("FileSrc \"%s\": cannot get config \"rate\"", name().c_str());
        return false;
    }
    property->set_rate(content_int);

    property->set_storage(BufferProperty::StorageType::kInterleaved);
    return true;
}

bool FileSrc::Activate(Mode mode)
{
    return ActivateDefault(mode);
}

bool FileSrc::Deactivate(void)
{
    if (!SeekBegin()) {
        DeactivateDefault();
        return false;
    }
    return DeactivateDefault();
}

bool FileSrc::SeekBegin(void)
{
    switch (type_) {
    case Type::kWav:
        wav_utils_->Seek(file::FileUtils::kBegin, 44);
        break;
    case Type::kPcm:
        file_utils_->Seek(file::FileUtils::kBegin, 0);
        break;
    default:
        TCLOG_ERROR("FileSrc \"%s\": unrecognized type", name().c_str());
        return false;
    }
    return true;
}

int FileSrc::WavRead(void* data, int frames)
{
    return wav_utils_->Read(static_cast<char*>(data), frames);
}

int FileSrc::PcmRead(void* data, int frames)
{
    auto property = src_pad_->buffer()->property();
    int effective_bits, total_bits;
    if (!BufferProperty::FormatTypeToBits(
                property->format(), &effective_bits, &total_bits)) {
        TCLOG_ERROR("FileSrc \"%s\": not supported format: %s", name().c_str(),
                BufferProperty::FormatTypeToString(property->format()).c_str());
        return file::FileUtils::kErrorReturnValue;
    }
    int bytes = total_bits / 8 * property->channels() * frames;
    return file_utils_->Read(static_cast<char*>(data), bytes);
}

int FileSrc::Loop(void)
{
    int frames = src_pad_->buffer()->property()->frames();
    int ret = (this->*(kReadFuncs[static_cast<int>(type_)]))(
            src_pad_->buffer()->data(), frames);
    if (ret == file::FileUtils::kEofReturnValue) {
        if (!SeekBegin()) {
            TCLOG_ERROR("FileSrc \"%s\": fail to seek beginning", name().c_str());
            return -1;
        }
        return repeated_ ? 0 : file::FileUtils::kEofReturnValue;
    } else if (ret < 0) {
        return ret;
    }
    return src_pad_->Push();
}

int FileSrc::PullChain(std::shared_ptr<Pad> pad)
{
    int frames = pad->buffer()->property()->frames();
    int ret = (this->*(kReadFuncs[static_cast<int>(type_)]))(
            pad->buffer()->data(), frames);
    if (ret == file::FileUtils::kEofReturnValue) {
        if (!SeekBegin()) {
            TCLOG_ERROR("FileSrc \"%s\": fail to seek beginning", name().c_str());
            return -1;
        }
        return repeated_ ? 0 : file::FileUtils::kEofReturnValue;
    }
    return ret;
}

} // namespace audio
} // namespace tconfigs
