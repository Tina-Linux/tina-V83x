/*
 * wifi_config.h
 *
 *  Created on: 2018年8月17日
 *      Author: hp
 */

#ifndef WIFI_CONFIG_H_
#define WIFI_CONFIG_H_
//#include "sc_common.h"
#include <sys/types.h>
#include <linux/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <pthread.h>

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define CFGFILE_OPEN			0
#define CFGFILE_WPA				1
#define CFGFILE_WEP				2
#define FLAG_WPA		0x0001
#define FLAG_WPA2		0x0002
#define FLAG_WEP		0x0004

/* 0.2 seconds of staying in each channel */
#define TIME_CHSWITCH			200000

/* Bandwidth Offset */
#define HAL_PRIME_CHNL_OFFSET_DONT_CARE	0
#define HAL_PRIME_CHNL_OFFSET_LOWER	1
#define HAL_PRIME_CHNL_OFFSET_UPPER	2

#define _FW_UNDER_SURVEY	0x0800
#define BUF_SIZ 10240

#define MAX_LINE_LEN		1024
#define MAX_SCAN_TIMES		10

#define IFNAMSIZ 16

typedef enum MS_ENUM_LOCK_CHN_INFO_T{
        MS_ENUM_NO_LOCK = 0x00,
        MS_ENUM_LOCK_MSC,
        MS_ENUM_LOCK_ALI,
        MS_ENUM_LOCK_JD,
        MS_ENUM_LOCK_AIRKISS,
}ms_enum_lock_chn_info_t;

typedef enum SNIFFER_STATUS_T{
	RTW_PROMISC_DISABLE = 0x00,
        RTW_PROMISC_ENABLE_2,
}sniffer_status;

struct _chplan {
        u8      ch;
        u8      bw_os;
};

struct ieee80211_radiotap_header {
        u8        it_version;     /* set to 0 */
        u8        it_pad;
        u16       it_len;         /* entire length */
        u32       it_present;     /* fields present */
} __attribute__((__packed__));

#endif /* WIFI_CONFIG_H_ */
