#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "utils/WavUtils.h"

namespace AW {
//for read
int WavUtils::create(const std::string &file, const std::string& mode)
{
    int ret = FileUtils::create(file, mode);
    if(ret < 0) return ret;

    ret = FileUtils::read((char*)&header, sizeof(header));
    if(ret < 0) return ret;

    m_flag = Flag::READ;

    return 0;
}

//for write
int WavUtils::create(const std::string &file,
           const std::string& mode,
           uint32_t bits_pre_sample,
           uint32_t num_channels,
           uint32_t sample_rate)
{
    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.bits_per_sample = bits_pre_sample;

    header.byte_rate = (header.bits_per_sample / 8) * header.num_channels * header.sample_rate;
    header.block_align = header.num_channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;

    int ret = FileUtils::create(file, mode);
    if(ret < 0) return ret;

    FileUtils::write((char*)&header, sizeof(struct wav_header));
/*
    ret = FileUtils::seek(sizeof(struct wav_header), SEEK_SET);
    if(ret < 0){
        printf("1seek fail: %s\n", strerror(errno));
    }
*/
    return 0;
}

void WavUtils::release()
{
    if(m_flag == Flag::READ) {
        FileUtils::release();
        return;
    }
    //long frames_read = FileUtils::tell() - sizeof(struct wav_header);
    //frames_read = frames_read/(header.num_channels * header.bits_per_sample/8);
    //printf("frame read is 0x%x\n", frames_read);
    header.data_sz = FileUtils::tell() - sizeof(struct wav_header);
    header.riff_sz = FileUtils::tell() - 8;
    char * p = (char*)&header;
    /*
    for(int i =0; i < sizeof(struct wav_header); i++){
        if(i % 16 == 0) printf("\n");
        if(i % 8 == 0) printf(" ");
        printf("%02x ", p[i]);
    }
    printf("\n");
    */
    int ret = FileUtils::seek(0L, SEEK_SET);
    if(ret < 0){
        printf("2seek fail: %s\n", strerror(errno));
    }

    FileUtils::write(p, sizeof(struct wav_header));
    FileUtils::release();
}

int WavUtils::write(const char *buf, int len)
{
    return FileUtils::write(buf, len * header.num_channels * header.bits_per_sample/8);
}
int WavUtils::read(char *buf, int len)
{
    int ret = FileUtils::read(buf, len * header.num_channels * header.bits_per_sample/8);
    if(ret <= 0) return ret;
    return ret/(header.num_channels * header.bits_per_sample/8);
}

}
