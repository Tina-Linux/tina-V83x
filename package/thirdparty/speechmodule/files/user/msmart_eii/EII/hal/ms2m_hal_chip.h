/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_hal_chip.h
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

#ifndef __MS2M_HAL_CHIP_H__
#define __MS2M_HAL_CHIP_H__

#include "ms2m_common.h"

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <sys/time.h>

#define MS2M_TICKS_PER_SECOND 1000//NC

#define MS2M_PRINTF(...) printf(__VA_ARGS__)
#define MS2M_PUTCHAR(c) putchar(c)
int MS2M_WRITE(const char *c, unsigned int l);

#define ms2m_hal_mem_realloc realloc

MS2M_STATUS ms2m_hal_now_time(char *time_str);

#define ms2m_hal_gettimeticks(tv, tzp) gettimeofday(tv, (struct  timezone *)tzp)
unsigned long ms2m_hal_get_ticks();
MS2M_STATUS ms2m_hal_ticks_compare(unsigned long start_tick, unsigned long end_tick, unsigned long interval_ms);

unsigned long ms2m_hal_ticks2msec(unsigned long ticks);
unsigned long ms2m_hal_msec2ticks(unsigned long ms);

void ms2m_hal_srand(unsigned int seed);
unsigned int ms2m_hal_rand();

void ms2m_hal_msleep(unsigned int ms);
void ms2m_hal_reboot();

void ms2m_hal_mem_left();

#define MS2M_HAL_FLASH_BLOCK_SIZE 0x1000

void ms2m_hal_flash_init();
MS2M_STATUS ms2m_hal_flash_erase_sector(unsigned int addr);
MS2M_STATUS ms2m_hal_flash_read(unsigned int addr, char *buf, unsigned int len);
MS2M_STATUS ms2m_hal_flash_write(unsigned int addr, char *buf, unsigned int len);

#define ms2m_hal_time time

#endif /* __MS2M_HAL_CHIP_H__ */
