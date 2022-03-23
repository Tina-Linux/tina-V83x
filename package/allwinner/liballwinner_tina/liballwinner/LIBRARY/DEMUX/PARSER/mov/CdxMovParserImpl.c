/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxMovParserImpl.c
 * Description :
 * History :
 *
 */

#include <stdint.h>
#include "CdxMovParser.h"
#include "mpeg4Vol.h"
#include <errno.h>
#include <stdint.h>

#ifdef __ANDROID__
#define __ZLIB
#endif

#ifndef UINT_MAX
#define UINT_MAX     0xffffffff
#endif

#ifdef __ZLIB
#include <zconf.h>
#include <zlib.h>
#endif

#define KEY_FRAME_PTS_CHECK     (0)
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#define InterlaceMode

#define ABS_EDIAN_FLAG_MASK         ((unsigned int)(1<<16))
#define ABS_EDIAN_FLAG_LITTLE       ((unsigned int)(0<<16))
#define ABS_EDIAN_FLAG_BIG          ((unsigned int)(1<<16))

#define ID3v1_GENRE_MAX 147

#define MAX_READ_LEN_ONCE  5*1024*1024

/* See Genre List at http://id3.org/id3v2.3.0 */
const char * const ff_id3v1_genre_str[ID3v1_GENRE_MAX + 1] = {
      [0] = "Blues",
      [1] = "Classic Rock",
      [2] = "Country",
      [3] = "Dance",
      [4] = "Disco",
      [5] = "Funk",
      [6] = "Grunge",
      [7] = "Hip-Hop",
      [8] = "Jazz",
      [9] = "Metal",
     [10] = "New Age",
     [11] = "Oldies",
     [12] = "Other",
     [13] = "Pop",
     [14] = "R&B",
     [15] = "Rap",
     [16] = "Reggae",
     [17] = "Rock",
     [18] = "Techno",
     [19] = "Industrial",
     [20] = "Alternative",
     [21] = "Ska",
     [22] = "Death Metal",
     [23] = "Pranks",
     [24] = "Soundtrack",
     [25] = "Euro-Techno",
     [26] = "Ambient",
     [27] = "Trip-Hop",
     [28] = "Vocal",
     [29] = "Jazz+Funk",
     [30] = "Fusion",
     [31] = "Trance",
     [32] = "Classical",
     [33] = "Instrumental",
     [34] = "Acid",
     [35] = "House",
     [36] = "Game",
     [37] = "Sound Clip",
     [38] = "Gospel",
     [39] = "Noise",
     [40] = "AlternRock",
     [41] = "Bass",
     [42] = "Soul",
     [43] = "Punk",
     [44] = "Space",
     [45] = "Meditative",
     [46] = "Instrumental Pop",
     [47] = "Instrumental Rock",
     [48] = "Ethnic",
     [49] = "Gothic",
     [50] = "Darkwave",
     [51] = "Techno-Industrial",
     [52] = "Electronic",
     [53] = "Pop-Folk",
     [54] = "Eurodance",
     [55] = "Dream",
     [56] = "Southern Rock",
     [57] = "Comedy",
     [58] = "Cult",
     [59] = "Gangsta",
     [60] = "Top 40",
     [61] = "Christian Rap",
     [62] = "Pop/Funk",
     [63] = "Jungle",
     [64] = "Native American",
     [65] = "Cabaret",
     [66] = "New Wave",
     [67] = "Psychadelic", /* sic, the misspelling is used in the specification */
     [68] = "Rave",
     [69] = "Showtunes",
     [70] = "Trailer",
     [71] = "Lo-Fi",
     [72] = "Tribal",
     [73] = "Acid Punk",
     [74] = "Acid Jazz",
     [75] = "Polka",
     [76] = "Retro",
     [77] = "Musical",
     [78] = "Rock & Roll",
     [79] = "Hard Rock",
     [80] = "Folk",
     [81] = "Folk-Rock",
     [82] = "National Folk",
     [83] = "Swing",
     [84] = "Fast Fusion",
     [85] = "Bebob",
     [86] = "Latin",
     [87] = "Revival",
     [88] = "Celtic",
     [89] = "Bluegrass",
     [90] = "Avantgarde",
     [91] = "Gothic Rock",
     [92] = "Progressive Rock",
     [93] = "Psychedelic Rock",
     [94] = "Symphonic Rock",
     [95] = "Slow Rock",
     [96] = "Big Band",
     [97] = "Chorus",
     [98] = "Easy Listening",
     [99] = "Acoustic",
    [100] = "Humour",
    [101] = "Speech",
    [102] = "Chanson",
    [103] = "Opera",
    [104] = "Chamber Music",
    [105] = "Sonata",
    [106] = "Symphony",
    [107] = "Booty Bass",
    [108] = "Primus",
    [109] = "Porn Groove",
    [110] = "Satire",
    [111] = "Slow Jam",
    [112] = "Club",
    [113] = "Tango",
    [114] = "Samba",
    [115] = "Folklore",
    [116] = "Ballad",
    [117] = "Power Ballad",
    [118] = "Rhythmic Soul",
    [119] = "Freestyle",
    [120] = "Duet",
    [121] = "Punk Rock",
    [122] = "Drum Solo",
    [123] = "A capella",
    [124] = "Euro-House",
    [125] = "Dance Hall",
    [126] = "Goa",
    [127] = "Drum & Bass",
    [128] = "Club-House",
    [129] = "Hardcore",
    [130] = "Terror",
    [131] = "Indie",
    [132] = "BritPop",
    [133] = "Negerpunk",
    [134] = "Polsk Punk",
    [135] = "Beat",
    [136] = "Christian Gangsta",
    [137] = "Heavy Metal",
    [138] = "Black Metal",
    [139] = "Crossover",
    [140] = "Contemporary Christian",
    [141] = "Christian Rock",
    [142] = "Merengue",
    [143] = "Salsa",
    [144] = "Thrash Metal",
    [145] = "Anime",
    [146] = "JPop",
    [147] = "SynthPop",
};

#if 0
static CDX_S32 GetByte(CdxStreamT *s)
{
    CDX_U8 t;
    CDX_S32 t1;
    int result;
    result = CdxStreamRead(s, &t, 1);
    if(result == 0)
        return 0;
    t1 = (CDX_S32)t;
    return t1;
}
#endif

static CDX_U32 MoovGetLe16(unsigned char *s)
{
    CDX_U32 val;
    val = (CDX_U32)(*s);
    s += 1;
    val |= (CDX_U32)(*s) << 8;
    return val;
}

static CDX_U32 MoovGetLe32(unsigned char *s)
{
    CDX_U32 val;
    val = MoovGetLe16(s);
    s += 2;
    val |= MoovGetLe16(s) << 16;
    return val;
}
static CDX_U32 MoovGetBe16(unsigned char *s)
{
    CDX_U32 val;
    val = (CDX_U32)(*s) << 8;
    s += 1;
    val |= (CDX_U32)(*s);
    return val;
}

static CDX_U32 MoovGetBe24(unsigned char *s)
{
    CDX_U32 val;
    val = MoovGetBe16(s) << 8;
    s += 2;
    val |= (CDX_U32)(*s);
    return val;
}

static CDX_U32 MoovGetBe32(unsigned char *s)
{
    CDX_U32 val;
    val = MoovGetBe16(s) << 16;
    s += 2 ;
    val |= MoovGetBe16(s);
    return val;
}

static CDX_S64 MoovGetBe64(unsigned char *s)
{
    CDX_S64 val;
    val = MoovGetBe32(s);
    s += 4;
    val = val<<32;
    val = val | MoovGetBe32(s);

    return val;
}

/************************************* read the table data *********************************/
static CDX_U32 ReadStsc(MOVContext *c, MOVStreamContext *st, cdx_uint32 idx)
//bytes of offset from STSC start
{
    CDX_U32 Cache_Data;
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = 0;

#if 0
    if(idx >= st->stsc_size*12)
    {
        CDX_LOGW("index<%d> is large than stsc_size<%d>, maybe error", idx/12, st->stsc_size);
        //return INT_MAX;
    }
#endif
    offset = st->stsc_offset + idx;
    Cache_Data = MoovGetBe32(buffer+offset);

    return    Cache_Data;
}

static CDX_U32 ReadStsz(MOVContext *c, MOVStreamContext *st, cdx_uint32 idx)
    //bytes of offset from STSZ start
{
    CDX_U32 Cache_Data;
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = 0;

    #if 0
    if(idx >= st->stsz_size*4)
    {
        CDX_LOGW("index<%d> is large than stsz_size<%d>, maybe error", idx, st->stsz_size);
        //return INT_MAX;
    }
    #endif

    if(st->sample_size)
        return st->sample_size;

    offset = st->stsz_offset + idx;
    Cache_Data = MoovGetBe32(buffer+offset);

    return    Cache_Data;

}

CDX_U32 ReadStss(MOVContext *c, MOVStreamContext *st, cdx_uint32 idx)
    //bytes of offset from STSS start
{
    CDX_U32 Cache_Data = 0;
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = 0;

    #if 0
    if(idx >= st->stss_size*4)
    {
        CDX_LOGW("index<%d> is large than stss_size<%d>, maybe error", idx/4, st->stss_size);
        //return INT_MAX;
    }
    #endif

    if(st->rap_seek_count == 0)
    {
        offset = st->stss_offset + idx;
        Cache_Data = MoovGetBe32(buffer+offset);
    }
    else
    {
        if(idx == 0)
        {
            Cache_Data = 1;
        }
        else
        {
            Cache_Data = st->rap_seek[(idx>>2)-1];
        }
    }

    return Cache_Data;
}

static CDX_U32 ReadStts(MOVContext *c, MOVStreamContext *st, cdx_uint32 idx)
{
    CDX_U32 Cache_Data;
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = 0;
    #if 0
    if(idx >= st->stts_size*8)
    {
        CDX_LOGW("index<%d> is large than stts_size<%d>, maybe error", idx, st->stts_size);
        return INT_MAX;
    }
    #endif

    offset = st->stts_offset + idx;
    Cache_Data = MoovGetBe32(buffer+offset);

    return    Cache_Data;
}

static CDX_U64 ReadStco(MOVContext *c, MOVStreamContext *st, cdx_uint32 idx)
{
    CDX_U64 Cache_Data;
    unsigned char* buffer = c->moov_buffer;
    CDX_U64 offset = 0;

    #if 0
    if(idx >= st->stco_size)
    {
        CDX_LOGW("index<%d> is large than stco_size<%d>, maybe error", idx, st->stco_size);
        //return 0;
    }
    #endif

    if(!st->co64)
    {
        idx <<= 2;
        offset = st->stco_offset + idx;
        Cache_Data = MoovGetBe32(buffer+offset);
    }
    else
    {
        idx <<= 3;
        offset = st->stco_offset + idx;
        Cache_Data = MoovGetBe64(buffer+offset);
    }

    return    Cache_Data;
}

#if 0
static void MakeFourCCString(CDX_U32 x, char *s) {
    s[0] = x & 0xff;
    s[1] = (x >> 8) & 0xff;
    s[2] = (x >> 16) & 0xff;
    s[3] = x >> 24;
    s[4] = '\0';
}
#endif

static MOVStreamContext *AvNewStream(MOVContext *c, CDX_S32 id)
{
    MOVStreamContext *st;

    if (c->nb_streams >= MOV_MAX_STREAMS)
        return NULL;

    st = (MOVStreamContext *)malloc(sizeof(MOVStreamContext));
    if (!st)
        return NULL;
    memset(st,0,sizeof(MOVStreamContext));

    st->index = c->nb_streams;
    st->id = id;
    st->rotate[0] = '\0';

    return st;
}

//*****************************************************//
//*****************************************************//

/*
*       tfhd ( Track Fragment Header Box )
*       set up information and defaults used for samples in fragment
*/
static int MovParseTfhd(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    if(atom.size < 12)
    {
        CDX_LOGI("Careful: the tfhd box size is less than 12 bytes!");
    }

    MOVFragment* frag = &s->fragment;
    MOVTrackExt* trex = NULL;
    int flags, track_id, i;
    unsigned char buf[8] = {0};
    int ret;
    int size = atom.size - 8;

    ret = CdxStreamRead(pb, buf, 4);
    if(ret < 4)
    {
        CDX_LOGE("read failed.");
        return -1;
    }
    flags = MoovGetBe24(buf+1);  // version 1byte; tf_flags (3 bytes)
    size -= 4;

    ret = CdxStreamRead(pb, buf, 4);
    size -= 4;
    track_id = MoovGetBe32(buf);
    if(!track_id)
    {
        CDX_LOGE("Track id =0.");
        return -1;
    }
    frag->track_id = track_id;

    //find the same track id in the 'trex' ( in 'moov', which setup the default information )
    for(i=0; i<s->trex_num; i++)
    {
        if(s->trex_data[i].track_id == frag->track_id)
        {
            trex = &s->trex_data[i];
            break;
        }
    }

    if(!trex)
    {
        if(!s->bSmsSegment) //* sms has no trex ??
        {
            CDX_LOGE("Worning: could not find corresponding trex!");
            return -1;
        }
    }

    if(flags & CDX_MOV_TFHD_BASE_DATA_OFFSET)
    {
        ret = CdxStreamRead(pb, buf, 8);
        size -= 8;
        frag->base_data_offset = MoovGetBe64(buf);
    }
    else
    {
        frag->base_data_offset = frag->moof_offset;
    }

    if(flags & CDX_MOV_TFHD_STSD_ID)
    {
        ret = CdxStreamRead(pb, buf, 4);
        size -= 4;
        frag->stsd_id = MoovGetBe32(buf);
    }
    else
    {
        if(trex)
            frag->stsd_id = trex->stsd_id;
        else
            frag->stsd_id = 0;
    }

    if(flags & CDX_MOV_TFHD_DEFAULT_DURATION)
    {
        ret = CdxStreamRead(pb, buf, 4);
        size -= 4;
        frag->duration = MoovGetBe32(buf);
    }
    else
    {
        if(trex)
            frag->duration = trex->duration;
        else
            frag->duration = 0;
    }

    if(flags & CDX_MOV_TFHD_DEFAULT_SIZE)
    {
        ret = CdxStreamRead(pb, buf, 4);
        size -= 4;
        frag->size = MoovGetBe32(buf);
    }
    else
    {
        if(trex)
            frag->size = trex->size;
        else
            frag->size = 0;
    }

    if(flags & CDX_MOV_TFHD_DEFAULT_FLAGS)
    {
        ret = CdxStreamRead(pb, buf, 4);
        size -= 4;
        frag->flags   =  MoovGetBe32(buf);
    }
    else
    {
        if(trex)
            frag->flags = trex->flags;
        else
            frag->flags = 0;
    }

    if(size < 0)
    {
        CDX_LOGI("the size<%d> is error", size);
        return -1;
    }

    return 0;
}

/*
 *  'uuid'  (Sample Encryption Box)
 */
static int MovParseSenc(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    unsigned char buf[20] = {0};
    int size = atom.size - 8;
    int ret = 0;
    int flags;
    unsigned int sample_count;
    int i, j;

    CDX_LOGV("MovParseSenc");

    ret = CdxStreamRead(pb, buf, 16); // a2394f52-5a9b-4f14-a244-6c427c648df4
    CDX_LOGV("type=%2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-"
        "%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
        buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);

    ret = CdxStreamRead(pb, buf, 4);
    //version  ( 1 byte )
    flags = MoovGetBe24(buf+1);

    if (flags & 0x1) {
        ret = CdxStreamRead(pb, buf, 20); // AlgorithmID, iv_size, KID
    }

    ret = CdxStreamRead(pb, buf, 4);
    sample_count = MoovGetBe32(buf); //sample count
    CDX_LOGV("sample_count=%d", sample_count);

    if (s->senc_data)
    {
        free(s->senc_data);
        s->senc_data = NULL;
    }
    s->senc_data = malloc(sample_count*sizeof(MOVSampleEnc));
    if (s->senc_data == NULL) {
        CDX_LOGE("malloc fail.");
        return -1;
    }
    memset(s->senc_data, 0, sample_count*sizeof(MOVSampleEnc));

    for (i = 0; i < (int)sample_count; ++i) {
        ret = CdxStreamRead(pb, buf, 8);
        s->senc_data[i].iv = MoovGetBe64(buf);
        if (flags & 0x2) {
            unsigned int entries;
            ret = CdxStreamRead(pb, buf, 2);
            entries = MoovGetBe16(buf);
            if (entries > 16) {
                CDX_LOGE("region too large.");
                return -1;
            }
            s->senc_data[i].region_count = entries*2;
            for (j = 0; j < (int)entries; ++j) {
                ret = CdxStreamRead(pb, buf, 6);
                if (j < 16) {
                    s->senc_data[i].regions[j*2] = MoovGetBe16(buf); // bytes_of_clear
                    s->senc_data[i].regions[j*2+1] = MoovGetBe32(buf+2); //bytes_of_encrypted
                }
            }
        }
    }
    return 0;
}

/*
*    'trun'  ( Track Fragment Run Box )
*         if the duration_is_empty flag is set in the tf_flag of 'tfhd' box,
*         so need a track run document
*         @ choose the audio stream or video stream by the track_id of trun
*/
static int MovParseTrun(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    int size = atom.size - 8;
    unsigned char buf[8] = {0};
    int ret = 0;
    MOVFragment* frag = &s->fragment;
    MOVStreamContext* st = NULL;
    MOVCtts *ctts_data;
    int flags;
    CDX_U32 data_offset_delta = 0;
    unsigned int first_sample_flags = frag->flags;
    int found_keyframe = 0;

    int i;
    int entries;
    //find the corresponding track stream
    for(i=0; i<s->nb_streams; i++)
    {
        // streams->id is the stream track_id, set in 'tkhd' atom in every 'trak'
        if(s->streams[i]->id == (int)frag->track_id)
        {
            st = s->streams[i];
            break;
        }
    }

    if(s->bSmsSegment)
    {
        st = s->streams[0];
        i = 0;
    }

    if(!st)
    {
        CDX_LOGW(" could not find corresponding track id for trun!");
        return -1;
    }

    // select the approparite stream when bandwidth changed
    if(st->codec.codecType == CODEC_TYPE_VIDEO)
    {
        s->video_stream_idx = i;
    }
    else if(st->codec.codecType == CODEC_TYPE_AUDIO)
    {
        s->audio_stream_idx = i;
    }

    ret = CdxStreamRead(pb, buf, 8);
      //version  ( 1 byte )
    flags = MoovGetBe24(buf+1);  //tr_flags 3 bytes
    entries = MoovGetBe32(buf+4); //sample count

    size -= 8;

    /* Always assume the presence of composition time offsets.
     * Without this assumption, for instance, we cannot deal with a track in fragmented movies
     * that meet the following.
     *  1) in the initial movie, there are no samples.
     *  2) in the first movie fragment, there is only one sample without composition time offset.
     *  3) in the subsequent movie fragments, there are samples with composition time offset. */

    // if the stsz have sample size, add it to ctts
    if(!st->ctts_size && st->sample_size)
    {
        /* Complement ctts table if moov atom doesn't have ctts atom. */
        ctts_data = malloc(sizeof(*st->ctts_data));
        if(!ctts_data) return -1;

        st->ctts_data = ctts_data;
        st->ctts_data[st->ctts_size].count = st->sample_size;
        st->ctts_data[st->ctts_size].duration = 0;
        st->ctts_size ++;
    }

    ctts_data = realloc(st->ctts_data, (entries+st->ctts_size)*sizeof(*st->ctts_data));
    if(!ctts_data) return -1;

    st->ctts_data = ctts_data;

    // data offset is the relative offset  to the 'moof' box,
    // so the first sample offset in the file is caculated by ( base_data_offset + Offset_moof
    // + data_offset )
    if(flags & CDX_MOV_TRUN_DATA_OFFSET)
    {
        if(size < 4)
        {
            return -1;
        }
        ret = CdxStreamRead(pb, buf, 4);
        data_offset_delta = MoovGetBe32(buf);
        size -= 4;
    }

    if(flags & CDX_MOV_TRUN_FIRST_SAMPLE_FLAGS)
    {
        if(size < 4)
        {
            return -1;
        }
        ret = CdxStreamRead(pb, buf, 4);
        first_sample_flags = MoovGetBe32(buf);
        size -= 4;
    }

    CDX_U64 data_offset = frag->base_data_offset + data_offset_delta;
    s->nDataOffsetDelta = data_offset_delta;

    //CDX_LOGD("base data offset = %llx, data_offset_delta = %x, data_offset=%lld",
    //frag->base_data_offset, data_offset_delta, data_offset); //* for debug
    for(i=0; i<entries; i++)
    {
        unsigned int sample_size = frag->size;
        int sample_flags = i ? frag->flags : first_sample_flags;
        unsigned int sample_duration = frag->duration;
        int keyframe = 0;

        if(flags & CDX_MOV_TRUN_SAMPLE_DURATION)
        {
            ret = CdxStreamRead(pb, buf, 4);
            sample_duration = MoovGetBe32(buf);
        }
        if(flags & CDX_MOV_TRUN_SAMPLE_SIZE)
        {
            ret = CdxStreamRead(pb, buf, 4);
            sample_size     = MoovGetBe32(buf);
        }
        if(flags & CDX_MOV_TRUN_SAMPLE_FLAGS)
        {
            ret = CdxStreamRead(pb, buf, 4);
            sample_flags    = MoovGetBe32(buf);
        }

        st->ctts_data[st->ctts_size].count = 1;
        if(flags & CDX_MOV_TRUN_SAMPLE_CTS)
        {
            ret = CdxStreamRead(pb, buf, 4);
            st->ctts_data[st->ctts_size].duration = MoovGetBe32(buf);
        }
        else
        {
            st->ctts_data[st->ctts_size].duration = 0;
        }
        st->ctts_size++;

        if(st->codec.codecType==CODEC_TYPE_AUDIO)
        {
            keyframe = 1;
        }
        else if(!found_keyframe)
        {
            // if sample_flags =0x01010000, the sampe is not the key frame
             keyframe = found_keyframe =
                !(sample_flags & (CDX_MOV_FRAG_SAMPLE_FLAG_IS_NON_SYNC |
                                  CDX_MOV_FRAG_SAMPLE_FLAG_DEPENDS_YES));
        }

        Sample* tmp = (Sample*)malloc(sizeof(Sample));
        if(!tmp) return -1;
        memset(tmp, 0, sizeof(Sample));

        tmp->duration = sample_duration;
        tmp->offset = data_offset;
        tmp->size = sample_size;
        tmp->keyframe = keyframe; // keyframe is not need now
        tmp->index = i;

        //CDX_LOGD("duration = %x, offset = %llx, sample size = %x, flag = %x, data_offset = %llu",
        //sample_duration, data_offset, tmp->size, sample_flags, data_offset);
        if(st->codec.codecType == CODEC_TYPE_VIDEO)
        {
            if( aw_list_add(s->Vsamples, tmp) < 0)
            {
                CDX_LOGE("aw_list_add error!");
                return -1;
            }
        }
        else if(st->codec.codecType == CODEC_TYPE_AUDIO)
        {
            if( aw_list_add(s->Asamples, tmp) < 0)
            {
                CDX_LOGE("aw_list_add error!");
                return -1;
            }
        }

        //s->last_sample_pts = tmp->pts;
        data_offset += sample_size;
    }
    return 0;
}

