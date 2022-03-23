#define DEBUG
#include "MtpDataBase.h"
#include "MtpProperty.h"
#include "mtp.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

static uint16_t DeviceProperties[] = {
	MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER,
	MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,
	MTP_DEVICE_PROPERTY_IMAGE_SIZE,
	MTP_DEVICE_PROPERTY_BATTERY_LEVEL,
};

static MtpDevicePropertyList mDeviceProperties = {
	.array = DeviceProperties,
	.size = sizeof(DeviceProperties)/sizeof(uint16_t),
};

static MtpObjectFormatList mCaptureFormats = {
	.array = NULL,
	.size = 0,
};
#if 1
static uint16_t PlaybackFormats[] = {
	MTP_FORMAT_UNDEFINED,
	MTP_FORMAT_ASSOCIATION,
	MTP_FORMAT_TEXT,
	MTP_FORMAT_HTML,
	MTP_FORMAT_WAV,
	MTP_FORMAT_MP3,
	MTP_FORMAT_MPEG,
	MTP_FORMAT_EXIF_JPEG,
	MTP_FORMAT_TIFF_EP,
	MTP_FORMAT_BMP,
	MTP_FORMAT_GIF,
	MTP_FORMAT_JFIF,
	MTP_FORMAT_PNG,
	MTP_FORMAT_TIFF,
	MTP_FORMAT_WMA,
	MTP_FORMAT_OGG,
	MTP_FORMAT_AAC,
	MTP_FORMAT_MP4_CONTAINER,
	MTP_FORMAT_MP2,
	MTP_FORMAT_3GP_CONTAINER,
	MTP_FORMAT_ABSTRACT_AV_PLAYLIST,
	MTP_FORMAT_WPL_PLAYLIST,
	MTP_FORMAT_M3U_PLAYLIST,
	MTP_FORMAT_PLS_PLAYLIST,
	MTP_FORMAT_XML_DOCUMENT,
	MTP_FORMAT_FLAC,
};

static MtpObjectFormatList mPlaybackFormats = {
	.array = PlaybackFormats,
	.size = ARRAY_SIZE(PlaybackFormats),
};
#else
static uint16_t PlaybackFormats[] = {
	MTP_FORMAT_ASSOCIATION,
	MTP_FORMAT_TEXT,
};
static MtpObjectFormatList mPlaybackFormats = {
	.array = PlaybackFormats,
	.size = ARRAY_SIZE(PlaybackFormats),
};
#endif

typedef struct {
	MtpObjectProperty   property;
	int type;
} PropertyTableEntry;

static const PropertyTableEntry kObjectPropertyTable[] = {
    {   MTP_PROPERTY_STORAGE_ID,        MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_OBJECT_FORMAT,     MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_PROTECTION_STATUS, MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_OBJECT_SIZE,       MTP_TYPE_UINT64     },
    {   MTP_PROPERTY_OBJECT_FILE_NAME,  MTP_TYPE_STR        },
    {   MTP_PROPERTY_DATE_MODIFIED,     MTP_TYPE_STR        },
    {   MTP_PROPERTY_PARENT_OBJECT,     MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_PERSISTENT_UID,    MTP_TYPE_UINT128    },
    {   MTP_PROPERTY_NAME,              MTP_TYPE_STR        },
    {   MTP_PROPERTY_DISPLAY_NAME,      MTP_TYPE_STR        },
    {   MTP_PROPERTY_DATE_ADDED,        MTP_TYPE_STR        },
    {   MTP_PROPERTY_ARTIST,            MTP_TYPE_STR        },
    {   MTP_PROPERTY_ALBUM_NAME,        MTP_TYPE_STR        },
    {   MTP_PROPERTY_ALBUM_ARTIST,      MTP_TYPE_STR        },
    {   MTP_PROPERTY_TRACK,             MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_ORIGINAL_RELEASE_DATE, MTP_TYPE_STR    },
    {   MTP_PROPERTY_GENRE,             MTP_TYPE_STR        },
    {   MTP_PROPERTY_COMPOSER,          MTP_TYPE_STR        },
    {   MTP_PROPERTY_DURATION,          MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_DESCRIPTION,       MTP_TYPE_STR        },
    {   MTP_PROPERTY_AUDIO_WAVE_CODEC,  MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_BITRATE_TYPE,      MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_AUDIO_BITRATE,     MTP_TYPE_UINT32     },
    {   MTP_PROPERTY_NUMBER_OF_CHANNELS,MTP_TYPE_UINT16     },
    {   MTP_PROPERTY_SAMPLE_RATE,       MTP_TYPE_UINT32     },
};

