/*
 * =============================================================================
 *
 *       Filename:  def.h
 *
 *    Description:
 *
 *     Created on:  2015年12月15日
 *
 *     Created by:
 *
 * =============================================================================
 */

#ifndef __DEF_H__
#define __DEF_H__

#define VERSION		"0.10"
#define BUF_SIZ		10240

#define	SEND_ACK_TIMES	1000
#define ACK_DEST_PORT	10001
#define GET_IP_TIMEOUT	40			/* seconds of waiting ip address obtained */

#define CFGFILE_OPEN				0
#define CFGFILE_WPA				1
#define CFGFILE_WEP				2
//#define g_debuglevel 1
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
enum loglevel{
	ALWAYS=0,
	NOTICE=1,
	INFO=2,
	DETAIL=3,
	NOTICE_=4,
	INFO_=5,
	DETAIL_=6
};


#define LOG(level, fmt, args...)	do{		\
			if(level <= g_debuglevel || level == 0)	\
			printf(""fmt,## args);		\
			if(level > 3)	\
				if(level <= g_debuglevel)	\
					printf(""fmt,## args);		\
			}while(0)


#define	TYPE_MASK	0x0C
#define SUBTYPE_MASK	0xF0
#define TDFD_MASK	0x03
#define TYPE_DATA	0x08

struct ieee80211_radiotap_header {
        u8        it_version;     /* set to 0 */
        u8        it_pad;
        u16       it_len;         /* entire length */
        u32       it_present;     /* fields present */
} __attribute__((__packed__));



#endif	//__DEF_H__
