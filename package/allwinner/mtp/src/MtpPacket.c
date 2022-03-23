#define DEBUG
#include "MtpPacket.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static uint16_t getUInt16(uint8_t *buf, int offset)
{
	return ((uint16_t)buf[offset + 1] << 8) | (uint16_t)buf[offset];
}

static uint32_t getUInt32(uint8_t *buf, int offset)
{
	return ((uint32_t)buf[offset + 3] << 24) | ((uint32_t)buf[offset + 2] << 16) |
           ((uint32_t)buf[offset + 1] << 8)  | (uint32_t)buf[offset];
}

static void putUInt32(uint8_t *buf, int offset, uint32_t value)
{
	buf[offset++] = (uint8_t)(value & 0xFF);
	buf[offset++] = (uint8_t)((value >> 8) & 0xFF);
	buf[offset++] = (uint8_t)((value >> 16) & 0xFF);
	buf[offset++] = (uint8_t)((value >> 24) & 0xFF);
}

static void putUInt16(uint8_t *buf, int offset, uint16_t value)
{
	buf[offset++] = (uint8_t)(value & 0xFF);
	buf[offset++] = (uint8_t)((value >> 8) & 0xFF);
}

static uint16_t getContainerCode(uint8_t *buf)
{
	return getUInt16(buf, MTP_CONTAINER_CODE_OFFSET);
}

static void setContainerCode(uint8_t *buf, uint16_t code)
{
	putUInt16(buf, MTP_CONTAINER_CODE_OFFSET, code);
}

/* REQUEST */
static int MtpPacketRequestRead(int fd, struct MtpPacket *mPacket)
{
	int ret = read(fd, mPacket->mBuffer, mPacket->mBufferSize);
	if (ret < 0)
		return ret;
	if (ret >= MTP_CONTAINER_HEADER_SIZE
		&& ret <= MTP_CONTAINER_HEADER_SIZE + 5 * sizeof(uint32_t)
		&& ((ret - MTP_CONTAINER_HEADER_SIZE) & 3) == 0) {
		mPacket->mPacketSize = ret;
		mPacket->mParameterCount = (ret - MTP_CONTAINER_HEADER_SIZE) / sizeof(uint32_t);
	} else {
		DLOG("Malformed MTP request packet");
		ret = -1;
	}
	return ret;
}

static int MtpPacketRequestWrite(int fd, struct MtpPacket *mPacket)
{

}

static inline MtpOperationCode MtpPacketRequestgetOperationCode(struct MtpPacket *mPacket)
{
	return getContainerCode(mPacket->mBuffer);
}

static MtpTransactionID MtpPacketRequestgetTransactionID(struct MtpPacket *mPacket)
{
	return getUInt32(mPacket->mBuffer, MTP_CONTAINER_TRANSACTION_ID_OFFSET);
}

static uint16_t MtpPacketRequestgetContainerType(struct MtpPacket *mPacket)
{
	return getUInt16(mPacket->mBuffer, MTP_CONTAINER_TYPE_OFFSET);
}

static int MtpPacketRequestgetParameterCount(struct MtpPacket *mPacket)
{
	return mPacket->mParameterCount;
}

static uint32_t MtpPacketRequestgetParameter(int index, struct MtpPacket *mPacket)
{
	if (index < 1 || index > 5) {
		DLOG("index %d out of range in MtpPacket setParameter", index);
		return 0;
	}
	return getUInt32(mPacket->mBuffer, MTP_CONTAINER_PARAMETER_OFFSET + (index - 1) * sizeof(uint32_t));
}
/* DATA */
static int MtpPacketDataRead(int fd, struct MtpPacket *mPacket)
{
	int ret = read(fd, mPacket->mBuffer, mPacket->mBufferSize);
	if (ret < MTP_CONTAINER_HEADER_SIZE)
		return -1;

	mPacket->mPacketSize = ret;
	mPacket->mOffset = MTP_CONTAINER_HEADER_SIZE;

	return ret;
}

static int MtpPacketDataWrite(int fd, struct MtpPacket *mPacket)
{
	int ret;
	putUInt32(mPacket->mBuffer, MTP_CONTAINER_LENGTH_OFFSET, mPacket->mPacketSize);
	putUInt16(mPacket->mBuffer, MTP_CONTAINER_TYPE_OFFSET, MTP_CONTAINER_TYPE_DATA);
	ret = write(fd, mPacket->mBuffer, mPacket->mPacketSize);

	return (ret < 0 ? ret: 0);
}