static const PropertyTableEntry   kDevicePropertyTable[] = {
	{ MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER, MTP_TYPE_STR },
	{ MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,    MTP_TYPE_STR },
	{ MTP_DEVICE_PROPERTY_IMAGE_SIZE,              MTP_TYPE_STR },
	{ MTP_DEVICE_PROPERTY_BATTERY_LEVEL,           MTP_TYPE_UINT8 },
};

static int getDeviceProperty(int property, long *outIntValue, char *outStringValue)
{
	switch (property) {
		case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
		case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
			/* TODO */
			strcpy(outStringValue, "");
			break;
		case MTP_DEVICE_PROPERTY_IMAGE_SIZE:
			break;
		default:
			break;
	}
	return 0;
}

static bool getObjectPropertyInfo(MtpObjectProperty property, int *type)
{
	int i, count;
	const PropertyTableEntry *entry;

	count = sizeof(kObjectPropertyTable) / sizeof(kObjectPropertyTable[0]);
	entry = kObjectPropertyTable;
	for (i = 0; i < count; i++, entry++) {
		if (entry->property == property) {
			*type = entry->type;
			return true;
		}
	}
	return false;
}

static bool getDevicePropertyInfo(MtpDeviceProperty property, int *type)
{
	int count, i;
	const PropertyTableEntry *entry;

	count = sizeof(kDevicePropertyTable) / sizeof(kDevicePropertyTable[0]);
	entry = kDevicePropertyTable;
	for (i = 0; i < count; i++, entry++) {
		if (entry->property == property) {
			*type = entry->type;
			return true;
		}
	}
	return false;
}


static struct MtpProperty *getObjectPropertyDesc(MtpDeviceProperty property, MtpObjectFormat format)
{
	struct MtpProperty * result = NULL;

	switch (property) {
		case MTP_PROPERTY_OBJECT_FORMAT:
			result = MtpPropertyInit(property, MTP_TYPE_UINT16, false, format);
			break;
		case MTP_PROPERTY_OBJECT_SIZE:
			result = MtpPropertyInit(property, MTP_TYPE_UINT64, false, 0);
			break;
		case MTP_PROPERTY_PERSISTENT_UID:
			result = MtpPropertyInit(property, MTP_TYPE_UINT64, false, 0);
			break;
		case MTP_PROPERTY_OBJECT_FILE_NAME:
			result = MtpPropertyInit(property, MTP_TYPE_STR, true, 0);
			break;
		case MTP_PROPERTY_PROTECTION_STATUS:
			result = MtpPropertyInit(property, MTP_TYPE_UINT16, true, 0);
			break;
		case MTP_PROPERTY_STORAGE_ID:
		case MTP_PROPERTY_PARENT_OBJECT:
			result = MtpPropertyInit(property, MTP_TYPE_UINT32, true, 0);
			break;
		case MTP_PROPERTY_NAME:
		case MTP_PROPERTY_DISPLAY_NAME:
			result = MtpPropertyInit(property, MTP_TYPE_STR, true, 0);
			break;
		case MTP_PROPERTY_DATE_MODIFIED:
		case MTP_PROPERTY_DATE_ADDED:
		case MTP_PROPERTY_ORIGINAL_RELEASE_DATE:
			result = MtpPropertyInit(property, MTP_TYPE_STR, true, 0);
			//snprintf(mProperty->mDefaultValue.str, 64, "%s", );
			break;
		case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
			result = MtpPropertyInit(property, MTP_TYPE_STR, true, 0);
			break;
		default:
			DLOG("TODO unknown property!\n");
			return NULL;
	}
	return result;
}

static struct MtpProperty *getDevicePropertyDesc(MtpDeviceProperty property)
{
	bool writable = false;
	struct MtpProperty * result = NULL;

