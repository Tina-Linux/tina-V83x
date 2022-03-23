#include <gtest/gtest.h>

#include "tconfigs/audio/common/bus.h"

bool is_stop = false;

void WhenSignal(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGHUP:
        is_stop = true;
        break;
    case SIGPIPE:
    default:
        break;
    }
}

TEST(BusTest, Synchronous) {
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto bus = tconfigs::audio::Bus::Create(0);
    ASSERT_NE(bus, nullptr);

    auto sender1 = tconfigs::audio::MessageSender::Create();
    ASSERT_NE(sender1, nullptr);
    auto sender2 = tconfigs::audio::MessageSender::Create();
    ASSERT_NE(sender2, nullptr);

    ASSERT_TRUE(bus->RegisterSender(sender1));
    ASSERT_TRUE(bus->RegisterSender(sender2));
    ASSERT_TRUE(bus->RegisterCallback("message1", [] {
                    std::cout << "sender1 message got" << std::endl;
                    sleep(1);
                }));
    ASSERT_TRUE(bus->RegisterCallback("message1", [] {
                    std::cout << "sender1 message got 2" << std::endl;
                    sleep(1);
                }));
    ASSERT_TRUE(bus->RegisterCallback("message2", [] {
                    std::cout << "sender2 message got" << std::endl;
                    sleep(2);
                }));

    while (!is_stop) {
        sender1->Emit({tconfigs::audio::Message::Priority::kNormal, "message1"});
        sender2->Emit({tconfigs::audio::Message::Priority::kNormal, "message2"});
//        sender2->Emit(tconfigs::audio::Message::Priority::kNormal, "message");
    }
}

TEST(BusTest, Asynchronous) {
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto bus = tconfigs::audio::Bus::Create(1);
    ASSERT_NE(bus, nullptr);

    auto sender1 = tconfigs::audio::MessageSender::Create();
    ASSERT_NE(sender1, nullptr);
    auto sender2 = tconfigs::audio::MessageSender::Create();
    ASSERT_NE(sender2, nullptr);

    ASSERT_TRUE(bus->RegisterSender(sender1));
    ASSERT_TRUE(bus->RegisterSender(sender2));
    ASSERT_TRUE(bus->RegisterCallback("message1", [] {
                    std::cout << "sender1 message got" << std::endl;
                }));
    ASSERT_TRUE(bus->RegisterCallback("message1", [] {
                    std::cout << "sender1 message got 2" << std::endl;
                }));
    ASSERT_TRUE(bus->RegisterCallback("message2", [] {
                    sleep(2);
                    std::cout << "sender2 message got" << std::endl;
                }));

    while (!is_stop) {
        sender1->Emit({tconfigs::audio::Message::Priority::kNormal, "message1"});
        std::cout << "sender1 emit end" << std::endl;
        sender2->Emit({tconfigs::audio::Message::Priority::kNormal, "message2"});
        sleep(3);
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
