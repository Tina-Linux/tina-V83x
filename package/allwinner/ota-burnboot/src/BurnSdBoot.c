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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>

#include "Utils.h"
#include "BurnSdBoot.h"
#include "mmc.h"
#include "private_boot0.h"
#include "private_toc.h"
#include "dram.h"

#define DEVNODE_PATH_SD "/dev/mmcblk0"

#define SECTOR_SIZE 512
/*Normal boot 0 and boot 1 */
#define SD_BOOT0_SECTOR_START   16
#define SD_BOOT0_SIZE_KBYTES    32

#define SD_UBOOT_SECTOR_START_PRE   38192
#define SD_UBOOT_SECTOR_START   32800
#define SD_UBOOT_SIZE_KBYTES    1024

/*TOC0 : sector 16 - 256*/
#define SD_TOC0_SECTOR_START   16
#define SD_TOC0_SIZE_KBYTES    120

/*TOC1 : sector 32800 - 40960 */
#define SD_TOC1_SECTOR_START   32800
#define SD_TOC1_SIZE_KBYTES    4080
static int sd_boot0_start , sd_boot0_len;
static int sd_boot1_start , sd_boot1_len;
static int sd_boot1_start_pre , sd_boot1_len_pre;

/*some chip (e.g. H8) BOOT0 save in mmcblk0boot0 */
#define SD_BOOT0_PERMISION "/sys/block/mmcblk0boot0/force_ro"
#define DEVNODE_PATH_SD_BOOT0 "/dev/mmcblk0boot0"

static void sdBootInit(void)
{
    static int inited = 0;

    if (inited == 1)
        return ;

    if (check_soc_is_secure()) { //secure
        sd_boot0_start  =   SD_TOC0_SECTOR_START *  SECTOR_SIZE;
        sd_boot0_len    =   SD_TOC0_SIZE_KBYTES * 1024;
        sd_boot1_start  =   SD_TOC1_SECTOR_START * SECTOR_SIZE ;
        sd_boot1_len    =   SD_TOC1_SIZE_KBYTES * 1024;
    } else {
        sd_boot0_start  =   SD_BOOT0_SECTOR_START *  SECTOR_SIZE;
        sd_boot0_len    =   SD_BOOT0_SIZE_KBYTES * 1024;
        sd_boot1_start  =   SD_UBOOT_SECTOR_START * SECTOR_SIZE ;
        sd_boot1_len    =   SD_UBOOT_SIZE_KBYTES * 1024;
    }
    sd_boot1_start_pre  =   SD_UBOOT_SECTOR_START_PRE * SECTOR_SIZE ;
    sd_boot1_len_pre    =   SD_UBOOT_SIZE_KBYTES * 1024;
    inited =1;
    return;
}

static int writeSdBoot(int fd, void *buf, off_t offset, size_t bootsize) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        ob_error("reset the cursor failed! the error num is %d:%s\n", errno, strerror(errno));
        return -1;
    }
    if (lseek(fd, offset, SEEK_CUR) == -1) {
        ob_error("lseek failed! the error num is %d:%s\n", errno, strerror(errno));
        return -1;
    }
    ob_debug("Write sd boot : offset = 0x%llx, len= 0x%x\n", offset, bootsize);

    int result = write(fd, buf, bootsize);
    fsync(fd);
    return result;
}

static int readSdBoot(int fd ,off_t offset, size_t bootsize, void *buffer) {
    memset(buffer, 0, bootsize);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        ob_error("reset the cursor failed! the error num is %d:%s\n", errno, strerror(errno));
        return -1;
    }

    if (lseek(fd, offset, SEEK_CUR) == -1) {
        ob_error("lseek failed! the error num is %d:%s\n", errno, strerror(errno));
        return -1;
    }

    read(fd,buffer,bootsize);
    return 0;
}

