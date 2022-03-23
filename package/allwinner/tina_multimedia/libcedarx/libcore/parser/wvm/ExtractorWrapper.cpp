/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : ExtractorWrapper.cpp
* Description : widevine parser interface wrapper
* History :
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "wvmWrapper"
#include <utils/Log.h>
#include <cdx_config.h>

#include <dlfcn.h>

#include <utils/List.h>
#include <utils/KeyedVector.h>

#include <include/ESDS.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>

#include <CdxParser.h>
#include "WVMDataSource.h"
#include "Extractor.h"
using namespace android;

struct TrackInfo {
    int32_t mTrackIndex;
    int32_t mTrackType;
    int32_t mStreamIndex;
    int32_t mFlags;
    uint8_t *mCSD;
    size_t mCSDSize;
};

extern "C" {

struct BufferInfo {
    uint32_t size;
    void * buffer;
};
enum CreationFlags {
    //TODO:move in cedarx
    kPreferSoftwareCodecs    = 1,
    kIgnoreCodecSpecificData = 2,

    // The client wants to access the output buffer's video
    // data for example for thumbnail extraction.
    kClientNeedsFramebuffer  = 4,

    // Request for software or hardware codecs. If request
    // can not be fullfilled, Create() returns NULL.
    kSoftwareCodecsOnly      = 8,
    kHardwareCodecsOnly      = 16,

    // Store meta data in video buffers
    kStoreMetaDataInVideoBuffers = 32,

    // Only submit one input buffer at one time.
    kOnlySubmitOneInputBufferAtOneTime = 64,

    // Enable GRALLOC_USAGE_PROTECTED for output buffers from native window
    kEnableGrallocUsageProtected = 128,

    // Secure decoding mode
    kUseSecureInputBuffers = 256,
};

static uint16_t WVM_U16_AT(const uint8_t *ptr) {
    return ptr[0] << 8 | ptr[1];
}

uint32_t WVM_U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

uint64_t WVM_U64_AT(const uint8_t *ptr) {
    return ((uint64_t)WVM_U32_AT(ptr)) << 32 | WVM_U32_AT(ptr + 4);
}

uint16_t WVM_U16LE_AT(const uint8_t *ptr) {
    return ptr[0] | (ptr[1] << 8);
}

uint32_t WVM_U32LE_AT(const uint8_t *ptr) {
    return ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
}

uint64_t WVM_U64LE_AT(const uint8_t *ptr) {
    return ((uint64_t)WVM_U32LE_AT(ptr + 4)) << 32 | WVM_U32LE_AT(ptr);
}

struct ExtractorWrapper {
public:
    ExtractorWrapper();
    ~ExtractorWrapper();
    //Always put CdxParserT at start of class.
    CdxParserT mParser;
    void *mExtractor;
    bool mEos;
    size_t mCurrentTrack;
    int32_t mStatus;
    int64_t mDurationUs;
    KeyedVector<size_t, TrackInfo *> mTracks;
private:
    ExtractorWrapper(const ExtractorWrapper&);
    ExtractorWrapper& operator=(const ExtractorWrapper&);
};

ExtractorWrapper::ExtractorWrapper()
    :mExtractor(NULL),
     mEos(false),
     mCurrentTrack(0),
     mStatus(0),
     mDurationUs(0) {
}

ExtractorWrapper::~ExtractorWrapper () {

}

static cdx_int32 extractorWrapperControl(CdxParserT * parser, cdx_int32 cmd, void *param)
{
    ALOGV("extractorWrapperControl");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    switch(cmd) {
    case CDX_PSR_CMD_SET_SECURE_BUFFER_COUNT:
        extractor->setBufferCount(*((int32_t*)param));
        break;
    case CDX_PSR_CMD_SET_SECURE_BUFFERS:
        extractor->setBuffers(((BufferInfo*)param)->buffer, ((BufferInfo*)param)->size);
        break;
    default:break;
    }
    return 0;
}

static cdx_int32 extractorWrapperPrefetch(CdxParserT *parser, CdxPacketT* pkt)
{
    //ALOGV("extractorWrapperPrefetch");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    if(wrapper->mEos) {
        return -1;
    }
    size_t trackIndex;
    if(extractor->getSampleTrackIndex(&trackIndex) < 0) {
        wrapper->mEos = true;
        return -1;
    }
    ssize_t index = wrapper->mTracks.indexOfKey(trackIndex);
    if(index < 0) {
        return -1;
    }
    TrackInfo *trackInfo = wrapper->mTracks.valueAt(index);
    extractor->getSampleSize((size_t *)&pkt->length);
    extractor->getSampleTime(&pkt->pts);
    pkt->flags = FIRST_PART|LAST_PART;
    pkt->type = trackInfo->mTrackType;
    pkt->streamIndex = trackInfo->mStreamIndex;
    //ALOGV("index %d, type %d, length %d, pts %lld", index, pkt->type, pkt->length, pkt->pts);
    return 0;
}

