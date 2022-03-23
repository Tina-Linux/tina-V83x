/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef  __utils_h
#define  __utils_h

#include <sys/types.h>
#include <stdio.h>

#define DEBUG 0

#if DEBUG
#define ob_debug(fmt, args...) printf("ota-boot:" fmt, ## args)
#else
#define ob_debug(fmt, args...)
#endif

#define ob_error(fmt, args...) printf("ota-boot:[error]:" fmt, ## args)

#define FLASH_TYPE_UNKNOW	-1
#define FLASH_TYPE_NAND     0
#define FLASH_TYPE_SDCARD   1
#define FLASH_TYPE_MMC      2
#define FLASH_TYPE_NOR      3
#define FLASH_TYPE_MMC2     4
#define FLASH_TYPE_SPINAND  5

#define STAMP_VALUE             0x5F0A6C39

typedef struct {
    unsigned char* buffer;
    long len;
} BufferExtractCookie;

int check_soc_is_secure(void);
int check_is_gpt(void);
int check_is_ubi(void);
int getFlashType();
int getBufferExtractCookieOfFile(const char* path, BufferExtractCookie* cookie);
int getUbootstartsector(BufferExtractCookie* cookie);

int checkBoot0Sum(BufferExtractCookie* cookie);
int checkUbootSum(BufferExtractCookie* cookie);

int genBoot0CheckSum(void *cookie);
int initInfo(void);

#endif
