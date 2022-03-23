#ifndef __WAVUTILS_H__
#define __WAVUTILS_H__

#include "utils/FileUtils.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1

struct wav_header {
    uint32_t riff_id;           /*00H ~ 03H*/   //"RIFF"
    uint32_t riff_sz;           /*04H ~ 07H*/
    uint32_t riff_fmt;          /*08H ~ 0BH*/   //"WAVE"
    uint32_t fmt_id;            /*0CH ~ 0FH*/   //"fmt "
    uint32_t fmt_sz;            /*10H ~ 13H*/   //PCM 16
    uint16_t audio_format;      /*14H ~ 15H*/   //PCM 1
    uint16_t num_channels;      /*16H ~ 17H*/   //PCM 1
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

namespace AW {

class WavUtils : public FileUtils
{
public:
    WavUtils() = default;
    ~WavUtils() = default;

    int create(const std::string &file,
               const std::string& mode,
               uint32_t bits_pre_sample,
               uint32_t num_channels,
               uint32_t sample_rate);
    int create(const std::string &file,
               const std::string& mode);
    void release() override;
    int write(const char *buf, int len) override;
    int read(char *buf, int len) override;

    struct wav_header& getWavHeader() { return header; };
private:
    enum class Flag
    {
        WRITE,
        READ
    };
    struct wav_header header;
    Flag m_flag{Flag::WRITE};
};

}
#endif
