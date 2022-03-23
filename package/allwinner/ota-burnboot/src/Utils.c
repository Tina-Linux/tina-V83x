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


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#include "BootHead.h"
#include "Utils.h"

#define SD_UBOOT_SECTOR_START_DEFAULT 32800

#define CMDLINE_FILE_PATH "/proc/cmdline"
#define SYSINFO_FILE_PATH "/sys/class/sunxi_info/sys_info"
#define SYSINFO_SECURE "sunxi_secure      : secure"
#define VERIFIED_BOOT "androidboot.verifiedbootstate="
#define USE_GPT "gpt=1"

/* #define CHECK_SECURE_BY_CMDLINE */
#define CHECK_SECURE_BY_SYSINFO


static int _is_secure = 0;
static int _is_gpt = 0;

static int spliteKeyAndValue(char* str, char** key, char** value)
{
    int elocation = strcspn(str,"=");
    if (elocation < 0){
        return -1;
    }
    str[elocation] = '\0';
    *key = str;
    *value = str + elocation + 1;
    return 0;
}

static int getInfoFromCmdline(const char* key, char* value, int val_len)
{
    FILE* fp;
    char cmdline[1024];

    /* read partition info from /proc/cmdline */
    if ((fp = fopen(CMDLINE_FILE_PATH, "r")) == NULL) {
        ob_debug("can't open %s \n", CMDLINE_FILE_PATH);
        return -1;
    }
    fgets(cmdline, 1024, fp);
    fclose(fp);

#ifdef CHECK_SECURE_BY_CMDLINE
    if (strstr(cmdline, VERIFIED_BOOT))
        _is_secure = 1;
    else
        _is_secure = 0;
#endif

    if (strstr(cmdline, USE_GPT))
        _is_gpt = 1;
    else
        _is_gpt = 0;

    /* splite the cmdline by space */
    char* p = NULL;
    char* lkey = NULL;
    char* lvalue = NULL;
    p = strtok(cmdline, " ");
    if (!spliteKeyAndValue(p, &lkey, &lvalue)){
        if (!strcmp(lkey,key)){
            goto done;
        }
    }

    while ((p = strtok(NULL, " "))){
        if (!spliteKeyAndValue(p, &lkey, &lvalue)){
            if (!strcmp(lkey,key)){
                goto done;
            }
        }
    }

    ob_debug("No key named %s in cmdline.\n", key);
    return -1;

done:
    strncpy(value, lvalue, val_len);
    return 0;
}

static int getInfoFromSysinfo(void)
{
    FILE* fp;
    char sysinfo_line1[256];
    char sysinfo_line2[256];

#ifdef CHECK_SECURE_BY_SYSINFO
    if ((fp = fopen(SYSINFO_FILE_PATH, "r")) == NULL) {
        ob_debug("can't open %s\n", SYSINFO_FILE_PATH);
        return -1;
    }
    fgets(sysinfo_line1, 256, fp);
    fgets(sysinfo_line2, 256, fp);
    fclose(fp);

    if (strstr(sysinfo_line2, SYSINFO_SECURE))
        _is_secure = 1;
    else
        _is_secure = 0;
#endif

    return 0;
}

int initInfo(void)
{
    getInfoFromSysinfo();
    return 0;
}

int getFlashType() {
    char ctype[20];
    int flash_type = FLASH_TYPE_UNKNOW;

    // get type from boot_type
    if (getInfoFromCmdline("boot_type", ctype, 10) == 0) {
        ob_debug("flash type = %s\n", ctype);
        //atoi出错时会返回0,当ctype字符串为0时也会返回0，所以这里要判断是否出错.
        flash_type = atoi(ctype);
        if (flash_type == 0 && ctype[0] != '0')
            return FLASH_TYPE_UNKNOW;
        return flash_type;
    // get type from root
    } else if (getInfoFromCmdline("root", ctype, 20) == 0) {
        if (!strncmp(ctype, "/dev/mmcblk", sizeof("/dev/mmcblk")-1))
            return FLASH_TYPE_MMC;
        if (!strncmp(ctype, "/dev/nand", sizeof("/dev/nand")-1))
            return FLASH_TYPE_NAND;
        if (!strncmp(ctype, "/dev/ubi", sizeof("/dev/ubi")-1))
            return FLASH_TYPE_NAND;
        if (!strncmp(ctype, "/dev/mtdblock", sizeof("/dev/mtdblock")-1))
            return FLASH_TYPE_NOR;
    }
    return FLASH_TYPE_UNKNOW;
}

