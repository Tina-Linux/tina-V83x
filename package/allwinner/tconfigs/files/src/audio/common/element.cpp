#include "tconfigs/audio/common/element.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

const std::unordered_map<Element::Mode, std::string> Element::kModeToStringMap = {
    {Mode::kPush, "Push"},
    {Mode::kPull, "Pull"},
    {Mode::kPullPush, "Pull-Push"},
};

std::string Element::ModeToString(Mode mode)
{
    auto itr = kModeToStringMap.find(mode);
    if (itr == kModeToStringMap.end()) {
        return "Unknown";
    }
    return itr->second;
}

Element::Element(const std::string& name)
    : name_(name)
{
}

bool Element::AddPad(const std::string& name, Pad::Direction direction,
        std::shared_ptr<Element> parent)
{
    if (direction == Pad::Direction::kUnknown) {
        return false;
    }
    std::vector<std::shared_ptr<Pad>>* pads =
            direction == Pad::Direction::kSrc ?  &src_pads_ : &sink_pads_;
    int index = pads->size();
    auto pad = std::shared_ptr<Pad>(new Pad(name, index, direction, parent));
    if (!pad) {
        return false;
    }
    pads->push_back(pad);
    return true;
}

std::shared_ptr<Pad> Element::FindPad(const std::string& name, Pad::Direction direction)
{
    if (direction == Pad::Direction::kUnknown) {
        return nullptr;
    }
    std::vector<std::shared_ptr<Pad>>* pads =
            direction == Pad::Direction::kSrc ?  &src_pads_ : &sink_pads_;
    for (auto itr = pads->begin(); itr != pads->end(); ++itr) {
        if ((*itr)->name() == name) {
            return *itr;
        }
    }
    return nullptr;
}

#define UPDATE_PROPERTY_MASK(member) \
do { \
    for (auto src_itr = src_pads_.begin(); src_itr != src_pads_.end(); ++src_itr) { \
        if (!(*src_itr)->property()) \
            continue; \
        for (auto sink_itr = sink_pads_.begin(); sink_itr != sink_pads_.end(); ++sink_itr) { \
            if (!(*sink_itr)->property()) \
                continue; \
            if ((*src_itr)->property()->member() != (*sink_itr)->property()->member()) { \
                property_mask_.member = false; \
                break; \
            } \
        } \
        if (!property_mask_.member) \
            break; \
    } \
} while (0)

void Element::UpdatePropertyMask(void)
{
    UPDATE_PROPERTY_MASK(frames);
    UPDATE_PROPERTY_MASK(channels);
    UPDATE_PROPERTY_MASK(rate);
    UPDATE_PROPERTY_MASK(format);
    UPDATE_PROPERTY_MASK(storage);
}

void Element::AddAvailableMode(std::initializer_list<Mode> mode)
{
    available_mode_.insert(mode);
}

bool Element::ModeIsAvailable(Mode mode)
{
    return available_mode_.find(mode) == available_mode_.end() ? false : true;
}

bool Element::ActivateDefault(Mode mode)
{
    return SetMode(mode);
}

bool Element::DeactivateDefault(void)
{
    mode_ = Mode::kUnknown;
    return true;
}

bool Element::CreateInternalMessageSender(void)
{
    if (internal_message_sender_) {
        return true;
    }
    internal_message_sender_ = MessageSender::Create();
    if (!internal_message_sender_) {
        TCLOG_ERROR("Element \"%s\": fail to create internal_message_sender", name_.c_str());
        return false;
    }
    return true;
}

bool Element::CreateExternalMessageSender(void)
{
    if (external_message_sender_) {
        return true;
    }
    external_message_sender_ = MessageSender::Create();
    if (!external_message_sender_) {
        TCLOG_ERROR("Element \"%s\": fail to create external_message_sender", name_.c_str());
        return false;
    }
    return true;
}

void Element::SendInternalMessage(const Message& message)
{
    if (!internal_message_sender_) {
        TCLOG_ERROR("Element \"%s\": internal_message_sender is nullptr", name_.c_str());
        return;
    }
    internal_message_sender_->Emit(message);
}

void Element::SendExternalMessage(const Message& message)
{
    if (!external_message_sender_) {
        TCLOG_ERROR("Element \"%s\": external_message_sender is nullptr", name_.c_str());
        return;
    }
    external_message_sender_->Emit(message);
}

bool Element::AddInternalMessageCallback(const std::string& message,
        std::function<void (void)> callback)
{
    auto ret = internal_message_callbacks_.insert({message, callback});
    if (!ret.second) {
        TCLOG_ERROR("Message \"%s\" has already existed", message.c_str());
        return false;
    }
    return true;
}

bool Element::AddExternalMessageCallback(const std::string& message,
        std::function<void (void)> callback)
{
    auto ret = external_message_callbacks_.insert({message, callback});
    if (!ret.second) {
        TCLOG_ERROR("Message \"%s\" has already existed", message.c_str());
        return false;
    }
    return true;
}

bool Element::AddStateChangeMessageCallback(StateChangeDirection dir,
        State current_state, std::function<void (void)> callback)
{
    int dir_index = static_cast<int>(dir);
    int current_state_index = static_cast<int>(current_state);
    return AddInternalMessageCallback(
            kStateChangeMessageStrings[dir_index][current_state_index],
            callback);
}

void Element::SetPadsMode(std::vector<std::shared_ptr<Pad>>* pads, Pad::Mode pad_mode)
{
    for (auto itr = pads->begin(); itr != pads->end(); ++itr) {
        (*itr)->set_mode(pad_mode);
    }
}

bool Element::SetMode(Mode mode)
{
    if (!ModeIsAvailable(mode)) {
        return false;
    }
    Pad::Mode src_pads_mode, sink_pads_mode;
    switch (mode) {
    case Mode::kPush:
        src_pads_mode = Pad::Mode::kPush;
        sink_pads_mode = Pad::Mode::kPush;
        break;
    case Mode::kPull:
        src_pads_mode = Pad::Mode::kPull;
        sink_pads_mode = Pad::Mode::kPull;
        break;
    case Mode::kPullPush:
        src_pads_mode = Pad::Mode::kPull;
        sink_pads_mode = Pad::Mode::kPush;
        break;
    default:
        return false;
    }
    SetPadsMode(&src_pads_, src_pads_mode);
    SetPadsMode(&sink_pads_, sink_pads_mode);
    mode_ = mode;
    return true;
}

Element::Mode Element::EngineMode(void)
{
    Mode mode = Mode::kUnknown;
    auto num_src_pads = src_pads_.size();
    auto num_sink_pads = sink_pads_.size();

    if (num_src_pads <= 0 && num_sink_pads <= 0) {
        TCLOG_ERROR("Element \"%s\" has neither src pads nor sink pads", name_.c_str());
        return Mode::kUnknown;
    } else if (num_src_pads > 0 && num_sink_pads <= 0) {
        mode = Mode::kPush;
    } else if (num_src_pads <= 0 && num_sink_pads > 0) {
        mode = Mode::kPull;
    } else {
        mode = Mode::kPullPush;
    }
    if (!ModeIsAvailable(mode)) {
        TCLOG_ERROR("Element \"%s\": mode \"%s\" is unavailable", name_.c_str(),
                ModeToString(mode).c_str());
        return Mode::kUnknown;
    }
    return mode;
}

} // namespace audio
} // namespace tconfigs