static void MtpPacketDataReset(struct MtpPacket *mPacket)
{
	mPacket->mOffset = MTP_CONTAINER_HEADER_SIZE;
	mPacket->mPacketSize = MTP_CONTAINER_HEADER_SIZE;
	memset(mPacket->mBuffer, 0, mPacket->mBufferSize);
}

static bool MtpPacketDatahasData(struct MtpPacket *mPacket)
{
	return mPacket->mPacketSize > MTP_CONTAINER_HEADER_SIZE;
}

static void allocate(size_t length, struct MtpPacket *mPacket)
{
	if (length > mPacket->mBufferSize) {
		DLOG("mBuffer too small!!!!!");
	}
}

static void MtpPacketDataputData8(uint8_t value, struct MtpPacket *mPacket)
{
	allocate(mPacket->mOffset + 1, mPacket);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)value;
	if (mPacket->mPacketSize < mPacket->mOffset)
		mPacket->mPacketSize = mPacket->mOffset;
}

static void MtpPacketDataputData16(uint16_t value, struct MtpPacket *mPacket)
{
	allocate(mPacket->mOffset + 2, mPacket);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)(value & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 8) & 0xFF);
	if (mPacket->mPacketSize < mPacket->mOffset)
		mPacket->mPacketSize = mPacket->mOffset;
}

static void MtpPacketDataputData32(uint32_t value, struct MtpPacket *mPacket)
{
	allocate(mPacket->mOffset + 4, mPacket);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)(value & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 8) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 16) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 24) & 0xFF);
	if (mPacket->mPacketSize < mPacket->mOffset)
		mPacket->mPacketSize = mPacket->mOffset;
}

static void MtpPacketDataputData64(uint64_t value, struct MtpPacket *mPacket)
{
	allocate(mPacket->mOffset + 8, mPacket);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)(value & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 8) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 16) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 24) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 32) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 40) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 48) & 0xFF);
	mPacket->mBuffer[mPacket->mOffset++] = (uint8_t)((value >> 56) & 0xFF);
	if (mPacket->mPacketSize < mPacket->mOffset)
		mPacket->mPacketSize = mPacket->mOffset;
}

static void MtpPacketDataputData128(uint128_t value, struct MtpPacket *mPacket)
{
	MtpPacketDataputData32(value[0], mPacket);
	MtpPacketDataputData32(value[1], mPacket);
	MtpPacketDataputData32(value[2], mPacket);
	MtpPacketDataputData32(value[3], mPacket);
}

static void MtpPacketDataputAData16(const uint16_t *values, size_t size, struct MtpPacket *mPacket)
{
	size_t i;
	MtpPacketDataputData32(size, mPacket);
	for (i = 0; i < size; i++)
		MtpPacketDataputData16(values[i], mPacket);
}

static void MtpPacketDataputAData32(const uint32_t *values, size_t size, struct MtpPacket *mPacket)
{
	size_t i;
	MtpPacketDataputData32(size, mPacket);
	for (i = 0; i < size; i++)
		MtpPacketDataputData32(values[i], mPacket);

}

static size_t convertToUnicode(const char *string, uint16_t *unicode)
{
	size_t size = 0, count = strlen(string);
	int i;

	for (i = 0; i < count;) {
		uint16_t ch;
		uint16_t ch1 = *(string+i);
		i++;
		if ((ch1 & 0x80) == 0) {
			ch = ch1;
		} else if ((ch1 & 0xE0) == 0xC0) {
			uint16_t ch2 = *(string+i);
			i++;
			ch = ((ch1 & 0x1F) << 6) | (ch2 & 0x3F);
		} else {
			uint16_t ch2;
			uint16_t ch3;
			ch2 = *(string+i);
			i++;
			ch3 = *(string+i);
			i++;
			ch = ((ch1 & 0x0F) << 12) | ((ch2 & 0x3F) << 6) | (ch3 & 0x3F);
		}
		unicode[size++] = ch;
	}
	return size;
}

void MtpPacketDataputString(char *string, struct MtpPacket *mPacket)
{
	size_t count = strlen(string);
	uint16_t *unicode = NULL;
	int i;

	unicode = (uint16_t *)calloc_wrapper(1, count*sizeof(uint16_t));
	count = convertToUnicode(string, unicode);

	MtpPacketDataputData8(count > 0 ? count + 1 : 0, mPacket);

	for (i = 0; i < count; i++) {
		MtpPacketDataputData16(unicode[i], mPacket);
	}
	if (count > 0)
		MtpPacketDataputData16(0, mPacket);
	free_wrapper(unicode);

	return;
}