	switch (property) {
		case MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER:
		case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
			writable = true;
		case MTP_DEVICE_PROPERTY_IMAGE_SIZE: {
			result = MtpPropertyInit(property, MTP_TYPE_STR, writable, 0);
			break;
		case MTP_DEVICE_PROPERTY_BATTERY_LEVEL:
			result = MtpPropertyInit(property, MTP_TYPE_UINT8, false, 0);
			/* mBatteryScale */
			result->mMinimumValue.u.u8 = 0;
			result->mMaximumValue.u.u8 = 100;
			result->mStepSize.u.u8 = 1;
			/* mCurrentValue */
			result->mCurrentValue.u.u8 = 50;
			break;
		}
	}
	return result;
}
static MtpResponseCode getDevicePropertyValue(MtpDeviceProperty property, struct MtpPacket *mData)
{
	if (property == MTP_DEVICE_PROPERTY_BATTERY_LEVEL) {
		/* Battery Level */
		mData->putData8(50, mData);
		return MTP_RESPONSE_OK;
	} else {
		int type;

		if (!getDevicePropertyInfo(property, &type))
			return MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;

		//getDeviceProperty(property);
		/* TODO */
		mData->putString("", mData);
	}
	return MTP_RESPONSE_OK;
}

static MtpObjectHandle gHandle = 1;

static void MtpDataBaseBindObjectInfo(struct MtpStorage *mStorage, struct list_head *objectList)
{
	struct list_head *pos = NULL;
	struct list_head *head = &mStorage->mDisk->dirList;
	struct list_head *headFile = &mStorage->mDisk->fileList;
	struct Dir *dirRoot = mStorage->mDisk->dDirRoot;
	struct MtpObjectInfo *newObject = NULL;
	int i;

	DLOG("");
	newObject = MtpObjectInfoInitWithDir(MTP_PARENT_ROOT, dirRoot);
	newObject->fillObject(mStorage, newObject);
	newObject->mParent = 0;
	list_add(&newObject->mList, objectList);
#if 0
	list_for_each(pos, head) {
		struct Dir *dir = NULL;

		dir = list_entry(pos, struct Dir, list);
		DLOG("dirName: %s", dir->dirName);
	}
#endif
	list_for_each(pos, head) {
		struct Dir *dir = NULL;
		//DLOG("pos:%p, head:%p", pos, head);
		dir = list_entry(pos, struct Dir, list);
		DLOG("dirName: %s", dir->dirName);
		//sleep(1);
		newObject = MtpObjectInfoInitWithDir(gHandle++, dir);
		newObject->fillObject(mStorage, newObject);
		newObject->mParent = ((struct MtpObjectInfo *)dir->parentDir->object)->mHandle;
		list_add(&newObject->mList, objectList);
	}
	list_for_each(pos, headFile) {
		struct File *file = NULL;
		struct MtpObjectInfo *newObject = NULL;
		file = list_entry(pos, struct File, list);
		DLOG("fileName: %s", file->fileName);
		//sleep(1);
		newObject = MtpObjectInfoInitWithFile(gHandle++, file);
		newObject->fillObject(mStorage, newObject);
		newObject->mParent = ((struct MtpObjectInfo *)file->parentDir->object)->mHandle;
		list_add(&newObject->mList, objectList);
	}

	return;
}

void MtpDataBaseAddStorage(struct MtpStorage *mStorage, struct MtpDataBase *mDataBase)
{
	struct list_head *objectLists;

	VectorAdd(mStorage, &mDataBase->mStorageList);

	objectLists = calloc_wrapper(1, sizeof(struct list_head));
	VectorAdd(objectLists, &mDataBase->mRootObjectInfoList);
	INIT_LIST_HEAD(objectLists);

	MtpDataBaseBindObjectInfo(mStorage, objectLists);

	DLOG("add new Storage into MtpDataBase MtpObject");
}

static struct list_head *getRootObjectInfo(MtpStorageID id, struct MtpDataBase *mDataBase)
{
	int i;

	for (i = 0; i < VectorSize(&mDataBase->mStorageList); i++) {
		struct MtpStorage *mStorage = VectorObject(i, &mDataBase->mStorageList);
		if (mStorage->mStorageID == id) {
			DLOG("found storage ID:%u", id);
			return VectorObject(i, &mDataBase->mRootObjectInfoList);
		}
	}
	return NULL;
}

uint32_t *MtpDataBaseGetObjectHandlesList(MtpStorageID id,
					MtpObjectFormat format,
					MtpObjectHandle parent,
					size_t *arrayNum,
					struct MtpDataBase *mDataBase)
{
	struct list_head *objectLists = getRootObjectInfo(id, mDataBase);

	return getMtpObjectHandlesList(parent, arrayNum, objectLists);
}

