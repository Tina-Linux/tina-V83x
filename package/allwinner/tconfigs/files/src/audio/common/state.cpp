#include "tconfigs/audio/common/state.h"

namespace tconfigs {
namespace audio {

const std::unordered_map<State, std::string> kStateToStringMap = {
    {State::kNull, "Null"},
    {State::kReady, "Ready"},
    {State::kPaused, "Paused"},
    {State::kPlaying, "Playing"}
};

const std::string kStateChangeMessageStrings
        [static_cast<int>(StateChangeDirection::kNumOfStateChangeDirections)]
        [static_cast<int>(State::kNumOfStates)] = {
    {"[S]NullToReady", "[S]ReadyToPaused", "[S]PausedToPlaying", "[S]Unknown"},
    {"[S]Unknown", "[S]ReadyToNull", "[S]PausedToReady", "[S]PlayingToPaused"}
};

std::string StateToString(State state)
{
    auto itr = kStateToStringMap.find(state);
    if (itr == kStateToStringMap.end()) {
        return "Unknown";
    }
    return itr->second;
}

} // namespace audio
} // namespace tconfigs
