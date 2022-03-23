#include "tconfigs/audio/common/pipeline.h"

#include <stdlib.h>
#include "tconfigs/log/logging.h"
#include "tconfigs/audio/common/element_factory.h"

namespace tconfigs {
namespace audio {

std::shared_ptr<Pipeline> Pipeline::Create(const std::string& name)
{
    return std::shared_ptr<Pipeline>(new Pipeline(name));
}

std::shared_ptr<Pipeline> Pipeline::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto pipeline = std::shared_ptr<Pipeline>(new Pipeline(name));
    if (!pipeline || !pipeline->Init(config)) {
        TCLOG_ERROR("Fail to create pipeline \"%s\"", name.c_str());
        return nullptr;
    }
    return pipeline;
}

Pipeline::Pipeline(const std::string& name)
    : name_(name)
{
}

Pipeline::~Pipeline(void)
{
    SetState(State::kNull);
}

bool Pipeline::Init(const rapidjson::Value& config)
{
    const rapidjson::Value* elements_config = nullptr;
    if (!json::pointer::GetObject(config, "/elements", &elements_config)) {
        TCLOG_ERROR("Pipeline \"%s\": cannot get config \"elements\"", name_.c_str());
        return false;
    }
    if (!CreateElementsByConfig(*elements_config)) {
        return false;
    }
    if (!LinkElementsByConfig(*elements_config)) {
        return false;
    }
    if (!CreateAndLinkBuffers()) {
        return false;
    }
    if (!GetEngineFromConfig(config)) {
        return false;
    }

    int content_int = 0;
    int internal_bus_threads = 0;
    int external_bus_threads = 1;
    if (json::pointer::GetInt(config, "/internal_bus_threads", &content_int)) {
        TCLOG_DEBUG("Pipeline \"%s\": get internal_bus_threads (%d) from config",
                name_.c_str(), content_int);
        internal_bus_threads = content_int;
    }
    if (json::pointer::GetInt(config, "/external_bus_threads", &content_int)) {
        TCLOG_DEBUG("Pipeline \"%s\": get external_bus_threads (%d) from config",
                name_.c_str(), content_int);
        external_bus_threads = content_int;
    }
    if (!CreateMessageComponents(internal_bus_threads, external_bus_threads)) {
        TCLOG_ERROR("Pipeline \"%s\": fail to create message components", name_.c_str());
        return false;
    }
    if (!RegisterPipelineMessageComponents()) {
        return false;
    }
    if (!RegisterElementsMessageComponents()) {
        return false;
    }

    // Debug: print all pads' property
    PrintAllProperties();

    return true;
}

void Pipeline::AddElements(std::initializer_list<std::shared_ptr<Element>> elements)
{
    for (auto ptr = elements.begin(); ptr != elements.end(); ++ptr) {
        elements_.insert({(*ptr)->name(), *ptr});
    }
}

std::shared_ptr<Element> Pipeline::GetElement(const std::string& element_name)
{
    auto itr = elements_.find(element_name);
    if (itr == elements_.end()) {
        return nullptr;
    }
    return itr->second;
}

bool Pipeline::CreateElementsByConfig(const rapidjson::Value& elements_config)
{
    for (rapidjson::Value::ConstMemberIterator itr = elements_config.MemberBegin();
            itr != elements_config.MemberEnd(); ++itr) {
        auto element = ElementFactory::CreateElement(itr->name.GetString(), itr->value);
        if (!element) {
            TCLOG_ERROR("Pipeline \"%s\": fail to create element \"%s\"",
                    name_.c_str(), itr->name.GetString());
            return false;
        }
        auto ret = elements_.insert({element->name(), element});
        if (!ret.second) {
            TCLOG_ERROR("Pipeline \"%s\": fail to insert element \"%s\"",
                    name_.c_str(), element->name().c_str());
            return false;
        }
    }
    return true;
}

bool Pipeline::LinkElementsByConfig(const rapidjson::Value& elements_config)
{
    const rapidjson::Value* config;
    std::string token;

    for (auto elm_itr = elements_.begin(); elm_itr != elements_.end(); ++elm_itr) {
        token = "/" + elm_itr->first;
        if (!json::pointer::GetObject(elements_config, token.c_str(), &config)) {
            TCLOG_ERROR("Pipeline \"%s\": fail to find element \"%s\" in config",
                    name_.c_str(), elm_itr->first.c_str());
            return false;
        }
        if (!ElementLinkPeerPadsByConfig(elm_itr->second, Pad::Direction::kSrc, *config)) {
            TCLOG_ERROR("Pipeline \"%s\": error while linking src pads of element \"%s\"",
                    name_.c_str(), elm_itr->first.c_str());
            return false;
        }
#if 0
        // Not need to link sink pads. Sink pads and src pads are pairs,
        // the above process linking src pads would also link sink pads.

        if (!ElementLinkPeerPadsByConfig(elm_itr->second, Pad::Direction::kSink, *config)) {
            TCLOG_ERROR("Pipeline \"%s\": error while linking sink pads of element \"%s\"",
                    name_.c_str(), elm_itr->first.c_str());
            return false;
        }
#endif
    }
    return true;
}

bool Pipeline::ElementLinkPeerPadsByConfig(std::shared_ptr<Element> element,
        Pad::Direction direction, const rapidjson::Value& config)
{
    const std::vector<std::shared_ptr<Pad>>* pads = element->pads(direction);

    std::string pads_config_name =
        direction == Pad::Direction::kSrc ? "src_pads" : "sink_pads";
    std::string token_prefix, token, peer_element_name, peer_pad_name;
    const char* content = nullptr;

    for (auto pad_itr = pads->begin(); pad_itr != pads->end(); ++pad_itr) {
        if ((*pad_itr)->peer()) {
            continue;
        }

        token_prefix = "/" + pads_config_name + "/" + (*pad_itr)->name();
        token = token_prefix + "/peer_element";
        if (!json::pointer::GetString(config, token.c_str(), &content)) {
            TCLOG_ERROR("Pipeline \"%s\": cannot get config \"%s\" from element \"%s\"",
                    name_.c_str(), token.c_str(), element->name().c_str());
            return false;
        }
        peer_element_name = content;
        token = token_prefix + "/peer_pad";
        if (!json::pointer::GetString(config, token.c_str(), &content)) {
            TCLOG_ERROR("Pipeline \"%s\": cannot get config \"%s\" from element \"%s\"",
                    name_.c_str(), token.c_str(), element->name().c_str());
            return false;
        }
        peer_pad_name = content;

        auto peer_element_itr = elements_.find(peer_element_name);
        if (peer_element_itr == elements_.end()) {
            TCLOG_ERROR("Pipeline \"%s\": fail to find element \"%s\"",
                    name_.c_str(), peer_element_name.c_str());
            return false;
        }

        auto peer_element = peer_element_itr->second;
        Pad::Direction peer_direction =
            direction == Pad::Direction::kSrc ? Pad::Direction::kSink : Pad::Direction::kSrc;
        auto peer_pad = peer_element->FindPad(peer_pad_name, peer_direction);
        if (!peer_pad) {
            TCLOG_ERROR("Pipeline \"%s\": element \"%s\" has no pad named \"%s\"",
                    name_.c_str(), peer_element->name().c_str(), peer_pad_name.c_str());
            return false;
        }

        if (!(*pad_itr)->LinkPeer(peer_pad) || !peer_pad->LinkPeer(*pad_itr)) {
            TCLOG_ERROR("Pipeline \"%s\": fail to link \"%s\"-\"%s\" to "
                    "peer \"%s\"-\"%s\"", name_.c_str(),
                    element->name().c_str(), (*pad_itr)->name().c_str(),
                    peer_element->name().c_str(), peer_pad->name().c_str());
            return false;
        }
    }
    return true;
}

bool Pipeline::CreateAndLinkBuffers(void)
{
    for (auto elm_itr = elements_.begin(); elm_itr != elements_.end(); ++elm_itr) {
        std::shared_ptr<Element> element = elm_itr->second;
        if (!ElementLinkBuffers(element, Pad::Direction::kSrc)) {
            TCLOG_ERROR("Pipeline \"%s\": element \"%s\": error while linking buffers "
                    "for src pads", name_.c_str(), element->name().c_str());
            return false;
        }
        if (!ElementLinkBuffers(elm_itr->second, Pad::Direction::kSink)) {
            TCLOG_ERROR("Pipeline \"%s\": element \"%s\": error while linking buffers "
                    "for sink pads", name_.c_str(), element->name().c_str());
            return false;
        }
    }
    return true;
}

bool Pipeline::ElementLinkBuffers(
        std::shared_ptr<Element> element, Pad::Direction direction)
{
    auto pads = element->pads(direction);
    for (auto pad_itr = pads->begin(); pad_itr != pads->end(); ++pad_itr) {
        std::shared_ptr<Pad> pad = *pad_itr;
        if (pad->buffer()) {
            continue;
        }
        BufferProperty::Mask mask;
        if (!CompletePadPropertyRecursively(pad, pad, direction, &mask)) {
            TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": error while completing property",
                    name_.c_str(), pad->name().c_str());
            return false;
        }
        if (!pad->property() || !pad->property()->IsComplete()) {
            Pad::Direction opposite_dir = direction == Pad::Direction::kSrc ?
                Pad::Direction::kSink : Pad::Direction::kSrc;
            auto opposite_pads = element->pads(opposite_dir);
            if (opposite_pads->size() <= 0) {
                TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": incomplete property "
                        "in the whole pipeline", name_.c_str(), pad->name().c_str());
                if (!pad->property()) {
                    TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": property is empty",
                            name_.c_str(), pad->name().c_str());
                } else {
                    auto prop = pad->property();
                    TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": current property: "
                            "frames: %d, channels: %d, rate: %d, format: %s, storage: %s",
                            name_.c_str(), pad->name().c_str(),
                            prop->frames(), prop->channels(), prop->rate(),
                            BufferProperty::FormatTypeToString(prop->format()).c_str(),
                            BufferProperty::StorageTypeToString(prop->storage()).c_str());
                }
                return false;
            }
            BufferProperty::Mask opposite_mask;
            BufferProperty::OverlapMask(&opposite_mask, element->property_mask());
            std::shared_ptr<Pad> opposite_pad = opposite_pads->front();
            if (!CompletePadPropertyRecursively(pad, opposite_pad,
                        opposite_dir, &opposite_mask)) {
                TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": error while completing property",
                        name_.c_str(), opposite_pad->name().c_str());
                return false;
            }
            if (!pad->property() || !pad->property()->IsComplete()) {
                TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": incomplete property "
                        "in the whole pipeline", name_.c_str(), pad->name().c_str());
                if (!pad->property()) {
                    TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": property is empty",
                            name_.c_str(), pad->name().c_str());
                } else {
                    auto prop = pad->property();
                    TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": current property: "
                            "frames: %d, channels: %d, rate: %d, format: %s, storage: %s",
                            name_.c_str(), pad->name().c_str(),
                            prop->frames(), prop->channels(), prop->rate(),
                            BufferProperty::FormatTypeToString(prop->format()).c_str(),
                            BufferProperty::StorageTypeToString(prop->storage()).c_str());
                }
                return false;
            }
        }
        pad->peer()->property()->Complete(pad->property(), nullptr);
        if (!BufferProperty::IsMatching(pad->property(), pad->peer()->property())) {
            TCLOG_ERROR("Pipeline \"%s\": \"%s\"-\"%s\" and \"%s\"-\"%s\": "
                    "property no matching", name_.c_str(),
                    pad->parent()->name().c_str(), pad->name().c_str(),
                    pad->peer()->parent()->name().c_str(), pad->peer()->name().c_str());
            return false;
        }
        auto buffer = BufferInterface::Create(*pad->property());
        if (!buffer) {
            TCLOG_ERROR("Pipeline \"%s\": error while creating buffer from property of pad \"%s\"",
                    name_.c_str(), pad->name().c_str());
            return false;
        }
        pad->LinkBuffer(buffer);
        pad->peer()->LinkBuffer(buffer);
    }
    return true;
}

