/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "Extractor"
#include <utils/Log.h>

#include <include/NuCachedSource2.h>
#include <include/WVMExtractor.h>
#include <include/DRMExtractor.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <cutils/properties.h>
#include <cdx_log.h>
#include <sc_interface.h>

#include "cdx_config.h"
#if (CONF_ANDROID_MAJOR_VER >= 5)
#include <media/IMediaHTTPConnection.h>
#include <media/IMediaHTTPService.h>
#endif
#include "Extractor.h"
#include "memoryAdapter.h"

namespace android {

Extractor::Extractor()
    :mDataSource(NULL),
     mImpl(NULL),
     mIsWidevineExtractor(false),
     mTotalBitrate(-1ll),
     mDurationUs(-1ll),
     mDrmManagerClient(NULL),
     mDecryptHandle(NULL),
     mSeekTimeUs(0),
     mBufferCount(0),
     mExtractorFlags(0) {
    DataSource::RegisterDefaultSniffers();
#if (CONF_ANDROID_MAJOR_VER >= 5)
    mHTTPService = NULL;
#endif
    #ifdef SECUREOS_ENABLED
    mMemOps = SecureMemAdapterGetOpsS();
    #else
    mMemOps = MemAdapterGetOpsS();
    #endif
    CdcMemOpen(mMemOps);
    ALOGV("Extractor()");
}

Extractor::~Extractor() {
    ALOGV("~Extractor()");
    stop();
    releaseTrackSamples();

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        CHECK_EQ((status_t)OK, info->mSource->stop());
    }

    mSelectedTracks.clear();
#if (CONF_ANDROID_MAJOR_VER >= 5)
    if(mHTTPService != NULL) {
        mHTTPService->decStrong(this);
        mHTTPService.clear();
    }
#endif
    CdcMemClose(mMemOps);
}

status_t Extractor::start() {
    ALOGV("start");
    if (mDecryptHandle != NULL) {
        mDrmManagerClient->setPlaybackStatus(mDecryptHandle,
                Playback::START, mSeekTimeUs/1000);
    }
    return OK;
}

status_t Extractor::pause() {
    ALOGV("pause");
    if (mDecryptHandle != NULL) {
        mDrmManagerClient->setPlaybackStatus(mDecryptHandle,
                Playback::PAUSE, 0);
    }
    return OK;
}

status_t Extractor::stop() {
    ALOGV("stop");
    if(mDrmManagerClient != NULL) {
        mDrmManagerClient->setPlaybackStatus(mDecryptHandle, Playback::STOP, 0);
        mDecryptHandle = NULL;
        mDrmManagerClient = NULL;
    }
    return OK;
}

void Extractor::checkDrmStatus(const sp<DataSource>& dataSource) {
    ALOGV("checkDrmStatus");
    dataSource->getDrmInfo(mDecryptHandle, &mDrmManagerClient);
    if (mDecryptHandle != NULL) {
        CHECK(mDrmManagerClient);
        if (RightsStatus::RIGHTS_VALID != mDecryptHandle->status) {
            ALOGE("license not right");
        }
    }
}

status_t Extractor::setDataSource(
        const char *uri, const KeyedVector<String8, String8> *headers,
        void *httpService) {
    ALOGV("setDataSource url");

    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL) {
        return -EINVAL;
    }

    sp<DataSource> dataSource;

#if (CONF_ANDROID_MAJOR_VER >= 5)
    mHTTPService = static_cast<IMediaHTTPService *>(httpService);
    dataSource = DataSource::CreateFromURI(
            mHTTPService, uri, headers);
    if(mHTTPService != NULL)
        mHTTPService->incStrong(this);

#else
    dataSource = DataSource::CreateFromURI(uri, headers);
#endif

    if (dataSource == NULL) {
        return -ENOENT;
    }

    mIsWidevineExtractor = false;
    if (!strncasecmp("widevine://", uri, 11)) {
        String8 mimeType;
        float confidence;
        sp<AMessage> dummy;
        bool success = SniffWVM(dataSource, &mimeType, &confidence, &dummy);

        if (!success
                || strcasecmp(
                    mimeType.string(), MEDIA_MIMETYPE_CONTAINER_WVM)) {
            return ERROR_UNSUPPORTED;
        }

        sp<WVMExtractor> extractor = new WVMExtractor(dataSource);
        extractor->setAdaptiveStreamingMode(true);

        mImpl = extractor;
        mIsWidevineExtractor = true;
    } else {
        mImpl = MediaExtractor::Create(dataSource);
    }

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }
    if (mImpl->getDrmFlag()) {
        checkDrmStatus(dataSource);
    }
    mExtractorFlags = mImpl->flags();

    mDataSource = dataSource;

    updateDurationAndBitrate();

    return OK;
}

