#include <string>
#include <gtest/gtest.h>

#include "tconfigs/file/file_utils.h"
#include "tconfigs/audio/common/pipeline.h"

std::string json_file = "/etc/tconfigs/test/unit_test_file_src_converter_file_sink.json";

TEST(ConverterTest, Complete) {
    tconfigs::file::FileUtils file;
    auto json_string = tconfigs::json::JsonStringFromFile::Create(json_file);
    ASSERT_NE(json_string, nullptr);

    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDomRelaxed(json_string->string(), &doc))
        << "The JSON string is:" << std::endl << json_string->string();

    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/complete_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("complete", *config);
    ASSERT_NE(pipeline, nullptr);
    ASSERT_TRUE(pipeline->Activate());

    auto file_src = pipeline->GetElement("file_src");
    ASSERT_NE(file_src, nullptr);
    while (file_src->Loop() >= 0);
}

TEST(ConverterTest, Simplest) {
    tconfigs::file::FileUtils file;
    auto json_string = tconfigs::json::JsonStringFromFile::Create(json_file);
    ASSERT_NE(json_string, nullptr);

    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDomRelaxed(json_string->string(), &doc));

    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/simplest_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("simplest", *config);
    ASSERT_NE(pipeline, nullptr);
    ASSERT_TRUE(pipeline->Activate());

    auto file_src = pipeline->GetElement("file_src");
    ASSERT_NE(file_src, nullptr);
    while (file_src->Loop() >= 0);
}

TEST(ConverterTest, Divide) {
    tconfigs::file::FileUtils file;
    auto json_string = tconfigs::json::JsonStringFromFile::Create(json_file);
    ASSERT_NE(json_string, nullptr);

    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDomRelaxed(json_string->string(), &doc));

    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/divide_pipeline", &config));
    auto pipeline = tconfigs::audio::Pipeline::Create("divide", *config);
    ASSERT_NE(pipeline, nullptr);
    ASSERT_TRUE(pipeline->Activate());

    auto file_src = pipeline->GetElement("file_src");
    ASSERT_NE(file_src, nullptr);
    while (file_src->Loop() >= 0);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
