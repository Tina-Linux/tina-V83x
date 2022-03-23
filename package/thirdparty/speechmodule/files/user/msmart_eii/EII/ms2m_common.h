/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_common.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/08/15
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

#ifndef __MS2M_COMMON_H__
#define __MS2M_COMMON_H__

#include "stdint.h"

#define MS2M_COMBINE(h,l) ((h << 8) | l)

#define MS2M_HIGH_BIT(x) ((x & 0xFF00) >> 8)
#define MS2M_LOW_BIT(x) (x & 0xFF)

#define IS_NULL(ptr)              (NULL == (ptr))

typedef enum
{
    M_NO_MEM = -2,
    M_ERROR = -1,
    M_OK = 0,
    M_LOSS,
    M_NO_LKG,
    M_DIRTY,
    M_CONTINUE
}MS2M_STATUS;

typedef enum
{
    MS2M_CLOUD_M2N_REQUEST_LUA = 0X0001,    //request Lua script
	MS2M_CLOUD_M2N_REQUEST_SCENE_OPT ,      //request scene configuration
	MS2M_CLOUD_N2M_LKG_EXEC_ISSUED,         //linkage execute issued
	MS2M_CLOUD_N2M_LKG_WHEN_SET,            //linkage requirement set
	MS2M_CLOUD_N2M_LKG_THEN_SET,            //linkage operating set
	MS2M_CLOUD_N2M_LKG_JUDGE,               //judge operation requiremet
	MS2M_CLOUD_N2M_LKG_DEL,                 //delete linkage set
    MS2M_CLOUD_N2M_LKG_OPT,                 //the linkage option
    MS2M_CLOUD_N2M_LKG_STORE,               //store the linkage
    MS2M_CLOUD_M2N_LKG_EXEC_UPDATA,         //updata execute
    MS2M_CLOUD_M2N_LKG_EXEC_FIN_UPDATA,     //updata already executed operation
    MS2M_CLOUD_M2N_LKG_WHEN_UPDATA = 0x0c,  //updata linkage requirement

	MS2M_CLOUD_M2N_REQUEST_LOCATION = 0x10,
	MS2M_CLOUD_N2M_DEV_CNTL,
	MS2M_CLOUD_M2N_DEV_CNTL_ACK,
	MS2M_CLOUD_M2N_DEV_STATUS_UPDATE,
}MS2M_CLOUD_MSG_YTPE;

struct msmart_net_packet {
	int       command;
	uint8_t * data;
	int       datalen;
};

typedef enum
{
    EII_STATUS_COMMAND_MIN = 0x0000,
    EII_STATUS_NO_SCRIPT,
    EII_STATUS_LKG_WHEN_ERROR,
    EII_STATUS_LKG_THEN_ERROR,
    EII_STATUS_SCRIPT_ERROR,
    EII_STATUS_LKG_INFO,
    EII_STATUS_COMMAND_MAX,
    EII_CMD_COMMAND_MIN = 0x1000,
    EII_CMD_SCRIPT_READY,
    EII_CMD_EXTRA_INPUT,
    EII_CMD_COMMAND_MAX,
    EII_CLOUD_COMMAND_MIN = 0x2000,
    EII_CLOUD_DEV_CNTL,
    EII_CLOUD_DEV_CNTL_ACK,
    EII_CLOUD_DEV_STATUS_GET,
    EII_CLOUD_DEV_STATUS_CHANGE,
    EII_CLOUD_DEV_NOTIFY,
    EII_CLOUD_LKG_EXEC,
    EII_CLOUD_LKG_JUDGE,
    EII_CLOUD_LKG_WHEN_SET,
    EII_CLOUD_LKG_THEN_SET,
    EII_CLOUD_LKG_DEL,
    EII_CLOUD_LKG_OPT,
    EII_CLOUD_LKG_STORE,
    EII_CLOUD_LKG_INFO,
    EII_CLOUD_COMMAND_MAX,
    EII_LAN_COMMAND_MIN = 0x3000,
    EII_LAN_LKG_EXEC,
    EII_LAN_LKG_JUDGE,
    EII_LAN_LKG_ACT,
    EII_LAN_DEV_CNTL,
    EII_LAN_COMMAND_MAX,
}EII_COMMAND;

typedef struct MS_MS2M_HANDLE
{
	uint8_t cloud_sub_cmd;
	EII_COMMAND eii_cmd;
}MS_MS2M_HANDLE;

//-LOG Config-//
/* enable assert check */
//#define ELOG_ASSERT_ENABLE
/* setting static output log level */
//#define ELOG_OUTPUT_LVL                      ELOG_LVL_VERBOSE
#define ELOG_OUTPUT_LVL                      ELOG_LVL_INFO
/* buffer size for every line's log */
#define ELOG_LINE_BUF_SIZE                   1024

/* output line number max length */
#define ELOG_LINE_NUM_MAX_LEN                5
/* output filter's tag max length */
#define ELOG_FILTER_TAG_MAX_LEN              16
/* output filter's keyword max length */
#define ELOG_FILTER_KW_MAX_LEN               16
/* output newline sign */
#define ELOG_NEWLINE_SIGN                    "\r\n"
/* enable log output. default open this macro */
#define ELOG_OUTPUT_ENABLE
/* enable log color */
#define ELOG_COLOR_ENABLE

#endif /* __MS2M_COMMON_H__ */
