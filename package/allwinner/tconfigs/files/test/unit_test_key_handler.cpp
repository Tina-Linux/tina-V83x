#include <gtest/gtest.h>

#include "tconfigs/event/event_loop.h"
#include "tconfigs/input/input_event_watcher.h"
#include "tconfigs/input/key_input_event_handler.h"
#include "tconfigs/input/key_handler.h"
#include "tconfigs/threading/executor.h"

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

TEST(KeyHandlerTest, SunxiKeyboard)
{
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto executor = tconfigs::threading::Executor::Create(2);
    ASSERT_NE(executor, nullptr);

    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);
    auto input_event_watcher = tconfigs::input::InputEventWatcher::Create("sunxi-keyboard");
    ASSERT_NE(input_event_watcher, nullptr);
    auto key_input_event_handler = tconfigs::input::KeyInputEventHandler::Create();
    ASSERT_NE(key_input_event_handler, nullptr);
    auto key_handler = tconfigs::input::KeyHandler::Create(114, event_loop, executor);
    ASSERT_NE(key_handler, nullptr);

    key_handler->set_response_style(tconfigs::input::KeyHandler::ResponseStyle::kAllMotions);
    key_handler->AddPressMotion([] {
            std::cout << "114 press and sleep 1 sec" << std::endl;
            sleep(1);
            std::cout << "114 press sleep end" << std::endl; });
    key_handler->AddReleaseMotion(
            [] { std::cout << "114 release" << std::endl; });
    timeval timeout = { 5, 0 };
    key_handler->AddLongPressPostReleaseMotion(&timeout,
            [] { std::cout << "114 long press post release more than 5 secs" << std::endl; });
    key_handler->AddLongPressPreReleaseMotion(3,
            [] { std::cout << "114 long press pre release 3 secs" << std::endl; });
    key_handler->AddLongPressPreReleaseMotion(4,
            [] { std::cout << "114 long press pre release 4 secs" << std::endl; });

    key_input_event_handler->AddKeyHandler(key_handler);
    input_event_watcher->AddHandler(key_input_event_handler);
    event_loop->AddIoWatcher(input_event_watcher);

    ASSERT_TRUE(event_loop->Start());
    while (!is_stop) {
        sleep(1);
    }
    std::cout << "Stop" << std::endl;
//    ASSERT_TRUE(event_loop->Stop());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
