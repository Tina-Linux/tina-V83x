/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awmetadataretriever.cpp
 * Description : metadataretriever
 * History :
 *
 */

#define LOG_TAG "awmetadataretriever"
#include "cdx_log.h"

#include "awmetadataretriever.h"
#include "memoryAdapter.h"
//#include <AwPluginManager.h>
#include <stdio.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <private/media/VideoFrame.h>

#if (CONF_ANDROID_MAJOR_VER >= 5)
#include "media/CharacterEncodingDetector.h"
#endif
#if (CONF_ANDROID_MAJOR_VER >= 6)
#include "awMediaDataSource.h"
#endif
#include "unicode/ucnv.h"
#include "unicode/ustring.h"
#include <cutils/properties.h>

//#include "vdecoder.h"           //* video decode library in "LIBRARY/CODEC/VIDEO/DECODER"
//#include "CdxParser.h"          //* parser library in "LIBRARY/DEMUX/PARSER/include/"
//#include "CdxStream.h"          //* parser library in "LIBRARY/DEMUX/STREAM/include/"
#include <iniparserapi.h>
#include "xmetadataretriever.h"

#define MediaScanDedug (0)

struct Map {
    int from;
    int to;
    const char *name;
};

static const Map kMap[] = {
    { kKeyMIMEType, METADATA_KEY_MIMETYPE, NULL },
    { kKeyCDTrackNumber, METADATA_KEY_CD_TRACK_NUMBER, "tracknumber" },
    { kKeyDiscNumber, METADATA_KEY_DISC_NUMBER, "discnumber" },
    { kKeyAlbum, METADATA_KEY_ALBUM, "album" },
    { kKeyArtist, METADATA_KEY_ARTIST, "artist" },
    { kKeyAlbumArtist, METADATA_KEY_ALBUMARTIST, "albumartist" },
    { kKeyAuthor, METADATA_KEY_AUTHOR, NULL },
    { kKeyComposer, METADATA_KEY_COMPOSER, "composer" },
    { kKeyDate, METADATA_KEY_DATE, NULL },
    { kKeyGenre, METADATA_KEY_GENRE, "genre" },
    { kKeyTitle, METADATA_KEY_TITLE, "title" },
    { kKeyYear, METADATA_KEY_YEAR, "year" },
    { kKeyWriter, METADATA_KEY_WRITER, "writer" },
    { kKeyCompilation, METADATA_KEY_COMPILATION, "compilation" },
    { kKeyLocation, METADATA_KEY_LOCATION, NULL },
};

// MIMETYPE define in libstagefright/Mediadefs.cpp
const char *MEDIA_MIMETYPE_VIDEO_VP8 = "video/x-vnd.on2.vp8";
const char *MEDIA_MIMETYPE_VIDEO_VP9 = "video/x-vnd.on2.vp9";
const char *MEDIA_MIMETYPE_VIDEO_AVC = "video/avc";
const char *MEDIA_MIMETYPE_VIDEO_HEVC = "video/hevc";
const char *MEDIA_MIMETYPE_VIDEO_MPEG4 = "video/mp4v-es";
const char *MEDIA_MIMETYPE_VIDEO_H263 = "video/3gpp";
const char *MEDIA_MIMETYPE_VIDEO_MPEG2 = "video/mpeg2";
const char *MEDIA_MIMETYPE_VIDEO_RAW = "video/raw";
const char *MEDIA_MIMETYPE_VIDEO_DOLBY_VISION = "video/dolby-vision";
const char *MEDIA_MIMETYPE_VIDEO_WMV1 = "video/wmv1";
const char *MEDIA_MIMETYPE_VIDEO_WMV2 = "video/wmv2";
const char *MEDIA_MIMETYPE_VIDEO_VC1 = "video/wvc1";
//const char *MEDIA_MIMETYPE_VIDEO_VC1 = "video/x-ms-wmv";
//const char *MEDIA_MIMETYPE_VIDEO_VP6 = "video/x-vp6";
const char *MEDIA_MIMETYPE_VIDEO_VP6 = "video/x-vnd.on2.vp6";
const char *MEDIA_MIMETYPE_VIDEO_S263 = "video/flv1";
const char *MEDIA_MIMETYPE_VIDEO_MJPEG = "video/jpeg";
const char *MEDIA_MIMETYPE_VIDEO_MPEG1 = "video/mpeg1";
const char *MEDIA_MIMETYPE_VIDEO_MSMPEG4V1 = "video/x-ms-mpeg4v1";
const char *MEDIA_MIMETYPE_VIDEO_MSMPEG4V2 = "video/x-ms-mpeg4v2";
const char *MEDIA_MIMETYPE_VIDEO_DIVX = "video/divx";
const char *MEDIA_MIMETYPE_VIDEO_XVID = "video/xvid";
const char *MEDIA_MIMETYPE_VIDEO_RXG2 = "video/rvg2";
const char *MEDIA_MIMETYPE_VIDEO_VPX = "video/x-vnd.on2.vp8";
const char *MEDIA_MIMETYPE_AUDIO_AMR_NB = "audio/3gpp";
const char *MEDIA_MIMETYPE_AUDIO_AMR_WB = "audio/amr-wb";
const char *MEDIA_MIMETYPE_AUDIO_MPEG = "audio/mpeg";
const char *MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I = "audio/mpeg-L1";
const char *MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II = "audio/mpeg-L2";
const char *MEDIA_MIMETYPE_AUDIO_MIDI = "audio/midi";
const char *MEDIA_MIMETYPE_AUDIO_AAC = "audio/mp4a-latm";
const char *MEDIA_MIMETYPE_AUDIO_QCELP = "audio/qcelp";
const char *MEDIA_MIMETYPE_AUDIO_VORBIS = "audio/vorbis";
const char *MEDIA_MIMETYPE_AUDIO_OPUS = "audio/opus";
const char *MEDIA_MIMETYPE_AUDIO_G711_ALAW = "audio/g711-alaw";
const char *MEDIA_MIMETYPE_AUDIO_G711_MLAW = "audio/g711-mlaw";
const char *MEDIA_MIMETYPE_AUDIO_RAW = "audio/raw";
const char *MEDIA_MIMETYPE_AUDIO_FLAC = "audio/flac";
const char *MEDIA_MIMETYPE_AUDIO_AAC_ADTS = "audio/aac-adts";
const char *MEDIA_MIMETYPE_AUDIO_MSGSM = "audio/gsm";
const char *MEDIA_MIMETYPE_AUDIO_AC3 = "audio/ac3";
const char *MEDIA_MIMETYPE_AUDIO_EAC3 = "audio/eac3";

