#include <iostream>
#include <gtest/gtest.h>

#include "tconfigs/file/file_utils.h"
#include "tconfigs/audio/volume/volume_manager.h"

std::string json_file = "/etc/tconfigs/test/unit_test_volume_manager.json";

TEST(VolumeManagerTest, Softvol) {
    tconfigs::file::FileUtils file;
    auto json_string = tconfigs::json::JsonStringFromFile::Create(json_file);
    ASSERT_NE(json_string, nullptr);

    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDomRelaxed(json_string->string(), &doc));

    const rapidjson::Value* config = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/volume_manager", &config));

    auto volume_manager = tconfigs::audio::VolumeManager::Create(*config);
    ASSERT_NE(volume_manager, nullptr);

    const std::string kMuteSwitchName = "SpeakerMute";
    const std::string kVolumeControlName = "SpeakerVolume";

    auto mute_switch = volume_manager->FindMuteSwitch(kMuteSwitchName);
    ASSERT_NE(mute_switch, nullptr);
    auto volume_control = volume_manager->FindVolumeControl(kVolumeControlName);
    ASSERT_NE(volume_control, nullptr);

    EXPECT_TRUE(mute_switch->SetMute(true));
    EXPECT_TRUE(mute_switch->IsMute());
    EXPECT_TRUE(mute_switch->SetMute(false));

    int volume = 0;
    EXPECT_TRUE(volume_control->GetVolume(&volume));
    std::cout << "Volume: " << volume << std::endl;
    std::cout << "Set volume to 25" << std::endl;
    EXPECT_TRUE(volume_control->SetVolume(25));
    EXPECT_TRUE(volume_control->GetVolume(&volume));
    std::cout << "Volume: " << volume << std::endl;
    std::cout << "Set volume to 50" << std::endl;
    EXPECT_TRUE(volume_control->SetVolume(50));
    EXPECT_TRUE(volume_control->GetVolume(&volume));
    std::cout << "Volume: " << volume << std::endl;
    std::cout << "Set volume to 75" << std::endl;
    EXPECT_TRUE(volume_control->SetVolume(75));
    EXPECT_TRUE(volume_control->GetVolume(&volume));
    std::cout << "Volume: " << volume << std::endl;
    std::cout << "Set volume to 100" << std::endl;
    EXPECT_TRUE(volume_control->SetVolume(100));
    EXPECT_TRUE(volume_control->GetVolume(&volume));
    std::cout << "Volume: " << volume << std::endl;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
