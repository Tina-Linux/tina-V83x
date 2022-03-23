/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_EII.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/08/07
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

#ifndef __MS2M_EII_H__
#define __MS2M_EII_H__

#include "stdio.h"

#include "ms2m_common.h"

typedef struct eii_config_s
{
    int script_block_size;
    int when_block_size;
    int then_block_size;
    unsigned int base_addr;
}eii_config_t;

MS2M_STATUS eii_init(eii_config_t *c);
MS2M_STATUS eii_appliance_type_set(uint8_t appliance_type);
MS2M_STATUS eii_sn_set(uint8_t *sn);
MS2M_STATUS eii_model_set(uint8_t model_l, uint8_t model_h);

//MS2M_STATUS eii_dev_home_id_set(char *id);
//MS2M_STATUS eii_dev_room_set(uint32_t room, char *room_type, uint16_t room_type_len);
MS2M_STATUS eii_dev_position_set(uint8_t *payload, uint16_t payload_len);
MS2M_STATUS eii_script_download_get(uint8_t *payload, uint16_t payload_len,
                                    uint8_t *ver, uint16_t *ver_len,
                                    uint8_t *md5, uint16_t *md5_len,
                                    uint8_t *url, uint16_t *url_len);

uint8_t eii_appliance_type_get();
MS2M_STATUS eii_sn_get(uint8_t *sn);
uint16_t eii_model_get();

unsigned int eii_script_addr_get();

MS2M_STATUS eii_script_clear();
MS2M_STATUS eii_script_version_get(char *ver, int ver_size);

void eii_cloud_dev_enable(uint8_t flag);
void eii_cloud_lkg_check(uint8_t flag);
MS2M_STATUS eii_cloud_lkg_check_status();

void eii_cloud_cb_set(MS2M_STATUS (* cb)(EII_COMMAND cmd, uint8_t *payload, uint16_t len));
unsigned int eii_cloud_cmd(EII_COMMAND cmd, uint8_t *payload, uint16_t len, uint8_t **ack, size_t *ack_len);
void eii_cloud_ack_free(uint8_t *ack);

unsigned int eii_cloud_ack_error_code(uint8_t *ack, uint16_t ack_len);

uint8_t *eii_cloud_script_request_package(size_t *len);
uint8_t *eii_cloud_linkage_request_package(size_t *len);
uint8_t *eii_cloud_position_request_package(size_t *len);

MS2M_STATUS eii_uart_cb_set(MS2M_STATUS (* cb)(uint8_t *payload, uint16_t len));
MS2M_STATUS eii_uart_put(uint8_t *payload, uint16_t len);

MS2M_STATUS eii_security_random_reset();

void eii_process();

#endif /* __MS2M_EII_H__ */