void MtpPacketDatasetOperationCode(MtpOperationCode code, struct MtpPacket *mPacket)
{
	putUInt16(mPacket->mBuffer, MTP_CONTAINER_CODE_OFFSET, code);
}

static uint8_t *MtpPacketDatagetData(struct MtpPacket *mPacket)
{
	return mPacket->mBuffer + MTP_CONTAINER_HEADER_SIZE;
#if 0
	int length = mPacket->mPacketSize - MTP_CONTAINER_HEADER_SIZE;
	if (length > 0) {
		void * result = malloc_wrapper(length);
		if (result) {
			memcpy(result, mPacket->mBuffer + MTP_CONTAINER_HEADER_SIZE, length);
			outLength = length;
			return result;
		}
	}
	outLength = 0;
	return NULL;
#endif
}

static bool MtpPacketDatagetUint32(uint32_t *value, struct MtpPacket *mPacket)
{
	size_t offset = mPacket->mOffset;
	if (mPacket->mPacketSize - offset < sizeof(uint32_t))
		return false;
	*value = (uint32_t)mPacket->mBuffer[offset] | ((uint32_t)mPacket->mBuffer[offset + 1] << 8) |
		((uint32_t)mPacket->mBuffer[offset + 2] << 16) | ((uint32_t)mPacket->mBuffer[offset + 3] << 24);

	mPacket->mOffset += sizeof(uint32_t);
	return true;
}

static bool MtpPacketDatagetUint16(uint16_t *value, struct MtpPacket *mPacket)
{
	size_t offset = mPacket->mOffset;
	if (mPacket->mPacketSize - offset < sizeof(uint16_t))
		return false;
	*value = (uint16_t)mPacket->mBuffer[offset] | ((uint16_t)mPacket->mBuffer[offset + 1] << 8);

	mPacket->mOffset += sizeof(uint16_t);
	return true;
}

static bool MtpPacketDatagetUint8(uint8_t *value, struct MtpPacket *mPacket)
{
	if (mPacket->mPacketSize - mPacket->mOffset < sizeof(uint8_t))
		return false;
	*value = mPacket->mBuffer[mPacket->mOffset++];

	return true;
}
static bool MtpPacketDatagetString(struct MtpStringBuffer *string, struct MtpPacket *mPacket)
{
	uint8_t count;
	uint8_t i;
	if (!mPacket->getUint8(&count, mPacket))
		return false;

	char *dest = string->mBuffer;
	for (i = 0; i < count; i++) {
		uint16_t ch;
		if (!mPacket->getUint16(&ch, mPacket))
			return false;
		if (ch >= 0x0800) {
			*dest++ = (uint8_t)(0xE0 | (ch >> 12));
			*dest++ = (uint8_t)(0x80 | (ch >> 6) & 0x3F);
			*dest++ = (uint8_t)(0x80 | (ch & 0x3F));
		} else if (ch >= 0x80) {
			*dest++ = (uint8_t)(0xC0 | (ch >> 6));
			*dest++ = (uint8_t)(0x80 | (ch & 0x3F));
		} else {
			*dest++ = ch;
		}
	}
	*dest++ = 0;
	string->mCharCount = count;
	string->mByteCount = dest - (char *)string->mBuffer;
	return true;
}

/* RESPONSE */
static int MtpPacketResponseWrite(int fd, struct MtpPacket *mPacket)
{
	int ret;
	putUInt32(mPacket->mBuffer, MTP_CONTAINER_LENGTH_OFFSET, mPacket->mPacketSize);
	putUInt16(mPacket->mBuffer, MTP_CONTAINER_TYPE_OFFSET, MTP_CONTAINER_TYPE_RESPONSE);
	ret = write(fd, mPacket->mBuffer, mPacket->mPacketSize);

	return (ret < 0 ? ret: 0);
}

static void MtpPacketResponseReset(struct MtpPacket *mPacket)
{
	mPacket->mOffset = MTP_CONTAINER_HEADER_SIZE;
	mPacket->mPacketSize = MTP_CONTAINER_HEADER_SIZE;
	memset(mPacket->mBuffer, 0, mPacket->mBufferSize);
}

static void MtpPacketResponsesetParameter(int index, uint32_t value, struct MtpPacket *mPacket)
{
	int offset;
	if (index < 1 || index > 5) {
		DLOG("index %d out of range in MtpPacket setParameter", index);
		return ;
	}
	offset = MTP_CONTAINER_PARAMETER_OFFSET + (index - 1) * sizeof(uint32_t);
	if (mPacket->mPacketSize < offset + sizeof(uint32_t))
		mPacket->mPacketSize = offset + sizeof(uint32_t);
	putUInt32(mPacket->mBuffer, offset, value);
}