const char *MEDIA_MIMETYPE_CONTAINER_MPEG4 = "video/mp4";
const char *MEDIA_MIMETYPE_CONTAINER_WAV = "audio/x-wav";
const char *MEDIA_MIMETYPE_CONTAINER_OGG = "application/ogg";
const char *MEDIA_MIMETYPE_CONTAINER_MATROSKA = "video/x-matroska";
const char *MEDIA_MIMETYPE_CONTAINER_MPEG2TS = "video/mp2ts";
const char *MEDIA_MIMETYPE_CONTAINER_AVI = "video/avi";
const char *MEDIA_MIMETYPE_CONTAINER_MPEG2PS = "video/mp2p";

const char *MEDIA_MIMETYPE_CONTAINER_WVM = "video/wvm";

const char *MEDIA_MIMETYPE_TEXT_3GPP = "text/3gpp-tt";
const char *MEDIA_MIMETYPE_TEXT_SUBRIP = "application/x-subrip";
const char *MEDIA_MIMETYPE_TEXT_VTT = "text/vtt";
const char *MEDIA_MIMETYPE_TEXT_CEA_608 = "text/cea-608";
const char *MEDIA_MIMETYPE_TEXT_CEA_708 = "text/cea-708";
const char *MEDIA_MIMETYPE_DATA_TIMED_ID3 = "application/x-id3v4";

static cdx_int32 kNumMapEntries = sizeof(kMap) / sizeof(kMap[0]);

#if SAVE_BITSTREAM
const char* bitstreamPath = "/data/camera/out.h264";
static FILE* fph264 = NULL;
#endif

/* process 4096 packets to get a frame at maximum. */
#define MAX_PACKET_COUNT_TO_GET_A_FRAME 4096

/* use 5 seconds to get a frame at maximum. */
#define MAX_TIME_TO_GET_A_FRAME 5000000

/* use 10 seconds to get a stream at maximum. */
#define MAX_TIME_TO_GET_A_STREAM 10000000
#define MAX_OUTPUT_STREAM_SIZE (1024*1024)

typedef struct MetadataPriData{
        CdxDataSourceT              mSource;
        CdxMediaInfoT               mMediaInfo;
        int                         mCancelPrepareFlag;
        MediaAlbumArt*              mAlbumArtPic;
        KeyedVector<int, String8>*  mMetaData;
#if MediaScanDedug
        int                         mFd;
#endif
        XRetriever                 *mXRetriever;
        CdxKeyedVectorT            *mHeader;

#if (CONF_ANDROID_MAJOR_VER >= 6)
        sp<DataSource>              mDataSource;
        CdxStreamT*                 mStream;
#endif
}MetadataPriData;

static int parseHeaders(CdxKeyedVectorT **header, char *uri,
                            KeyedVector<String8, String8>* pHeaders);
static void clearHeaders(CdxKeyedVectorT **header);

static int64_t GetSysTime();

static int metafindCStringByKey(CdxMediaInfoT* mediaInfo, int key, cdx_uint8** value);

AwMetadataRetriever::AwMetadataRetriever()
{
    logd("AwMetadataRetriever Create.");
    //AwPluginInit();
//TODO: add plugin in config proc
    mPriData = (MetadataPriData*)malloc(sizeof(MetadataPriData));
    memset(mPriData,0x00,sizeof(MetadataPriData));
    mPriData->mCancelPrepareFlag  = 0;
    mPriData->mAlbumArtPic        = NULL;
#if (CONF_ANDROID_MAJOR_VER >= 6)
    mPriData->mDataSource = NULL;
    mPriData->mStream = NULL;
#endif

#if MediaScanDedug
    mPriData->mFd                    = 0;
#endif
    memset(&mPriData->mSource, 0, sizeof(CdxDataSourceT));

    mPriData->mMetaData = new KeyedVector<int, String8>();

    mPriData->mXRetriever = XRetrieverCreate();
    if (mPriData->mXRetriever == NULL)
    {
        loge("XRetrieverCreate failed.");
        if (mPriData->mMetaData)
        {
            delete mPriData->mMetaData;
            mPriData->mMetaData = NULL;
        }
        free(mPriData);
        mPriData = NULL;
    }
}

AwMetadataRetriever::~AwMetadataRetriever()
{
    mPriData->mCancelPrepareFlag = 1;
    clear();

    if (mPriData->mMetaData)
    {
        delete mPriData->mMetaData;
        mPriData->mMetaData = NULL;
    }

    if (mPriData->mXRetriever)
    {
        XRetrieverDestory(mPriData->mXRetriever);
    }

#if (CONF_ANDROID_MAJOR_VER >= 6)
    if((mPriData->mStream != NULL)
        && (mPriData->mDataSource != NULL))
    {
        MediaDataSourceClose(mPriData->mStream);
        mPriData->mStream = NULL;
        mPriData->mDataSource = NULL;
    }
#endif
    if (mPriData)
    {
        free(mPriData);
        mPriData = NULL;
    }
    logi("AwMetadataRetriever destroyed.");
}

void AwMetadataRetriever::clear()
{
    //* set mCancelPrepareFlag to force the CdxParserPrepare() quit.
    //* this can prevend the setDataSource() operation from blocking at a network io.
    //* but the retriever's setDataSource() method is a synchronize operation, so I think
    //* this take no effect, because user can not return from setDataSource() to call this
    //* method if user's thread is blocked at the setDataSource() operation.

    if(mPriData->mAlbumArtPic != NULL)
    {
        delete mPriData->mAlbumArtPic;
        mPriData->mAlbumArtPic = NULL;
    }

    clearHeaders(&mPriData->mHeader);
    memset(&mPriData->mMediaInfo, 0, sizeof(CdxMediaInfoT));
    mPriData->mMetaData->clear();

#if MediaScanDedug
    mPriData->mFd = -1;
#endif

    return;
}

#if (CONF_ANDROID_MAJOR_VER >= 5)
status_t AwMetadataRetriever::setDataSource(const sp<IMediaHTTPService> &httpService,
            const char* pUrl, const KeyedVector<String8, String8>* pHeaders)
#else
status_t AwMetadataRetriever::setDataSource(const char* pUrl,
            const KeyedVector<String8, String8>* pHeaders)