int MovParseTraf(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    int ret = 0;
    unsigned char buf[4096];
    if(atom.size == 0)
        return 0;

    unsigned int offset = 0;
    MOV_atom_t a = {0, 0, 0};
    while( (offset < atom.size-8)&& (!ret) )
    {
        ret = CdxStreamRead(pb, buf, 8);
        if(ret < 8)
        {
            break;
        }

        a.size = MoovGetBe32(buf);   //big endium
        a.type = MoovGetLe32(buf+4);

        //CDX_LOGD("traf  size = %x, type=%x", a.size, a.type);
        if((a.size == 0) || (a.type == 0))
        {
            CDX_LOGE("atom error!");
            return -1;
        }

        if(a.type == MKTAG( 't', 'f', 'h', 'd' ))
        {
            // sample default infomation: base offset, duration,...
            ret = MovParseTfhd(s, pb, a);
            if(ret < 0)
            {
                CDX_LOGE("parse tfhd failed.");
                return -1;
            }
        }
        else if(a.type == MKTAG( 't', 'r', 'u', 'n' ))
        {
            // the offset, duration, size of samples
            ret = MovParseTrun(s, pb, a);
            if(ret < 0)
            {
                CDX_LOGE("parse trun failed.");
                return -1;
            }
        }
        else if(a.type == MKTAG( 'u', 'u', 'i', 'd' ))
        {
            ret = MovParseSenc(s, pb, a);
            if(ret < 0)
            {
                CDX_LOGE("parse trun failed.");
                return -1;
            }
        }
        //else if(a.type == MKTAG( 's', 'd', 't', 'p' ))
        //{
        //    ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
        //    if(ret < 0) return -1;
        //}
        else
        {
            if(s->bSeekAble)
            {
                ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                if(ret < 0) return -1;
            }
            else
            {
                ret = CdxStreamSkip(pb, a.size-8);
                if(ret < 0) return -1;
            }
        }
        offset += a.size;
        //CDX_LOGD("offset=%u", offset);
    }

    return ret;
}

/*
********************************************************************************************
// every segment MP4 file in DASH
 ----styp
 ----sidx  ( time scale )  'mvhd' atom also have the same time scale
 ----moof
            ------mfhd
            ------traf
                        ------tfhd ( the sample default infomation  )
                        ------tfdt (not used)
                        ------trun ( the sample offset in file  )
                        ------sdtp(not used)
********************************************************************************************
*/

// some media segment was cut to many chunks( like 'stco' in MP4 file ),
//'sidx' was used for get the chunk offset and chunk duration in a same segment.
// but in DASH, there are two kinds of segment
// @ 1. every segment is a chunk, sidx is in the start of every segment, and the sidx did not
//      have any effect to the segment but for seeking in the segment( HDWorld )
//          in this case, we should parser sidx in seek
// @ 2. the segments are in one mp4 file, and each segment is set by the indexRange in mpd, so
//      the sidx is very important in this environment (Big buck bunny)
//in this case, all of the sidx atoms are in the start of file, so we parse all the sidx in movTop
static int MovParseSidx(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    int size = atom.size-8;
    int ret = 0;
    int flags;
    int version;
    unsigned char buf[8] = {0};
    MOVStreamContext* st = NULL;

    ret = CdxStreamRead(pb, buf, 4);
    version = buf[0];
    flags = MoovGetBe24(buf+1);
    size -= 4;

    ret = CdxStreamRead(pb, buf, 8);
    int reference_id = MoovGetBe32(buf);
    int time_scale = MoovGetBe32(buf+4);
    size -= 8;

    int i;
    //CDX_LOGD("nb_stream = %d, time_scale = %d", s->nb_streams, time_scale);
    for(i=0; i<s->nb_streams; i++)
    {
        // streams->id is the stream track_id, set in 'tkhd' atom in every 'trak'
        if(s->streams[i]->id == reference_id)
        {
            st = s->streams[i];
            break;
        }
    }
    if(!st)
    {
        CDX_LOGW("Worning: sidx- could not find corresponding stream id!");
        return -1;
    }
    st->time_scale = time_scale;

    long long int earlist_presentation_time;
    long long int first_offset;

    if(version == 0)
    {
        if(size < 8)
        {
            CDX_LOGE("the size of 'sidx' is error!");
            return -1;
        }
        int tmp;
        ret = CdxStreamRead(pb, buf, 8);
        tmp = MoovGetBe32(buf);
        earlist_presentation_time = tmp;
        tmp = MoovGetBe32(buf+4);
        first_offset = tmp;

        size -= 8;
    }
    else
    {
        if(size < 16)
        {
            CDX_LOGE("the size of 'sidx' is error!");
            return -1;
        }
        ret = CdxStreamRead(pb, buf, 8);
        earlist_presentation_time = MoovGetBe64(buf);
        ret = CdxStreamRead(pb, buf, 8);
        first_offset = MoovGetBe64(buf);

        size -= 16;
    }

    if(size < 4) return -1;

    ret = CdxStreamRead(pb, buf, 4);
        //reserved (2 bytes)
    CDX_S16 reference_count = MoovGetBe16(buf+2);
    CDX_S32 refrence_count_end = s->sidx_count + reference_count;
    size -= 4;

    if(size < reference_count*12)
    {
        return -1;
    }

    CDX_U64 total_duration = 0;
    CDX_U64 offset = 0;

    MOVSidx* sidx;
    sidx = (MOVSidx*)realloc(s->sidx_buffer, refrence_count_end * sizeof(MOVSidx));
    if(!sidx)
    {
        CDX_LOGE("sidx realloc error!");
        return -1;
    }
    else
    {
        s->sidx_buffer = sidx;
    }
    int sub_sidx = 0;

    CDX_U32 d1, d2, d3;
    for(i=s->sidx_count; i<refrence_count_end; i++)
    {
        CdxStreamRead(pb, buf, 4);
        d1 = MoovGetBe32(buf); //refrerence size
        CdxStreamRead(pb, buf, 4);
        d2 = MoovGetBe32(buf); //reference duration
        CdxStreamRead(pb, buf, 4);
        d3 = MoovGetBe32(buf); //reference type

        if(d1 & 0x80000000)
        {
            //it is the index table of other sidx box, so skip it
            sub_sidx = 1;
        }

        int sap = d3 & 0x80000000;
        int saptype = (d3&0x70000000) >> 28;
        if(!sap || saptype > 2)
        {
            CDX_LOGI("not a Stream Access Point, unsupported type!");
        }

        s->sidx_buffer[i].current_dts = s->sidx_time + total_duration;
        s->sidx_buffer[i].offset = offset + atom.size + atom.offset + first_offset;
        //s->sidx_buffer[i].offset = offset ;
        s->sidx_buffer[i].size = d1 & 0x7fffffff;
        s->sidx_buffer[i].duration = ((CDX_U64)d2) *1000/ (CDX_U64)time_scale ;
        //CDX_LOGD("current_dts = %lld, sidx_time = %lld, total_duration = %lld",
        //s->sidx_buffer[i].current_dts, s->sidx_time, total_duration);
        //LOGD("duration = %lld. offst = %lld", s->sidx_buffer[i].duration,
        //s->sidx_buffer[i].offset);
        offset += d1 & 0x7fffffff;
        total_duration += (CDX_U64)d2;
    }

    s->sidx_count = refrence_count_end;
    s->sidx_time += total_duration;
    s->sidx_total_time += total_duration*1000 / (CDX_U64)time_scale;
    if(sub_sidx)
    {
        free(s->sidx_buffer);
        s->sidx_buffer = NULL;
        s->sidx_count = 0;
        s->sidx_time = 0;
        s->sidx_total_time = 0;
    }

    return 0;
}

/*
*    the movie fragment Box of fragment MP4 file
*    get the sample offset, sample duration and sample size from it
*
*/
static int MovParseMoof(MOVContext* s, CdxStreamT* pb, MOV_atom_t atom)
{
    int ret = 0;
    if(atom.size == 0)
        return 0;
    unsigned char buf[8];

    unsigned int offset = 0;
    MOV_atom_t a = {0, 0, 0};
    while((offset < atom.size-8) && (!ret))
    {
        ret = CdxStreamRead(pb, buf, 8);
        if(ret < 8)
        {
            break;
        }

        a.size = MoovGetBe32(buf);   //big endium
        a.type = MoovGetLe32(buf+4);

        //LOGD("type = %x, size=%x", a.type, a.size);
        if((a.size == 0) || (a.type == 0))
        {
            CDX_LOGE(" moof atom error!");
            return -1;
        }

        //CDX_LOGD("type = %x, size=%x", a.type, a.size);
        if(a.type == MKTAG( 't', 'r', 'a', 'f' ))
        {
            ret = MovParseTraf(s, pb, a);
        }
        else
        {
             if(s->bSeekAble)
            {
                ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                if(ret < 0) return -1;
            }
            else
            {
                ret = CdxStreamSkip(pb, a.size-8);
                if(ret < 0) return -1;
            }
        }
        offset += a.size;
    }

    return ret;
}

//--------------------------------------------------------
/************************************************************************/
/*  Build KeyFrame index for ff or rev                                  */
/*每隔tbl_itl个关键帧创建一个关键帧*/
/************************************************************************/

