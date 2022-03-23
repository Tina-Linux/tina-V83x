#include <getopt.h>
#include <string>
#include <gtest/gtest.h>

#include "tconfigs/audio/common/pipeline.h"
#include "tconfigs/audio/elements/file_src.h"
#include "tconfigs/audio/elements/file_sink.h"

std::string input_file = "";
std::string output_file = "";

void PrintHelpMessage(const std::string& app_name)
{
    std::cout << std::endl;
    std::cout << "USAGE: "<< std::endl;
    std::cout << "\t" << app_name << " [OPTION]" << std::endl;
    std::cout << "OPTION:" << std::endl;
    std::cout << "\t-h,--help           : print help messages" << std::endl;
    std::cout << "\t-i,--input FILE     : select input file by name, "
        << "otherwise it would be parsed from JSON" << std::endl;
    std::cout << "\t-o,--output FILE    : select output file by name, "
        << "otherwise it would be parsed from JSON" << std::endl;
}

std::string wav_string =
"{"
    "\"file_src\" : {"
        "\"element_type\" : \"FileSrc\","
        "\"type\" : \"wav\","
        "\"path\" : \"/tmp/src.wav\","
        "\"loop_frames\" : 160,"
        "\"src_pads\" : {"
            "\"src_pad\" : {"
                "\"peer_element\" : \"file_sink\","
                "\"peer_pad\" : \"sink_pad\""
            "}"
        "}"
    "},"
    "\"file_sink\" : {"
        "\"element_type\" : \"FileSink\","
        "\"type\" : \"wav\","
        "\"path\" : \"/tmp/sink.wav\","
        "\"sink_pads\" : {"
            "\"sink_pad\" : {"
                "\"peer_element\" : \"file_src\","
                "\"peer_pad\" : \"src_pad\""
            "}"
        "}"
    "}"
"}";

std::string pcm_string =
"{"
    "\"file_src\" : {"
        "\"element_type\" : \"FileSrc\","
        "\"type\" : \"pcm\","
        "\"path\" : \"/tmp/src.pcm\","
        "\"format\" : \"S16_LE\","
        "\"channels\" : 2,"
        "\"rate\" : 16000,"
        "\"loop_frames\" : 160,"
        "\"src_pads\" : {"
            "\"src_pad\" : {"
                "\"peer_element\" : \"file_sink\","
                "\"peer_pad\" : \"sink_pad\""
            "}"
        "}"
    "},"
    "\"file_sink\" : {"
        "\"element_type\" : \"FileSink\","
        "\"type\" : \"pcm\","
        "\"path\" : \"/tmp/sink.pcm\","
        "\"sink_pads\" : {"
            "\"sink_pad\" : {"
                "\"peer_element\" : \"file_src\","
                "\"peer_pad\" : \"src_pad\""
            "}"
        "}"
    "}"
"}";

TEST(FileSrcTest, WavSrcWavSink) {
    auto pipeline = tconfigs::audio::Pipeline::Create("pipeline");
    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDom(wav_string.c_str(), &doc));
    const rapidjson::Value* file_src_config = nullptr;
    const rapidjson::Value* file_sink_config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/file_src", &file_src_config));
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/file_sink", &file_sink_config));
    auto file_src = tconfigs::audio::FileSrc::Create("file_src",
            *file_src_config, input_file);
    ASSERT_NE(file_src, nullptr);
    auto file_sink = tconfigs::audio::FileSink::Create("file_sink",
            *file_sink_config, output_file);
    ASSERT_NE(file_sink, nullptr);
    auto buffer = tconfigs::audio::Buffer<int16_t>::Create(*(file_src->src_pad_property()));
    ASSERT_NE(buffer, nullptr);
    std::cout << "buffer property: "
        << buffer->property()->frames() << ", "
        << buffer->property()->channels() << ", "
        << buffer->property()->rate() << ", "
        << static_cast<int>(buffer->property()->format()) << ", "
        << static_cast<int>(buffer->property()->storage()) << std::endl;

    auto link_result = pipeline->LinkPads(file_src, "src_pad", file_sink, "sink_pad", buffer);
    ASSERT_TRUE(link_result);
    file_src->Loop();
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hi:o:", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'h':
            PrintHelpMessage(argv[0]);
            return 0;
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        default:
            std::cout << "Invalid option: -" << (char)opt << std::endl;
            PrintHelpMessage(argv[0]);
            return -1;
        }
    }

    return RUN_ALL_TESTS();
}
