/*
 * =============================================================================
 *
 *       Filename:  sc_def.h
 *
 *    Description:
 *
 *     Created on:  2015年12月17日
 *
 *     Created by:
 *
 * =============================================================================
 */

#ifndef __SC_DEF_H__
#define __SC_DEF_H__


#define VALID_MASK		0x1FF
#define MAGIC_MASK		0x01F0
#define INFO_MASK		0x0F

#define IS_SEQ_HEADER(l)	(l & 0X180)==0x80
#define IS_SEQ_DATA(l)		l & 0X100

#define SEQ_CRC_MASK		0x7F
#define SEQ_IDX_MASK		0x7F
#define SEQ_DATA_MASK		0xFF

#define EXTRA_PREAMBLE_ADDED	256

#define FIB_MAX	15

#define FIB_MIN_MATCH 4
#define SEARCH_WIN 10
#define UPDATE_WIN 3

#define MAX_MACS 20
#define REORDER_WIN 32

#define MAGIC_CONFIRM_TIMES 4


#define USE_LOG
#ifdef USE_LOG
#define LLOG(fmt, args...) printf(""fmt,## args);
#else
#define LLOG(fmt, args...)
#endif

//#define LOG_DETAIL
#ifdef LOG_DETAIL
#define LLLOG LLOG
#else
#define LLLOG(fmt, args...)
#endif


//#define USE_BASE64_



struct magic_code_field{
	char	len_hi;			//high nibble
	char	len_lo;
	char	ssid_crc_hi;
	char	ssid_crc_lo;
};

struct prefix_code_field{
	char	psw_len_hi;
	char	psw_len_lo;
	char	psw_len_crc_hi;
	char	psw_len_crc_lo;
};

struct sequence{
	unsigned char	seq_crc8;
	unsigned char	seq_idx;
	unsigned char	data[4];
};

struct encoded_buf{
	struct magic_code_field		*magic_code;
	struct prefix_code_field	*prefix_code;
	struct sequence			seq[0];
};


typedef enum{
	PREAMBLE_DETECTING,
	MAGIC_CODE_COLLECTING,
	PREFIX_CODE_COLLCTING,
	DATA_COLLECTING,
	ERR_STATE
} decode_state_t;

typedef enum{
	EXPECTING_SEQ_CRC,
	EXPECTING_SEQ_IDX,
	EXPECTING_DATA,
} data_collect_state;

struct collected_flags{
	int	num;
	int	setted;
	char	flags[0];
};

struct confirms{
	char info;
	short cnt;
};


struct mac_uniq{
	unsigned char mac[6];
	unsigned times;
	unsigned newer_cnt;
	unsigned long long last_appeared;
	short lens[SEARCH_WIN + UPDATE_WIN];
};

struct macs{
	unsigned cur_cnt;
	struct mac_uniq mac_infos[0];
};


#endif //__SC_DEF_H__
