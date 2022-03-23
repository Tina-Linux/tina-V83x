/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 *****************************************************************************/
#ifndef __RTW_CUSDEF_H__
#define __RTW_CUSDEF_H__

/********************		Customize Part	*******************/
#define PROGRAM_VERSION			"v0.2"
#define WIFI_MODULE_NAME		"/lib/modulus/3.4.39/8723bs.ko"
#define PROC_MODULE_PATH		"rtl8723bs"
#define TIME_CHSWITCH			100000			/* 0.1 seconds of staying in each channel */
#define TIME_CHECK_IP			30			/* seconds of waiting ip address obtained */
/******************************************************************/

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned char uint8;
typedef signed char int8_t;


#define DEBUG_PRINT(fmt, args...)	do{				\
						if(g_debuglv)		\
						printf(""fmt,## args);	\
					} while(0)

#define DEBUG_PRINT2(fmt, args...) \
				if(g_debuglv == 2) \
				printf(""fmt,## args);




#ifndef NULL
#define NULL				0
#endif
#define TRUE				1
#define FALSE				0
#define OPMODE_STATION			0
#define OPMODE_MONITOR			1
#define GetAddr1Ptr(pbuf)       ((unsigned char *)(pbuf) + 4)
#define GetAddr2Ptr(pbuf)       ((unsigned char *)(pbuf) + 10)
#define GetAddr3Ptr(pbuf)       ((unsigned char *)(pbuf) + 16)
#define GetAddr4Ptr(pbuf)       ((unsigned char *)(pbuf) + 24)


struct ieee80211_radiotap_header {
        u8        it_version;     /* set to 0 */
        u8        it_pad;
        u16       it_len;         /* entire length */
        u32       it_present;     /* fields present */
} __attribute__((__packed__));
#define FRTODS_MASK		0x03
#define TYPE_MASK		0x0C
#define SUBTYPE_MASK		0xF0
#define TYPE_DATA		0x08
#define GetToDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_TO_DS_)) != 0)
#define GetFrDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_FROM_DS_)) != 0)

#define _FW_UNDER_SURVEY	0x0800
#define BUF_SIZ			10240

#endif  //__RTW_CUSDEF_H__