static int updateBoot0Info(void *in_buffer, void *out_buffer,unsigned int buffer_size) {
    //struct boot_sdmmc_private_info_t priv_info;
    if ((in_buffer == NULL) ||(out_buffer == NULL)
            || (buffer_size < (sizeof(struct boot_sdmmc_private_info_t)))) {
        printf("%s %d:wrong input arg\n",__FUNCTION__,__LINE__);
        return  -1;
    }
    /*
    ----- normal
    offset 0~127: boot0 struct _boot_sdcard_info_t;
    offset 128~255: struct tune_sdly, timing parameter for speed mode and frequency
    ----- secure
    offset 128 ~ (224=384-160): struct tune_sdly, timing parameter for speed mode and frequency
    sizeof(priv_info)  is about 60 bytes.
    */
    memcpy((void *)((unsigned char*)out_buffer+SDMMC_PRIV_INFO_ADDR_OFFSET),
            (void *)((unsigned char*)in_buffer + SDMMC_PRIV_INFO_ADDR_OFFSET),
            sizeof(struct boot_sdmmc_private_info_t));

    {
        u32 i, *p;
        printf("out buffer\n");
        p = (u32 *)((unsigned char*)out_buffer+SDMMC_PRIV_INFO_ADDR_OFFSET);
        for (i=0; i<sizeof(struct boot_sdmmc_private_info_t)/4; i++)
            printf("%d %x\n", i, p[i]);
    }
    {
        u32 i, *p;
        printf("int buffer\n");
        p = (u32 *)((unsigned char*)in_buffer+SDMMC_PRIV_INFO_ADDR_OFFSET);
        for (i=0; i<sizeof(struct boot_sdmmc_private_info_t)/4; i++)
            printf("%d %x\n", i, p[i]);
    }
    return 0;
}

static void dump_dram_para(void* dram, uint size)
{
        int i;
        uint *addr = (uint *)dram;

        for(i = 0; i < size; i++) {
                printf("dram para[%d] = %x\n", i, addr[i]);
        }
}

static void set_boot_dram_update_flag(u32 *dram_para)
{
        /* dram_tpr13:bit31 */
        /* 0:uboot update boot0  1: not */
        u32 flag             = 0;
        __dram_para_t *pdram = (__dram_para_t *)dram_para;
        flag                 = pdram->dram_tpr13;
        flag |= (0x1U << 31);
        pdram->dram_tpr13 = flag;
}

static int updateSdBoot0(int fd, BufferExtractCookie *cookie) {
    int ret = 0;
    unsigned char* buffer = (unsigned char *)(malloc(cookie->len));
    readSdBoot(fd, sd_boot0_start, cookie->len, buffer);

    if (check_soc_is_secure()) {
        sbrom_toc0_config_t *oldToc0Config = (sbrom_toc0_config_t *)(buffer + 0x80);
        sbrom_toc0_config_t *newToc0Config = (sbrom_toc0_config_t *)(cookie->buffer + 0x80);
        ret = updateBoot0Info((void *)(oldToc0Config->storage_data+160), (void *)(newToc0Config->storage_data+160), 384-160);
    } else {
        boot0_file_head_t *oldBoot0  = (boot0_file_head_t *)(buffer);
        boot0_file_head_t *newBoot0  = (boot0_file_head_t *)(cookie->buffer);
        ret = updateBoot0Info((void *)oldBoot0->prvt_head.storage_data, (void *)newBoot0->prvt_head.storage_data, STORAGE_BUFFER_SIZE);

        /* copy the ddr param from old to new */
        memcpy((void *)newBoot0->prvt_head.dram_para, (void *)oldBoot0->prvt_head.dram_para, (32 *4));

        /*show dram param*/
        dump_dram_para((void *)newBoot0->prvt_head.dram_para, 32);
    }
    if (ret == 0) genBoot0CheckSum(cookie->buffer);
    free(buffer);
    return ret;
}

