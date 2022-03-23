#include <gtest/gtest.h>

#include "tconfigs/audio/common/pipeline.h"

std::string pipeline_string =
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
        "}"
    "}"
"}";

TEST(PipelineTest, CreateByConfig) {
    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDom(pipeline_string.c_str(), &doc));
    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/foobar_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("foobar_pipeline", *config);
    ASSERT_NE(pipeline, nullptr);
    //ASSERT_TRUE(pipeline->Activate());
    pipeline->SetState(tconfigs::audio::Pipeline::State::kPlaying);
    auto file_src = pipeline->GetElement("file_src");
    ASSERT_NE(file_src, nullptr);
//    file_src->Loop();
    while(file_src->Loop() > 0);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
