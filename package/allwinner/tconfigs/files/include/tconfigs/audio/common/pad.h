#ifndef __TCONFIGS_AUDIO_COMMON_PAD_H__
#define __TCONFIGS_AUDIO_COMMON_PAD_H__

#include <string>
#include <memory>
#include <vector>
#include "tconfigs/audio/common/buffer.h"

namespace tconfigs {
namespace audio {

class Element;

class Pad {
public:
    enum class Direction {
        kUnknown,
        kSrc,
        kSink,
    };

    enum class Mode {
        kUnknown,
        kPush,
        kPull
    };

    Pad(void) = delete;
    Pad(const std::string& name, int index, Direction direction,
            std::shared_ptr<Element> parent);
    ~Pad(void) = default;

    bool LinkPeer(std::shared_ptr<Pad> peer);
    void LinkBuffer(std::shared_ptr<BufferInterface> buffer);

    void UnlinkPeer(void);

    int Push(void);
    int Pull(void);

    void set_mode(Mode mode) { mode_ = mode; }
    void set_property(const BufferProperty& property) {
        if (property_.size() == 0)
            property_.push_back(property);
        else
            property_.front() = property;
    }

    const std::string& name(void) const { return name_; }
    int index(void) const { return index_; }
    Direction direction(void) const { return direction_; }
    Mode mode(void) const { return mode_; }
    BufferProperty* property(void) {
            return property_.size() == 0 ? nullptr : &property_.front(); }
    std::shared_ptr<BufferInterface> buffer(void) { return buffer_; }
    std::shared_ptr<Element> parent(void) { return parent_.lock(); }
    std::shared_ptr<Pad> peer(void) { return peer_.lock(); }

private:
    std::string name_;  // Used for identification
    int index_ = 0;     // Used to be accessed (More efficient than accessed by name?)
    Direction direction_ = Direction::kUnknown;
    Mode mode_ = Mode::kUnknown;
    std::vector<BufferProperty> property_;
    std::shared_ptr<BufferInterface> buffer_ = nullptr;
    // Use weak_ptr to avoid cyclic references
    std::weak_ptr<Element> parent_;
    std::weak_ptr<Pad> peer_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_PAD_H__ */