status_t Extractor::setDataSource(int fd, off64_t offset, off64_t size) {
    ALOGV("setDataSource (%d, %lld, %lld)", fd, offset, size);
    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL) {
        return -EINVAL;
    }

    sp<FileSource> fileSource = new FileSource(dup(fd), offset, size);

    status_t err = fileSource->initCheck();
    if (err != OK) {
        return err;
    }

    mImpl = MediaExtractor::Create(fileSource);

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }

    if (mImpl->getDrmFlag()) {
        checkDrmStatus(fileSource);
    }
    mExtractorFlags = mImpl->flags();

    mDataSource = fileSource;

    updateDurationAndBitrate();

    return OK;
}

status_t Extractor::setDataSource(const sp<DataSource> &source) {
    ALOGV("setDataSource");
    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL) {
        return -EINVAL;
    }

    status_t err = source->initCheck();
    if (err != OK) {
        return err;
    }
    String8 uri = source->getUri();
    if (!strncasecmp("widevine://", uri.string(), 11)) {
        String8 mimeType;
        float confidence;
        sp<AMessage> dummy;
        bool success = SniffWVM(source, &mimeType, &confidence, &dummy);

        if (!success
                || strcasecmp(
                    mimeType.string(), MEDIA_MIMETYPE_CONTAINER_WVM)) {
            return ERROR_UNSUPPORTED;
        }

        sp<WVMExtractor> extractor = new WVMExtractor(source);
        extractor->setAdaptiveStreamingMode(true);

        mImpl = extractor;
        mIsWidevineExtractor = true;
    } else {
        mImpl = MediaExtractor::Create(source);
    }

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }

    mExtractorFlags = mImpl->flags();

    mDataSource = source;

    updateDurationAndBitrate();

    return OK;
}

void Extractor::updateDurationAndBitrate() {
    ALOGV("updateDurationAndBitrate");
    mTotalBitrate = 0ll;
    mDurationUs = -1ll;

    for (size_t i = 0; i < mImpl->countTracks(); ++i) {
        sp<MetaData> meta = mImpl->getTrackMetaData(i);

        int32_t bitrate;
        if (!meta->findInt32(kKeyBitRate, &bitrate)) {
            const char *mime;
            CHECK(meta->findCString(kKeyMIMEType, &mime));
            ALOGV("track of type '%s' does not publish bitrate", mime);

            mTotalBitrate = -1ll;
        } else if (mTotalBitrate >= 0ll) {
            mTotalBitrate += bitrate;
        }

        int64_t durationUs;
        if (meta->findInt64(kKeyDuration, &durationUs)
                && durationUs > mDurationUs) {
            mDurationUs = durationUs;
        }
    }
}

size_t Extractor::countTracks() const {
    ALOGV("countTracks");
    Mutex::Autolock autoLock(mLock);

    return mImpl == NULL ? 0 : mImpl->countTracks();
}

status_t Extractor::getTrackFormat(
        size_t index, sp<AMessage> *format) const {
    ALOGV("getTrackFormat");
    Mutex::Autolock autoLock(mLock);

    *format = NULL;

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    sp<MetaData> meta = mImpl->getTrackMetaData(index);
    return convertMetaDataToMessage(meta, format);
}

sp<MetaData> Extractor::getTrackFormat(
        size_t index) const {
    ALOGV("getTrackFormat");
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return NULL;
    }

    if (index >= mImpl->countTracks()) {
        return NULL;
    }

    return mImpl->getTrackMetaData(index);
}

status_t Extractor::getFileFormat(sp<AMessage> *format) const {
    Mutex::Autolock autoLock(mLock);

    *format = NULL;

    if (mImpl == NULL) {
        return -EINVAL;
    }

    sp<MetaData> meta = mImpl->getMetaData();

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));
    *format = new AMessage();
    (*format)->setString("mime", mime);