/*
 * Check secure solution or not
 * Return 0 if normal , return 1 if secure
 */
int check_soc_is_secure(void)
{
    return _is_secure;
/*
#define CHECK_SOC_SECURE_ATTR 0x00
#define CHECK_BOOT_SECURE_ATTR 0x04

    int fd, ret;

    fd = open("/dev/sunxi_soc_info", O_RDWR);
    if (fd == -1) {
        bb_debug("open /dev/sunxi_soc_info failed!\n");
        return 0 ;
    }

    ret = ioctl(fd, CHECK_SOC_SECURE_ATTR, NULL);
    if (ret) {// ret=1 in secure case
        bb_debug("soc is secure. (return value:%x)\n", ret);
    } else {
        bb_debug("soc is normal. (return value:%x)\n", ret);
        ret = ioctl(fd, CHECK_BOOT_SECURE_ATTR, NULL);
        if (ret)
            bb_debug("secure boot for normal case\n");
    }

    close(fd);
    return ret;
*/
}

/*
 * Check gpt or not
 * Return 0 if partition table is mbr , return 1 if is gpt
 */
int check_is_gpt(void)
{
    return _is_gpt;
}

int check_is_ubi(void)
{
    if (access("/dev/ubi0", F_OK) != -1)
	    return 1;

    return 0;
}


int getBufferExtractCookieOfFile(const char* path, BufferExtractCookie* cookie)
{

    if (cookie == NULL || path == NULL)
        return -EINVAL;

    struct stat statbuff;
    if (stat(path, &statbuff) < 0)
        return -ENOENT;

    cookie->len = statbuff.st_size;
    ob_debug("file size is %d\n",(int)cookie->len);

    unsigned char* buffer = (unsigned char *)malloc(cookie->len);
    if (buffer==NULL)
        return -errno;

    FILE* fp = fopen(path,"r");
    if (fp == NULL)
        goto err;

    if (!fread(buffer, cookie->len, 1, fp))
        goto err;

    cookie->buffer = buffer;

    fclose(fp);
    return 0;

err:
    free(buffer);
    fclose(fp);
    return -errno;
}

static int verify_toc_addsum(void *mem_base, unsigned int size, unsigned int *psum) {
    unsigned int *buf;
    unsigned int count;
    unsigned int src_sum;
    unsigned int sum;

    /* 生成校验和 */
    src_sum = *psum;                  // 从Boot_file_head中的“check_sum”字段取出校验和
    ob_debug("read sum :0x%x\n",src_sum);
    *psum = STAMP_VALUE;              // 将STAMP_VALUE写入Boot_file_head中的“check_sum”字段

    count = size >> 2;                         // 以 字（4bytes）为单位计数
    sum = 0;
    buf = (unsigned int *)mem_base;
    do {
        sum += *buf++;                         // 依次累加，求得校验和
        sum += *buf++;                         // 依次累加，求得校验和
        sum += *buf++;                         // 依次累加，求得校验和
        sum += *buf++;                         // 依次累加，求得校验和
    } while ((count -= 4) > (4 - 1));

    while (count-- > 0)
        sum += *buf++;

    *psum = src_sum;                  // 恢复Boot_file_head中的“check_sum”字段的值
    ob_debug("calc sum :0x%x\n",sum);

    if (sum == src_sum)
        return 0;                           // 校验成功
    else
        return -1;                          // 校验失败
}

int checkBoot0Sum(BufferExtractCookie* cookie) {
    standard_boot_file_head_t  *head_p;
    toc0_private_head_t *head_t;
    unsigned int length;
    unsigned int *buf;
    unsigned int loop;
    unsigned int i;
    unsigned int sum;
    unsigned int csum;

    if (check_soc_is_secure()) {
        unsigned int *psum;
        head_t = (toc0_private_head_t *)cookie->buffer;
        if (head_t->magic != TOC_MAIN_INFO_MAGIC) {
            ob_debug("toc0 magic error\n");
            return -1;
        }
        psum = &(head_t->check_sum);
        return verify_toc_addsum(cookie->buffer, cookie->len, psum);

    } else {
        head_p = (standard_boot_file_head_t *)cookie->buffer;

        if (strncmp((const char *)head_p->magic, BOOT0_MAGIC, MAGIC_SIZE)) {
            ob_debug("boot0 magic error\n");
            return -1;
        }

        length = head_p->length;
        if ((length & 0x3) != 0)                   // must 4-byte-aligned
            return -1;
        if ((length > 32 * 1024) != 0 ) {
            ob_debug("boot0 file length over size!!\n");
        }
        if ((length & (512 - 1)) != 0) {
            ob_debug("boot0 file did not aliged!!\n");
        }
        buf = (unsigned int *)cookie->buffer;
        csum = head_p->check_sum;
        head_p->check_sum = STAMP_VALUE;              // fill stamp
        loop = length >> 2;

        for (i = 0, sum = 0; i < loop; i++)
            sum += buf[i];

        head_p->check_sum = csum;
        ob_debug("Boot0 -> File length is %u,original sum is %u,new sum is %u\n", length, head_p->check_sum, sum);
        return !(csum == sum);
    }
}

