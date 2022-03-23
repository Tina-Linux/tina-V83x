#ifndef _MTPPACKET_H
#define _MTPPACKET_H
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "mtp.h"
#include "MtpCommon.h"

enum {
	MTPREQUESTPACKET = 0,
	MTPRESPONSEPACKET,
	MTPDATAPACKET,
};

struct MtpPacket {
	uint32_t type;
	uint8_t *mBuffer;
	size_t mBufferSize;
	size_t mPacketSize;
	int (*read)(int fd, struct MtpPacket *mPacket);
	int (*write)(int fd, struct MtpPacket *mPacket);
	void (*reset)(struct MtpPacket *mPacket);
	MtpOperationCode (*getOperationCode)(struct MtpPacket *mPacket);
	void (*setOperationCode)(MtpOperationCode code, struct MtpPacket *mPacket);
	MtpTransactionID (*getTransactionID)(struct MtpPacket *mPacket);
	uint16_t (*getContainerType)(struct MtpPacket *mPacket);
	void (*setParameter)(int index, uint32_t value, struct MtpPacket *mPacket);
	int (*getParameterCount)(struct MtpPacket *mPacket);
	uint32_t (*getParameter)(int index, struct MtpPacket *mPacket);
	void (*setResponseCode)(MtpResponseCode code, struct MtpPacket *mPacket);
	MtpResponseCode (*getResponseCode)(struct MtpPacket *mPacket);
	bool (*hasData)(struct MtpPacket *mPacket);
	void (*setTransactionID)(MtpTransactionID id, struct MtpPacket *mPacket);
	void (*dump)(struct MtpPacket *mPacket);
	uint8_t *(*getData)(struct MtpPacket *mPacket);


	void (*putData8)(uint8_t value, struct MtpPacket *mPacket);
	void (*putData16)(uint16_t value, struct MtpPacket *mPacket);
	void (*putData32)(uint32_t value, struct MtpPacket *mPacket);
	void (*putData64)(uint64_t value, struct MtpPacket *mPacket);
	void (*putData128)(uint128_t value, struct MtpPacket *mPacket);
	void (*putString)(char *string, struct MtpPacket *mPacket);
	void (*putAData16)(const uint16_t *values, size_t size, struct MtpPacket *mPacket);
	void (*putAData32)(const uint32_t *values, size_t size, struct MtpPacket *mPacket);
	bool (*getUint8)(uint8_t *value, struct MtpPacket *mPacket);
	bool (*getUint16)(uint16_t *value, struct MtpPacket *mPacket);
	bool (*getUint32)(uint32_t *value, struct MtpPacket *mPacket);
	bool (*getString)(struct MtpStringBuffer *string, struct MtpPacket *mPacket);

	int mParameterCount;
	size_t mOffset;

#if 0
	union {
		struct RequestPacketOps {
			int (*read)(int fd, struct MtpPacket *mPacket);
			int (*write)(int fd, struct MtpPacket *mPacket);
		};
		struct ResponsePacketOps {
			int (*read)(int fd, struct MtpPacket *mPacket);
			int (*write)(int fd, struct MtpPacket *mPacket);
		};
		struct DataPacketOps {
			int (*read)(int fd, struct MtpPacket *mPacket);
			int (*write)(int fd, struct MtpPacket *mPacket);
		};
	};
#endif
};

struct MtpPacket *MtpPacketInit(int bufferSize, int type);
void MtpPacketRelease(struct MtpPacket *mPacket);

#endif
