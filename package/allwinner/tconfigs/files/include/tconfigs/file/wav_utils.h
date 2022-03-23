#ifndef __TCONFIGS_FILE_WAV_UTILS_H__
#define __TCONFIGS_FILE_WAV_UTILS_H__

#include "tconfigs/file/file_utils.h"

namespace tconfigs {
namespace file {

class WavUtils : public FileUtils {
public:
    struct Header {
        uint32_t riff_id;           // "RIFF"
        uint32_t riff_size;
        uint32_t riff_format;       // "WAVE"
        uint32_t fmt_id;            // "fmt "
        uint32_t fmt_size;          // 16 for PCM
        uint16_t audio_format;      // 1 for PCM
        uint16_t channels;
        uint32_t sample_rate;
        uint32_t byte_rate;         // sample_rate * block_align
        uint16_t block_align;
        uint16_t bits_per_sample;
        uint32_t data_id;           // "data"
        uint32_t data_size;
    };

    WavUtils() = default;
    ~WavUtils();

    int OpenRead(const std::string& file) override;

    // These two functions are both used to open the wav file to write, but
    // the former one hides the parameter "block_align". It will calculate it
    // based on "bits_per_sample" and "channels":
    //      block_align = bits_per_sample / 8 * channels
    int OpenWrite(const std::string& file,
                  uint32_t channels,
                  uint32_t sample_rate,
                  uint32_t bits_per_sample);
    int OpenWrite(const std::string& file,
                  uint32_t channels,
                  uint32_t sample_rate,
                  uint32_t bits_per_sample,
                  uint32_t block_align);

    void Close(void) override;
    int Write(const char* buf, int frames) override;
    int Read(char* buf, int frames) override;

    const Header* header(void) const { return &header_; }

private:
    using FileUtils::OpenWrite;

    enum OperationMode {
        kDefault,
        kRead,
        kWrite
    };
    Header header_;
    OperationMode operation_mode_ = kDefault;
    uint32_t write_data_bytes_ = 0;
};

} // namespace file
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_FILE_WAV_UTILS_H__ */