#endif
{
    int ret = 0;

#if (CONF_ANDROID_MAJOR_VER >= 5)
    CEDARX_UNUSE(httpService);
#endif

    logd("set data source, url = %s", pUrl);
    clear();    //* release parser, decoder and other resource for previous file.

    //* 1. parse pHeaders.
    ret = parseHeaders(&mPriData->mHeader, (char*)pUrl, (KeyedVector<String8, String8>*)pHeaders);
    if (ret < 0)
    {
        loge("parseHeaders failed.");
        return NO_MEMORY;
    }

    //* 2. XRetriever set datasource.
    ret = XRetrieverSetDataSource(mPriData->mXRetriever, pUrl, mPriData->mHeader);
    if (ret < 0)
    {
        loge("XRetrieverSetDataSource failed.");
        return UNKNOWN_ERROR;
    }

    XRetrieverGetMetaData(mPriData->mXRetriever, METADATA_MEDIAINFO, &mPriData->mMediaInfo);

    storeMetadata();

    return OK;
}

status_t AwMetadataRetriever::setDataSource(int fd, int64_t nOffset, int64_t nLength)
{
    char str[128];
    int ret = 0;

    clear();    //* release parser, decoder and other resource for previous file.
    logd("set data source, fd = %d, nLength: %lld", fd, nLength);
#if MediaScanDedug
    mPriData->mFd = fd;
#endif

    //* 1. set the datasource object.
    sprintf(str, "fd://%d?offset=%ld&length=%lld", fd, (long int)nOffset, (long long)nLength);

    ret = XRetrieverSetDataSource(mPriData->mXRetriever, str, mPriData->mHeader);
    if (ret < 0)
    {
        loge("XRetrieverSetDataSource failed.");
        return UNKNOWN_ERROR;
    }

    XRetrieverGetMetaData(mPriData->mXRetriever, METADATA_MEDIAINFO, &mPriData->mMediaInfo);

    storeMetadata();

    return OK;
}

#if (CONF_ANDROID_MAJOR_VER >= 6)
#if ((CONF_ANDROID_MAJOR_VER < 8) || \
    ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER == 0)))
status_t AwMetadataRetriever::setDataSource(const sp<DataSource> &source)
#elif ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER > 0))
status_t AwMetadataRetriever::setDataSource(const sp<DataSource>& source, const char *mime)
#endif
{

#if ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER > 0))
    CEDARX_UNUSE(mime);
#endif

    int ret = 0;
    CdxStreamT *stream = MediaDataSourceOpen(source);
    if(stream == NULL)
    {
        loge("awmetadataretriever MediaDataSourceOpen fail!");
        return UNKNOWN_ERROR;
    }
    mPriData->mDataSource = source;
    mPriData->mStream = stream;
    char str[128];
    sprintf(str, "datasource://%p",stream);
    ret = XRetrieverSetDataSource(mPriData->mXRetriever, str, mPriData->mHeader);
    if (ret < 0)
    {
        loge("XRetrieverSetDataSource failed.");
        return UNKNOWN_ERROR;
    }

    XRetrieverGetMetaData(mPriData->mXRetriever, METADATA_MEDIAINFO, &mPriData->mMediaInfo);

    storeMetadata();
    return (status_t)ret;
}
#endif

void AwMetadataRetriever::storeMetadata(void)
{
    int  i;
    char tmp[256];
    int nStrLen = 0;
    String8 mStrData;
    if(mPriData->mMediaInfo.pAlbumArtBuf != NULL &&
            mPriData->mMediaInfo.nAlbumArtBufSize > 0 &&
            mPriData->mAlbumArtPic == NULL)
    {
#if (CONF_ANDROID_MAJOR_VER >= 5)
        mPriData->mAlbumArtPic = MediaAlbumArt::fromData(mPriData->mMediaInfo.nAlbumArtBufSize,
                mPriData->mMediaInfo.pAlbumArtBuf);
#else
        mPriData->mAlbumArtPic = new MediaAlbumArt;
        mPriData->mAlbumArtPic->mSize = mPriData->mMediaInfo.nAlbumArtBufSize;
        mPriData->mAlbumArtPic->mData = new uint8_t[mPriData->mMediaInfo.nAlbumArtBufSize];
        memcpy(mPriData->mAlbumArtPic->mData, mPriData->mMediaInfo.pAlbumArtBuf,
                mPriData->mMediaInfo.nAlbumArtBufSize);
#endif
    }

#if (CONF_ANDROID_MAJOR_VER >= 5)
    CharacterEncodingDetector *detector = new CharacterEncodingDetector();

    for (int i = 0; i < kNumMapEntries; ++i) {
        cdx_uint8 *value;
        if (metafindCStringByKey(&mPriData->mMediaInfo, kMap[i].from, &value)) {
            if (kMap[i].name) {
                // add to charset detector
                detector->addTag(kMap[i].name, (const char*)value);
            } else {
                // directly add to output list
                mPriData->mMetaData->add(kMap[i].to, String8((const char*)value));
            }
        }
    }

    detector->detectAndConvert();
    int size = detector->size();
    logd("detect conventor size : %d", size);
    if (size) {
        for (int i = 0; i < size; i++) {
            const char *name;
            const char *value;
            detector->getTag(i, &name, &value);
            for (int j = 0; j < kNumMapEntries; ++j) {
                if (kMap[j].name && !strcmp(kMap[j].name, name)) {
                    mPriData->mMetaData->add(kMap[j].to, String8(value));
                }
            }
        }
    }
    delete detector;
#else
    for (int i = 0; i < kNumMapEntries; ++i) {
        cdx_uint8 *value;
        if (metafindCStringByKey(&mPriData->mMediaInfo, kMap[i].from, &value)) {
            mPriData->mMetaData->add(kMap[i].to, String8((const char *)value));
        }
    }
#endif
    //* no information to give the title.
    //* mp3 file may contain this information in its metadata, index by tag "TYE" or "TYER";
    //* mov file may contain this information in its metadata, index by FOURCC "0xa9 'd' 'a' 'y'";

    //* /**
    //*  * The metadata key to retrieve the playback duration of the data source.
    //*  */
    //* public static final int METADATA_KEY_DURATION        = 9;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        //* The duration value is a string representing the duration in ms.
        sprintf(tmp, "%d",
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].duration);
        mPriData->mMetaData->add(METADATA_KEY_DURATION, String8(tmp));
        logv("duration:%d",
            mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].duration);
    }

    //* /**
    //*  * The metadata key to retrieve the number of tracks, such as audio, video,
    //*  * text, in the data source, such as a mp4 or 3gpp file.
    //*  */
    //* public static final int METADATA_KEY_NUM_TRACKS      = 10;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        int nTrackCount =
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum +
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].audioNum +
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].subtitleNum;
        sprintf(tmp, "%d", nTrackCount);
        mPriData->mMetaData->add(METADATA_KEY_NUM_TRACKS, String8(tmp));
    }

    //* /**
    //*  * The metadata key to retrieve the information of the writer (such as
    //*  * lyricist) of the data source.
    //*  */
    //* public static final int METADATA_KEY_WRITER          = 11;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_WRITER, String8("unknown writer"));
    //* no information to give the writer.
    //* ogg file may contain this information in its vorbis comment, index by tag "LYRICIST";
    //* mov file may contain this information in its metadata, index by FOURCC "0xa9 'w' 'r' 't'";
