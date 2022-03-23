/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_os.h
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

#ifndef __MS2M_HAL_OS_H__
#define __MS2M_HAL_OS_H__

#include "stdint.h"

#include "pthread.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "semaphore.h"

#include "ms2m_common.h"

typedef pthread_t* ms2m_hal_os_thread_t;
typedef int* ms2m_hal_os_queue_t;
typedef pthread_mutex_t* ms2m_hal_os_mutex_t;
typedef sem_t* ms2m_hal_os_semaphore_t;
typedef int* ms2m_hal_os_timer_t;

MS2M_STATUS ms2m_hal_os_thread_name_get(char *name);

MS2M_STATUS ms2m_hal_os_thread_create(ms2m_hal_os_thread_t *thandle,
                                    const char *name,
                                    void (*main_func) (void *arg),
                                    void *arg,
                                    uint32_t stack_depth,
                                    int32_t priority);

MS2M_STATUS ms2m_hal_os_queue_create(ms2m_hal_os_queue_t *qhandle,
                                     const char *name,
                                     int32_t msgsize,
                                     int32_t queue_length);
MS2M_STATUS ms2m_hal_os_queue_send(ms2m_hal_os_queue_t *qhandle,
                                   const void *msg,
                                   uint32_t msecs);

MS2M_STATUS ms2m_hal_os_queue_recv(ms2m_hal_os_queue_t *qhandle,
                                 void *msg,
                                 uint32_t msecs);

MS2M_STATUS ms2m_hal_os_mutex_creat(ms2m_hal_os_mutex_t *mutex);
MS2M_STATUS ms2m_hal_os_mutex_delete(ms2m_hal_os_mutex_t mutex);

void  ms2m_hal_os_mutex_lock(ms2m_hal_os_mutex_t mutex);
void  ms2m_hal_os_mutex_unlock(ms2m_hal_os_mutex_t mutex);

#endif /* __MS2M_HAL_OS_H__ */