// Pseudocode:
//  func: CompletePadPropertyRecursively
//  params: pad, this_pad, mask, dir
//      CompletePadsEachOtherProperty(pad, this_pad->peer, mask)
//      if (complete?)
//          return true
//      peer_element = this_pad->peer->parent
//      further_pads = peer_element->pads(dir)
//      if (further_pads.size <= 0)
//          return false
//      overlap_mask(mask, peer_element->mask)
//      return CompletePadPropertyRecursively(pad, further_pads.front, mask)
bool Pipeline::CompletePadPropertyRecursively(
        std::shared_ptr<Pad> base_pad, std::shared_ptr<Pad> start_pad,
        Pad::Direction direction, BufferProperty::Mask* mask)
{
    std::shared_ptr<Pad> peer_pad = start_pad->peer();
    if (!peer_pad) {
        TCLOG_ERROR("Pipeline \"%s\": pad \"%s\": no peer pad",
                name_.c_str(), start_pad->name().c_str());
        return false;
    }
    CompletePadsEachOtherProperty(base_pad, peer_pad, mask);
    if (base_pad->property() && base_pad->property()->IsComplete()) {
        return true;
    }
    std::shared_ptr<Element> peer_element = peer_pad->parent();
    if (!peer_element) {
        TCLOG_ERROR("Pipeline \"%s\": peer pad \"%s\": no parent",
                name_.c_str(), peer_pad->name().c_str());
        return false;
    }
    auto further_pads = peer_element->pads(direction);
    if (further_pads->size() <= 0) {
        return true;    // true or false?
    }
    BufferProperty::OverlapMask(mask, peer_element->property_mask());
    return CompletePadPropertyRecursively(
            base_pad, further_pads->front(), direction, mask);
}

