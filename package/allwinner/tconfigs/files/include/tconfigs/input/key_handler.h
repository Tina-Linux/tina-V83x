#ifndef __TCONFIGS_INPUT_KEY_HANDLER_H__
#define __TCONFIGS_INPUT_KEY_HANDLER_H__

#include <linux/input.h>
#include <sys/time.h>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>

#include "tconfigs/event/event_watcher_interface.h"
#include "tconfigs/event/event_loop.h"
#include "tconfigs/threading/executor.h"

namespace tconfigs {
namespace input {

class KeyHandler : public std::enable_shared_from_this<KeyHandler> {
public:
    // whether respond all motions.
    // "kOneMotion" means that it will only respond to the first motion that
    // has been added. "kAllMotions" means that it will respond to all added
    // motions.
    //
    // For example, we use both AddPressMotion() and AddReleaseMotion() for
    // one key. When using "kOneMotion", only the press motion will be responded
    // to. (Press always comes before release, right?) When using "kAllMotions",
    // both the press and release motion will be responded to.
    enum class ResponseStyle {
        kOneMotion,
        kAllMotions
    };

    static std::shared_ptr<KeyHandler> Create(int code,
            std::shared_ptr<event::EventLoop> event_loop,
            std::shared_ptr<threading::Executor> executor);

    ~KeyHandler(void) = default;

    bool AddPressMotion(std::function<void (void)> callback);
    bool AddReleaseMotion(std::function<void (void)> callback);
    bool AddLongPressPreReleaseMotion(
            double timeout_sec, std::function<void (void)> callback);
    bool AddLongPressPostReleaseMotion(
            const timeval* timeout, std::function<void (void)> callback);

    void OnPress(const struct input_event* event);
    void OnRelease(const struct input_event* event);

    int code(void) const { return code_; }
    ResponseStyle response_style(void) const { return response_style_; }

    void set_response_style(ResponseStyle style) { response_style_ = style; }

private:
    enum Motion : int {
        kNone                   = 0x00,
        kPress                  = 0x01,
        kRelease                = 0x02,
        kLongPressPreRelease    = 0x04,
        kLongPressPostRelease   = 0x08,
    };

    enum class State {
        kReleased,
        kPressing
    };

    struct LongPressPostReleaseCallback {
        timeval timeout;
        std::function<void (void)> func;
    };

    class KeyLongPressPreReleaseHandler {
    public:
        static std::shared_ptr<KeyLongPressPreReleaseHandler> Create(
                double timeout_sec, std::function<void (void)> callback,
                std::shared_ptr<KeyHandler> parent);
        ~KeyLongPressPreReleaseHandler(void) = default;
        void OnMotion(void);
        double timeout_sec(void) const { return timeout_sec_; }
    private:
        KeyLongPressPreReleaseHandler(double timeout_sec,
                std::function<void (void)> callback, std::shared_ptr<KeyHandler> parent);
        double timeout_sec_ = 0;
        std::function<void (void)> callback_;
        std::weak_ptr<KeyHandler> parent_;
    };

    class KeyPressDurationWatcher : public event::EventTimerWatcherInterface {
    public:
        static std::shared_ptr<KeyPressDurationWatcher> Create(
                std::shared_ptr<KeyLongPressPreReleaseHandler> handler);
        ~KeyPressDurationWatcher(void) = default;
        double GetTimeoutSeconds(void) override { return handler_->timeout_sec(); }
        double GetRepeatIntervalSeconds(void) override { return 0; }
        void OnEvent(double* repeat_interval_sec) override;
    private:
        KeyPressDurationWatcher(std::shared_ptr<KeyLongPressPreReleaseHandler> handler);
        std::shared_ptr<KeyLongPressPreReleaseHandler> handler_;
    };

    KeyHandler(void) = delete;
    KeyHandler(int code, std::shared_ptr<event::EventLoop> event_loop,
            std::shared_ptr<threading::Executor> executor);

    bool MotionWillBeResponded(Motion motion) { return motions_ & static_cast<int>(motion); }

    int code_;
    ResponseStyle response_style_ = ResponseStyle::kOneMotion;

    std::mutex mutex_;
    State state_ = State::kReleased;
    timeval press_time_ = {0, 0};
    bool motion_has_been_responded_ = false;

    int motions_ = kNone;

    std::vector<std::function<void (void)>> press_callbacks_;
    std::vector<std::function<void (void)>> release_callbacks_;
    std::vector<LongPressPostReleaseCallback> long_press_post_release_callbacks_;

    std::vector<std::shared_ptr<KeyPressDurationWatcher>> press_duration_watchers_;

    std::weak_ptr<event::EventLoop> event_loop_;
    std::shared_ptr<threading::Executor> executor_;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_KEY_HANDLER_H__ */
