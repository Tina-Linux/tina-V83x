#include "tconfigs/audio/elements/data_queue.h"

#include <cstring>
#include "tconfigs/audio/common/element_factory.h"
#include "tconfigs/log/logging.h"

namespace tconfigs {
namespace audio {

REGISTER_ELEMENT(DataQueue)

// Queue =======================================================================
std::shared_ptr<DataQueue::Queue> DataQueue::Queue::Create(
        int max_units, int unit_bytes)
{
    auto q = std::shared_ptr<Queue>(new Queue(max_units, unit_bytes));
    if (!q || !q->Init()) {
        return nullptr;
    }
    return q;
}

DataQueue::Queue::Queue(int max_units, int unit_bytes)
    : max_units_(max_units),
      unit_bytes_(unit_bytes)
{
}

DataQueue::Queue::~Queue(void)
{
    if (data_buf_) {
        delete [] data_buf_;
        data_buf_ = nullptr;
    }
}

bool DataQueue::Queue::Init(void)
{
    data_buf_ = new uint8_t[unit_bytes_ * max_units_];
    if (!data_buf_) {
        return false;
    }
    return true;
}

bool DataQueue::Queue::Enqueue(const uint8_t* input_data)
{
    if (current_units_ >= max_units_) {
        TCLOG_ERROR("Fail to enqueue, the queue is full");
        return false;
    }

    rear_pos_ %= unit_bytes_ * max_units_;
    std::memcpy(data_buf_ + rear_pos_, input_data, unit_bytes_);
    rear_pos_ += unit_bytes_;
    ++current_units_;
    return true;
}

bool DataQueue::Queue::Dequeue(uint8_t* output_data)
{
    if (current_units_ <= 0) {
        TCLOG_ERROR("Fail to dequeue, the queue is empty");
        return false;
    }

    front_pos_ %= unit_bytes_ * max_units_;
    std::memcpy(output_data, data_buf_ + front_pos_, unit_bytes_);
    front_pos_ += unit_bytes_;
    --current_units_;
    return true;
}

bool DataQueue::Queue::IsEmpty(void)
{
    return current_units_ <= 0;
}

bool DataQueue::Queue::IsFull(void)
{
    return current_units_ >= max_units_;
}
// =============================================================================

std::shared_ptr<DataQueue> DataQueue::Create(const std::string& name,
        const rapidjson::Value& config)
{
    auto data_queue = std::shared_ptr<DataQueue>(new DataQueue(name));
    if (!data_queue || !data_queue->Init(config)) {
        return nullptr;
    }
    return data_queue;
}

DataQueue::DataQueue(const std::string& name)
    : Element(name)
{
}

bool DataQueue::Init(const rapidjson::Value& config)
{
    const char* content_string = nullptr;
    int content_int;

    if (!json::pointer::GetString(config, "/element_type", &content_string)
            || std::string(content_string) != "DataQueue") {
        TCLOG_ERROR("\"%s\" is not a DataQueue", name().c_str());
        return false;
    }

    if (!json::pointer::GetInt(config, "/max_buffers", &content_int)) {
        TCLOG_ERROR("DataQueue \"%s\": cannot get config \"max_buffers\"",
                name().c_str());
        return false;
    }
    if (content_int <= 0) {
        TCLOG_ERROR("DataQueue \"%s\": config \"max_buffers\" shoule be greater than 0",
                name().c_str());
        return false;
    }
    max_buffers_ = content_int;

    const rapidjson::Value* sink_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/sink_pads", &sink_pads_config)) {
        TCLOG_ERROR("DataQueue \"%s\": cannot get config \"sink_pads\"", name().c_str());
        return false;
    }
    // DataQueue has only one sink pad
    if (sink_pads_config->Size() != 1) {
        TCLOG_ERROR("DataQueue \"%s\": the number of sink pads in config is not equal to 1",
                name().c_str());
        return false;
    }
    std::string sink_pad_name = sink_pads_config->MemberBegin()->name.GetString();
    if (!AddPad(sink_pad_name, Pad::Direction::kSink, shared_from_this())) {
        TCLOG_ERROR("DataQueue \"%s\": fail to add sink pad named \"%s\"",
                name().c_str(), sink_pad_name.c_str());
        return false;
    }
    sink_pad_ = FindPad(sink_pad_name, Pad::Direction::kSink);