void Pipeline::CompletePadsEachOtherProperty(
        std::shared_ptr<Pad> p1, std::shared_ptr<Pad> p2, BufferProperty::Mask* mask)
{
    BufferProperty* p1_prop = p1->property();
    BufferProperty* p2_prop = p2->property();
    if (!p1_prop && !p2_prop) {
        return;
    }
    if (!p1_prop) {
        BufferProperty prop;
        prop.Complete(p2_prop, mask);
        p1->set_property(prop);
    } else if (!p2_prop) {
        BufferProperty prop;
        prop.Complete(p1_prop, mask);
        p2->set_property(prop);
    } else {
        p1_prop->Complete(p2_prop, mask);
        p2_prop->Complete(p1_prop, mask);
    }
}

bool Pipeline::GetEngineFromConfig(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    if (!json::pointer::GetString(config, "/engine", &content_string)) {
        TCLOG_ERROR("Pipeline \"%s\": cannot get config \"engine\"", name_.c_str());
        return false;
    }
    std::string engine_name = content_string;
    engine_ = GetElement(engine_name);
    if (!engine_) {
        TCLOG_ERROR("Pipeline \"%s\": fail to get engine from config: "
                "no element named \"%s\"", name_.c_str(), engine_name.c_str());
        return false;
    }
    return true;
}

