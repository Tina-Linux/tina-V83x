#ifndef _MTPOBJECTINFO_H
#define _MTPOBJECTINFO_H

#include "mtp.h"
#include "list.h"
#include "Dir.h"
#include "File.h"
#include "MtpStorage.h"


typedef union {
	struct Dir *dir;
	struct File *file;
}MtpObject;

struct MtpObjectInfo {
	MtpObjectHandle mHandle;
	MtpStorageID mStorageID;
	MtpObjectFormat mFormat;
	MtpObjectHandle mParent;
	struct list_head mList;

	uint16_t mProtectionStatus;
	uint32_t mCompressedSize;
	MtpObjectFormat mThumbFormat;
	uint32_t mThumbCompressedSize;
	uint32_t mThumbPixWidth;
	uint32_t mThumbPixHeight;
	uint32_t mImagePixWidth;
	uint32_t mImagePixHeight;
	uint32_t mImagePixDepth;
	uint16_t mAssociationType;
	uint32_t mAssociationDesc;
	uint32_t mSequenceNumber;
	char *mName;
	time_t mDateCreated;
	time_t mDateModified;
	char *mKeywords;

	MtpObject object;

	void (*print)(struct MtpObjectInfo *info);
	void (*fillObject)(struct MtpStorage *mStorage, struct MtpObjectInfo *info);
};

struct MtpObjectInfo *MtpObjectInfoInitWithDir(MtpObjectHandle handle, struct Dir *dir);
struct MtpObjectInfo *MtpObjectInfoInitWithFile(MtpObjectHandle handle, struct File *file);
void MtpObjectInfoRelease(struct MtpObjectInfo *mObjectInfo);
void MtpObjectInfoListRelease(struct list_head *head);
void MtpObjectSetParent(MtpObjectHandle parent, struct MtpObjectInfo *object);
struct MtpObjectInfo *getMtpObjectByHandle(MtpObjectHandle handle, struct list_head *objectLists);
struct MtpObjectInfo *getMtpObjectByPath(const char *path, struct list_head *objectLists);
struct MtpObjectInfo *getMtpObjectByNameWithParent(const char *name, MtpObjectHandle parent, struct list_head *objectLists);
uint32_t *getMtpObjectHandlesList(MtpObjectHandle parent, size_t *arrayNum, struct list_head *objectLists);
void deleteObjectInfo(struct MtpObjectInfo *info);
void updateObjectInfo(struct MtpObjectInfo *info);
#endif
