#define DEBUG
#include "MtpServer.h"
#include "MtpDebug.h"
#include "MtpProperty.h"
#include "f_mtp.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <signal.h>

static const MtpOperationCode kSupportedOperationCodes[] = {
	MTP_OPERATION_GET_DEVICE_INFO,
	MTP_OPERATION_OPEN_SESSION,
	MTP_OPERATION_CLOSE_SESSION,
	MTP_OPERATION_GET_STORAGE_IDS,
	MTP_OPERATION_GET_STORAGE_INFO,
	MTP_OPERATION_GET_NUM_OBJECTS,
	MTP_OPERATION_GET_OBJECT_HANDLES,
	MTP_OPERATION_GET_OBJECT_INFO,
	MTP_OPERATION_GET_OBJECT,
	MTP_OPERATION_GET_THUMB,
	MTP_OPERATION_DELETE_OBJECT,
	MTP_OPERATION_SEND_OBJECT_INFO,
	MTP_OPERATION_SEND_OBJECT,
#if 0
	MTP_OPERATION_INITIATE_CAPTURE,
	MTP_OPERATION_FORMAT_STORE,
	MTP_OPERATION_RESET_DEVICE,
	MTP_OPERATION_SELF_TEST,
	MTP_OPERATION_SET_OBJECT_PROTECTION,
	MTP_OPERATION_POWER_DOWN,
#endif
	MTP_OPERATION_GET_DEVICE_PROP_DESC,
	MTP_OPERATION_GET_DEVICE_PROP_VALUE,
	MTP_OPERATION_SET_DEVICE_PROP_VALUE,
	MTP_OPERATION_RESET_DEVICE_PROP_VALUE,
#if 0
	MTP_OPERATION_TERMINATE_OPEN_CAPTURE,
	MTP_OPERATION_MOVE_OBJECT,
	MTP_OPERATION_COPY_OBJECT,
#endif
	MTP_OPERATION_GET_PARTIAL_OBJECT,
#if 0
	MTP_OPERATION_INITIATE_OPEN_CAPTURE,
#endif
	MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED,
	MTP_OPERATION_GET_OBJECT_PROP_DESC,
	MTP_OPERATION_GET_OBJECT_PROP_VALUE,
	MTP_OPERATION_SET_OBJECT_PROP_VALUE,
	MTP_OPERATION_GET_OBJECT_PROP_LIST,
#if 0
	MTP_OPERATION_GET_INTERDEPENDENT_PROP_DESC,
	MTP_OPERATION_SEND_OBJECT_PROP_LIST,
#endif
	MTP_OPERATION_GET_OBJECT_REFERENCES,
	MTP_OPERATION_SET_OBJECT_REFERENCES,

	MTP_OPERATION_GET_PARTIAL_OBJECT_64,
	MTP_OPERATION_SEND_PARTIAL_OBJECT,
	MTP_OPERATION_TRUNCATE_OBJECT,
	MTP_OPERATION_BEGIN_EDIT_OBJECT,
	MTP_OPERATION_END_EDIT_OBJECT,
};

static const MtpEventCode kSupportedEventCodes[] = {
	MTP_EVENT_OBJECT_ADDED,
	MTP_EVENT_OBJECT_REMOVED,
	MTP_EVENT_STORE_ADDED,
	MTP_EVENT_STORE_REMOVED,
	MTP_EVENT_DEVICE_PROP_CHANGED,
};

static struct MtpStorage *getStorage(MtpStorageID id, struct MtpServer *mServer)
{
	int count = VectorSize(&mServer->mStorageList);
	int i;

	DLOG("count = %d id = %u", count, id);
	for (i = 0; i < count; i++) {
		struct MtpStorage *mStorage = VectorObject(i, &mServer->mStorageList);
		if (mStorage->mStorageID == id) {
			DLOG("found Storage. path:%s", MtpStorageGetPath(mStorage));
			return mStorage;
		}
	}
	return NULL;
}

static bool hasStorage(struct MtpServer *mServer)
{
	if (VectorSize(&mServer->mStorageList) > 0)
		return true;
	return false;
}

struct ObjectEdit {
	MtpObjectHandle mHandle;
	char *mPath;
	uint64_t mSize;
	MtpObjectFormat mFormat;
	int mFd;
};

static struct ObjectEdit *ObjectEditInit(MtpObjectHandle handle,
					const char *path,
					uint64_t size,
					MtpObjectFormat format, int fd)
{
	struct ObjectEdit *mObjectEdit;

	mObjectEdit = (struct ObjectEdit *)calloc_wrapper(1, sizeof(struct ObjectEdit));

	mObjectEdit->mHandle = handle;
	mObjectEdit->mPath = strdup_wrapper(path);
	mObjectEdit->mSize = size;
	mObjectEdit->mFormat = format;
	mObjectEdit->mFd = fd;

	return mObjectEdit;
}

static void ObjectEditRelease(struct ObjectEdit *mObjectEdit)
{
	if (!mObjectEdit)
		return ;
	if (mObjectEdit->mPath)
		free_wrapper(mObjectEdit->mPath);
	free_wrapper(mObjectEdit);
}

static struct ObjectEdit *getEditObject(MtpObjectHandle handle, Vector *list)
{
	int count = VectorSize(list);
	int i;
	for (i = 0; i < count; i++) {
		struct ObjectEdit *edit = VectorObject(i, list);
		if (edit->mHandle == handle)
			return edit;
	}
	return NULL;
}

static void addEditObject(MtpObjectHandle handle, const char *path, uint64_t size,
			MtpObjectFormat format, int fd, Vector *list)
{
	struct ObjectEdit *edit = ObjectEditInit(handle, path, size, format, fd);

	VectorAdd(edit, list);

	return ;
}

static void removeEditObject(MtpObjectHandle handle, Vector *list)
{
	int count = VectorSize(list);
	int i;

	for (i = 0; i < count; i++) {
		struct ObjectEdit *edit = VectorObject(i, list);
		if (edit->mHandle == handle) {
			ObjectEditRelease(edit);
			VectorRemove(i, list);
		}
	}
}

