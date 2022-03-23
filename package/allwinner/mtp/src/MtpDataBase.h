#ifndef _MTPDATABASE_H
#define _MTPDATABASE_H

#include "MtpTypes.h"
#include "MtpPacket.h"
#include "MtpObjectInfo.h"
#include "MtpStorage.h"
#include "MtpCommon.h"


struct MtpDataBase {

	MtpDevicePropertyList *mDeviceProperties;
	MtpObjectFormatList *mCaptureFormats;
	MtpObjectFormatList *mPlaybackFormats;

	Vector mStorageList;

	Vector mRootObjectInfoList;

	struct MtpProperty *(*getDevicePropertyDesc)(MtpDeviceProperty property);
	struct MtpProperty *(*getObjectPropertyDesc)(MtpDeviceProperty property, MtpObjectFormat format);
	MtpResponseCode (*getDevicePropertyValue)(MtpDeviceProperty property, struct MtpPacket *mData);

};

struct MtpDataBase *MtpDataBaseInit();
void MtpDataBaseRelease(struct MtpDataBase *mDataBase);
MtpResponseCode MtpDataBaseGetObjectInfo(MtpObjectHandle handle, struct MtpObjectInfo *object, struct MtpDataBase *mDataBase);
void MtpDataBaseAddStorage(struct MtpStorage *mStorage, struct MtpDataBase *mDataBase);
uint32_t *MtpDataBaseGetObjectHandlesList(MtpStorageID id,
					MtpObjectFormat format,
					MtpObjectHandle parent,
					size_t *arrayNum,
					struct MtpDataBase *mDataBase);
MtpResponseCode MtpDataBaseGetObjectFilePath(MtpObjectHandle handle,
						char *pathBuf, size_t pathBufLen,
						uint64_t *fileLenth,
						MtpObjectFormat *format,
						struct MtpDataBase *mDataBase);
void MtpDataBaseEndSendObject(const char *path, MtpObjectHandle handle,
			MtpObjectFormat format, bool succeeded,
			struct MtpDataBase *mDataBase);
MtpResponseCode MtpDataBaseDeleteFile(MtpObjectHandle handle, struct MtpDataBase *mDataBase);

MtpObjectHandle MtpDataBaseBeginSendObject(const char *path, MtpObjectFormat format,
					MtpObjectHandle parent,
					MtpStorageID storage,
					uint64_t size,
					time_t modified,
					struct MtpDataBase *mDataBase);
MtpObjectHandle MtpDataBaseSetObjectPropertyValue(MtpObjectHandle handle,
						MtpObjectProperty property,
						struct MtpPacket *mData,
						struct MtpDataBase *mDataBas);
typedef enum {
	MTP_TOOLS_FUNCTION_ADD = 0,
	MTP_TOOLS_FUNCTION_REMOVE,
	MTP_TOOLS_FUNCTION_UPDATE,
	MTP_TOOLS_FUNCTION_CUT,
	MTP_TOOLS_FUNCTION_COPY,
} mtp_tools_function_t;

void deletePath(const char *path);
int MtpToolsCommandControl(mtp_tools_function_t contrl, const char *path, MtpObjectFormat format,
			int group, int permission,
			MtpStorageID storageID, struct MtpStorage *storage,
			struct MtpDataBase *mDataBase);
#endif
