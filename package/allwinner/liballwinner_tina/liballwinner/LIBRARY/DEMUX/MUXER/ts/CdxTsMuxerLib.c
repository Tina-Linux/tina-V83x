/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTsMuxer.c
 * Description : Allwinner TS Muxer Definition
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#include "CdxTsMuxer.h"
#include "CdxTsMuxerCfg.h"

#define CODEC_TYPE_SUBTITLE 3
#define ADTS_HEADER_LENGTH 7

const unsigned int g_audio_sample_rate[13] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000,
    11025, 8000, 7350
};

static cdx_uint16 tsBsWap16(cdx_uint16 x)
{
    x= (x>>8) | (x<<8);
    return x;
}

static cdx_uint32 tsBsWap32(cdx_uint32 x)
{
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
    return x;
}

static cdx_uint64 tsBsWap64(cdx_uint64 x)
{
    union {
        cdx_uint64 ll;
        cdx_uint32 l[2];
    } w, r;
    w.ll = x;
    r.l[0] = tsBsWap32 (w.l[1]);
    r.l[1] = tsBsWap32 (w.l[0]);
    return r.ll;
}

#define TS_BE2ME_16(x) tsBsWap16(x)
#define TS_BE2ME_32(x) tsBsWap32(x)
#define TS_BE2ME_64(x) tsBsWap64(x)
#define TS_LE2ME_16(x) (x)
#define TS_LE2ME_32(x) (x)
#define TS_LE2ME_64(x) (x)


static cdx_int32 tsWriteBufferStream(ByteIOContext *pCdxStreamT, char *buf, int size)
{
#if FS_WRITER
    return pCdxStreamT->fsWrite(pCdxStreamT, buf, size);
#else
    return pCdxStreamT->cdxWrite(pCdxStreamT, buf, size);
#endif
}

int TsSectionWritePacket(MpegTSSection *s, cdx_uint8 *packet)
{
    TsMuxContext *ctx = s->opaque;
    cdx_int32 ret;

    if (ctx->is_sdcard_disappear)
    {
        return -1;
    }
#if FS_WRITER
    ret = tsWriteBufferStream(ctx->fs_writer_info.mp_fs_writer, (char*)packet, TS_PACKET_SIZE);
#else
    ret = tsWriteBufferStream(ctx->stream_writer, (char*)packet, TS_PACKET_SIZE);
#endif
    if (ret < 0)
    {
        ctx->is_sdcard_disappear = 1;
        loge("sdcard may be disappeared!");
    }
    return ret;
}

unsigned long generateCRC32(unsigned char * DataBuf,unsigned long  len)
{
    register unsigned long i;
    unsigned long crc = 0xffffffff;
    static unsigned long crc_table[256] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
        0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
        0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
        0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
        0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
        0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
        0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
        0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
        0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
        0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
        0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
        0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
        0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
        0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
        0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
        0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
        0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
        0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
        0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
        0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
        0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
        0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

    for (i = 0; i < len; i++)
    {
        crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *DataBuf++) & 0xff];
    }
    return crc;
}