#endif

    //* /**
    //*  * The metadata key to retrieve the mime type of the data source. Some
    //*  * example mime types include: "video/mp4", "audio/mp4", "audio/amr-wb",
    //*  * etc.
    //*  */
    //* public static final int METADATA_KEY_MIMETYPE        = 12;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        int type = -1;
        XRetrieverGetMetaData(mPriData->mXRetriever, METADATA_PARSER_TYPE, &type);
        switch(type)
        {
            case CDX_PARSER_MOV:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mp4"));
                break;
            case CDX_PARSER_MKV:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-matroska"));
                break;
            case CDX_PARSER_ASF:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-ms-asf"));
                break;
            case CDX_PARSER_MPG:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mpeg"));
                break;
            case CDX_PARSER_TS:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mp2ts"));
                break;
            case CDX_PARSER_AVI:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/avi"));
                break;
            case CDX_PARSER_FLV:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_PMP:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_HLS:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                        String8("application/vnd.apple.mpegurl"));
                break;
            case CDX_PARSER_DASH:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_MMS:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_BD:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_OGG:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("application/ogg"));
                break;
            case CDX_PARSER_M3U9:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_RMVB:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_PLAYLIST:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            case CDX_PARSER_WAV:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("audio/x-wav"));
                break;
            case CDX_PARSER_APE:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("audio/cedara"));
                break;
            case CDX_PARSER_MP3:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("audio/mpeg"));
                break;
            case CDX_PARSER_WVM:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                        String8("application/x-android-drm-fl"));
                break;
            case CDX_PARSER_REMUX:
                mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/cedarx"));
                break;
            default:
                break;
            //* going to be add:
            //* WAV: audio/x-wav
            //* APE:
            //* ...
            //* Widevine: video/wvm
            //* ...
        }

        #if 0
        //* add video mimetype.
        for(i=0; i<mPriData->mMediaInfo.program[mMediaInfo.programIndex].videoNum; i++)
        {
            switch(mPriData->mMediaInfo.program[mMediaInfo.programIndex].video[i].eCodecFormat)
            {
                case VIDEO_CODEC_FORMAT_MJPEG:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/jpeg"));
                    break;
                case VIDEO_CODEC_FORMAT_MPEG1:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mpeg1"));
                    break;
                case VIDEO_CODEC_FORMAT_MPEG2:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mpeg2"));
                    break;
                case VIDEO_CODEC_FORMAT_MPEG4:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/mp4v-es"));
                    break;
                case VIDEO_CODEC_FORMAT_MSMPEG4V1:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-ms-mpeg4v1"));
                    break;
                case VIDEO_CODEC_FORMAT_MSMPEG4V2:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-ms-mpeg4v2"));
                    break;
                case VIDEO_CODEC_FORMAT_DIVX3:
                case VIDEO_CODEC_FORMAT_DIVX4:
                case VIDEO_CODEC_FORMAT_DIVX5:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/divx"));
                    break;
                case VIDEO_CODEC_FORMAT_XVID:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/xvid"));
                    break;
                case VIDEO_CODEC_FORMAT_H263:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/3gpp"));
                    break;
                case VIDEO_CODEC_FORMAT_SORENSSON_H263:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/flv1"));
                    break;
                case VIDEO_CODEC_FORMAT_RXG2:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/rvg2"));
                    break;
                case VIDEO_CODEC_FORMAT_WMV1:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/wmv1"));
                    break;
                case VIDEO_CODEC_FORMAT_WMV2:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/wmv2"));
                    break;
                case VIDEO_CODEC_FORMAT_WMV3:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-ms-wmv"));
                    break;
                case VIDEO_CODEC_FORMAT_VP6:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-vnd.on2.vp6"));
                    break;
                case VIDEO_CODEC_FORMAT_VP8:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-vnd.on2.vp8"));
                    break;
                case VIDEO_CODEC_FORMAT_VP9:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/x-vnd.on2.vp9"));
                    break;
                case VIDEO_CODEC_FORMAT_RX:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/realvideo"));
                    break;
                case VIDEO_CODEC_FORMAT_H264:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/avc"));
                    break;
                case VIDEO_CODEC_FORMAT_H265:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/hevc"));
                    break;
                case VIDEO_CODEC_FORMAT_AVS:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("video/avs"));
                    break;
                default:
                    break;
            }
        }
        #endif

        #if 1
        if(mPriData->mMediaInfo.programIndex >= 0 &&
                mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
        {
            if((mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].audioNum > 0) &&
                (mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum <= 0))
            {
                //* add audio mimetype.
                int programIndex = mPriData->mMediaInfo.programIndex;
                int audioNum = mPriData->mMediaInfo.program[programIndex].audioNum;
                for(i = 0; i < audioNum; i++)
                {
                    enum EAUDIOCODECFORMAT eCodecFormat =
                            mPriData->mMediaInfo.program[programIndex].audio[i].eCodecFormat;
                    switch(eCodecFormat)
                    {
                        case AUDIO_CODEC_FORMAT_MP1:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I));
                            break;
                        case AUDIO_CODEC_FORMAT_MP2:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II));
                            break;
                        case AUDIO_CODEC_FORMAT_MP3:
                        case AUDIO_CODEC_FORMAT_MP3_PRO:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_MPEG));
                            break;
                        case AUDIO_CODEC_FORMAT_MPEG_AAC_LC:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_AAC_ADTS));
                            break;
                        case AUDIO_CODEC_FORMAT_AC3:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_AC3));
                            break;
                        case AUDIO_CODEC_FORMAT_DTS:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/cedara"));
                            break;
                        case AUDIO_CODEC_FORMAT_LPCM_V:
                        case AUDIO_CODEC_FORMAT_LPCM_A:
                        case AUDIO_CODEC_FORMAT_ADPCM:
                        case AUDIO_CODEC_FORMAT_PPCM:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_RAW));
                            break;
                        case AUDIO_CODEC_FORMAT_PCM:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/cedara"));
                            break;
                        case AUDIO_CODEC_FORMAT_WMA_STANDARD:
                        case AUDIO_CODEC_FORMAT_WMA_LOSS:
                        case AUDIO_CODEC_FORMAT_WMA_PRO:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/x-ms-wma"));
                            break;
                        case AUDIO_CODEC_FORMAT_FLAC:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_FLAC));
                            break;
                        case AUDIO_CODEC_FORMAT_APE:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/cedara"));
                            break;
                        case AUDIO_CODEC_FORMAT_OGG:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_VORBIS));
                            break;
                        case AUDIO_CODEC_FORMAT_RAAC:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8(MEDIA_MIMETYPE_AUDIO_AAC_ADTS));
                            break;
                        case AUDIO_CODEC_FORMAT_COOK:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/cook"));
                            break;
                        case AUDIO_CODEC_FORMAT_SIPR:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/sipr"));
                            break;
                        case AUDIO_CODEC_FORMAT_ATRC:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/atrc"));
                            break;
                        case AUDIO_CODEC_FORMAT_AMR:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                String8(MEDIA_MIMETYPE_AUDIO_AMR_WB));
                            break;
                        case AUDIO_CODEC_FORMAT_RA:
                            mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                                    String8("audio/cedara"));
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        #endif

        #if 0
        //* add subtitle mimetype.
        for(i=0; i<mMediaInfo.program[mMediaInfo.programIndex].subtitleNum; i++)
        {
            switch(mMediaInfo.program[mMediaInfo.programIndex].subtitle[i].eCodecFormat)
            {
                case SUBTITLE_CODEC_DVDSUB:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("subtitle/x-subrip"));
                    break;
                case SUBTITLE_CODEC_IDXSUB:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                            String8("subtitle/index-subtitle"));
                    break;
                case SUBTITLE_CODEC_PGS:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE,
                            String8("subtitle/bd-subtitle"));
                    break;
                case SUBTITLE_CODEC_TXT:
                case SUBTITLE_CODEC_TIMEDTEXT:
                case SUBTITLE_CODEC_SRT:
                case SUBTITLE_CODEC_SMI:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("text/3gpp-tt"));
                    break;
                case SUBTITLE_CODEC_SSA:
                    mPriData->mMetaData->add(METADATA_KEY_MIMETYPE, String8("text/ssa"));
                    break;
                default:
                    break;
            }
        }
        #endif
    }

    //* /**
    //*  * The metadata key to retrieve the information about the performers or
    //*  * artist associated with the data source.
    //*  */
    //* public static final int METADATA_KEY_ALBUMARTIST     = 13;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_ALBUMARTIST, String8("unknown album artist"));
    //* no information to give the artist.
    //* ogg file may contain this information in its vorbis comment, index by tag "ALBUMARTIST"
    //* or "ALBUM ARTIST";
    //* mp3 file may contain this information in its metadata, index by tag "TPE2" or "TP2";
    //* mov file may contain this information in its metadata, index by FOURCC "'a' 'A' 'R' 'T'";
