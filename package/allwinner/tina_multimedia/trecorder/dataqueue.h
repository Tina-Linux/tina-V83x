#ifndef _DATAQUEUE_H
#define _DATAQUEUE_H

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <CdxQueue.h>
#include <AwPool.h>
#include <typedef.h>
#include <ion_mem_alloc.h>
#include "CdxMuxer.h"
#include <linux/videodev2.h>
#include "vdecoder.h"
#include "CdxAtomic.h"


enum moduleName{
    CAMERA      = 0x01,
    AUDIO       = 0x02,
    DECODER     = 0x04,
    ENCODER     = 0x08,
    SCREEN      = 0x10,
    MUXER       = 0x20,
    CUSTOM      = 0x40,
};

enum packetType{
    VIDEO_RAW   = 0x01,
    VIDEO_YUV   = 0x02,
    AUDIO_PCM   = 0x04,
    VIDEO_ENC   = 0x08,
    AUDIO_ENC   = 0x10,
    SCALE_YUV   = 0x20,
    CUSTOM_TYPE = 0X80,
};

typedef int (*NotifyCallbackForSinkModule)(void *handle);
typedef void (*SinkNotifySrcModule)(void *handle);
typedef int (*freePacketCallBack)(void *packet);

struct freeGroup{
    void *freePacketHdl;        /* free packet callback handle */
    freePacketCallBack freeFunc; /* free packet callback function  */
};

struct notifyGroup{
    void *notifySrcHdl;
    SinkNotifySrcModule notifySrcFunc;
};

struct outputSrc{
    CdxQueueT *srcQueue;        /* next module data source */
    unsigned int *sinkInputType;/* next module supported input data type */
    cdx_atomic_t *packetCount;  /* next module queue data packet count(type:signed long) */
    unsigned int *moduleEnable;  /* next module enable flag */
    NotifyCallbackForSinkModule SinkNotifyFunc;
    void *SinkNotifyHdl;
    struct outputSrc *next;
};

struct moduleAttach{
	struct outputSrc *output;   /* next module data source */
    AwPoolT *sinkDataPool;      /* self module data pool */
    CdxQueueT *sinkQueue;       /* self module data queue */
    unsigned int name;          /* sele module name */
    unsigned int src_name;      /* input data module name */
    unsigned int inputTyte;     /* supported input data type */
    unsigned int outputTyte;     /* supported output data type */
    unsigned int moduleEnable;  /* self module enable flag */
    cdx_atomic_t packetCount;   /* queue data packet count(type:signed long) */
    void *freePacketHdl;        /* free packet callback handle */
    freePacketCallBack freeFunc; /* free packet callback function  */
    sem_t waitReceiveSem;
    sem_t waitReturnSem;
    NotifyCallbackForSinkModule notifyFunc;
    void *notifyHdl;
};

struct modulePacket{
    enum packetType packet_type;  /* packet data type */
    cdx_atomic_t ref;              /* the number of packet references */
    int OnlyMemFlag;
    union{
        struct freeGroup free;
        struct notifyGroup notify;
    }mode;
    void *buf;
    void *reserved;             /* reserved for special use, the default is zero */
};

/* media packet */
struct MediaPacket{
	unsigned int buf_index;
    unsigned int width;
    unsigned int height;
    unsigned char *Vir_Y_addr;
    unsigned char *Vir_C_addr;
    unsigned char *Phy_Y_addr;
    unsigned char *Phy_C_addr;
    int data_len;
    int bytes_used;
	int format;
    long long nPts;
	int fps;
};

struct AudioPacket{
    char   *pData;            //data buff
    int             nLen;            //data len
    long long       nPts;            //pts
};

int modules_link(struct moduleAttach *module_1, struct moduleAttach *module_2, ...);
int modules_unlink(struct moduleAttach *module_1, struct moduleAttach *module_2, ...);
int modulePort_SetEnable(struct moduleAttach *module, int enable);
int module_detectLink(struct moduleAttach *module_1, struct moduleAttach *module_2);
int module_push(struct moduleAttach *module, struct modulePacket *mPacket);
void *module_pop(struct moduleAttach *module);
int module_InputQueueEmpty(struct moduleAttach *module);
int NotifyCallbackToSink(void *handle);
int ModuleSetNotifyCallback(struct moduleAttach *module, NotifyCallbackForSinkModule notify, void *handle);
int alloc_VirPhyBuf(unsigned char **pVirBuf, unsigned char **pPhyBuf, int length);
void memFlushCache(void *pVirBuf, int nSize);
int free_VirPhyBuf(unsigned char *pVirBuf);
struct modulePacket *packetCreate(int bufSize);
int packetDestroy(struct modulePacket *mPacket);

static inline int module_NoInputPort(struct moduleAttach *module)
{
    return (!module->sinkQueue) ? 1 : 0;
}

static inline int module_NoOutputPort(struct moduleAttach *module)
{
    return (!module->output) ? 1 : 0;
}

void module_waitReturnSem(void *handle);
void module_postReturnSem(void *handle);
void module_waitReceiveSem(void *handle);
void module_postReceiveSem(void *handle);

static inline int queueCountRead(struct moduleAttach *module)
{
    return (int)CdxAtomicRead(&module->packetCount);
}

static inline int queueCountInc(cdx_atomic_t *ref)
{
    return (int)CdxAtomicInc(ref);
}

static inline int queueCountDec(cdx_atomic_t *ref)
{
    return (int)CdxAtomicDec(ref);
}

static inline int packetRefRead(struct modulePacket *mPacket)
{
    return (int)CdxAtomicRead(&mPacket->ref);
}

static inline int packetRefInc(struct modulePacket *mPacket)
{
    return (int)CdxAtomicInc(&mPacket->ref);
}

static inline int packetRefDec(struct modulePacket *mPacket)
{
    return (int)CdxAtomicDec(&mPacket->ref);
}


#endif