#if ((CONF_ANDROID_MAJOR_VER > 4) || ((CONF_ANDROID_MAJOR_VER == 4)&&(CONF_ANDROID_SUB_VER >= 4)))
    /* >= 4.4 */
    //Protection System Specific Header box is used for common encryption.
    uint32_t type;
    const void *pssh;
    size_t psshsize;
    if (meta->findData(kKeyPssh, &type, &pssh, &psshsize)) {
        sp<ABuffer> buf = new ABuffer(psshsize);
        memcpy(buf->data(), pssh, psshsize);
        (*format)->setBuffer("pssh", buf);
    }
#endif
    return OK;
}

status_t Extractor::selectTrack(size_t index) {
    ALOGV("selectTrack");
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mTrackIndex == index) {
            // This track has already been selected.
            return OK;
        }
    }

    sp<MediaSource> source = mImpl->getTrack(index);

    CHECK_EQ((status_t)OK, source->start());

    mSelectedTracks.push();
    TrackInfo *info = &mSelectedTracks.editItemAt(mSelectedTracks.size() - 1);

    info->mSource = source;
    info->mTrackIndex = index;
    info->mFinalResult = OK;
    info->mSample = NULL;
    info->mSampleTimeUs = -1ll;
    info->mTrackFlags = 0;

    const char *mime;
    CHECK(source->getFormat()->findCString(kKeyMIMEType, &mime));

    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
        info->mTrackFlags |= kIsVorbis;
    }

    return OK;
}

status_t Extractor::unselectTrack(size_t index) {
    ALOGV("unselectTrack");
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    size_t i;
    for (i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mTrackIndex == index) {
            break;
        }
    }

    if (i == mSelectedTracks.size()) {
        // Not selected.
        return OK;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(i);

    if (info->mSample != NULL) {
        info->mSample->release();
        info->mSample = NULL;

        info->mSampleTimeUs = -1ll;
    }

    CHECK_EQ((status_t)OK, info->mSource->stop());

    mSelectedTracks.removeAt(i);

    return OK;
}

void Extractor::releaseTrackSamples() {
    ALOGV("releaseTrackSamples");
    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mSample != NULL) {
            info->mSample->release();
            info->mSample = NULL;

            info->mSampleTimeUs = -1ll;
        }
    }
}

ssize_t Extractor::fetchTrackSamples(
        int64_t seekTimeUs, MediaSource::ReadOptions::SeekMode mode) {
    ALOGV("fetchTrackSamples");
    TrackInfo *minInfo = NULL;
    ssize_t minIndex = -1;

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (seekTimeUs >= 0ll) {
            info->mFinalResult = OK;

            if (info->mSample != NULL) {
                info->mSample->release();
                info->mSample = NULL;
                info->mSampleTimeUs = -1ll;
            }
        } else if (info->mFinalResult != OK) {
            continue;
        }

        if (info->mSample == NULL) {
            MediaSource::ReadOptions options;
            if (seekTimeUs >= 0ll) {
                options.setSeekTo(seekTimeUs, mode);
            }
            status_t err = info->mSource->read(&info->mSample, &options);

            if (err != OK) {
                CHECK(info->mSample == NULL);

                info->mFinalResult = err;

                if (info->mFinalResult != ERROR_END_OF_STREAM) {
                    ALOGW("read on track %d failed with error %d",
                          info->mTrackIndex, err);
                }

                info->mSampleTimeUs = -1ll;
                continue;
            } else {
                CHECK(info->mSample != NULL);
                CHECK(info->mSample->meta_data()->findInt64(
                            kKeyTime, &info->mSampleTimeUs));
            }
        }

        if (minInfo == NULL  || info->mSampleTimeUs < minInfo->mSampleTimeUs) {
            minInfo = info;
            minIndex = i;
        }
    }

    return minIndex;
}

status_t Extractor::seekTo(
        int64_t timeUs, MediaSource::ReadOptions::SeekMode mode) {
    ALOGV("seekTo, timeUs %lld", timeUs);
    Mutex::Autolock autoLock(mLock);
    ssize_t minIndex = fetchTrackSamples(timeUs, mode);

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }
    if (mDecryptHandle != NULL) {
        mDrmManagerClient->setPlaybackStatus(mDecryptHandle,
                Playback::PAUSE, 0);
        mDrmManagerClient->setPlaybackStatus(mDecryptHandle,
                Playback::START, timeUs / 1000);
    }
    mSeekTimeUs = timeUs;
    return OK;
}