#endif

    //* /**
    //*  * The metadata key to retrieve the numberic string that describes which
    //*  * part of a set the audio data source comes from.
    //*  */
    //* public static final int METADATA_KEY_DISC_NUMBER     = 14;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_DISC_NUMBER, String8("unknown disk number"));
    //* no information to tell which part of a set the audio data source comes from.
    //* ogg file may contain this information in its vorbis comment, index by tag "DISCNUMBER";
    //* mp3 file may contain this information in its metadata, index by tag "TPA" or "TPOS";
    //* mov file may contain this information in its metadata, index by FOURCC "'d' 'i' 's' 'k'";
#endif

    //* /**
    //*  * The metadata key to retrieve the music album compilation status.
    //*  */
    //* public static final int METADATA_KEY_COMPILATION     = 15;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_COMPILATION, String8("unknown compilation"));
    //* no information to tell which part of a set the audio data source comes from.
    //* ogg file may contain this information in its vorbis comment, index by tag "COMPILATION";
    //* mp3 file may contain this information in its metadata, index by tag "TCP" or "TCMP";
    //* mov file may contain this information in its metadata, index by FOURCC "'c' 'p' 'i' 'l'";
#endif

    //* /**
    //*  * If this key exists the media contains audio content.
    //*  */
    //* public static final int METADATA_KEY_HAS_AUDIO       = 16;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        //* add HAS_AUDIO info when have audio,or not add
        //* (reference to StagefrightMetadataRetriever.cpp)
        if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].audioNum > 0)
            mPriData->mMetaData->add(METADATA_KEY_HAS_AUDIO, String8("yes"));
    }

    //* /**
    //*  * If this key exists the media contains video content.
    //*  */
    //* public static final int METADATA_KEY_HAS_VIDEO       = 17;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        //* add HAS_VIDEO info when hava video, or not add
        //* (reference to StagefrightMetadataRetriever.cpp)
        if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum > 0)
            mPriData->mMetaData->add(METADATA_KEY_HAS_VIDEO, String8("yes"));
    }

    //* /**
    //*  * If the media contains video, this key retrieves its width.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_WIDTH     = 18;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum > 0)
        {
            sprintf(tmp, "%d",
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].video[0].nWidth);
            mPriData->mMetaData->add(METADATA_KEY_VIDEO_WIDTH, String8(tmp));
        }
    }

    //* /**
    //*  * If the media contains video, this key retrieves its height.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_HEIGHT    = 19;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum > 0)
        {
            sprintf(tmp, "%d",
                mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].video[0].nHeight);
            //* add the video height
            mPriData->mMetaData->add(METADATA_KEY_VIDEO_HEIGHT, String8(tmp));
        }
    }

    //* /**
    //*  * This key retrieves the average bitrate (in bits/sec), if available.
    //*  */
    //* public static final int METADATA_KEY_BITRATE         = 20;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_BITRATE, String8("unknown bitrate"));
    //* no information to give the average bitrate.