static cdx_int32 tsWriteSection(MpegTSSection *s, cdx_uint8 *buf, int len)
{
    TsMuxContext* context = (TsMuxContext*)s->opaque;
    TsWriter *ts = ((TsMuxContext*)s->opaque)->priv_data;
    unsigned int crc;
    unsigned char packet[TS_PACKET_SIZE];
    const unsigned char *buf_ptr;
    unsigned char *q;

    if (context->is_sdcard_disappear)
    {
        return -1;
    }
    crc = generateCRC32(buf, len -4);

    //crc = tsBsWap32(tsAvCrc(av_crc_get_table(AV_CRC_32_IEEE), -1, buf, len - 4));
    buf[len - 4] = (crc >> 24) & 0xff;
    buf[len - 3] = (crc >> 16) & 0xff;
    buf[len - 2] = (crc >> 8) & 0xff;
    buf[len - 1] = (crc) & 0xff;

    /* send each packet */
    buf_ptr = buf;
    while (len > 0)
    {
        int first, b, len1, left;
        first = (buf == buf_ptr);
        q = packet;
        *q++ = 0x47;
        b = (s->pid >> 8);
        if (first)
        {
            b |= 0x40;
        }
        *q++ = b;
        *q++ = s->pid;
        s->cc = (s->cc + 1) & 0xf;
        *q++ = 0x10 | s->cc;//do not write adaption section
        if (first)
        {
            *q++ = 0; /* 0 offset */
        }
        len1 = TS_PACKET_SIZE - (q - packet);
        if (len1 > len)
        {
            len1 = len;
        }
        memcpy(q, buf_ptr, len1);
        q += len1;
        /* add known padding data */
        left = TS_PACKET_SIZE - (q - packet);
        if (left > 0)
        {
            memset(q, 0xff, left);
        }

        ts->cache_size += TS_PACKET_SIZE;
        ts->cache_size_total += TS_PACKET_SIZE;

        memcpy(ts->ts_write_ptr,packet,TS_PACKET_SIZE);
        ts->ts_write_ptr += TS_PACKET_SIZE;
        if(ts->ts_write_ptr>ts->ts_cache_end)
        {
            ts->ts_write_ptr = ts->ts_cache_start;
        }

        //save bitstream to file if need, when use output bitstream in callback
        //just cache the buffer
        if(context->output_buffer_mode != OUTPUT_CALLBACK_BUFFER)
        {
            if(ts->cache_size >= 1024*TS_PACKET_SIZE)
            {
                cdx_int32 ret;
#if FS_WRITER
                ret = tsWriteBufferStream(context->fs_writer_info.mp_fs_writer,(char*)ts->ts_read_ptr,
                                    TS_PACKET_SIZE * 128);
#else
                ret = tsWriteBufferStream(context->stream_writer,(char*)ts->ts_read_ptr,
                                    TS_PACKET_SIZE * 128);
#endif
                if (ret < 0)
                {
                    context->is_sdcard_disappear = 1;
                    loge("(f:%s, l:%d) sdcard may be disappeared!", __FUNCTION__, __LINE__);
                    return ret;
                }
                ts->ts_read_ptr += TS_PACKET_SIZE*128;
                ts->cache_size = ts->cache_size - TS_PACKET_SIZE*128;
                ts->cache_page_num ++;
                if(ts->ts_read_ptr > ts->ts_cache_end)
                {
                    ts->ts_read_ptr = ts->ts_cache_start;
                }
            }
        }

        buf_ptr += len1;
        len -= len1;
    }
    return 0;
}

static void tsPut16Bits(cdx_uint8 **q_ptr, int val)
{
    cdx_uint8 *q;
    q = *q_ptr;
    *q++ = val >> 8;
    *q++ = val;
    *q_ptr = q;
}

