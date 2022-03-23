#define DEBUG
#include "MtpProperty.h"
#include "MtpCommon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct MtpPropertyList {
	uint32_t mCount;
	uint32_t *mObjectHandles;
	uint16_t *mPropertyCodes;
	uint16_t *mDataTypes;
	uint32_t **mLongValues;
	char **mStringValues;

	void (*append)(uint32_t handle, uint32_t property, uint32_t type, void *Value, struct MtpPropertyList *list);
};

static uint16_t FILE_PROPERTIES[] = {
	MTP_PROPERTY_STORAGE_ID,
	MTP_PROPERTY_OBJECT_FORMAT,
	MTP_PROPERTY_PROTECTION_STATUS,
	MTP_PROPERTY_OBJECT_SIZE,
	MTP_PROPERTY_OBJECT_FILE_NAME,
	MTP_PROPERTY_DATE_MODIFIED,
	MTP_PROPERTY_PARENT_OBJECT,
	MTP_PROPERTY_PERSISTENT_UID,
	MTP_PROPERTY_NAME,
	MTP_PROPERTY_DISPLAY_NAME,
	MTP_PROPERTY_DATE_ADDED,
};

static uint16_t AUDIO_PROPERTIES[] = {
	// NOTE must match FILE_PROPERTIES above
	MTP_PROPERTY_STORAGE_ID,
        MTP_PROPERTY_OBJECT_FORMAT,
        MTP_PROPERTY_PROTECTION_STATUS,
        MTP_PROPERTY_OBJECT_SIZE,
        MTP_PROPERTY_OBJECT_FILE_NAME,
        MTP_PROPERTY_DATE_MODIFIED,
        MTP_PROPERTY_PARENT_OBJECT,
        MTP_PROPERTY_PERSISTENT_UID,
        MTP_PROPERTY_NAME,
        MTP_PROPERTY_DISPLAY_NAME,
        MTP_PROPERTY_DATE_ADDED,

	// audio specific properties
	MTP_PROPERTY_ARTIST,
        MTP_PROPERTY_ALBUM_NAME,
        MTP_PROPERTY_ALBUM_ARTIST,
        MTP_PROPERTY_TRACK,
        MTP_PROPERTY_ORIGINAL_RELEASE_DATE,
        MTP_PROPERTY_DURATION,
        MTP_PROPERTY_GENRE,
        MTP_PROPERTY_COMPOSER,
        MTP_PROPERTY_AUDIO_WAVE_CODEC,
        MTP_PROPERTY_BITRATE_TYPE,
        MTP_PROPERTY_AUDIO_BITRATE,
        MTP_PROPERTY_NUMBER_OF_CHANNELS,
        MTP_PROPERTY_SAMPLE_RATE,
};

static uint16_t VIDEO_PROPERTIES[] = {
	// NOTE must match FILE_PROPERTIES above
	MTP_PROPERTY_STORAGE_ID,
	MTP_PROPERTY_OBJECT_FORMAT,
	MTP_PROPERTY_PROTECTION_STATUS,
	MTP_PROPERTY_OBJECT_SIZE,
	MTP_PROPERTY_OBJECT_FILE_NAME,
	MTP_PROPERTY_DATE_MODIFIED,
	MTP_PROPERTY_PARENT_OBJECT,
	MTP_PROPERTY_PERSISTENT_UID,
	MTP_PROPERTY_NAME,
	MTP_PROPERTY_DISPLAY_NAME,
	MTP_PROPERTY_DATE_ADDED,

	// video specific properties
	MTP_PROPERTY_ARTIST,
	MTP_PROPERTY_ALBUM_NAME,
	MTP_PROPERTY_DURATION,
	MTP_PROPERTY_DESCRIPTION,
};

static uint16_t IMAGE_PROPERTIES[] = {
	// NOTE must match FILE_PROPERTIES above
	MTP_PROPERTY_STORAGE_ID,
	MTP_PROPERTY_OBJECT_FORMAT,
	MTP_PROPERTY_PROTECTION_STATUS,
	MTP_PROPERTY_OBJECT_SIZE,
	MTP_PROPERTY_OBJECT_FILE_NAME,
	MTP_PROPERTY_DATE_MODIFIED,
	MTP_PROPERTY_PARENT_OBJECT,
	MTP_PROPERTY_PERSISTENT_UID,
	MTP_PROPERTY_NAME,
	MTP_PROPERTY_DISPLAY_NAME,
	MTP_PROPERTY_DATE_ADDED,

	// image specific properties
	MTP_PROPERTY_DESCRIPTION,
};


