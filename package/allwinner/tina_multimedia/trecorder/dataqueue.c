#include "dataqueue.h"
#include "TRlog.h"

struct SunxiMemOpsS *pQueueMemops = NULL;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

static int internal_modules_link(struct moduleAttach *module_1, struct moduleAttach *module_2)
{
    struct outputSrc *src_output = module_1->output;
    struct outputSrc *next_node = NULL;

    /* first check whether the input and output types of the two are the same */
    if((module_1->outputTyte & module_2->inputTyte) == 0){
        TRerr("[%s] src module(name 0x%x output type 0x%x) \
            and sink module(name 0x%x input type 0x%x) types do not match\n",
                            __func__, module_1->name, module_1->outputTyte,
                            module_2->name, module_2->inputTyte);

        return -1;
    }

    /* if src_output == NULL, create list head */
    if(!src_output){
        src_output = malloc(sizeof(struct outputSrc));
        if(!src_output)
            return -1;

        memset(src_output, 0x00, sizeof(struct outputSrc));

        /* updata queue */
        module_1->output = src_output;
    }

    /* move to the end of the linked list and check if it is already linked */
    next_node = src_output;
    while(next_node){
        if((next_node->srcQueue && module_2->sinkQueue)
            && (next_node->srcQueue == module_2->sinkQueue))
            return 0;

        src_output = next_node;
        next_node = next_node->next;
    }

    if(src_output->srcQueue){
        next_node = malloc(sizeof(struct outputSrc));
        if(!next_node)
            return -1;

        memset(next_node, 0x00, sizeof(struct outputSrc));

        src_output->next = next_node;
        src_output = src_output->next;
    }

    /* here, src_output is empty */

    /* sink create queue */
    if(!module_2->sinkDataPool || !module_2->sinkQueue){
        module_2->sinkDataPool = AwPoolCreate(NULL);
	module_2->sinkQueue = CdxQueueCreate(module_2->sinkDataPool);
    }

    src_output->SinkNotifyHdl = module_2->notifyHdl;
    src_output->SinkNotifyFunc = module_2->notifyFunc;

    /* record input name */
    module_2->src_name |= module_1->name;

    /* save sink input data type */
    src_output->sinkInputType = &module_2->inputTyte;
    src_output->srcQueue = module_2->sinkQueue;
    /* set ref */
    CdxAtomicSet(&module_2->packetCount, 0);
    src_output->packetCount = &(module_2->packetCount);
    module_2->moduleEnable = 0;
    src_output->moduleEnable = &(module_2->moduleEnable);

	return 0;
}

int modules_link(struct moduleAttach *module_1, struct moduleAttach *module_2, ...)
{
    int ret = -1;
    va_list args;
    struct moduleAttach *src_module,*sink_module;
    src_module = module_1;
    sink_module = module_2;

    va_start(args, module_2);

    while(sink_module){
        ret = internal_modules_link(src_module, sink_module);
        if( ret < 0)
            break;

        src_module = sink_module;
        sink_module = va_arg(args, struct moduleAttach *);
    }

    va_end(args);

	return ret;
}

static int internal_modules_unlink(struct moduleAttach *module_1, struct moduleAttach *module_2)
{
    struct outputSrc *src_output = module_1->output;
    struct outputSrc **list_head = &module_1->output;
    struct moduleAttach *output_module = module_1;
    struct moduleAttach *input_module = module_2;
    struct outputSrc *tmp_node = NULL;
    void *packet;

    if(!src_output || !input_module->sinkQueue)
        return -1;

    /* find input_module->sinkQueue from  output_module->output */
    if(src_output->srcQueue == input_module->sinkQueue){  /* single node */
        *list_head = src_output->next;
        tmp_node = src_output;
    }else{
        while(src_output){
            if(src_output->next->srcQueue == input_module->sinkQueue)
                break;
            src_output = src_output->next;
        }
        if(!src_output)
            return -1;
        /* here, src_output->next->srcQueue == input_module->sinkQueue */
        tmp_node = src_output->next;
        src_output->next = tmp_node->next;
    }

    /* tmp_node is the node we are looking for */
    tmp_node->srcQueue = NULL;
    tmp_node->next = NULL;
    free(tmp_node);
    tmp_node = NULL;

    /* delete output_module queue */
    input_module->src_name &= ~(output_module->name);
    if(input_module->src_name == 0){   /* no user */
        while(!CdxQueueEmpty(input_module->sinkQueue)){
            packet = module_pop(input_module);
            packetDestroy(packet);
        }

	CdxQueueDestroy(input_module->sinkQueue);
	AwPoolDestroy(input_module->sinkDataPool);

        input_module->sinkQueue = NULL;
        input_module->sinkDataPool = NULL;
    }

    return 0;
}

