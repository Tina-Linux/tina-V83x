/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_os.c
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/11
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "sys/prctl.h"

#include "ms2m_hal_chip.h"
#include "ms2m_hal_os.h"
#include "ms_common.h"

typedef struct ms2m_hal_os_thread_arg_e
{
    const char *name;
    void *arg;
    void (*main_func) (void *arg);
}ms2m_hal_os_thread_arg_t;

void *_new_thread(void *arg)
{
    ms2m_hal_os_thread_arg_t data;

    memcpy(&data, arg, sizeof(ms2m_hal_os_thread_arg_t));
    free(arg);

    if(data.name != NULL)
    {
        prctl(PR_SET_NAME, data.name);
    }
    data.main_func(data.arg);
    return NULL;
}

typedef struct ms2m_queue_msg_s
{
    uint8_t msg_id;
    uint8_t cmd;
    uint8_t src_queue;
    uint8_t dst_queue;
    uint8_t payload_type;
    uint16_t payload_len;
    void *payload;
}ms2m_queue_msg_t;

typedef struct ms_hal_msgbuf_s
{
   long mtype;
   ms2m_queue_msg_t mq;
}ms_hal_msgbuf_t;

//---------------------------------------------------------------------------//

MS2M_STATUS ms2m_hal_os_thread_create(ms2m_hal_os_thread_t *thandle,
                                      const char *name,
                                      void (*main_func) (void *arg),
                                      void *arg,
                                      uint32_t stack_depth,
                                      int32_t priority)
{
    pthread_attr_t attr;
    struct sched_param param;
    ms2m_hal_os_thread_arg_t *p = NULL;

    if(0 != pthread_attr_init(&attr))
    {
        return M_ERROR;
    }

    if(0 != pthread_attr_getschedparam(&attr, &param))
    {
        return M_ERROR;
    }
    param.sched_priority = priority;
    if(0 != pthread_attr_setschedparam(&attr, &param))
    {
        return M_ERROR;
    }

    if(0 != pthread_attr_setstacksize(&attr, stack_depth))
    {
        return M_ERROR;
    }

    *thandle = malloc(sizeof(pthread_t));
    if(*thandle == NULL)
    {
        return M_ERROR;
    }
    memset(*thandle,0,sizeof(ms2m_hal_os_thread_arg_t));

    p = malloc(sizeof(ms2m_hal_os_thread_arg_t));
    if(p == NULL)
    {
        free(*thandle);
        return M_ERROR;
    }
    memset(p,0,sizeof(ms2m_hal_os_thread_arg_t));
    p->arg = arg;
    p->main_func = main_func;
    p->name = name;

    if (0 == pthread_create(*thandle, &attr, _new_thread, (void *)p))
    {
        return M_OK;
    }

    free(*thandle);
    free(p);
    return M_ERROR;
}


MS2M_STATUS ms2m_hal_os_queue_create(ms2m_hal_os_queue_t *qhandle,
                                     const char *name,
                                     int32_t msgsize,
                                     int32_t queue_length)
{
    //MS2M_STATUS msRet = M_ERROR;
    static int id = 1;
    struct msqid_ds msg_info;
    key_t key = ftok(".",++id);
    int msid = msgget(key,IPC_CREAT|0666);
    if(msid<0)
    {
        return M_ERROR;
    }

    *qhandle = malloc(sizeof(int));
    memcpy(*qhandle, &msid, sizeof(int));

    if(msgsize > 0 && queue_length > 0)
    {
        msgctl(msid,IPC_STAT,&msg_info);
        msg_info.msg_qbytes = (unsigned short)(msgsize * queue_length);
        //msg_info.msg_qnum = (unsigned short)queue_length;
        if(0 != msgctl(msid,IPC_SET,&msg_info))
        {
            free(*qhandle);
            *qhandle = NULL;
            MS_ERR_TRACE("msgctl set error\r\n");
            return M_ERROR;
        }
    }

    return M_OK;
}

MS2M_STATUS ms2m_hal_os_queue_send(ms2m_hal_os_queue_t *qhandle,
                                   const void *msg,
                                   uint32_t msecs)
{
    MS2M_STATUS msRet = M_ERROR;
    int i = 0;
    ms_hal_msgbuf_t msgbuf;
    memset(&msgbuf,0,sizeof(ms_hal_msgbuf_t));
    int flag = 0;

    if(msecs == 0)
    {
        flag = IPC_NOWAIT;
    }

    msgbuf.mtype = 1;
    memcpy(&(msgbuf.mq),msg,sizeof(ms2m_queue_msg_t));
    do
    {
        if(0 == msgsnd(**qhandle, &msgbuf, sizeof(ms2m_queue_msg_t), flag))
        {
            msRet = M_OK;
            break;
        }
        else
        {
            ms2m_hal_msleep(1);
            if(errno == EAGAIN)
            {
                continue;
            }
            //	ms_hal_sys_reset(); debugbyyip
        }
        i++;
    }while(i <= msecs);

//printf("ZZZZZ-1:%d:%d\r\n",msRet,errno);
    return msRet;
}

MS2M_STATUS ms2m_hal_os_queue_recv(ms2m_hal_os_queue_t *qhandle,
                                   void *msg,
                                   uint32_t msecs)
{
    int i = 0;
    MS2M_STATUS msRet = M_ERROR;
    ms_hal_msgbuf_t msgbuf;
    memset(&msgbuf,0,sizeof(ms_hal_msgbuf_t));
    int flag = 0;

    if(msecs == 0)
    {
        flag = IPC_NOWAIT;
    }

    do
    {
        if(-1 != msgrcv(**qhandle, &msgbuf,sizeof(ms2m_queue_msg_t),0,flag))
        {
            memcpy(msg, &(msgbuf.mq), sizeof(ms2m_queue_msg_t));
            msRet = M_OK;
            break;
        }
        else
        {
            ms2m_hal_msleep(1);
        }
        i++;
    }while(i < msecs);

    return msRet;
}

MS2M_STATUS ms2m_hal_os_mutex_creat(ms2m_hal_os_mutex_t *mutex)
{
    *mutex = malloc(sizeof(pthread_mutex_t));
    if(*mutex == NULL)
    {
        return M_NO_MEM;
    }
    pthread_mutex_init(*mutex, NULL);
    return M_OK;
}

MS2M_STATUS ms2m_hal_os_mutex_delete(ms2m_hal_os_mutex_t mutex)
{
    if(mutex == NULL)
    {
        return M_OK;
    }
    int ret = pthread_mutex_destroy(mutex);
    if(ret != 0)
    {
        return M_ERROR;
    }
    free(mutex);
    //mutex = NULL;
    return M_OK;
}

void ms2m_hal_os_mutex_lock(ms2m_hal_os_mutex_t mutex)
{
    pthread_mutex_lock(mutex);
}

void ms2m_hal_os_mutex_unlock(ms2m_hal_os_mutex_t mutex)
{
    pthread_mutex_unlock(mutex);
}

MS2M_STATUS ms2m_hal_os_thread_name_get(char *name)
{
    if(name == NULL)
    {
        return M_ERROR;
    }
    prctl(PR_GET_NAME,name);
    return M_OK;
}