static void append(uint32_t handle, uint32_t property, uint32_t type, void *Value, struct MtpPropertyList *list)
{
	uint32_t index = list->mCount++;
	list->mObjectHandles[index] = handle;
	list->mPropertyCodes[index] = property;
	list->mDataTypes[index] = type;

	if (list->mStringValues[index] != NULL || list->mLongValues[index] != NULL) {
		printf("mStringValues[%u] = %p, mLongValues[%u] = %p, may error?\n",
			index, list->mStringValues[index], index, list->mLongValues[index]);
		exit(-1);
	}
	switch (type) {
		case MTP_TYPE_STR:
			list->mStringValues[index] = strdup_wrapper((char*)Value);
			break;
		case MTP_TYPE_UINT16:
			list->mLongValues[index] = calloc_wrapper(1, sizeof(uint32_t));
			memcpy(list->mLongValues[index], Value, sizeof(uint16_t));
			break;
		case MTP_TYPE_UINT32:
			list->mLongValues[index] = calloc_wrapper(1, sizeof(uint32_t));
			*list->mLongValues[index] = *((uint32_t *)Value);
			break;
		case MTP_TYPE_UINT64:
			list->mLongValues[index] = calloc_wrapper(1, sizeof(uint64_t));
			memcpy(list->mLongValues[index], Value, sizeof(uint64_t));
			break;
		case MTP_TYPE_UINT128:
			list->mLongValues[index] = calloc_wrapper(1, sizeof(uint128_t));
			memcpy(list->mLongValues[index], Value, sizeof(uint128_t));
			break;
	}
	return ;
}

static void test(struct MtpPropertyList *list)
{
#if 0
	int count = list->mCount;
	int i;

	for (i = 0; i < count; i++) {
		if (list->mStringValues[i] != NULL)
			DLOG("mStringValues[%d] != NULL", i);
	}
#endif
}

static void MtpPropertyListCreateObject(struct MtpObjectInfo *info, struct MtpPropertyList *list)
{
	char buf[128];
	uint64_t objectSize = info->mCompressedSize;
	uint32_t parentObject = info->mParent;
	uint128_t persistentUID = {info->mHandle, info->mStorageID, 0x0, 0x0};
	char *displayName = "";

	test(list);
	list->append(info->mHandle, MTP_PROPERTY_STORAGE_ID, MTP_TYPE_UINT32, &info->mStorageID, list);
	test(list);
	list->append(info->mHandle, MTP_PROPERTY_OBJECT_FORMAT, MTP_TYPE_UINT16, &info->mFormat, list);
	test(list);
	list->append(info->mHandle, MTP_PROPERTY_PROTECTION_STATUS, MTP_TYPE_UINT16, &info->mProtectionStatus, list);
	test(list);
	list->append(info->mHandle, MTP_PROPERTY_OBJECT_SIZE, MTP_TYPE_UINT64, &objectSize, list);
	test(list);
	list->append(info->mHandle, MTP_PROPERTY_OBJECT_FILE_NAME, MTP_TYPE_STR, info->mName, list);
	test(list);
	formatDateTime(info->mDateModified, buf, sizeof(buf));
	list->append(info->mHandle, MTP_PROPERTY_DATE_MODIFIED, MTP_TYPE_STR, buf, list);
	list->append(info->mHandle, MTP_PROPERTY_PARENT_OBJECT, MTP_TYPE_UINT32, &parentObject, list);
	list->append(info->mHandle, MTP_PROPERTY_PERSISTENT_UID, MTP_TYPE_UINT128, &persistentUID, list);
	list->append(info->mHandle, MTP_PROPERTY_NAME, MTP_TYPE_STR, info->mName, list);
	test(list);
	list->append(info->mHandle, MTP_PROPERTY_DISPLAY_NAME, MTP_TYPE_STR, displayName, list);
	formatDateTime(info->mDateCreated, buf, sizeof(buf));
	list->append(info->mHandle, MTP_PROPERTY_DATE_ADDED, MTP_TYPE_STR, buf, list);
	test(list);
}

static struct MtpPropertyList *MtpPropertyListInit(uint32_t maxCount)
{
	struct MtpPropertyList * list;

	list = (struct MtpPropertyList *)calloc_wrapper(1, sizeof(struct MtpPropertyList));
	memset(list, 0, sizeof(struct MtpPropertyList));

	list->mCount = 0;
	list->mObjectHandles = calloc_wrapper(maxCount, sizeof(uint32_t));
	list->mPropertyCodes = calloc_wrapper(maxCount, sizeof(uint16_t));
	list->mDataTypes = calloc_wrapper(maxCount, sizeof(uint16_t));
	list->mLongValues = calloc_wrapper(maxCount, sizeof(uint32_t **));
	list->mStringValues = calloc_wrapper(maxCount, sizeof(char **));

	list->append = append;

	return list;
}