static struct MtpObjectInfo *getObjectInfo(MtpObjectHandle handle, struct MtpDataBase *mDataBase)
{
	struct list_head *objectLists = NULL;
	struct MtpObjectInfo *info = NULL;
	int i, num = VectorSize(&mDataBase->mRootObjectInfoList);

	DLOG("search handle, num :%d", num);
	for (i = 0; i < num; i++) {
		objectLists = VectorObject(i, &mDataBase->mRootObjectInfoList);
		info = getMtpObjectByHandle(handle, objectLists);
		if (info != NULL) {
			return info;
		}

	}
	return NULL;
}

MtpResponseCode MtpDataBaseGetObjectInfo(MtpObjectHandle handle, struct MtpObjectInfo *object, struct MtpDataBase *mDataBase)
{
	struct MtpObjectInfo *info = NULL;

	info = getObjectInfo(handle, mDataBase);
	if (!info) {
		printf("getMtpObjectByHandle failed, handle:%u\n", handle);
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
	}
	*object = *info;
	return MTP_RESPONSE_OK;
}

MtpResponseCode MtpDataBaseGetObjectFilePath(MtpObjectHandle handle,
						char *pathBuf, size_t pathBufLen,
						uint64_t *fileLenth,
						MtpObjectFormat *format,
						struct MtpDataBase *mDataBase)
{
	struct MtpObjectInfo *info;

	info = getObjectInfo(handle, mDataBase);
	if (!info)
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

	*format = info->mFormat;
	if (info->mFormat == MTP_FORMAT_ASSOCIATION) {
		struct Dir *dir = info->object.dir;
		if (strlen(dir->path) + 1 > pathBufLen)
			return MTP_RESPONSE_GENERAL_ERROR;
		DLOG("dir path:%s", dir->path);
		strcpy(pathBuf, dir->path);
		*fileLenth = dir->dirSize;
	} else {
		struct File *file = info->object.file;
		if (strlen(file->path) + 1 > pathBufLen)
			return MTP_RESPONSE_GENERAL_ERROR;
		DLOG("file path:%s", file->path);
		strcpy(pathBuf, file->path);
		*fileLenth = file->fileSize;
	}

	return MTP_RESPONSE_OK;
}

static int MtpDataBaseDelete(MtpObjectHandle handle, struct list_head *objectLists)
{
	struct MtpObjectInfo *info = NULL;

	info = getMtpObjectByHandle(handle, objectLists);
	if (info != NULL) {
		DLOG("delete %s", info->mName);
		if (info->mFormat == MTP_FORMAT_ASSOCIATION) {
			uint32_t *array = NULL;
			size_t arrayNum, i;
			array = getMtpObjectHandlesList(handle, &arrayNum, objectLists);
			if (!array)
				return -1;
			for (i = 0; i < arrayNum; i++) {
				DLOG("recursion delete handle:%u", handle);
				if (MtpDataBaseDelete(array[i], objectLists) != 0) {
					free_wrapper(array);
					return -1;
				}
			}
			free_wrapper(array);
			DLOG("delete dir & object [%s]", info->object.dir->dirName);
			deleteDirInfo(info->object.dir);
			DLOG("");
			DirRelease(info->object.dir);
			DLOG("");
		} else {
			DLOG("delete file & object [%s]", info->object.file->fileName);
			deleteFileInfo(info->object.file);
			DLOG("");
			FileRelease(info->object.file);
			info->object.file = NULL;
			DLOG("");
		}
		DLOG("");
		deleteObjectInfo(info);
		DLOG("");
		MtpObjectInfoRelease(info);
		DLOG("");
		return 0;
	}
	return -1;
}

MtpResponseCode MtpDataBaseDeleteFile(MtpObjectHandle handle, struct MtpDataBase *mDataBase)
{
	struct list_head *objectLists = NULL;
	int i, num = VectorSize(&mDataBase->mRootObjectInfoList);

	for (i = 0; i < num; i++) {
		objectLists = VectorObject(i, &mDataBase->mRootObjectInfoList);
		if (!MtpDataBaseDelete(handle, objectLists))
			return MTP_RESPONSE_OK;
	}

	return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
}

