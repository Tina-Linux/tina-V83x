#ifndef __TCONFIGS_AUDIO_COMMON_MESSAGE_H__
#define __TCONFIGS_AUDIO_COMMON_MESSAGE_H__

#include <string>
#include <memory>
#include <unordered_map>
#include "tconfigs/common/signal_slot.h"
#include "tconfigs/json/json_utils.h"

namespace tconfigs {
namespace audio {

class Message {
public:
    enum class Priority {
        kHigh,      // Asynchronously with high priority
        kNormal,    // Asynchronously with normal priority
        kSync,      // Synchronously
    };

    Message(void) = default;
    Message(Priority priority, const std::string& content);
    ~Message(void) = default;

    void set_priority(Priority priority) { priority_ = priority; }
    void set_content(const std::string& content) { content_ = content; }

    Priority priority(void) const { return priority_; }
    const std::string& content(void) const { return content_; }

    /**
     * @return @c true means that the @c message has been updated, otherwise
     *      @c false means that the @c message hasn't changed.
     */
    static bool UpdateFromConfig(Message* message, const rapidjson::Value& config);

private:
    Priority priority_ = Priority::kSync;
    std::string content_;

    static const std::unordered_map<std::string, Priority> kStringToPriorityMap;
};

class MessageReceiver;

class MessageSender {
public:
    static std::shared_ptr<MessageSender> Create(void);

    ~MessageSender(void) = default;

    void Connect(std::shared_ptr<MessageReceiver> receiver);

    void Emit(const Message& message);

private:
    MessageSender(void) = default;

    bool Init(void);

    std::shared_ptr<common::Signal<const Message&>> signal_ = nullptr;
};

class MessageReceiver {
public:
    static std::shared_ptr<MessageReceiver> Create(
            std::function<void (const Message&)> callback);

    ~MessageReceiver(void) = default;

    std::shared_ptr<common::Slot<const Message&>>
        slot(void) { return slot_; }

private:
    MessageReceiver(void) = default;

    bool Init(std::function<void (const Message&)> callback);

    std::shared_ptr<common::Slot<const Message&>> slot_ = nullptr;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_MESSAGE_H__ */
