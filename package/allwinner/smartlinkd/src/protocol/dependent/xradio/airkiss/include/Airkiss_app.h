/******************************************************************************
 *
 * AllwinnerTeck
 *
 *****************************************************************************/
#ifndef __AIRKISS_APP_H__
#define __AIRKISS_APP_H__

/********************		Customize Part	*******************/

#define PROGRAM_VERSION			"v0.1"

#define SCAN_FILE_PATH		"/tmp/scan.txt"
#define IF_NAME             "wlan0"


#define UDP_TX_PORT			(10000)
#define UDP_RX_PORT			(10000)


#define T_MONITOR_MODE		(60)//second//60
#define T_ONE_CHANNEL_SCAN   (400 * 1000)//us, timeout for one channel scanning
#define T_SHORT_CHANNEL_SCAN (100 * 1000)//us, timeout for one channel scanning

#define T_ALL_CHANNEL_SCAN	(T_MONITOR_MODE)
#define T_SELECT_TIMEOUT	(T_SHORT_CHANNEL_SCAN)//us
#define T_CHANNEL_LOCK_TIMEOUT	(4 * 1000)//ms//4

#define DEFAULT_IF              "wlan0"
#define DEFAULT_WPAFILE_PATH    "/tmp/wpa.conf"
#define DEFAULT_AK_KEY          "1234567890123456"
#define ACK_DEST_PORT           10000
#define SEND_ACK_TIMES          30

#define CFGFILE_OPEN 0
#define CFGFILE_WPA	 1
#define CFGFILE_WEP 2


#define SUCCESS     1
#define ERROR -1


#define	PATTERN_WEP     "[WEP]"
#define	PATTERN_WPA	    "[WPA]"
#define	PATTERN_WPA2    "[WPA2]"
#define	PATTERN_WPS	    "[WPS]"
#define	PATTERN_IBSS    "[IBSS]"
#define	PATTERN_ESS	    "[ESS]"
#define	PATTERN_P2P	    "[P2P]"
#define FLAG_WPA		0x0001
#define FLAG_WPA2		0x0002
#define FLAG_WEP		0x0004
#define FLAG_WPS		0x0008
#define FLAG_IBSS		0x0010
#define FLAG_ESS		0x0020
#define FLAG_P2P		0x0040
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

enum {
    LOGDEF_NONE,
    LOGDEF_MSG,
    LOGDEF_DEBUG

};


#define LOG_DEBUG(fmt, args...)		\
    do {			\
        if ( g_log_enable == LOGDEF_DEBUG )	\
        {   \
            printf(fmt, ##args);\
            printf("[D/%s] %s,line:%d: ", LOG_TAG, __FUNCTION__, __LINE__);\
            printf("\n");\
        }  \
    }while(0);


#define LOG_MSG(fmt, args...)		\
    do {            \
        if ( g_log_enable >= LOGDEF_MSG )   \
        {    \
            printf(fmt, ##args);\
            printf("\n");\
        }   \
    }while(0);




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




#define FRTODS_MASK		0x03
#define TYPE_MASK		0x0C
#define SUBTYPE_MASK		0xF0
#define TYPE_DATA		0x08
#define GetToDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_TO_DS_)) != 0)
#define GetFrDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_FROM_DS_)) != 0)

#define _FW_UNDER_SURVEY	0x0800


#endif
