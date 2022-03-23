/*
 * (C) Copyright 2012
 *     wangflord@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#ifndef  __boot0_v2_h
#define  __boot0_v2_h

#include <linux/types.h>
#define STAMP_VALUE 0x5F0A6C39

#define MAGIC_SIZE              8
#define BOOT0_MAGIC             "eGON.BT0"
#define UBOOT_MAGIC             "uboot"

#define SECURE_TOC1_NAME       "sunxi-secure"
#define BOOT_PACKAGE_NAME      "sunxi-package"

typedef struct
{
    unsigned int        ChannelCnt;
    unsigned int        ChipCnt;                            //the count of the total nand flash chips are currently connecting on the CE pin
    unsigned int        ChipConnectInfo;                    //chip connect information, bit == 1 means there is a chip connecting on the CE pin
    unsigned int        RbCnt;
    unsigned int        RbConnectInfo;                      //the connect  information of the all rb  chips are connected
    unsigned int        RbConnectMode;                      //the rb connect  mode
    unsigned int        BankCntPerChip;                     //the count of the banks in one nand chip, multiple banks can support Inter-Leave
    unsigned int        DieCntPerChip;                      //the count of the dies in one nand chip, block management is based on Die
    unsigned int        PlaneCntPerDie;                     //the count of planes in one die, multiple planes can support multi-plane operation
    unsigned int        SectorCntPerPage;                   //the count of sectors in one single physic page, one sector is 0.5k
    unsigned int        PageCntPerPhyBlk;                   //the count of physic pages in one physic block
    unsigned int        BlkCntPerDie;                       //the count of the physic blocks in one die, include valid block and invalid block
    unsigned int        OperationOpt;                       //the mask of the operation types which current nand flash can support support
    unsigned int        FrequencePar;                       //the parameter of the hardware access clock, based on 'MHz'
    unsigned int        EccMode;                            //the Ecc Mode for the nand flash chip, 0: bch-16, 1:bch-28, 2:bch_32
    unsigned char       NandChipId[8];                      //the nand chip id of current connecting nand chip
    unsigned int        ValidBlkRatio;                      //the ratio of the valid physical blocks, based on 1024
    unsigned int        good_block_ratio;                   //good block ratio get from hwscan
    unsigned int        ReadRetryType;                      //the read retry type
    unsigned int        DDRType;
    unsigned int        Reserved[32];
}boot_nand_para_t0;


//通用的，和GPIO相关的数据结构
typedef struct _normal_gpio_cfg
{
    unsigned char      port;                       //端口号
    unsigned char      port_num;                   //端口内编号
    char      mul_sel;                    //功能编号
    char      pull;                       //电阻状态
    char      drv_level;                  //驱动驱动能力
    char      data;                       //输出电平
    unsigned char      reserved[2];                //保留位，保证对齐
}normal_gpio_cfg;

/******************************************************************************/
/*                              file head of Boot0                            */
/******************************************************************************/
typedef struct _boot0_private_head_t
{
    unsigned int            prvt_head_size;
    char                    prvt_head_vsn[4];       // the version of boot0_private_head_t
    unsigned int            dram_para[32];          // DRAM patameters for initialising dram. Original values is arbitrary,
    int                        uart_port;              // UART控制器编号
    normal_gpio_cfg         uart_ctrl[2];           // UART控制器(调试打印口)数据信息
    int                     enable_jtag;            // 1 : enable,  0 : disable
    normal_gpio_cfg            jtag_gpio[5];           // 保存JTAG的全部GPIO信息
    normal_gpio_cfg         storage_gpio[32];       // 存储设备 GPIO信息
    char                    storage_data[512 - sizeof(normal_gpio_cfg) * 32];      // 用户保留数据信息
    //boot_nand_connect_info_t    nand_connect_info;
}boot0_private_head_t;


typedef struct standard_Boot_file_head
{
    unsigned int  jump_instruction;   // one intruction jumping to real code
    unsigned char   magic[8];           // ="eGON.BT0",  not C-style string.
    unsigned int  check_sum;          // generated by PC
    unsigned int  length;             // generated by PC
    unsigned int  pub_head_size;      // the size of boot_file_head_t
    unsigned char   pub_head_vsn[4];    // the version of boot_file_head_t
    unsigned char   file_head_vsn[4];   // the version of boot0_file_head_t
    unsigned char   Boot_vsn[4];        // Boot version
    unsigned char   eGON_vsn[4];        // eGON version
    unsigned char   platform[8];        // platform information
}standard_boot_file_head_t;


