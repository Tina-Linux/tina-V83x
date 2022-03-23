/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_demo_uart_msmart.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/04/24
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

#ifndef __MS2M_DEMO_UART_MSMART_H__
#define __MS2M_DEMO_UART_MSMART_H__

#include "ms2m_common.h"

#define MS2M_UNKNOWN_APPLIANCE_TYPE 0xFF

#define MS2M_UART_CMD_SN_QUERY 0x07
#define MS2M_UART_CMD_TYPE_QUERY 0xA0

#define MS2M_UART_CMD_AC_SN_QUERY 0x65

typedef struct ms2m_uart_msmart_type_s
{
    uint8_t reserve_1;
    uint8_t type;
    uint8_t model_l;
    uint8_t model_h;
}ms2m_uart_msmart_type_t;

typedef struct ms2m_uart_msmart_ac_sn_s
{
    uint8_t reserve_1;
    uint8_t type;
    uint8_t model_l;
    uint8_t model_h;
    uint8_t sn_len;
    uint8_t sn[1];
}ms2m_uart_msmart_ac_sn_t;

MS2M_STATUS ms2m_uart_msmart_checksum_check(uint8_t *package);

MS2M_STATUS ms2m_uart_msmart_pack(uint8_t cmd, uint8_t appliance_type,\
                                  uint8_t *payload, uint16_t payload_len, \
                                  uint8_t *package, uint16_t *package_len);

MS2M_STATUS ms2m_uart_msmart_recv(uint8_t *package, uint16_t *package_len);

#endif /* __MS2M_DEMO_UART_MSMART_H__ */