void MtpPropertyListRelease(struct MtpPropertyList * list)
{
	if (!list)
		return;
	if (list->mObjectHandles)
		free_wrapper(list->mObjectHandles);
	if (list->mPropertyCodes)
		free_wrapper(list->mPropertyCodes);
	if (list->mDataTypes)
		free_wrapper(list->mDataTypes);
	if (list->mLongValues) {
		int i;
		for (i = 0; i < list->mCount; i++) {
			if (list->mLongValues[i] != NULL) {
				free_wrapper(list->mLongValues[i]);
				list->mLongValues[i] = NULL;
			}
		}
		free_wrapper(list->mLongValues);
	}
	if (list->mStringValues) {
		int i;
		for (i = 0; i < list->mCount; i++) {
			if (list->mStringValues[i] != NULL) {
				free_wrapper(list->mStringValues[i]);
				list->mStringValues[i] = NULL;
			}
		}
		free_wrapper(list->mStringValues);
	}
	free_wrapper(list);
	return ;
}

MtpResponseCode getSupportedObjectProperties(uint32_t format, struct MtpPacket *mData)
{
	uint16_t *list = NULL;
	uint32_t num = 0;
	uint32_t i;

	DLOG("list size:%d, num:%d", sizeof(FILE_PROPERTIES), ARRAY_SIZE(FILE_PROPERTIES));
	switch (format) {
		case MTP_FORMAT_MP3:
		case MTP_FORMAT_WAV:
		case MTP_FORMAT_WMA:
		case MTP_FORMAT_OGG:
		case MTP_FORMAT_AAC:
			list = AUDIO_PROPERTIES;
			num = ARRAY_SIZE(AUDIO_PROPERTIES);
			break;
		case MTP_FORMAT_MPEG:
		case MTP_FORMAT_3GP_CONTAINER:
		case MTP_FORMAT_WMV:
			list = AUDIO_PROPERTIES;
			num = ARRAY_SIZE(AUDIO_PROPERTIES);
			break;
		case MTP_FORMAT_EXIF_JPEG:
		case MTP_FORMAT_GIF:
		case MTP_FORMAT_PNG:
		case MTP_FORMAT_BMP:
		case MTP_FORMAT_DNG:
			list = AUDIO_PROPERTIES;
			num = ARRAY_SIZE(AUDIO_PROPERTIES);
			break;
		case MTP_FORMAT_ASSOCIATION:
		case MTP_FORMAT_TEXT:
		default:
			list = FILE_PROPERTIES;
			num = ARRAY_SIZE(FILE_PROPERTIES);
			break;
	}

	if (!list)
		return MTP_RESPONSE_GENERAL_ERROR;

	mData->putData32(num, mData);
	for (i = 0; i < num; i++) {
		mData->putData16(list[i], mData);
	}

	return MTP_RESPONSE_OK;
}

MtpResponseCode getObjectPropertyList(struct MtpObjectInfo *info,
					uint32_t format,
					uint32_t property,
					int groupCode,
					int depth,
					struct MtpPacket *mData)
{
	struct MtpPropertyList *list = NULL;
	uint32_t count;
	int i;

	if (property == 0xFFFFFFFFL) {
		count = 11;
	} else {
		count = 1;
	}
	list = MtpPropertyListInit(count);
	DLOG("-----format=0x%x, property=0x%x----", format, property);
	switch (property) {
		case MTP_PROPERTY_OBJECT_FORMAT:
			list->append(info->mHandle, MTP_PROPERTY_OBJECT_FORMAT, MTP_TYPE_UINT16, &info->mFormat, list);
			break;
		case MTP_PROPERTY_PROTECTION_STATUS:
			list->append(info->mHandle, MTP_PROPERTY_PROTECTION_STATUS, MTP_TYPE_UINT16, &info->mProtectionStatus, list);
			break;
		case MTP_PROPERTY_OBJECT_SIZE:
			{
			uint64_t objectSize = 0;
			list->append(info->mHandle, MTP_PROPERTY_OBJECT_SIZE, MTP_TYPE_UINT64, &objectSize, list);
			}
			break;
		case 0xFFFFFFFFL:
			MtpPropertyListCreateObject(info, list);
			break;
		default:
			DLOG("Unknow property!");
			break;
	}

	DLOG("count=%u, list mCount=%u", count, list->mCount);

