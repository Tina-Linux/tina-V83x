#include "tconfigs/audio/common/message.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

// Message =====================================================================
const std::unordered_map<std::string, Message::Priority> Message::kStringToPriorityMap = {
    {"high", Priority::kHigh},
    {"normal", Priority::kNormal},
    {"sync", Priority::kSync},
};

bool Message::UpdateFromConfig(Message* message, const rapidjson::Value& config)
{
    bool is_updated = false;
    const char* string_value = nullptr;

    if (json::pointer::GetString(config, "/priority", &string_value)) {
        auto itr = kStringToPriorityMap.find(string_value);
        if (itr == kStringToPriorityMap.end()) {
            TCLOG_ERROR("unrecognized priority type: \"%s\"", string_value);
        } else {
            message->set_priority(itr->second);
            is_updated = true;
        }
    }

    if (json::pointer::GetString(config, "/content", &string_value)) {
        message->set_content(string_value);
        is_updated = true;
    }

    return is_updated;
}

Message::Message(Priority priority, const std::string& content)
    : priority_(priority),
      content_(content)
{
}

// MessageSender ===============================================================
std::shared_ptr<MessageSender> MessageSender::Create(void)
{
    auto sender = std::shared_ptr<MessageSender>(new MessageSender());
    if (!sender || !sender->Init()) {
        TCLOG_ERROR("Fail to create MessageSender");
        return nullptr;
    }
    return sender;
}

bool MessageSender::Init(void)
{
    signal_ = common::SignalCreate<const Message&>("sender");
    if (!signal_) {
        TCLOG_ERROR("Fail to create internal signal in MessageSender");
        return false;
    }
    return true;
}

void MessageSender::Connect(std::shared_ptr<MessageReceiver> receiver)
{
    signal_->Connect(receiver->slot());
}

void MessageSender::Emit(const Message& message)
{
    signal_->Emit(message);
}

// MessageReceiver =============================================================
std::shared_ptr<MessageReceiver> MessageReceiver::Create(
            std::function<void (const Message&)> callback)
{
    auto receiver = std::shared_ptr<MessageReceiver>(new MessageReceiver());
    if (!receiver || !receiver->Init(callback)) {
        TCLOG_ERROR("Fail to create MessageReceiver");
        return nullptr;
    }
    return receiver;
}

bool MessageReceiver::Init(std::function<void (const Message&)> callback)
{
    slot_ = common::SlotCreate<const Message&>("receiver", callback);
    if (!slot_) {
        TCLOG_ERROR("Fail to create internal slot in MessageReceiver");
        return false;
    }
    return true;
}

} // namespace audio
} // namespace tconfigs