int burnSdBoot0(BufferExtractCookie *cookie) {
    sdBootInit();
    if (checkBoot0Sum(cookie)){
        ob_error("illegal binary file!\n");
        return -1;
    }

    int fd = open(DEVNODE_PATH_SD, O_RDWR);
    if (fd == -1) {
        ob_debug("open device node failed ! errno is %d : %s\n", errno, strerror(errno));
        return -1;
    }

    ob_debug("burnSdBoot0 in mmcblk0:offset = 0x%x, len =0x%lx\n", sd_boot0_start, cookie->len);
    updateSdBoot0(fd, cookie);
    int ret = writeSdBoot(fd, cookie->buffer, sd_boot0_start, cookie->len);
    fsync(fd);
    close(fd);
    if (ret < 0) {
        ob_error("burnSdBoot0 failed! error is %d : %s\n", errno, strerror(errno));
    } else {
        ob_debug("burnSdBoot0 succeed! writed %d bytes\n", ret);
        ret = 0;
    }

#if 0
    fd = open(DEVNODE_PATH_SD_BOOT0, O_RDWR);
    if (fd > 0) {
        int pmsFd = open(SD_BOOT0_PERMISION, O_WRONLY);
        if (pmsFd > 0) {
            ret = write(pmsFd,"0",1);
            close(pmsFd);
        } else {
            ob_error("can't open %s :%s \n", SD_BOOT0_PERMISION, strerror(errno));
            close(fd);
            return -1;
        }
        if (ret < 0) {
            ob_error("can't write 0 to %s :%s \n", SD_BOOT0_PERMISION, strerror(errno));
            close(fd);
            return -1;
        }
        // wipe boot0 in mmcblk0boot0
        char *empty_buffer = (char *)malloc(cookie->len);
        memset(empty_buffer, 0, cookie->len);
        ret = writeSdBoot(fd, empty_buffer, 0, cookie->len);
        free(empty_buffer);
        if (ret < 0) {
            ob_error("wipe boot0 in mmcblk0boot0 failed! error is %d : %s\n",
                    errno, strerror(errno));
            return ret;
        }
        ob_debug("wipe boot0 in mmcblk0boot0 succeed! on %d writed %d bytes\n", 0, ret);
        fsync(fd);
    } else {
        ob_debug("open device node failed ! errno is %d : %s\n", errno, strerror(errno));
    }
#endif

    return ret;
}

int burnSdUboot(BufferExtractCookie *cookie) {
    sdBootInit();
    int ret = -1;
    int defaultOffset = -1;

    if (checkUbootSum(cookie)) {
        ob_error("illegal uboot binary file!\n");
        return -1;
    }

    ob_debug("uboot binary length is %ld\n", cookie->len);
    int fd = open(DEVNODE_PATH_SD, O_RDWR);
    if (fd == -1) {
        ob_error("open device node failed ! errno is %d : %s\n", errno, strerror(errno));
        return -1;
    }

    if (check_soc_is_secure()) {
        ret = writeSdBoot(fd, cookie->buffer, sd_boot1_start, cookie->len);
    } else {
        char *empty_buffer;
        empty_buffer = (char *)malloc(sd_boot1_len_pre);
        memset(empty_buffer, 0, sd_boot1_len_pre);
        writeSdBoot(fd, empty_buffer, sd_boot1_start_pre, sd_boot1_len_pre);
        free(empty_buffer);
        defaultOffset = getUbootstartsector(cookie) * SECTOR_SIZE;
        if (defaultOffset <= 0) defaultOffset = sd_boot1_start;
        ret = writeSdBoot(fd, cookie->buffer, defaultOffset, cookie->len);
    }
    close(fd);

    if (ret < 0) {
        ob_error("burnSdUboot failed! errno is %d : %s\n", errno, strerror(errno));
    } else {
        ob_debug("burnSdUboot succeed! writed %d bytes\n", ret);
        ret = 0;
    }
    return ret;
}