int modules_unlink(struct moduleAttach *module_1, struct moduleAttach *module_2, ...)
{
    int ret;
    va_list args;
    struct moduleAttach *src_module,*sink_module;
    src_module = module_1;
    sink_module = module_2;

    va_start(args, module_2);

    while(sink_module){
        if(internal_modules_unlink(src_module, sink_module) < 0){
            ret = -1;
            break;
        }

        src_module = sink_module;
        sink_module = va_arg(args, struct moduleAttach *);
    }

    va_end(args);

	return ret;
}

int module_detectLink(struct moduleAttach *module_1, struct moduleAttach *module_2)
{
    struct moduleAttach *srcModule = module_1;
    struct moduleAttach *sinkModule = module_2;

    if(!srcModule || !sinkModule)
        return 0;

    struct outputSrc *src_output = srcModule->output;

    while(src_output){
        if(src_output->srcQueue == sinkModule->sinkQueue){
            return 1;
        }

        src_output = src_output->next;
    }

    return 0;
}

int modulePort_SetEnable(struct moduleAttach *module, int enable)
{
    if(!module)
        return -1;

    pthread_rwlock_wrlock(&rwlock);
    module->moduleEnable = enable;
    pthread_rwlock_unlock(&rwlock);

    /* clear queue packet count */
    if(enable > 0)
        CdxAtomicSet(&module->packetCount, 0);

    return 0;
}

/* return push queue count */
int module_push(struct moduleAttach *module, struct modulePacket *mPacket)
{
     struct outputSrc *outputPort = module->output;
     enum packetType packet_type;  /* packet data type */
     int ref = 0;

     if((!outputPort) || (!outputPort->srcQueue))
         return -1;

    pthread_rwlock_rdlock(&rwlock);

    packet_type = mPacket->packet_type;

     while(outputPort && outputPort->srcQueue){
        /* module can only be pushed when the same type and sink enabled */
        if((*(outputPort->moduleEnable) > 0)
                && (*(outputPort->sinkInputType) & packet_type)){
            ref++;
            if(ref > 1)
                CdxAtomicInc(&mPacket->ref);
            CdxQueuePush(outputPort->srcQueue, mPacket);
            queueCountInc(outputPort->packetCount);
        }
         outputPort = outputPort->next;
     }

    /* prevent thread scheduling, so it's all push to notify sink */
     outputPort = module->output;
     while(outputPort && outputPort->srcQueue){
        if((*(outputPort->moduleEnable) > 0)
                && (*(outputPort->sinkInputType) & packet_type)){
            if(outputPort->SinkNotifyFunc && outputPort->SinkNotifyHdl)
                outputPort->SinkNotifyFunc(outputPort->SinkNotifyHdl);
        }
         outputPort = outputPort->next;
     }

     pthread_rwlock_unlock(&rwlock);

    return ref;
}

void *module_pop(struct moduleAttach *module)
{
    if(!module->sinkQueue || CdxQueueEmpty(module->sinkQueue))
        return NULL;

    queueCountDec(&module->packetCount);

    return CdxQueuePop(module->sinkQueue);
}

int module_InputQueueEmpty(struct moduleAttach *module)
{
    if(!module->sinkQueue)
        return 1;

    return CdxQueueEmpty(module->sinkQueue);
}

int NotifyCallbackToSink(void *handle)
{
    struct moduleAttach *module = (struct moduleAttach *)handle;

    if(!module)
        return -1;

    module_postReceiveSem(module);

    return 0;
}

