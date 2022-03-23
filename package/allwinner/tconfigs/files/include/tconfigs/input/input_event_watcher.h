#ifndef __TCONFIGS_INPUT_INPUT_EVENT_WATCHER_H__
#define __TCONFIGS_INPUT_INPUT_EVENT_WATCHER_H__

#include <list>

#include "tconfigs/event/event_watcher_interface.h"
#include "tconfigs/input/input_utils.h"
#include "tconfigs/input/input_event_handler_interface.h"

namespace tconfigs {
namespace input {

class InputEventWatcher : public event::EventIoWatcherInterface {
public:
    static std::shared_ptr<InputEventWatcher> Create(const std::string& device_name);

    ~InputEventWatcher(void) = default;

    int GetFd(void) override { return input_utils_->event_fd(); }
    EventType GetEventType(void) override { return EventType::kReadable; }
    bool IsOneShot(void) override { return false; }

    void OnEvent(int fd, EventType event_type) override;

    void AddHandler(std::shared_ptr<InputEventHandlerInterface> handler);

private:
    InputEventWatcher(void) = default;

    bool Init(const std::string& device_name);

    std::shared_ptr<InputUtils> input_utils_ = nullptr;
    std::list<std::shared_ptr<InputEventHandlerInterface>> handlers_;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_INPUT_EVENT_WATCHER_H__ */
