#ifndef __TCONFIGS_AUDIO_COMMON_BUFFER_H__
#define __TCONFIGS_AUDIO_COMMON_BUFFER_H__

#include <memory>
#include <unordered_map>

namespace tconfigs {
namespace audio {

class BufferProperty {
public:
    enum class StorageType {
        kUnknown = 0,
        kInterleaved,
        kNoninterleaved,
    };

    enum class FormatType {
        UNKNOWN = 0,
        S16_LE,
        S24_LE,
        S32_LE,
    };

    struct BitsPerSample {
        int effective_bits;
        int total_bits;
    };

    struct Mask{
        bool frames = true;
        bool channels = true;
        bool rate = true;
        bool format = true;
        bool storage = true;
    };

    BufferProperty(void) = default;
    BufferProperty(int frames, int channels, int rate, FormatType format,
            StorageType storage);
    ~BufferProperty(void) = default;

    /**
     * @brief Convert the sample bits to @c FormatType
     *
     * @param effective_bits Effective bits in one sample (e.g. S24_LE is 24)
     * @param total_bits Total bits width of one sample (e.g. S24_LE is 32)
     * @return Corresponding @c FormatType; if not exits, return @c FormatType::UNKNOWN
     */
    static FormatType BitsToFormatType(int effective_bits, int total_bits);
    /**
     * @brief Inverse function of @c BitsToFormatType
     *
     * @param type
     * @param[out] effective_bits
     * @param[out] total_bits
     * @return @c true if the conversion is successful, otherwise @c false
     */
    static bool FormatTypeToBits(FormatType type, int* effective_bits, int* total_bits);
    /**
     * @brief Convert the format string to @c FormatType
     *
     * @param format_string Such as "S16_LE", "S24_LE", etc., which is the
     *      string of @c FormatType
     * @return Corresponding @c FormatType; if not exits, return @c FormatType::UNKNOWN
     */
    static FormatType StringToFormatType(const std::string& format_string);
    /**
     * @brief Inverse function of @c StringToFormatType
     *
     * @param type
     * @return Format string; if not exits, return "UNKNOWN"
     */
    static std::string FormatTypeToString(FormatType type);
    /**
     * @brief Convert the storage string to @c StorageType
     *
     * @param storage_string Such as "interleaved", "noninterleaved", etc.
     * @return Corresponding @c StorageType; if not exits, return @c StorageType::kUnknown
     */
    static StorageType StringToStorageType(const std::string& storage_string);
    /**
     * @brief Inverse function of @c StringToStorageType
     *
     * @param type
     * @return Storage string; if not exits, return "unknown"
     */
    static std::string StorageTypeToString(StorageType type);

    /**
     * @brief Check whether the two @c BufferProperty is matching
     *
     * "Matching" means that the parameters of these @c BufferProperty are the same.
     */
    static bool IsMatching(const BufferProperty* p1, const BufferProperty* p2);

    static void OverlapMask(Mask* dst, const Mask* src);

    bool IsEmpty(void) const;
    bool IsComplete(void) const;
    void Complete(const BufferProperty* target_property, const Mask* mask);

    void set_frames(int frames) { frames_ = frames; }
    void set_channels(int channels) { channels_ = channels; }
    void set_rate(int rate) { rate_ = rate; }
    void set_format(FormatType format) { format_ = format; }
    void set_storage(StorageType storage) { storage_ = storage; }

    int frames(void) const { return frames_; }
    int channels(void) const { return channels_; }
    int rate(void) const { return rate_; }
    FormatType format(void) const { return format_; }
    StorageType storage(void) const { return storage_; }

private:
    int frames_ = 0;
    int channels_ = 0;
    int rate_ = 0;
    FormatType format_ = FormatType::UNKNOWN;
    StorageType storage_ = StorageType::kUnknown;

    static const std::unordered_map<FormatType, BitsPerSample> kFormatToBitsMap;
    static const std::unordered_map<FormatType, std::string> kFormatToStringMap;
    static const std::unordered_map<StorageType, std::string> kStorageToStringMap;
};

class BufferInterface {
public:
    BufferInterface(void) = default;
    virtual ~BufferInterface(void) = default;

    virtual void* data(void) = 0;
    virtual const BufferProperty* property(void) const = 0;

    static std::shared_ptr<BufferInterface> Create(const BufferProperty& property);
};

template <typename T>
class Buffer: public BufferInterface {
public:
    static std::shared_ptr<BufferInterface> Create(const BufferProperty& property);

    ~Buffer(void);

    void* data(void) override { return static_cast<void*>(data_); }
    const BufferProperty* property(void) const override { return &property_; }

private:
    Buffer(const BufferProperty& property);
    Buffer(void) = delete;
    Buffer(const Buffer& buffer) = delete;
    Buffer& operator=(const Buffer& buffer) = delete;

    bool Init(void);
    void Release(void);

    T* data_ = nullptr;
    BufferProperty property_;
};

template <typename T>
std::shared_ptr<BufferInterface> Buffer<T>::Create(const BufferProperty& property)
{
    auto buffer = std::shared_ptr<Buffer<T>>(new Buffer<T>(property));
    if (!buffer || !buffer->Init()) {
        return nullptr;
    }
    std::shared_ptr<BufferInterface> buffer_interface(buffer);
    return buffer_interface;
}

template <typename T>
Buffer<T>::Buffer(const BufferProperty& property)
    : property_(property)
{
}

template <typename T>
Buffer<T>::~Buffer(void)
{
    Release();
}

template <typename T>
bool Buffer<T>::Init(void)
{
    data_ = (T*)malloc(
            sizeof(T) * property_.frames() * property_.channels());
    return data_ ? true : false;
}

template <typename T>
void Buffer<T>::Release(void)
{
    if (data_) {
        free(data_);
        data_ = nullptr;
    }
}

} // namespace audio {
} // namespace tconfigs {

#endif /* ifndef __TCONFIGS_AUDIO_COMMON_BUFFER_H__ */
