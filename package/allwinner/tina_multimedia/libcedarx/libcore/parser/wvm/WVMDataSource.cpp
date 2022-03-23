/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#define LOG_NDEBUG 0
#define LOG_TAG "WVMDataSource"
#include <utils/Log.h>
#include <media/stagefright/foundation/ADebug.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "WVMDataSource.h"

namespace android {

WVMDataSource::WVMDataSource(CdxStreamT *stream)
    : mStream(stream),
      mDecryptHandle(NULL),
      mDrmManagerClient(NULL) {
    CHECK(mStream != NULL);

    char *uri = NULL;
    mStream->ops->getMetaData(mStream, "uri", (void**)&uri);
    mURI.setTo(uri);
}

WVMDataSource::~WVMDataSource() {

    if (mDecryptHandle != NULL) {
        // To release mDecryptHandle
        CHECK(mDrmManagerClient);
        mDrmManagerClient->closeDecryptSession(mDecryptHandle);
        mDecryptHandle = NULL;
    }

    if (mDrmManagerClient != NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }
}

status_t WVMDataSource::initCheck() const {
    return (mURI.startsWith("fd://")
            || mURI.startsWith("http://")
            || mURI.startsWith("widevine://")) ? OK : NO_INIT;
}

ssize_t WVMDataSource::readAt(off64_t offset, void *data, size_t size) {
    if(mStream->ops->tell(mStream) != offset) {
        if(CdxStreamSeek(mStream, offset, SEEK_SET)) {
            //seek failed.
            ALOGE("seek to %lld failed", offset);
            return -1;
        }
    }
    return CdxStreamRead(mStream, data, size);
}

status_t WVMDataSource::getSize(off64_t *size) {
    *size = CdxStreamSize(mStream);
    return *size != -1;
}

sp<DecryptHandle> WVMDataSource::DrmInitialization(const char *mime) {
    if (mDrmManagerClient == NULL) {
        mDrmManagerClient = new DrmManagerClient();
    }

    if (mDrmManagerClient == NULL) {
        return NULL;
    }

    if (mDecryptHandle == NULL) {
        if(!strncasecmp(mURI.c_str(), "fd://", 5)) {
            //local file.
            int fd = -1;
            off64_t offset = 0;
            off64_t length = 0;
            sscanf(mURI.c_str(), "fd://%d?offset=%lld&length=%lld",
                   &fd, (long long *)&offset, (long long *)&length);
            if(fd < 0) {
                ALOGE("invalid file descriptor");
                return NULL;
            }

            mDecryptHandle = mDrmManagerClient->openDecryptSession(
                    fd, offset, length, mime);
        } else {
            //http source.
            CHECK(!mURI.empty());
            const char *uri;
            String8 tmp;
            if(!strncasecmp("widevine://", mURI.c_str(), 11)) {
                tmp = String8("http://");
                tmp.append(mURI.c_str() + 11);
                uri = tmp.string();
            } else if(!strncasecmp("http://", mURI.c_str(), 7)
                    || !strncasecmp("https://", mURI.c_str(), 8)) {
                uri = mURI.c_str();
            } else {
                ALOGE("invalid uri:%s",  mURI.c_str());
                return NULL;
            }
            mDecryptHandle = mDrmManagerClient->openDecryptSession(
                    uri, mime);
        }
    }

    if (mDecryptHandle == NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }

    return mDecryptHandle;
}

void WVMDataSource::getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client) {
    handle = mDecryptHandle;

    *client = mDrmManagerClient;
}

String8 WVMDataSource::getUri() {
    Mutex::Autolock autoLock(mLock);
    const char *uri = mURI.c_str();
    if(!strncasecmp("widevine://", mURI.c_str(), 11)) {
        String8 tmp;
        tmp = String8("http://");
        tmp.append(mURI.c_str() + 11);
        uri = tmp.string();
    }
    return String8(uri);
}

String8 WVMDataSource::getMIMEType() const {
    return String8("");
}

status_t WVMDataSource::reconnectAtOffset(off64_t offset) {
    if(mStream->ops->tell(mStream) != offset) {
        if(CdxStreamSeek(mStream, offset, SEEK_SET)) {
            //seek failed.
            return -1;
        }
    }
    return OK;
}

}  // namespace android