typedef struct _boot0_file_head_t
{
    standard_boot_file_head_t   boot_head;
    boot0_private_head_t          prvt_head;
}boot0_file_head_t;






#endif     //  ifndef __boot0_h

/* end of boot0.h */

typedef struct _boot_core_para_t
{
    unsigned int  user_set_clock;                 // 运行频率 M单位
    unsigned int  user_set_core_vol;              // 核心电压 mV单位
    unsigned int  vol_threshold;                  // 开机门限电压
}boot_core_para_t;


#ifndef  __uboot_h
#define  __uboot_h

/******************************************************************************/
/*                              file head of Boot2.0-uboot                          */
/******************************************************************************/
typedef struct _uboot_private_head_t
{
    unsigned int                dram_para[32];
    int                            run_clock;                // Mhz
    int                            run_core_vol;            // mV
    int                            uart_port;              // UART控制器编号
    normal_gpio_cfg             uart_gpio[2];           // UART控制器(调试打印口)GPIO信息
    int                            twi_port;               // TWI控制器编号
    normal_gpio_cfg             twi_gpio[2];            // TWI控制器GPIO信息，用于控制TWI
    int                            work_mode;              // 工作模式
    int                         storage_type;           // 存储介质类型  0：nand   1：sdcard    2: spinor
    normal_gpio_cfg             nand_gpio[32];          // nand GPIO信息
    char                        nand_spare_data[256];    // nand 额外信息
    normal_gpio_cfg             sdcard_gpio[32];        // sdcard GPIO信息
    char                         sdcard_spare_data[256];    // sdcard 额外信息
    int                         secureos_exist;
    uint                        uboot_start_sector_in_mmc;
    int                            reserved[4];            // 保留数据位, 256bytes align
}uboot_private_head_t;

typedef struct _uBoot_file_head
{
    unsigned int  jump_instruction;   // one intruction jumping to real code
    unsigned char magic[8];  // ="u-boot"
    unsigned int  check_sum;          // generated by PC
    unsigned int  align_size;          // align size in byte
    unsigned int  length;             // the size of all file
    unsigned int  uboot_length;       // the size of uboot
    unsigned char version[8];         // uboot version
    unsigned char platform[8];        // platform information
    int           reserved[1];        //stamp space, 16bytes align
}uboot_file_head;


typedef struct _uboot_file_head_t
{
    uboot_file_head      boot_head;
    uboot_private_head_t  prvt_head;
}uboot_file_head_t;

#endif     //  ifndef __uboot_h

/* end of uboot.h */

#ifndef  __toc_h
#define  __toc_h
//toc1的头部数据结构
typedef struct sbrom_toc1_head_info
{
    char name[16]    ;    //user can modify
    unsigned int  magic    ;    //must equal TOC_U32_MAGIC
    unsigned int  add_sum    ;

    unsigned int  serial_num    ;    //user can modify
    unsigned int  status        ;    //user can modify,such as TOC_MAIN_INFO_STATUS_ENCRYP_NOT_USED

    unsigned int  items_nr;    //total entry number
    unsigned int  valid_len;
    unsigned int  reserved[5];    //reserved for future
    unsigned int  end;
}sbrom_toc1_head_info_t;

typedef struct
{
    unsigned char  name[8];
    unsigned int magic;
    unsigned int check_sum;

    unsigned int serial_num;
    unsigned int status;

    unsigned int items_nr;
    unsigned int length;
    unsigned char  platform[4];

    unsigned int reserved[2];
    unsigned int end;

}toc0_private_head_t;

typedef struct sbrom_toc0_config
{
    unsigned char        config_vsn[4];
    unsigned int          dram_para[32];      // dram参数
    int                      uart_port;          // UART控制器编号
    normal_gpio_cfg       uart_ctrl[2];        // UART控制器GPIO
    int                  enable_jtag;        // JTAG使能
    normal_gpio_cfg       jtag_gpio[5];        // JTAG控制器GPIO
    normal_gpio_cfg      storage_gpio[50];     // 存储设备 GPIO信息
                                            // 0-23放nand，24-31存放卡0，32-39放卡2
                                            // 40-49存放spi
    char                   storage_data[384];  // 0-159,存储nand信息；160-255,存放卡信息
    unsigned int        secure_dram_mbytes; //
    unsigned int        drm_start_mbytes;   //
    unsigned int        drm_size_mbytes;    //
    unsigned int          res[8];               // 总共1024字节
}sbrom_toc0_config_t;

#define TOC0_CONFIG_HEAD_OFFSET 0x80
#define  TOC_MAIN_INFO_MAGIC    0x89119800
#endif /*endif toc_*/
