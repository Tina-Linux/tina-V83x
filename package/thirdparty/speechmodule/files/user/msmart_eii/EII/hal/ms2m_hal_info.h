/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_info.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/07/10
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

#ifndef __MS2M_HAL_INFO_H__
#define __MS2M_HAL_INFO_H__

#include "ms2m_common.h"

MS2M_STATUS ms2m_hal_info_ip(unsigned int *ip);
MS2M_STATUS ms2m_hal_info_mac(uint8_t *mac);

void ms2m_hal_info_regist();

#endif /* __MS2M_HAL_INFO_H__ */