static void commitEdit(struct ObjectEdit *edit, struct MtpDataBase *mDataBase)
{
	MtpDataBaseEndSendObject((const char *)edit->mPath, edit->mHandle, edit->mFormat, true, mDataBase);
}

static MtpResponseCode doGetStorageIDs(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	int count, i;
	DLOG("");
	if(!mServer->mSessionOpen)
		return MTP_RESPONSE_SESSION_NOT_OPEN;

	count = VectorSize(&mServer->mStorageList);
	mData->putData32(count, mData);
	for (i = 0; i < count; i++) {
		struct MtpStorage *mStorage = VectorObject(i, &mServer->mStorageList);
		mData->putData32(mStorage->mStorageID, mData);
	}

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetStorageInfo(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	MtpStorageID id;
	struct MtpStorage *mStorage = NULL;

	if(!mServer->mSessionOpen)
		return MTP_RESPONSE_SESSION_NOT_OPEN;
	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	id = mRequest->getParameter(1, mRequest);

	mStorage = getStorage(id, mServer);
	if (!mStorage)
		return MTP_RESPONSE_INVALID_STORAGE_ID;
	/* Type */
	mData->putData16(3, mData);
	/* FileSystem Type */
	mData->putData16(2, mData);
	/* AccessCapability */
	mData->putData16(0, mData);
	/* MaxCapacity */
	mData->putData64(mStorage->mMaxCapacity, mData);
	/* FreeSpace */
	mData->putData64(mStorage->mFreeSpace, mData);
	/* Free Space in Objects */
	mData->putData32(1024*1024*1024, mData);
	/* Volume Identifier */
	mData->putString(mStorage->mDescription, mData);
	mData->putData8(0, mData);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObjectPropsSupported(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	MtpObjectFormat format;

	format = mRequest->getParameter(2, mRequest);

	getSupportedObjectProperties(format, mData);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObjectReferences(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;

	mData->putData32(0, mData);
	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObjectHandles(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpStorageID storageID;
	MtpObjectFormat format;
	MtpObjectHandle parent;
	uint32_t *array;
	size_t arrayNum;

	if(!mServer->mSessionOpen)
		return MTP_RESPONSE_SESSION_NOT_OPEN;
	if (mRequest->getParameterCount(mRequest) < 3)
		return MTP_RESPONSE_INVALID_PARAMETER;

	storageID = mRequest->getParameter(1, mRequest);
	format = mRequest->getParameter(2, mRequest);
	parent = mRequest->getParameter(3, mRequest);

	DLOG("storageID:%u", storageID);
	DLOG("format:%u", format);
	DLOG("parent:%u", parent);

#if 0
	mStorage = getStorage(storageID, mServer);
	if (!mStorage)
		return MTP_RESPONSE_INVALID_STORAGE_ID;
	/* TODO */
	count = DirGetObjectCount(mStorage->mDisk->dDirRoot);
	array = (uint32_t *)malloc_wrapper(sizeof(uint32_t) * count);
	for (i = 0; i < count; i++)
		array[i] = i+1;
	mData->putAData32(array, sizeof(array)/sizeof(uint32_t), mData);
#else
	array = MtpDataBaseGetObjectHandlesList(storageID, format, parent, &arrayNum, mDataBase);
	mData->putAData32(array, arrayNum, mData);
#if 0
	DLOG("array num:%u", arrayNum);
	int i;
	for (i = 0; i < arrayNum; i++)
		printf("0x%x ", array[i]);
	printf("\n");
#endif
#endif

	free_wrapper(array);
	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObjectPropList(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct MtpObjectInfo info;
	MtpObjectHandle handle;
	uint32_t format, property;
	int groupCode, depth, result;

	if (mRequest->getParameterCount(mRequest) < 5)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	format = mRequest->getParameter(2, mRequest);
	property = mRequest->getParameter(3, mRequest);
	groupCode = mRequest->getParameter(4, mRequest);
	depth = mRequest->getParameter(5, mRequest);

	DLOG("handle:%u, format:%s, property:%s, group:%d, depth:%d\n",
		handle, getFormatCodeName(format),
		getObjectPropCodeName(property), groupCode, depth);

	memset(&info, 0, sizeof(struct MtpObjectInfo));
	result = MtpDataBaseGetObjectInfo(handle, &info, mDataBase);
	if (result != MTP_RESPONSE_OK)
		return result;

	DLOG("");
	info.print(&info);
	getObjectPropertyList(&info, format, property, groupCode, depth, mData);
	DLOG("");
#if 0
	/* Object Count */
	mData->putData32(11, mData);
	/* MTP_PROPERTY_STORAGE_ID */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_STORAGE_ID, mData);
	mData->putData16(MTP_TYPE_UINT32, mData);
	mData->putData32(65537, mData);
	/* MTP_PROPERTY_OBJECT_FORMAT */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_OBJECT_FORMAT, mData);
	mData->putData16(MTP_TYPE_UINT16, mData);
	mData->putData16(MTP_FORMAT_ASSOCIATION, mData);
	/* MTP_PROPERTY_PROTECTION_STATUS */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_PROTECTION_STATUS, mData);
	mData->putData16(MTP_TYPE_UINT16, mData);
	mData->putData16(0, mData);
	/* MTP_PROPERTY_OBJECT_SIZE */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_OBJECT_SIZE, mData);
	mData->putData16(MTP_TYPE_UINT64, mData);
	mData->putData64(0, mData);
	/* MTP_PROPERTY_OBJECT_FILE_NAME */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_OBJECT_FILE_NAME, mData);
	mData->putData16(MTP_TYPE_STR, mData);
	mData->putString("Music", mData);
	/* MTP_PROPERTY_DATE_MODIFIED */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_DATE_MODIFIED, mData);
	mData->putData16(MTP_TYPE_STR, mData);
	mData->putString("20100101T080142", mData);
	/* MTP_PROPERTY_PARENT_OBJECT */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_PARENT_OBJECT, mData);
	mData->putData16(MTP_TYPE_UINT32, mData);
	mData->putData32(0, mData);
	/* MTP_PROPERTY_PERSISTENT_UID */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_PERSISTENT_UID, mData);
	mData->putData16(MTP_TYPE_UINT128, mData);
	uint128_t value = {0x01, 0x10001, 0x0, 0x0};
	mData->putData128(value, mData);
	/* MTP_PROPERTY_NAME */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_NAME, mData);
	mData->putData16(MTP_TYPE_STR, mData);
	mData->putString("Music", mData);
	/* MTP_PROPERTY_DISPLAY_NAME */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_DISPLAY_NAME, mData);
	mData->putData16(MTP_TYPE_STR, mData);
	mData->putString("", mData);
	/* MTP_PROPERTY_DATE_ADDED */
	mData->putData32(1, mData);
	mData->putData16(MTP_PROPERTY_DATE_ADDED, mData);
	mData->putData16(MTP_TYPE_STR, mData);
	mData->putString("20100101T080142", mData);