#endif

    //* /**
    //*  * This key retrieves the language code of text tracks, if available.
    //*  * If multiple text tracks present, the return value will look like:
    //*  * "eng:chi"
    //*  * @hide
    //*  */
    //* public static final int METADATA_KEY_TIMED_TEXT_LANGUAGES      = 21;
    if(mPriData->mMediaInfo.programIndex >= 0 &&
            mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
        if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].subtitleNum > 0)
        {
            int len;
            char buffer[256];
            tmp[0] = '\0';
            buffer[0] = '\0';
            int index = mPriData->mMediaInfo.programIndex;
            for(i=0; i<mPriData->mMediaInfo.program[index].subtitleNum; i++)
            {
                if(i != 0)
                    sprintf(buffer, "%s:", tmp);   //* add a ":".

                len = strlen((const char*)mPriData->mMediaInfo.program[index].subtitle[i].strLang);
                if(len == 0)
                    len = strlen("unknown");

                if(len + strlen(tmp) >= (sizeof(tmp)-1))
                {
                    logw("can not set the language of subtitle correctly");
                    break;  //* for save.
                }

                if(strlen((const char*)mPriData->mMediaInfo.program[index].subtitle[i].strLang) > 0)
                    sprintf(buffer, "%s%s", tmp,
                            mPriData->mMediaInfo.program[index].subtitle[i].strLang);
                else
                    sprintf(buffer, "%s%s", tmp, "unknown");
            }
            mPriData->mMetaData->add(METADATA_KEY_TIMED_TEXT_LANGUAGES, String8(buffer));
        }
    }

    //* /**
    //*  * If this key exists the media is drm-protected.
    //*  * @hide
    //*  */
    //* public static final int METADATA_KEY_IS_DRM          = 22;
#if 0
    mPriData->mMetaData->add(METADATA_KEY_IS_DRM, String8("0"));
    //* no information to give the drm info.
#endif

    //* /**
    //*  * This key retrieves the location information, if available.
    //*  * The location should be specified according to ISO-6709 standard, under
    //*  * a mp4/3gp box "@xyz". Location with longitude of -90 degrees and latitude
    //*  * of 180 degrees will be retrieved as "-90.0000+180.0000", for instance.
    //*  */
    //* public static final int METADATA_KEY_LOCATION        = 23;

    //* set the location info
    //* we should not add it when the nStrlen is 0
    nStrLen = strlen((const char*)mPriData->mMediaInfo.location);
    if(nStrLen != 0)
        mPriData->mMetaData->add(METADATA_KEY_LOCATION,
                String8((const char*)mPriData->mMediaInfo.location));

    //* /**
    //*  * This key retrieves the video rotation angle in degrees, if available.
    //*  * The video rotation angle may be 0, 90, 180, or 270 degrees.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_ROTATION = 24;

    //* set rotation info
    //* mov parser can get the rotate angle.
    //* we should not add it when the nStrlen is 0
    nStrLen = strlen((const char*)mPriData->mMediaInfo.rotate);
    if(nStrLen != 0)
        mPriData->mMetaData->add(METADATA_KEY_VIDEO_ROTATION,
                String8((const char*)mPriData->mMediaInfo.rotate));

#if (CONF_ANDROID_MAJOR_VER >= 8)
    //* METADATA_KEY_CAPTURE_FRAMERATE = 25
    if(mPriData->mMediaInfo.programIndex >= 0 &&
           mPriData->mMediaInfo.programNum >= mPriData->mMediaInfo.programIndex)
    {
       if(mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].videoNum > 0)
       {
           if(mPriData->mMediaInfo.androidCaptureFps)
           {
               sprintf(tmp, "%f",mPriData->mMediaInfo.androidCaptureFps);
               //* add the frame rate
               mPriData->mMetaData->add(METADATA_KEY_CAPTURE_FRAMERATE, String8(tmp));
           }
           else
           {
               sprintf(tmp, "%d",
                   mPriData->mMediaInfo.program[mPriData->mMediaInfo.programIndex].video[0].nFrameRate/1000);
               //* add the frame rate
               mPriData->mMetaData->add(METADATA_KEY_CAPTURE_FRAMERATE, String8(tmp));
           }
       }
    }
#endif

    return;
}

