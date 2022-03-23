#ifndef __TCONFIGS_AUDIO_ELEMENTS_DATA_QUEUE_H__
#define __TCONFIGS_AUDIO_ELEMENTS_DATA_QUEUE_H__

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "tconfigs/json/json_utils.h"
#include "tconfigs/audio/common/element.h"

namespace tconfigs {
namespace audio {

class DataQueue : public Element,
                  public std::enable_shared_from_this<DataQueue> {
public:
    static std::shared_ptr<DataQueue> Create(const std::string& name,
            const rapidjson::Value& config);

    ~DataQueue(void) = default;

    bool Activate(Mode mode) override;
    bool Deactivate(void) override;

    int PushChain(std::shared_ptr<Pad> pad) override;

private:
    class Queue {
    public:
        static std::shared_ptr<Queue> Create(int max_units, int unit_bytes);
        ~Queue(void);
        bool Enqueue(const uint8_t* input_data);
        bool Dequeue(uint8_t* output_data);
        bool IsEmpty(void);
        bool IsFull(void);
    private:
        Queue(int max_units, int unit_bytes);
        bool Init(void);
        int max_units_;
        int unit_bytes_;
        int current_units_ = 0;
        int front_pos_ = 0;
        int rear_pos_ = 0;
        uint8_t* data_buf_ = nullptr;
    };

    DataQueue(void) = delete;
    DataQueue(const std::string& name);

    bool Init(const rapidjson::Value& config);

    void DequeueLoop(void);

    std::shared_ptr<Pad> sink_pad_;
    std::shared_ptr<Pad> src_pad_;

    int max_buffers_ = 0;
    std::shared_ptr<Queue> internal_queue_ = nullptr;

    std::atomic_bool is_stopped_;
    std::mutex queue_mutex_;
    std::condition_variable queue_not_empty_;
    std::condition_variable queue_not_full_;

    std::thread queue_thread_;
};

} // namespace audio
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_AUDIO_ELEMENTS_DATA_QUEUE_H__ */
