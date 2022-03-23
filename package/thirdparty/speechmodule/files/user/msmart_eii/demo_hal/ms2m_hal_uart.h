/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_uart.h
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

#ifndef __MS2M_HAL_UART_H__
#define __MS2M_HAL_UART_H__

#include "ms2m_common.h"

#define MS2M_HAL_UART_ERROR_CNT_MAX 5

void ms2m_hal_uart_port_set(char *port);

MS2M_STATUS ms2m_hal_uart_init(void);

int ms2m_hal_uart_read(unsigned char *buffer, unsigned int len);
int ms2m_hal_uart_write(unsigned char *buffer, unsigned int len);

#endif /* __MS2M_HAL_UART_H__ */
