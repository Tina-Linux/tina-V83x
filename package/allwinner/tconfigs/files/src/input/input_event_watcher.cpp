#include "tconfigs/input/input_event_watcher.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace input {

std::shared_ptr<InputEventWatcher> InputEventWatcher::Create(
        const std::string& device_name)
{
    auto watcher = std::shared_ptr<InputEventWatcher>(new InputEventWatcher());
    if (!watcher || !watcher->Init(device_name)) {
        TCLOG_ERROR("Fail to create InputEventWatcher for \"%s\"",
                device_name.c_str());
        return nullptr;
    }
    return watcher;
}

bool InputEventWatcher::Init(const std::string& device_name)
{
    input_utils_ = InputUtils::Create(device_name);
    if (!input_utils_) {
        return false;
    }
    return true;
}

void InputEventWatcher::AddHandler(std::shared_ptr<InputEventHandlerInterface> handler)
{
    handlers_.push_back(handler);
}

void InputEventWatcher::OnEvent(int fd, EventType event_type)
{
    if (fd != input_utils_->event_fd()) {
        TCLOG_ERROR("Incorrect file descriptor (current: %d, expected: %d)",
                fd, input_utils_->event_fd());
        return;
    }
    if (event_type != EventType::kReadable) {
        TCLOG_ERROR("Not an event that the file descriptor %d is readable", fd);
        return;
    }
    struct input_event event;
    if (!input_utils_->ReadEvent(&event)) {
        TCLOG_ERROR("Error while reading input event (device: %s)",
                input_utils_->device_name().c_str());
        return;
    }

    if (event.type == EV_SYN) {
        TCLOG_DEBUG("EV_SYN");
        return;
    }

    TCLOG_DEBUG("time: %lds, %ldus; type: %d; code: %d; value: %d",
            event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);

    for (auto itr = handlers_.begin(); itr != handlers_.end(); ++itr) {
        (*itr)->OnEvent(&event);
    }
}

} // namespace input
} // namespace tconfigs
