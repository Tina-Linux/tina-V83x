#ifndef __TCONFIGS_AUDIO_ELEMENTS_FILE_SRC_H__
#define __TCONFIGS_AUDIO_ELEMENTS_FILE_SRC_H__

#include "tconfigs/json/json_utils.h"
#include "tconfigs/file/file_utils.h"
#include "tconfigs/file/wav_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class FileSrc : public Element,
                public std::enable_shared_from_this<FileSrc> {
public:
    // Used for array indexes, don't change the number
    enum class Type : int {
        kWav = 0,
        kPcm,
        kNumOfTypes
    };

    static std::shared_ptr<FileSrc> Create(const std::string& name,
            const rapidjson::Value& config);
    static std::shared_ptr<FileSrc> Create(const std::string& name,
            const rapidjson::Value& config, const std::string& file_path);

    ~FileSrc(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int Loop(void) override;
    int PullChain(std::shared_ptr<Pad> pad) override;

    void set_repeated(bool repeated) { repeated_ = repeated; }
    bool repeated(void) const { return repeated_; }

    // @deprecated (TODO: remove it in the future)
    const BufferProperty* src_pad_property(void) const { return src_pad_->property(); }

private:
    typedef int (FileSrc::*ReadFunc)(void* data, int frames);

    FileSrc(void) = delete;
    FileSrc(const std::string& name);
    FileSrc(const std::string& name, const std::string& file_path);

    bool Init(const rapidjson::Value& config);

    bool GetWavParameters(std::shared_ptr<file::WavUtils> wav_utils,
            BufferProperty* property);
    bool GetPcmParameters(const rapidjson::Value& config,
            BufferProperty* property);

    int WavRead(void* data, int frames);
    int PcmRead(void* data, int frames);

    bool SeekBegin(void);

    static Type StringToType(const std::string& s);

    Type type_ = Type::kNumOfTypes;
    std::string path_ = "";
    std::shared_ptr<Pad> src_pad_ = nullptr;

    std::shared_ptr<file::WavUtils> wav_utils_ = nullptr;
    std::shared_ptr<file::FileUtils> file_utils_ = nullptr;

    bool repeated_ = false;

    const ReadFunc kReadFuncs[static_cast<int>(Type::kNumOfTypes)] = {
        &FileSrc::WavRead,
        &FileSrc::PcmRead,
    };

    static const std::unordered_map<std::string, Type> kStringToTypeMap;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_FILE_SRC_H__ */
