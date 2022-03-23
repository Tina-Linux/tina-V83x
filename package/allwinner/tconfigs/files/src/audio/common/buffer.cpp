#include "tconfigs/audio/common/buffer.h"

namespace tconfigs {
namespace audio {

BufferProperty::BufferProperty(int frames, int channels, int rate,
        FormatType format, StorageType storage)
    : frames_(frames),
      channels_(channels),
      rate_(rate),
      format_(format),
      storage_(storage)
{
}

const std::unordered_map<BufferProperty::FormatType, BufferProperty::BitsPerSample>
BufferProperty::kFormatToBitsMap = {
    // {FormatType, {effective_bits, total_bits}}
    {FormatType::S16_LE, {16, 16}},
    {FormatType::S24_LE, {24, 32}},
    {FormatType::S32_LE, {32, 32}},
};

const std::unordered_map<BufferProperty::FormatType, std::string>
BufferProperty::kFormatToStringMap = {
    {FormatType::S16_LE, "S16_LE"},
    {FormatType::S24_LE, "S24_LE"},
    {FormatType::S32_LE, "S32_LE"}
};

const std::unordered_map<BufferProperty::StorageType, std::string>
BufferProperty::kStorageToStringMap = {
    {StorageType::kInterleaved, "interleaved"},
    {StorageType::kNoninterleaved, "noninterleaved"}
};

BufferProperty::FormatType BufferProperty::BitsToFormatType(
        int effective_bits, int total_bits)
{
    for (auto itr = kFormatToBitsMap.begin(); itr != kFormatToBitsMap.end(); ++itr) {
        if (itr->second.effective_bits == effective_bits
                && itr->second.total_bits == total_bits) {
            return itr->first;
        }
    }
    return FormatType::UNKNOWN;
}

bool BufferProperty::FormatTypeToBits(FormatType type,
        int* effective_bits, int* total_bits)
{
    auto itr = kFormatToBitsMap.find(type);
    if (itr != kFormatToBitsMap.end()) {
        *effective_bits = itr->second.effective_bits;
        *total_bits = itr->second.total_bits;
        return true;
    }
    return false;
}

BufferProperty::FormatType BufferProperty::StringToFormatType(
        const std::string& format_string)
{
    for (auto itr = kFormatToStringMap.begin(); itr != kFormatToStringMap.end(); ++itr) {
        if (itr->second == format_string) {
            return itr->first;
        }
    }
    return FormatType::UNKNOWN;
}

std::string BufferProperty::FormatTypeToString(FormatType type)
{
    auto itr = kFormatToStringMap.find(type);
    if (itr == kFormatToStringMap.end()) {
        return "UNKNOWN";
    }
    return itr->second;
}

BufferProperty::StorageType BufferProperty::StringToStorageType(
        const std::string& storage_string)
{
    for (auto itr = kStorageToStringMap.begin(); itr != kStorageToStringMap.end(); ++itr) {
        if (itr->second == storage_string) {
            return itr->first;
        }
    }
    return StorageType::kUnknown;
}

std::string BufferProperty::StorageTypeToString(StorageType type)
{
    auto itr = kStorageToStringMap.find(type);
    if (itr == kStorageToStringMap.end()) {
        return "unknown";
    }
    return itr->second;
}

bool BufferProperty::IsMatching(const BufferProperty* p1,
        const BufferProperty* p2)
{
    if (p1->frames() == p2->frames()
            && p1->channels() == p2->channels()
            && p1->rate() == p2->rate()
            && p1->format() == p2->format()
            && p1->storage() == p2->storage()) {
        return true;
    }
    return false;
}

bool BufferProperty::IsEmpty(void) const
{
    if (frames_ == 0
            && channels_ == 0
            && rate_ == 0
            && format_ == FormatType::UNKNOWN
            && storage_ == StorageType::kUnknown) {
        return true;
    }
    return false;
}

bool BufferProperty::IsComplete(void) const
{
    if (frames_ != 0
            && channels_ != 0
            && rate_ != 0
            && format_ != FormatType::UNKNOWN
            && storage_ != StorageType::kUnknown) {
        return true;
    }
    return false;
}

#define OVERLAP_MASK_MEMBER(member, dst, src) \
do { \
    dst->member = dst->member && src->member; \
} while (0)

void BufferProperty::OverlapMask(Mask *dst, const Mask* src)
{
    OVERLAP_MASK_MEMBER(frames, dst, src);
    OVERLAP_MASK_MEMBER(channels, dst, src);
    OVERLAP_MASK_MEMBER(rate, dst, src);
    OVERLAP_MASK_MEMBER(format, dst, src);
    OVERLAP_MASK_MEMBER(storage, dst, src);
}

#define COMPLETE_PROPERTY_MEMBER(member, member_default_value, target_property) \
do { \
    if (member##_ == member_default_value) \
        member##_ = target_property->member(); \
} while (0)

//void BufferProperty::Complete(const BufferProperty* target_property)
//{
//    COMPLETE_PROPERTY_MEMBER(frames, 0, target_property);
//    COMPLETE_PROPERTY_MEMBER(channels, 0, target_property);
//    COMPLETE_PROPERTY_MEMBER(rate, 0, target_property);
//    COMPLETE_PROPERTY_MEMBER(format, FormatType::UNKNOWN, target_property);
//    COMPLETE_PROPERTY_MEMBER(storage, StorageType::kUnknown, target_property);
//}

#define COMPLETE_PROPERTY_MEMBER_WITH_MASK( \
        member, member_default_value, target_property, mask) \
do { \
    if (mask->member && member##_ == member_default_value) \
        member##_ = target_property->member(); \
} while (0)

//void BufferProperty::CompleteWithMask(
//        const BufferProperty* target_property, const Mask* mask)
//{
//    COMPLETE_PROPERTY_MEMBER_WITH_MASK(frames, 0, target_property, mask);
//    COMPLETE_PROPERTY_MEMBER_WITH_MASK(channels, 0, target_property, mask);
//    COMPLETE_PROPERTY_MEMBER_WITH_MASK(rate, 0, target_property, mask);
//    COMPLETE_PROPERTY_MEMBER_WITH_MASK(format, FormatType::UNKNOWN, target_property, mask);
//    COMPLETE_PROPERTY_MEMBER_WITH_MASK(storage, StorageType::kUnknown, target_property, mask);
//}

void BufferProperty::Complete(const BufferProperty* target_property, const Mask* mask)
{
    if (!mask) {
        COMPLETE_PROPERTY_MEMBER(frames, 0, target_property);
        COMPLETE_PROPERTY_MEMBER(channels, 0, target_property);
        COMPLETE_PROPERTY_MEMBER(rate, 0, target_property);
        COMPLETE_PROPERTY_MEMBER(format, FormatType::UNKNOWN, target_property);
        COMPLETE_PROPERTY_MEMBER(storage, StorageType::kUnknown, target_property);
    } else {
        COMPLETE_PROPERTY_MEMBER_WITH_MASK(frames, 0, target_property, mask);
        COMPLETE_PROPERTY_MEMBER_WITH_MASK(channels, 0, target_property, mask);
        COMPLETE_PROPERTY_MEMBER_WITH_MASK(rate, 0, target_property, mask);
        COMPLETE_PROPERTY_MEMBER_WITH_MASK(format, FormatType::UNKNOWN, target_property, mask);
        COMPLETE_PROPERTY_MEMBER_WITH_MASK(storage, StorageType::kUnknown, target_property, mask);
    }
}

std::shared_ptr<BufferInterface> BufferInterface::Create(const BufferProperty& property)
{
    switch(property.format()) {
    case BufferProperty::FormatType::S16_LE:
        return Buffer<int16_t>::Create(property);
    case BufferProperty::FormatType::S24_LE:
    case BufferProperty::FormatType::S32_LE:
        return Buffer<int32_t>::Create(property);
    default:
        return nullptr;
    }
}

} // namespace audio
} // namespace tconfigs