bool Pipeline::CreateMessageComponents(int internal_bus_threads, int external_bus_threads)
{
    internal_bus_ = Bus::Create(internal_bus_threads);
    if (!internal_bus_) {
        TCLOG_ERROR("Pipeline \"%s\": fail to create internal bus", name_.c_str());
        return false;
    }
    external_bus_ = Bus::Create(external_bus_threads);
    if (!external_bus_) {
        TCLOG_ERROR("Pipeline \"%s\": fail to create external bus", name_.c_str());
        return false;
    }
    internal_message_sender_ = MessageSender::Create();
    if (!internal_message_sender_) {
        TCLOG_ERROR("Pipeline \"%s\": fail to create internal message sender", name_.c_str());
        return false;
    }
    external_message_sender_ = MessageSender::Create();
    if (!external_message_sender_) {
        TCLOG_ERROR("Pipeline \"%s\": fail to create external message sender", name_.c_str());
        return false;
    }
    return true;
}

bool Pipeline::RegisterPipelineMessageComponents(void)
{
    if (!internal_bus_->RegisterSender(internal_message_sender_)) {
        TCLOG_ERROR("Pipeline \"%s\": fail to register message sender to"
                "internal bus", name_.c_str());
        return false;
    }
    if (!external_bus_->RegisterSender(external_message_sender_)) {
        TCLOG_ERROR("Pipeline \"%s\": fail to register message sender to"
                "external bus", name_.c_str());
        return false;
    }
    return true;
}

