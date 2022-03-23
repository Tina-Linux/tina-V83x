#define DEBUG
#include "MtpObjectInfo.h"
#include "MtpCommon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct MtpObjectInfo *getMtpObjectByNameWithParent(const char *name, MtpObjectHandle parent, struct list_head *objectLists)
{
	struct MtpObjectInfo *object = NULL;
	struct list_head *pos = NULL;
	struct list_head *head = objectLists;

	list_for_each(pos, head) {
		object = list_entry(pos, struct MtpObjectInfo, mList);
		DLOG("object parent:%u", object->mParent);
		if (object->mParent == parent) {
			if (!strcmp(name, object->mName)) {
				DLOG("found object(%s) with parent:%u", object->mName, parent);
				return object;
			}
		}
	}

	return NULL;
}

uint32_t *getMtpObjectHandlesList(MtpObjectHandle parent, size_t *arrayNum, struct list_head *objectLists)
{
	struct MtpObjectInfo *object = NULL;
	struct list_head *pos = NULL;
	struct list_head *head = objectLists;
	uint32_t *array = NULL;
	size_t num = 0;
#define OBJECHANDLESTLIST_ARRAY_NUM 10
	size_t defaultNum = OBJECHANDLESTLIST_ARRAY_NUM;

	array = (uint32_t *)malloc_wrapper(defaultNum*sizeof(uint32_t));
	if (!array) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}
#if 1
	list_for_each(pos, head) {
		object = list_entry(pos, struct MtpObjectInfo, mList);
		DLOG("object: name(%s) handle(%u) parent(%u)", object->mName, object->mHandle, object->mParent);
	}
#endif
	list_for_each(pos, head) {
		object = list_entry(pos, struct MtpObjectInfo, mList);
		DLOG("object parent:%u", object->mParent);
		if (object->mParent == parent) {
			num++;
			if (num > defaultNum) {
				uint32_t *array_tmp = array;
				defaultNum += OBJECHANDLESTLIST_ARRAY_NUM;
				array = realloc_wrapper(array, defaultNum*sizeof(uint32_t));
				if (!array) {
					free_wrapper(array_tmp);
					return NULL;
				}
			}
			DLOG("add handle:%u", object->mHandle);
			array[num-1] = object->mHandle;
		}
	}
	DLOG("found (parent:%u) ObjectHandlesList num:%u", parent, num);
	*arrayNum = num;

	return array;
}

struct MtpObjectInfo *getMtpObjectByPath(const char *path, struct list_head *objectLists)
{
	struct MtpObjectInfo *object;
	struct list_head *pos = NULL;
	struct list_head *head = objectLists;

	list_for_each(pos, head) {
		object = list_entry(pos, struct MtpObjectInfo, mList);
		DLOG("search path, object name:%s", object->mName);
		if (object->mFormat == MTP_FORMAT_ASSOCIATION) {
			struct Dir *dir = object->object.dir;
			if (!strcmp(path, dir->path)) {
				DLOG("found path:%s", dir->path);
				object->print(object);
				return object;
			}
		} else {
			struct File *file = object->object.file;
			if (!strcmp(path, file->path)) {
				DLOG("found path:%s", file->path);
				object->print(object);
				return object;
			}
		}
	}

	return NULL;
}

struct MtpObjectInfo *getMtpObjectByHandle(MtpObjectHandle handle, struct list_head *objectLists)
{
	struct MtpObjectInfo *object;
	struct list_head *pos = NULL;
	struct list_head *head = objectLists;

	list_for_each(pos, head) {
		object = list_entry(pos, struct MtpObjectInfo, mList);
		DLOG("search handle, object handle:%u", object->mHandle);
		if (object->mHandle == handle) {
			DLOG("found handle:%u", handle);
			object->print(object);
			return object;
		}
	}

	return NULL;
}

void MtpObjectSetParent(MtpObjectHandle parent, struct MtpObjectInfo *object)
{
	object->mParent = parent;
}

static void print(struct MtpObjectInfo *info)
{
	DLOG("");
	DLOG("MtpObject Info 0x%x: %s", info->mHandle, info->mName);
	DLOG("StorageID: 0x%x mFormat: 0x%x mProtectionStatus: 0x%0x",
		info->mStorageID, info->mFormat, info->mProtectionStatus);
	DLOG("mCompressedSize: %u mThumbFormat: 0x%x mThumbCompressedSize: %u",
		info->mCompressedSize, info->mThumbFormat, info->mThumbCompressedSize);
	DLOG("mThumbPixWidth: %u mThumbPixHeight: %u",
		info->mThumbPixWidth, info->mThumbPixHeight);
	DLOG("mImagePixWidth: %u mImagePixHeight: %u",
		info->mImagePixWidth, info->mImagePixHeight);
	DLOG("mParent: 0x%x mAsssociationType: 0x%x mAssociationDesc 0x%x",
		info->mParent, info->mAssociationType, info->mAssociationDesc);
	DLOG("mSequenceNumber %u mDateCreated %ld mDateModified: %ld mKeywords: %s",
		info->mSequenceNumber, info->mDateCreated, info->mDateModified, info->mKeywords);
}

