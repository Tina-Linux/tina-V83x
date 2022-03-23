/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_entry.c
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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pthread.h"

#include "ms2m_hal_config.h"
#include "ms2m_hal_uart.h"
#include "ms2m_demo_main.h"

#define _ENTRY_STACK_SIZE MS2M_EII_STACK_SIZE
#define _ENTRY_PRIORITY MS2M_EII_PRIORITY

int main(int argc, char *argv[])
{
#if 1
    if(argc >=2)
    {
        ms2m_hal_uart_port_set(argv[1]);
    }
    ms2m_main();
#else
    int ret;
    pthread_t main_thread;

    pthread_attr_t attr;
    struct sched_param param;

    if(0 != pthread_attr_init(&attr))
    {
        return -1;
    }

    if(0 != pthread_attr_getschedparam(&attr, &param))
    {
        return M_ERROR;
    }
    param.sched_priority = _ENTRY_PRIORITY;
    if(0 != pthread_attr_setschedparam(&attr, &param))
    {
        return M_ERROR;
    }
    if(0 != pthread_attr_setstacksize(&attr, _ENTRY_STACK_SIZE))
    {
        return M_ERROR;
    }
    ret = pthread_create(&main_thread,&attr,(void*)ms2m_main,NULL);
    if(ret == -1)
    {
        printf("create pthread error!\n");
        return -1;
    }
    pthread_join(main_thread, NULL);

#endif
    return 0;
}
