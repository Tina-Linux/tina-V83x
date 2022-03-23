#ifndef __TCONFIGS_INPUT_INPUT_EVENT_HANDLER_INTERFACE_H__
#define __TCONFIGS_INPUT_INPUT_EVENT_HANDLER_INTERFACE_H__

#include <linux/input.h>

namespace tconfigs {
namespace input {

class InputEventHandlerInterface {
public:
    virtual ~InputEventHandlerInterface(void) = default;

    virtual void OnEvent(const struct input_event* event) = 0;
};

} // namespace input
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_INPUT_INPUT_EVENT_INTERFACE_HANDLER_H__ */