static void MtpPacketResponsesetResponseCode(MtpResponseCode code, struct MtpPacket *mPacket)
{
	return setContainerCode(mPacket->mBuffer, code);
}

static MtpResponseCode MtpPacketResponsegetResponseCode(struct MtpPacket *mPacket)
{
	return getContainerCode(mPacket->mBuffer);
}

/* common MtpPacket */
static void MtpPacketsetTransactionID(MtpTransactionID id, struct MtpPacket *mPacket)
{
	putUInt32(mPacket->mBuffer, MTP_CONTAINER_TRANSACTION_ID_OFFSET, id);
}

static void MtpPacketdump(struct MtpPacket *mPacket)
{
#define DUMP_BYTES_PER_ROW 16
	char buffer[500];
	char *bufptr = buffer;
	int i;

	for (i = 0; i < mPacket->mPacketSize; i++) {
		sprintf(bufptr, "%02X ", mPacket->mBuffer[i]);
		bufptr += strlen(bufptr);
		if (i % DUMP_BYTES_PER_ROW == (DUMP_BYTES_PER_ROW - 1)) {
			DLOG("%s", buffer);
			bufptr = buffer;
		}
	}
	if (bufptr != buffer) {
		DLOG("%s", buffer);
	}
	DLOG("");
}

struct MtpPacket *MtpPacketInit(int bufferSize, int type)
{
	struct MtpPacket *mPacket;

	mPacket = (struct MtpPacket *)calloc_wrapper(1, sizeof(struct MtpPacket));
	mPacket->mBufferSize = bufferSize;
	mPacket->mBuffer = calloc_wrapper(1, mPacket->mBufferSize);
	mPacket->type = type;

	switch(type) {
	case MTPREQUESTPACKET:
		mPacket->read = MtpPacketRequestRead;
		mPacket->write = MtpPacketRequestWrite;
		mPacket->getOperationCode = MtpPacketRequestgetOperationCode;
		mPacket->getTransactionID = MtpPacketRequestgetTransactionID;
		mPacket->getContainerType = MtpPacketRequestgetContainerType;
		mPacket->getParameter = MtpPacketRequestgetParameter;
		mPacket->getParameterCount = MtpPacketRequestgetParameterCount;
		mPacket->dump = MtpPacketdump;
		break;
	case MTPRESPONSEPACKET:
		//mPacket->read = MtpPacketResponseRead;
		mPacket->write = MtpPacketResponseWrite;
		mPacket->reset = MtpPacketResponseReset;
		mPacket->setParameter = MtpPacketResponsesetParameter;
		mPacket->setResponseCode = MtpPacketResponsesetResponseCode;
		mPacket->getResponseCode = MtpPacketResponsegetResponseCode;
		mPacket->setTransactionID = MtpPacketsetTransactionID;
		mPacket->dump = MtpPacketdump;
		break;
	case MTPDATAPACKET:
		mPacket->read = MtpPacketDataRead;
		mPacket->write = MtpPacketDataWrite;
		mPacket->reset = MtpPacketDataReset;
		mPacket->hasData = MtpPacketDatahasData;
		mPacket->putData8 = MtpPacketDataputData8;
		mPacket->putData16 = MtpPacketDataputData16;
		mPacket->putData32 = MtpPacketDataputData32;
		mPacket->putData64 = MtpPacketDataputData64;
		mPacket->putData128 = MtpPacketDataputData128;
		mPacket->putString = MtpPacketDataputString;
		mPacket->putAData16 = MtpPacketDataputAData16;
		mPacket->putAData32 = MtpPacketDataputAData32;
		mPacket->setOperationCode = MtpPacketDatasetOperationCode;
		mPacket->setTransactionID = MtpPacketsetTransactionID;
		mPacket->dump = MtpPacketdump;
		mPacket->getData = MtpPacketDatagetData;
		mPacket->getUint32 = MtpPacketDatagetUint32;
		mPacket->getUint16 = MtpPacketDatagetUint16;
		mPacket->getUint8 = MtpPacketDatagetUint8;
		mPacket->getString = MtpPacketDatagetString;
		break;
	}


	return mPacket;
}

void MtpPacketRelease(struct MtpPacket *mPacket)
{
	if (mPacket->mBuffer) {
		free_wrapper(mPacket->mBuffer);
		mPacket->mBuffer = NULL;
	}
	free_wrapper(mPacket);
}
