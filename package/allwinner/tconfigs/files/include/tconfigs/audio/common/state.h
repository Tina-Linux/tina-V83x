#ifndef __TCONFIGS_AUDIO_COMMON_STATE_H__
#define __TCONFIGS_AUDIO_COMMON_STATE_H__

#include <string>
#include <unordered_map>

namespace tconfigs {
namespace audio {

// All states are in order, don't change their numbers
enum class State {
    kNull = 0,
    kReady = 1,
    kPaused = 2,
    kPlaying = 3,
    kNumOfStates
};

enum class StateChangeDirection {
    kUp = 0,
    kDown = 1,
    kNumOfStateChangeDirections
};

extern const std::unordered_map<State, std::string> kStateToStringMap;

extern const std::string kStateChangeMessageStrings
        [static_cast<int>(StateChangeDirection::kNumOfStateChangeDirections)]
        [static_cast<int>(State::kNumOfStates)];

std::string StateToString(State state);

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_STATE_H__ */
