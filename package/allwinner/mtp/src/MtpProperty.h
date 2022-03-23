#ifndef _MTPPROPERTY_H
#define _MTPPROPERTY_H

#include "mtp.h"
#include "MtpPacket.h"
#include "MtpObjectInfo.h"

typedef struct {
    union {
        int8_t          i8;
        uint8_t         u8;
        int16_t         i16;
        uint16_t        u16;
        int32_t         i32;
        uint32_t        u32;
        int64_t         i64;
        uint64_t        u64;
    } u;
    char*               str;
}MtpPropertyValue;

enum {
	kFormNone = 0,
	kFormRange = 1,
	kFormEnum = 2,
	kFormDataTime = 3,
};

struct MtpProperty {
	MtpPropertyCode mCode;
	MtpDataType mType;
	bool mWriteable;

	uint8_t mFormFlag;
	uint32_t mGroupCode;
	MtpPropertyValue mDefaultValue;
	MtpPropertyValue mCurrentValue;

	MtpPropertyValue mMinimumValue;
	MtpPropertyValue mMaximumValue;
	MtpPropertyValue mStepSize;

	void (*write)(struct MtpPacket *mData, struct MtpProperty *mProperty);

};

struct MtpProperty *MtpPropertyInit(MtpPropertyCode propCode, MtpDataType type, bool writeable, int defaultValue);
void MtpPropertyRelease(struct MtpProperty *mProperty);
MtpResponseCode getObjectPropertyList(struct MtpObjectInfo *info,
					uint32_t format,
					uint32_t property,
					int groupCode,
					int depth,
					struct MtpPacket *mData);
MtpResponseCode getSupportedObjectProperties(uint32_t format, struct MtpPacket *mData);

#endif
