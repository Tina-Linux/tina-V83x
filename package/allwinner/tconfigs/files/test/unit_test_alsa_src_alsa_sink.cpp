#include <string>
#include <gtest/gtest.h>

#include "tconfigs/audio/common/pipeline.h"
#include "tconfigs/audio/elements/file_src.h"
#include "tconfigs/audio/elements/file_sink.h"
#include "tconfigs/audio/elements/alsa_src.h"
#include "tconfigs/audio/elements/alsa_sink.h"

int should_stop = 0;

void when_signal(int sig)
{
    switch(sig) {
    case SIGINT:
        std::cout << "SIGINT coming, stop" << std::endl;
        should_stop = 1;
        break;
    case SIGQUIT:
        std::cout << "SIGQUIT coming, stop" << std::endl;
        should_stop = 1;
        break;
    case SIGHUP:
        std::cout << "SIGHUP coming, stop" << std::endl;
        should_stop = 1;
        break;
    case SIGPIPE:
        //When the client is closed after start scaning and parsing,
        //this signal will come, ignore it!
        std::cout << "do nothings for PIPE signal" << std::endl;
        break;
    }
}

std::string file_src_alsa_sink_string =
"{"
    "\"foobar_pipeline\" : {"
        "\"engine\" : \"file_src\","
        "\"elements\" : {"
            "\"file_src\" : {"
                "\"element_type\" : \"FileSrc\","
                "\"type\" : \"wav\","
                "\"path\" : \"/tmp/src.wav\","
                "\"loop_frames\" : 160,"
                "\"src_pads\" : {"
                    "\"src_pad\" : {"
                        "\"peer_element\" : \"alsa_sink\","
                        "\"peer_pad\" : \"default\""
                    "}"
                "}"
            "},"
            "\"alsa_sink\" : {"
                "\"element_type\" : \"AlsaSink\","
                "\"devices\" : {"
                    "\"default\" : {"
                        "\"device\" : \"default\","
                        "\"loop_frames\" : 160,"
                        "\"access\" : \"RW_INTERLEAVED\","
                        "\"format\" : \"S16_LE\","
                        "\"rate\" : 48000,"
                        "\"channels\" : 2,"
                        "\"period_frames\" : 1024,"
                        "\"periods\" : 4"
                    "}"
                "},"
                "\"sink_pads\" : {"
                    "\"default\" : {"
                        "\"peer_element\" : \"file_src\","
                        "\"peer_pad\" : \"src_pad\""
                    "}"
                "}"
            "}"
        "}"
    "}"
"}";

std::string alsa_src_file_sink_string =
"{"
    "\"foobar_pipeline\" : {"
        "\"engine\" : \"alsa_src\","
        "\"elements\" : {"
            "\"file_sink\" : {"
                "\"element_type\" : \"FileSink\","
                "\"type\" : \"wav\","
                "\"path\" : \"/tmp/sink.wav\","
                "\"sink_pads\" : {"
                    "\"sink_pad\" : {"
                        "\"peer_element\" : \"alsa_src\","
                        "\"peer_pad\" : \"default\""
                    "}"
                "}"
            "},"
            "\"alsa_src\" : {"
                "\"element_type\" : \"AlsaSrc\","
                "\"devices\" : {"
                    "\"default\" : {"
                        "\"device\" : \"hw:audiocodec\","
                        "\"loop_frames\" : 160,"
                        "\"access\" : \"RW_INTERLEAVED\","
                        "\"format\" : \"S16_LE\","
                        "\"rate\" : 48000,"
                        "\"channels\" : 2,"
                        "\"period_frames\" : 1024,"
                        "\"periods\" : 4"
                    "}"
                "},"
                "\"src_pads\" : {"
                    "\"default\" : {"
                        "\"peer_element\" : \"file_sink\","
                        "\"peer_pad\" : \"sink_pad\""
                    "}"
                "}"
            "}"
        "}"
    "}"
"}";

TEST(AlsaSinkTest, FileSrcAlsaSink) {
    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDom(file_src_alsa_sink_string.c_str(), &doc));
    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/foobar_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("foobar_pipeline", *config);
    ASSERT_NE(pipeline, nullptr);
    ASSERT_TRUE(pipeline->Activate());

    auto file_src = pipeline->GetElement("file_src");
    ASSERT_NE(file_src, nullptr);
    while(file_src->Loop() >= 0);
}

TEST(AlsaSrcTest, AlsaSrcFileSink) {
    // TODO: must capture the signal, otherwise the FileSink with type "wav" may
    // exit abnormally, causing the lack of total file size in wav header
    signal(SIGHUP, when_signal);
    signal(SIGQUIT, when_signal);
    signal(SIGINT, when_signal);
    signal(SIGPIPE, when_signal);

    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDom(alsa_src_file_sink_string.c_str(), &doc));
    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/foobar_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("foobar_pipeline", *config);
    ASSERT_NE(pipeline, nullptr);
    ASSERT_TRUE(pipeline->Activate());

    auto alsa_src = pipeline->GetElement("alsa_src");
    ASSERT_NE(alsa_src, nullptr);
//    while(alsa_src->Loop() >= 0);
    while (!should_stop && alsa_src->Loop() >= 0);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