void MtpDataBaseEndSendObject(const char *path, MtpObjectHandle handle,
			MtpObjectFormat format, bool succeeded,
			struct MtpDataBase *mDataBase)
{
	struct MtpObjectInfo *info;
	info = getObjectInfo(handle, mDataBase);
	if (!info)
		return ;
	if (succeeded) {
		if (info->mFormat == MTP_FORMAT_ASSOCIATION)
			updateDirInfo(info->object.dir);
		else
			updateFileInfo(info->object.file);
		updateObjectInfo(info);
	} else {
		/* TODO */
		DLOG("!!!!!!! fixme");
		if (info->mFormat == MTP_FORMAT_ASSOCIATION) {
			deleteDirInfo(info->object.dir);
			DirRelease(info->object.dir);
		} else {
			deleteFileInfo(info->object.file);
			FileRelease(info->object.file);
		}
		deleteObjectInfo(info);
		MtpObjectInfoRelease(info);
	}
}

static struct MtpStorage *getStorage(MtpStorageID id, struct MtpDataBase *mDataBase)
{
	int count = VectorSize(&mDataBase->mStorageList);
	int i;

	DLOG("count = %d id = %u", count, id);
	for (i = 0; i < count; i++) {
		struct MtpStorage *mStorage = VectorObject(i, &mDataBase->mStorageList);
		if (mStorage->mStorageID == id) {
			DLOG("found Storage. path:%s", MtpStorageGetPath(mStorage));
			return mStorage;
		}
	}
	return NULL;
}

static struct list_head *getObjectList(MtpStorageID id, struct MtpDataBase *mDataBase)
{
	int count = VectorSize(&mDataBase->mStorageList);
	int i;

	DLOG("count = %d id = %u", count, id);
	for (i = 0; i < count; i++) {
		struct MtpStorage *mStorage = VectorObject(i, &mDataBase->mStorageList);
		if (mStorage->mStorageID == id) {
			DLOG("found Storage. path:%s", MtpStorageGetPath(mStorage));
			return VectorObject(i, &mDataBase->mRootObjectInfoList);
		}
	}
	return NULL;
}

MtpObjectHandle MtpDataBaseBeginSendObject(const char *path, MtpObjectFormat format,
					MtpObjectHandle parent,
					MtpStorageID storageID,
					uint64_t size,
					time_t modified,
					struct MtpDataBase *mDataBase)
{
	struct MtpObjectInfo *info;
	struct MtpStorage *storage;
	struct list_head *objectList;

	DLOG("");
	storage = getStorage(storageID, mDataBase);
	if (!storage)
		return kInvalidObjectHandle;
	DLOG("");
	info = getObjectInfo(parent, mDataBase);
	if (!info)
		return kInvalidObjectHandle;
	DLOG("fount parent ObjectInfo:%s", info->mName);
	objectList = getObjectList(storageID, mDataBase);
	if (!objectList)
		return kInvalidObjectHandle;
	DLOG("");
	if (format == MTP_FORMAT_ASSOCIATION) {
		struct Dir *dir, *parentDir;

		DLOG("Dir & Object Init");
		parentDir = info->object.dir;
		dir = DirInit(path, parentDir);
		storage->mDisk->addSubDir(dir, storage->mDisk);

		info = MtpObjectInfoInitWithDir(gHandle++, dir);
		info->fillObject(storage, info);
		info->mParent = parent;
		list_add(&info->mList, objectList);
	} else {
		struct File *file;
		struct Dir *parentDir;

		DLOG("File & Object Init");
		parentDir = info->object.dir;
		file = FileInit(path, parentDir);
		storage->mDisk->addSubFile(file, storage->mDisk);
		info = MtpObjectInfoInitWithFile(gHandle++, file);

		info->fillObject(storage, info);
		info->mParent = parent;
		list_add(&info->mList, objectList);
	}

	return info->mHandle;
}

static void getNewFilePath(char *path, size_t pathLen, const char *name)
{
	char *ptr = NULL;
	size_t len = strlen(path);

	if (path[len - 1] == '/')
		path[len - 1] = 0;
	ptr = strrchr(path, '/');
	if (ptr != NULL)
		strcpy(++ptr, name);
	DLOG("new path: %s", path);
}

static int renameFile(MtpObjectHandle handle,
			const char *stringValue,
			struct MtpDataBase *mDataBase)
{
	struct MtpObjectInfo *info;

	info = getObjectInfo(handle, mDataBase);