static cdx_int32 extractorWrapperRead(CdxParserT *parser, CdxPacketT* pkt)
{
    //ALOGV("extractorWrapperRead");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    if(wrapper->mEos) {
        return -1;
    }
    size_t trackIndex;
    if(extractor->getSampleTrackIndex(&trackIndex) < 0) {
        wrapper->mEos = true;
        return -1;
    }
    ssize_t index = wrapper->mTracks.indexOfKey(trackIndex);
    if(index < 0) {
        return -1;
    }
    TrackInfo *trackInfo = wrapper->mTracks.valueAt(index);
    status_t err = extractor->readSampleData((uint8_t *)pkt->buf, pkt->buflen,
                (uint8_t *)pkt->ringBuf, pkt->ringBufLen,
                trackInfo->mFlags & kUseSecureInputBuffers);
        extractor->advance();

    if(err != OK) {
        wrapper->mEos = true;
        return -1;
    }
    return err;
}

static void extractTrackMediaInfo(int32_t mediaType, sp<MetaData> meta, void *stream, String8 *mime)
{
    if(mediaType == CDX_MEDIA_VIDEO) {
        VideoStreamInfo* video = (VideoStreamInfo*)stream;
        if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_VIDEO_AVC)) {
            video->eCodecFormat = VIDEO_CODEC_FORMAT_H264;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_VIDEO_MPEG4)) {
            video->eCodecFormat = VIDEO_CODEC_FORMAT_MPEG4;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_VIDEO_H263)) {
            video->eCodecFormat = VIDEO_CODEC_FORMAT_H263;
        } else {
            video->eCodecFormat = VIDEO_CODEC_FORMAT_UNKNOWN;
        }
        int32_t width, height, bitrate;
        meta->findInt32(kKeyWidth, &width);
        meta->findInt32(kKeyHeight, &height);
        if (!meta->findInt32(kKeyBitRate, &bitrate)) {
            bitrate = 0;
        }

        video->nWidth = width;
        video->nHeight = height;
        video->nFrameRate = 0;
        video->bIs3DStream = 0;
        video->nCodecSpecificDataLen = 0;
        video->pCodecSpecificData = NULL;
    } else if(mediaType == CDX_MEDIA_AUDIO) {
        AudioStreamInfo* audio = (AudioStreamInfo*)stream;
        if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_AMR_NB)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_AMR;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_MPEG)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_MP3;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_MP1;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_MP2;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_AAC)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_MPEG_AAC_LC;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_QCELP)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_VORBIS)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_G711_ALAW)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_G711_MLAW)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_RAW)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_FLAC)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else if (!strcasecmp(mime->string(), MEDIA_MIMETYPE_AUDIO_AAC_ADTS)) {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        } else {
            audio->eCodecFormat = AUDIO_CODEC_FORMAT_UNKNOWN;
            audio->eSubCodecFormat = 0;
        }
        int32_t channels, sample_rate;
        meta->findInt32(kKeyChannelCount, &channels);
        meta->findInt32(kKeySampleRate, &sample_rate);

        audio->nChannelNum = channels;
        audio->nSampleRate = sample_rate;
        audio->nCodecSpecificDataLen = 0;
        audio->pCodecSpecificData = NULL;
    }
}