bool Pipeline::RegisterElementsMessageComponents(void)
{
    bool ret;
    for (auto elm_itr = elements_.begin(); elm_itr != elements_.end(); ++elm_itr) {
        std::shared_ptr<MessageSender> sender = elm_itr->second->internal_message_sender();
        if (sender) {
            ret = internal_bus_->RegisterSender(sender);
            if (!ret) {
                TCLOG_ERROR("Pipeline \"%s\": fail to register message sender of "
                        "element \"%s\" to internal bus",
                        name_.c_str(), elm_itr->second->name().c_str());
                return false;
            }
        }

        sender = elm_itr->second->external_message_sender();
        if (sender) {
            ret = external_bus_->RegisterSender(sender);
            if (!ret) {
                TCLOG_ERROR("Pipeline \"%s\": fail to register message sender of "
                        "element \"%s\" to external bus",
                        name_.c_str(), elm_itr->second->name().c_str());
                return false;
            }
        }

        auto callbacks = elm_itr->second->internal_message_callbacks();
        for (auto cb_itr = callbacks->begin(); cb_itr != callbacks->end(); ++cb_itr) {
            ret = internal_bus_->RegisterCallback(cb_itr->first, cb_itr->second);
            if (!ret) {
                TCLOG_ERROR("Pipeline \"%s\": fail to register callback of message "
                        "\"%s\" in element \"%s\" to internal bus", name_.c_str(),
                        cb_itr->first.c_str(), elm_itr->second->name().c_str());
                return false;
            }
        }

        callbacks = elm_itr->second->external_message_callbacks();
        for (auto cb_itr = callbacks->begin(); cb_itr != callbacks->end(); ++cb_itr) {
            ret = external_bus_->RegisterCallback(cb_itr->first, cb_itr->second);
            if (!ret) {
                TCLOG_ERROR("Pipeline \"%s\": fail to register callback of message "
                        "\"%s\" in element \"%s\" to external bus", name_.c_str(),
                        cb_itr->first.c_str(), elm_itr->second->name().c_str());
                return false;
            }
        }
    }
    return true;
}

bool Pipeline::LinkPads(std::shared_ptr<Pad> pad1, std::shared_ptr<Pad> pad2,
        std::shared_ptr<BufferInterface> buffer)
{
    if (!pad1 || !pad2) {
        return false;
    }
    if (!pad1->LinkPeer(pad2) || !pad2->LinkPeer(pad1)) {
        return false;
    }
    pad1->LinkBuffer(buffer);
    pad2->LinkBuffer(buffer);
    return true;
}

bool Pipeline::LinkPads(
        std::shared_ptr<Element> src_element, const std::string src_pad_name,
        std::shared_ptr<Element> sink_element, const std::string sink_pad_name,
        std::shared_ptr<BufferInterface> buffer)
{
    std::shared_ptr<Pad> src_pad = src_element->FindPad(src_pad_name, Pad::Direction::kSrc);
    std::shared_ptr<Pad> sink_pad = sink_element->FindPad(sink_pad_name, Pad::Direction::kSink);
    return LinkPads(src_pad, sink_pad, buffer);
}