status_t Extractor::advance() {
    ALOGV("advance");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);

    info->mSample->release();
    info->mSample = NULL;
    info->mSampleTimeUs = -1ll;

    return OK;
}

status_t Extractor::readSampleData(const sp<ABuffer> &buffer) {
    ALOGV("readSampleData");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);

    size_t sampleSize = info->mSample->range_length();

    if (info->mTrackFlags & kIsVorbis) {
        // Each sample's data is suffixed by the number of page samples
        // or -1 if not available.
        sampleSize += sizeof(int32_t);
    }

    if (buffer->capacity() < sampleSize) {
        return -ENOMEM;
    }

    const uint8_t *src =
        (const uint8_t *)info->mSample->data()
            + info->mSample->range_offset();

    memcpy((uint8_t *)buffer->data(), src, info->mSample->range_length());

    if (info->mTrackFlags & kIsVorbis) {
        int32_t numPageSamples;
        if (!info->mSample->meta_data()->findInt32(
                    kKeyValidSamples, &numPageSamples)) {
            numPageSamples = -1;
        }

        memcpy((uint8_t *)buffer->data() + info->mSample->range_length(),
               &numPageSamples,
               sizeof(numPageSamples));
    }

    buffer->setRange(0, sampleSize);

    return OK;
}

status_t Extractor::readSampleData(uint8_t *buf, size_t length,
        uint8_t *extraBuf, size_t extraLength, bool secure) {
    ALOGV("readSampleData 2");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);

    size_t sampleSize = info->mSample->range_length();

    if (info->mTrackFlags & kIsVorbis) {
        // Each sample's data is suffixed by the number of page samples
        // or -1 if not available.
        sampleSize += sizeof(int32_t);
    }

    if (length + extraLength < sampleSize) {
        return -ENOMEM;
    }

    uint8_t *src =
        (uint8_t *)info->mSample->data()
            + info->mSample->range_offset();
    if(secure) {
        //source buffer is secure buffer, we supplied physical address
        //for wvm extractor.
#ifdef SECUREOS_ENABLED
        src = (uint8_t *)src/*SecureMemAdapterGetVirtualAddressCpu(src)*/;
#endif
    }
    if(info->mSample->range_length() > length) {
        if(!secure) {
            memcpy(buf, src, length);
            memcpy(extraBuf, src + length, info->mSample->range_length() - length);
        } else {
            CdcMemCopy(mMemOps, buf, src, length);
            CdcMemCopy(mMemOps, extraBuf, src + length, info->mSample->range_length() - length);
        }
    } else {
        if(!secure) {
            memcpy(buf, src, info->mSample->range_length());
        } else {
            CdcMemCopy(mMemOps,buf, src, info->mSample->range_length());
        }
    }
    if (info->mTrackFlags & kIsVorbis) {
        int32_t numPageSamples;
        if (!info->mSample->meta_data()->findInt32(
                    kKeyValidSamples, &numPageSamples)) {
            numPageSamples = -1;
        }
        int32_t tailSize = info->mSample->range_length() - length;
        uint8_t *pageSamples = (uint8_t *)&numPageSamples;
        if(tailSize > 0) {
            if((size_t)tailSize >= sizeof(numPageSamples)) {
                //case buf is enough to fill page samples
                if(!secure) {
                    memcpy(buf + info->mSample->range_length(),
                           pageSamples, sizeof(numPageSamples));
                } else {
                    CdcMemWrite(mMemOps, buf + info->mSample->range_length(),
                                pageSamples, sizeof(numPageSamples));
                }
            } else {
                //case page samples have to be copied in two buffers.
                if(!secure) {
                    memcpy(buf + info->mSample->range_length(), pageSamples, tailSize);
                    memcpy(extraBuf, pageSamples + tailSize, sizeof(numPageSamples) - tailSize);
                } else {
                    CdcMemWrite(mMemOps,buf + info->mSample->range_length(),
                                pageSamples, tailSize);
                    CdcMemWrite(mMemOps, extraBuf, pageSamples + tailSize,
                                sizeof(numPageSamples) - tailSize);
                }
            }
        } else {
            //case buf is full.
            if(!secure) {
                memcpy(extraBuf + info->mSample->range_length() - length, &numPageSamples,
                        sizeof(numPageSamples));
            } else {
                CdcMemWrite(mMemOps, extraBuf + info->mSample->range_length() - length,
                           &numPageSamples,sizeof(numPageSamples));
            }
        }
    }

    return OK;
}