int ModuleSetNotifyCallback(struct moduleAttach *module, NotifyCallbackForSinkModule notify, void *handle)
{
    if(!module || !notify || !handle)
        return -1;

    sem_init(&module->waitReceiveSem, 0, 0);
    sem_init(&module->waitReturnSem, 0, 0);

    module->notifyFunc = notify;
    module->notifyHdl = handle;

    return 0;
}

int alloc_VirPhyBuf(unsigned char **pVirBuf, unsigned char **pPhyBuf, int length)
{
    if(!pVirBuf || !pPhyBuf || (length <= 0))
        return -1;

    if(!pQueueMemops){
        pQueueMemops = GetMemAdapterOpsS();
        if(SunxiMemOpen(pQueueMemops) < 0){
            TRerr("[%s] SunxiMemOpen error\n", __func__);
            pQueueMemops = NULL;
            return -1;
        }
    }

    *pVirBuf = (unsigned char *)SunxiMemPalloc(pQueueMemops, length);
    if(*pVirBuf == NULL){
        TRerr("[%s] SunxiMemPalloc error\n", __func__);
        return -1;
    }
    *pPhyBuf = (unsigned char *)SunxiMemGetPhysicAddressCpu(pQueueMemops, *pVirBuf);
    if(*pPhyBuf == NULL){
        TRerr("[%s] SunxiMemGetPhysicAddressCpu error", __func__);
        SunxiMemPfree(pQueueMemops, *pVirBuf);
        return -1;
    }

    return 0;
}

void memFlushCache(void *pVirBuf, int nSize)
{
    if(!pVirBuf || nSize < 0 || !pQueueMemops)
        return;

    SunxiMemFlushCache(pQueueMemops, pVirBuf, nSize);
}

int free_VirPhyBuf(unsigned char *pVirBuf)
{
	if(!pVirBuf || !pQueueMemops)
		return -1;

    SunxiMemPfree(pQueueMemops, pVirBuf);

	return 0;
}

struct modulePacket *packetCreate(int bufSize)
{
    struct modulePacket *packet = NULL;

    packet = (struct modulePacket *)malloc(sizeof(struct modulePacket));
    if(!packet)
        return NULL;

    memset(packet, 0, sizeof(struct modulePacket));

    if(bufSize > 0){
        packet->buf = (void *)malloc(bufSize);
        if(packet->buf == NULL){
            free(packet);
            return NULL;
        }
        memset(packet->buf, 0, bufSize);
    }

    CdxAtomicSet(&packet->ref, 1);

    return packet;
}

int packetDestroy(struct modulePacket *mPacket)
{
    int ret = 0;

    if(!mPacket)
		return -1;

    if(mPacket->OnlyMemFlag != 1){
        if(mPacket->mode.notify.notifySrcHdl && mPacket->mode.notify.notifySrcFunc)
            mPacket->mode.notify.notifySrcFunc(mPacket->mode.notify.notifySrcHdl);
    }else{
        if(CdxAtomicDec(&mPacket->ref) == 0) {
            if(mPacket->mode.free.freeFunc)
                ret = mPacket->mode.free.freeFunc(mPacket);

            free(mPacket->buf);
            free(mPacket);
	}
    }

    return ret;
}

void module_waitReturnSem(void *handle)
{
    if(!handle)
        return;

    struct moduleAttach *module = (struct moduleAttach *)handle;

    sem_wait(&module->waitReturnSem);
}

void module_postReturnSem(void *handle)
{
    if(!handle)
        return;

    struct moduleAttach *module = (struct moduleAttach *)handle;

    sem_post(&module->waitReturnSem);
}

void module_waitReceiveSem(void *handle)
{
    if(!handle)
        return;

    struct moduleAttach *module = (struct moduleAttach *)handle;

    sem_wait(&module->waitReceiveSem);
}

void module_postReceiveSem(void *handle)
{
    if(!handle)
        return;

    struct moduleAttach *module = (struct moduleAttach *)handle;

    sem_post(&module->waitReceiveSem);
}