static void fillObject(struct MtpStorage *mStorage, struct MtpObjectInfo *info)
{
	info->mStorageID = mStorage->mStorageID;
}

void updateObjectInfo(struct MtpObjectInfo *info)
{
	if (info->mName)
		free_wrapper(info->mName);
	if (info->mFormat == MTP_FORMAT_ASSOCIATION) {
		struct Dir *dir = info->object.dir;
		info->mName = strdup_wrapper((const char *)dir->dirName);
		info->mDateCreated = dir->dirAccessTime;
		info->mDateModified = dir->dirModifyTime;
	} else {
		struct File *file = info->object.file;
		info->mName = strdup_wrapper((const char *)file->fileName);
		info->mDateCreated = file->fileAccessTime;
		info->mDateModified = file->fileModifyTime;
		info->mCompressedSize = (uint32_t)file->fileSize;
	}
}

void deleteObjectInfo(struct MtpObjectInfo *info)
{
	list_del(&info->mList);
}

struct MtpObjectInfo *MtpObjectInfoInitWithDir(MtpObjectHandle handle, struct Dir *dir)
{
	struct MtpObjectInfo *mObjectInfo = NULL;

	mObjectInfo = (struct MtpObjectInfo *)calloc_wrapper(1, sizeof(struct MtpObjectInfo));

	mObjectInfo->mHandle = handle;
	mObjectInfo->mFormat = MTP_FORMAT_ASSOCIATION;

	INIT_LIST_HEAD(&mObjectInfo->mList);

	dir->object = mObjectInfo;

	mObjectInfo->mName = strdup_wrapper((const char *)dir->dirName);
	mObjectInfo->mDateCreated = dir->dirAccessTime;
	mObjectInfo->mDateModified = dir->dirModifyTime;
	mObjectInfo->mCompressedSize = (dir->dirSize > 0xFFFFFFFFLL ? 0xFFFFFFFF : (uint32_t)dir->dirSize);

	mObjectInfo->print = print;
	mObjectInfo->fillObject = fillObject;

	mObjectInfo->object.dir = dir;

	DLOG("init !!!object:%p!!!", mObjectInfo);
	return mObjectInfo;
}

struct MtpObjectInfo *MtpObjectInfoInitWithFile(MtpObjectHandle handle, struct File *file)
{
	struct MtpObjectInfo *mObjectInfo = NULL;

	mObjectInfo = (struct MtpObjectInfo *)calloc_wrapper(1, sizeof(struct MtpObjectInfo));

	mObjectInfo->mHandle = handle;
	/* TODO according file->fileType */
	mObjectInfo->mFormat = MTP_FORMAT_TEXT;
	INIT_LIST_HEAD(&mObjectInfo->mList);

	file->object = mObjectInfo;

	mObjectInfo->mName = strdup_wrapper((const char *)file->fileName);
	mObjectInfo->mDateCreated = file->fileAccessTime;
	mObjectInfo->mDateModified = file->fileModifyTime;
	mObjectInfo->mCompressedSize = (file->fileSize > 0xFFFFFFFFLL ? 0xFFFFFFFF : (uint32_t)file->fileSize);

	mObjectInfo->print = print;
	mObjectInfo->fillObject = fillObject;

	mObjectInfo->object.file = file;
	DLOG("init !!!object:%p!!!", mObjectInfo);
	return mObjectInfo;
}

void MtpObjectInfoRelease(struct MtpObjectInfo *mObjectInfo)
{
	DLOG("exit !!!object:%p!!!", mObjectInfo);
	if (!mObjectInfo)
		return;
	if (mObjectInfo->mName)
		free_wrapper(mObjectInfo->mName);
	free_wrapper(mObjectInfo);
}

void MtpObjectInfoListRelease(struct list_head *head)
{

	while(!list_empty(head)) {
		struct MtpObjectInfo *object = NULL;
		object = list_first_entry(head, struct MtpObjectInfo, mList);
		DLOG("object name:%s, list:%p, object:%p", object->mName, &object->mList, object);
		list_del(&object->mList);
		MtpObjectInfoRelease(object);
	}
	free_wrapper(head);
}
