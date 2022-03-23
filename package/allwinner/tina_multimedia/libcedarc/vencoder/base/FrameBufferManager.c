/*
 * Copyright (C) 2008-2015 Allwinner Technology Co. Ltd.
 * Author: Ning Fang <fangning@allwinnertech.com>
 *         Caoyuan Yang <yangcaoyuan@allwinnertech.com>
 *
 * This software is confidential and proprietary and may be used
 * only as expressly authorized by a licensing agreement from
 * Softwinner Products.
 *
 * The entire notice above must be reproduced on all copies
 * and should not be removed.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "vencoder.h"
#include "FrameBufferManager.h"
#include "EncAdapter.h"

//* should include other  '*.h'  before CdcMalloc.h
#include "CdcMalloc.h"


static void enqueue(VencInputBufferInfo** pp_head, VencInputBufferInfo* p)
{
    VencInputBufferInfo* cur;

    cur = *pp_head;

    if (cur == NULL)
    {
        *pp_head = p;
        p->next  = NULL;
        return;
    }
    else
    {
        while(cur->next != NULL)
            cur = cur->next;

        cur->next = p;
        p->next   = NULL;

        return;
    }
}

static VencInputBufferInfo* dequeue(VencInputBufferInfo** pp_head)
{
    VencInputBufferInfo* head;

    head = *pp_head;

    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        *pp_head = head->next;
        head->next = NULL;
        return head;
    }
}

FrameBufferManager* FrameBufferManagerCreate(int num, struct ScMemOpsS *memops,
                                                                    void *veOps, void *pVeopsSelf)
{
    FrameBufferManager* context = NULL;
    int i;

    context = (FrameBufferManager*)malloc(sizeof(FrameBufferManager));
    if (!context)
    {
        loge("malloc FrameBufferManager fail!");
        return NULL;
    }

    memset(context, 0, sizeof(FrameBufferManager));

    context->inputbuffer_list.max_size = num;
    context->inputbuffer_list.buffer_quene= (VencInputBufferInfo*) \
                                                calloc(sizeof(VencInputBufferInfo),
                                                num);

    if (!context->inputbuffer_list.buffer_quene)
    {
        loge("inputbuffer alloc quene buffer failed");
        free(context);
        return NULL;
    }

    memset(context->inputbuffer_list.buffer_quene, 0, sizeof(VencInputBufferInfo)*num);

    // all buffer enquene empty quene
    for (i = 0; i < (int)num; i++)
    {
        enqueue(&context->inputbuffer_list.empty_quene,&context->inputbuffer_list.buffer_quene[i]);
    }

    context->memops = memops;
    context->veops = veOps;
    context->pVeOpsSelf = pVeopsSelf;
    pthread_mutex_init(&context->inputbuffer_list.mutex, NULL);

    return context;
}

void FrameBufferManagerDestroy(FrameBufferManager* fbm)
{
    if (!fbm)
    {
        return;
    }
    struct ScMemOpsS *_memops = fbm->memops;
    void *veOps = fbm->veops;
    void *pVeopsSelf = fbm->pVeOpsSelf;

    if (fbm->inputbuffer_list.buffer_quene)
    {
        free(fbm->inputbuffer_list.buffer_quene);
        fbm->inputbuffer_list.buffer_quene = NULL;
    }

    if (fbm->ABM_inputbuffer.allocate_buffer)
    {
        int i;
        for (i = 0; i < (int)fbm->ABM_inputbuffer.buffer_num; i++)
        {
            if(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY)
            {
                EncAdapterMemPfree(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY);
            }
            if(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC)
            {
                EncAdapterMemPfree(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC);
            }
        }
        pthread_mutex_destroy(&fbm->ABM_inputbuffer.mutex);
        free(fbm->ABM_inputbuffer.allocate_buffer);
        fbm->ABM_inputbuffer.allocate_buffer = NULL;
    }

    pthread_mutex_destroy(&fbm->inputbuffer_list.mutex);

    free(fbm);
}

int AddInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    VencInputBufferInfo* input_buffer_info = NULL;

    if (!fbm)
    {
        loge("fbm is NULL, please check\n");
        return -1;
    }

    pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
    input_buffer_info = dequeue(&fbm->inputbuffer_list.empty_quene);
    pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);

    if (input_buffer_info != NULL)
    {
        memcpy(&input_buffer_info->inputbuffer, inputbuffer, sizeof(VencInputBuffer));

        pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
        enqueue(&fbm->inputbuffer_list.input_quene, input_buffer_info);
        fbm->added_buff_num++;
        pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
    }
    else
    {
        logw("AddInputBuffer fail, all input buffer is used by encoder\n");
        return -1;
    }

    return 0;
}

int GetInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    VencInputBufferInfo* input_buffer_info = NULL;

    if (!fbm)
    {
        return -1;
    }

    pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
    input_buffer_info = fbm->inputbuffer_list.input_quene;
    pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);

    if (input_buffer_info != NULL)
    {
        memcpy(inputbuffer, &input_buffer_info->inputbuffer, sizeof(VencInputBuffer));
    }
    else
    {
        return -1;
    }

    return 0;
}

int AddUsedInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    VencInputBufferInfo* input_buffer_info = NULL;

    if (!fbm)
    {
        return -1;
    }

    pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
    input_buffer_info = dequeue(&fbm->inputbuffer_list.input_quene);

    if (input_buffer_info != NULL)
    {

        if (inputbuffer->nID != input_buffer_info->inputbuffer.nID)
        {
            loge("AddUsedInputBuffer failed");
            pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
            return -1;
        }
        enqueue(&fbm->inputbuffer_list.used_quene, input_buffer_info);
        fbm->added_buff_num--;
    }
    else
    {
        pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
        return -1;
    }

    pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
    return 0;
}

int GetUsedInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    VencInputBufferInfo* input_buffer_info = NULL;

    if (!fbm)
    {
        return -1;
    }

    pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
    input_buffer_info = dequeue(&fbm->inputbuffer_list.used_quene);
    pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);

    if (input_buffer_info != NULL)
    {
        memcpy(inputbuffer, &input_buffer_info->inputbuffer, sizeof(VencInputBuffer));
        pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
        enqueue(&fbm->inputbuffer_list.empty_quene, input_buffer_info);
        pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
    }
    else
    {
        return -1;
    }

    return 0;
}

int AllocateInputBuffer(FrameBufferManager* fbm, VencAllocateBufferParam *buffer_param)
{
    int i= 0;
    if (!fbm)
    {
        return -1;
    }
    struct ScMemOpsS *_memops = fbm->memops;
    void *veOps = fbm->veops;
    void *pVeopsSelf = fbm->pVeOpsSelf;

    fbm->ABM_inputbuffer.buffer_num = buffer_param->nBufferNum;
    fbm->ABM_inputbuffer.allocate_buffer = (VencInputBufferInfo*) \
                                                calloc(sizeof(VencInputBufferInfo),
                                                buffer_param->nBufferNum);

    if (!fbm->ABM_inputbuffer.allocate_buffer)
    {
        loge("allocate_buffer error");
        return -1;
    }

    fbm->size_y = buffer_param->nSizeY;
    fbm->size_c = buffer_param->nSizeC;

    memset(fbm->ABM_inputbuffer.allocate_buffer, 0, \
       sizeof(VencInputBufferInfo)*buffer_param->nBufferNum);

    for (i = 0; i < (int)buffer_param->nBufferNum; i++)
    {
        fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.bAllocMemSelf = 1;
        fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.nID = i;
        fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY = \
                                (unsigned char *)EncAdapterMemPalloc(fbm->size_y);
        if (!fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY)
        {
            loge("ABM_inputbuffer Y alloc error");
            break;
        }

        fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrPhyY = \
            (unsigned char *)EncAdapterMemGetPhysicAddressCpu\
            (fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY);

        EncAdapterMemFlushCache(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY, \
            fbm->size_y);

        if (fbm->size_c > 0)
        {
            fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC = \
                (unsigned char *)EncAdapterMemPalloc((int)fbm->size_c);
            if (!fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC)
            {
                loge("ABM_inputbuffer C alloc error");
                break;
            }

            fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrPhyC = \
                (unsigned char *)EncAdapterMemGetPhysicAddressCpu\
                (fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC);
            EncAdapterMemFlushCache(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC, \
               fbm->size_c);
        }
    }

    if (i < (int)buffer_param->nBufferNum)
    {
        for (i=0; i<(int)buffer_param->nBufferNum; i++)
        {
            if (fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY)
            {
                EncAdapterMemPfree(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirY);
            }

            if (fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC)
            {
                EncAdapterMemPfree(fbm->ABM_inputbuffer.allocate_buffer[i].inputbuffer.pAddrVirC);
            }
        }

        free(fbm->ABM_inputbuffer.allocate_buffer);
        fbm->ABM_inputbuffer.allocate_buffer = NULL;
        return -1;
    }

    // all allocate buffer enquene
    for (i = 0; i < (int)buffer_param->nBufferNum; i++)
    {
        enqueue(&fbm->ABM_inputbuffer.allocate_queue,&fbm->ABM_inputbuffer.allocate_buffer[i]);
    }

    pthread_mutex_init(&fbm->ABM_inputbuffer.mutex, NULL);

    return 0;
}

int GetOneAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer* inputbuffer)
{
    VencInputBufferInfo* input_buffer_info = NULL;

    if (!fbm)
    {
        loge("fbm is NULL\n");
        return VENC_RESULT_NULL_PTR;
    }

    if (!fbm->ABM_inputbuffer.allocate_buffer)
    {
        loge("input allocate_buffer have not been allcated");
        return VENC_RESULT_NO_RESOURCE;
    }

    pthread_mutex_lock(&fbm->ABM_inputbuffer.mutex);
    input_buffer_info = dequeue(&fbm->ABM_inputbuffer.allocate_queue);
    fbm->added_buff_num++;
    pthread_mutex_unlock(&fbm->ABM_inputbuffer.mutex);

    if (input_buffer_info != NULL)
    {
        memcpy(inputbuffer, &input_buffer_info->inputbuffer, sizeof(VencInputBuffer));
    }
    else
    {
        loge("there is no empty AllocateInputBuffer");
        return VENC_RESULT_NO_FRAME_BUFFER;
    }

    return 0;
}

int FlushCacheAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    struct ScMemOpsS *_memops = fbm->memops;
    EncAdapterMemFlushCache(inputbuffer->pAddrVirY, fbm->size_y);
    if(fbm->size_c > 0)
    {
        EncAdapterMemFlushCache(inputbuffer->pAddrVirC, fbm->size_c);
    }
    return 0;
}

int ReturnOneAllocateInputBuffer(FrameBufferManager* fbm, VencInputBuffer *inputbuffer)
{
    int result = 0;

    VencInputBufferInfo* input_buffer_info = NULL;

    if (inputbuffer->nID >= (unsigned long)fbm->ABM_inputbuffer.buffer_num)
    {
        loge("this buffer isn't allocate buffer,can't return");
        return VENC_RESULT_ILLEGAL_PARAM;
    }

    input_buffer_info = &fbm->ABM_inputbuffer.allocate_buffer[inputbuffer->nID];

    pthread_mutex_lock(&fbm->ABM_inputbuffer.mutex);
    enqueue(&fbm->ABM_inputbuffer.allocate_queue, input_buffer_info);
    fbm->added_buff_num--;
    pthread_mutex_unlock(&fbm->ABM_inputbuffer.mutex);

    return result;
}

int ResetFrameBuffer(FrameBufferManager* fbm)
{
    int num, i;
    int result = 0;

    if (!fbm)
    {
        loge("ResetFrameBuffer error, the fbm pointer is null\n");
        return -1;
    }

    num = fbm->inputbuffer_list.max_size;
    if (num > 0)
    {
        pthread_mutex_lock(&fbm->inputbuffer_list.mutex);

        memset(fbm->inputbuffer_list.buffer_quene, 0, sizeof(VencInputBufferInfo)*num);
        fbm->inputbuffer_list.used_quene = NULL;
        fbm->inputbuffer_list.empty_quene = NULL;
        fbm->inputbuffer_list.input_quene = NULL;

        // all buffer enquene empty quene
        for(i=0; i<(int)num; i++)
        {
            enqueue(&fbm->inputbuffer_list.empty_quene,&fbm->inputbuffer_list.buffer_quene[i]);
        }

        pthread_mutex_unlock(&fbm->inputbuffer_list.mutex);
    }

    num = fbm->ABM_inputbuffer.buffer_num;
    if (num > 0)
    {
        pthread_mutex_lock(&fbm->ABM_inputbuffer.mutex);

        fbm->ABM_inputbuffer.allocate_queue = NULL;
        // all allocate buffer enquene
        for (i = 0; i < (int)num; i++)
        {
            enqueue(&fbm->ABM_inputbuffer.allocate_queue,&fbm->ABM_inputbuffer.allocate_buffer[i]);
        }

         pthread_mutex_unlock(&fbm->ABM_inputbuffer.mutex);
    }

    return result;
}

unsigned int GetUnencodedBufferNum(FrameBufferManager* fbm)
{
   unsigned int result = 0;
   if (fbm->inputbuffer_list.max_size)
   {
       pthread_mutex_lock(&fbm->inputbuffer_list.mutex);
       result = fbm->added_buff_num;
       pthread_mutex_unlock(&fbm->ABM_inputbuffer.mutex);
   }
   else if (fbm->ABM_inputbuffer.buffer_num)
   {
       pthread_mutex_lock(&fbm->ABM_inputbuffer.mutex);
       result = fbm->added_buff_num;
       pthread_mutex_unlock(&fbm->ABM_inputbuffer.mutex);
   }

   return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