static int tsWriteSection1(MpegTSSection *s, int tid, int id,
                          int version, int sec_num, int last_sec_num,
                          cdx_uint8 *buf, int len)
{
    cdx_uint8 section[1024], *q;
    unsigned int tot_len;

    tot_len = 3 + 5 + len + 4;
    /* check if not too big */
    if (tot_len > 1024)
    {
        return -1;
    }

    q = section;
    *q++ = tid;//write table_id
    tsPut16Bits(&q, 0xb000 | (len + 5 + 4)); /* 5 byte header + 4 byte CRC */
    tsPut16Bits(&q, id);
    *q++ = 0xc1 | (version << 1); /* current_next_indicator = 1 */
    *q++ = sec_num;
    *q++ = last_sec_num;
    memcpy(q, buf, len);

    if (tsWriteSection(s, section, tot_len) < 0)
    {
        loge("(f:%s, l:%d) tsWriteSection() failed", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static void tsPutLe32Bits(cdx_uint8 *s, cdx_uint32 val)
{
    cdx_uint8 i;
    for(i=0;i<4;i++)
        *s++ = (cdx_uint8)(val>>i);
}

static void tsPutBe32Bits(cdx_uint8 *s, cdx_uint32 val)
{
    cdx_uint8 i;
    val= ((val<<8)&0xFF00FF00) | ((val>>8)&0x00FF00FF);
    val= (val>>16) | (val<<16);
    for(i=0;i<4;i++)
    {
        *s++ = (cdx_int8)(val>>(i<<3));
    }
}

//add for h264encoder
#define TS_AV_RB32(x)  ((((const cdx_uint8*)(x))[0] << 24) | \
                     (((const cdx_uint8*)(x))[1] << 16) | \
                     (((const cdx_uint8*)(x))[2] <<  8) | \
                      ((const cdx_uint8*)(x))[3])


static cdx_int32 tsAvRescaleRnd(cdx_int64 a, cdx_int64 b, cdx_int64 c)
{
    return (a * b + c-1)/c;
}

static cdx_int32 tsWritePat(TsMuxContext *s)
{
    TsWriter *ts = s->priv_data;
    cdx_uint8 data[1012], *q;
    int i;
    cdx_int32 ret;
    q = data;
    for(i = 0; i < ts->nb_services; i++)
    {
        MpegTSService *service = ts->services[i];
        tsPut16Bits(&q, service->sid);
        tsPut16Bits(&q, 0xe000 | service->pmt.pid);
    }
    ret = tsWriteSection1(&ts->pat, PAT_TID, ts->tsid, 1, 0, 0, data, q - data);
    if (ret < 0)
    {
        loge("(f:%s, l:%d) FileWriter() failed", __FUNCTION__, __LINE__);
    }
    return ret;
}

static cdx_int32 tsWritePmt(TsMuxContext *s, MpegTSService *service)
{
    // TsWriter *ts = s->priv_data;
    cdx_uint8 data[1012], *q,  *program_info_length_ptr;
    int val, stream_type=-1, i;
    cdx_int32 ret;

    q = data;
    tsPut16Bits(&q, 0xe000 | service->pcr_pid);

    program_info_length_ptr = q;
    q += 2; /* patched after */

    /* put program info here */
    val = 0xf000 | (q - program_info_length_ptr - 2);
    program_info_length_ptr[0] = val >> 8;
    program_info_length_ptr[1] = val;

    for(i = 0; i < s->nb_streams; i++)
    {
        AVStream *st = s->streams[i];
        MpegTSWriteStream *ts_st = st->priv_data;
        cdx_uint8 *desc_length_ptr;
        switch(st->codec.codec_id)
        {
            case MUX_CODEC_ID_H264:
                stream_type = STREAM_TYPE_VIDEO_H264;
                break;
            case MUX_CODEC_ID_AAC:
                stream_type = STREAM_TYPE_AUDIO_AAC;
                break;
            case MUX_CODEC_ID_ADPCM:
                stream_type = STREAM_TYPE_AUDIO_LPCM;
                break;
            default:
                loge("error type");
        }
        *q++ = stream_type;
        tsPut16Bits(&q, 0xe000 | ts_st->pid);
        desc_length_ptr = q;

        q += 2; /* patched after */

        if(stream_type == STREAM_TYPE_AUDIO_LPCM)
        {
            desc_length_ptr[2] = 0x83;
            desc_length_ptr[3] = 0x02;
            desc_length_ptr[4] = 0x46;
            desc_length_ptr[5] = 0x3F;
            q += 4; // add by fangning 2012-12-20 16:55
        }

        val = 0xf000 | (q - desc_length_ptr - 2);
        desc_length_ptr[0] = val >> 8;
        desc_length_ptr[1] = val;
    }
    ret = tsWriteSection1(&service->pmt, PMT_TID, service->sid, 1, 0, 0,
                          data, q - data);
    if (ret < 0)
    {
        loge("(f:%s, l:%d) tsWriteSection1() failed", __FUNCTION__, __LINE__);
    }
    return ret;
}

static cdx_int32 tsWritePcrTable(TsMuxContext *s, cdx_int64 pts)
{
    //TsMuxContext* context = (TsMuxContext*)s->opaque;
    TsWriter *ts = s->priv_data;

    if (s->is_sdcard_disappear)
    {
        return -1;
    }

    unsigned char buffer[188];
    long long pcr = pts; //fix it later, pcr 27MHZ

    memset(buffer, 0xff, 188);

    buffer[0] = 0x47;
    buffer[1] = 0x50;
    buffer[2] = 0x00; //pcr pid must be 0x1000, used by wifi display
    buffer[3] = 0x20 | s->pcr_counter ++; //wirte adaptation field flag and pcr table counter

    s->pcr_counter &= 0x0f;

    buffer[4] = 0xB7; //wirte adaptation field length
    buffer[5] = 0x10; //set pcr flag

    buffer[6] = pcr >> 25;
    buffer[7] = pcr >> 17;
    buffer[8] = pcr >> 9;
    buffer[9] = pcr >> 1;
    buffer[10] = (pcr & 1) << 7;
    buffer[11] = 0;

    ts->cache_size += TS_PACKET_SIZE;
    ts->cache_size_total += TS_PACKET_SIZE;

    memcpy(ts->ts_write_ptr, buffer, TS_PACKET_SIZE);
    ts->ts_write_ptr += TS_PACKET_SIZE;

    if(ts->ts_write_ptr>ts->ts_cache_end)
    {
         ts->ts_write_ptr = ts->ts_cache_start;
    }

    //save bitstream to file if need, when use output bitstream in callback
    //just cache the buffer
    if(s->output_buffer_mode != OUTPUT_CALLBACK_BUFFER)
    {
        if(ts->cache_size >= 1024*TS_PACKET_SIZE)
        {
            cdx_int32 ret;
#if FS_WRITER
            ret = tsWriteBufferStream(s->fs_writer_info.mp_fs_writer, (char*)ts->ts_read_ptr,
                                TS_PACKET_SIZE * 128);
#else
            ret = tsWriteBufferStream(s->stream_writer, (char*)ts->ts_read_ptr,
                                TS_PACKET_SIZE * 128);
#endif
            if (ret < 0)
            {
                s->is_sdcard_disappear = 1;
                loge("(f:%s, l:%d) sdcard may be disappeared!", __FUNCTION__, __LINE__);
                return ret;
            }
            ts->ts_read_ptr += TS_PACKET_SIZE*128;
            ts->cache_size = ts->cache_size - TS_PACKET_SIZE*128;
            ts->cache_page_num ++;
            if(ts->ts_read_ptr > ts->ts_cache_end)
            {
                ts->ts_read_ptr = ts->ts_cache_start;
            }
        }
    }
    return 0;
}

static cdx_int32 tsRetransmitSiInfo(TsMuxContext *s, cdx_int64 pts, int idx)
{
    TsWriter *ts = s->priv_data;
    int i;

    if(s->output_buffer_mode == OUTPUT_CALLBACK_BUFFER)
    {
        if(s->pat_pmt_counter == 4 || s->pat_pmt_flag == 1)
        {
            s->pat_pmt_counter = 0;
            s->pat_pmt_flag = 0;

            if (tsWritePat(s) < 0)
            {
                loge("(f:%s, l:%d) tsWritePat() failed", __FUNCTION__, __LINE__);
                return -1;
            }

            for(i = 0; i < ts->nb_services; i++)
            {
                if (tsWritePmt(s, ts->services[i]) < 0)
                {
                    loge("(f:%s, l:%d) tsWritePmt() failed", __FUNCTION__, __LINE__);
                    return -1;
                }
            }
        }
        if (idx == CODEC_TYPE_VIDEO)
        {
            if (tsWritePcrTable(s, pts) < 0)
            {
                loge("(f:%s, l:%d) tsWritePcrTable() failed", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    }
    else if (ts->pat_packet_count == ts->pat_packet_period)
    {
        ts->pat_packet_count = 0;

        if (tsWritePat(s) < 0)
        {
            loge("(f:%s, l:%d) tsWritePat() failed", __FUNCTION__, __LINE__);
            return -1;
        }
        for(i = 0; i < ts->nb_services; i++)
        {
            if (tsWritePmt(s, ts->services[i]) < 0)
            {
                loge("(f:%s, l:%d) tsWritePat() failed", __FUNCTION__, __LINE__);
                return -1;
            }
        }
        if (tsWritePcrTable(s, pts) < 0)
        {
            loge("(f:%s, l:%d) tsWritePcrTable() failed", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    return 0;
}

static void tsWritePts(cdx_uint8 *q, int fourbits, cdx_int64 pts)
{
    int val;

    val = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;
    *q++ = val;
    val = (((pts >> 15) & 0x7fff) << 1) | 1;
    *q++ = val >> 8;
    *q++ = val;
    val = (((pts) & 0x7fff) << 1) | 1;
    *q++ = val >> 8;
    *q++ = val;
}

/* Add a pes header to the front of payload, and segment into an integer number of
 * ts packets. The final ts packet is padded using an over-sized adaptation header
 * to exactly fill the last ts packet.
 * NOTE: 'payload' contains a complete PES payload.
 */

static cdx_int32 tsWritePes(TsMuxContext *s, AVStream *st,
                             const cdx_uint8 *payload, int payload_size,
                             cdx_int64 pts, cdx_int64 dts,cdx_int32 idx)
{
    MpegTSWriteStream *ts_st = st->priv_data;
    TsWriter *ts = s->priv_data;
    cdx_uint8 buf[TS_PACKET_SIZE];
    cdx_uint8 *q;
    int is_start, len, header_len, private_code, flags, afc_len;
    cdx_int64 pcr = -1; /* avoid warning */
    // cdx_int64 delay = tsAvRescaleRnd(s->max_delay, 90000, AV_TIME_BASE);
    int stuffingbytes;

    if (s->is_sdcard_disappear)
    {
        return -1;
    }

    is_start = 1;
    if (tsRetransmitSiInfo(s, pts, idx) < 0)
    {
        loge("(f:%s, l:%d) tsRetransmitSiInfo() failed", __FUNCTION__, __LINE__);
        return -1;
    }

    ts->pat_packet_count++;

    while (payload_size > 0)
    {
        int val, write_pcr, stuffing_len;

        write_pcr = 0;

        /* prepare packet header */
        q = buf;
        *q++ = 0x47;
        val = (ts_st->pid >> 8);
        if (is_start)
        {
            val |= 0x40;
        }
        *q++ = val;
        *q++ = ts_st->pid;
        ts_st->cc = (ts_st->cc + 1) & 0xf;
        *q++ = 0x10 | ts_st->cc | (write_pcr ? 0x20 : 0);

        if (write_pcr)
        {
            *q++ = 7; /* AFC length */
            *q++ = 0x10; /* flags: PCR present */
            *q++ = pcr >> 25;
            *q++ = pcr >> 17;
            *q++ = pcr >> 9;
            *q++ = pcr >> 1;
            *q++ = (pcr & 1) << 7;
            *q++ = 0;
        }

        if (is_start)
        {
            /* write PES header */
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x01;
            private_code = 0;
            if (st->codec.codec_id == MUX_CODEC_ID_H264)
            {
                *q++ = 0xe0; //for h264
            }
            else if(st->codec.codec_id == MUX_CODEC_ID_ADPCM)
            {
                *q++ = 0xBD; //for LPCM
            }
            else
            {
                *q++ = 0xc0; //for aac
            }
            header_len = 0;
            flags = 0;

            stuffingbytes = 2;
            if (pts != -1)
            {
                header_len += 5; // packet_len 2, val 1, flags 1, header_len 1

                header_len += stuffingbytes;
                flags |= 0x80;
            }

            len = payload_size + header_len + 3; // stream id is not inculded
            if (private_code != 0)
            {
                len++;
            }
            if (len > 0xffff)
            {
                len = 0;
            }
            *q++ = len >> 8;
            *q++ = len;
           // val = 0x84;
            val = 0x80;

            /* data alignment indicator is required for subtitle data */
            if (st->codec.codec_type == CODEC_TYPE_SUBTITLE)
            {
                val |= 0x04;
            }
            *q++ = val;
            *q++ = flags;
            *q++ = header_len;
            if (pts != -1)
            {
                tsWritePts(q, flags >> 6, pts);
                q += 5;
            }

            //stuffing bytes
            *q++ = 0xFF;
            *q++ = 0xFF;

            if (private_code != 0)
            {
                *q++ = private_code;
            }

            is_start = 0;
        }
        /* header size */
        header_len = q - buf; // real header_len
        /* data len */
        len = TS_PACKET_SIZE - header_len;
        if (len > payload_size)
        {
            len = payload_size;
        }
        stuffing_len = TS_PACKET_SIZE - header_len - len;
        if (stuffing_len > 0)
        {
            /* add stuffing with AFC */
            if (buf[3] & 0x20)
            {
                /* stuffing already present: increase its size */
                afc_len = buf[4] + 1;//afc_len
                //memmove = memcpy(s,d,n);
                memmove(buf + 4 + afc_len + stuffing_len,
                        buf + 4 + afc_len,header_len - (4 + afc_len));
                buf[4] += stuffing_len;
                memset(buf + 4 + afc_len, 0xff, stuffing_len);
            }
            else
            {
                /* add stuffing */
                memmove(buf + 4 + stuffing_len, buf + 4, header_len - 4);
                buf[3] |= 0x20;
                buf[4] = stuffing_len - 1;
                if (stuffing_len >= 2)
                {
                    buf[5] = 0x00;
                    memset(buf + 6, 0xff, stuffing_len - 2);
                }
            }
        }
        memcpy(buf + TS_PACKET_SIZE - len, payload, len);
        payload += len;
        payload_size -= len;
        ts->cache_size += TS_PACKET_SIZE;
        ts->cache_size_total += TS_PACKET_SIZE;

        memcpy(ts->ts_write_ptr,buf,TS_PACKET_SIZE);
        ts->ts_write_ptr += TS_PACKET_SIZE;
        if(ts->ts_write_ptr>ts->ts_cache_end)
        {
             ts->ts_write_ptr = ts->ts_cache_start;
        }

        //save output bitstream to file
        if(s->output_buffer_mode != OUTPUT_CALLBACK_BUFFER)
        {
            if(ts->cache_size >= 1024*TS_PACKET_SIZE)
            {
                cdx_int32 ret;
#if FS_WRITER
                ret = tsWriteBufferStream(s->fs_writer_info.mp_fs_writer,
                                    (char*)ts->ts_read_ptr, TS_PACKET_SIZE * 128);
#else
                ret = tsWriteBufferStream(s->stream_writer, (char*)ts->ts_read_ptr,
                                    TS_PACKET_SIZE * 128);
#endif
                if (ret < 0)
                {
                    s->is_sdcard_disappear = 1;
                    loge("(f:%s, l:%d) sdcard may be disappear!", __FUNCTION__, __LINE__);
                    return ret;
                }
                ts->ts_read_ptr += TS_PACKET_SIZE * 128;
                ts->cache_size = ts->cache_size - TS_PACKET_SIZE * 128;
                ts->cache_page_num ++;
                if(ts->ts_read_ptr > ts->ts_cache_end)
                {
                    ts->ts_read_ptr = ts->ts_cache_start;
                }
            }
        }
    }
    return 0;
}

cdx_int32 TsWritePacket(TsMuxContext *s, CdxMuxerPacketT *pkt)
{
    int size = 0;
    cdx_uint8 *buf_tmp = s->pes_buffer;
    AVStream *st = s->streams[pkt->streamIndex];

    if(st->codec.codec_id == MUX_CODEC_ID_H264)
    {
        int state = 0;
        int total_size = 0;
        s->pat_pmt_counter++;

        if ((state & 0x1f) != 9)
        {
            cdx_uint8 data[6];

            // AUD NAL// state & 0x1f = 1
            tsPutBe32Bits(data, 0x00000001);
            data[4] = 0x09; // access_unit_delimiter of H.264
            data[5] = 0xf0; // primary_pic_type == 7
            memcpy(buf_tmp, data, 6);
            buf_tmp += 6;
        }

        if(st->firstframeflag)
        {
            memcpy(buf_tmp, st->vos_data, st->vos_len);
            buf_tmp +=st->vos_len;
            st->firstframeflag = 0;
        }

        total_size = buf_tmp - s->pes_buffer + pkt->buflen;
        if (s->pes_bufsize < total_size)
        {
            int offset = buf_tmp - s->pes_buffer;
            logd("<F:%s, L:%d> realloc pes_buffer [%d]->[%d]",
                 __FUNCTION__, __LINE__, s->pes_bufsize, total_size + 32 * 1024);
            s->pes_buffer = (cdx_uint8*)realloc(s->pes_buffer, total_size + 32 * 1024);
            if (s->pes_buffer == NULL)
            {
                loge("<F:%s, L:%d> realloc pes_buffer error!!", __FUNCTION__, __LINE__);
                return -1;
            }
            s->pes_bufsize = total_size+32*1024;
            buf_tmp = s->pes_buffer + offset;
        }

        if(pkt->buflen)
        {
            memcpy(buf_tmp, pkt->buf, pkt->buflen);
            buf_tmp += pkt->buflen;
        }

        size = buf_tmp - s->pes_buffer;

        if(s->first_video_pts == 1)
        {
            s->first_video_pts = 0;
            s->base_video_pts = pkt->pts;
        }

        pkt->pts = (pkt->pts - s->base_video_pts) * 90;
    }
    else if (st->codec.codec_id == MUX_CODEC_ID_AAC)
    {
        unsigned int channels;
        unsigned int frame_length;
        unsigned int obj_type;
        unsigned int num_data_block;
        cdx_uint8 adts_header[ADTS_HEADER_LENGTH];
        int i;
        int total_size = ADTS_HEADER_LENGTH + pkt->buflen;

        if (s->pes_bufsize < total_size)
        {
            logd("<F:%s, L:%d> realloc pes_buffer [%d]->[%d]",
                 __FUNCTION__, __LINE__, s->pes_bufsize, total_size + 32 * 1024);
            s->pes_buffer = (cdx_uint8*)realloc(s->pes_buffer, total_size + 32 * 1024);
            if (s->pes_buffer == NULL)
            {
                loge("<F:%s, L:%d> realloc pes_buffer error!!", __FUNCTION__, __LINE__);
                return 0;
            }
            s->pes_bufsize = total_size+32*1024;
            buf_tmp = s->pes_buffer;
        }

        obj_type = AAC_PROFILE_LC; //AACLC
        channels = st->codec.channels;

        frame_length = pkt->buflen + ADTS_HEADER_LENGTH;
        num_data_block = frame_length / 1024;

        for(i=0;i<13;i++)
        {
            if((unsigned int)st->codec.sample_rate == g_audio_sample_rate[i])
            {
                break;
            }
        }

        adts_header[0] = 0xFF;
        //adts_header[1] = 0xF9; //mpeg-4 aac
        adts_header[1] = 0xF1;  //mpeg-2 aac

        adts_header[2] = obj_type << 6;
        adts_header[2] |= (i << 2);
        adts_header[2] |= (channels & 0x4) >> 2;
        adts_header[3] = (channels & 0x3) << 6;

        //adts_header[3] |= (frame_length & 0x1800) >> 15;
        adts_header[3] |= (frame_length & 0x1800) >> 11;
        adts_header[4] = (frame_length & 0x1FF8) >> 3;
        adts_header[5] = (frame_length & 0x7) << 5;
        adts_header[5] |= 0x1F;

        adts_header[6] = 0xFC;// one raw data blocks .
        adts_header[6] |= num_data_block & 0x03; //Set raw Data blocks.

        memcpy(buf_tmp, adts_header, ADTS_HEADER_LENGTH);

        buf_tmp += ADTS_HEADER_LENGTH;
        memcpy(buf_tmp, pkt->buf, pkt->buflen);

        pkt->pts = s->audio_frame_num * 90000 * 1024 / st->codec.sample_rate;
        s->audio_frame_num ++;
        size = frame_length;
    }
    else if(st->codec.codec_id == MUX_CODEC_ID_ADPCM)
    {
        int total_size = pkt->buflen;

        if (s->pes_bufsize < total_size)
        {
            logd("<F:%s, L:%d> realloc pes_buffer [%d]->[%d]",
                 __FUNCTION__, __LINE__, s->pes_bufsize, total_size + 32 * 1024);
            s->pes_buffer = (cdx_uint8*)realloc(s->pes_buffer, total_size + 32 * 1024);
            if (s->pes_buffer == NULL)
            {
                loge("<F:%s, L:%d> realloc pes_buffer error!!", __FUNCTION__, __LINE__);
                return 0;
            }
            s->pes_bufsize = total_size+32*1024;
            buf_tmp = s->pes_buffer;
        }
        memcpy(buf_tmp, pkt->buf, pkt->buflen);
        size = pkt->buflen;

        pkt->pts = pkt->pts*90;
        s->audio_frame_num ++;
    }
    else
    {
        loge("unsupport codec");
        return 0;
    }

    if (tsWritePes(s, st, s->pes_buffer, size, pkt->pts, 0, pkt->streamIndex) < 0)
    {
        return -1;
    }
    return 0;
}

cdx_int32 TsWriteTrailer(TsMuxContext *s)
{
    TsWriter *ts = s->priv_data;
    cdx_uint32 i;

    if (s->output_buffer_mode == OUTPUT_CALLBACK_BUFFER)
    {
        return 0;
    }

    /* flush current packets */
    for(i = ts->cache_page_num*128;i<ts->cache_size_total/TS_PACKET_SIZE;i++)
    {
        cdx_int32 ret;
        if(ts->ts_read_ptr > ts->ts_cache_end)
        {
            ts->ts_read_ptr = ts->ts_cache_start;
        }
#if FS_WRITER
        ret = tsWriteBufferStream(s->fs_writer_info.mp_fs_writer, (char*)ts->ts_read_ptr,
            TS_PACKET_SIZE);
#else
        ret = tsWriteBufferStream(s->stream_writer, (char*)ts->ts_read_ptr, TS_PACKET_SIZE);
#endif
        if (ret < 0)
        {
            s->is_sdcard_disappear = 1;
            loge("(f:%s, l:%d) tsWriteBufferStream() failed", __FUNCTION__, __LINE__);
            return ret;
        }
        ts->ts_read_ptr += TS_PACKET_SIZE;
    }

    ts->ts_write_ptr = ts->ts_read_ptr = ts->ts_cache_start;
    ts->cache_size = 0;
    ts->cache_page_num = 0;
    ts->cache_size_total = 0;

    return 0;
}
