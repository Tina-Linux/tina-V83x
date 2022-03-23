#include "tconfigs/audio/common/pad.h"
#include "tconfigs/audio/common/element.h"

#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

Pad::Pad(const std::string& name, int index, Direction direction,
        std::shared_ptr<Element> parent)
    : name_(name),
      index_(index),
      direction_(direction),
      parent_(parent)
{
}

bool Pad::LinkPeer(std::shared_ptr<Pad> peer)
{
    switch (direction_) {
    case Direction::kSrc:
        if (peer->direction() != Direction::kSink) {
            return false;
        }
        break;
    case Direction::kSink:
        if (peer->direction() != Direction::kSrc) {
            return false;
        }
        break;
    default:
        return false;
    }
    peer_ = peer;
    return true;
}

void Pad::LinkBuffer(std::shared_ptr<BufferInterface> buffer)
{
    buffer_ = buffer;
}

void Pad::UnlinkPeer(void)
{
    peer_.reset();
    buffer_ = nullptr;
}

int Pad::Push(void)
{
    auto peer = peer_.lock();
    if (!peer) {
        return -EPERM;
    }
    auto parent = peer->parent();
    if (!parent) {
        return -EPERM;
    }
    return parent->PushChain(peer);
}

int Pad::Pull(void)
{
    auto peer = peer_.lock();
    if (!peer) {
        return -EPERM;
    }
    auto parent = peer->parent();
    if (!parent) {
        return -EPERM;
    }
    return parent->PullChain(peer);
}

} // namespace audio
} // namespace tconfigs
