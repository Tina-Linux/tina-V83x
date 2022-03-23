#include "tconfigs/input/key_input_event_handler.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace input {

std::shared_ptr<KeyInputEventHandler> KeyInputEventHandler::Create(void)
{
    return std::shared_ptr<KeyInputEventHandler>(new KeyInputEventHandler);
}

bool KeyInputEventHandler::AddKeyHandler(std::shared_ptr<KeyHandler> handler)
{
    auto ret = key_handlers_.insert({handler->code(), handler});
    if (!ret.second) {
        TCLOG_WARNING("Add nothing (Handler of key code %d has been already added)",
                handler->code());
        return false;
    }
    return true;
}

bool KeyInputEventHandler::RemoveKeyHandler(int code)
{
    if (!key_handlers_.erase(code)) {
        TCLOG_WARNING("Remove nothing (Handler of key code %d not found)", code);
        return false;
    }
    return true;
}

void KeyInputEventHandler::OnEvent(const struct input_event* event)
{
    if (event->type != EV_KEY) {
        return;
    }

    auto itr = key_handlers_.find(event->code);
    if (itr == key_handlers_.end()) {
        TCLOG_DEBUG("Handler of key code %d not found", event->code);
        return;
    }

    switch (event->value) {
    case 0: // release
        itr->second->OnRelease(event);
        break;
    case 1: // press
        itr->second->OnPress(event);
        break;
    default:
        return;
    }
}

} // namespace input
} // namespace tconfigs