	if (info->mFormat == MTP_FORMAT_ASSOCIATION) {
		struct Dir *dir = info->object.dir;
		char newPath[PATH_MAX];

		strcpy(newPath, dir->path);
		getNewFilePath(newPath, sizeof(newPath), stringValue);
		if (!dir->renameTo(newPath, dir))
			return MTP_RESPONSE_GENERAL_ERROR;

	} else {
		struct File *file = info->object.file;
		char newPath[PATH_MAX];

		strcpy(newPath, file->path);
		getNewFilePath(newPath, sizeof(newPath), stringValue);
		if (!file->renameTo(newPath, file))
			return MTP_RESPONSE_GENERAL_ERROR;

	}
	updateObjectInfo(info);

	return MTP_RESPONSE_OK;
}

static MtpResponseCode setObjectProperty(MtpObjectHandle handle,
					MtpObjectProperty property,
					const char *stringValue,
					struct MtpDataBase *mDataBase)
{
	int result = MTP_RESPONSE_OK;
	switch(property) {
		case MTP_PROPERTY_OBJECT_FILE_NAME:
			result = renameFile(handle, stringValue, mDataBase);
			break;
		case MTP_PROPERTY_NAME:
			/* TODO? */
			DLOG("--%s--", stringValue);
			result = MTP_RESPONSE_OK;
			break;
		default:
			DLOG("unknow property: 0x%x", property);
			return MTP_RESPONSE_INVALID_OBJECT_PROP_FORMAT;
	}
	return result;
}

MtpObjectHandle MtpDataBaseSetObjectPropertyValue(MtpObjectHandle handle,
						MtpObjectProperty property,
						struct MtpPacket *mData,
						struct MtpDataBase *mDataBase)
{
	int type;
	MtpObjectHandle result = MTP_RESPONSE_INVALID_OBJECT_PROP_FORMAT;

	if (!getObjectPropertyInfo(property, &type))
		return MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
	if (type == MTP_TYPE_STR) {
		struct MtpStringBuffer buffer;
		if (!mData->getString(&buffer, mData))
			goto fail;
		result = setObjectProperty(handle, property, buffer.mBuffer, mDataBase);
	} else {
		DLOG("!!!!! FIXME handle other type value");
	}
fail:
	return result;
}

struct MtpDataBase *MtpDataBaseInit()
{
	struct MtpDataBase *mDataBase;

	mDataBase = (struct MtpDataBase *)calloc_wrapper(1, sizeof(struct MtpDataBase));

	mDataBase->mDeviceProperties = &mDeviceProperties;
	mDataBase->mCaptureFormats = &mCaptureFormats;
	mDataBase->mPlaybackFormats = &mPlaybackFormats;

	mDataBase->getDevicePropertyDesc = getDevicePropertyDesc;
	mDataBase->getObjectPropertyDesc = getObjectPropertyDesc;
	mDataBase->getDevicePropertyValue = getDevicePropertyValue;

	return mDataBase;
}

void MtpDataBaseRelease(struct MtpDataBase *mDataBase)
{
	if (!mDataBase)
		return ;
	//VectorDestroyWithObject(mDataBase->mStorageList, struct MtpStorage, MtpStorageRelease);
	VectorDestroy(&mDataBase->mStorageList);
	//VectorDestroy(&mDataBase->mStorageList);
	VectorDestroyWithObject(mDataBase->mRootObjectInfoList, struct list_head, MtpObjectInfoListRelease);
	//VectorDestroy(&mDataBase->mRootObjectInfoList);
	free_wrapper(mDataBase);
}

static void deleteRecursive(const char *path)
{
	char pathBuf[PATH_MAX];
	size_t pathLength = strlen(path);
	char *fileSpot;
	int pathRemaining;
	DIR *dir;
	struct dirent *entry;

	DLOG("delete path:%s", path);
	if (pathLength >= sizeof(pathBuf) - 1) {
		printf("path too long: %s\n", path);
		exit(-1);
	}
	strcpy(pathBuf, path);
	if (pathBuf[pathLength - 1] != '/')
		pathBuf[pathLength++] = '/';

	fileSpot = pathBuf + pathLength;
	pathRemaining = sizeof(pathBuf) - pathLength - 1;
	dir = opendir(path);
	if (!dir) {
		printf("opendir %s failed: %s\n", path, strerror(errno));
		return;
	}
	while ((entry = readdir(dir))) {
		const char *name = entry->d_name;
		int nameLength;

		if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
			continue;
		DLOG("name: %s", name);
		nameLength = strlen(name);
		if (nameLength > pathRemaining) {
			printf("path too long: %s name:%s\n", path, name);
			exit(-1);
		}
		strcpy(fileSpot, name);
		if (entry->d_type == DT_DIR) {
			deleteRecursive(pathBuf);
			rmdir(pathBuf);
		} else {
			unlink(pathBuf);
		}
	}
	closedir(dir);
	return ;
}