// Pseudocode:
//  func: ActivateElementsRecursively
//  params: element_begin, mode
//      element_begin->Activate()
//      if (mode == kPushPull || mode == kPush)
//          for (all src pads)
//              ActivateElementsRecursively(pad->peer->parent, kPush)
//      if (mode == kPushPull || mode == kPull)
//          for (all sink pads)
//              ActivateElementsRecursively(pad->peer->parent, kPull)
//      return true
bool Pipeline::ActivateElementsRecursively(std::shared_ptr<Element> element,
        Element::Mode mode)
{
    if (element->mode() == Element::Mode::kUnknown) {
        if (!element->Activate(mode)) {
            TCLOG_ERROR("Pipeline \"%s\": element \"%s\": error while activated as mode \"%s\"",
                    name_.c_str(), element->name().c_str(),
                    Element::ModeToString(mode).c_str());
            return false;
        }
    }
    if (mode == Element::Mode::kPush || mode == Element::Mode::kPullPush) {
        auto pads = element->pads(Pad::Direction::kSrc);
        for (auto itr = pads->begin(); itr != pads->end(); ++itr) {
            if (!(*itr)->peer()) {
                TCLOG_ERROR("Pipeline \"%s\": element \"%s\": pad \"%s\": peer not exists",
                        name_.c_str(), element->name().c_str(), (*itr)->name().c_str());
                return false;
            }
            if (!ActivateElementsRecursively(
                        (*itr)->peer()->parent(), Element::Mode::kPush)) {
                return false;
            }
        }
    }
    if (mode == Element::Mode::kPull || mode == Element::Mode::kPullPush) {
        auto pads = element->pads(Pad::Direction::kSink);
        for (auto itr = pads->begin(); itr != pads->end(); ++itr) {
            if (!(*itr)->peer()) {
                TCLOG_ERROR("Pipeline \"%s\": element \"%s\": pad \"%s\": peer not exists",
                        name_.c_str(), element->name().c_str(), (*itr)->name().c_str());
                return false;
            }
            if (!ActivateElementsRecursively(
                        (*itr)->peer()->parent(), Element::Mode::kPull)) {
                return false;
            }
        }
    }
    return true;
}

bool Pipeline::Activate(void)
{
    if (!engine_) {
        TCLOG_ERROR("Pipeline \"%s\": engine not determined", name_.c_str());
        return false;
    }
    Element::Mode engine_mode = engine_->EngineMode();
    if (engine_mode == Element::Mode::kUnknown) {
        TCLOG_ERROR("Pipeline \"%s\": fail to get engine mode of element \"%s\", "
                "it cannot be the engine", name_.c_str(), engine_->name().c_str());
        return false;
    }
    if (!ActivateElementsRecursively(engine_, engine_mode)) {
        TCLOG_ERROR("Pipeline \"%s\": fail to activate elements", name_.c_str());
        return false;
    }
    return true;
}

bool Pipeline::Deactivate(void)
{
    for (auto itr = elements_.begin(); itr != elements_.end(); ++itr) {
        if (!itr->second->Deactivate()) {
            TCLOG_ERROR("Pipeline \"%s\": fail to deactivate element \"%s\"",
                    name_.c_str(), itr->second->name().c_str());
            return false;
        }
    }
    return true;
}

void Pipeline::MainLoop(void)
{
    // elements state from Null to Ready
    ElementsStateChange(State::kNull, State::kReady);
    State last_state = State::kReady;

    while (true) {
        std::unique_lock<std::mutex> state_lock(state_mutex_);
        if (last_state != state_) {
            ElementsStateChange(last_state, state_);
            last_state = state_;
        }
        state_cond_var_.wait(state_lock,
                [&] { return state_ != State::kReady && state_ != State::kPaused; });
        if (last_state != state_) {
            ElementsStateChange(last_state, state_);
            last_state = state_;
        }
        if (state_ == State::kNull) {
            break;
        }
        state_lock.unlock();
        int ret = engine_->Loop();
        // TODO: the state_ can be modified in the engine_->Loop()?
        if (ret < 0) {
            state_lock.lock();
            if (state_ == State::kNull) {
                ElementsStateChange(last_state, State::kNull);
                break;
            } else {
                ElementsStateChange(last_state, State::kReady);
                state_ = State::kReady;
                last_state = state_;
                continue;
            }
        }
    }
}

