#ifndef __TCONFIGS_AUDIO_ELEMENTS_FILE_SINK_H__
#define __TCONFIGS_AUDIO_ELEMENTS_FILE_SINK_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/file/file_utils.h"
#include "tconfigs/file/wav_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class FileSink : public Element,
                 public std::enable_shared_from_this<FileSink> {
public:
    // Used for array indexes, don't change the number
    enum class Type : int {
        kWav = 0,
        kPcm,
        kNumOfTypes
    };

    static std::shared_ptr<FileSink> Create(const std::string& name,
            const rapidjson::Value& config);
    static std::shared_ptr<FileSink> Create(const std::string& name,
            const rapidjson::Value& config, const std::string& file_path);

    ~FileSink(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int Loop(void) override;
    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    typedef int (FileSink::*WriteFunc)(void* data, int frames);

    FileSink(void) = delete;
    FileSink(const std::string& name);
    FileSink(const std::string& name, const std::string& file_path);

    bool Init(const rapidjson::Value& config);

    int WavWrite(void* data, int frames);
    int PcmWrite(void* data, int frames);

    static Type StringToType(const std::string& s);

    Type type_ = Type::kNumOfTypes;
    std::string path_ = "";
    std::shared_ptr<Pad> sink_pad_ = nullptr;
    BufferProperty sink_pad_property_;

    std::shared_ptr<file::WavUtils> wav_utils_ = nullptr;
    std::shared_ptr<file::FileUtils> file_utils_ = nullptr;

    const WriteFunc kWriteFuncs[static_cast<int>(Type::kNumOfTypes)] = {
        &FileSink::WavWrite,
        &FileSink::PcmWrite,
    };

    static const std::unordered_map<std::string, Type> kStringToTypeMap;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_FILE_SINK_H__ */
