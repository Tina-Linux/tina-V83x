#include "tconfigs/input/key_handler.h"

#include "tconfigs/log/logging.h"
#include "tconfigs/common/time.h"

namespace tconfigs {
namespace input {

std::shared_ptr<KeyHandler> KeyHandler::Create(int code,
        std::shared_ptr<event::EventLoop> event_loop,
        std::shared_ptr<threading::Executor> executor)
{
    return std::shared_ptr<KeyHandler>(new KeyHandler(code, event_loop, executor));
}

KeyHandler::KeyHandler(int code, std::shared_ptr<event::EventLoop> event_loop,
        std::shared_ptr<threading::Executor> executor)
    : code_(code),
      event_loop_(event_loop),
      executor_(executor)
{
}

bool KeyHandler::AddPressMotion(std::function<void (void)> callback)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    if (!callback) {
        TCLOG_ERROR("The callback of press motion is not callable");
        return false;
    }
    motions_ |= kPress;
    press_callbacks_.push_back(callback);
    return true;
}

bool KeyHandler::AddReleaseMotion(std::function<void (void)> callback)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    if (!callback) {
        TCLOG_ERROR("The callback of release motion is not callable");
        return false;
    }
    motions_ |= kRelease;
    release_callbacks_.push_back(callback);
    return true;
}

bool KeyHandler::AddLongPressPreReleaseMotion(
        double timeout_sec, std::function<void (void)> callback)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    if (!callback) {
        TCLOG_ERROR("The callback of long press motion (pre release) is not callable");
        return false;
    }
    motions_ |= kLongPressPreRelease;
    auto handler =
        KeyLongPressPreReleaseHandler::Create(timeout_sec, callback, shared_from_this());
    auto watcher = KeyPressDurationWatcher::Create(handler);
    press_duration_watchers_.push_back(watcher);
    return true;
}

bool KeyHandler::AddLongPressPostReleaseMotion(
        const timeval* timeout, std::function<void (void)> callback)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    if (!callback) {
        TCLOG_ERROR("The callback of long press motion (post release) is not callable");
        return false;
    }
    motions_ |= kLongPressPostRelease;
    long_press_post_release_callbacks_.push_back({*timeout, callback});
    return true;
}

void KeyHandler::OnPress(const struct input_event* event)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    state_ = State::kPressing;
    press_time_ = event->time;
    motion_has_been_responded_ = false;

    if (MotionWillBeResponded(kPress)) {
        for (auto itr = press_callbacks_.begin(); itr != press_callbacks_.end(); ++itr) {
            executor_->SubmitBack(*itr);
        }
        if (response_style_ == ResponseStyle::kOneMotion) {
            motion_has_been_responded_ = true;
        }
    }

    if (MotionWillBeResponded(kLongPressPreRelease)) {
        auto event_loop = event_loop_.lock();
        if (!event_loop) {
            TCLOG_ERROR("Invalid event loop in KeyHandler");
            return;
        }
        for (auto itr = press_duration_watchers_.begin();
                itr != press_duration_watchers_.end(); ++itr) {
            if (!event_loop->AddTimerWatcher(*itr)) {
                TCLOG_ERROR("Fail to add timer watcher for watching press duration");
                return;
            }
        }
    }
}

void KeyHandler::OnRelease(const struct input_event* event)
{
    std::unique_lock<std::mutex> mutex_lock(mutex_);
    state_ = State::kReleased;

    if (MotionWillBeResponded(kLongPressPreRelease)) {
        auto event_loop = event_loop_.lock();
        if (!event_loop) {
            TCLOG_ERROR("Invalid event loop in KeyHandler");
            return;
        }
        for (auto itr = press_duration_watchers_.begin();
                itr != press_duration_watchers_.end(); ++itr) {
            event_loop->RemoveTimerWatcher(*itr);
        }
    }

    if (motion_has_been_responded_) {
        return;
    }
    if (MotionWillBeResponded(kLongPressPostRelease)) {
        timeval press_duration;
        if (!common::TimeInterval(&press_time_, &(event->time), &press_duration)) {
            TCLOG_ERROR("Cannot get long press duration");
            return;
        }
        TCLOG_DEBUG("Press duration: %lds, %ldus", press_duration.tv_sec, press_duration.tv_usec);
        for (auto itr = long_press_post_release_callbacks_.begin();
                itr != long_press_post_release_callbacks_.end(); ++itr) {
            if (common::TimeIsGreaterThanOrEqualTo(&press_duration, &(*itr).timeout)) {
                executor_->SubmitBack((*itr).func);
            }
        }
        if (response_style_ == ResponseStyle::kOneMotion) {
            motion_has_been_responded_ = true;
        }
    }

    // The motion @c kRelease should be checked after @c kLongPressPostRelease.
    // Therefore we can do some release operations in the @c kRelease motion
    // callback, and needn't concern about whether the @c kLongPressPostRelease
    // motion callback exists or not.

    if (motion_has_been_responded_) {
        return;
    }
    if (MotionWillBeResponded(kRelease)) {
        for (auto itr = release_callbacks_.begin(); itr != release_callbacks_.end(); ++itr) {
            executor_->SubmitBack(*itr);
        }
        if (response_style_ == ResponseStyle::kOneMotion) {
            motion_has_been_responded_ = true;
        }
    }
}

// KeyLongPressPreReleaseHandler ===============================================
std::shared_ptr<KeyHandler::KeyLongPressPreReleaseHandler>
KeyHandler::KeyLongPressPreReleaseHandler::Create(double timeout_sec,
        std::function<void (void)> callback, std::shared_ptr<KeyHandler> parent)
{
    return std::shared_ptr<KeyLongPressPreReleaseHandler>(
            new KeyLongPressPreReleaseHandler(timeout_sec, callback, parent));
}

KeyHandler::KeyLongPressPreReleaseHandler::KeyLongPressPreReleaseHandler(
        double timeout_sec, std::function<void (void)> callback,
        std::shared_ptr<KeyHandler> parent)
    : timeout_sec_(timeout_sec),
      callback_(callback),
      parent_(parent)
{
}

void KeyHandler::KeyLongPressPreReleaseHandler::OnMotion(void)
{
    auto parent = parent_.lock();
    if (!parent) {
        TCLOG_ERROR("Invalid parent KeyHandler");
        return;
    }
    std::unique_lock<std::mutex> mutex_lock(parent->mutex_);
    if (!parent->MotionWillBeResponded(kLongPressPreRelease)) {
        return;
    }
    if (parent->state_ != State::kPressing) {
        return;
    }
    parent->executor_->SubmitBack(callback_);

    if (parent->response_style_ == ResponseStyle::kOneMotion) {
        parent->motion_has_been_responded_ = true;
    }
}

// KeyPressDurationWatcher =====================================================
std::shared_ptr<KeyHandler::KeyPressDurationWatcher>
KeyHandler::KeyPressDurationWatcher::Create(
        std::shared_ptr<KeyLongPressPreReleaseHandler> handler)
{
    return std::shared_ptr<KeyPressDurationWatcher>(new KeyPressDurationWatcher(handler));
}

KeyHandler::KeyPressDurationWatcher::KeyPressDurationWatcher(
        std::shared_ptr<KeyLongPressPreReleaseHandler> handler)
    : handler_(handler)
{
}

void KeyHandler::KeyPressDurationWatcher::OnEvent(double* repeat_interval_sec)
{
    handler_->OnMotion();
}

} // namespace input
} // namespace tconfigs
