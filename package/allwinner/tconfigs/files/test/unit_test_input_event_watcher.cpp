#include <gtest/gtest.h>

#include "tconfigs/event/event_loop.h"
#include "tconfigs/input/input_event_watcher.h"

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

TEST(InputEventWatcherTest, SunxiKeyboard)
{
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);
    auto watcher = tconfigs::input::InputEventWatcher::Create("sunxi-keyboard");
    ASSERT_NE(watcher, nullptr);
    event_loop->AddIoWatcher(watcher);
    ASSERT_TRUE(event_loop->Start());
    while (!is_stop) {
        sleep(1);
    }
    std::cout << "Stop" << std::endl;
    ASSERT_TRUE(event_loop->Stop());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