void Pipeline::PrintAllProperties(void)
{
    TCLOG_INFO("Propeties of pipeline \"%s\":", name_.c_str());
    for (auto elm_itr = elements_.begin(); elm_itr != elements_.end(); ++elm_itr) {
        auto element = elm_itr->second;
        auto src_pads = element->pads(Pad::Direction::kSrc);
        auto sink_pads = element->pads(Pad::Direction::kSink);

        for (auto pad_itr = src_pads->begin(); pad_itr != src_pads->end(); ++pad_itr) {
            auto property = (*pad_itr)->property();
            TCLOG_INFO("\t[%s]-[%s] (src_pad):",
                    element->name().c_str(), (*pad_itr)->name().c_str());
            TCLOG_INFO("\t\tframes: %d", property->frames());
            TCLOG_INFO("\t\tchannels: %d", property->channels());
            TCLOG_INFO("\t\trate: %d", property->rate());
            TCLOG_INFO("\t\tformat: %s",
                    BufferProperty::FormatTypeToString(property->format()).c_str());
            TCLOG_INFO("\t\tstorage: %s",
                    BufferProperty::StorageTypeToString(property->storage()).c_str());
            TCLOG_INFO("\t\tpeer: [%s]-[%s]",
                    (*pad_itr)->peer()->parent()->name().c_str(),
                    (*pad_itr)->peer()->name().c_str());
        }
        for (auto pad_itr = sink_pads->begin(); pad_itr != sink_pads->end(); ++pad_itr) {
            auto property = (*pad_itr)->property();
            TCLOG_INFO("\t[%s]-[%s] (sink_pad):",
                    element->name().c_str(), (*pad_itr)->name().c_str());
            TCLOG_INFO("\t\tframes: %d", property->frames());
            TCLOG_INFO("\t\tchannels: %d", property->channels());
            TCLOG_INFO("\t\trate: %d", property->rate());
            TCLOG_INFO("\t\tformat: %s",
                    BufferProperty::FormatTypeToString(property->format()).c_str());
            TCLOG_INFO("\t\tstorage: %s",
                    BufferProperty::StorageTypeToString(property->storage()).c_str());
            TCLOG_INFO("\t\tpeer: [%s]-[%s]",
                    (*pad_itr)->peer()->parent()->name().c_str(),
                    (*pad_itr)->peer()->name().c_str());
        }
    }
}

void Pipeline::ElementsStateChange(State current, State target)
{
    if (current == target) {
        return;
    }
    TCLOG_DEBUG("Pipeline \"%s\": ElementsStateChange: current: \"%s\", target: \"%s\"",
            name_.c_str(), StateToString(current).c_str(), StateToString(target).c_str());

    int current_index = static_cast<int>(current);
    int target_index = static_cast<int>(target);
    int diff_index = target_index - current_index;
    StateChangeDirection dir = diff_index > 0 ?
        StateChangeDirection::kUp : StateChangeDirection::kDown;
    int dir_index = static_cast<int>(dir);
    for (int i = 0; i < abs(diff_index); ++i) {
        internal_message_sender_->Emit({Message::Priority::kSync,
                kStateChangeMessageStrings[dir_index][current_index]});
        current_index += (dir == StateChangeDirection::kUp) ? 1 : -1;
    }
}

bool Pipeline::SetState(State state)
{
    std::lock_guard<std::mutex> change_state_lock(state_set_mutex_);

    std::unique_lock<std::mutex> state_lock(state_mutex_);
    TCLOG_DEBUG("Pipeline \"%s\": current state: \"%s\", target state: \"%s\"",
            name_.c_str(), StateToString(state_).c_str(), StateToString(state).c_str());

    if (state_ == state) {
        return true;
    }

    if (state_ == State::kNull) {
        if (!Activate()) {
            TCLOG_ERROR("Pipeline \"%s\": fail to activate", name_.c_str());
            return false;
        }
        if (main_loop_thread_) {
            TCLOG_ERROR("Pipeline \"%s\": main loop has been created", name_.c_str());
            return false;
        }
        state_ = State::kReady;
        main_loop_thread_ = std::shared_ptr<std::thread>(
                new std::thread(std::bind(&Pipeline::MainLoop, this)));
    }

    state_ = state;

    if (state_ == State::kNull) {
        if (!main_loop_thread_) {
            TCLOG_ERROR("Pipeline \"%s\": main loop has not been created", name_.c_str());
            return false;
        }
        state_ = State::kNull;
        state_cond_var_.notify_one();
        state_lock.unlock();
        if (main_loop_thread_->joinable()) {
            main_loop_thread_->join();
        }
        main_loop_thread_ = nullptr;
        if (!Deactivate()) {
            TCLOG_ERROR("Pipeline \"%s\": fail to deactive", name_.c_str());
            return false;
        }
    } else {
        state_cond_var_.notify_one();
        state_lock.unlock();
    }

    return true;
}

bool Pipeline::AddExternalMessageCallback(const std::string& message,
        std::function<void (void)> callback)
{
    return external_bus_->RegisterCallback(message, callback);
}

} // namespace audio
} // namespace tconfigs
