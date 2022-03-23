#ifndef _TDMIN_INTERFACE_H_
#define _TDMIN_INTERFACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/byteorder.h>
#include <alsa/asoundlib.h>

#define PB_ALSA_DEBUG_TAG "<playback_alsa>"
#if defined(AISPEECH_2MIC)
//#define AISPEECH_TDMIC_PCM_RECORD_DEVICE_NAME  "2mic_loopback"
#define AISPEECH_TDMIC_PCM_RECORD_DEVICE_NAME  "hw:audiocodec" //add by 926 @20190605
#define AISPEECH_TDMIC_CHANNLE		  3

#elif defined(AISPEECH_A_4MIC)
#define AISPEECH_TDMIC_PCM_RECORD_DEVICE_NAME  "4mic_loopback"
#define AISPEECH_TDMIC_CHANNLE		  5
#else

#endif

#define AISPEECH_TDMIC_SAMPLERATE    16000
#define AISPEECH_TDMIC_BITWIDTH	  16
#define PCM_SEEK_BIT    3

#define AISPEECH_FEED_DATA_LENGTH_32  AISPEECH_TDMIC_SAMPLERATE*AISPEECH_TDMIC_CHANNLE*AISPEECH_TDMIC_BITWIDTH/8/10
#define AISPEECH_FEED_DATA_LENGTH_24 AISPEECH_FEED_DATA_LENGTH_32*3/4
#define AISPEECH_FEED_DATA_LENGTH_16 AISPEECH_FEED_DATA_LENGTH_32/2

#define TDMIN_OK                            (0)
#define TDMIN_FAIL                          (-1) /* abnormal return must < 0 */
#define TDMIN_INV_ARG                       (-2)

/* Definitions for Microsoft WAVE format */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)    ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)        (v)
#define LE_INT(v)        (v)
#define BE_SHORT(v)        bswap_16(v)
#define BE_INT(v)        bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)    ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)        bswap_16(v)
#define LE_INT(v)        bswap_32(v)
#define BE_SHORT(v)        (v)
#define BE_INT(v)        (v)
#else
#error "Wrong endian"
#endif




typedef long long _off64_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef struct PCMContainer {
	snd_pcm_t *handle;              /*need to set*/
	snd_output_t *log;
	snd_pcm_uframes_t chunk_size;   /*auto calc*/
	snd_pcm_uframes_t buffer_size;  /*auto calc*/
	snd_pcm_format_t format;        /*need to set*/
	uint16_t channels;              /*need to set*/
	size_t chunk_bytes;
	size_t chunk_bytes_16;
	size_t bits_per_sample;
	size_t bits_per_frame;
	size_t sample_rate;	            /*need to set*/
	_off64_t chunk_count;            /*usb wav need to set */

	uint8_t *data_buf;
	uint8_t *data_buf_16;
} PCMContainer_t;

extern int _pcm_32_to_16(char *dst_buf, char *ori_buf, int ori_len, int fseek_bit);
extern int alsa_tdmin_init(PCMContainer_t *pcm_params);
extern void alsa_tdmin_uninit(PCMContainer_t *pcm_params);
extern int alsa_read_tdmin_pcm(PCMContainer_t *pcm_params);

#endif
