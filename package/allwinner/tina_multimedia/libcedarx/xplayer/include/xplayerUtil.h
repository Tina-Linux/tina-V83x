/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : mediainfo.h
* Description : media infomation
* History :
*   Author  : PDC-PPD
*   Date    : 2018/01/23
*   Comment : first version
*
*/

#ifndef XPLAYER_UTIL_H
#define XPLAYER_UTIL_H

#include <stdlib.h>

enum AwBufferingMode {
    // Do not support buffering.
    AW_BUFFERING_MODE_NONE             = 0,
    // Support only time based buffering.
    AW_BUFFERING_MODE_TIME_ONLY        = 1,
    // Support only size based buffering.
    AW_BUFFERING_MODE_SIZE_ONLY        = 2,
    // Support both time and size based buffering, time based calculation precedes size based.
    // Size based calculation will be used only when time information is not available for
    // the stream.
    AW_BUFFERING_MODE_TIME_THEN_SIZE   = 3,
    // Number of modes.
    AW_BUFFERING_MODE_COUNT            = 4,
};

typedef struct AwBufferingSettings_t {
    int kNoWatermark;

    enum AwBufferingMode mInitialBufferingMode;  // for prepare
    enum AwBufferingMode mRebufferingMode;  // for playback

    int mInitialWatermarkMs;  // time based
    int mInitialWatermarkKB;  // size based

    // When cached data is below this mark, playback will be paused for buffering
    // till data reach |mRebufferingWatermarkHighMs| or end of stream.
    int mRebufferingWatermarkLowMs;
    // When cached data is above this mark, buffering will be paused.
    int mRebufferingWatermarkHighMs;

    // When cached data is below this mark, playback will be paused for buffering
    // till data reach |mRebufferingWatermarkHighKB| or end of stream.
    int mRebufferingWatermarkLowKB;
    // When cached data is above this mark, buffering will be paused.
    int mRebufferingWatermarkHighKB;

}AwBufferingSettings;

#endif
