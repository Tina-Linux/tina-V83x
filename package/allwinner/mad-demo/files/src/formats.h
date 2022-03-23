#ifndef FORMATS_H
#define FORMATS_H

#include <endian.h>
#include <byteswap.h>

typedef struct {
        u_int magic;            /* 'RIFF' */
        u_int length;           /* filelen */
        u_int type;             /* 'WAVE' */
} WaveHeader;

typedef struct {
        u_short format;         /* see WAV_FMT_* */
        u_short channels;
        u_int sample_fq;        /* frequence of sample */
        u_int byte_p_sec;
        u_short byte_p_spl;     /* samplesize; 1 or 2 bytes */
        u_short bit_p_spl;      /* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
        WaveFmtBody format;
        u_short ext_size;
        u_short bit_p_spl;
        u_int channel_mask;
        u_short guid_format;    /* WAV_FMT_* */
        u_char guid_tag[14];    /* WAV_GUID_TAG */
} WaveFmtExtensibleBody;

typedef struct {
        u_int type;             /* 'data' */
        u_int length;           /* samplecount */
} WaveChunkHeader;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)     ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)             (v)
#define LE_INT(v)               (v)
#define BE_SHORT(v)             bswap_16(v)
#define BE_INT(v)               bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)     ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)             bswap_16(v)
#define LE_INT(v)               bswap_32(v)
#define BE_SHORT(v)             (v)
#define BE_INT(v)               (v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF                COMPOSE_ID('R','I','F','F')
#define WAV_RIFX                COMPOSE_ID('R','I','F','X')
#define WAV_WAVE                COMPOSE_ID('W','A','V','E')
#define WAV_FMT                 COMPOSE_ID('f','m','t',' ')
#define WAV_DATA                COMPOSE_ID('d','a','t','a')

#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003

#endif
