#include <cstdlib>
#include <gtest/gtest.h>

#include "tconfigs/event/event_loop.h"
#include "tconfigs/event/event_watcher_interface.h"

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

class DemoIoWatcher : public tconfigs::event::EventIoWatcherInterface {
public:
    int GetFd(void) override { return STDIN_FILENO; }
    EventType GetEventType(void) override { return EventType::kReadable; }
    bool IsOneShot(void) override { return true; }

    void OnEvent(int fd, EventType event_type) override {
        std::cout << "fd " << fd << " is readable" << std::endl;
    }
};

class DemoTimerWatcher : public tconfigs::event::EventTimerWatcherInterface {
public:
    double GetTimeoutSeconds(void) override { return 3; }
    double GetRepeatIntervalSeconds(void) override { return 2; }

    void OnEvent(double* repeat_interval_sec) override {
        ++count_;
        std::cout << "count: " << count_
            << ", repeat_interval: " << *repeat_interval_sec << std::endl;
        if (count_ >= 3) {
            *repeat_interval_sec = 0.7;
        }
        if (count_ >= 6) {
            *repeat_interval_sec = 0;
        }
    }
private:
    int count_ = 0;
};

TEST(EventLoopTest, NormalStartStop) {
    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);

    std::cout << "Start" << std::endl;
    ASSERT_TRUE(event_loop->Start());
    sleep(1);
    std::cout << "Stop" << std::endl;
    ASSERT_TRUE(event_loop->Stop());
}

TEST(EventLoopTest, StartWithoutStop) {
    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);

    std::cout << "Start" << std::endl;
    ASSERT_TRUE(event_loop->Start());
    sleep(1);
}

TEST(EventLoopTest, IoWatcher) {
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);

    auto demo = std::shared_ptr<DemoIoWatcher>(new DemoIoWatcher());
    ASSERT_NE(demo, nullptr);
    ASSERT_TRUE(event_loop->AddIoWatcher(demo));
    ASSERT_TRUE(event_loop->Start());
    while (!is_stop) {
        sleep(1);
    }
    ASSERT_TRUE(event_loop->Stop());
}

TEST(EventLoopTest, TimerWatcher) {
    is_stop = false;
    signal(SIGHUP, WhenSignal);
    signal(SIGQUIT, WhenSignal);
    signal(SIGINT, WhenSignal);
    signal(SIGPIPE, WhenSignal);

    auto event_loop = tconfigs::event::EventLoop::Create();
    ASSERT_NE(event_loop, nullptr);

    auto demo = std::shared_ptr<DemoTimerWatcher>(new DemoTimerWatcher());
    ASSERT_NE(demo, nullptr);
    ASSERT_TRUE(event_loop->AddTimerWatcher(demo));
    ASSERT_TRUE(event_loop->Start());
    while (!is_stop) {
        sleep(1);
    }
    ASSERT_TRUE(event_loop->Stop());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
