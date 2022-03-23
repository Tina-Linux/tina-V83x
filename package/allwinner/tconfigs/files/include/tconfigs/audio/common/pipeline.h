#ifndef __TCONFIGS_AUDIO_COMMON_PIPELINE_H__
#define __TCONFIGS_AUDIO_COMMON_PIPELINE_H__

#include <initializer_list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/state.h"
#include "tconfigs/audio/common/element.h"
#include "tconfigs/audio/common/bus.h"

namespace tconfigs {
namespace audio {

class Pipeline {
public:
    // @deprecated (TODO: remove it?)
    static std::shared_ptr<Pipeline> Create(const std::string& name);

    static std::shared_ptr<Pipeline> Create(const std::string& name,
            const rapidjson::Value& config);

    ~Pipeline(void);

    // @deprecated (TODO: remove it?)
    void AddElements(std::initializer_list<std::shared_ptr<Element>> elements);

    std::shared_ptr<Element> GetElement(const std::string& element_name);

    // @deprecated (TODO: remove them?)
    bool LinkPads(std::shared_ptr<Pad> pad1, std::shared_ptr<Pad> pad2,
            std::shared_ptr<BufferInterface> buffer);
    bool LinkPads(
            std::shared_ptr<Element> src_element, const std::string src_pad_name,
            std::shared_ptr<Element> sink_element, const std::string sink_pad_name,
            std::shared_ptr<BufferInterface> buffer);

    bool AddExternalMessageCallback(const std::string& message,
            std::function<void (void)> callback);

    template <typename... Args>
    bool ConnectElementSignal(const std::string& element_name,
            const std::string& signal_name,
            typename common::Slot<Args...>::CallbackType callback) {
        auto element = GetElement(element_name);
        if (!element) {
            return false;
        }
        auto signal = element->FindSignal<Args...>(signal_name);
        if (!signal) {
            return false;
        }
        auto slot = common::SlotCreate<Args...>(signal_name, callback);
        if (!slot) {
            return false;
        }
        signal->Connect(slot);
    }

    bool SetState(State state);

    const std::string& name(void) const { return name_; }
    int num_elements(void) const { return elements_.size(); }

private:
    typedef bool (Pipeline::*StateChangeFunc)(void);

    Pipeline(void) = delete;
    Pipeline(const std::string& name);

    bool Init(const rapidjson::Value& config);

    bool CreateElementsByConfig(const rapidjson::Value& elements_config);

    bool LinkElementsByConfig(const rapidjson::Value& elements_config);
    bool ElementLinkPeerPadsByConfig(std::shared_ptr<Element> element,
            Pad::Direction direction, const rapidjson::Value& config);

    bool CreateAndLinkBuffers(void);
    bool ElementLinkBuffers(std::shared_ptr<Element> element, Pad::Direction direction);
    void CompletePadsEachOtherProperty(
            std::shared_ptr<Pad> p1, std::shared_ptr<Pad> p2, BufferProperty::Mask* mask);
    bool CompletePadPropertyRecursively(
            std::shared_ptr<Pad> base_pad, std::shared_ptr<Pad> start_pad,
            Pad::Direction direction, BufferProperty::Mask* mask);

    bool GetEngineFromConfig(const rapidjson::Value& config);

    bool CreateMessageComponents(int internal_bus_threads, int external_bus_threads);
    bool RegisterPipelineMessageComponents(void);
    bool RegisterElementsMessageComponents(void);

    bool Activate(void);
    bool ActivateElementsRecursively(std::shared_ptr<Element> element, Element::Mode mode);
    bool Deactivate(void);

    void ElementsStateChange(State current, State target);

    void MainLoop(void);

    void PrintAllProperties(void);

    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<Element>> elements_;

    std::shared_ptr<Element> engine_ = nullptr;

    State state_ = State::kNull;
    std::mutex state_mutex_;
    std::condition_variable state_cond_var_;
    std::mutex state_set_mutex_;

    std::shared_ptr<std::thread> main_loop_thread_ = nullptr;

    std::shared_ptr<Bus> internal_bus_ = nullptr;
    std::shared_ptr<Bus> external_bus_ = nullptr;
    std::shared_ptr<MessageSender> internal_message_sender_ = nullptr;
    std::shared_ptr<MessageSender> external_message_sender_ = nullptr;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_PIPELINE_H__ */
