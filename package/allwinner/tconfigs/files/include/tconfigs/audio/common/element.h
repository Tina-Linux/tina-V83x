#ifndef __TCONFIGS_AUDIO_COMMON_ELEMENT_H__
#define __TCONFIGS_AUDIO_COMMON_ELEMENT_H__

#include <string>
#include <memory>
#include <unordered_set>
#include <map>
#include <initializer_list>
#include "tconfigs/common/any.h"
#include "tconfigs/common/signal_slot.h"
#include "tconfigs/audio/common/pad.h"
#include "tconfigs/audio/common/state.h"
#include "tconfigs/audio/common/message.h"

namespace tconfigs {
namespace audio {

class Element {
public:
    enum class Mode {
        kUnknown,
        kPush,
        kPull,
        kPullPush
    };

    Element(void) = delete;
    Element(const std::string& name);
    virtual ~Element(void) = default;

    /**
     * @brief Main loop function if the element is used as the engine of pipeline
     */
    virtual int Loop(void) { return 0; }
    /**
     * @brief Chain function when the element works in @c kPush mode
     *
     * @param pad The pad that calls the chain function
     * @return A value greater than or equal to 0 means successful, otherwise a
     *      negative value (The specific meaning of the value is defined by the
     *      derived class)
     */
    virtual int PushChain(std::shared_ptr<Pad> pad) { return -EPERM; }
    /**
     * @brief Chain function when the element works in @c kPull mode
     *
     * @param pad The pad that calls the chain function
     * @return A value greater than or equal to 0 means successful, otherwise a
     *      negative value (The specific meaning of the value is defined by the
     *      derived class)
     */
    virtual int PullChain(std::shared_ptr<Pad> pad) { return -EPERM; }

    std::shared_ptr<Pad> FindPad(const std::string& name, Pad::Direction direction);

    template <typename... Args>
    std::shared_ptr<common::Signal<Args...>> FindSignal(const std::string& name) {
        auto itr = signals_.find(name);
        if (itr == signals_.end()) {
            return nullptr;
        }
        return itr->second.Cast<std::shared_ptr<common::Signal<Args...>>>();
    }

    template <typename... Args>
    std::shared_ptr<common::Slot<Args...>> FindSlot(const std::string& name) {
        auto itr = slots_.find(name);
        if (itr == slots_.end()) {
            return nullptr;
        }
        return itr->second.Cast<std::shared_ptr<common::Slot<Args...>>>();
    }

    bool ModeIsAvailable(Mode mode);

    Mode EngineMode(void);
    virtual bool Activate(Mode mode) = 0;
    virtual bool Deactivate(void) = 0;

    static std::string ModeToString(Mode mode);

    const std::string& name(void) const { return name_; }
    const std::vector<std::shared_ptr<Pad>>* pads(Pad::Direction direction) {
        switch (direction) {
        case Pad::Direction::kSrc: return &src_pads_;
        case Pad::Direction::kSink: return &sink_pads_;
        default: return nullptr; }
    }
    const BufferProperty::Mask* property_mask(void) const {
        return &property_mask_; }
    Mode mode(void) const { return mode_; }

    std::shared_ptr<MessageSender> internal_message_sender(void) {
        return internal_message_sender_;
    }
    std::shared_ptr<MessageSender> external_message_sender(void) {
        return external_message_sender_;
    }
    const std::map<std::string, std::function<void (void)>>*
            internal_message_callbacks(void) const {
        return &internal_message_callbacks_;
    }
    const std::map<std::string, std::function<void (void)>>*
            external_message_callbacks(void) const {
        return &external_message_callbacks_;
    }

protected:
    bool AddPad(const std::string& name, Pad::Direction direction,
            std::shared_ptr<Element> parent);
    void UpdatePropertyMask(void);
    void AddAvailableMode(std::initializer_list<Mode> mode);
    bool ActivateDefault(Mode mode);
    bool DeactivateDefault(void);

    // Here the "internal" means "inside the whole pipeline" but not "inside the
    // element". Similarly, "external" means "outside the pipeline" but not
    // "outside the element".
    bool CreateInternalMessageSender(void);
    bool CreateExternalMessageSender(void);
    void SendInternalMessage(const Message& message);
    void SendExternalMessage(const Message& message);
    bool AddInternalMessageCallback(const std::string& message,
            std::function<void (void)> callback);
    bool AddExternalMessageCallback(const std::string& message,
            std::function<void (void)> callback);

    bool AddStateChangeMessageCallback(StateChangeDirection dir,
            State current_state, std::function<void (void)> callback);

    template <typename... Args>
    bool AddSignal(const std::string& name) {
        std::shared_ptr<common::Signal<Args...>> sig =
            common::SignalCreate<Args...>(name);
        if (!sig) {
            return false;
        }
        signals_.insert({name, sig});
        return true;
    }

    template <typename... Args>
    bool AddSlot(const std::string& name,
            typename common::Slot<Args...>::CallbackType callback) {
        std::shared_ptr<common::Slot<Args...>> slot =
            common::SlotCreate<Args...>(name, callback);
        if (!slot) {
            return false;
        }
        slots_.insert({name, slot});
        return true;
    }

private:
    bool SetMode(Mode mode);
    void SetPadsMode(std::vector<std::shared_ptr<Pad>>* pads, Pad::Mode pad_mode);

    std::string name_;
    std::vector<std::shared_ptr<Pad>> src_pads_;
    std::vector<std::shared_ptr<Pad>> sink_pads_;

    BufferProperty::Mask property_mask_;

    Mode mode_ = Mode::kUnknown;
    std::unordered_set<Mode> available_mode_;

    std::map<std::string, common::Any> signals_;
    std::map<std::string, common::Any> slots_;

    std::shared_ptr<MessageSender> internal_message_sender_ = nullptr;
    std::shared_ptr<MessageSender> external_message_sender_ = nullptr;
    std::map<std::string, std::function<void (void)>> internal_message_callbacks_;
    std::map<std::string, std::function<void (void)>> external_message_callbacks_;

    static const std::unordered_map<Mode, std::string> kModeToStringMap;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_ELEMENT_H__ */
