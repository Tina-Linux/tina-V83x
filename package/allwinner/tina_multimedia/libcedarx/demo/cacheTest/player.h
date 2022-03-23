/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : player.h
 *
 * Description : fake player header file
 * History :
 *   Author  : Zhao Zhili
 *   Date    : 2016/04/25
 *   Comment : first version
 *
 */

#ifndef PLAYER_H
#define PLAYER_H

#define Player  int

inline int PlayerHasVideo(Player *p)
{
    (void)p;
    return 1;
}

inline int PlayerHasAudio(Player *p)
{
    (void)p;
    return 1;
}

inline int PlayerGetVideoBitrate(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetAudioBitrate(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetValidPictureNum(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetVideoFrameDuration(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetVideoStreamDataSize(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetAudioStreamDataSize(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetAudioPcmDataSize(Player *p)
{
    (void)p;
    return 0;
}

inline int PlayerGetAudioParam(Player* p, int* pSampleRate, int* pChannelCount, int* pBitsPerSample)
{
    (void)p;
    *pSampleRate = 0;
    *pChannelCount = 0;
    *pBitsPerSample = 0;
    return 0;
}

int64_t PlayerGetPosition(Player *p);

#endif
