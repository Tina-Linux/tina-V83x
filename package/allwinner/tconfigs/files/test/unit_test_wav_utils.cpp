#include <iostream>
#include <gtest/gtest.h>
#include "tconfigs/utils/wav_utils.h"

const std::string wav_file = "/tmp/test.wav";

void WavHeaderPrint(const tconfigs::utils::WavUtils::WavHeader* header)
{
    std::cout << "Wav Header:" << std::endl;
    std::cout << "\triff_id: " << std::hex << "0x" << header->riff_id << std::endl;
    std::cout << "\triff_size: " << std::dec << header->riff_size << std::endl;
    std::cout << "\triff_format: " << std::hex << "0x" << header->riff_format << std::endl;
    std::cout << "\tfmt_id: " << std::hex << "0x" << header->fmt_id << std::endl;
    std::cout << "\tfmt_size: " << std::dec << header->fmt_size << std::endl;
    std::cout << "\taudio_format: " << std::dec << header->audio_format << std::endl;
    std::cout << "\tchannels: " << std::dec << header->channels << std::endl;
    std::cout << "\tsample_rate: " << std::dec << header->sample_rate << std::endl;
    std::cout << "\tbyte_rate: " << std::dec << header->byte_rate << std::endl;
    std::cout << "\tblock_align: " << std::dec << header->block_align << std::endl;
    std::cout << "\tbits_per_sample: " << std::dec << header->bits_per_sample << std::endl;
    std::cout << "\tdata_id: " << std::hex << "0x" << header->data_id << std::endl;
    std::cout << "\tdata_size: " << std::dec << header->data_size << std::endl;
}

//TEST(Read, Open) {
//    tconfigs::utils::WavUtils utils;
//    ASSERT_EQ(0, utils.OpenRead(wav_file));
//    WavHeaderPrint(utils.header());
//    utils.Close();
//}

TEST(Write, Open) {
    tconfigs::utils::WavUtils utils;
    ASSERT_EQ(0, utils.OpenWrite(wav_file, 2, 16000, 16));
    WavHeaderPrint(utils.header());
    utils.Close();
}

TEST(Write, WriteData) {
    tconfigs::utils::WavUtils utils;
    utils.OpenWrite(wav_file, 2, 16000, 16);
    utils.Close();
    WavHeaderPrint(utils.header());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
