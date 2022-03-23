/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awmetadataretriever.h
 * Description : metadataretriever
 * History :
 *
 */

#ifndef AW_METADATA_RETRIEVER
#define AW_METADATA_RETRIEVER

#include <utils/String8.h>
#include <media/MediaMetadataRetrieverInterface.h>
#include <media/stagefright/MetaData.h>
#include <utils/KeyedVector.h>
#include <media/MediaPlayerInterface.h>

using namespace android;

struct AwMetadataRetriever : public MediaMetadataRetrieverInterface
{
    AwMetadataRetriever();
    virtual ~AwMetadataRetriever();

#if (CONF_ANDROID_MAJOR_VER >= 5)
    virtual status_t setDataSource(const sp<IMediaHTTPService> &httpService,
                const char* pUrl, const KeyedVector<String8, String8>* pHeaders);
#else
    virtual status_t setDataSource(const char* pUrl,
                const KeyedVector<String8, String8>* pHeaders);
#endif
    virtual status_t setDataSource(int fd, int64_t offset, int64_t length);

#if (((CONF_ANDROID_MAJOR_VER >= 6 && CONF_ANDROID_MAJOR_VER < 8)) || \
    ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER == 0)))
    virtual status_t setDataSource(const sp<DataSource>& source);

#elif ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER > 0))
    virtual status_t setDataSource(const sp<DataSource>& source, const char *mime);
#endif

    /* option enumerate is defined in
     * 'android/framework/base/media/java/android/media/MediaMetadataRetriver.java'
     * option == 0: OPTION_PREVIOUS_SYNC
     * option == 1: OPTION_NEXT_SYNC
     * option == 2: OPTION_CLOSEST_SYNC
     * option == 3: OPTION_CLOSEST
     */
#if ((CONF_ANDROID_MAJOR_VER < 8) || \
        ((CONF_ANDROID_MAJOR_VER == 8) && (CONF_ANDROID_SUB_VER == 0)))
    virtual VideoFrame* getFrameAtTime(int64_t timeUs, int option);
#else
    virtual VideoFrame* getFrameAtTime(int64_t timeUs, int option, int colorFormat, bool metaOnly);
#endif

    virtual MediaAlbumArt* extractAlbumArt();
    virtual const char* extractMetadata(int keyCode);
    virtual sp<IMemory> getStreamAtTime(int64_t timeUs);

private:
    struct MetadataPriData *mPriData;

private:
    void clear();
    void storeMetadata();
    AwMetadataRetriever(const AwMetadataRetriever &);
    AwMetadataRetriever &operator=(const AwMetadataRetriever &);
};

#endif
