/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_config.h
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

#ifndef __MS2M_HAL_CONFIG_H__
#define __MS2M_HAL_CONFIG_H__

#include "ms2m_common.h"

#if 0
#define MS2M_MAIN_STACK_SIZE 1024 * (3 + 16)
#define MS2M_MAIN_PRIORITY 0

#define MS2M_LAN_STACK_SIZE 1024 * (2  + 16)
#define MS2M_LAN_PRIORITY 0

#define MS2M_FTY_STACK_SIZE 1024 * (6  + 16)
#define MS2M_FTY_PRIORITY 0

#define MS2M_UART_STACK_SIZE 1024 * (2  + 16)
#define MS2M_UART_PRIORITY 0

#else
#define MS2M_DEMO_UART_STACK_SIZE 1024 * (2  + 16)
#define MS2M_DEMO_UART_PRIORITY 0

#define MS2M_EII_STACK_SIZE 1024 * (12  + 16)
#define MS2M_EII_PRIORITY 0
#endif

#endif /* __MS2M_HAL_CONFIG_H__ */
