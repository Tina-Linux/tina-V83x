/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_uart_msmart.c
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

#include "stdint.h"
#include "string.h"

#include "ms2m_hal_uart.h"
#include "ms2m_hal_chip.h"

#include "ms2m_demo_uart_msmart.h"

#define MS2M_UART_HEAD 0xAA

typedef struct ms2m_uart_msmart_head_s
{
    uint8_t head;
    uint8_t len;
    uint8_t appliance_type;
    uint8_t reserve[6];
    uint8_t cmd;
    uint8_t msg_body[1];
}ms2m_uart_msmart_head_t;

//-------------------------------------------------------------//
static uint8_t _checksum(uint8_t *data, uint16_t len)
{
    int i = 0;
    uint8_t check_sum = 0;

    for(i = 0; i < len; i++)
    {
        check_sum += data[i];
    }
    check_sum = (~(check_sum)) + 1;
    return check_sum;
}

static MS2M_STATUS _msmart_checksum_pack(uint8_t *package)
{
    if(package == NULL)
    {
        return M_ERROR;
    }
    uint8_t *d = &(package[1]);
    uint16_t len = (uint16_t)package[1] - 1;
    d[len] = _checksum(d, len);
    return M_OK;
}

MS2M_STATUS ms2m_uart_msmart_checksum_check(uint8_t *package)
{
    if(package == NULL)
    {
        return M_ERROR;
    }
    uint8_t *d = &(package[1]);
    uint16_t len = (uint16_t)package[1] - 1;
    if(d[len] != _checksum(d, len))
    {
        return M_ERROR;
    }
    return M_OK;
}

MS2M_STATUS ms2m_uart_msmart_pack(uint8_t cmd, uint8_t appliance_type,\
                                  uint8_t *payload, uint16_t payload_len, \
                                  uint8_t *package, uint16_t *package_len)
{
    *package_len = sizeof(ms2m_uart_msmart_head_t) + payload_len;
    ms2m_uart_msmart_head_t *head = (ms2m_uart_msmart_head_t *)package;
    head->head = MS2M_UART_HEAD;
    head->len = *package_len - 1;
    head->appliance_type = appliance_type;
    head->cmd = cmd;
    memcpy(head->msg_body, payload, payload_len);
    _msmart_checksum_pack(package);
    return M_OK;
}
//-------------------------------------------------------------//

MS2M_STATUS ms2m_uart_msmart_recv(uint8_t *package, uint16_t *package_len)
{
    int i = 0;
    uint8_t *p = package;
    int ret = 0;
    int error_count = 0;
    int temp_len = 1;

    for(i = 0 ; i < 3 ;i++)
    {
        //iRet = recv(sockfd, recv_p, temp_len, 0);
        ret = ms2m_hal_uart_read(p, temp_len);

        if(ret <= 0)
        {
            if((i >= 1) && (error_count < MS2M_HAL_UART_ERROR_CNT_MAX))
            {
                i--;
                error_count++;
                //ms2m_hal_msleep(5);
                continue;
            }
            return M_ERROR;
        }
        error_count = 0;

        switch (i)
        {
        case 0:
            if(ret == temp_len && *p == 0xAA)
            {
                p++;
            }
            else
            {
                i = -1;
            }
            break;
        case 1:
            if(ret == temp_len)
            {
                temp_len = *p - 1;
                *package_len = *p + 1;
                p++;
            }
            else
            {
                i = -1;
                temp_len = 1;
                p = package;
            }
            break;
        default:
            if(ret != temp_len)
            {
                i = 1;
                temp_len = temp_len - ret;
                p += ret;
            }
            break;
        }
    }

    return M_OK;
}
