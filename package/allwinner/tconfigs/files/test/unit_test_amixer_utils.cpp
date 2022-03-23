#include <iostream>
#include <gtest/gtest.h>

#include <alsa/asoundlib.h>

#include "tconfigs/audio/utils/amixer_utils.h"

TEST(AmixerUtilsTest, SetAndGetControl) {
    tconfigs::audio::AmixerUtils utils;
    std::string device = "hw:audiocodec";
    int numid = 3;
    std::string iface = "MIXER";
    std::string name = "Lineout volume";
    std::vector<int> values_get;

    std::cout << "SetControl to 22 by numid" << std::endl;
    ASSERT_TRUE(utils.SetControl(device, numid, "22"));

    ASSERT_TRUE(utils.GetControl(device, numid, &values_get));
    std::cout << "GetControl by numid: count: " << values_get.size()
        << ", first value: " << values_get.front() << std::endl;

    std::cout << "SetControl to 24 by iface & name" << std::endl;
    ASSERT_TRUE(utils.SetControl(device, iface, name, "24"));

    values_get.clear();
    ASSERT_TRUE(utils.GetControl(device, iface, name, &values_get));
    std::cout << "GetControl by numid: count: " << values_get.size()
        << ", first value: " << values_get.front() << std::endl;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