void deletePath(const char *path)
{
	struct stat sb;
	if (stat(path, &sb) == 0) {
		if (S_ISDIR(sb.st_mode)) {
			deleteRecursive(path);
			rmdir(path);
		} else {
			unlink(path);
		}
	} else {
		DLOG("deletePath stat failed: %s: %s", path, strerror(errno));
	}
}

/* MtpToools Command */

static int filter(const struct dirent *dir_ent)
{
	if (strlen(dir_ent->d_name) == 1 && '.' == *dir_ent->d_name)
		return 0;
	if (strlen(dir_ent->d_name) == 2 && !strcmp(dir_ent->d_name, ".."))
		return 0;
	return 1;
}

static void MtpToolsCommandUpdateDir(struct Dir *dir, struct list_head *objectList, struct MtpStorage *storage)
{
	struct dirent **namelist = NULL;
	int num, i, j;
	char pathBuf[PATH_MAX];
	int parent = ((struct MtpObjectInfo *)dir->object)->mHandle;
	uint32_t *objectChild = NULL;
	size_t objectChildNum = 0, actualNum = 0;


	objectChild = getMtpObjectHandlesList(parent, &objectChildNum, objectList);
	num = scandir_wrapper(dir->path, &namelist, filter, alphasort);
	DLOG("objectChildNum = %u, scandir num = %d", objectChildNum, num);
	for (i = 0; i < num; i++) {
		if (strlen(namelist[i]->d_name) +
			strlen(dir->path) + 1 > PATH_MAX)	{
			printf("path too long : %s %s\n", namelist[i]->d_name, dir->path);
			exit(-1);
		}
		if (namelist[i]->d_type & DT_DIR) {
			struct MtpObjectInfo *objectInfo = NULL;
			objectInfo = getMtpObjectByNameWithParent(namelist[i]->d_name, parent, objectList);
			if (objectInfo != NULL)	 {
				updateDirInfo(objectInfo->object.dir);
				updateObjectInfo(objectInfo);
				actualNum++;
				for (j = 0; j < objectChildNum; j++) {
					if (objectInfo->mHandle == objectChild[j])
						objectChild[j] = 0;
				}
			} else {
				struct Dir *tmpDir = NULL;
				snprintf(pathBuf, sizeof(pathBuf), "%s/%s", dir->path, namelist[i]->d_name);
				DLOG("-------------------------------------------------------");
				tmpDir = DirInit(pathBuf, dir);
				storage->mDisk->addSubDir(tmpDir, storage->mDisk);

				objectInfo = MtpObjectInfoInitWithDir(gHandle++, tmpDir);
				objectInfo->fillObject(storage, objectInfo);
				objectInfo->mParent = parent;
				list_add(&objectInfo->mList, objectList);
			}
			MtpToolsCommandUpdateDir(objectInfo->object.dir, objectList, storage);
		} else if (namelist[i]->d_type & DT_REG) {
			struct MtpObjectInfo *objectInfo = NULL;
			objectInfo = getMtpObjectByNameWithParent(namelist[i]->d_name, parent, objectList);
			if (objectInfo != NULL)	 {
				updateFileInfo(objectInfo->object.file);
				updateObjectInfo(objectInfo);
				actualNum++;
				for (j = 0; j < objectChildNum; j++) {
					if (objectInfo->mHandle == objectChild[j])
						objectChild[j] = 0;
				}
			} else {
				struct File *tmpFile = NULL;
				snprintf(pathBuf, sizeof(pathBuf), "%s/%s", dir->path, namelist[i]->d_name);
				tmpFile = FileInit(pathBuf, dir);
				storage->mDisk->addSubFile(tmpFile, storage->mDisk);

				objectInfo = MtpObjectInfoInitWithFile(gHandle++, tmpFile);

				objectInfo->fillObject(storage, objectInfo);
				objectInfo->mParent = parent;
				list_add(&objectInfo->mList, objectList);
			}
		} else {
			printf("unknow dir type:0x%x [%s]\n", namelist[i]->d_type, namelist[i]->d_name);
			exit(-1);
		}
		free_wrapper(namelist[i]);
		namelist[i] = NULL;
	}
	if (namelist != NULL)
		free_wrapper(namelist);
	if (actualNum != objectChildNum) {
		int handle;
		DLOG("some dir or file has been delete.");
		for (j = 0; j < objectChildNum; j++) {
			if (objectChild[j] != 0) {
				DLOG("handle:%u should be deleted", objectChild[j]);
				MtpDataBaseDelete(objectChild[j], objectList);
			}
		}
	}


	if (objectChild != NULL)
		free_wrapper(objectChild);
	return ;
}

