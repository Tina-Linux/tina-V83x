/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef __EDID_H__
#define __EDID_H__

#include <stdint.h>
#include <vector>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/kernel.h>

#include "../sunxi_display2.h"
#include "cea_vic.h"


#define BLOCK_LENGTH    (128)
#define EDID_MAX_LENGTH (BLOCK_LENGTH * 2)

enum video_format_e {
    _videoFmtRGB,
    _videoFmtYUV444,
    _videoFmtYUV422,
    _videoFmtYUV420,
};

enum colorimetry_e {
    XVYCC601    = 0x01,
    XVYCC709    = 0x02,
    SYCC601     = 0x04,
    ADOBE_YCC601= 0x08,
    ADOBE_RGB   = 0x10,
    BT2020_CYCC = 0x20,
    BT2020_YCC  = 0x40,
    BT2020_RGB  = 0x80,
};

enum eotf_e {
    EOTF_SDR_LUMINANCE_RANGE = (1UL << 0),
    EOTF_HDR_LUMINANCE_RANGE = (1UL << 1),
    EOTF_SMPTE_ST_2084       = (1UL << 2),
    EOTF_FUTURE_EOTF         = (1UL << 3),
};

enum sink_type_e {
    SINK_TYPE_DVI  = 1,
    SINK_TYPE_HDMI = 2,
};

class EdidParser {
public:
    EdidParser(const char *path);
   ~EdidParser();

    int reload();
    void dump();

    friend class EdidStrategy;
public:
    struct videoFormat {
        int format;
        int dc_30bit;
        int dc_36bit;
        int dc_48bit;
        const char *name;

        videoFormat() :format(0), dc_30bit(0),
            dc_36bit(0), dc_48bit(0), name(0)
        {}
    };

    struct videoInformationCode {
        int vic;
        int native;
        int regular_sampling;
        int ycbcr420_sampling;
        const char *name;

        videoInformationCode(int v, int f, int r, int y, const char *n)
          : vic(v), native(f),
            regular_sampling(r), ycbcr420_sampling(y), name(n)
        {}
    };

    void clear();
    int parse(const char *path);
    int parseMonitorInfo();
    int parseStandardTiming();
    int parseDetailedTiming();
    int parseDescriptorBlock(size_t begin);
    int parseCeaExtensionBlock();

    void parseCeaDataBlocks(const unsigned char *x);
    void AudioDataBlock(const unsigned char *x);
    void videoDataBlock(const unsigned char *x);
    void vendorSpecificDataBlock(const unsigned char *x);
    void ceaSvd(const unsigned char *x, int n, int r, int y, std::vector<videoInformationCode *>& out);
    videoFormat *getVideoFormat(int format);
    void hdmi1p4VSDB(const unsigned char *x);
    void hdmi4k2kVICParse(const unsigned char *x, int size);
    void hdmi2p0VSDB(const unsigned char *x);

    void YCbCr420VideoDataBlock(const unsigned char *x);
    void updateYCbCr420CapabilityVIC();
    void YCbCr420CapabilityMapDataBlock(const unsigned char *x);

    void colorimetryDataBlock(const unsigned char *x);
    void hdrStaticMetadataBlock(const unsigned char *x);

    void getVideoFormatSupported();

    const char *mPath;
    unsigned char edid[EDID_MAX_LENGTH] = {0};

    /* Monitor Info from base block */
    char mMonitorName[16];
    char mVendorName[4];
    uint16_t mProductId;
    uint32_t mSerialNumber;
    uint16_t mManufactureWeek;
    uint16_t mManufactureYear;
    uint16_t mVersion;

    /* Horizontal & Vertical Display size in centimetres. */
    uint16_t mDisplaySizeH;
    uint16_t mDisplaySizeV;

    /* MaxAudioPcmChannels */
    uint16_t mMaxAudioPcmChs;
    int mPreferredVideoFormat;
    int VideoFormatSupported[HDMI_RESERVED];

    std::vector<int> mStandardTiming;
    std::vector<char *> mDetailedTiming;

    int mExtensionBlock;

    /* HDMI 1.4 VSDB */
    int mMaxTmdsCharacterRate1p4;
    uint8_t mColorimetry;
    int m3Dpresent;
    int mRGBOnly;
    int mSinkType;
    std::vector<videoInformationCode *> mHdmi4k2kVIC;

    /* HDMI 2.0 HF-VSDB */
    int mMaxTmdsCharacterRate2p0;
    int mY420SamplingAny;

    /* HDR static metadata */
    uint8_t mEotf;
    uint8_t mMetadataDescriptor;
    uint8_t mMaxLuminance;
    uint8_t mMaxFrameAverageLuminance;
    uint8_t mMinLuminance;

    std::vector<int> mYCbCr420CapabilityMap;
    std::vector<videoInformationCode *> mSupportedVIC;
    std::vector<videoInformationCode *> mY420VIC;
    std::map<int, videoFormat *> mSupportedFormat;
};

#endif