int checkUbootSum(BufferExtractCookie* cookie) {
    uboot_file_head  *head_p;
    sbrom_toc1_head_info_t *head_toc1;
    unsigned int length;
    unsigned int *buf;
    unsigned int loop;
    unsigned int i;
    unsigned int sum;
    unsigned int csum;
    unsigned int *psum;

    if (check_soc_is_secure()) {
        head_toc1 = (sbrom_toc1_head_info_t *)(cookie->buffer);
        if (head_toc1->magic != TOC_MAIN_INFO_MAGIC) {
            ob_debug("toc1 info magic error\n");
            return -1;
        }
        if (strncmp(head_toc1->name, SECURE_TOC1_NAME, strlen(SECURE_TOC1_NAME))) {
            ob_debug("toc1 name error\n");
            return -1;
        }
        psum = &(head_toc1->add_sum);
        return verify_toc_addsum(cookie->buffer, cookie->len, psum);
    } else {
        head_toc1 = (sbrom_toc1_head_info_t *)(cookie->buffer);
        if (head_toc1->magic == TOC_MAIN_INFO_MAGIC) {
            /* for boot_package */
            if (strncmp(head_toc1->name, BOOT_PACKAGE_NAME, strlen(BOOT_PACKAGE_NAME))) {
                ob_debug("boot_package name error\n");
                return -1;
            }
            psum = &(head_toc1->add_sum);
            return verify_toc_addsum(cookie->buffer, cookie->len, psum);
        } else {
            /* for uboot2011 */
            head_p = (uboot_file_head *)cookie->buffer;
            if (strncmp(head_p->magic, UBOOT_MAGIC, strlen(UBOOT_MAGIC))) {
                ob_debug("uboot magic error\n");
                return -1;
            }
            length = head_p->length;
            if ((length & 0x3) != 0)                   // must 4-byte-aligned
                return -1;

            buf = (unsigned int *)cookie->buffer;
            csum = head_p->check_sum;

            head_p->check_sum = STAMP_VALUE;              // fill stamp
            loop = length >> 2;

            for (i = 0, sum = 0;  i < loop;  i++)
                sum += buf[i];

            head_p->check_sum = csum;
            ob_debug("Uboot -> File length is %u,original sum is %u,new sum is %u\n", length, head_p->check_sum, sum);
            return !(csum == sum);
        }
    }
}

int getUbootstartsector(BufferExtractCookie* cookie) {
    if(check_soc_is_secure()){
        return -1;
    }
    uboot_file_head_t  *head_p = NULL;
    unsigned int  start_sector = 0;
    head_p = (uboot_file_head_t *)cookie->buffer;

    if (head_p == NULL)
        return -1;

    start_sector = head_p->prvt_head.uboot_start_sector_in_mmc;

    if(start_sector == 0)
    {
        start_sector = SD_UBOOT_SECTOR_START_DEFAULT;
    }

    return (int)start_sector;
}

int genBoot0CheckSum(void *cookie)
{
    standard_boot_file_head_t  *head_p;
    unsigned int length;
    unsigned int *buf;
    unsigned int loop;
    unsigned int i;
    unsigned int sum;
    unsigned int *psum;

    if(check_soc_is_secure()){
        toc0_private_head_t *head_t = (toc0_private_head_t *)cookie;
        psum = &(head_t->check_sum);
        length = head_t->length;

    }else{
        head_p = (standard_boot_file_head_t *)cookie;
        psum = &head_p->check_sum ;
        length = head_p->length;
    }

    if( ( length & 0x3 ) != 0 )                   // must 4-byte-aligned
        return -1;
    buf = (unsigned int *)cookie;
    *psum = STAMP_VALUE;              // fill stamp
    loop = length >> 2;
    sum = 0 ;
    for( i = 0, sum = 0;  i < loop;  i++ )
        sum += buf[i];

    /* write back check sum */
    *psum = sum;
    return 0 ;
}