	mData->putData32(list->mCount, mData);
	for (i = 0; i < count; i++) {
		mData->putData32(list->mObjectHandles[i], mData);
		mData->putData16(list->mPropertyCodes[i], mData);
		mData->putData16(list->mDataTypes[i], mData);
		switch(list->mDataTypes[i]) {
			case MTP_TYPE_UINT16:
				mData->putData16(*((uint16_t *)list->mLongValues[i]), mData);
				break;
			case MTP_TYPE_UINT32:
				mData->putData32(*((uint32_t *)list->mLongValues[i]), mData);
				break;
			case MTP_TYPE_UINT64:
				mData->putData64(*((uint64_t *)list->mLongValues[i]), mData);
				break;
			case MTP_TYPE_UINT128:
				mData->putData128(*((uint128_t *)list->mLongValues[i]), mData);
				break;
			case MTP_TYPE_STR:
				mData->putString(list->mStringValues[i], mData);
				break;
		}
	}

	MtpPropertyListRelease(list);
	return MTP_RESPONSE_OK;
}


static bool isDeviceProperty(struct MtpProperty *mProperty)
{
	return ( ((mProperty->mCode & 0xF000) == 0x5000)
		|| ((mProperty->mCode & 0xF800) == 0xD000));
}

static void writeValue(struct MtpPacket *mData, MtpPropertyValue Value, struct MtpProperty *mProperty)
{
	switch (mProperty->mType) {
		case MTP_TYPE_UINT8:
		case MTP_TYPE_AUINT8:
			mData->putData8(Value.u.u8, mData);
			break;
		case MTP_TYPE_UINT16:
		case MTP_TYPE_AUINT16:
			mData->putData16(Value.u.u16, mData);
			break;
		case MTP_TYPE_UINT32:
		case MTP_TYPE_AUINT32:
			mData->putData32(Value.u.u32, mData);
			break;
		case MTP_TYPE_UINT64:
		case MTP_TYPE_AUINT64:
			mData->putData64(Value.u.u64, mData);
			break;
		case MTP_TYPE_STR:
			if (Value.str)
				mData->putString(Value.str, mData);
			else
				mData->putData8(0, mData);
			break;
		default:
			DLOG("TOGO unknown type 0x%X", mProperty->mType);
			break;
	}
}

static void MtpPropertyWrite(struct MtpPacket *mData, struct MtpProperty *mProperty)
{
	bool deviceProp = isDeviceProperty(mProperty);
	mData->putData16(mProperty->mCode, mData);
	mData->putData16(mProperty->mType, mData);
	mData->putData8(mProperty->mWriteable ? 1 : 0, mData);

	DLOG("Type:%d deviceProp:%d", mProperty->mType, deviceProp ? 1 : 0);
	switch (mProperty->mType) {
		case MTP_TYPE_AINT8:
			break;
		default:
			writeValue(mData, mProperty->mDefaultValue, mProperty);
			if (deviceProp)
				writeValue(mData, mProperty->mCurrentValue, mProperty);
	}
	if (!deviceProp)
		mData->putData32(mProperty->mGroupCode, mData);
	mData->putData8(mProperty->mFormFlag, mData);
	if (mProperty->mFormFlag != kFormNone) {
		DLOG("TODO mFormFlag value!");
	}

}


struct MtpProperty *MtpPropertyInit(MtpPropertyCode propCode, MtpDataType type, bool writeable, int defaultValue)
{
	struct MtpProperty *mProperty;

	mProperty = (struct MtpProperty *)calloc_wrapper(1, sizeof(struct MtpProperty));

	mProperty->mCode = propCode;
	mProperty->mType = type;
	mProperty->mWriteable = writeable;
	mProperty->mGroupCode = 0;
	mProperty->mFormFlag = kFormNone;

	mProperty->write = MtpPropertyWrite;

	if (defaultValue) {
		switch (type) {
			case MTP_TYPE_INT8:
				mProperty->mDefaultValue.u.i8 = defaultValue;
				break;
			case MTP_TYPE_UINT8:
				mProperty->mDefaultValue.u.u8 = defaultValue;
				break;
			case MTP_TYPE_INT16:
				mProperty->mDefaultValue.u.i16 = defaultValue;
				break;
			case MTP_TYPE_UINT16:
				mProperty->mDefaultValue.u.u16 = defaultValue;
				break;
			case MTP_TYPE_INT32:
				mProperty->mDefaultValue.u.i32 = defaultValue;
				break;
			case MTP_TYPE_UINT32:
				mProperty->mDefaultValue.u.u32 = defaultValue;
				break;
			case MTP_TYPE_INT64:
				mProperty->mDefaultValue.u.i64 = defaultValue;
				break;
			case MTP_TYPE_UINT64:
				mProperty->mDefaultValue.u.u64 = defaultValue;
				break;
			default:
				DLOG("unknown type %04X in MtpProperty", type);
				break;
		}
	}

	return mProperty;
}

void MtpPropertyRelease(struct MtpProperty *mProperty)
{
	if (!mProperty)
		return ;
	free_wrapper(mProperty);
}