static void extractTrackCSD(sp<MetaData> meta, List<sp<ABuffer> > *csd) {
    uint32_t type;
    const void *data;
    size_t size;
    if (meta->findData(kKeyAVCC, &type, &data, &size)) {
        // Parse the AVCDecoderConfigurationRecord

        const uint8_t *ptr = (const uint8_t *)data;

        CHECK(size >= 7);
        CHECK_EQ((unsigned)ptr[0], 1u);  // configurationVersion == 1
        uint8_t profile = ptr[1];
        uint8_t level = ptr[3];

        // There is decodable content out there that fails the following
        // assertion, let's be lenient for now...
        // CHECK((ptr[4] >> 2) == 0x3f);  // reserved

        size_t lengthSize = 1 + (ptr[4] & 3);

        // commented out check below as H264_QVGA_500_NO_AUDIO.3gp
        // violates it...
        // CHECK((ptr[5] >> 5) == 7);  // reserved

        size_t numSeqParameterSets = ptr[5] & 31;

        ptr += 6;
        size -= 6;

        sp<ABuffer> buffer = new ABuffer(1024);
        buffer->setRange(0, 0);

        for (size_t i = 0; i < numSeqParameterSets; ++i) {
            CHECK(size >= 2);
            size_t length = WVM_U16_AT(ptr);

            ptr += 2;
            size -= 2;

            CHECK(size >= length);

            memcpy(buffer->data() + buffer->size(), "\x00\x00\x00\x01", 4);
            memcpy(buffer->data() + buffer->size() + 4, ptr, length);
            buffer->setRange(0, buffer->size() + 4 + length);

            ptr += length;
            size -= length;
        }

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);

        csd->push_back(buffer);

        buffer = new ABuffer(1024);
        buffer->setRange(0, 0);

        CHECK(size >= 1);
        size_t numPictureParameterSets = *ptr;
        ++ptr;
        --size;

        for (size_t i = 0; i < numPictureParameterSets; ++i) {
            CHECK(size >= 2);
            size_t length = WVM_U16_AT(ptr);

            ptr += 2;
            size -= 2;

            CHECK(size >= length);

            memcpy(buffer->data() + buffer->size(), "\x00\x00\x00\x01", 4);
            memcpy(buffer->data() + buffer->size() + 4, ptr, length);
            buffer->setRange(0, buffer->size() + 4 + length);

            ptr += length;
            size -= length;
        }

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        csd->push_back(buffer);
    } else if (meta->findData(kKeyESDS, &type, &data, &size)) {
        ESDS esds((const char *)data, size);
        CHECK_EQ(esds.InitCheck(), (status_t)OK);

        const void *codec_specific_data;
        size_t codec_specific_data_size;
        esds.getCodecSpecificInfo(
                &codec_specific_data, &codec_specific_data_size);

        sp<ABuffer> buffer = new ABuffer(codec_specific_data_size);

        memcpy(buffer->data(), codec_specific_data,
               codec_specific_data_size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        csd->push_back(buffer);
    } else if (meta->findData(kKeyVorbisInfo, &type, &data, &size)) {
        sp<ABuffer> buffer = new ABuffer(size);
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        csd->push_back(buffer);

        if (!meta->findData(kKeyVorbisBooks, &type, &data, &size)) {
            return;
        }

        buffer = new ABuffer(size);
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        csd->push_back(buffer);
    }
}