#endif

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObject(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpObjectHandle handle;
	MtpObjectFormat format;
	int result, ret;
	char pathBuf[128];
	size_t pathBufLen = sizeof(pathBuf);
	uint64_t fileLength;
	struct mtp_file_range mfr;

	if (!hasStorage(mServer))
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);

	result = MtpDataBaseGetObjectFilePath(handle, pathBuf, pathBufLen, &fileLength, &format, mDataBase);
	if (result != MTP_RESPONSE_OK)
		return result;

	mfr.fd = open(pathBuf, O_RDONLY);
	if (mfr.fd < 0)
		return MTP_RESPONSE_GENERAL_ERROR;
	mfr.offset = 0;
	mfr.length = fileLength;
	mfr.command = mRequest->getOperationCode(mRequest);
	mfr.transaction_id = mRequest->getTransactionID(mRequest);

	ret = ioctl(mServer->mFd, MTP_SEND_FILE_WITH_HEADER, (unsigned long)&mfr);
	DLOG("MTP_SEND_FILE_WITH_HEADER returned %d", ret);
	close(mfr.fd);
	if (ret < 0) {
		if (errno == ECANCELED)
			return MTP_RESPONSE_TRANSACTION_CANCELLED;
		else
			return MTP_RESPONSE_GENERAL_ERROR;
	}

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetObjectInfo(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct MtpObjectInfo info;
	char date[20];
	MtpObjectHandle handle;
	int result;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	DLOG("handle:%u", handle);

	memset(&info, 0, sizeof(struct MtpObjectInfo));
	result = MtpDataBaseGetObjectInfo(handle, &info, mDataBase);
	if (result != MTP_RESPONSE_OK)
		return result;

	/* mStorageID */
	mData->putData32(info.mStorageID, mData);
	/* Format */
	mData->putData16(info.mFormat, mData);
	/* ProtectionStatus */
	mData->putData16(info.mProtectionStatus, mData);

	/* ObjectInfo */
	/* Size */
	mData->putData32(0x1000, mData);
	/* ThumbFormat */
	mData->putData16(info.mThumbFormat, mData);
	/* ThumbCompressedSize */
	mData->putData32(info.mThumbCompressedSize, mData);
	/* ThumbPixWidth */
	mData->putData32(info.mThumbPixWidth, mData);
	/* ThumbPixHeight */
	mData->putData32(info.mThumbPixHeight, mData);
	/* ImagePixWidth */
	mData->putData32(info.mImagePixWidth, mData);
	/* ImagePixHeight */
	mData->putData32(info.mImagePixHeight, mData);
	/* ImagePixDepth */
	mData->putData32(info.mImagePixDepth, mData);
	/* Parent */
	mData->putData32(info.mParent, mData);
	/* AssociationType */
	mData->putData16(info.mAssociationType, mData);
	/* AssociationDesc */
	mData->putData32(info.mAssociationDesc, mData);
	/* SequenceNumber */
	mData->putData32(info.mSequenceNumber, mData);
	/* Name */
	mData->putString(info.mName, mData);
	/* Created Date */
	formatDateTime(info.mDateCreated, date, sizeof(date));
	mData->putString(date, mData);
	/* Modify Date */
	formatDateTime(info.mDateModified, date, sizeof(date));
	mData->putString(date, mData);
	mData->putData8(0, mData);

#if 0
	/* TODO */
	/* mStorageID */
	mData->putData32(65537, mData);
	/* Format */
	mData->putData16(0x3001, mData);
	/* ProtectionStatus */
	mData->putData16(0, mData);


	/* ObjectInfo */
	/* Size */
	mData->putData32(0x1000, mData);
	/* ThumbFormat */
	mData->putData16(0, mData);
	/* ThumbCompressedSize */
	mData->putData32(0, mData);
	/* ThumbPixWidth */
	mData->putData32(0, mData);
	/* ThumbPixHeight */
	mData->putData32(0, mData);
	/* ImagePixWidth */
	mData->putData32(0, mData);
	/* ImagePixHeight */
	mData->putData32(0, mData);
	/* ImagePixDepth */
	mData->putData32(0, mData);
	/* Parent */
	mData->putData32(0, mData);
	/* AssociationType */
	mData->putData16(0, mData);
	/* AssociationDesc */
	mData->putData32(0, mData);
	/* SequenceNumber */
	mData->putData32(0, mData);
	/* Name */
	mData->putString("Test", mData);
	/* Created Date */
	mData->putString("20100101T080142", mData);
	/* Modify Date */
	mData->putString("20100101T080142", mData);
	mData->putData8(0, mData);
#endif
	return MTP_RESPONSE_OK;
}

