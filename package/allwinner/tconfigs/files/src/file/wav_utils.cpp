#include "tconfigs/file/wav_utils.h"

#include <endian.h>
#include <limits>
#include "tconfigs/log/logging.h"

#if __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#else
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#endif

#define WAV_ID_RIFF COMPOSE_ID('R','I','F','F')
#define WAV_ID_WAVE COMPOSE_ID('W','A','V','E')
#define WAV_ID_FMT COMPOSE_ID('f','m','t',' ')
#define WAV_ID_DATA COMPOSE_ID('d','a','t','a')
#define WAV_FORMAT_PCM 0x0001

namespace tconfigs {
namespace file {

const uint32_t uint32_max = std::numeric_limits<uint32_t>::max();

WavUtils::~WavUtils(void)
{
    Close();
}

int WavUtils::OpenRead(const std::string& file)
{
    int ret = FileUtils::OpenRead(file);
    if (ret < 0) {
        return ret;
    }
    ret = FileUtils::Read((char*)&header_, sizeof(Header));
    if (ret < static_cast<int>(sizeof(Header))) {
        TCLOG_ERROR("Fail to read wav header from \'%s\'",
                FileUtils::file_name().c_str());
        return FileUtils::kErrorReturnValue;
    }

    operation_mode_ = kRead;
    return 0;
}

int WavUtils::OpenWrite(const std::string& file, uint32_t channels,
        uint32_t sample_rate, uint32_t bits_per_sample)
{
    uint32_t block_align = bits_per_sample / 8 * channels;
    return OpenWrite(file, channels, sample_rate, bits_per_sample, block_align);
}

int WavUtils::OpenWrite(const std::string& file, uint32_t channels,
        uint32_t sample_rate, uint32_t bits_per_sample, uint32_t block_align)
{
    int ret = FileUtils::OpenWrite(file);
    if (ret < 0) {
        return ret;
    }

    header_.riff_id = WAV_ID_RIFF;
    header_.riff_size = 0;
    header_.riff_format = WAV_ID_WAVE;
    header_.fmt_id = WAV_ID_FMT;
    header_.fmt_size = 16;
    header_.audio_format = WAV_FORMAT_PCM;
    header_.channels = channels;
    header_.sample_rate = sample_rate;
    header_.byte_rate = sample_rate * block_align;
    header_.block_align = block_align;
    header_.bits_per_sample = bits_per_sample;
    header_.data_id = WAV_ID_DATA;
    header_.data_size = 0;

    ret = FileUtils::Write((char*)&header_, sizeof(Header));
    if (ret < static_cast<int>(sizeof(Header))) {
        TCLOG_ERROR("Fail to write wav header to \'%s\'",
                FileUtils::file_name().c_str());
        return FileUtils::kErrorReturnValue;
    }

    operation_mode_ = kWrite;
    write_data_bytes_ = 0;
    return 0;
}

void WavUtils::Close(void)
{
    if (operation_mode_ != kWrite) {
        FileUtils::Close();
        return;
    }
    header_.riff_size = write_data_bytes_ - 36;
    header_.data_size = write_data_bytes_;

    int ret;
    ret = FileUtils::Seek(FileUtils::kBegin, 0);
    if (ret < 0) {
        TCLOG_ERROR("Fail to seek the beginning of \'%s\'",
                FileUtils::file_name().c_str());
        goto close;
    }
    ret = FileUtils::Write((char*)&header_, sizeof(Header));
    if (ret < static_cast<int>(sizeof(Header))) {
        TCLOG_ERROR("Fail to write wav header to \'%s\'",
                FileUtils::file_name().c_str());
    }
close:
    FileUtils::Close();
    operation_mode_ = kDefault;
}

int WavUtils::Read(char* buf, int frames)
{
    if (operation_mode_ != kRead) {
        return FileUtils::kErrorReturnValue;
    }
    int bytes = static_cast<int>(header_.block_align) * frames;
    int ret = FileUtils::Read(buf, bytes);
    if (ret <= 0) {
        return ret;
    }
    return ret / static_cast<int>(header_.block_align);
}

int WavUtils::Write(const char *buf, int frames)
{
    if (operation_mode_ != kWrite) {
        return FileUtils::kErrorReturnValue;
    }
    int bytes = static_cast<int>(header_.block_align) * frames;
    if (uint32_max - write_data_bytes_ < static_cast<uint32_t>(bytes)) {
        bytes = static_cast<int>(uint32_max - write_data_bytes_);
    }
    if (bytes == 0) {
        return 0;
    }
    int ret = FileUtils::Write(buf, bytes);
    if (ret <= 0) {
        return ret;
    }
    write_data_bytes_ += static_cast<uint32_t>(ret);
    return ret / static_cast<int>(header_.block_align);
}

} // namespace file
} // namespace tconfigs