static cdx_int32 extractorWrapperGetMediaInfo(CdxParserT* parser, CdxMediaInfoT* mediaInfo)
{
    ALOGV("extractorWrapperGetMediaInfo");
    ExtractorWrapper * wrapper = (ExtractorWrapper *) parser;
    if (wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *) wrapper->mExtractor;
    if (extractor == NULL) {
        return -1;
    }
    VideoStreamInfo* video = NULL;
    AudioStreamInfo* audio = NULL;
    int32_t videoIndex = 0;
    int32_t audioIndex = 0;
    int32_t flags = 0;
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<MetaData> meta = extractor->getTrackFormat(i);
        TrackInfo *trackInfo = new TrackInfo();
        trackInfo->mTrackIndex = i;
        trackInfo->mCSD = NULL;
        trackInfo->mCSDSize = 0;
        trackInfo->mFlags = 0;
        const char *_mime;
        CHECK(meta->findCString(kKeyMIMEType, &_mime));

        String8 mime = String8(_mime);
        ALOGV("mime %s", mime.string());
        if (!strncasecmp(mime.string(), "video/", 6)) {
            video = &mediaInfo->program[0].video[mediaInfo->program[0].videoNum];

            extractTrackMediaInfo(CDX_MEDIA_VIDEO, meta, video, &mime);

            int32_t requiresSecureBuffers;
            if (meta->findInt32(kKeyRequiresSecureBuffers, &requiresSecureBuffers)
                    && requiresSecureBuffers) {
                    //widevine level 1
                ALOGV("require secure buffer");
                trackInfo->mFlags |= kIgnoreCodecSpecificData;
                trackInfo->mFlags  |= kUseSecureInputBuffers;
                trackInfo->mFlags |= kEnableGrallocUsageProtected;
                //TODO:
                flags |= kUseSecureInputBuffers;
                flags |= kEnableGrallocUsageProtected;
            }
            mediaInfo->program[0].videoNum++;
            trackInfo->mTrackType = CDX_MEDIA_VIDEO;
            trackInfo->mStreamIndex = videoIndex ++;
        } else if (!strncasecmp(mime.string(), "audio/", 6)) {
            audio =    &mediaInfo->program[0].audio[mediaInfo->program[0].audioNum];
            extractTrackMediaInfo(CDX_MEDIA_AUDIO, meta, audio, &mime);

            mediaInfo->program[0].audioNum++;
            trackInfo->mTrackType = CDX_MEDIA_AUDIO;
            trackInfo->mStreamIndex = audioIndex ++;
            if((audio->eCodecFormat == AUDIO_CODEC_FORMAT_MPEG_AAC_LC)
                    && (flags & kUseSecureInputBuffers)) {
                //It's strange, MediaCodec can read samples with frame header data,
                //but player cannot, why?
                audio->nFlags = 1;
            }
        } else {
            //we have not support subtile.
            delete trackInfo;
            continue;
        }
        if(!(trackInfo->mFlags & kIgnoreCodecSpecificData)) {
            List<sp<ABuffer> > trackCSD;
            extractTrackCSD(meta, &trackCSD);

            size_t size = 0;
            List<sp<ABuffer> >::iterator it = trackCSD.begin();
            while(it != trackCSD.end()) {
                size += (*it)->size();
                ++it;
            }
            uint8_t *buffer = new uint8_t[size];

            it = trackCSD.begin();
            size = 0;
            while(it != trackCSD.end()) {
                memcpy(buffer + size, (*it)->data(), (*it)->size());
                size += (*it)->size();
                ++it;
            }
            ALOGV("codec specific data len %u", size);
            if(trackInfo->mTrackType == CDX_MEDIA_AUDIO) {
                audio->nCodecSpecificDataLen = size;
                audio->pCodecSpecificData = (char*)buffer;
            } else if(trackInfo->mTrackType == CDX_MEDIA_VIDEO) {
                video->nCodecSpecificDataLen = size;
                video->pCodecSpecificData = (char*)buffer;
            }

            trackInfo->mCSD = buffer;
            trackInfo->mCSDSize = size;
        }
        int64_t duration;
        meta->findInt64(kKeyDuration, &duration);
        wrapper->mDurationUs = duration > wrapper->mDurationUs ? duration : wrapper->mDurationUs;
        ALOGV("add track (%d, %p), mime %s", i, trackInfo, mime.string());
        wrapper->mTracks.add(i, trackInfo);
        extractor->selectTrack(i);
    }
    mediaInfo->bSeekable = !!(extractor->getSourceFlags() & MediaExtractor::CAN_SEEK);
    mediaInfo->programIndex = 0;
    mediaInfo->programNum = 1;
    mediaInfo->program[0].duration = wrapper->mDurationUs/1000ll;
    mediaInfo->program[0].audioIndex = 0;
    mediaInfo->program[0].videoIndex = 0;
    mediaInfo->program[0].subtitleIndex = 0;
    mediaInfo->program[0].flags = flags;

    return 0;
}

static cdx_int32 extractorWrapperSeekTo(CdxParserT* parser, cdx_int64 timeUs)
{
    ALOGV("extractorWrapperSeekTo");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    wrapper->mEos = false;
    extractor->seekTo(timeUs);
    return 0;
}

static cdx_uint32 extractorWrapperAttribute(CdxParserT* parser)
{
    ALOGV("extractorWrapperAttribute");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    return 0;
}

static cdx_int32 extractorWrapperForceStop(CdxParserT *parser)
{
    ALOGV("extractorWrapperForceStop");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    return 0;
}

static cdx_int32 extractorWrapperGetStatus(CdxParserT* parser)
{
    ALOGV("extractorWrapperGetStatus");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    return 0;
}