const char* AwMetadataRetriever::extractMetadata(int keyCode)
{
    //* keyCode is defined in "android/framworks/av/include/media/mediametadataretriever.h" and
    //* "android/frameworks/base/media/java/android/media/mediametadataretriever.java".
    //* keyCodes list here.
    //* /**
    //*  * The metadata key to retrieve the numeric string describing the
    //*  * order of the audio data source on its original recording.
    //*  */
    //* public static final int METADATA_KEY_CD_TRACK_NUMBER = 0;
    //* /**
    //*  * The metadata key to retrieve the information about the album title
    //*  * of the data source.
    //*  */
    //* public static final int METADATA_KEY_ALBUM           = 1;
    //* /**
    //*  * The metadata key to retrieve the information about the artist of
    //*  * the data source.
    //*  */
    //* public static final int METADATA_KEY_ARTIST          = 2;
    //* /**
    //*  * The metadata key to retrieve the information about the author of
    //*  * the data source.
    //*  */
    //* public static final int METADATA_KEY_AUTHOR          = 3;
    //* /**
    //*  * The metadata key to retrieve the information about the composer of
    //*  * the data source.
    //*  */
    //* public static final int METADATA_KEY_COMPOSER        = 4;
    //* /**
    //*  * The metadata key to retrieve the date when the data source was created
    //*  * or modified.
    //*  */
    //* public static final int METADATA_KEY_DATE            = 5;
    //* /**
    //*  * The metadata key to retrieve the content type or genre of the data
    //*  * source.
    //*  */
    //* public static final int METADATA_KEY_GENRE           = 6;
    //* /**
    //*  * The metadata key to retrieve the data source title.
    //*  */
    //* public static final int METADATA_KEY_TITLE           = 7;
    //* /**
    //*  * The metadata key to retrieve the year when the data source was created
    //*  * or modified.
    //*  */
    //* public static final int METADATA_KEY_YEAR            = 8;
    //* /**
    //*  * The metadata key to retrieve the playback duration of the data source.
    //*  */
    //* public static final int METADATA_KEY_DURATION        = 9;
    //* /**
    //*  * The metadata key to retrieve the number of tracks, such as audio, video,
    //*  * text, in the data source, such as a mp4 or 3gpp file.
    //*  */
    //* public static final int METADATA_KEY_NUM_TRACKS      = 10;
    //* /**
    //*  * The metadata key to retrieve the information of the writer (such as
    //*  * lyricist) of the data source.
    //*  */
    //* public static final int METADATA_KEY_WRITER          = 11;
    //* /**
    //*  * The metadata key to retrieve the mime type of the data source. Some
    //*  * example mime types include: "video/mp4", "audio/mp4", "audio/amr-wb",
    //*  * etc.
    //*  */
    //* public static final int METADATA_KEY_MIMETYPE        = 12;
    //* /**
    //*  * The metadata key to retrieve the information about the performers or
    //*  * artist associated with the data source.
    //*  */
    //* public static final int METADATA_KEY_ALBUMARTIST     = 13;
    //* /**
    //*  * The metadata key to retrieve the numberic string that describes which
    //*  * part of a set the audio data source comes from.
    //*  */
    //* public static final int METADATA_KEY_DISC_NUMBER     = 14;
    //* /**
    //*  * The metadata key to retrieve the music album compilation status.
    //*  */
    //* public static final int METADATA_KEY_COMPILATION     = 15;
    //* /**
    //*  * If this key exists the media contains audio content.
    //*  */
    //* public static final int METADATA_KEY_HAS_AUDIO       = 16;
    //* /**
    //*  * If this key exists the media contains video content.
    //*  */
    //* public static final int METADATA_KEY_HAS_VIDEO       = 17;
    //* /**
    //*  * If the media contains video, this key retrieves its width.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_WIDTH     = 18;
    //* /**
    //*  * If the media contains video, this key retrieves its height.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_HEIGHT    = 19;
    //* /**
    //*  * This key retrieves the average bitrate (in bits/sec), if available.
    //*  */
    //* public static final int METADATA_KEY_BITRATE         = 20;
    //* /**
    //*  * This key retrieves the language code of text tracks, if available.
    //*  * If multiple text tracks present, the return value will look like:
    //*  * "eng:chi"
    //*  * @hide
    //*  */
    //* public static final int METADATA_KEY_TIMED_TEXT_LANGUAGES      = 21;
    //* /**
    //*  * If this key exists the media is drm-protected.
    //*  * @hide
    //*  */
    //* public static final int METADATA_KEY_IS_DRM          = 22;
    //* /**
    //*  * This key retrieves the location information, if available.
    //*  * The location should be specified according to ISO-6709 standard, under
    //*  * a mp4/3gp box "@xyz". Location with longitude of -90 degrees and latitude
    //*  * of 180 degrees will be retrieved as "-90.0000+180.0000", for instance.
    //*  */
    //* public static final int METADATA_KEY_LOCATION        = 23;
    //* /**
    //*  * This key retrieves the video rotation angle in degrees, if available.
    //*  * The video rotation angle may be 0, 90, 180, or 270 degrees.
    //*  */
    //* public static final int METADATA_KEY_VIDEO_ROTATION = 24;

    int index;
    const char* strMetadataName[] =
    {
        "CD_TRACK_NUMBER",      "ALBUM",                "ARTIST",           "AUTHOR",
        "CD_TRACK_COMPOSER",    "DATE",                 "GENRE",            "TITLE",
        "YEAR",                 "DURATION",             "NUM_TRACKS",       "WRITER",
        "MIMETYPE",             "ALBUMARTIST",          "DISC_NUMBER",      "COMPILATION",
        "HAS_AUDIO",            "HAS_VIDEO",            "VIDEO_WIDTH",      "VIDEO_HEIGHT",
        "BITRATE",              "TIMED_TEXT_LANGUAGES", "IS_DRM",           "LOCATION",
        "VIDEO_ROTATION"
    };

    index = mPriData->mMetaData->indexOfKey(keyCode);
    if (index < 0)
    {
        if(keyCode >=0 && keyCode <= 24)
        {
            logw("no metadata for %s.", strMetadataName[keyCode]);
        }
        return NULL;
    }

    return mPriData->mMetaData->valueAt(index).string();
}

MediaAlbumArt* AwMetadataRetriever::extractAlbumArt()
{
    //* no album art picture extracted.
    //* ogg parser can find the album art picture in its vorbis comment,
    //* index by tag "METADATA_BLOCK_PICTURE";
    //* flac parser can find the album art picture in its meta data;
    //* mp3 parser can find the album art picture in its id3 data;
    //* mov parser can find the album art picture in its metadata,
    //* index by FOURCC "'c' 'o' 'v' 'r'";

    if(mPriData->mAlbumArtPic != NULL)
    {
#if (CONF_ANDROID_MAJOR_VER >= 5)
        return mPriData->mAlbumArtPic->clone();
#else
        return new MediaAlbumArt(*mPriData->mAlbumArtPic);
#endif
    }
    else
    {
        return NULL;
    }
    //* note:
    //* if it is a media file with drm protection, we should not return an album art picture.
}

sp<IMemory> AwMetadataRetriever::getStreamAtTime(int64_t timeUs)
{
    sp<IMemory>   mem = NULL;
    XVideoStream *pXVideoStream;
    uint8_t* pOutBuf;
    int32_t  nBitStreamLength;

    pXVideoStream = XRetrieverGetStreamAtTime(mPriData->mXRetriever, timeUs);
    if(pXVideoStream == NULL)
    {
        loge("XRetrieverGetStreamAtTime failed.");
        return NULL;
    }

    pOutBuf = pXVideoStream->mBuf;
    nBitStreamLength = pXVideoStream->mSize;

    //* copy bitstream to ashmem.
    if(pOutBuf && nBitStreamLength)
    {
        sp<MemoryHeapBase> heap = new MemoryHeapBase(nBitStreamLength + 4, 0,
                                            "awmetadataretriever");
        unsigned char* pData = NULL;
        if (heap == NULL)
            loge("failed to create MemoryDealer");

        mem = new MemoryBase(heap, 0, nBitStreamLength + 4);
        if (mem == NULL)
        {
            loge("not enough memory for stream size = %d", nBitStreamLength);
        }
        else
        {
            pData = static_cast<unsigned char*>(mem->pointer());
            pData[0] = (nBitStreamLength) & 0xff;
            pData[1] = (nBitStreamLength>>8) & 0xff;
            pData[2] = (nBitStreamLength>>16) & 0xff;
            pData[3] = (nBitStreamLength>>24) & 0xff;
            memcpy(pData+4, pOutBuf, nBitStreamLength);
        }
    }

    logv("bBitStreamLength: %p,%d", pOutBuf, nBitStreamLength);

    return mem;
}

#if ((CONF_ANDROID_MAJOR_VER < 8) || \
    ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER == 0)))