int MtpToolsCommandControl(mtp_tools_function_t control, const char *path, MtpObjectFormat format,
			int group, int permission,
			MtpStorageID storageID, struct MtpStorage *storage,
			struct MtpDataBase *mDataBase)
{
	int ret;
	char buf[PATH_MAX];
	char *name = NULL;
	size_t len;
	char *ptr = NULL;
	struct list_head *objectList;
	struct MtpObjectInfo *info, *objectInfo;
	MtpObjectHandle parent;

	len = strlen(path);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, path);

	objectList = getObjectList(storageID, mDataBase);
	if (!objectList)
		return -1;

	/* get parent Dir path */
	if (buf[len - 1] == '/')
		buf[len - 1] = '\0';

	/* is Root Dir? */
	if (control == MTP_TOOLS_FUNCTION_UPDATE &&
		!strcmp(storage->mFilePath, buf)) {
		MtpToolsCommandUpdateDir(storage->mDisk->dDirRoot, objectList, storage);
		return 0;
	}

	ptr = strrchr(buf, '/');
	if (ptr != NULL && ptr != buf) {
		name = ptr+1;
		*ptr = '\0';
	}

	info = getMtpObjectByPath(buf, objectList);
	if (!info) {
		DLOG("parent object can not found");
		return -1;
	}
	parent = info->mHandle;
	DLOG("fount parent ObjectInfo:%s", info->mName);
	objectInfo = getMtpObjectByNameWithParent(name, parent, objectList);
	if (objectInfo != NULL) {
		if (control == MTP_TOOLS_FUNCTION_UPDATE) {
			if (objectInfo->mFormat == MTP_FORMAT_ASSOCIATION)
				MtpToolsCommandUpdateDir(objectInfo->object.dir, objectList, storage);
			else
				MtpDataBaseEndSendObject(path, objectInfo->mHandle, objectInfo->mFormat, true, mDataBase);
		} else if (control == MTP_TOOLS_FUNCTION_REMOVE) {
			if (MtpDataBaseDeleteFile(objectInfo->mHandle, mDataBase) == MTP_RESPONSE_OK)
				deletePath(path);
			else {
				printf("delete handle:%u failed", objectInfo->mHandle);
			}
		} else {
			printf("control=%d, but %s already exist!\n", control, path);
			return -1;
		}
	} else {
		if (control == MTP_TOOLS_FUNCTION_ADD ||
			control == MTP_TOOLS_FUNCTION_UPDATE) {
			if (format == MTP_FORMAT_ASSOCIATION) {
				struct Dir *dir, *parentDir;

				DLOG("Dir & Object Init");
				parentDir = info->object.dir;
				dir = DirInit(path, parentDir);
				storage->mDisk->addSubDir(dir, storage->mDisk);

				info = MtpObjectInfoInitWithDir(gHandle++, dir);
				info->fillObject(storage, info);
				info->mParent = parent;
				list_add(&info->mList, objectList);
				chown(path, getuid(), group);
				MtpDataBaseEndSendObject(path, info->mHandle, format, true, mDataBase);
				if (control == MTP_TOOLS_FUNCTION_UPDATE)
					MtpToolsCommandUpdateDir(dir, objectList, storage);
			} else {
				struct File *file;
				struct Dir *parentDir;
				mode_t mask;

				if (access(path, F_OK) != 0)
					return -1;

				DLOG("File & Object Init");
				parentDir = info->object.dir;
				file = FileInit(path, parentDir);
				storage->mDisk->addSubFile(file, storage->mDisk);
				info = MtpObjectInfoInitWithFile(gHandle++, file);

				info->fillObject(storage, info);
				info->mParent = parent;
				list_add(&info->mList, objectList);

				chown(path, getuid(), group);
				mask = umask(0);
				chmod(path, permission);
				umask(mask);
				MtpDataBaseEndSendObject(path, info->mHandle, format, true, mDataBase);
			}
		} else {
			printf("control=%d, but %s isn't exist!\n", control, path);
			return -1;
		}
	}

	return 0;
}
