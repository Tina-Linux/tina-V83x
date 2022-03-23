#ifndef __TCONFIGS_INPUT_KEY_INPUT_EVENT_HANDLER_H__
#define __TCONFIGS_INPUT_KEY_INPUT_EVENT_HANDLER_H__

#include <memory>
#include <unordered_map>

#include "tconfigs/input/input_event_handler_interface.h"
#include "tconfigs/input/key_handler.h"

namespace tconfigs {
namespace input {

class KeyInputEventHandler : public InputEventHandlerInterface {
public:
    static std::shared_ptr<KeyInputEventHandler> Create(void);
    ~KeyInputEventHandler(void) = default;

    bool AddKeyHandler(std::shared_ptr<KeyHandler> handler);
    bool RemoveKeyHandler(int code);

    void OnEvent(const struct input_event* event) override;

private:
    KeyInputEventHandler(void) = default;

    std::unordered_map<int, std::shared_ptr<KeyHandler>> key_handlers_;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_KEY_INPUT_EVENT_HANDLER_H__ */