CDX_S16 MovBuildKeyframeIdx(struct CdxMovParser *p, CDX_S32 tbl_itl) //(sample_num: 0..x)
{
    MOVContext      *c;
    //VirCacheContext *vc = p->vc;
    MOVStreamContext *st;

    CDX_U32       stsc_first=0,stsc_next=0,stsc_samples=0;
    CDX_S32       samples_in_chunks=0,sample_in_chunk_ost=0;
    CDX_S32       i;
    CDX_S32       idx;
    CDX_U32       stss_idx, stsc_idx=0,stco_idx;
    CDX_U64       chunk_ost;
    CDX_U64       chunk_sample_size;
    CDX_S32       sample_num, stsc_samples_acc=0;
    CDX_S32         chunk_first_sample_ost;
    CDX_S32       tbl_idx = 0;

    c = (MOVContext*)p->privData;
    st = c->streams[c->video_stream_idx];
    if(st->sample_size != 0)
    {
        return -1;
    }

    st->mov_idx_curr.stts_samples_acc = 0;   //stts sample accumulate
    st->mov_idx_curr.stts_duration_acc = 0;
    st->mov_idx_curr.stts_idx = 0;

    //stss_idx is the keyframe index
    for(stss_idx=0; stss_idx < st->stss_size; stss_idx+=tbl_itl)
    {
        //read the sample index of keyframe from stss
        st->mov_idx_curr.sample_idx = sample_num = ReadStss(c, st, stss_idx<<2) - 1;

        //if the sample_num exist , find the chunk of key frame
        if (sample_num > stsc_samples_acc-1 && stsc_idx <= st->stsc_size)
        {
            //accumulate the samples in the same chunk ,until find the chunk where the
            //keyframe index sample in
            while(1)
            {
                idx = stsc_idx * 12;

                stsc_first   = ReadStsc(c, st, idx);  // read the chunk id in the stsc
                stsc_samples = ReadStsc(c, st, idx+4); //sample number in this chunk

                //if the chunk id is not exist( larger than the stsc size )
                if(st->mov_idx_curr.stsc_idx + 1 >= st->stsc_size)
                {
                    stsc_next = 30*3600*24;
                    samples_in_chunks = 0x7ffffff;//not int max
                }
                else
                {
                    stsc_next    = ReadStsc(c, st, idx+12); //the next chunk id

                    samples_in_chunks = (stsc_next - stsc_first)*stsc_samples;
                    //the sample number between the two chunks
                }

                //accumulate the samples in the same chunk
                stsc_samples_acc += samples_in_chunks;

                stsc_idx++;
                //if find the chunk, break
                if(sample_num <= stsc_samples_acc-1 || st->mov_idx_curr.stsc_idx > st->stsc_size)
                    break;
                stsc_first = stsc_next;
            }
        }

        //the offset of the keyframe sample in the chunk
        //sample在该chunk中的偏移
        sample_in_chunk_ost = sample_num - (stsc_samples_acc - samples_in_chunks);
        if (stsc_samples == 0)
        {
            return -1;;
        }
        else if (stsc_samples == 1)//if the chunk has only one sample
        {
            stco_idx = stsc_first + sample_in_chunk_ost - 1;
        }
        else
        {
            stco_idx = stsc_first + (sample_in_chunk_ost / stsc_samples) - 1;
        }
        chunk_ost = ReadStco(c, st, stco_idx); //read chunk offset from stco

        sample_in_chunk_ost = sample_in_chunk_ost - (stco_idx + 1 - stsc_first)*stsc_samples;
        chunk_sample_size = 0;
        chunk_first_sample_ost = (sample_num - sample_in_chunk_ost)<<2;

        //stsz must always exist in video trak, else error, must acc it
        if(sample_in_chunk_ost)
        {
            for(i=0; i<sample_in_chunk_ost; i++)
            {
                chunk_sample_size += ReadStsz(c, st, chunk_first_sample_ost + (i<<2) );
            }
        }
        c->idx_buf[tbl_idx].offset  = chunk_ost + chunk_sample_size;

        c->idx_buf[tbl_idx].size     = ReadStsz(c, st, sample_num<<2);

        //if stts size > 1 ( the duration of samples is not equal )
        if(st->stts_size > 1)
        {
            if(st->mov_idx_curr.sample_idx >= st->mov_idx_curr.stts_samples_acc)
            {
                CDX_S32 stts_sample_count;
                CDX_S32 stts_sample_duration;
                CDX_S32 idx;

                while(1)
                {
                    idx = st->mov_idx_curr.stts_idx<<3;

                    stts_sample_count    = ReadStts(c, st, idx);
                    stts_sample_duration = ReadStts(c, st, idx+4);

                    st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                    st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                        stts_sample_duration;

                    st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                    st->mov_idx_curr.stts_idx++;
                    if(st->mov_idx_curr.sample_idx < st->mov_idx_curr.stts_samples_acc)
                    {
                        break;
                    }
                }
            }

            c->idx_buf[tbl_idx].current_dts =
                st->mov_idx_curr.stts_duration_acc -
                (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                (st->mov_idx_curr.stts_samples_acc - st->mov_idx_curr.sample_idx);
        }

        tbl_idx++;
    }

    st->mov_idx_curr.stts_samples_acc = 0;
    st->mov_idx_curr.stts_duration_acc = 0;
    st->mov_idx_curr.stts_idx = 0;
    st->mov_idx_curr.sample_idx = 0;

    return 0;
}

CDX_S32 MovTimeToSampleSidx(struct CdxMovParser *p, int seconds, int stream_index, int type)
{
    MOVContext      *s;
    s = (MOVContext*)p->privData;
    CdxStreamT    *pb = s->fp;
    int sidx_idx = 0;
    int ret = 0;
    CDX_UNUSE(type);

    MOVSidx* sidx = s->sidx_buffer;
    int i = 0;

    CDX_U32 time_scale = s->streams[stream_index]->time_scale;
    CDX_U64 time = (CDX_U64)seconds * time_scale / 1000;

    if(s->bSmsSegment) //* for sms
    {
        //CDX_LOGD("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
        s->streams[stream_index]->mov_idx_curr.current_dts = time;
        return 0;
    }

    if(!sidx)
    {
        CDX_LOGW("--- sidx is NULL, cannot seek");
        return 0;
    }
    for(i=0; i<s->sidx_count; i++)
    {
        if(sidx[i].current_dts <= time)
        {
            sidx_idx = i;
        }
        else
        {
            break;
        }
    }
    s->streams[stream_index]->mov_idx_curr.current_dts = sidx[sidx_idx].current_dts;
    //reset the audio pts

    CDX_U64 offset = sidx[sidx_idx].offset;
    //CdxStreamSeek(pb, s->first_moof_offset+offset, SEEK_SET);
    ret = CdxStreamSeek(pb, offset, SEEK_SET);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

//only support video track time to sample
CDX_S32 MovTimeToSample(struct CdxMovParser *p, int seconds)
{
    MOVContext      *c;
    //VirCacheContext *vc = p->vc;
    CDX_U64   duration_seek;
    CDX_U64   duration_acc=0;
    CDX_S32   sample_count,sample_count_acc=0,sample_duration;
    CDX_S32   sample_idx;
    CDX_S32   time_scale;
    CDX_S32   stream_idx;
    CDX_S32   idx;
    CDX_U32   stts_idx;

    c = (MOVContext*)p->privData;

    stream_idx = c->video_stream_idx;
    time_scale = c->streams[stream_idx]->time_scale;
    duration_seek = (CDX_U64)time_scale * seconds/1000;

    //look up from video stts table
    stts_idx = 0;

    while(1)
    { //modify 20090327 night
        if(p->exitFlag)
        {
            return -1;
        }

        idx = stts_idx << 3;

        sample_count    = ReadStts(c, c->streams[stream_idx], idx);
        sample_duration = ReadStts(c, c->streams[stream_idx], idx+4);

        duration_acc += (CDX_U64)sample_count * sample_duration;
        sample_count_acc += sample_count;
        if(duration_seek < duration_acc || stts_idx > c->streams[stream_idx]->stts_size)
            break;
        stts_idx++;
    }

    if(sample_duration != 0)
    {
        sample_idx = sample_count_acc - (duration_acc - duration_seek) / sample_duration;
    }
    else
    {
        CDX_LOGI("Be careful: sample_duration is zero, set sample_idx to zero!");
        sample_idx = 0;
    }

    return sample_idx;
}

//when seek a sample, we must update:
//video: stsc_idx  stco_idx stsz_idx chunk_sample_size   chunk_sample_idx    sample_idx
//audio: stsc_idx  stco_idx stsz_idx chunk_sample_size=0 chunk_sample_idx=0  chunk_idx=stco_idx-1
//(sample_num: 0..x)
CDX_S16 MovSeekSample(struct CdxMovParser *p, CDX_U32 sample_num, CDX_S32 seconds)
{
    MOVContext          *c;
    //VirCacheContext     *vc = p->vc;
    CDX_U32       stsc_first_v,stsc_next_v,stsc_samples,stsc_samples_acc=0;
    CDX_S32       samples_in_chunks,sample_in_chunk_ost;
    CDX_S32       i;
    CDX_U32       idx;

    c = (MOVContext*)p->privData;
    MOVStreamContext* st;

    // we should seek the video firstly, then seek audio and subtitle,
    // because they are depending on video current dts after seek
    if(c->has_video)
    {
    st = c->streams[c->video_stream_idx];
    if(st->stsd_type == 1)
    {
        st->mov_idx_curr.stsc_idx = 0;

        stsc_first_v = ReadStsc(c, st, 0);

        while(1)
        {
            if(p->exitFlag)
            {
                return -1;
            }

            idx = st->mov_idx_curr.stsc_idx * 12;

            stsc_samples = ReadStsc(c, st, idx+4);
            stsc_next_v    = ReadStsc(c, st, idx+12);

            if(st->mov_idx_curr.stsc_idx + 1 >= st->stsc_size)
            {
                stsc_next_v = 30*3600*24;
                samples_in_chunks = 0x7ffffff;
            }
            else
            {
                samples_in_chunks = (stsc_next_v - stsc_first_v)*stsc_samples;
            }

            stsc_samples_acc += samples_in_chunks;
            st->mov_idx_curr.stsc_idx++;//has read a stsc, switch to normal play, no need to inc it
            if(sample_num <= stsc_samples_acc-1 || st->mov_idx_curr.stsc_idx > st->stsc_size)
                break;
            stsc_first_v = stsc_next_v;
        }

        st->mov_idx_curr.stsc_samples = stsc_samples;
        st->mov_idx_curr.stsc_end = stsc_next_v-1;

        //sample_in_chunk_ost : 0..x
        //below 2 lines :: -1  first: for round , second: for stsc_first start with 1
        sample_in_chunk_ost = sample_num - (stsc_samples_acc - samples_in_chunks);
        if(stsc_samples==1)
            st->mov_idx_curr.stco_idx = stsc_first_v + sample_in_chunk_ost - 1;
        else
            st->mov_idx_curr.stco_idx = stsc_first_v + (sample_in_chunk_ost / stsc_samples) - 1;
        st->mov_idx_curr.chunk_idx = st->mov_idx_curr.stco_idx;
        sample_in_chunk_ost = sample_in_chunk_ost - (st->mov_idx_curr.stco_idx + 1 -
            stsc_first_v)*stsc_samples;
        st->mov_idx_curr.chunk_sample_idx = sample_in_chunk_ost;

        if(st->sample_size != 0)
            return -1;

        {//stsz must always exist in video trak, else error, must acc it
            CDX_S32 chunk_first_sample_ost;
            st->mov_idx_curr.sample_idx = st->mov_idx_curr.stsz_idx = sample_num;
            st->mov_idx_curr.chunk_sample_size = 0;

            chunk_first_sample_ost = (sample_num - sample_in_chunk_ost)<<2;
            for(i=0;i<sample_in_chunk_ost;i++)
            {
                st->mov_idx_curr.chunk_sample_size += ReadStsz(c, st,
                                                      chunk_first_sample_ost + (i<<2));
            }
        }

        //calc video dts
        {
            st->mov_idx_curr.stts_idx = 0;
            st->mov_idx_curr.stts_samples_acc = 0;
            st->mov_idx_curr.stts_duration_acc = 0;

            if(st->mov_idx_curr.sample_idx >= st->mov_idx_curr.stts_samples_acc)
            {
                //CDX_S32 frame_rate;
                CDX_S32 stts_sample_count;
                CDX_S32 stts_sample_duration;

                while(1)
                {
                    if(p->exitFlag)
                    {
                        return -1;
                    }

                    idx = st->mov_idx_curr.stts_idx<<3;

                    stts_sample_count    = ReadStts(c, st, idx);
                    stts_sample_duration = ReadStts(c, st, idx+4);

                    st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                    st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                        stts_sample_duration;
                    st->mov_idx_curr.stts_idx++;
                    st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                    if(st->mov_idx_curr.sample_idx < st->mov_idx_curr.stts_samples_acc)
                    {
                        break;
                    }
                }

                //to fix mpeg4 B-VOP jump play bug   ..transform mov can use it
                {
                    //CDX_S64     tmpLargeVal;

                    //calculate video frame rate
                    //if(st->mov_idx_curr.stts_sample_duration)
                    //{
                    //    tmpLargeVal = (CDX_S64)st->time_scale;
                    //    tmpLargeVal = (tmpLargeVal * 1000) /
                    //(CDX_S64)st->mov_idx_curr.stts_sample_duration;
                    //    c->ptr_fpsr->vFormat.nFrameRate = (CDX_U32)tmpLargeVal;
                    //}
                }
            }
            st->mov_idx_curr.current_dts =
                st->mov_idx_curr.stts_duration_acc -
                (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                (st->mov_idx_curr.stts_samples_acc - st->mov_idx_curr.sample_idx);

        }
    }
    }

    for(i=0; i<c->nb_streams; i++)
    {
        st = c->streams[i];
        //calc stts end here
        //----------------------------------------------------
        //      we should update audio offsets also
        //----------------------------------------------------
        if(st->stsd_type == 2)
        {
            //CDX_S64 video_chunk_ost,audio_chunk_ost;
            CDX_U32 stsz_acc = 0;
            CDX_S32 stsc_first, stts_sample_count,stts_sample_duration = 0;
            CDX_U32 stsc_next;
            CDX_U64 curr_chunk_duration = 0, seek_dts;
            CDX_U32 seek_audio_sample=0;
            CDX_U32 stsc_first_flag = 0;

            //search audio stco > video stco
            st->mov_idx_curr.stco_idx = 0;

            if(c->has_video)
                seek_dts = c->streams[c->video_stream_idx]->mov_idx_curr.current_dts *
                st->time_scale /c->streams[c->video_stream_idx]->time_scale;
            else
                seek_dts = (CDX_U64)st->time_scale * seconds/1000;

            st->mov_idx_curr.stsc_idx = 0;
            st->mov_idx_curr.stts_idx = 0;
            st->mov_idx_curr.stts_samples_acc = 0;
            st->mov_idx_curr.stts_duration_acc = 0;

            idx = 0;
            stsz_acc = 0;
            stsc_first_flag = 0;

            stsc_first   = ReadStsc(c, st, 0);
            stsc_samples = ReadStsc(c, st, idx+4);
            stsc_next    = ReadStsc(c, st, idx+12);

            stsz_acc += stsc_samples;

            st->mov_idx_curr.stsc_idx++;
            if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
            {
                stsc_next = INT_MAX;
            }
            while(1)
            {
                if(p->exitFlag)
                {
                    return -1;
                }

                if (st->mov_idx_curr.stts_idx == 0)
                {
                    stts_sample_count    = ReadStts(c, st, idx);
                    stts_sample_duration = ReadStts(c, st, idx+4);

                    st->mov_idx_curr.stts_samples_acc = stts_sample_count;
                    st->mov_idx_curr.stts_duration_acc = (CDX_U64)stts_sample_count *
                        stts_sample_duration;
                    st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                    st->mov_idx_curr.stts_idx++;
                }
                else if ((stsz_acc > st->mov_idx_curr.stts_samples_acc)
                        && (st->mov_idx_curr.stts_idx <= (st->stts_size-1)))
                {
                    idx = st->mov_idx_curr.stts_idx<<3;

                    stts_sample_count    = ReadStts(c, st, idx);
                    stts_sample_duration = ReadStts(c, st, idx+4);

                    // for recoding 3gpp file
                    if(!c->has_video)
                    {
                        int i;
                        CDX_U32 duration = st->mov_idx_curr.stts_duration_acc;
                        seek_audio_sample = st->mov_idx_curr.stts_samples_acc;
                        for(i=0; i<stts_sample_count; i++)
                        {
                            duration += stts_sample_duration;
                            if(duration > seek_dts)
                                break;

                            seek_audio_sample ++;
                        }
                    }

                    st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                    st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                        stts_sample_duration;
                    st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                    st->mov_idx_curr.stts_idx++;
                }

                if(seek_dts > 0)
                {
                    if(st->mov_idx_curr.stts_duration_acc > seek_dts)
                    {
                        break;
                    }
                }

                if (stsz_acc <= st->mov_idx_curr.stts_samples_acc)
                {
                    break;
                }
            }

            if(st->stts_size == 1)
            {
                seek_audio_sample = (CDX_U32)(seek_dts/stts_sample_duration);
            }

            while(1)
            {
                if(p->exitFlag)
                {
                    return -1;
                }

                if(st->mov_idx_curr.stco_idx+1 >= stsc_next)
                {
                    idx = st->mov_idx_curr.stsc_idx * 12;
                    //stsc_first = ReadAudioSTSC(vc, 0);

                    stsc_samples = ReadStsc(c, st, idx+4);
                    stsc_next    = ReadStsc(c, st, idx+12);

                    //stsz_acc += stsc_samples;
                    st->mov_idx_curr.stsc_idx++;
                    if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
                    {
                        stsc_next = INT_MAX;
                    }
                }

                if (0 == stsc_first_flag)
                {
                    stsc_first_flag = 1;
                }
                else
                {
                    stsz_acc += stsc_samples;
                }

                if(st->stts_size == 1)//most of streams goes here
                {
                    if(stsz_acc > seek_audio_sample)
                    {
                        break;
                    }
                }
                else
                {
                    while(1)
                    {
                        if(p->exitFlag)
                        {
                            return -1;
                        }

                        if((stsz_acc > st->mov_idx_curr.stts_samples_acc)
                            && (st->mov_idx_curr.stts_idx <= (st->stts_size-1)))
                        {
                            idx = st->mov_idx_curr.stts_idx<<3;

                            stts_sample_count    = ReadStts(c, st, idx);
                            stts_sample_duration = ReadStts(c, st, idx+4);

                            st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                            st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                                stts_sample_duration;
                            st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                            st->mov_idx_curr.stts_idx++;
                        }
                        else
                        {
                            break;
                        }

                        if(st->mov_idx_curr.stts_duration_acc > seek_dts)
                        {
                            break;
                        }
                    }
                    curr_chunk_duration =
                        st->mov_idx_curr.stts_duration_acc -
                                        (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                                        (st->mov_idx_curr.stts_samples_acc - stsz_acc);
                    if (stsz_acc > st->mov_idx_curr.stts_samples_acc)
                    {
                        curr_chunk_duration = st->mov_idx_curr.stts_duration_acc;
                        break;
                    }
                    else if (curr_chunk_duration > seek_dts)
                    {
                        break;
                    }
                }
                if(st->mov_idx_curr.stco_idx >= st->stco_size)
                {
                    st->read_va_end = 1;
                    break;
                }

                st->mov_idx_curr.stco_idx++;
            }

            if(st->stts_size == 1)//most of streams goes here
            {
                if(st->stco_size==1)
                    curr_chunk_duration = (CDX_U64)seek_audio_sample*stts_sample_duration;
                else
                    curr_chunk_duration = (CDX_U64)stsz_acc*stts_sample_duration;
            }

            st->mov_idx_curr.current_dts = curr_chunk_duration;
            //c->mov_idx_curr[1].stsc_first = stsc_first;
            st->mov_idx_curr.stsc_samples = stsc_samples;
            st->mov_idx_curr.stsc_end = stsc_next;
            st->mov_idx_curr.chunk_idx = st->mov_idx_curr.stco_idx;//+1 to make it read stsc_end
            //audio stco_idx search end

            //audio stsc search begin  stsz calc
            st->mov_idx_curr.stsc_idx = 0;
            stsz_acc = 0;

            st->mov_idx_curr.stsc_first = stsc_first = ReadStsc(c, st, 0);

            while(1)
            {
                if(p->exitFlag)
                {
                    return -1;
                }

                idx = st->mov_idx_curr.stsc_idx * 12;

                stsc_samples = ReadStsc(c, st, idx+4);
                stsc_next    = ReadStsc(c, st, idx+12);

                //stsc_next count from 1, stco_idx count from 0
                if(stsc_next > st->mov_idx_curr.stco_idx+1 ||
                   st->mov_idx_curr.stsc_idx > st->stsc_size)
                {
                    break;
                }
                st->mov_idx_curr.stsc_idx++;
                stsz_acc += stsc_samples*(stsc_next - stsc_first);
                stsc_first = stsc_next;
                if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
                {
                    break;
                }
            }
            st->mov_idx_curr.stsc_samples = stsc_samples;//add 20090512 to fix audio 1 table bug
            st->mov_idx_curr.stsc_end = 0; //force to read a audio stco idx
            //c->mov_idx_curr[1].stsc_first = stsc_first-1;
            //we must add the skiped stsc
            stsz_acc += stsc_samples*(st->mov_idx_curr.stco_idx - (stsc_first-1));

            if(st->sample_size == 0)
            {
                st->mov_idx_curr.stsz_idx = stsz_acc;
            }
            else
            {
                st->mov_idx_curr.stsz_idx = 0;
            }

            if(st->stco_size == 1)
            {   //some 3GP stream goes here
                st->mov_idx_curr.sample_idx = seek_audio_sample;
                st->mov_idx_curr.stsz_idx = 0;
                st->mov_idx_curr.chunk_sample_size = 0;
                for(idx=0; idx<seek_audio_sample; idx++)
                {
                    st->mov_idx_curr.chunk_sample_size += ReadStsz(c, st,
                        st->mov_idx_curr.stsz_idx<<2);

                    st->mov_idx_curr.stsz_idx++;
                }
                st->mov_idx_curr.chunk_sample_idx = seek_audio_sample;
                c->chunk_sample_idx_assgin_by_seek = 1;
            }
            else
            {
                // for 3D-1080P-B052.mov, the sample_idx is not right.
                // but it is not a good idea
                if((st->stts_size > 1) && (stsc_samples > 10))
                {
                    st->mov_idx_curr.sample_idx = stsz_acc + stsc_samples;
                    //st->mov_idx_curr.sample_idx = st->mov_idx_curr.stts_samples_acc-1;
                    CDX_LOGD("--- sample_idx=%d", st->mov_idx_curr.sample_idx);
                }
                else
                {
                    st->mov_idx_curr.sample_idx = stsz_acc;
                }
                st->mov_idx_curr.chunk_sample_size = 0;
                st->mov_idx_curr.chunk_sample_idx = 0;
            }
            /*no need to calc it as for last: fpos_last = fpos_curr +
            4*c->mov_idx_curr[1].stsc_idx*12 + 12;*/
        }

        //calc stts end here
        //----------------------------------------------------
        //      we should update subtitle offsets also
        //----------------------------------------------------
        else if(st->stsd_type == 3)
        {
            if(p->exitFlag)
            {
                return -1;
            }

            //CDX_S32 video_chunk_ost,audio_chunk_ost;
            CDX_U32 stsz_acc;
            CDX_S32 stsc_first, stts_sample_count,stts_sample_duration;
            CDX_U32 stsc_next;
            CDX_U64 curr_chunk_duration = 0,seek_dts;
            CDX_U32 seek_subtitle_sample=0;

            //search audio stco > video stco
            st->mov_idx_curr.stco_idx = 0;

            st->mov_idx_curr.stsc_idx = 0;
            st->mov_idx_curr.stts_idx = 0;
            idx = 0;
            stsz_acc = 0;

            stsc_first = ReadStsc(c, st, 0);
            stsc_samples = ReadStsc(c, st, idx+4);
            stsc_next    = ReadStsc(c, st, idx+12);

            st->mov_idx_curr.stsc_idx++;
            if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
            {
                stsc_next = INT_MAX;
            }

            stts_sample_count    = ReadStts(c, st, idx);
            stts_sample_duration = ReadStts(c, st, idx+4);

            st->mov_idx_curr.stts_samples_acc = stts_sample_count;
            st->mov_idx_curr.stts_duration_acc = (CDX_U64)stts_sample_count * stts_sample_duration;
            st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
            st->mov_idx_curr.stts_idx = 1;
            seek_dts = c->streams[c->video_stream_idx]->mov_idx_curr.current_dts * st->time_scale
                         / c->streams[c->video_stream_idx]->time_scale;

            if(st->stts_size == 1)
            {
                seek_subtitle_sample = (CDX_U32)(seek_dts/stts_sample_duration);
            }

            while(1)
            {
                if(p->exitFlag)
                {
                    return -1;
                }

                if(st->mov_idx_curr.stco_idx+1 > stsc_next)
                {
                    idx = st->mov_idx_curr.stsc_idx * 12;
                    //stsc_first = ReadSubtitleSTSC(vc, 0);

                    stsc_samples = ReadStsc(c, st, idx+4);
                    stsc_next    = ReadStsc(c, st, idx+12);

                    st->mov_idx_curr.stsc_idx++;
                    if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
                    {
                        stsc_next = INT_MAX;
                    }
                }

                stsz_acc += stsc_samples;

                if(st->stts_size == 1)//most of streams goes here
                {
                    if(stsz_acc > seek_subtitle_sample)
                        break;
                }
                else
                {
                    curr_chunk_duration += (CDX_U64)stsc_samples*stts_sample_duration;
                    if(curr_chunk_duration > seek_dts)
                        break;

                    if(stsz_acc > st->mov_idx_curr.stts_samples_acc
                        && st->mov_idx_curr.stts_idx < st->stts_size-1)
                    {
                        idx = st->mov_idx_curr.stts_idx<<3;

                        stts_sample_count    = ReadStts(c, st, idx);
                        stts_sample_duration = ReadStts(c, st, idx+4);

                        st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                        st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                            stts_sample_duration;
                        st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                        st->mov_idx_curr.stts_idx++;
                    }
                }

                if(st->mov_idx_curr.stco_idx >= st->stco_size)
                {
                    st->read_va_end = 1;
                    break;
                }

                st->mov_idx_curr.stco_idx++;
            }

            if(st->stts_size == 1)//most of streams goes here
            {
                if(st->stco_size==1)
                    curr_chunk_duration = (CDX_U64)seek_subtitle_sample*stts_sample_duration;
                else
                    curr_chunk_duration = (CDX_U64)stsz_acc*stts_sample_duration;
            }

            st->mov_idx_curr.current_dts = curr_chunk_duration;
            st->mov_idx_curr.stsc_samples = stsc_samples;
            st->mov_idx_curr.stsc_end = stsc_next;
            st->mov_idx_curr.chunk_idx = st->mov_idx_curr.stco_idx;//+1 to make it read stsc_end
            //audio stco_idx search end

            //audio stsc search begin  stsz calc
            st->mov_idx_curr.stsc_idx = 0;
            stsz_acc = 0;

            st->mov_idx_curr.stsc_first = stsc_first = ReadStsc(c, st, 0);

            while(1)
            {
                if(p->exitFlag)
                {
                    return -1;
                }
                idx = st->mov_idx_curr.stsc_idx * 12;

                stsc_samples = ReadStsc(c, st, idx+4);
                stsc_next    = ReadStsc(c, st, idx+12);

                //stsc_next count from 1, stco_idx count from 0
                if(stsc_next > st->mov_idx_curr.stco_idx+1 ||
                   st->mov_idx_curr.stsc_idx > st->stsc_size)
                    break;

                st->mov_idx_curr.stsc_idx++;
                stsz_acc += stsc_samples*(stsc_next - stsc_first);
                stsc_first = stsc_next;
                if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
                    break;
            }
            st->mov_idx_curr.stsc_samples = stsc_samples;//add 20090512 to fix audio 1 table bug
            st->mov_idx_curr.stsc_end = 0; //force to read a audio stco idx

            //we must add the skiped stsc
            stsz_acc += stsc_samples*(st->mov_idx_curr.stco_idx - (stsc_first-1));
            if(st->sample_size == 0)
            {
                st->mov_idx_curr.stsz_idx = stsz_acc;
            }
            else
            {
                st->mov_idx_curr.stsz_idx = 0;
            }

            if(st->stco_size==1)
            {   //some 3GP stream goes here
                st->mov_idx_curr.sample_idx = seek_subtitle_sample;
                st->mov_idx_curr.stsz_idx = 0;
                st->mov_idx_curr.chunk_sample_size = 0;
                for(idx=0;idx<seek_subtitle_sample;idx++)
                {
                    st->mov_idx_curr.chunk_sample_size += ReadStsz(c, st,
                        st->mov_idx_curr.stsz_idx<<2);

                    st->mov_idx_curr.stsz_idx++;
                }
                st->mov_idx_curr.chunk_sample_idx = seek_subtitle_sample;
                c->chunk_sample_idx_assgin_by_seek = 1;
            }
            else
            {
                st->mov_idx_curr.sample_idx = stsz_acc;
                st->mov_idx_curr.chunk_sample_size = 0;
                st->mov_idx_curr.chunk_sample_idx = 0;
            }
            /*no need to calc it as for last: fpos_last = fpos_curr +
            4*c->mov_idx_curr[1].stsc_idx*12 + 12;*/
        }

    }
    return 0;
}

/*
*  fragment MP4
*  find 'moof' and get the sample from it ,then store it to a list
*   @ offset: the offset in the file, used to seek from current
*   @ moov: if DASH change bandwidth, the new init segment will contain a 'moov' atom
*/
CDX_S32 MovReadSampleList(struct CdxMovParser *p)
{
    int ret = 0;
    MOVContext*         s = (MOVContext*)p->privData;
    CdxStreamT    *pb = s->fp;
    CDX_U64 offset = 0;
    unsigned char buf[8] = {0};
    MOV_atom_t a;

    if(s->moof_end_offset)
    {
        offset = s->moof_end_offset;
        CdxStreamSeek(pb, s->moof_end_offset, SEEK_SET);
    }
    else
    {
        //the first moof will goes here
        offset = CdxStreamTell(pb);
        //CDX_LOGD("xxxxx offset=%llu", offset);
    }

    // find the moof , if no, the file is end
    while(ret > -1)
    {
        a.offset = CdxStreamTell(pb);
        ret = CdxStreamRead(pb, buf, 8);
        if(ret < 8)
        {
            CDX_LOGD("end of file? reslut(%d)", ret);
            break;
        }

        a.size = MoovGetBe32(buf);   //big endium
        a.type = MoovGetLe32(buf+4);

        // read end
        if((a.size == 0) || (a.type == 0))
        {
            break;
        }

        if(a.type == MKTAG( 'm', 'o', 'o', 'f' ))
        {
            s->fragment.moof_offset = a.offset;
            ret = MovParseMoof(s, pb, a);
        }
        else if (a.type == MKTAG( 'm', 'd', 'a', 't' ))
        {
            //offset and duration of the chunk in segment
            s->moof_end_offset = a.offset+a.size;
            s->nSmsMdatOffset = CdxStreamTell(pb); //* for sms
            //CDX_LOGD("xxx s->nSmsMdatOffset=%llu", s->nSmsMdatOffset);
            return 0;
        }
        else if (a.type == MKTAG( 's', 'i', 'd', 'x' ))
        {
            //offset and duration of the chunk in segment
            //if get a sidx the first_moof_offset should set to 0
            s->sidx_flag = 1;
            ret = MovParseSidx(s, pb, a);
        }
        else if (a.type == MKTAG( 's', 's', 'i', 'x' ))
        {
            //offset and duration of the chunk in segment
            //MovParseSidx(s, pb, a);
            ret = CdxStreamSeek(pb, a.size - 8,SEEK_CUR);
        }
        else if (a.type == MKTAG( 'm', 'o', 'o', 'v' ))
        {
            //offset and duration of the chunk in segment
            //LOGD("-- sample list -- moov");
            //MovParseSidx(s, pb, a);
            ret = CdxStreamSeek(pb, a.size - 8,SEEK_CUR);
        }
        else
        {
            ret = CdxStreamSeek(pb, a.size - 8,SEEK_CUR);
        }
    }

    // end of stream
    return 1;
}

//************************************************************************/
//*  read a sample sequence in streams      (fragment MP4) */
//*  Return value  :
//*      1   read finish
//*      0  allright
//*    -1  read error
//************************************************************************/
CDX_S16 MovReadSampleFragment(struct CdxMovParser *p)
{
    int ret;
    MOVContext*         s = (MOVContext*)p->privData;
    AW_List* Vsamples = s->Vsamples;
    AW_List* Asamples = s->Asamples;
    MOV_CHUNK* chunk_info = &s->chunk_info;
    int a_end = 0, v_end = 0;

    if(!s->has_audio)
    {
        a_end = 1;
    }
    if(!s->has_video)
    {
        v_end = 1;
    }

    // get the video and audio sample list
    if(s->has_audio && !a_end)
    {
        while(!aw_list_count(Asamples) )
        {
            ret = MovReadSampleList(p);
            if(ret == 1)
            {
                a_end = 1;
                break;
            }
            else if(ret < 0)
            {
                return -1;
            }
        }
    }
    if(s->has_video && !v_end)
    {
        while(!aw_list_count(Vsamples) )
        {
            ret = MovReadSampleList(p);
            if(ret == 1)
            {
                v_end = 1;
                break;
            }
            else if (ret < 0)
            {
                return -1;
            }
        }
    }

    CDX_U64 apts = 0xFFFFFFFF, vpts = 0xFFFFFFFF;
    Sample* Vsample = NULL;
    Sample* Asample = NULL;
    CDX_U64 voffset = 0xFFFFFFFF;
    CDX_U64 aoffset = 0xFFFFFFFF;
    CDX_U64 vNum = aw_list_count(Vsamples);
    //CDX_LOGD("lin vNum = %llu, s->video_stream_idx = %d", vNum, s->video_stream_idx);
    if(s->has_video && vNum)
    {
        // get the video sample
        Vsample = aw_list_get(Vsamples, 0);
        if(!Vsample)
        {
            voffset = 0xFFFFFFFF;
        }
        else
        {
            if(s->bSmsSegment && s->nSmsMdatOffset && !s->nDataOffsetDelta) //* for sms
            {
                //CDX_LOGD("000 s->nSmsMdatOffset = %lld, s->fp=%p, Vsample->offset=%llu",
                //s->nSmsMdatOffset, s->fp, Vsample->offset);
                Vsample->offset += s->nSmsMdatOffset;
                //CDX_LOGD("111 s->nSmsMdatOffset = %lld, s->fp=%p, Vsample->offset=%llu",
                //s->nSmsMdatOffset, s->fp, Vsample->offset);
            }
            voffset = Vsample->offset;
        }
        vpts = (CDX_S64)s->streams[s->video_stream_idx]->mov_idx_curr.current_dts * 1000 * 1000
                                / s->streams[s->video_stream_idx]->time_scale +
                                s->basetime[0]*1000*1000;
        CDX_LOGV("xxx vpts=%lld, current dts=%llu, s->video_stream_idx=%d,"
            " s->streams[s->video_stream_idx]->time_scale=%d, s->basetime[0]=%d, voffset=%llu",
           vpts, (CDX_S64)s->streams[s->video_stream_idx]->mov_idx_curr.current_dts,
           s->video_stream_idx, s->streams[s->video_stream_idx]->time_scale,
           s->basetime[0], voffset);
    }

    if(s->has_audio && aw_list_count(Asamples))
    {
        //get the audio sample and the sample pts
        Asample = aw_list_get(Asamples, 0);
        if(!Asample)
        {
            aoffset = 0xFFFFFFFF;
        }
        else
        {
            aoffset = Asample->offset;
        }
        apts = (CDX_S64)s->streams[s->audio_stream_idx]->mov_idx_curr.current_dts * 1000*1000
                            / s->streams[s->audio_stream_idx]->time_scale +
                            s->basetime[1]*1000*1000;
    }
//    CDX_LOGD("has_video = %d, has_audio = %d", s->has_video, s->has_audio);

    // how to select the sample( audio or video ) to show, there were two methods to deal with it
    // @ 1.compare with their pts, the sample with the smaller pts would be show in first
    // @ 2.compare with their sample offset. this method should be used in network
    if( (vpts < apts) && s->has_video && !v_end)
    {
        s->streams[s->video_stream_idx]->mov_idx_curr.current_dts += (CDX_U64)Vsample->duration;
        s->streams[s->video_stream_idx]->mov_idx_curr.current_duration = (CDX_U64)Vsample->duration;

        chunk_info->offset = Vsample->offset;
        chunk_info->length = Vsample->size;
        chunk_info->index = Vsample->index;
        chunk_info->type = 0; //video

        free(Vsample);
        ret = aw_list_rem(Vsamples, 0);
        if(ret < 0)
        {
            return -1;
        }
    }
    else if((vpts >= apts) && s->has_audio && !a_end)
    {
        s->streams[s->audio_stream_idx]->mov_idx_curr.current_dts += (CDX_U64)Asample->duration;
        s->streams[s->audio_stream_idx]->mov_idx_curr.current_duration =
            (CDX_U64)Asample->duration;

        chunk_info->offset = Asample->offset;
        chunk_info->length = Asample->size;
        chunk_info->index = Asample->index;
        chunk_info->type = 1; //audio

        free(Asample);
        ret = aw_list_rem(Asamples, 0);
        if(ret < 0)
        {
            return -1;
        }
    }

    if(v_end && a_end)
    {
        return READ_FINISH;
    }

    return READ_FREE;
}

static int GetMin(CDX_U64* offset, int idx)
{
    int i;
    int ret = 0;

    for(i=1; i<idx; i++)
    {
        if(offset[i] < offset[ret])
             ret = i;
    }

    return ret;
}
//************************************************************************/
//*  read a sample sequence in streams   (normal MP4 file) */
//*  Return value  :
//*      1   read finish
//*      0  allright
//*    -1  read error
//************************************************************************/
CDX_S16 MovReadSample(struct CdxMovParser *p)
{
    MOVContext      *c;
    CDX_U64           vas_chunk_ost[MOV_MAX_STREAMS];
    CDX_U64           vas_time[MOV_MAX_STREAMS];
    CDX_U64           curr_sample_offset,curr_sample_size;
    CDX_S32           va_sel = 1;
    MOV_CHUNK       *chunk_info;
    VirCacheContext *vc;
    CDX_S32           idx;
    CdxStreamT*      pb;

    vc = p->vc;
    c = (MOVContext*)p->privData;
    chunk_info = &c->chunk_info;
    pb = c->fp;

    MOVStreamContext *st = NULL;

    int i;
    for(i=0; i<c->nb_streams; i++)
    {
        //CDX_LOGD("+++++ stsd = %d, c->streams[%d]->read_va_end = %d",
        //c->streams[i]->stsd_type, i, c->streams[i]->read_va_end);
        vas_chunk_ost[i] = (CDX_U64)-1LL;
        vas_time[i] = (CDX_U64)-1LL;
    }

    for(i=0; i<c->nb_streams; i++)
    {
        st = c->streams[i];
        if((st->stsd_type && !st->read_va_end))
        {
            // we only need one video stream
            if(st->stsd_type == 1 && st->stream_index != 0)
            {
                st->read_va_end = 1;
                continue;
            }
            vas_chunk_ost[i] = ReadStco(c, st, st->mov_idx_curr.stco_idx);
        }
    }

    // read sample from file offset in network stream,
    // from dts ( not pts ) in local file
    if(CdxStreamIsNetStream(pb))
    {
        va_sel = GetMin(vas_chunk_ost, c->nb_streams);
    }
    else
    {
        for(i=0; i<c->nb_streams; i++)
        {
            st = c->streams[i];
            if(st->stsd_type && !st->read_va_end)
            {
                if(st->stsd_type == 1 && st->stream_index != 0)
                {
                    st->read_va_end = 1;
                    continue;
                }
                vas_time[i] = st->mov_idx_curr.current_dts * 1000 / st->time_scale;
            }
        }
        va_sel = GetMin(vas_time, c->nb_streams);
    }

    c->prefetch_stream_index = va_sel;
    st = c->streams[va_sel];
//------------------ read the stream sample -----------------------------------
    {   //load a new stsc function
        if(st->mov_idx_curr.stsc_idx < st->stsc_size
           && st->mov_idx_curr.chunk_idx >= st->mov_idx_curr.stsc_end)
           {
            idx = st->mov_idx_curr.stsc_idx*12;

            //first chunk id in 'stsc' atom
             st->mov_idx_curr.stsc_first = ReadStsc(c, st, idx) - 1;
             //sample number of this chunk
            st->mov_idx_curr.stsc_samples = ReadStsc(c, st, idx + 4);

            /////******************  care , i do not kown it. maybe for audio seek
            if(st->stsd_type==2 && c->chunk_sample_idx_assgin_by_seek)
            {
                c->chunk_sample_idx_assgin_by_seek = 0;//assigned by seek
                // maybe should      st->chunk_sample_idx_assgin_by_seek = 0;
                CDX_LOGE("***** maybe error here");
            }
            else
            {
                st->mov_idx_curr.chunk_sample_idx = 0;//a new sample idx
            }
            st->mov_idx_curr.stsc_idx++;

            // the next entry of stsc atom
            st->mov_idx_curr.stsc_end = ReadStsc(c, st, idx + 12) - 1;
            // check if it is beyond the boundary
            if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
            {
                st->mov_idx_curr.stsc_end = INT_MAX;
            }

            //? skip one redundance stsc table for stream Tomsk_PSP.mp4,
            // some entry of this table is error
            if(st->mov_idx_curr.stsc_end == st->mov_idx_curr.stsc_first)
            {
                idx = st->mov_idx_curr.stsc_idx*12;
                st->mov_idx_curr.stsc_first = ReadStsc(c, st, idx) - 1;
                st->mov_idx_curr.stsc_samples = ReadStsc(c, st, idx + 4);
                st->mov_idx_curr.chunk_sample_idx = 0;//a new sample idx
                st->mov_idx_curr.stsc_idx++;
                st->mov_idx_curr.stsc_end = ReadStsc(c, st, idx + 12) - 1;

                if(st->mov_idx_curr.stsc_idx >= st->stsc_size)
                {
                    st->mov_idx_curr.stsc_end = INT_MAX;
                }
            }

       }

        //calc current sample offset;
        curr_sample_offset = vas_chunk_ost[va_sel] + st->mov_idx_curr.chunk_sample_size;

        //read current sample size;
        //video or there is stsz table
        if(st->sample_size == 0  || st->stsd_type == 1 || st->stsd_type == 3)
        {
            // the size of this sample in stsz
            curr_sample_size = ReadStsz(c, st, st->mov_idx_curr.stsz_idx<<2);

            //add the current sample size tp the chunk_sample_size to get the next sample offset
            st->mov_idx_curr.chunk_sample_size += curr_sample_size;
            st->mov_idx_curr.stsz_idx++;
        }
        else /*No stsz table, read from stsc for 1 chunk, audio sample size are all same,
        (else only for audio)*/
        {
            //MPX000002.3gp akira.mov will goes here
            if (st->sample_size> 1 )
            {
                if ((st->eCodecFormat == AUDIO_CODEC_FORMAT_PCM) && st->audio_sample_size)
                {
                    if (st->sample_size != 0)
                    {                    //201200621 for ulaw;
                        st->audio_sample_size = st->sample_size;
                    }
                    curr_sample_size = st->mov_idx_curr.stsc_samples * st->audio_sample_size;
                }
                else //MPX000002.3gp akira.mov will goes here
                {
                    curr_sample_size = st->mov_idx_curr.stsc_samples * st->sample_size;
                }
                st->mov_idx_curr.chunk_sample_size += curr_sample_size;
            }
            else if ( st->samples_per_frame > 0 && //akira.mov will goes here
                      (st->mov_idx_curr.stsc_samples * st->bytes_per_frame %
                      st->samples_per_frame == 0))
            {
                if (1 || st->samples_per_frame < 160) //.lys 2012-03-26 always set it to compute!
                {
                    curr_sample_size = st->mov_idx_curr.stsc_samples * st->bytes_per_frame /
                        st->samples_per_frame;
                }
                else
                {
                    //!!maybe multiframe error! see ffmpeg
                    curr_sample_size = st->bytes_per_frame;
                }
                st->mov_idx_curr.chunk_sample_size += curr_sample_size;
            }
            else if (st->sample_size == 1 )//MPX000002.3gp akira.mov P1000100.MOV will goes here
            {
                if(st->audio_sample_size)
                    curr_sample_size = st->mov_idx_curr.stsc_samples * st->audio_sample_size;
                else
                    curr_sample_size = st->mov_idx_curr.stsc_samples * st->sample_size;
                st->mov_idx_curr.chunk_sample_size += curr_sample_size;
            }
            else
            { //kodak.mov will goes here  //!caution, not same as origin source
                if(c->nb_streams<3)
                {
                    curr_sample_size = vas_chunk_ost[0] - vas_chunk_ost[1];
                    st->mov_idx_curr.chunk_sample_size += curr_sample_size;
                }
                else
                {/*has not found any stream, it's too complex to decode the stream,
                not decode the stream */
                    curr_sample_size = 0;
                }
            }
        }

        //calc stts from here  (sample duration)
        {
            //如果当前sample不在累加的sample ( stts_samples_acc)里，继续累加
            if(st->mov_idx_curr.sample_idx >= st->mov_idx_curr.stts_samples_acc)
            {
                CDX_S32 stts_sample_count;
                CDX_S32 stts_sample_duration;

                while(1)  //modify 20090327
                {
                    if(p->exitFlag)
                    {
                        return -1;
                    }

                    idx = st->mov_idx_curr.stts_idx<<3;

                    //sample number of the same duration
                    stts_sample_count    = ReadStts(c, st, idx);
                    //duration
                    stts_sample_duration = ReadStts(c, st, idx+4);

                    st->mov_idx_curr.stts_samples_acc += stts_sample_count;
                    st->mov_idx_curr.stts_duration_acc += (CDX_U64)stts_sample_count *
                        stts_sample_duration;
                    st->mov_idx_curr.stts_sample_duration = stts_sample_duration;
                    st->mov_idx_curr.stts_idx++;

                    //find the sample presentation time
                    if(st->mov_idx_curr.sample_idx < st->mov_idx_curr.stts_samples_acc)
                    {
                        break;
                    }
                }

                /// calculate farmerate in parseStts, so we donot need the code below
                //if(va_sel == 0)
                //{
                //    CDX_S64     tmpLargeVal;

                    //calculate video frame rate
                //    if(c->mov_idx_curr[0].stts_sample_duration)
                //    {
                //        tmpLargeVal = (CDX_S64)c->streams[c->video_stream_idx]->time_scale;
                //        tmpLargeVal = (tmpLargeVal * 1000) /
                //                      (CDX_S64)c->mov_idx_curr[0].stts_sample_duration;
                //        c->ptr_fpsr->vFormat.nFrameRate = (CDX_U32)tmpLargeVal;
                //    }
                //}

            }
            //current pts
            if(st->mov_idx_curr.stts_duration_acc < (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                            (st->mov_idx_curr.stts_samples_acc - st->mov_idx_curr.sample_idx))
            {
                st->mov_idx_curr.current_dts = 0;
            }
            else
            {
                st->mov_idx_curr.current_dts =
                            st->mov_idx_curr.stts_duration_acc -
                            (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                            (st->mov_idx_curr.stts_samples_acc - st->mov_idx_curr.sample_idx);
            }
            // only video sample need add cts
            if(st->stsd_type == 1 && !c->is_fragment && st->eCodecFormat != VIDEO_CODEC_FORMAT_H265)
            {
                if(st->ctts_data && st->ctts_index < st->ctts_size)
                {
                    st->mov_idx_curr.current_dts += (st->dts_shift +
                        st->ctts_data[st->ctts_index].duration);
                    // update ctts context
                    st->ctts_sample ++;
                    if((st->ctts_index < st->ctts_size) &&
                        (st->ctts_data[st->ctts_index].count == st->ctts_sample) )
                    {
                        st->ctts_index ++;
                        st->ctts_sample = 0;
                    }
                }
            }

            if(st->stsd_type == 3)
            {
                st->mov_idx_curr.current_duration = (CDX_U64)st->mov_idx_curr.stts_sample_duration;
            }
            else
            {
                st->mov_idx_curr.current_duration =
                                (CDX_U64)st->mov_idx_curr.stts_sample_duration *
                                (st->mov_idx_curr.stts_samples_acc - st->mov_idx_curr.sample_idx);
            }
        }//calc stts end here

        //save chunk information
        chunk_info->type   = st->stsd_type;
        chunk_info->offset = curr_sample_offset;  // calculate from stsc
        chunk_info->length = curr_sample_size;    // read from stsz

        if(st->sample_size == 0 || st->stsd_type == 1 || st->stsd_type == 3)
        {
            st->mov_idx_curr.sample_idx++;//for video every sample(not every chunck) gets here
            st->mov_idx_curr.chunk_sample_idx++;
        }
        else
        {
            //c->mov_idx_curr[va_sel].sample_idx += curr_sample_size;
            st->mov_idx_curr.sample_idx += st->mov_idx_curr.stsc_samples;
            //modify 080508 every chunck gets
            st->mov_idx_curr.chunk_sample_idx = INT_MAX;
        }

        //read 1 chunk end
        if(st->mov_idx_curr.chunk_sample_idx >= st->mov_idx_curr.stsc_samples)
        {
            st->mov_idx_curr.chunk_sample_idx = 0;//for skiped chunk reset
            st->mov_idx_curr.stco_idx++;
            st->mov_idx_curr.chunk_idx++;
            st->mov_idx_curr.chunk_sample_size = 0;
            if(st->mov_idx_curr.stco_idx >= st->stco_size)
                st->read_va_end = 1;
        }
        //CDX_LOGD("---type = %d stco_idx = %x", st->stsd_type, st->mov_idx_curr.stco_idx);
    }//end of read audio/video sample

    for(i=0; i<c->nb_streams; i++)
    {
        //CDX_LOGD("stsd = %d, c->streams[%d]->read_va_end = %d", c->streams[i]->stsd_type,
        //i, c->streams[i]->read_va_end);
        if((c->streams[i]->read_va_end == 0) && c->streams[i]->stsd_type)
        {
            break;
        }
    }

    if(i==c->nb_streams)
    {
        CDX_LOGW("retrun read finish\n");
        return READ_FINISH;
    }

    return READ_FREE;
}

/************************************************************************/
/*  Find from current pos's nearest keyframe index                      */
/************************************************************************/
CDX_S32 MovFindKeyframe(struct CdxMovParser *p, CDX_S32 direction,
                        CDX_U32 curr_sample_num)
{
    MOVContext    *c;
    //VirCacheContext *vc = p->vc;
    CDX_U32   stss_size;
    CDX_U32   keyframe_num;
    CDX_S32    low,high,i,nexti;
    MOVStreamContext    *st;

    c = (MOVContext*)p->privData;
    st = c->streams[c->video_stream_idx];
    //seek to stss atom
    stss_size = st->stss_size;

    //protect stss_idx region
    if(st->mov_idx_curr.stss_idx >= (int)stss_size)
        st->mov_idx_curr.stss_idx = stss_size-1;
    if(st->mov_idx_curr.stss_idx < 0)
        st->mov_idx_curr.stss_idx = 0;

    if(stss_size < 1)
    {
        return -1;
    }

    int FrameInterval;
    st->mov_idx_curr.stss_idx  = 0;

    keyframe_num = ReadStss(c, st, st->mov_idx_curr.stss_idx<<2);
    if((curr_sample_num == 0) || (st->mov_idx_curr.stss_idx == (int)st->stss_size))
    {
        return (keyframe_num - 1);
    }

    FrameInterval = (keyframe_num - curr_sample_num);//absolution
    if(FrameInterval < 2048 && FrameInterval > -2048)
    {
        int cmpflag = 0;

        if(keyframe_num < curr_sample_num)
            cmpflag = 1;

        while((-1 < st->mov_idx_curr.stss_idx) && (st->mov_idx_curr.stss_idx < (int)stss_size))
        {
            if(p->exitFlag)
            {
                return -1;
            }

            keyframe_num = ReadStss(c, st, st->mov_idx_curr.stss_idx<<2);

            //CDX_LOGD("2.keyframe_num=%d, curr_sample_num = %d",  keyframe_num, curr_sample_num);
            if(keyframe_num < curr_sample_num)
            {
                if(!cmpflag)
                {
                    break;
                }
                st->mov_idx_curr.stss_idx++;
            }
            else
            {
                if(cmpflag)
                {
                    //keyframe_num = lastKeyFrameNum;
                    break;
                }
                st->mov_idx_curr.stss_idx--;
            }
        }
        if(st->mov_idx_curr.stss_idx < 0)
        {
            st->mov_idx_curr.stss_idx = 0;
        }
    }
    else
    {
        low = 0;
        high = stss_size-1;
        nexti = (low+high)>>1;
        while (low+1 < high)
        {
            if(p->exitFlag)
            {
                return -1;
            }

            i = nexti;

            keyframe_num = ReadStss(c, st, nexti<<2);

            if(keyframe_num < curr_sample_num)
            {
                low = i;
                nexti = (low+high)>>1;
            }
            else
            {
                high = i;
                nexti = (low+high)>>1;
            }
        }

        st->mov_idx_curr.stss_idx = high;
    }

    // not used now
    if(direction == 1 && st->mov_idx_curr.stss_idx > 0)
    {
        st->mov_idx_curr.stss_idx--;
    }
    return (keyframe_num-1);
}

//------------------------------------------------------------------------------

static CDX_S32 MovParseTkhd(MOVContext *c, MOV_atom_t a)
{
    CDX_S32 i;
    CDX_S32 width;
    CDX_S32 height;
    //CDX_S64 disp_transform[2];
    CDX_S32 display_matrix[3][2];
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = a.offset;
    CDX_S32 version = buffer[offset];

    offset += 4;        /* flags */

    if (version == 1) {
        MoovGetBe64(buffer+offset);
        offset += 8;
        MoovGetBe64(buffer+offset);
        offset += 8;
    } else {
        MoovGetBe32(buffer+offset);    /* creation time */
        offset += 4;
        MoovGetBe32(buffer+offset);    /* modification time */
        offset += 4;
    }

    st->id = (CDX_S32)MoovGetBe32(buffer+offset); /* track id (NOT 0 !)*/
    offset += 4;
    MoovGetBe32(buffer+offset);        /* reserved */
    offset += 4;
    //st->start_time = 0; /* check */
    (version == 1) ? MoovGetBe64(buffer+offset) : MoovGetBe32(buffer+offset);
    /* highlevel (considering edits) duration in movie timebase */
    offset += (version == 1) ? 8 : 4;

    //MoovGetBe32(buffer+offset);        /* reserved */
    offset += 4;
    //MoovGetBe32(buffer+offset);        /* reserved */
    offset += 4;

    offset += 8;
     /* layer (2bytes)*/
     /* alternate group (2bytes)*/
     /* volume (2bytes)*/
    /* reserved (2bytes)*/

    //read in the display matrix (outlined in ISO 14496-12, Section 6.2.2)
    // they're kept in fixed point format through all calculations
    // ignore u,v,z b/c we don't need the scale factor to calc aspect ratio
    //                       | a     b      u |
    //  (x, y, 1)   *   | c     d      v |   = (x', y', 1)
    //                       | tx   ty    w |
    for (i = 0; i < 3; i++) {
        display_matrix[i][0] = MoovGetBe32(buffer+offset);   // 16.16 fixed point
        offset += 4;
        display_matrix[i][1] = MoovGetBe32(buffer+offset);   // 16.16 fixed point
        offset += 4;
        MoovGetBe32(buffer+offset);           // 2.30 fixed point (not used)
        offset += 4;
    }

    { //below code are used for android
        CDX_U32 rotationDegrees;
        CDX_S32 a00 = display_matrix[0][0];
        CDX_S32 a01 = display_matrix[0][1];
        CDX_S32 a10 = display_matrix[1][0];
        CDX_S32 a11 = display_matrix[1][1];

        static const CDX_S32 kFixedOne = 0x10000;
        if (a00 == kFixedOne && a01 == 0 && a10 == 0 && a11 == kFixedOne) {
            // Identity, no rotation
            rotationDegrees = 0;
            strcpy((char*)st->rotate, "0");
        } else if (a00 == 0 && a01 == kFixedOne && a10 == -kFixedOne && a11 == 0) {
            rotationDegrees = 90;
             strcpy((char*)st->rotate, "90");
        } else if (a00 == 0 && a01 == -kFixedOne && a10 == kFixedOne && a11 == 0) {
            rotationDegrees = 270;
             strcpy((char*)st->rotate, "270");
        } else if (a00 == -kFixedOne && a01 == 0 && a10 == 0 && a11 == -kFixedOne) {
            rotationDegrees = 180;
             strcpy((char*)st->rotate, "180");
        } else {
            CDX_LOGW("We only support 0,90,180,270 degree rotation matrices");
             strcpy((char*)st->rotate, "0");
            rotationDegrees = 0;
        }

        if (rotationDegrees != 0) {
            CDX_LOGD("nRotation variable is not in vFormat");
            //c->ptr_fpsr->vFormat.nRotation = rotationDegrees;
        }
    }

    width = MoovGetBe32(buffer + offset);       // 16.16 fixed point track width
    offset += 4;
    height = MoovGetBe32(buffer+offset);      // 16.16 fixed point track height
    offset += 4;
    st->width = width >> 16;
    st->height = height >> 16;
    CDX_LOGD("tkhd width = %d, height = %d", st->width, st->height);
    // transform the display width/height according to the matrix
    // skip this if the display matrix is the default identity matrix
    // or if it is rotating the picture, ex iPhone 3GS
    // to keep the same scale, use [width height 1<<16]

    return 0;
}

static CDX_S32 MovParseElst(MOVContext *c, MOV_atom_t a)
{
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = a.offset;

    CDX_S32 i, entries;

    offset += 4; /* version */ /* flags */

    entries = MoovGetBe32(buffer+offset);
    offset += 4;
    if((CDX_U64)entries*12+8 > a.size)  //CDX_U64
        return -1;

    for(i=0; i<entries; i++)
    {
        CDX_S32 time;
        CDX_S32 duration = MoovGetBe32(buffer+offset); /* Track duration */
        offset += 4;
        time = MoovGetBe32(buffer+offset); /* Media time */
        offset += 4;
        //MoovGetBe32(pb); /* Media rate */
        offset += 4;
        if (i == 0 && time >= -1)
        {
            st->time_offset = time != -1 ? time : -duration;
        }
    }
    return 0;

}

/* map numeric codes from mdhd atom to ISO 639 */
/* cf. QTFileFormat.pdf p253, qtff.pdf p205 */
/* http://developer.apple.com/documentation/mac/Text/Text-368.html */
/* deprecated by putting the code as 3*5bit ascii */
static const char mov_mdhd_language_map[][4] = {
    /* 0-9 */
    "eng", "fra", "ger", "ita", "dut", "sve", "spa", "dan", "por", "nor",
    "heb", "jpn", "ara", "fin", "gre", "ice", "mlt", "tur", "hr "/*scr*/, "chi"/*ace?*/,
    "urd", "hin", "tha", "kor", "lit", "pol", "hun", "est", "lav",    "",
    "fo ",    "", "rus", "chi",    "", "iri", "alb", "ron", "ces", "slk",
    "slv", "yid", "sr ", "mac", "bul", "ukr", "bel", "uzb", "kaz", "aze",
    /*?*/
    "aze", "arm", "geo", "mol", "kir", "tgk", "tuk", "mon",    "", "pus",
    "kur", "kas", "snd", "tib", "nep", "san", "mar", "ben", "asm", "guj",
    "pa ", "ori", "mal", "kan", "tam", "tel",    "", "bur", "khm", "lao",
    /*                   roman? arabic? */
    "vie", "ind", "tgl", "may", "may", "amh", "tir", "orm", "som", "swa",
    /*==rundi?*/
    "kin", "run", "nya", "mlg", "epo",    "",    "",    "",    "",    "",
    /* 100 */
       "",    "",    "",    "",    "",    "",    "",    "",    "",    "",
       "",    "",    "",    "",    "",    "",    "",    "",    "",    "",
       "",    "",    "",    "",    "",    "",    "",    "", "wel", "baq",
    "cat", "lat", "que", "grn", "aym", "tat", "uig", "dzo", "jav"
};

static int MovLangToISO639(unsigned int code, char to[4])
{
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

    int i;
    memset(to, 0, 4);

    if((code >= 0x400) && (code != 0x7fff))
    {
        for(i=2; i>=0; i--)
        {
            to[i] = 0x60 + (code & 0x1f);
            code >>= 5;
        }

        return i;
    }

    /* Macintosh Language Codecs */
    if(code >= ARRAY_ELEMS(mov_mdhd_language_map))
    {
        return 0;
    }
    if(!mov_mdhd_language_map[code][0])
    {
        return 0;
    }
    memcpy(to, mov_mdhd_language_map[code], 4);
    return 1;
}

/**************************************************************************************/
//* mdhd atom is in trak atom
//* specifies the characteristics of a media,(a stream)
//* including time scale and duration.
/**************************************************************************************/
static CDX_S32 MovParseMdhd(MOVContext *c, MOV_atom_t a)
{

    MOVStreamContext *st = c->streams[c->nb_streams-1];
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = a.offset;

    CDX_S32 version = buffer[offset];
    CDX_S32 lang;

    if (version > 1)
        return 1;    /* unsupported */

    offset += 4;    /* version and flags */

    if (version == 1)
    {
        //MoovGetBe64(buffer+offset);
        offset += 8;
        //MoovGetBe64(buffer+offset);
        offset += 8;
    }
    else
    {
        //MoovGetBe32(buffer+offset);    /* creation time */
        offset += 4;
        //MoovGetBe32(buffer+offset);    /* modification time */
        offset += 4;
    }

    st->time_scale = MoovGetBe32(buffer+offset);
    offset += 4;
    st->duration = (version == 1) ? MoovGetBe64(buffer+offset) : MoovGetBe32(buffer+offset);
    /* duration */
    offset += (version == 1) ? 8 : 4;

    //st->totaltime = st->duration/st->time_scale;
    st->totaltime = (CDX_S32)((CDX_S64)st->duration*1000/st->time_scale);   //fuqiang
    st->basetime = st->time_offset/st->time_scale;

    //**< the language of this media
    lang = MoovGetBe16(buffer+offset); /* language */
    offset += 2;

    char language[4] = {0};
    if(MovLangToISO639(lang, language))
    {
       CDX_LOGD("-- language = %s", language);
       strcpy((char *)st->language, language);
    }
    //MoovGetBe16(buffer+offset); /* quality */
    offset += 2;

    return 0;
}

static CDX_S32 MovParseHdlr(MOVContext *c, MOV_atom_t a)
{

    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_U32 type;

    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;

    //int err = 0;

    offset += 4;  /* version */ /* flags */

    offset += 4;   //component type
    type = MoovGetLe32(buffer+offset); /* component subtype */
    offset += 4;

    //!latest mplayer del it     if(!ctype)
    //!latest mplayer del it         c->isom = 1;

    if(type == MKTAG('v', 'i', 'd', 'e'))
    {
        st->codec.codecType = CODEC_TYPE_VIDEO;
        //c->video_stream_idx = c->nb_streams-1;
        c->has_video ++;
        c->ptr_fpsr->hasVideo ++;
        c->basetime[0] = st->basetime;    //auido basetime -/+
    }

    else if(type == MKTAG('s', 'o', 'u', 'n'))
    {
        if(c->has_audio < AUDIO_STREAM_LIMIT)//select the first audio track as default
        {
            st->codec.codecType = CODEC_TYPE_AUDIO;
            c->has_audio++;
            c->ptr_fpsr->hasAudio++;
        }
    }
    //else if(type == MKTAG('t', 'e', 'x', 't') || type == MKTAG('s', 'u', 'b', 'p')){
    else if (type == MKTAG('t', 'e', 'x', 't'))
    {
        if(c->has_subtitle < SUBTITLE_STREAM_LIMIT)
        { //select the first subtitle track as default
            st->codec.codecType = CODEC_TYPE_SUBTITLE;
            c->has_subtitle++;
            c->ptr_fpsr->hasSubTitle++;
        }
    }

    /* component  manufacture     (4 bytes)*/
    /* component flags                    (4 bytes)*/
    /* component flags mask         (4 bytes)*/
   /*   Companent name         (variable )  */
    return 0;
}

#if 0
static int MovParseSvq3(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset-8;  // care , extradata of svq3 contains aize and type
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    if (c->nb_streams < 1)
        return 0;

    if((CDX_U64)a.size > (1<<30))
        return -1;

    //LOGD("--- <%x%x%x%x>", buffer[offset], buffer[offset+1], buffer[offset+2], buffer[offset+3]);
    if(st->codec.extradata)
    {
        free(st->codec.extradata);
        st->codec.extradata = NULL;
    }

    st->codec.extradata = malloc(a.size);
    if (!st->codec.extradata)
        return -1;
    st->codec.extradataSize = a.size;
    memcpy(st->codec.extradata, buffer+offset, a.size);
    CDX_LOGD("---  extradata_size = %d", st->codec.extradataSize);
    return 0;
}
#endif

/*
glbl - QuickTime is not contain this atom
*/
static int MovParseGlbl(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    if (c->nb_streams < 1)
        return 0;

    if((CDX_U64)a.size > (1<<30))
        return -1;

    //LOGD("--- <%x%x%x%x>", buffer[offset], buffer[offset+1], buffer[offset+2], buffer[offset+3]);
    if(st->codec.extradata)
    {
        free(st->codec.extradata);
        st->codec.extradata = NULL;
    }

    st->codec.extradata = malloc(a.size -8);
    if (!st->codec.extradata)
        return -1;
    st->codec.extradataSize = a.size-8;
    memcpy(st->codec.extradata, buffer+offset, a.size-8);
    //CDX_LOGD("---  extradata_size = %d", st->codec.extradataSize);
    return 0;
}

/*
'avcC'
contain the avc extra_data ( sps and pps )
*/
static int MovParseAvcc(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    if(st->codec.extradata){
        CDX_LOGW("mov extradata has been init???");
        free(st->codec.extradata);
        st->codec.extradata = NULL;
    }

    st->codec.extradata = malloc(a.size-8);
    if(!st->codec.extradata)
        return -1;

    st->codec.extradataSize = a.size - 8;

    CDX_LOGV("mov_read_avcc size:%d",a.size);
    memcpy(st->codec.extradata, buffer+offset, a.size-8);

    return 0;
}

static int Mp4ParseDescr(MOVContext *c,int *tag, unsigned int *offset)
{
    unsigned char* buffer = c->moov_buffer;
    int len=0;
    int count = 4;

    *tag = buffer[(*offset)++];
    while (count--)
    {
        int ch = buffer[(*offset)++];
        len = (len << 7) | (ch & 0x7f);
        if (!(ch & 0x80))
        {
            break;
        }
    }
    return len;
}

static int MovParseHvcc(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    if(st->codec.extradata){
        CDX_LOGW("mov extradata has been init???");
        free(st->codec.extradata);
    }

    st->codec.extradata = malloc(a.size-8);
    if(!st->codec.extradata)
        return -1;

    st->codec.extradataSize = a.size-8;

    memcpy(st->codec.extradata, buffer+offset, a.size-8);
    return 0;
}

/*
MPEG-4 Elementary Stream Descriptor Atom ('esds')
This atom is a required extension to the sound sample description for MPEG-4 audio.
This atom contains an elementary stream descriptor, which is defined in ISO/IEC FDIS 14496.
*/
static int MovParseEsds(MOVContext *c, MOV_atom_t a)
{
//---- esds section
#define MP4ESDescrTag                   0x03
#define MP4DecConfigDescrTag            0x04
#define MP4DecSpecificDescrTag          0x05

    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    int tag, len = 0;

    offset += 4; /* version + flags */
      len = Mp4ParseDescr(c, &tag, &(offset));
      //LOGD("edsd len = %x, tag = %x.", len, tag);

    if (tag == MP4ESDescrTag)
    {
        /* ID (2 bytes)*/
        /* priority  (1 bytes)*/
        offset += 3;
    }
    else
    {
        offset += 2; /* ID(2 bytes) */
    }

    len = Mp4ParseDescr(c, &tag, &(offset));
    //LOGD("edsd len = %x, tag = %x.", len, tag);
    if (tag == MP4DecConfigDescrTag)
    {
        int object_type_id = buffer[offset ++];

         /* stream type (1byte)*/
         /* buffer size db (3bytes) */
         /* max bitrate (4 bytes)*/
         /* avg bitrate (4bytes)*/
        offset += 12;
        switch (object_type_id)
        {
            case 32:
            {
                st->eCodecFormat = VIDEO_CODEC_FORMAT_XVID;
                break;
            }
            case 33:
            {
                st->eCodecFormat = VIDEO_CODEC_FORMAT_H264;
                break;
            }
            case 108:
            {
                st->eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;//sorenson video3
                break;
            }
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x6a: //mpeg1
            {
                st->eCodecFormat = VIDEO_CODEC_FORMAT_MPEG2;
                st->eCodecFormat = (object_type_id == 0x6a ? VIDEO_CODEC_FORMAT_MPEG1 :
                                    VIDEO_CODEC_FORMAT_MPEG2);
                break;
            }
            case 64:
            case 102:
            case 103:
            case 104:
            {
                st->eCodecFormat = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
                break;
            }
            case 105:
            case 107:  //0x6b
            {
                st->eCodecFormat = AUDIO_CODEC_FORMAT_MP3;
                break;
            }
            case 0xdd: //OGG
            {
                CDX_LOGW("it is ogg audio, not support");
                st->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
                break;
            }
        }

        len = Mp4ParseDescr(c, &tag, &(offset));
        //LOGD("edsd len = %x, tag = %x.", len, tag);
        if (tag == MP4DecSpecificDescrTag)
        {
             if(len > 16384)
                 return -1;
            st->codec.extradata = (CDX_S8*)malloc(len);
            if (!st->codec.extradata)
                return -1;
            st->codec.extradataSize = len;
            memcpy(st->codec.extradata, buffer+offset, len);

            if(st->eCodecFormat == VIDEO_CODEC_FORMAT_MPEG4
                || st->eCodecFormat == VIDEO_CODEC_FORMAT_DIVX3
                || st->eCodecFormat == VIDEO_CODEC_FORMAT_DIVX4
                || st->eCodecFormat == VIDEO_CODEC_FORMAT_XVID)
            {
                int width=0,height=0;

                mov_getvolhdr((CDX_U8*)st->codec.extradata,len,&width,&height);
                if(width!=0 && height!=0)
                {
                    st->codec.width  = width;
                    st->codec.height = height;
                    CDX_LOGD("esds width = %d, height = %d", st->codec.width, st->codec.height);
                }
            }
       }
    }
    return 0;

}

/*
'wave'
*/
static int MovParseWave(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = offset + a.size - 8;
    int err = 0;

    if(a.size > (1<<30))
        return -1;

    if (a.size > 16)
    {
        /* to read frma, esds atoms */
        while((offset < total_size) && !err)
        {
            a.size = MoovGetBe32(buffer+offset);
            offset += 4;
            a.type = MoovGetLe32(buffer+offset);
            offset += 4;
            if((a.size==0) || (a.type == 0))
            {
                CDX_LOGV("mov atom is end!");
                break;
            }

            if(a.size == 1)  /* 64 bit extended size */
            {
                a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
                offset += 8;
            }

            if(a.type == MKTAG( 'f', 'r', 'm', 'a' ))
            {
                //LOGD("--xuqi-- frma");
                offset = offset + a.size - 8;
            }
            else if(a.type == MKTAG( 'e', 's', 'd', 's' ))
            {
                //LOGD("--xuqi-- esds");
                a.offset = offset;
                err = MovParseEsds(c, a);
                offset = offset + a.size - 8;
            }
            else
            {
                offset = offset + a.size - 8;
            }

        }
    }
    else
        offset += a.size-8;
    return 0;
}

static CDX_S32 MovParseStsd(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_S32 entries;
    CDX_U32 format;
    CDX_S32 bits_per_sample = 0;
    CDX_S32 bytes_per_packet = 0;
    int remaind_size = 0;
    unsigned int total_size = 0;
    int err = 0;

    offset += 4;  /*version and  flags */

    entries = MoovGetBe32(buffer+offset); //stsd entries
    if(entries > 2)
    {
        logw("stsd entry: %d", entries);
        entries = 2;
    }
    offset += 4;

    //Parsing Sample description table
    while(entries--)
    {
        MOV_atom_t a = { 0, 0, 0 };
        CDX_U32 start_pos = offset;  // entry body start position
        CDX_U32 size = MoovGetBe32(buffer+offset);  /* size */
        offset +=4;
        format = MoovGetLe32(buffer+offset);        /* data format */
        //CDX_LOGD("---offset = %x, stsd format  = %c%c%c%c", offset, buffer[offset],
        //buffer[offset+1], buffer[offset+2], buffer[offset+3]);
        offset +=4;
        st->codec.codecTag = format;

        // cmov_kongfupanda.mov have two avc1 atom in stsd, but we only need one of them
        if(((st->codec.codecType==CODEC_TYPE_VIDEO)&&(st->stsd_type != 1))
           || ((st->codec.codecType==CODEC_TYPE_AUDIO &&(st->stsd_type != 2)))
           || (/*(st->codec.codecType == CODEC_TYPE_SUBTITLE )&&*/(st->stsd_type != 3)))
        {
            switch (st->codec.codecTag)
            {
                //parse video format type
                case MKTAG('m', 'p', '4', 'v'):
                case MKTAG('X', 'V', 'I', 'D'):
                case MKTAG('3', 'I', 'V', '2'):
                case MKTAG('D', 'X', '5', '0'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_XVID;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('D', 'I', 'V', '3'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_DIVX3;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('D', 'I', 'V', 'X'):// OpenDiVX
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_DIVX4;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('a','v','c','1'):
                case MKTAG('A','V','C','1'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_H264;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('h','v','c','1'):
                case MKTAG('H','V','C','L'):
                case MKTAG('h','e','v','1'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_H265;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('s','2','6','3'):
                case MKTAG('h','2','6','3'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_H263;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;

                case MKTAG('J', 'P', 'E', 'G'):
                case MKTAG('j', 'p', 'e', 'g'):
                case MKTAG('m', 'j', 'p', 'a')://锟剿达拷锟斤拷锟斤拷要锟斤拷写
                case MKTAG('m', 'j', 'p', 'b'):
                case MKTAG('d', 'm', 'b', '1'):
                case MKTAG('A', 'V', 'D', 'J'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('m', 'p', 'e', 'g'):
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_MPEG1;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('h', 'd', 'v', '1') : /* HDV 720p30 */
                case MKTAG('h', 'd', 'v', '2') : /* MPEG2 produced by Sony HD camera */
                case MKTAG('h', 'd', 'v', '3') : /* HDV produced by FCP */
                case MKTAG('h', 'd', 'v', '5') : /* HDV 720p25 */
                case MKTAG('m', 'x', '5', 'n') : /* MPEG2 IMX NTSC 525/60 50mb/s produced by FCP */
                case MKTAG('m', 'x', '5', 'p') : /* MPEG2 IMX PAL 625/50 50mb/s produced by FCP */
                case MKTAG('m', 'x', '4', 'n') : /* MPEG2 IMX NTSC 525/60 40mb/s produced by FCP */
                case MKTAG('m', 'x', '4', 'p') : /* MPEG2 IMX PAL 625/50 40mb/s produced by FCP */
                case MKTAG('m', 'x', '3', 'n') : /* MPEG2 IMX NTSC 525/60 30mb/s produced by FCP */
                case MKTAG('m', 'x', '3', 'p') : /* MPEG2 IMX PAL 625/50 30mb/s produced by FCP */
                case MKTAG('x', 'd', 'v', '2') :  /* XDCAM HD 1080i60 */
                case MKTAG('A', 'V', 'm', 'p'):    /* AVID IMX PAL */
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_MPEG2;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;
                case MKTAG('v', 'c', '-', '1'):    /* AVID IMX PAL */
                CDX_LOGD(" the codec atom is vc-1, maybe the codec format is not right");
                    st->eCodecFormat = VIDEO_CODEC_FORMAT_WMV1;
                    st->stsd_type = 1;
                    st->stream_index = c->video_stream_num;
                    c->video_stream_num++;
                    break;

                case MKTAG('S', 'V', 'Q', '3'):
                    CDX_LOGD("---- can not support svq3");
                    st->stsd_type = 0;
                    st->unsurpoort = 1;
                    st->read_va_end = 1;
                    break;

                //parse audio format type
                case MKTAG('m','p','4','a'):
                case MKTAG('M','P','4','A'):
                case MKTAG('a','a','c',' '):
                case MKTAG('A','C','C',' '):
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;
                case MKTAG('a','l','a','c'):
                    st->eCodecFormat =  AUDIO_CODEC_FORMAT_ALAC;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;
                case MKTAG('m','s', 0 ,'U'):
                case MKTAG('m','s', 0 ,'P'):
                case MKTAG('.','m','p','3'):
                    CDX_LOGD("----- mp3");
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_MP3;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    CDX_LOGD("--- audio stream index = %d", c->audio_stream_num);
                    break;

                case MKTAG('.','m','p','2'):
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_MP2;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                //pcm type, adpcm/raw pcm/alaw pc/ulaw pcm/...
                case MKTAG('u','l','a','w'):    //u-law pcm
                case MKTAG('U','L','A','W'):    //u-law pcm
                    bits_per_sample = 8;
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_MULAW | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case MKTAG('a','l','a','w'):    //a-law pcm
                case MKTAG('A','L','A','W'):    //a-law pcm
                    bits_per_sample = 8;
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_ALAW | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case MKTAG('i', 'n', '3', '2'): //32bit signed, little ending
                case MKTAG('i', 'n', '2', '4'): //24bit signed, little ending
                case MKTAG('l', 'p', 'c', 'm'): //lpcm
                case MKTAG('N', 'O', 'N', 'E'): //uncompressed
                case MKTAG('r', 'a', 'w', ' '):    //raw pcm
                case MKTAG('R', 'A', 'W', ' '):    //raw pcm
                    bits_per_sample = 8;
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_PCM | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

               case MKTAG('i', 'm', 'a', '4'):  //IMA-4 ADPCM
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_DVI_ADPCM | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case MKTAG('s','o','w','t'):    //signed/two\'s complement little endian
                case MKTAG('S','O','W','T'):    //signed/two\'s complement little endian
                    bits_per_sample = 16;
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_PCM | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case MKTAG('t','w','o','s'):    //signed/two\'s complement big endian
                case MKTAG('T','W','O','S'):    //signed/two\'s complement big endian
                    bits_per_sample = 16;
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_PCM | ABS_EDIAN_FLAG_BIG;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case MKTAG('s','a','m','r'):    //amr narrow band
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
                    st->eSubCodecFormat = AMR_FORMAT_NARROWBAND;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;
                case MKTAG('s','a','w','b'):    //amr wide band
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
                    st->eSubCodecFormat = AMR_FORMAT_WIDEBAND;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;
                case MKTAG('W', 'M', 'A', '2') ://CODEC_ID_WMAV2
                    st->eCodecFormat =  AUDIO_CODEC_FORMAT_WMA_STANDARD;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;
                case MKTAG('A','C','-','3'):
                case MKTAG('E','C','-','3'):
                case MKTAG('a','c','-','3'):
                case MKTAG('e','c','-','3'):
                    st->eCodecFormat =  AUDIO_CODEC_FORMAT_AC3;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    st->stream_index = c->audio_stream_num;
                    c->audio_stream_num++;
                    break;

                case 0x1100736d:
                    loge("unsupport this audio type");
                    st->unsurpoort = 1;
                    st->read_va_end = 1;
                    break;

                //parse subtitle format type
                case MKTAG('T', 'E', 'X', 'T'):
                case MKTAG('T', 'X', '3', 'G'):
                case MKTAG('t', 'e', 'x', 't'):
                case MKTAG('t', 'x', '3', 'g'):
                    CDX_LOGW(" subtitle is not define yet!!!!");
                    st->codec.codecType = CODEC_TYPE_SUBTITLE;  //for Jumanji.mp4, which 'hdlr'
                                                                //atom do not have 'text'
                    st->eCodecFormat = SUBTITLE_CODEC_TIMEDTEXT;        // LYRIC_TXT;
                    st->eSubCodecFormat = SUBTITLE_TEXT_FORMAT_UTF8;    //LYRIC_SUB_ENCODE_UTF8;
                    st->stsd_type = 3;
                    st->stream_index = c->subtitle_stream_num;
                    c->subtitle_stream_num++;
                    break;
                default:
                    CDX_LOGW("unknown format tag: <0x%x>", st->codec.codecTag);
                    if(st->stsd_type == 0) // for cmov_kongfupanda.mov
                    {
                        st->stsd_type = 0;
                        st->read_va_end = 1; // we do not need read samples of this stream,
                                             // so set the end flag
                    }
                    break;
            }
        }
        else if((st->codec.codecType!=CODEC_TYPE_VIDEO)
                && (st->codec.codecType!=CODEC_TYPE_AUDIO)
                && ((st->codec.codecType!=CODEC_TYPE_SUBTITLE)))
        {
            CDX_LOGW("unknown format tag: <%d>", st->codec.codecTag);
            st->stsd_type = 0;
            st->read_va_end = 1;   // we do not need read samples of this stream,
                                   //so set the end flag
        }

        if(st->codec.codecType==CODEC_TYPE_AUDIO && (format&0xFFFF) == 'm'+('s'<<8))//
        {
            format >>= 16;
            format = ((format&0xff)<<8) | ((format>>8)&0xff);
            switch(format)
            {
                case 0x02:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_MS | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x11:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_IMA_WAV | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x20:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_YAMAHA | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x6:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_ALAW | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x7:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_MULAW | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x45:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = WAVE_FORMAT_G726_ADPCM | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x50:
                case 0x55:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_MP3;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    break;
                case 0x61:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_IMA_DK4 | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x62:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_IMA_DK3 | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0xFF:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    break;
                case 0x160:
                case 0x161:
                case 0x162:
                    st->eCodecFormat =  AUDIO_CODEC_FORMAT_WMA_STANDARD;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    break;
                case 0x200:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_CT | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                case 0x2000:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_AC3;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    break;
                case 0x2001:
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_DTS;
                    st->eSubCodecFormat = 0;
                    st->stsd_type = 2;
                    break;
                case (('S'<<8)+'F'):
                    st->eCodecFormat = AUDIO_CODEC_FORMAT_PCM;
                    st->eSubCodecFormat = ADPCM_CODEC_ID_SWF | ABS_EDIAN_FLAG_LITTLE;
                    st->stsd_type = 2;
                    break;
                default:
                    CDX_LOGW("unkown format <%x>", format);
                    st->stsd_type = 0;
                       break;
            }
        }

         /* reserved (6 bytes)*/
         /* index  (2 bytes)*/
        offset += 8;
        //parse the video sample description if it is a vedio trak
        if(st->codec.codecType == CODEC_TYPE_VIDEO)
        {
             /* version             (2 bytes)*/
             /* revision level         (2bytes) */
             /* vendor             (4 bytes)*/
             /* temporal quality         (4 bytes)*/
             /* spacial quality         (4 bytes)*/
            offset += 16;

            st->codec.width = MoovGetBe16(buffer+offset); /* width */
            offset += 2;
            st->codec.height = MoovGetBe16(buffer+offset); /* height */
            offset += 2;
            CDX_LOGD("stsd width = %d, height = %d", st->codec.width, st->codec.height);
            //c->ptr_fpsr->vFormat.nWidth = st->codec.width;
            //c->ptr_fpsr->vFormat.nHeight = st->codec.height;

            /* horiz resolution         (4 bytes)*/
            /* vert resolution         (4 bytes)*/
            /* data size, always 0     (4 bytes)*/
            /* frames per samples     (2 bytes)*/
            offset += 14;

            offset += 32;//skip codec_name
            st->codec.bitsPerSample = MoovGetBe16(buffer+offset); /* depth */
            offset += 2;

#if 1
            //parser color table
            {
                int color_table_id = 0;
                int color_depth = 0;
                int color_greyscale = 0;

                color_table_id = MoovGetBe16(buffer+offset); /* colortable id */
                offset += 2;
                /* figure out the palette situation */
                color_depth = st->codec.bitsPerSample & 0x1F;
                color_greyscale = st->codec.bitsPerSample & 0x20;

                /* if the depth is 2, 4, or 8 bpp, file is palettized */
                if ((color_depth == 2) || (color_depth == 4) ||
                    (color_depth == 8))
                {
                    /* for palette traversal */
                    unsigned int color_start, color_count, color_end;
                    unsigned char r, g, b;

                    if (color_greyscale) {

                    } else if (color_table_id) {

                    } else {
                        unsigned int j;
                        /* load the palette from the file */
                        color_start = MoovGetBe32(buffer+offset);
                        offset += 4;
                        color_count = MoovGetBe16(buffer+offset);
                        offset += 2;
                        color_end   = color_count;  //no use, just for avoid compiler warning
                        color_end = MoovGetBe16(buffer+offset);
                        offset += 2;

                        if ((color_start <= 255) &&
                            (color_end <= 255)) {
                            for (j = color_start; j <= color_end; j++) {
                            /* each R, G, or B component is 16 bits;
                            * only use the top 8 bits; skip alpha bytes
                                * up front */
                                offset += 2;
                                r = g; g= b; b= r;  //no use, just for avoid compiler warning
                                r = buffer[offset];
                                offset ++;
                                g = buffer[offset];
                                offset ++;
                                b = buffer[offset];
                                offset ++;
                            }
                        }
                    }
                }
            }
#endif
        }
        else if(st->codec.codecType==CODEC_TYPE_AUDIO)
        {
            CDX_U32 version = MoovGetBe16(buffer+offset);
            offset +=2;

            /* revision level (2 bytes) */
            /* vendor (4 bytes)*/
            offset += 6;
            st->codec.channels = MoovGetBe16(buffer+offset);    /* channel count */
            offset += 2;

            //Formats using more than 16 bits per sample set this field to 16 and use
            //sound version 1.
            st->codec.bitsPerSample = MoovGetBe16(buffer+offset);  /* sample size (8 or 16)*/
            offset += 2;
            /* do we need to force to 16 for AMR ? */

            /* handle specific s8 codec */
            /* compression id = 0 (2bytes)*/
            /* packet size = 0 (2bytes)*/
            offset += 4;

            // sample rate
            st->codec.sampleRate = ((MoovGetBe32(buffer+offset) >> 16));
            offset += 4;

            //Read QT version 1 fields. In version 0 these do not exist.
            if(!c->isom)
            {
                if(version==1)
                {
                    st->samples_per_frame = MoovGetBe32(buffer+offset);
                    offset += 4;
                    //get_be32(pb); /* bytes per packet */
                    bytes_per_packet = MoovGetBe32(buffer+offset); /* bytes per packet */
                    offset += 4;
                    st->bytes_per_frame = MoovGetBe32(buffer+offset);
                    offset += 4;
                    if(st->bytes_per_frame && st->eCodecFormat==AUDIO_CODEC_FORMAT_PCM)
                    {
                        //what is the meaning of  the code below, it will error if PCM audio
                        //c->ptr_fpsr->aFormat.nCodecSpecificDataLen = st->bytes_per_frame;
                    }
                    offset += 4; /* bytes per sample */
                }
                else if(version==2)
                {
                    offset += 4; /* sizeof struct only */
                    {
                        /*st->codec.sample_rate = av_int2dbl(get_be64(pb));  float 64 */
                        union av_intfloat64{
                            long long i;
                            double f;
                        };
                        union av_intfloat64 v;
                        v.i = MoovGetBe64(buffer+offset);
                        offset += 8;

                        st->codec.sampleRate = v.f;
                    }
                    st->codec.channels = MoovGetBe32(buffer+offset);
                    offset += 4;
                     /* always 0x7F000000 */
                     /* bits per channel if sound is uncompressed */
                     /* lcpm format specific flag */
                     /* bytes per audio packet if constant */
                     /* lpcm frames per audio packet if constant */
                     offset += 20;
                }
            }

            if (bits_per_sample)
            {
                if(version == 0)
                {
                    st->codec.bitsPerSample = bits_per_sample;
                    c->audio_sample_size = (st->codec.bitsPerSample >> 3) * st->codec.channels;
                    st->audio_sample_size = c->audio_sample_size;
                }
                else if(version == 1)
                {
                    st->codec.bitsPerSample = (bytes_per_packet << 3);
                    if (st->eCodecFormat == AUDIO_CODEC_FORMAT_PCM)
                    {
                        st->codec.bitsPerSample = bits_per_sample;
                    }
                    c->audio_sample_size = bytes_per_packet * st->codec.channels;
                    st->audio_sample_size = c->audio_sample_size;
                }
                else
                {
                      if(version == 2 && st->codec.codecTag!=MKTAG('l', 'p', 'c', 'm'))
                    {
                        st->codec.bitsPerSample = bits_per_sample;
                    }
                    c->audio_sample_size = (bits_per_sample >> 3) * st->codec.channels;
                    st->audio_sample_size = c->audio_sample_size;
                }
            }
        }
        else if (st->codec.codecType == CODEC_TYPE_SUBTITLE)
        {
            // ttxt stsd contains display flags, justification, background
            // color, fonts, and default styles, so fake an atom to read it
            MOV_atom_t fake_atom = { 0, 0, 0 };
            fake_atom.size = size -(offset - start_pos);
            //LOGD("----------- subtitle  size = %d", fake_atom.size);
            if (format != MKTAG('m','p','4','s'))
            {
                CDX_LOGD("-------- subtitle glbl, format = %x, offset = %x, size = %d",
                    format, offset, fake_atom.size);
                fake_atom.offset = offset;
                MovParseGlbl(c, fake_atom);
                st->codec.width = st->width;
                st->codec.height = st->height;
                offset += (fake_atom.size);
            }
            else
            {
                CDX_LOGI(" Be Careful: We do not support the 'mp4s' subtitle");
            }

        }
        else
        {
            /* other codec type, just skip the entry body (rtp, mp4s, tmcd ...) */
            CDX_LOGW(" We do not support this atom: <0x%x>", st->codec.codecType);
            return 0;
        }

        /* this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...) */
        remaind_size = size - (offset - start_pos);
        total_size = offset + remaind_size;
        if (remaind_size > 8)
        {
            while( (offset < total_size) && !err )
            {
                remaind_size = size - (offset - start_pos);
                if(remaind_size < 8)
                {
                    offset += remaind_size;
                }
                a.size = MoovGetBe32(buffer+offset);
                offset += 4;
                a.type = MoovGetLe32(buffer+offset);
                offset += 4;

                if((a.size==0) || (a.type == 0))
                {
                    CDX_LOGV("mov atom is end!");
                    break;
                }

                if(a.size == 1)  /* 64 bit extended size */
                {
                    a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
                    offset += 8;
                }

                if(a.type == MKTAG( 'a', 'v', 'c', 'C' ))
                {
                    a.offset = offset;
                    err = MovParseAvcc(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'h', 'v', 'c', 'C' ))
                {
                    a.offset = offset;
                    err = MovParseHvcc(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'a', 'l', 'a', 'c' ))
                {
                    a.offset = offset;
                    err = MovParseAvcc(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'w', 'a', 'v', 'e' ))
                {
                    a.offset = offset;
                    err = MovParseWave(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'e', 's', 'd', 's' ))
                {
                    a.offset = offset;
                    err = MovParseEsds(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'g', 'l', 'b', 'l' ))
                {
                    a.offset = offset;
                    err = MovParseGlbl(c, a);
                    offset = offset + a.size - 8;
                }
                else if(a.type == MKTAG( 'S', 'M', 'I', ' ' ))
                {
                    //a.offset = offset;
                    //err = MovParseSvq3(c, a);
                    offset = offset + a.size - 8;
                }
                else
                {
                    offset = offset + a.size - 8;
                }
            }

        }
        else if (a.size > 0) // 0<size && size < 8
        {
            offset += a.size;
        }
    }

    if(st->codec.codecType==CODEC_TYPE_AUDIO && st->codec.sampleRate==0 && st->time_scale>1)
    {
        st->codec.sampleRate= st->time_scale;
    }
    return 0;
}

/*
Time-to-sample atoms store duration information for every media's samples, providing a mapping
from a time in a media to the corresponding data sample.
*/
static CDX_S32 MovParseStts(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    int size = a.size - 8;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_U32 entries;

    offset += 4; /* version and flags */
    entries = MoovGetBe32(buffer+offset);
    offset += 4;
    size -= 8;

    if(entries == 0)
    {
        CDX_LOGW("---- stts entries is %d, careful", entries);
    }

    if(size <= 0)
    {
        return 0;
    }
    if(entries >= UINT_MAX/8)
        return -1;

    st->stts_size = entries;
    st->stts_offset = offset;

    // get the first sample duration to caculate the framerate of video
    if(st->codec.codecType == CODEC_TYPE_VIDEO)
    {
        offset += 4;
        st->sample_duration = MoovGetBe32(buffer+offset);
    }

    return 0;
}

/*
'stss' - The sync sample atom identifies the key frames in the media.
the key frame is the sample id
*/
static CDX_S32 MovParseStss(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_U32 entries;

    offset += 4; /* version and flags */
    entries = MoovGetBe32(buffer+offset);
    offset +=4;

    if(entries >= MAXENTRY)//720000=25fps @ 8hour I:P=1:3//modify by lys UINT_MAX / sizeof(CDX_S32))
        return -1;
    st->stss_size = entries;
    st->stss_offset = offset;
    return 0;
}

/*
'stsz' - Sample Size Atoms
specify the size of each sample in the media
*/
static CDX_S32 MovParseStsz(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    //int size = a.size - 8;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    CDX_S32 entries, sample_size;

    offset += 4; /* version and flags */

    //If all the samples are the same size, this field contains
    //that size value. If this field is set to 0, then the samples
    //have different sizes, and those sizes are
    //stored in the sample size table.
    sample_size = MoovGetBe32(buffer+offset);
    offset += 4;
    //modify by lys //if (!st->sample_size) /* do not overwrite value computed in stsd */
    st->sample_size = sample_size;
    CDX_LOGD("-- sample_size = %d", sample_size);
    entries = MoovGetBe32(buffer+offset);
    offset += 4;
    st->stsz_size = entries;
    st->stsz_offset = offset;
    return 0;
}

/*
'stsc' -   Sample-to-Chunk Atoms
The sample-to-chunk atom contains a table that maps
samples to chunks in the media data stream
*/
static CDX_S32 MovParseStsc(MOVContext *c, MOV_atom_t a)
{

    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_S32 entries;

    offset += 4; /* version and flags */
    entries = MoovGetBe32(buffer+offset);
    offset += 4;

    if(entries == 0)
    {
        CDX_LOGW("---- stsc entries is %d, careful", entries);
    }
    if(entries >= MAXENTRY || (int)a.size < entries)//modify by lys UINT_MAX / sizeof(MOV_stsc_t))
        return -1;
    st->stsc_size = entries;
    st->stsc_offset = offset;

    if(st->eCodecFormat==AUDIO_CODEC_FORMAT_PCM && entries==1
        && !st->codec.extradataSize && st->codec.codecType==CODEC_TYPE_AUDIO)
    {
        MoovGetBe32(buffer+offset);
        offset += 4;
        // if will error,if add the code below
        //st->codec.extradataSize = MoovGetBe32(buffer+offset);
        offset += 4;
        CDX_LOGW("read pcm size from stsc! 0x%x", st->codec.extradataSize);
    }
    return 0;
}

// ctts - composition time ( CTS )
static CDX_S32 MovParseCtts(MOVContext *c, MOV_atom_t a)
{
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))

    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    CDX_U32 entries;

    CDX_S32 version = buffer[offset];
    if(version == 1)
    {
        CDX_LOGW("Composition to Decode Box(cslg) must be here, see qtff");
    }

    offset += 4; // version and flags
    entries = MoovGetBe32(buffer+offset);
    offset += 4;

    CDX_LOGD("track[%d].ctts.entries = %d", c->nb_streams-1, entries);
    if(!entries)
        return 0;
    if(entries >= MAXENTRY || a.size < entries)
        return -1;
    st->ctts_data = malloc(entries * sizeof(*st->ctts_data));
    if(!st->ctts_data)
        return -1;

    st->ctts_size = entries;
    st->ctts_offset = offset;

    unsigned int i;
    int count;
    int duration;
    for(i=0; i<entries; i++)
    {
        count = MoovGetBe32(buffer+offset);
        offset += 4;
        duration = MoovGetBe32(buffer+offset);
        offset += 4;

        st->ctts_data[i].count    = count;
        st->ctts_data[i].duration = duration;

        if((FFABS(duration)>(1<<28)) && (i+2<entries))
        {
            CDX_LOGW("CTTS invalid!");
            free(st->ctts_data);
            st->ctts_data = NULL;
            st->ctts_size = 0;
            return 0;
        }

        // dts_shift, see ffmpeg
        if(i+2< entries)
        {
            if(duration < 0)
            {
                st->dts_shift = (st->dts_shift>(-duration)) ? st->dts_shift : (-duration);
            }
        }
    }

    // in trun atom also have ctts, so we need to copy the ctts data to a new buffer
    return 0;
}

/*
'stco' -  Chunk Offset Atoms
Chunk offset atoms identify the location of each chunk of data in the media's data stream
*/
static CDX_S32 MovParseStco(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];

    CDX_S32 entries;

    offset += 4; /* version and flags */

    entries = MoovGetBe32(buffer+offset);
    offset += 4;

    if(entries == 0)
    {
        CDX_LOGW("---- stco entries is %d, maybe an ISO base media file", entries);
    }

    if(entries >= MAXENTRY || (int)a.size < entries)//720000=25fps @ 8hour
        return -1;

    st->stco_size = entries;
    st->stco_offset = offset;

    if (a.type == MKTAG('c', 'o', '6', '4'))
    {
        st->co64 = 1;
    }

    return 0;
}

/*
'sbgp' -  Sample-To-Group Atoms
find the group that a sample belongs to and the associated description
of that sample group
*/
static CDX_S32 MovParseSbgp(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    MOVStreamContext *st = c->streams[c->nb_streams-1];
    unsigned int grouping_type;

    CDX_S32 entries;
    int i;

    int version = buffer[offset];
    offset += 4; /* version and flags */

    grouping_type = MoovGetLe32(buffer+offset);
    offset += 4;

    logd("========= grouping_type:0x%x", grouping_type);
    if(grouping_type != MKTAG('r', 'a', 'p', ' '))
        return 0;
    if(version == 1)
        offset += 4;  /* grouping_type_parameter */

    entries = MoovGetBe32(buffer+offset);
    offset += 4;
    if(entries == 0)
    {
        CDX_LOGW("---- sbgp entries is %d", entries);
    }

    if(entries >= MAXENTRY)
        return -1;

    st->rap_seek = malloc(entries*4);
    if(st->rap_seek == NULL)
        return -1;

    int sample_index = 0; // frame number
    int rap_goup_index = 0;
    for(i=0; i<entries; i++)
    {
        /* sample_count */
        sample_index +=  MoovGetBe32(buffer+offset);
        offset += 4;

        /* group_description_index */
        rap_goup_index = MoovGetBe32(buffer+offset);
        offset += 4;

        if(rap_goup_index > 0)
        {
            // it is a frame which can seek from here (not keyframe)
            st->rap_seek[st->rap_seek_count] = sample_index;
            st->rap_seek_count++;
        }
    }

    st->rap_group_count = i;
    logd("==== st->rap_group_count: %d, st->rap_seek_count: %d", st->rap_group_count, st->rap_seek_count);
    return 0;
}

/*
'stbl' - The sample table atom contains information for converting from media time to sample number to sample
location. This atom also indicates how to interpret the sample (for example, whether to decompress the video
data and, if so, how). This section describes the format and content of the sample table atom.
*/
int MovParseStbl(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size-8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;
        if((a.size==0) || (a.type == 0))
        {
            break;
        }

        if(a.size == 1)  /* 64 bit extended size */
        {
            a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
            offset += 8;
        }

        if(a.type == MKTAG( 's', 't', 's', 'd' ))
        {
            a.offset = offset;
            err = MovParseStsd(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 't', 's' ))
        {
            a.offset = offset;
            err = MovParseStts(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 's', 's' ))
        {
            a.offset = offset;
            err = MovParseStss(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 's', 'z' ))
        {
            a.offset = offset;
            err = MovParseStsz(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 's', 'c' ))
        {
            a.offset = offset;
            err = MovParseStsc(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 'c', 'o' ))
        {
            a.offset = offset;
            err = MovParseStco(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'c', 't', 't', 's' ))
        {
            CDX_LOGI(" !!!! careful ctts atom is tested yet");
            a.offset = offset;
            err = MovParseCtts(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'c', 'o', '6', '4' ))
        {
            a.offset = offset;
            err = MovParseStco(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 'b', 'g', 'p' ))
        {
            logd("============ sbgp");
            a.offset = offset;
            err = MovParseSbgp(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }
    }
    return 0;
}

/*Media information atom
store handler-specific information for a track's media data.
The media handler uses this information to map from media time to media data
and to process the media data.

contain the following data elements:
size(4 bytes)
type(4 bytes)
'vmhd'-Viedo media information atom
'hdlr' - handler refrence atom
'dinf' - data information atom
'stbl' - sample table atom
*/
static int MovParseMinf(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size-8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;
        if((a.size==0) || (a.type == 0))
        {
            break;
        }

        if(a.size == 1)  /* 64 bit extended size */
        {
            a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
            offset += 8;
        }

        if(a.type == MKTAG( 'h', 'd', 'l', 'r' ))
        {
            //LOGD("--xuqi-- hdlr in minf");
            a.offset = offset;
            err = MovParseHdlr(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 's', 't', 'b', 'l' ))
        {
            //LOGD("--xuqi-- stbl");
            a.offset = offset;
            err = MovParseStbl(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }

    }

    return 0;
}

static int MovParseMdia(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;
        if((a.size==0) || (a.type == 0))
        {
            //LOGV("mov atom is end!");
            break;
        }

        if(a.size == 1)  /* 64 bit extended size */
        {
            a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
            offset += 8;
        }

        if(a.type == MKTAG( 'm', 'd', 'h', 'd' ))
        {
            //LOGD("--xuqi-- mdhd");
            a.offset = offset;
            err = MovParseMdhd(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'h', 'd', 'l', 'r' ))
        {
            //LOGD("--xuqi-- hdlr");
            a.offset = offset;
            err= MovParseHdlr(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'm', 'i', 'n', 'f' ))
        {
            //LOGD("--xuqi-- minf");
            a.offset = offset;
            err= MovParseMinf(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }

    }
    return 0;
}

int MovParseTrak(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        if(offset+a.size > total_size)
        {
            logw("atom size error: %x", a.size);
            a.size = total_size - offset;
        }
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;

        if((a.size==0) || (a.type == 0))
        {
            //LOGV("mov atom is end!");
            break;
        }

        if(a.size == 1)  /* 64 bit extended size */
        {
            a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
            offset += 8;
        }

        if(a.type == MKTAG( 't', 'k', 'h', 'd' ))
        {
            //LOGD("--xuqi-- tkhd");
            a.offset = offset;
            err = MovParseTkhd(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'e', 'l', 's', 't' ))
        {
            //LOGD("--xuqi-- elst");
            a.offset = offset;
            err = MovParseElst(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'm', 'd', 'i', 'a' ))
        {

            //LOGD("--xuqi-- mdia");
            a.offset = offset;
            err = MovParseMdia(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }

    }
    return 0;
}

int MovParseUdta(MOVContext *c, MOV_atom_t a);

static int MovParseMeta(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    int err = 0;

    offset += 4;

    while (offset < total_size && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;

        if(a.type == MKTAG( 'i', 'l', 's', 't' ))
        {
            a.offset = offset;
            err = MovParseUdta(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }
    }
    return 0;
}

static void MovParseGnre(unsigned char *buffer, CDX_U8 *str, int size)
{
    int genre;
    unsigned int offset = 0;
    offset ++; // unkown

    genre = buffer[offset];
    CDX_LOGD("--- genre %s", ff_id3v1_genre_str[genre-1]);
    snprintf((char*)str, size, "%s", ff_id3v1_genre_str[genre-1]);
}

static void MovParseUdtaString2(unsigned char *buffer, CDX_U8 *str, int size)
{
    unsigned int offset = 0;
    CDX_U16 str_size = MoovGetBe16(buffer+offset); /* string length */;

    offset += 4; /* skip language */

    memcpy(str, buffer+offset, MIN(size, str_size));
}

/* user data atom */
int MovParseUdta(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    //int err = 0;

    int length;
    unsigned int next;

    while (offset < total_size)
    {
        a.size   = MoovGetBe32(buffer+offset);
        offset  += 4;
        a.type   = MoovGetLe32(buffer+offset);
        offset  += 4;
        next     = a.size - 8;

        if (next > total_size) // stop if tag_size is wrong
            break;

        switch (a.type)
        {
        case MKTAG('m', 'e', 't', 'a'):
            a.offset = offset;
            MovParseMeta(c, a);
            break;

        case MKTAG(0xA9,'n','a','m'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->title, sizeof(c->title));
            length = MIN(length, 31);
            c->title[length] = '\0';
            break;
        case MKTAG(0xA9,'w','r','t'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->writer, sizeof(c->writer));
            length = MIN(length, 31);
            c->writer[length] = '\0';
            break;

        case MKTAG(0xA9,'a','u','t'):
        case MKTAG(0xA9,'A','R','T'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->artist, sizeof(c->artist));
            length = MIN(length, 31);
            c->artist[length] = '\0';
            break;

        case MKTAG(0xA9,'d','a','y'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->date, sizeof(c->date));
            length = MIN(length, 31);
            c->date[length] = '\0';
            break;
        case MKTAG(0xA9,'a','l','b'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->album, sizeof(c->album));
            length = MIN(length, 31);
            c->album[length] = '\0';
            break;
        case MKTAG('a','A','R','T'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->albumArtistic, sizeof(c->albumArtistic));
            length = MIN(length, 31);
            c->albumArtistic[length] = '\0';
            break;
        case MKTAG(0xA9,'g','e','n'):
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->genre, sizeof(c->genre));
            length = MIN(length, 31);
            c->genre[length] = '\0';
            break;
        case MKTAG('g','n','r','e'):
            //length = a.size-8-4-1;
            CDX_LOGD("---- gnre, care");
            MovParseGnre(buffer+offset, c->genre, sizeof(c->genre));
            //length = MIN(length, 31);
            //c->genre[length] = '\0';
            break;
//        case MKTAG(0xa9,'c','p','y'):
//            mov_parse_udta_string(pb, c->fc->copyright, sizeof(c->fc->copyright));
//            break;
//        case MKTAG(0xa9,'i','n','f'):
//            mov_parse_udta_string(pb, c->fc->comment,   sizeof(c->fc->comment));
//            break;
        case MKTAG(0xA9, 'x', 'y', 'z'):
            // it is the geometry position of user, which is very important in a CTS test
            length = a.size-8-4-1;
            MovParseUdtaString2(buffer+offset, c->location, sizeof(c->location));
            length = MIN(length, 31);
            c->location[length] = '\0';
            break;
        default:
            break;
        }

        offset += next;
    }

    return 0;
}

/*
*      in the moov atom, if the trex atom is exist, the mp4 file is a fragment mp4
*      the duration from mvhd is not representing the whole file
*      @ set up default values used by movie fragment
*/
static int MovParseTrex(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    //unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom

    if(a.size < 32)
    {
        CDX_LOGI("warning: trex box <%d> is less than 32 bytes !", a.size);
        return 0;
    }

    MOVTrackExt* trex;
    trex = realloc(c->trex_data, (c->trex_num+1)*sizeof(*c->trex_data));
    if(!trex) return -1;
    c->trex_data = trex;

    trex = &c->trex_data[c->trex_num++];
    MoovGetBe32(buffer+offset); // version(1byte) and flag( 3bytes )
    offset += 4;

    trex->track_id = MoovGetBe32(buffer+offset);
    offset += 4;
    trex->stsd_id  = MoovGetBe32(buffer+offset);
    offset += 4;
    trex->duration = MoovGetBe32(buffer+offset);
    offset += 4;
    trex->size     = MoovGetBe32(buffer+offset);
    offset += 4;
    trex->flags    = MoovGetBe32(buffer+offset);

    //LOGD("%x, %x, %x, %x, %x",trex->track_id, trex->stsd_id, trex->duration,
    //trex->size, trex->flags );
    return 0;
}

/********************************************************/
//only present in ISO media file format ( DASH )
// in 'moov' atom, set the default infomation of these segments
/****************************************************/
static int MovParseMvex(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        a.size = MoovGetBe32(buffer+offset);
        offset += 4;
        a.type = MoovGetLe32(buffer+offset);
        offset += 4;

        if(a.type == MKTAG( 't', 'r', 'e', 'x' ))
        {
            a.offset = offset;
            err = MovParseTrex(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }

    }
    return err;
}

/********************************************************************************/
//* specify the characteristics of an entire QuickTime movie.
//*  defines characteristics of the entire QuickTime movie,
//* such as time scale and duration.
//* in DASH, the duration maybe 0 here
/********************************************************************************/
static int MovParseMvhd(MOVContext *c, MOV_atom_t a)
{
    unsigned int offset = a.offset;
    unsigned char* buffer = c->moov_buffer;
    unsigned int total_size = a.offset + a.size - 8;   //size of 'trak' atom
    int err = 0;

    while( (offset < total_size) && !err )
    {
        CDX_S32 version = buffer[offset]; /* version */
        offset += 4; /* flags */
        //MoovGetBe64(buffer + offset);

        if (version == 1)
        {
            //MoovGetBe64(buffer + offset);
            offset += 8;
            //MoovGetBe64(buffer + offset);
            offset += 8;
        }
        else if(version == 0)
        {
            //MoovGetBe32(buffer+offset); /* creation time */
            offset += 4;
            //MoovGetBe32(buffer+offset); /* modification time */
            offset += 4;
        }
        else
        {
            CDX_LOGW("version<%d> is not support!", version);
        }

        c->time_scale = MoovGetBe32(buffer+offset); /* time scale */
        offset += 4;

        CDX_U64 duration = 0;
        if(version == 1)
        {
            duration = MoovGetBe64(buffer+offset);
            c->duration = (CDX_S32)duration;
            offset += 8;
        }
        else
        {
            duration = MoovGetBe32(buffer+offset);
            c->duration = (CDX_S32)duration;
            offset += 4;
        }
        if(c->duration != 0)
        {
            c->mvhd_total_time = duration*1000 / c->time_scale;
        }
        //LOGD("duration = %llx, timescale = %x", duration, c->time_scale);
        //LOGD("mvhd duration = %d", c->mvhd_total_time);
        offset = offset +6+10+36+28;
    }
    return err;
}

int MovParserMoov(MOVContext *c);
#ifdef __ZLIB
static int MovParseCmov(MOVContext *c, MOV_atom_t a)
{
    uint8_t *cmov_data;
    uint8_t *moov_data; /* uncompressed data */
    long cmov_len, moov_data_len, dcom_len, cmvd_len;
    int ret = -1;
    int offset = 0;
    //unsigned int type;

    /* cmov */
    cmov_data = c->moov_buffer + a.offset; /* exclude cmov atom header */
    cmov_len = a.size;

    /* dcom */
    dcom_len = MoovGetBe32(cmov_data + offset);
    if (dcom_len != 12)
    {
        CDX_LOGE("invalid data, <%ld>", dcom_len);
        return -1;
    };
    offset += 4;
    if (MoovGetLe32(cmov_data + offset) != MKTAG('d', 'c', 'o', 'm'))
    {
        CDX_LOGE("invalid data");
        return -1;
    };
    offset += 4;
    if (MoovGetLe32(cmov_data + offset) != MKTAG('z', 'l', 'i', 'b'))
    {
        CDX_LOGE("invalid data");
        return -1;
    };
    offset += 4;

    /* cmvd */
    cmvd_len = MoovGetBe32(cmov_data + offset);
    offset += 4;
    if (MoovGetLe32(cmov_data + offset) != MKTAG('c', 'm', 'v', 'd'))
    {
        CDX_LOGE("invalid data");
        return -1;
    };
    offset += 4;

    moov_data_len = MoovGetBe32(cmov_data + offset); /* uncompressed size */
    offset += 4;

    moov_data = malloc(moov_data_len);
    CDX_CHECK(moov_data);

    if (uncompress(moov_data, (uLongf *)&moov_data_len,
            (const Bytef *)cmov_data + offset, cmov_len - offset) != Z_OK)
    {
        CDX_LOGE("uncompress cmov data fail.");
        free(moov_data);
        return -1;
    }

    free(c->moov_buffer);
    c->moov_buffer = moov_data;
    //FILE* fp = fopen("/mnt/sdcard/cmov.txt", "wb");
    //fwrite(c->moov_buffer, 1, moov_data_len, fp);
    //fclose(fp);

    ret = MovParserMoov(c);
    return ret;
}
#endif

//parse the data in the moov_buffer
int MovParserMoov(MOVContext *c)
{
    MOV_atom_t a = {0, 0, 0};
    CDX_S32 err = 0;
    unsigned char* buffer = c->moov_buffer;
    unsigned int offset = 0;
    unsigned int total_size = 0;

    a.size = MoovGetBe32(buffer+offset);
    offset += 4;
    a.type = MoovGetLe32(buffer+offset);
    offset += 4;
    if((a.size==0) || (a.type == 0))
    {
        CDX_LOGV("mov atom is end!");
    }

    total_size = a.size - 8;  // size of 'moov' atom

    //LOGD("buf[0] = %x, buf[1]= %x, buf[2]=%x, buf[3]=%x",
    //buffer[0], buffer[1], buffer[2], buffer[3]);
    //LOGD("size = %x, type = %x, totalsize = %x", a.size, a.type, total_size);

    while( (offset < total_size) && !err)
    {
        if(a.size >= 8)
        {
            a.size = MoovGetBe32(buffer+offset);
            offset += 4;
            a.type = MoovGetLe32(buffer+offset);
            offset += 4;
            if((a.size==0) || (a.type == 0))
            {
                CDX_LOGV("mov atom is end!");
                break;
            }
        }

        if(a.size == 1)  /* 64 bit extended size */
        {
            a.size = (unsigned int)(MoovGetBe64(buffer+offset)-8);
            offset += 8;
        }

        if(a.type == MKTAG( 'm', 'v', 'h', 'd' ))
        {
            a.offset = offset ;
            err = MovParseMvhd(c, a);
            offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'u', 'd', 't', 'a' ))
        {
            a.offset = offset ;
            err = MovParseUdta(c, a);
            offset = offset + a.size - 8;
        }

        else if(a.type == MKTAG( 't', 'r', 'a', 'k' ))
        {
            MOVStreamContext *st;

            if (c->nb_streams < MOV_MAX_STREAMS)
            {
                st = AvNewStream(c, c->nb_streams);
                if (!st)
                    return -2;
            }
            else
            {
                CDX_LOGW("the stream of this file is large than MOV_MAX_STREAMS !!!");
                return -2;
            }

            c->nb_streams++;
            st->codec.codecType = CODEC_TYPE_DATA;
            st->codec.extradata = NULL;
                c->streams[c->nb_streams-1] = st;

            a.offset = offset ;
                err = MovParseTrak(c, a);
                offset = offset + a.size - 8;
        }
        else if(a.type == MKTAG( 'm', 'v', 'e', 'x' ))
        {
            // we cannot sure it fragment mp4, if has mvex atom, // camera record for IPAD
            if(c->bDash)
            {
                c->is_fragment = 1;
            }
            a.offset = offset ;
            err = MovParseMvex(c, a);
            offset = offset + a.size - 8;
        }
#ifdef __ZLIB
        else if(a.type == MKTAG( 'c', 'm', 'o', 'v' ))
        {
            a.offset = offset;
            err = MovParseCmov(c, a);
            break;
            //offset = offset + a.size - 8; // do we need it?
        }
#endif
        else if(a.type == MKTAG( 'u', 'd', 't', 'a' ))
        {
            a.offset = offset ;
            err = MovParseUdta(c, a);
            offset = offset + a.size - 8;
        }
        else
        {
            offset = offset + a.size - 8;
        }

    }
    return 0;
}

//get the top level atom of mov, and read the 'moov' atom data to moov_buffer
static int MovTop(MOVContext *s, CdxStreamT* pb)
{
    MOV_atom_t a = {0, 0, 0};
    int ret = 0;
    int file_offset = 0;
    int datalen = 0;
    int readlen = 0;
    int tmplen = 0;
    unsigned char buf[1024] = {0};

    while(ret > -1)
    {
        a.offset = CdxStreamTell(pb);
        ret = CdxStreamRead(pb, buf, 8);
        if(ret < 8)
        {
            CDX_LOGI("end of file? reslut(%d)", ret);
            break;
        }

        a.size = MoovGetBe32(buf);   //big endium
        a.type = MoovGetLe32(buf+4);
        //CDX_LOGD("type = %x, size=%x, offset = %d", a.type, a.size, a.offset);
        if((a.size == 0) || (a.type == 0))
        {
            break;
        }

        if(a.type == MKTAG( 'f', 't', 'y', 'p' ))
        {
            CDX_U32 major_brand;
            int   minor_ver;
            ret = CdxStreamRead(pb, buf, a.size-8);
                major_brand = MoovGetLe32(buf);
            minor_ver = MoovGetBe32(buf+4);

            if(a.size < 16 || a.size > 1024)
            {
                CDX_LOGE("error, the ftyp atom is invalid");
                return -1;
            }

            char* compatible = (char*)buf+8;
            buf[a.size-8] = '\0';
            CDX_LOGD("---- compatible = %s", compatible);
                //if the major brand and compatible brand are neithor "qt  ",
                //it is ISO base media file
               if (major_brand != MKTAG('q','t',' ',' ') && !strstr(compatible, "qt  "))
               {
                s->isom = 1;
            }
        }
        else if (a.type == MKTAG( 'w', 'i', 'd', 'e' ))  //if have the 'wide' atom, it must
                                                         //before the 'mdat' atom
        {
            // sometimes, the mdat is in the wide atom
            if(a.size != 8)
            {
                CDX_LOGW("care, the 'wide' atom size is not 8 <%d> ", a.size);
                if(s->bSeekAble)
                {
                    ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                    if(ret < 0) return -1;
                }
                else
                {
                    ret = CdxStreamSkip(pb, a.size-8);
                    if(ret < 0) return -1;
                }
            }
        }
        else if (a.type == MKTAG( 'm', 'o', 'o', 'v' ))
        {
            //in dash, 'moov' atom will come twice, one for video init segment, another one for
            //audio init segment
            // if the video bitrate changed, there were be a 'moov' atom in the middle of file
            if(!s->found_moov)
            {
                s->found_moov = 1;
                s->moov_buffer = (unsigned char*)malloc(a.size);
                s->moov_size = a.size;
                if(!s->moov_buffer)
                {
                   return -1;
                }
                memset(s->moov_buffer, 0, a.size);
                memcpy(s->moov_buffer, buf, 8);
                datalen = a.size-8 ;
                readlen = 0;
                while ( datalen > 0 )
                {
                    if (datalen > MAX_READ_LEN_ONCE)
                                    tmplen = MAX_READ_LEN_ONCE;
                                else
                                    tmplen = datalen;
                                ret = CdxStreamRead(pb, s->moov_buffer+8+readlen , tmplen);
                                if(ret < 0)
                                {
                                    CDX_LOGE("CdxStreamRead error!ret (%d)",ret);
                                    return -1;
                                }
                                readlen += tmplen;
                                datalen = datalen - tmplen;
                }
            }
            else
            {
                CDX_LOGI("duplicated moov atom, skip it!");
                if(s->bSeekAble)
                {
                    ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                    if(ret < 0) return -1;
                }
                else
                {
                    ret = CdxStreamSkip(pb, a.size-8);
                    if(ret < 0) return -1;
                }
            }
#if 0
            FILE* fp = fopen("/mnt/sdcard/moov.s", "wb");
            fwrite(s->moov_buffer, 1, s->moov_size, fp);
            fclose(fp);
#endif
            ret = MovParserMoov(s);
            if(s->found_mdat)
            {
                break;
            }
        }
        else if (a.type == MKTAG( 'm', 'd', 'a', 't' ))
        {
            s->mdat_count++;
               s->found_mdat = 1;
               if(s->found_moov)
               {
                   break;
               }

            if(a.size == 1)
            {
                // if the mdat size is 1, is a extend mdat size.
                // the 8 size after 'mdat' is the true size
                ret = CdxStreamRead(pb, buf, 8);

                CDX_S64 size = MoovGetBe64(buf);

                   if(s->bSeekAble)
                {
                    //CDX_LOGD("---size=%llx, offset = %llx", size, CdxStreamTell(pb));
                    ret = CdxStreamSeek(pb, size-16, SEEK_CUR);

                    if(ret < 0) return -1;
                }
                else
                {
                    unsigned char* buf = malloc(size-16);
                    ret = CdxStreamRead(pb, buf, size-16);
                    free(buf);
                    if(ret < 0) return -1;
                }
            }
            else
            {
                   if(s->bSeekAble)
                {
                    ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                    if(ret < 0) return -1;
                }
                else
                {
                    unsigned char* buf = malloc(a.size-8);
                    ret = CdxStreamRead(pb, buf, a.size-8);
                    free(buf);
                    if(ret < 0) return -1;
                }
               }
        }
        else if (a.type == MKTAG( 's', 't', 'y', 'p' ))
        {
            //if every segment has a url, the 'styp' must be in the first box in the media segmnet
            //LOGD("--xuqi-- styp");
            s->first_styp_offset = file_offset;
            s->is_fragment = 1;
            if(s->bSeekAble)
            {
                ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                if(ret < 0) return -1;
            }
            else
            {
                ret = CdxStreamSkip(pb, a.size-8);
                if(ret < 0) return -1;
            }
        }
        else if (a.type == MKTAG( 's', 'i', 'd', 'x' ))
        {
            //offset and duration of the chunk in segment
            //sidx is used for dash (one segment), local file is not needed
            s->is_fragment = 1;
            s->sidx_flag = 1;

            ret = MovParseSidx(s, pb, a);
        }
        else if (a.type == MKTAG( 'm', 'o', 'o', 'f' ))
        {
            CDX_LOGD("the mov file contain movie fragment box!");
            s->is_fragment = 1;
            if(!s->first_moof_offset)
            {
                s->first_moof_offset = CdxStreamTell(pb)-8;
            }
            s->fragment.moof_offset = CdxStreamTell(pb)-8;
            if(s->bSeekAble)
            {
                ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                if(ret < 0) return -1;
            }
            else
            {
                ret = CdxStreamSkip(pb, a.size-8);
                if(ret < 0) return -1;
            }
            break;
            //MovParseMoof(s, pb, a);
        }
        else
        {
            if(s->bSeekAble)
            {
                ret = CdxStreamSeek(pb, a.size-8, SEEK_CUR);
                if(ret < 0) return -1;
            }
            else
            {
                ret = CdxStreamSkip(pb, a.size-8);
                if(ret < 0) return -1;
            }
        }
    }

    return 0;
}

CDX_S32 CdxMovOpen(struct CdxMovParser *p, CdxStreamT *stream)
{
    CDX_S32           result;
    MOVContext      *c;
    if(!p)
    {
        return -1;
    }

    c = (MOVContext*)p->privData;
    c->fp = stream;
    if(!c->fp)
    {
        return -1;
    }

    if(p->bSmsSegment) //* for sms, has no moov
    {
        MOVStreamContext *st;
        st = AvNewStream(c, c->nb_streams);
        if(!st)
        {
            CDX_LOGE("new stream failed.");
            return -1;
        }
        st->stsd_type = 1; //* 1:v 2:a lin
        st->stream_index = 0;
        st->codec.codecType = CODEC_TYPE_VIDEO;

        c->nb_streams++;
        c->streams[c->nb_streams-1] = st;

        c->is_fragment = 1;
        c->has_video = 1;
        c->video_stream_num = 1;
        c->video_stream_idx = 0;

        c->streams[0]->time_scale = 10000000; //* sms     / scale
        return 0;
    }
    result = MovTop(c, c->fp);
    if(result != 0)
    {
        return -1;
    }

    if(c->video_stream_num > 1)
    {
        CDX_LOGW("---  video stream number <%d>, only support one video stream",
            c->video_stream_num);
    }

    if(!c->has_audio && !c->has_video)
    {
        CDX_LOGW("Neither audio nor video is recognized!\n");
        return -1;
    }

    return 0;
}

CDX_S16 CdxMovClose(struct CdxMovParser *p)
{
    MOVContext  *c;
    CDX_S32       i;

    if(!p)
    {
        return -1;
    }

    c = (MOVContext*)p->privData;

    if(c)
    {
        if(c->sidx_buffer)
        {
            free(c->sidx_buffer);
            c->sidx_buffer = NULL;
        }
        if(c->moov_buffer)
        {
            free(c->moov_buffer);
            c->moov_buffer = NULL;
        }
        if(c->trex_data)
        {
            free(c->trex_data);
            c->trex_data = NULL;
        }
        if (c->senc_data)
        {
            free(c->senc_data);
            c->senc_data = NULL;
        }
        if (c->idx_buf)
        {
            free(c->idx_buf);
            c->idx_buf = NULL;
        }
        if(c->Vsamples)
        {
            //Sample* sample = aw_list_last(c->samples);
            unsigned int i;
            Sample* samp;
            for(i=0; i<aw_list_count(c->Vsamples); i++)
            {
                samp = aw_list_get(c->Vsamples, i);
                if(samp)
                {
                    free(samp);
                    samp = NULL;
                }
            }
            aw_list_del(c->Vsamples);
            c->Vsamples = NULL;
        }
        if(c->Asamples)
        {
            //Sample* sample = aw_list_last(c->samples);
            unsigned int i;
            Sample* samp;
            for(i=0; i<aw_list_count(c->Asamples); i++)
            {
                samp = aw_list_get(c->Asamples, i);
                if(samp)
                {
                    free(samp);
                    samp = NULL;
                }
            }
            aw_list_del(c->Asamples);
            c->Asamples = NULL;
        }

        for(i=0; i<c->nb_streams; i++)
        {
            if (c->streams[i])
            {
                if(c->streams[i]->codec.extradata)
                {
                    free(c->streams[i]->codec.extradata);
                    c->streams[i]->codec.extradata = 0;
                }
                if(c->streams[i]->ctts_data)
                {
                    free(c->streams[i]->ctts_data);
                    c->streams[i]->ctts_data = NULL;
                }
                if(c->streams[i]->rap_seek)
                {
                    free(c->streams[i]->rap_seek);
                    c->streams[i]->rap_seek = NULL;
                }
                free(c->streams[i]);
                c->streams[i] = 0;
            }
        }

        if(c->pAvccHdrInfo)
        {
            free(c->pAvccHdrInfo);
            c->pAvccHdrInfo = 0;
        }

        CDX_LOGD("mov close stream = %p", c->fp);
        if(c->fp)
        {
            CdxStreamClose(c->fp);
            c->fp = NULL;
        }
    }

    return 0;
}

CDX_S16 CdxMovExit(struct CdxMovParser *p)
{
    if(!p)
    {
        return -1;
    }

    if(p->vc)
    {
        free(p->vc);
        p->vc = NULL;
    }

    if (p->privData)
    {
        free(p->privData);
        p->privData = NULL;
    }

    free(p);

    return 0;
}

/**************************************************
    MOV_reader_get_data_block()
    Functionality : Get the next data block
    Return value  :
        1   read finish
         0  allright
        -1  read error
 **************************************************/
CDX_S16 CdxMovRead(struct CdxMovParser *p)
{
    MOVContext      *c;
    CDX_S16           ret = 0;

    if(!p)
    {
        CDX_LOGW("mov reader handle is invalid!\n");
        return -1;
    }
    c = (MOVContext*)p->privData;

    if(c->is_fragment)
    {
//        CDX_LOGD("xxx MovReadSampleFragment");
        ret = MovReadSampleFragment(p);
    }
    else
    {
        ret = MovReadSample(p);
    }

    p->nFRPicCnt = 0;
    return ret;
}

int CdxMovSeek(struct CdxMovParser *p, cdx_int64  timeUs)
{
    MOVContext      *c = (MOVContext*)p->privData;
    CdxStreamT    *pb = c->fp;
    MOVStreamContext       *st = NULL;
    CDX_S32           frmidx = 0;
    CDX_S32           result;

    int                seekTime;
    seekTime = timeUs / 1000;
    CDX_LOGD("=============mov seek to: %d ms, totaltime = %d ms\n",seekTime, p->totalTime);

    if(seekTime < 0)
    {
        CDX_LOGW("The parameter for jump play is invalid!\n");
        return -1;
    }

    if(!c->bSeekAble)
    {
        CDX_LOGD("-- can not seekable");
        return -1;
    }

    int i;
    for(i=0; i<c->has_subtitle; i++)
    {
        st = c->streams[c->s2st[i]];
        st->SubStreamSyncFlg = 1;
    }

    //in DASH, the totaltime maybe 0, but it seekable  ( BigBuckBunny audio segment )
    if(c->is_fragment)
    {
        // need to clean up Vsamples and Asamples
        while(aw_list_count(c->Vsamples))
        {
            void* item = aw_list_last(c->Vsamples);
            aw_list_rem_last(c->Vsamples);
            if(item)
            {
                free(item);
                item = NULL;
            }
        }
        while(aw_list_count(c->Asamples))
        {
            void* item = aw_list_last(c->Asamples);
            aw_list_rem_last(c->Asamples);
            if(item)
            {
                free(item);
                item = NULL;
            }
        }

        //sidx mode for single url in DASH
        if(!p->bDashSegment && !p->bSmsSegment) //* for sms
        {

            if(c->has_video)
            {
                result = MovTimeToSampleSidx(p, timeUs/1000, c->video_stream_idx, 0);
                if(result < 0)
                {
                    return -1;
                }
            }
            if(c->has_audio)
            {
                result = MovTimeToSampleSidx(p, timeUs/1000, c->audio_stream_idx, 1);
                if(result < 0)
                {
                    return -1;
                }
            }

            c->moof_end_offset = CdxStreamTell(pb); // we must reset it when jump
        }
        else  //mutil url in DASH
        {
            result = MovReadSampleFragment(p);
            if(c->has_video)
            {
                result = MovTimeToSampleSidx(p, timeUs/1000, c->video_stream_idx, 0);
                if(result < 0)
                {
                    return -1;
                }
            }
            if(c->has_audio)
            {
                result = MovTimeToSampleSidx(p, timeUs/1000, c->audio_stream_idx, 1);
                if(result < 0)
                {
                    return -1;
                }
            }
        }
    }
    else
    {
        if(seekTime >= (int)p->totalTime)
        {
            p->mErrno = PSR_EOS;
            return 1;
        }
        if (c->has_video)
        {
            //find the video sample nearest seekTime
            frmidx = MovTimeToSample(p, seekTime);
            if(frmidx < 0)
            {
                CDX_LOGW("TimeToSample failed!\n");
                return -1;
            }
            //CDX_LOGV("---- seek_time = %lld", timeUs);
            CDX_LOGD("############## sfrmidx = [%d]", frmidx);
            frmidx = MovFindKeyframe(p, 0, (unsigned int)frmidx);
            CDX_LOGD("--- key frame = %d", frmidx);
            if(frmidx < 0)
            {
                CDX_LOGW("look for key frame failed!\n");
                return -1;
            }

            // adjust ctts index
            MOVStreamContext* sc = c->streams[c->video_stream_idx];
            int time_sample;
            int i;
            if(sc->ctts_data)
            {
                time_sample = 0;
                int next = 0;
                for(i=0; i<sc->ctts_size; i++)
                {
                    next = time_sample + sc->ctts_data[i].count;
                    if(next > frmidx)
                    {
                        sc->ctts_index = i;
                        sc->ctts_sample = frmidx-time_sample;
                        break;
                    }
                    time_sample = next;
                }
            }
        }
        result = MovSeekSample(p, frmidx, seekTime);
        if(result < 0)
        {
            CDX_LOGW("Seek sample failed!\n");
            return -1;
        }

        if(CdxStreamIsNetStream(pb))
        {
            CDX_U64           vas_chunk_ost[MOV_MAX_STREAMS];
            CDX_S32           va_sel = 1;

            MOVStreamContext *st = NULL;

            int i;
            for(i=0; i<c->nb_streams; i++)
            {
                vas_chunk_ost[i] = (CDX_U64)-1LL;
            }

            for(i=0; i<c->nb_streams; i++)
            {
                st = c->streams[i];
                if((st->stsd_type && !st->read_va_end))
                {
                    // we only need one video stream
                    if(st->stsd_type == 1 && st->stream_index != 0)
                    {
                        st->read_va_end = 1;
                        continue;
                    }
                    vas_chunk_ost[i] = ReadStco(c, st, st->mov_idx_curr.stco_idx);
                }
            }

            // read sample from file offset in network stream,
            // from dts ( not pts ) in local file
            va_sel = GetMin(vas_chunk_ost, c->nb_streams);

            if(vas_chunk_ost[va_sel] != (CDX_U64)-1LL)
            {
                //CDX_LOGD("--- stream seek: %llx", vas_chunk_ost[va_sel]);
                result = CdxStreamSeek(pb, vas_chunk_ost[va_sel], SEEK_SET);
                if(result < 0)
                {
                    return -1;
                }
            }
        }
    }

    for(i=0; i<c->nb_streams; i++)
    {
        if(c->streams[i]->stsd_type && !c->streams[i]->unsurpoort)
        {
            c->streams[i]->read_va_end = 0; // reset the eof flag
        }
    }

    CDX_LOGD("---- seek end");

    return 0;
}

struct CdxMovParser* CdxMovInit(CDX_S32 *ret)
{
    MOVContext          *s;
    struct CdxMovParser  *p;

    *ret = 0;
    p = (struct CdxMovParser *)malloc(sizeof(struct CdxMovParser));
    if(!p)
    {
        *ret = -1;
        return NULL;
    }
    memset(p, 0, sizeof(struct CdxMovParser));
    p->mErrno = PSR_INVALID;

    p->privData = NULL;
    s = (MOVContext *)malloc(sizeof(MOVContext));
    if(!s)
    {
        *ret = -1;
        return p;
    }
    memset(s, 0, sizeof(MOVContext));

    p->privData = (void *)s;

    s->ptr_fpsr = p;
    s->Vsamples = aw_list_new();
    s->Asamples = aw_list_new();

    p->vc = (VirCacheContext *)malloc(sizeof(VirCacheContext));
    if (p->vc == NULL)
    {
        *ret = -1;
        return p;
    }
    memset(p->vc, 0, sizeof(VirCacheContext));

    return p;
}

//***************************************************************************/
//* initial in getmediaInfo, set the video, audio stream index
//***************************************************************************/
CDX_S32 CdxMovSetStream(struct CdxMovParser *p)
{
    MOVContext    *c;
    VirCacheContext *vc;
    //CDX_U32 fpos;
    MOVStreamContext *st;

    c = (MOVContext*)p->privData;

    vc = p->vc;

    c->streams[c->video_stream_idx]->stss_size += c->streams[c->video_stream_idx]->rap_seek_count;
    logd("== stss_size: %d", c->streams[c->video_stream_idx]->stss_size);
    // number of entries
    c->stbl_info.va_stsc_size[0] = c->streams[c->video_stream_idx]->stsc_size;
    c->stbl_info.va_stsz_size[0] = c->streams[c->video_stream_idx]->stsz_size;
    c->stbl_info.va_stco_size[0] = c->streams[c->video_stream_idx]->stco_size;
    c->stbl_info.va_stss_size    = c->streams[c->video_stream_idx]->stss_size;
    c->stbl_info.va_stts_size[0] = c->streams[c->video_stream_idx]->stts_size;

    //if sample_size = 0, sample is not the same size; else, every sample size is sample_size
    c->stbl_info.va_stsz_sample_size[0] = c->streams[c->video_stream_idx]->sample_size;

    c->stbl_info.va_stsc_size[1] = c->streams[c->audio_stream_idx]->stsc_size;
    c->stbl_info.va_stsz_size[1] = c->streams[c->audio_stream_idx]->stsz_size;
    c->stbl_info.va_stco_size[1] = c->streams[c->audio_stream_idx]->stco_size;
    c->stbl_info.va_stts_size[1] = c->streams[c->audio_stream_idx]->stts_size;

    c->stbl_info.va_stsz_sample_size[1] = c->streams[c->audio_stream_idx]->sample_size;

    c->stbl_info.va_stsc_size[2] = c->streams[c->subtitle_stream_idx]->stsc_size;
    c->stbl_info.va_stsz_size[2] = c->streams[c->subtitle_stream_idx]->stsz_size;
    c->stbl_info.va_stco_size[2] = c->streams[c->subtitle_stream_idx]->stco_size;
    c->stbl_info.va_stts_size[2] = c->streams[c->subtitle_stream_idx]->stts_size;

    c->basetime[0] = c->streams[c->video_stream_idx]->basetime;
    c->basetime[1] = c->streams[c->audio_stream_idx]->basetime;
    c->basetime[2] = c->streams[c->subtitle_stream_idx]->basetime;

    c->stbl_info.va_stsz_sample_size[2] = c->streams[c->subtitle_stream_idx]->sample_size;

    if(c->has_video)
    {
        st = c->streams[c->video_stream_idx];
        p->keyItl = 1;
        c->totaltime = st->totaltime;
        //if  chunk have the same number of samples
        if(st->stsc_size == 1)
        { //most of streams goes to here
            c->samples_per_chunk = ReadStsc(c, st, 4);
        }

        if(st->stts_size > 1 || st->stsc_size>1)
        {
            if(st->stss_size > 2*1024*1024)
            {
                st->stss_size = 0;  //not support ff or rev, //may be file error
            }
            else
            {
                CDX_S32 tbl_size,tbl_itl;

                //if the stss entry is too large,  we do not need so much key frame,
                //so get a key frame every tbl_itl key frames.
                if(st->stss_size > 16*1024)
                {
                    tbl_itl = (st->stss_size>>14) + 1;//
                    tbl_size = st->stss_size/tbl_itl + 1;
                }
                else
                {
                    tbl_itl  = 1;
                    tbl_size = st->stss_size;
                }
                c->idx_buf = (IndexBuffer *)malloc(tbl_size*sizeof(IndexBuffer));
                if (c->idx_buf == NULL)
                {
                    return -1;
                }
                MovBuildKeyframeIdx(p,tbl_itl);
                p->keyItl = tbl_itl;
            }
        }

    }

    if(c->has_audio && c->totaltime > c->streams[c->audio_stream_idx]->totaltime)
        c->totaltime = c->streams[c->audio_stream_idx]->totaltime;
    //p->total_time = c->totaltime * 1000;
    p->totalTime = c->totaltime;   //fuqiang

    if(c->mvhd_total_time > p->totalTime)
    {
        p->totalTime = c->mvhd_total_time;
    }
    if(c->sidx_total_time > p->totalTime)
    {
        p->totalTime = c->sidx_total_time;
    }
    CDX_LOGD("mvhd = %d, ", c->mvhd_total_time);

    //if(c->streams[c->video_stream_idx].stss_size < 2)
    if(c->stbl_info.va_stss_size<2)
    {
        //not support ff or rev
        p->hasIdx = 0;
    }
    else
    {
        p->hasIdx = 1;
    }

    if(c->sidx_flag)
    {
        p->hasIdx = 1;
    }

    return 0;
}