VideoFrame* AwMetadataRetriever::getFrameAtTime(int64_t timeUs, int option)
{
    VideoFrame*       pVideoFrame;
    XVideoFrame*      pXVideoFrame;

    CEDARX_UNUSE(option);

#if MediaScanDedug
        logd("getFrameAtTime, mFd=%d", mPriData->mFd);
#endif

    pXVideoFrame =  XRetrieverGetFrameAtTime(mPriData->mXRetriever, timeUs, option);
    if(pXVideoFrame == NULL)
    {
        loge("XRetrieverGetFrameAtTime failed.");
        return NULL;
    }

    pVideoFrame = new VideoFrame;
    if(pVideoFrame == NULL)
    {
        loge("can not allocate memory for video frame.");
        return NULL;
    }

    pVideoFrame->mDisplayWidth  = pXVideoFrame->mDisplayWidth;
    pVideoFrame->mDisplayHeight = pXVideoFrame->mDisplayHeight;
    pVideoFrame->mWidth         = pXVideoFrame->mWidth;
    pVideoFrame->mHeight        = pXVideoFrame->mHeight;
    pVideoFrame->mSize          = pXVideoFrame->mSize;
    pVideoFrame->mRotationAngle = pXVideoFrame->mRotationAngle;
    pVideoFrame->mData = new unsigned char[pVideoFrame->mSize];
    if(pVideoFrame->mData == NULL)
    {
        loge("can not allocate memory for video frame.");
        delete pVideoFrame;
        return NULL;
    }
    memcpy(pVideoFrame->mData, pXVideoFrame->mData, pXVideoFrame->mSize);

    return pVideoFrame;
}
#else
VideoFrame* AwMetadataRetriever::getFrameAtTime(int64_t timeUs, int option, int colorFormat, bool metaOnly)
{
    int bpp;
    XVideoFrame*      pXVideoFrame;

    CEDARX_UNUSE(option);
    if(metaOnly == true)
    {
        logd("==== metaOnly is true, cannot support");
        return NULL;
    }

    switch (colorFormat)
    {
        case HAL_PIXEL_FORMAT_RGB_565:
        {
            bpp = 2;
            break;
        }
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        {
            loge("Unsupported color format: %d", colorFormat);
            bpp = 4;
            return NULL;
        }
        default:
        {
            loge("Unsupported color format: %d", colorFormat);
            return NULL;
        }
    }

    pXVideoFrame =  XRetrieverGetFrameAtTime(mPriData->mXRetriever, timeUs, option);
    if(pXVideoFrame == NULL)
    {
        loge("XRetrieverGetFrameAtTime failed.");
        return NULL;
    }

    VideoFrame* pVideoFrame = new VideoFrame(pXVideoFrame->mWidth, pXVideoFrame->mHeight,
            pXVideoFrame->mDisplayWidth, pXVideoFrame->mDisplayHeight,
            pXVideoFrame->mRotationAngle, bpp, !metaOnly, NULL/*iccData*/, 0/*iccSize*/);

    pVideoFrame->mData = new unsigned char[pVideoFrame->mSize];
    if(pVideoFrame->mData == NULL)
    {
        loge("can not allocate memory for video frame.");
        delete pVideoFrame;
        return NULL;
    }
    memcpy(pVideoFrame->mData, pXVideoFrame->mData, pXVideoFrame->mSize);

    return pVideoFrame;
}
#endif


static int parseHeaders(CdxKeyedVectorT **header, char *uri,
                            KeyedVector<String8, String8>* pHeaders)
{
    int nHeaderSize;
    int i;

    clearHeaders(header);

    if(uri != NULL)
    {
        if(pHeaders != NULL &&
            (!strncasecmp("http://", uri, 7) || !strncasecmp("https://", uri, 8)))
        {
            //* remove "x-hide-urls-from-log" ?

            nHeaderSize = pHeaders->size();
            if (nHeaderSize <= 0)
                return -1;

            *header = CdxKeyedVectorCreate(nHeaderSize);
            if (*header == NULL)
            {
                logw("CdxKeyedVectorCreate() failed");
                return -1;
            }

            String8 key;
            String8 value;
            for (i = 0; i < nHeaderSize; i++)
            {
                key = pHeaders->keyAt(i);
                value = pHeaders->valueAt(i);
                if (CdxKeyedVectorAdd(*header, key.string(), value.string()) != 0)
                {
                    clearHeaders(header);
                    return -1;
                }
            }
        }
    }

    return 0;
}

static void clearHeaders(CdxKeyedVectorT **header)
{
    if (*header != NULL)
    {
        CdxKeyedVectorDestroy(*header);
        *header = NULL;
    }

    return;
}

static int64_t GetSysTime()
{
    int64_t time;
    struct timeval t;
    gettimeofday(&t, NULL);
    time = (int64_t)t.tv_sec * 1000000;
    time += t.tv_usec;
    return time;
}

static int metafindCStringByKey(CdxMediaInfoT* mediaInfo, int key, cdx_uint8** value)
{
    int strsz = 0;
    switch(key)
    {
        case kKeyAlbum:
            logd("get kKeyAlbum");
            *value = mediaInfo->album;
            strsz  = mediaInfo->albumsz;
            break;
        case kKeyArtist:
            logd("get kKeyArtist");
            *value = mediaInfo->artist;
            strsz  = mediaInfo->artistsz;
            break;
        case kKeyAlbumArtist:
            logd("get kKeyAlbumArtist");
            *value = mediaInfo->albumArtist;
            strsz  = mediaInfo->albumArtistsz;
            break;
        case kKeyAuthor:
            logd("get kKeyAuthor");
            *value = mediaInfo->author;
            strsz  = mediaInfo->authorsz;
            break;
        case kKeyComposer:
            logd("get kKeyComposer");
            *value = mediaInfo->composer;
            strsz  = mediaInfo->composersz;
            break;
        case kKeyDate:
            logd("get kKeyDate");
            *value = mediaInfo->date;
            strsz  = mediaInfo->datesz;
            break;
        case kKeyGenre:
            logd("get kKeyGenre");
            *value = mediaInfo->genre;
            strsz  = mediaInfo->genresz;
            break;
        case kKeyTitle:
            logd("get kKeyTitle");
            *value = mediaInfo->title;
            strsz  = mediaInfo->titlesz;
            break;
        case kKeyYear:
            logd("get kKeyYear");
            *value = mediaInfo->year;
            strsz  = mediaInfo->yearsz;
            break;
        case kKeyWriter:
            logd("get kKeyWriter");
            *value = mediaInfo->writer;
            strsz  = mediaInfo->writersz;
            break;
        case kKeyCompilation:
            logd("get kKeyCompilation");
            *value = mediaInfo->compilation;
            strsz  = mediaInfo->compilationsz;
            break;
        case kKeyCDTrackNumber:
            logd("get kKeyCDTrackNumber");
            *value = mediaInfo->cdTrackNumber;
            strsz  = mediaInfo->cdTrackNumbersz;
            break;
        default:
            logd("get null");
            *value = NULL;
            strsz  = 0;
            break;
    }
    return strsz;
}