status_t Extractor::getSampleTrackIndex(size_t *trackIndex) {
    ALOGV("getSampleTrackIndex");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *trackIndex = info->mTrackIndex;

    return OK;
}

status_t Extractor::getSampleTime(int64_t *sampleTimeUs) {
    ALOGV("getSampleTime");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *sampleTimeUs = info->mSampleTimeUs;

    return OK;
}

status_t Extractor::getSampleSize(size_t *sampleSize) {
    ALOGV("getSampleTime");
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *sampleSize = info->mSample->range_length();

    if (info->mTrackFlags & kIsVorbis) {
        // Each sample's data is suffixed by the number of page samples
        // or -1 if not available.
        *sampleSize += sizeof(int32_t);
    }

    return OK;
}

status_t Extractor::getSampleMeta(sp<MetaData> *sampleMeta) {
    ALOGV("getSampleMeta");
    Mutex::Autolock autoLock(mLock);

    *sampleMeta = NULL;

    ssize_t minIndex = fetchTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *sampleMeta = info->mSample->meta_data();

    return OK;
}

bool Extractor::getTotalBitrate(int64_t *bitrate) const {
    ALOGV("getTotalBitrate");
    if (mTotalBitrate >= 0) {
        *bitrate = mTotalBitrate;
        return true;
    }

    off64_t size;
    if (mDurationUs >= 0 && mDataSource->getSize(&size) == OK) {
        *bitrate = size * 8000000ll / mDurationUs;  // in bits/sec
        return true;
    }

    return false;
}

// Returns true iff cached duration is available/applicable.
bool Extractor::getCachedDuration(
        int64_t *durationUs, bool *eos) const {
    ALOGV("getCachedDuration");
    Mutex::Autolock autoLock(mLock);

    int64_t bitrate;
    if (mIsWidevineExtractor) {
        sp<WVMExtractor> wvmExtractor =
            static_cast<WVMExtractor *>(mImpl.get());

        status_t finalStatus;
        *durationUs = wvmExtractor->getCachedDurationUs(&finalStatus);
        *eos = (finalStatus != OK);
        return true;
    } else if ((mDataSource->flags() & DataSource::kIsCachingDataSource)
            && getTotalBitrate(&bitrate)) {
        //assume 10s, it's useless, cedarx does caching in framework.
        *durationUs = 10 * 1000 * 1000ll;
        return true;
    }

    return false;
}

uint32_t Extractor::getSourceFlags() const {
    return mExtractorFlags;
}

void Extractor::getSourceSize(off64_t *size) const {
    mDataSource->getSize(size);
}

void Extractor::setBufferCount(int32_t count)
{
    ALOGV("setBufferCount");
    mBufferCount = count;
}

int32_t Extractor::setBuffers(void *buf, int32_t size)
{
    ALOGV("setBuffers (%p, %d)", buf, size);
    CHECK_LE(mBuffers.size(), mBufferCount);
#ifdef SECUREOS_ENABLED
    MediaBuffer *mbuf = new MediaBuffer(buf/*SecureMemAdapterGetPhysicAddressCpu(buf)*/, size);
#else
    //For test software.
    MediaBuffer *mbuf = new MediaBuffer(buf, size);
#endif
    mBuffers.push(mbuf);
    if((int32_t)mBuffers.size() == mBufferCount) {
        for (size_t i = 0; i < mImpl->countTracks(); ++i) {
            sp<MetaData> meta = mImpl->getTrackMetaData(i);
            const char *mime;
            CHECK(meta->findCString(kKeyMIMEType, &mime));
            if (!strncasecmp("video/", mime, 6)) {
                //We always surppose there is only one video track.
                mImpl->getTrack(i)->setBuffers(mBuffers);
                break;
            }
        }
    }
    return 0;
}

bool Extractor::isWidevine() const {
    return mImpl->getDrmFlag();
}

}  // namespace android