static MtpResponseCode doSendObjectInfo(struct MtpServer *mServer)
{

	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mResponse = mServer->mResponsePacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	char path[PATH_MAX];
	size_t pathLen = sizeof(path);
	uint64_t length, maxFileSize;
	MtpObjectHandle parent, handle = 0;
	MtpStorageID storageID;
	struct MtpStorage *mStorage;
	MtpObjectFormat format;
	uint16_t temp16;
	uint32_t temp32;
	uint16_t associationType;
	uint32_t associationDesc;
	struct MtpStringBuffer name, created, modified, keywords;
	time_t modifiedTime;

	if (mRequest->getParameterCount(mRequest) < 2)
		return MTP_RESPONSE_INVALID_PARAMETER;

	storageID = mRequest->getParameter(1, mRequest);
	mStorage = getStorage(storageID, mServer);
	if (!mStorage)
		return MTP_RESPONSE_INVALID_STORAGE_ID;
	parent = mRequest->getParameter(2, mRequest);

	memset(path, 0, sizeof(path));
	if (parent == MTP_PARENT_ROOT) {
		strcpy(path, MtpStorageGetPath(mStorage));
		//parent = 0;
	}else {
		int result;
		result = MtpDataBaseGetObjectFilePath(parent, path, pathLen, &length, &format, mDataBase);
		if (result != MTP_RESPONSE_OK)
			return result;
		if (format != MTP_FORMAT_ASSOCIATION)
			return MTP_RESPONSE_INVALID_PARENT_OBJECT;
	}

	/* Storage ID */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Object Format */
	if (!mData->getUint16(&temp16, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	format = temp16;
	/* Protectioon Status */
	if (!mData->getUint16(&temp16, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Object Compressed Size */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	mServer->mSendObjectFileSize = temp32;
	/* Thumb Format */
	if (!mData->getUint16(&temp16, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Thumb Compressed Size */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Thumb Pix Width */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Thumb Pix Height */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Image Pix Width */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Image Pix Height */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Image Bit Depth */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Parent Object */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Association Type */
	if (!mData->getUint16(&temp16, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	associationType = temp16;
	/* Association Description */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	associationDesc = temp32;
	/* Sequence Number */
	if (!mData->getUint32(&temp32, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Filename */
	if (!mData->getString(&name, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Date Created */
	if (!mData->getString(&created, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Date Modified */
	if (!mData->getString(&modified, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;
	/* Keywords */
	if (!mData->getString(&keywords, mData))
		return MTP_RESPONSE_INVALID_PARAMETER;

	DLOG("name: %s format: %04X", name.mBuffer, format);

	if (!parseDateTime(modified.mBuffer, &modifiedTime))
		modifiedTime = 0;
	if (path[strlen(path) - 1] != '/')
		path[strlen(path)] = '/';
	strncat(path, name.mBuffer, sizeof(path)-strlen(path)-1);

	if (mServer->mSendObjectFileSize > MtpStorageGetFreeSpace(mStorage))
		return MTP_RESPONSE_STORAGE_FULL;
	maxFileSize = MtpStorageGetMaxFileSize(mStorage);
	if (maxFileSize != 0) {
		if (mServer->mSendObjectFileSize > maxFileSize || mServer->mSendObjectFileSize == 0xFFFFFFFF)
			return MTP_RESPONSE_OBJECT_TOO_LARGE;
	}

	DLOG("path: %s parent: %u storageID: 0x%08x", path, parent, storageID);

	handle = MtpDataBaseBeginSendObject(path, format, parent,
					storageID, mServer->mSendObjectFileSize,
					modifiedTime, mDataBase);
	DLOG("handle: 0x%x", handle);
	if (handle == kInvalidObjectHandle)
		return MTP_RESPONSE_GENERAL_ERROR;

	if (format == MTP_FORMAT_ASSOCIATION) {
		int ret;
		mode_t mask = umask(0);
		ret = mkdir(path, mServer->mDirectoryPermission);
		umask(mask);
		if (ret && ret != -EEXIST)
			return MTP_RESPONSE_GENERAL_ERROR;
		chown(path, getuid(), mServer->mFileGroup);
		MtpDataBaseEndSendObject(path, handle, MTP_FORMAT_ASSOCIATION, MTP_RESPONSE_OK, mDataBase);
	} else {
		if (mServer->mSendObjectFilePath) {
			free_wrapper(mServer->mSendObjectFilePath);
			mServer->mSendObjectFilePath = NULL;
		}
		DLOG("new file: %s handle: %u", path, handle);
		mServer->mSendObjectFilePath = strdup_wrapper(path);
		mServer->mSendObjectHandle = handle;
		mServer->mSendObjectFormat = format;
	}

	mResponse->setParameter(1, storageID, mResponse);
	mResponse->setParameter(2, parent, mResponse);
	mResponse->setParameter(3, handle, mResponse);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doSendObject(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpObjectHandle handle;
	MtpResponseCode result = MTP_RESPONSE_OK;
	int ret, initialData;
	struct mtp_file_range mfr;
	mode_t mask;

	if (!hasStorage(mServer))
		return MTP_RESPONSE_GENERAL_ERROR;

	if (mServer->mSendObjectHandle == kInvalidObjectHandle) {
		DLOG("Expected SendObjectInfo before SendObject");
		result = MTP_RESPONSE_NO_VALID_OBJECT_INFO;
		goto done;
	}

	ret = mData->read(mServer->mFd, mData);
	if (ret < MTP_CONTAINER_HEADER_SIZE) {
		result = MTP_RESPONSE_GENERAL_ERROR;
		goto done;
	}

	initialData = ret - MTP_CONTAINER_HEADER_SIZE;

	mfr.fd = open(mServer->mSendObjectFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (mfr.fd < 0) {
		result = MTP_RESPONSE_GENERAL_ERROR;
		goto done;
	}
	fchown(mfr.fd, getuid(), mServer->mFileGroup);
	mask = umask(0);
	fchmod(mfr.fd, mServer->mFilePermission);
	umask(mask);

	if (initialData > 0)
		ret = write(mfr.fd, mData->getData(mData), initialData);
	if (ret < 0) {
		DLOG("failed to write initial data");
		result = MTP_RESPONSE_GENERAL_ERROR;
	} else {
		if (mServer->mSendObjectFileSize - initialData > 0) {
			mfr.offset = initialData;
			if (mServer->mSendObjectFileSize == 0xFFFFFFFF) {
				/* tell driver to read until it receives a short packet */
				mfr.length = 0xFFFFFFFF;
			} else {
				mfr.length = mServer->mSendObjectFileSize - initialData;
			}
			DLOG("receiving %s", mServer->mSendObjectFilePath);
			/* transfer the file */
			ret = ioctl(mServer->mFd, MTP_RECEIVE_FILE, (unsigned long)&mfr);
			DLOG("MTP_RECEIVE_FILE returned %d", ret);
		}
	}
	close(mfr.fd);

	if (ret < 0) {
		unlink(mServer->mSendObjectFilePath);
		if (errno == ECANCELED)
			result = MTP_RESPONSE_TRANSACTION_CANCELLED;
		else
			result = MTP_RESPONSE_GENERAL_ERROR;
	}
done:
	mData->reset(mData);
	DLOG("result=0x%x", result);
	MtpDataBaseEndSendObject(mServer->mSendObjectFilePath, mServer->mSendObjectHandle,
				mServer->mSendObjectFormat, result == MTP_RESPONSE_OK, mDataBase);
	mServer->mSendObjectHandle = kInvalidObjectHandle;
	mServer->mSendObjectFormat = 0;

	return result;
}

static MtpResponseCode doBeginEditObject(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpObjectHandle handle;
	int result, fd;
	char pathBuf[128];
	size_t pathBufLen = sizeof(pathBuf);
	uint64_t fileLength;
	MtpObjectFormat format;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	if (getEditObject(handle, &mServer->mObjectEditList)) {
		printf("object already open for edit in doBeginEditObject\n");
		return MTP_RESPONSE_GENERAL_ERROR;
	}

	result = MtpDataBaseGetObjectFilePath(handle, pathBuf, pathBufLen, &fileLength, &format, mDataBase);
	if (result != MTP_RESPONSE_OK)
		return result;
	fd = open((const char *)pathBuf, O_RDWR | O_EXCL);
	if (fd < 0) {
		printf("open failed for %s indoBeginEditObject. errno=%d\n",
				pathBuf, errno);
		return MTP_RESPONSE_GENERAL_ERROR;
	}

	addEditObject(handle, (const char *)pathBuf, fileLength, format, fd, &mServer->mObjectEditList);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doTruncateObject(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct ObjectEdit *edit;
	MtpObjectHandle handle;
	uint64_t offset, offset2;

	if (mRequest->getParameterCount(mRequest) < 3)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	edit = getEditObject(handle, &mServer->mObjectEditList);
	if (!edit) {
		printf("object not open for edit in doTruncateObject\n");
		return MTP_RESPONSE_GENERAL_ERROR;
	}

	offset = mRequest->getParameter(2, mRequest);
	offset2 = mRequest->getParameter(3, mRequest);
	offset |= (offset2 << 32);

	if (ftruncate(edit->mFd, offset) != 0) {
		return MTP_RESPONSE_GENERAL_ERROR;
	} else {
		edit->mSize = offset;
		return MTP_RESPONSE_OK;
	}
}

static MtpResponseCode doSendPartialObject(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mResponse = mServer->mResponsePacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct ObjectEdit *edit;
	MtpObjectHandle handle;
	uint64_t offset, offset2, end;
	uint32_t length;
	int ret, initialData;

	if (!hasStorage(mServer))
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
	if (mRequest->getParameterCount(mRequest) < 4)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	offset = mRequest->getParameter(2, mRequest);
	offset2 = mRequest->getParameter(3, mRequest);
	offset |= (offset2 << 32);
	length = mRequest->getParameter(4, mRequest);

	edit = getEditObject(handle, &mServer->mObjectEditList);
	if (!edit) {
		printf("object not open for edit in doSendPartialObject\n");
		return MTP_RESPONSE_GENERAL_ERROR;
	}

	if (offset > edit->mSize) {
		printf("writing past end of object, offset: %lu edit->mSize: %lu\n", offset, edit->mSize);
		return MTP_RESPONSE_GENERAL_ERROR;
	}
	DLOG("receiving partial %s offset:%lu length:%u", edit->mPath, offset, length);

	ret = mData->read(mServer->mFd, mData);
	if (ret < MTP_CONTAINER_HEADER_SIZE)
		return MTP_RESPONSE_GENERAL_ERROR;
	initialData = ret - MTP_CONTAINER_HEADER_SIZE;
	if (initialData > 0) {
		ret = pwrite(edit->mFd, mData->getData(mData), initialData, offset);
		offset += initialData;
		length -= initialData;
	}

	if (ret < 0) {
		printf("failed to write initial data\n");
	} else {
		if (length > 0) {
			struct mtp_file_range mfr;
			mfr.fd = edit->mFd;
			mfr.offset = offset;
			mfr.length = length;

			ret = ioctl(mServer->mFd, MTP_RECEIVE_FILE, (unsigned long)&mfr);
			DLOG("MTP_RECEIVE_FILE returned %d", ret);
		}
	}
	if (ret < 0) {
		mResponse->setParameter(1, 0, mResponse);
		if (errno == ECANCELED)
			return MTP_RESPONSE_TRANSACTION_CANCELLED;
		else
			return MTP_RESPONSE_GENERAL_ERROR;
	}
	mData->reset(mData);
	mResponse->setParameter(1, length, mResponse);
	end = offset + length;
	if (end > edit->mSize)
		edit->mSize = end;

	return MTP_RESPONSE_OK;
}


static MtpResponseCode doDeleteObject(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct ObjectEdit *edit;
	MtpObjectHandle handle;
	MtpObjectFormat format;
	char pathBuf[128];
	size_t pathBufLen = sizeof(pathBuf);
	uint64_t fileLength;
	int result;

	if (!hasStorage(mServer))
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);

	result = MtpDataBaseGetObjectFilePath(handle, pathBuf, pathBufLen, &fileLength, &format, mDataBase);
	if (result == MTP_RESPONSE_OK) {
		result = MtpDataBaseDeleteFile(handle, mDataBase);
		if (result == MTP_RESPONSE_OK)
			deletePath(pathBuf);
	}

	return result;
}

static MtpResponseCode doEndEditObject(struct MtpServer *mServer)
{
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct ObjectEdit *edit;
	MtpObjectHandle handle;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;
	handle = mRequest->getParameter(1, mRequest);
	edit = getEditObject(handle, &mServer->mObjectEditList);
	if (!edit) {
		printf("object not open for edit in doEndEditObject\n");
		return MTP_RESPONSE_GENERAL_ERROR;
	}

	commitEdit(edit, mDataBase);
	removeEditObject(handle, &mServer->mObjectEditList);
	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetPartialObject(MtpOperationCode operation, struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	uint64_t offset;
	uint32_t length;
	MtpObjectHandle handle;
	int result, ret;
	char pathBuf[128];
	size_t pathBufLen = sizeof(pathBuf);
	uint64_t fileLength;
	struct mtp_file_range mfr;
	MtpObjectFormat format;


	if (!hasStorage(mServer))
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
	handle = mRequest->getParameter(1, mRequest);
	offset = mRequest->getParameter(2, mRequest);
	if (operation == MTP_OPERATION_GET_PARTIAL_OBJECT_64) {
		uint64_t offset2;
		if (mRequest->getParameterCount(mRequest) < 4)
			return MTP_RESPONSE_INVALID_PARAMETER;
		offset2 = mRequest->getParameter(3, mRequest);
		offset = offset | (offset2<<32);
		length = mRequest->getParameter(4, mRequest);
	} else {
		if (mRequest->getParameterCount(mRequest) < 3)
			return MTP_RESPONSE_INVALID_PARAMETER;
		length = mRequest->getParameter(3, mRequest);
	}

	result = MtpDataBaseGetObjectFilePath(handle, pathBuf, pathBufLen, &fileLength, &format, mDataBase);
	if (result != MTP_RESPONSE_OK)
		return result;
	if (offset + length > fileLength)
		length = fileLength - offset;

	mfr.fd = open(pathBuf, O_RDONLY);
	if (mfr.fd < 0)
		return MTP_RESPONSE_GENERAL_ERROR;
	mfr.offset = offset;
	mfr.length = length;
	mfr.command = mRequest->getOperationCode(mRequest);
	mfr.transaction_id = mRequest->getTransactionID(mRequest);

	ret = ioctl(mServer->mFd, MTP_SEND_FILE_WITH_HEADER, (unsigned long)&mfr);
	DLOG("MTP_SEND_FILE_WITH_HEADER returned %d", ret);
	close(mfr.fd);
	if (ret < 0) {
		if (errno == ECANCELED)
			return MTP_RESPONSE_TRANSACTION_CANCELLED;
		else
			return MTP_RESPONSE_GENERAL_ERROR;
	}

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doSetObjectPropValue(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpObjectHandle handle;
	MtpObjectProperty property;

	if (!hasStorage(mServer))
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

	if (mRequest->getParameterCount(mRequest) < 2)
		return MTP_RESPONSE_INVALID_PARAMETER;

	handle = mRequest->getParameter(1, mRequest);
	property = mRequest->getParameter(2, mRequest);
	DLOG("SetObectPropValue %u %s", handle, getObjectPropCodeName(property));

	return MtpDataBaseSetObjectPropertyValue(handle, property, mData, mDataBase);
}

static MtpResponseCode doGetObjectPropDesc(struct MtpServer *mServer)
{
	MtpObjectProperty propCode;
	MtpObjectFormat format;
	struct MtpProperty *mProperty = NULL;
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;

	if (mRequest->getParameterCount(mRequest) < 2)
		return MTP_RESPONSE_INVALID_PARAMETER;

	propCode = mRequest->getParameter(1, mRequest);
	format = mRequest->getParameter(2, mRequest);
	DLOG("GetObjectPropDesc %s %s", getObjectPropCodeName(propCode),
					getFormatCodeName(format));
	mProperty = mDataBase->getObjectPropertyDesc(propCode, format);
	if (!mProperty)
		return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
	mProperty->write(mData, mProperty);

	MtpPropertyRelease(mProperty);
	return MTP_RESPONSE_OK;
}

static MtpResponseCode doGetDevicePropValue(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	MtpDeviceProperty property;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	property = mRequest->getParameter(1, mRequest);
	DLOG("GetDevicePropValue %s", getDevicePropCodeName(property));

	return mDataBase->getDevicePropertyValue(property, mData);
}

static MtpResponseCode doGetDevicePropDesc(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;
	struct MtpProperty *mProperty = NULL;
	MtpDeviceProperty propCode;

	if (mRequest->getParameterCount(mRequest) < 1)
		return MTP_RESPONSE_INVALID_PARAMETER;

	propCode = mRequest->getParameter(1, mRequest);
	DLOG("GetDevicePropDesc %s", getDevicePropCodeName(propCode));

	mProperty = mDataBase->getDevicePropertyDesc(propCode);
	if (!mProperty)
		return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
	mProperty->write(mData, mProperty);

	//TODO
	MtpPropertyRelease(mProperty);
	return MTP_RESPONSE_OK;
}

#define CMDLINE_SIZE 2048
static char* get_cmdline_val(const char* name, char* out, int len)
{
        char line[CMDLINE_SIZE + 1], *c, *sptr;
        int fd = open("/proc/cmdline", O_RDONLY);
        ssize_t r = read(fd, line, sizeof(line) - 1);
        close(fd);

        if (r <= 0)
                return NULL;

        line[r] = 0;

        for (c = strtok_r(line, " \t\n", &sptr); c;
                        c = strtok_r(NULL, " \t\n", &sptr)) {
                char *sep = strchr(c, '=');
                ssize_t klen = sep - c;
                if (klen < 0 || strncmp(name, c, klen) || name[klen] != 0)
                        continue;

                strncpy(out, &sep[1], len);
                out[len-1] = 0;
                return out;
        }

        return NULL;
}

static MtpResponseCode doGetDeviceInfo(struct MtpServer *mServer)
{
	char string[128];
	struct MtpPacket *mData = mServer->mDataPacket;
	struct MtpDataBase *mDataBase = mServer->mDataBase;

	/* fill in device info */
	mData->putData16(MTP_STANDARD_VERSION, mData);
	if (mServer->mPtp) {
		mData->putData32(0, mData);
	} else {
		/* MTP Vendor Extension ID */
		mData->putData32(6, mData);
	}
	mData->putData16(MTP_STANDARD_VERSION, mData);
	if (mServer->mPtp) {
		/* no extensions */
		strncpy(string, "", sizeof(string));
	} else {
		/* MTP extensions */
		strncpy(string, "microsoft.com: 1.0; android.com: 1.0;", sizeof(string));
	}
	mData->putString(string, mData);
	/* Functional Mode */
	mData->putData16(0, mData);

	/* Supported Operations */
	mData->putAData16(kSupportedOperationCodes,
			sizeof(kSupportedOperationCodes) / sizeof(uint16_t), mData);
	/* Supportes Events */
	mData->putAData16(kSupportedEventCodes,
			sizeof(kSupportedEventCodes) / sizeof(uint16_t), mData);
	/* Supported Devices Properties */
	mData->putAData16(mDataBase->mDeviceProperties->array,
			mDataBase->mDeviceProperties->size, mData);
	/* Supported Capture Formats */
	mData->putAData16(mDataBase->mCaptureFormats->array,
			mDataBase->mCaptureFormats->size, mData);
	/* Supported Playback Formats */
	mData->putAData16(mDataBase->mPlaybackFormats->array,
			mDataBase->mPlaybackFormats->size, mData);

	/* Manufacturer */
	mData->putString("Allwinner", mData);
	/* Model */
#ifdef MODEL_NAME
	mData->putString(MODEL_NAME, mData);
#else
	if (!gethostname(string, sizeof(string)))
		mData->putString(string, mData);
	else
		mData->putString("Tinalinux", mData);
#endif
	/* Device Version */
	mData->putString("1.0", mData);
	/* Serial Number */

	if (!get_cmdline_val("androidboot.serialno", string, sizeof(string))
		|| !strcmp(string, "<NULL>"))
		strcpy(string, "20080411");
	mData->putString(string, mData);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode doOpenSession(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mResponse = mServer->mResponsePacket;

	DLOG("");
	if (mServer->mSessionOpen) {
		DLOG("");
		mResponse->setParameter(1, mServer->mSessionID, mResponse);
		return MTP_RESPONSE_SESSION_ALREADY_OPEN;
	}
	if (mRequest->getParameterCount(mRequest) < 1) {
		DLOG("");
		return MTP_RESPONSE_INVALID_PARAMETER;
	}

	mServer->mSessionID = mRequest->getParameter(1, mRequest);

	//mDatabase->sessionStarted();
	DLOG("");
	mServer->mSessionOpen = true;
	DLOG("");

	return MTP_RESPONSE_OK;
}

static bool handleRequest(struct MtpServer *mServer)
{
	struct MtpPacket *mRequest = mServer->mRequestPacket;
	struct MtpPacket *mResponse = mServer->mResponsePacket;
	MtpResponseCode response;
	MtpOperationCode operation;
	int containertype;

	DLOG("");

	operation = mRequest->getOperationCode(mRequest);

	mResponse->reset(mResponse);

	containertype = mRequest->getContainerType(mRequest);
	if (containertype != MTP_CONTAINER_TYPE_COMMAND) {
		DLOG("wrong container type %d", containertype);
		return false;
	}
	DLOG("got command: %s (%x)", getOperationCodeName(operation), operation);

	switch(operation) {
		case MTP_OPERATION_GET_DEVICE_INFO:
			response = doGetDeviceInfo(mServer);
			break;
		case MTP_OPERATION_OPEN_SESSION:
			response = doOpenSession(mServer);
			break;
		case MTP_OPERATION_GET_STORAGE_IDS:
			response = doGetStorageIDs(mServer);
			break;
		case MTP_OPERATION_GET_STORAGE_INFO:
			response = doGetStorageInfo(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
			response = doGetObjectPropsSupported(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_HANDLES:
			response = doGetObjectHandles(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_REFERENCES:
			response = doGetObjectReferences(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_PROP_DESC:
			response = doGetObjectPropDesc(mServer);
			break;
		case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
			response = doSetObjectPropValue(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_PROP_LIST:
			response = doGetObjectPropList(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT_INFO:
			response = doGetObjectInfo(mServer);
			break;
		case MTP_OPERATION_GET_OBJECT:
			response = doGetObject(mServer);
			break;
		case MTP_OPERATION_GET_PARTIAL_OBJECT:
		case MTP_OPERATION_GET_PARTIAL_OBJECT_64:
			response = doGetPartialObject(operation, mServer);
			break;
		case MTP_OPERATION_SEND_OBJECT_INFO:
			response = doSendObjectInfo(mServer);
			break;
		case MTP_OPERATION_SEND_OBJECT:
			response = doSendObject(mServer);
			break;
		case MTP_OPERATION_BEGIN_EDIT_OBJECT:
			response = doBeginEditObject(mServer);
			break;
		case MTP_OPERATION_TRUNCATE_OBJECT:
			response = doTruncateObject(mServer);
			break;
		case MTP_OPERATION_SEND_PARTIAL_OBJECT:
			response = doSendPartialObject(mServer);
			break;
		case MTP_OPERATION_END_EDIT_OBJECT:
			response = doEndEditObject(mServer);
			break;
		case MTP_OPERATION_DELETE_OBJECT:
			response = doDeleteObject(mServer);
			break;
		case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
			response = doGetDevicePropValue(mServer);
			break;
		case MTP_OPERATION_GET_DEVICE_PROP_DESC:
			response = doGetDevicePropDesc(mServer);
			break;
		default:
			DLOG("TODO unknown operation!");
			break;
	}
	if (response == MTP_RESPONSE_TRANSACTION_CANCELLED)
		return false;
	mResponse->setResponseCode(response, mResponse);
	return true;
}



static void *MtpServerRun(void *arg)
{
	int fd;
	struct MtpServer *mServer = (struct MtpServer *)arg;
	struct MtpPacket *mRequest;
	struct MtpPacket *mResponse;
	struct MtpPacket *mData;

	if (!mServer)
		return NULL;
	DLOG("");
	fd = mServer->mFd;
	mRequest = mServer->mRequestPacket;
	mResponse = mServer->mResponsePacket;
	mData = mServer->mDataPacket;

	while(1) {
		int ret;
		MtpOperationCode operation;
		MtpTransactionID transaction;
		bool dataIn;

		DLOG("");
		ret = mRequest->read(fd, mRequest);
		if (ret < 0) {
			printf("request read returned %d, errno=%d\n", ret, errno);
			break;
		}

		DLOG("");
		operation = mRequest->getOperationCode(mRequest);
		transaction = mRequest->getTransactionID(mRequest);
		DLOG(" operation: %s (%04X)", getOperationCodeName(operation), operation);
		mRequest->dump(mRequest);

		dataIn = (operation == MTP_OPERATION_SEND_OBJECT_INFO
			    || operation == MTP_OPERATION_SET_OBJECT_REFERENCES
			    || operation == MTP_OPERATION_SET_OBJECT_PROP_VALUE
			    || operation == MTP_OPERATION_SET_DEVICE_PROP_VALUE);
		if (dataIn) {
			mData->read(fd, mData);
			if (ret < 0) {
				printf("data read returned %d, errno %d", ret, errno);
				if (errno = ECANCELED)
					continue;
				break;
			}
			DLOG("received data:");
			mData->dump(mData);
		} else {
			mData->reset(mData);
		}

		if (handleRequest(mServer)) {
			DLOG("hasData:%d", mData->hasData(mData) ? 1 : 0);
			if (!dataIn && mData->hasData(mData)) {
				mData->setOperationCode(operation, mData);
				mData->setTransactionID(transaction, mData);
				DLOG("sending data:");
				mData->dump(mData);
				ret = mData->write(fd, mData);
				if (ret < 0) {
					DLOG("request write returned %d, errno: %d", ret, errno);
					break;
				}
			}
			mResponse->setTransactionID(transaction, mResponse);
			DLOG("sending response %04X", mResponse->getResponseCode(mResponse));
			ret = mResponse->write(fd, mResponse);
			mResponse->dump(mResponse);
			if (ret < 0) {
				DLOG("request write returned %d, errno%d", ret, errno);
				break;
			}
		} else {
			DLOG("skipping response");
		}

	}
	return NULL;
}

static int run(struct MtpServer *mServer)
{
	int ret;
	pthread_t tid;

	ret = pthread_create(&tid, NULL, &MtpServerRun, (void *)mServer);
	if (ret) {
		printf("pthread_create failed\n");
		return -1;
	}
	mServer->mTid = tid;
	return 0;
}

void MtpServerAddStorage(struct MtpStorage *mStorage, struct MtpServer *mServer)
{
	VectorAdd(mStorage, &mServer->mStorageList);
	DLOG("add new Storage (id:%u)", mStorage->mStorageID);
}


struct MtpServer *MtpServerInit(int fileGroup, int filePerm, int directoryPerm)
{
	struct MtpServer *mServer;

	DLOG("");
	mServer = (struct MtpServer *)calloc_wrapper(1, sizeof(struct MtpServer));
	memset(mServer, 0, sizeof(struct MtpServer));

	mServer->mFd = open("/dev/mtp_usb", O_RDWR);
	if (mServer->mFd < 0) {
		fprintf(stderr, "open /dev/mtp_usb failed\n");
		free_wrapper(mServer);
		return NULL;
	}


	mServer->mSessionOpen = false;
	mServer->mFilePermission = filePerm;
	mServer->mDirectoryPermission = directoryPerm;

	mServer->mRequestPacket = MtpPacketInit(512, MTPREQUESTPACKET);
	mServer->mResponsePacket = MtpPacketInit(512, MTPRESPONSEPACKET);
	mServer->mDataPacket = MtpPacketInit(16384, MTPDATAPACKET);

	mServer->mDataBase = MtpDataBaseInit();

	mServer->run = run;

	mServer->mPtp = false;

	mServer->mSendObjectHandle = kInvalidObjectHandle;

	return mServer;
}

void MtpServerRelease(struct MtpServer *mServer)
{
	DLOG("");
	if (!mServer)
		return;
	if (mServer->mTid) {
		//pthread_kill(mServer->mTid, SIGTERM);
		pthread_cancel(mServer->mTid);
		pthread_join(mServer->mTid, NULL);
		mServer->mTid = 0;
	}
	if (mServer->mFd >= 0)
		close(mServer->mFd);
	if (mServer->mRequestPacket)
		MtpPacketRelease(mServer->mRequestPacket);
	if (mServer->mResponsePacket)
		MtpPacketRelease(mServer->mResponsePacket);
	if (mServer->mDataPacket)
		MtpPacketRelease(mServer->mDataPacket);
	DLOG("");
	if (mServer->mDataBase)
		MtpDataBaseRelease(mServer->mDataBase);
	if (mServer->mSendObjectFilePath)
		free_wrapper(mServer->mSendObjectFilePath);
	DLOG("");
	VectorDestroy(&mServer->mStorageList);
	DLOG("");
	//VectorDestroy(&mServer->mObjectEditList);
	VectorDestroyWithObject(mServer->mObjectEditList, struct ObjectEdit, ObjectEditRelease);
	DLOG("");
	free_wrapper(mServer);
	memleak_print();
	memleak_exit();
}