    const rapidjson::Value* src_pads_config = nullptr;
    if (!json::pointer::GetObject(config, "/src_pads", &src_pads_config)) {
        TCLOG_ERROR("DataQueue \"%s\": cannot get config \"src_pads\"", name().c_str());
        return false;
    }
    // DataQueue has only one src pad
    if (src_pads_config->Size() != 1) {
        TCLOG_ERROR("DataQueue \"%s\": the number of src pads in config is not equal to 1",
                name().c_str());
        return false;
    }
    std::string src_pad_name = src_pads_config->MemberBegin()->name.GetString();
    if (!AddPad(src_pad_name, Pad::Direction::kSrc, shared_from_this())) {
        TCLOG_ERROR("DataQueue \"%s\": fail to add src pad named \"%s\"",
                name().c_str(), src_pad_name.c_str());
        return false;
    }
    src_pad_ = FindPad(src_pad_name, Pad::Direction::kSrc);

    AddAvailableMode({Mode::kPush});

    return true;
}

bool DataQueue::Activate(Mode mode)
{
    if (!ActivateDefault(mode)) {
        TCLOG_ERROR("DataQueue \"%s\": fail to activate (not supported mode: %s)",
                name().c_str(), ModeToString(mode).c_str());
        return false;
    }

    if (queue_thread_.joinable()) {
        TCLOG_ERROR("DataQueue \"%s\": the queue thread has been created",
                name().c_str());
        return false;
    }

    if (!sink_pad_ || !src_pad_) {
        TCLOG_ERROR("DataQueue \"%s\": invalid sink/src pad", name().c_str());
        return false;
    }
    if (!BufferProperty::IsMatching(
                sink_pad_->buffer()->property(), src_pad_->buffer()->property())) {
        TCLOG_ERROR("DataQueue \"%s\": buffer properties of sink pad and src pad "
                "are not matching", name().c_str());
        return false;
    }
    auto property = sink_pad_->buffer()->property();
    int effective_bits, total_bits;
    if (!BufferProperty::FormatTypeToBits(
                property->format(), &effective_bits, &total_bits)) {
        TCLOG_ERROR("DataQueue \"%s\": not supported format: %s", name().c_str(),
                BufferProperty::FormatTypeToString(property->format()).c_str());
        return false;
    }
    int unit_bytes = total_bits / 8 * property->channels() * property->frames();
    internal_queue_ = Queue::Create(max_buffers_, unit_bytes);
    if (!internal_queue_) {
        TCLOG_ERROR("DataQueue \"%s\": fail to create internal queue", name().c_str());
        return false;
    }

    is_stopped_ = false;
    queue_thread_ = std::thread(std::bind(&DataQueue::DequeueLoop, this));

    return true;
}

bool DataQueue::Deactivate(void)
{
    std::unique_lock<std::mutex> queue_lock(queue_mutex_);
    is_stopped_ = true;
    queue_not_empty_.notify_all();
    queue_not_full_.notify_all();
    queue_lock.unlock();

    if (queue_thread_.joinable()) {
        queue_thread_.join();
    }
    internal_queue_ = nullptr;

    return DeactivateDefault();
}

void DataQueue::DequeueLoop(void)
{
    while (!is_stopped_) {
        std::unique_lock<std::mutex> queue_lock(queue_mutex_);
        queue_not_empty_.wait(queue_lock,
                [&] { return is_stopped_ || !internal_queue_->IsEmpty(); });
        if (is_stopped_) {
            break;
        }
        if (!internal_queue_->Dequeue(
                    static_cast<uint8_t*>(src_pad_->buffer()->data()))) {
            TCLOG_ERROR("DataQueue \"%s\": dequeue error", name().c_str());
        }
        queue_not_full_.notify_one();
        queue_lock.unlock();
        int ret = src_pad_->Push();
        if (ret < 0) {
            is_stopped_ = true;
            break;
        }
    }
}

int DataQueue::PushChain(std::shared_ptr<Pad> pad)
{
    std::unique_lock<std::mutex> queue_lock(queue_mutex_);
    queue_not_full_.wait(queue_lock,
            [&] { return is_stopped_ || !internal_queue_->IsFull(); });
    if (is_stopped_) {
        return 0;
    }

    if (!internal_queue_->Enqueue(static_cast<uint8_t*>(sink_pad_->buffer()->data()))) {
        TCLOG_ERROR("DataQueue \"%s\": enqueue error, discard data", name().c_str());
    }
    queue_not_empty_.notify_one();
    return 0;
}

} // namespace audio
} // namespace tconfigs