static cdx_int32 extractorWrapperClose(CdxParserT* parser)
{
    ALOGV("extractorWrapperClose");
    ExtractorWrapper * wrapper = (ExtractorWrapper *)parser;
    if(wrapper == NULL) {
        return -1;
    }
    Extractor *extractor = (Extractor *)wrapper->mExtractor;
    if(extractor == NULL) {
        return -1;
    }
    while(wrapper->mTracks.size() > 0) {
        TrackInfo *trackInfo = wrapper->mTracks.valueAt(0);
        wrapper->mTracks.removeItemsAt(0);
        if(trackInfo->mCSD) {
            delete [] trackInfo->mCSD;
        }
        delete trackInfo;
    }
    extractor->stop();
    delete extractor;

    delete wrapper;
    return 0;
}

cdx_int32 extractorWrapperInit(CdxParserT *) {
    return 0;
}

struct CdxParserOpsS wvmOps = {
     extractorWrapperControl,
     extractorWrapperPrefetch,
     extractorWrapperRead,
     extractorWrapperGetMediaInfo,
     extractorWrapperSeekTo,
     extractorWrapperAttribute,
     extractorWrapperGetStatus,
     extractorWrapperClose,
     extractorWrapperInit
};

CdxParserT* extractorWrapperOpen(CdxStreamT *stream, cdx_uint32 flags __attribute__((__unused__)))
{
    ALOGI("extractorWrapperOpen");
    ExtractorWrapper * wrapper = new ExtractorWrapper();

    wrapper->mParser.ops = &wvmOps;
    wrapper->mParser.type = CDX_PARSER_WVM;
    Extractor *extractor = new Extractor();
    wrapper->mExtractor = (void *)extractor;

    sp<DataSource> dataSource = new WVMDataSource(stream);

    status_t err = extractor->setDataSource(dataSource);
    if(err) {
        ALOGE("setDataSource failed");
        delete extractor;
        delete wrapper;
        return NULL;
    }
    bool haveAudio = false;
    bool haveVideo = false;
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<MetaData> meta = extractor->getTrackFormat(i);
        const char *_mime;
        CHECK(meta->findCString(kKeyMIMEType, &_mime));

        String8 mime = String8(_mime);
        ALOGV("mime %s", mime.string());
        if (!haveVideo && !strncasecmp(mime.string(), "video/", 6)) {
            haveVideo = true;
        } else if (!haveAudio && !strncasecmp(mime.string(), "audio/", 6)) {
            haveAudio = true;
        }
    }

    ALOGV("have video %d, have audio %d", haveVideo, haveAudio);
    if(!haveVideo && !haveAudio) {
        delete extractor;
        delete wrapper;
        return NULL;
    }
    //filter out other streams.
    if(!extractor->isWidevine()) {
        ALOGI("This is not a widevine video");
        delete extractor;
        delete wrapper;
        return NULL;
    }
    err = extractor->start();
    if(err != 0) {
        ALOGE("extractor start failed");
        delete extractor;
        delete wrapper;
        return NULL;
    }
    return &wrapper->mParser;
}

static cdx_uint32 extractorWrapperProbe(CdxStreamProbeDataT *probeData)
{
    void *streamControlLibHandle = NULL;
#ifdef SECUREOS_ENABLED
    streamControlLibHandle = dlopen("libWVStreamControlAPI_L1.so", RTLD_NOW);
#else
    streamControlLibHandle = dlopen("libWVStreamControlAPI_L3.so", RTLD_NOW);
#endif
    if(streamControlLibHandle == NULL) {
        ALOGI("could not find WVStreamControlAPI lib");
        return 0;
    }

    typedef bool
            (*SnifferFunc)(const char *buffer, size_t length);
    SnifferFunc snifferFunc =
        (SnifferFunc) dlsym(streamControlLibHandle,
                            "_Z18WV_IsWidevineMediaPKcj");
    if(snifferFunc == NULL)
        return 0;

    if(snifferFunc(probeData->buf, probeData->len)) {
        ALOGI("This is widevine video");
        return 100;
    }
    return 0;
}

CdxParserCreatorT wvmParserCtor =
{
create: extractorWrapperOpen,
probe: extractorWrapperProbe
};

} //end extern "C"
