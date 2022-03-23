/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CTC_MediaProcessor.h
 * Description : CTC_MediaProcessor
 * History :
 *
 */

#ifndef _CTC_MEDIAPROCESSOR_H_
#define _CTC_MEDIAPROCESSOR_H_

extern "C" {
#include "vformat.h"
#include "aformat.h"
}

#include <gui/Surface.h>

using namespace android;
#define NewInterface (1)

/**
 * @视频参数
 */
typedef struct {
    unsigned short    pid;//pid
    int                nVideoWidth;//视频宽度
    int                nVideoHeight;//视频高度
    int                nFrameRate;//帧率
    vformat_t        vFmt;//视频格式
    unsigned long    cFmt;//编码格式
}VIDEO_PARA_T, *PVIDEO_PARA_T;

/**
 * @音频参数
 */
typedef struct {
    unsigned short    pid;            //audio pid
    int                nChannels;        //声道数
    int                nSampleRate;    //采样率
#if !NewInterface
    unsigned short    block_align;    //block align
    int             bit_per_sample;    //比特率
#endif
    aformat_t        aFmt;            //音频格式
    int                nExtraSize;
    unsigned char*    pExtraData;
}AUDIO_PARA_T, *PAUDIO_PARA_T;

/**
 * @视频显示模式
 */
typedef enum {
    IPTV_PLAYER_CONTENTMODE_NULL        = -1,
    IPTV_PLAYER_CONTENTMODE_LETTERBOX,            //源比例输出
    IPTV_PLAYER_CONTENTMODE_FULL,                //全屏输出
}IPTV_PLAYER_CONTENTMODE_e;

/**
 * @数据类型
 */
typedef enum {
    IPTV_PLAYER_STREAMTYPE_NULL        = -1,
    IPTV_PLAYER_STREAMTYPE_TS,                    //TS数据
    IPTV_PLAYER_STREAMTYPE_VIDEO,                //ES Video数据
    IPTV_PLAYER_STREAMTYPE_AUDIO,                //ES Audio数据
}IPTV_PLAYER_STREAMTYPE_e;

/**
 * @播放状态
 */
typedef enum {
    IPTV_PLAYER_STATE_OTHER            = -1,
    IPTV_PLAYER_STATE_PLAY,                        //Play State
    IPTV_PLAYER_STATE_PAUSE,                    //Pause State
    IPTV_PLAYER_STATE_STOP,                        //Stop State
}IPTV_PLAYER_STATE_e;

/**
 * @事件类型
 */
typedef enum {
    IPTV_PLAYER_EVT_FIRST_PTS,                    //解出第一帧
    IPTV_PLAYER_EVT_ABEND,                        //异常终止
    IPTV_PLAYER_EVT_MAX,
}IPTV_PLAYER_EVT_e;

/**
 * @brief                 事件回调处理函数指针
 * @param evt             事件类型
 * @param handler           用户句柄
 * @param param1         用户参数
 * @param param2         用户参数
 **/
typedef void (*IPTV_PLAYER_EVT_CB)(IPTV_PLAYER_EVT_e evt, void *handler,
                        unsigned long param1, unsigned long param2);

/**
 * @CTC_Mediaprocessor
 */
class CTC_MediaProcessor{
public:
    CTC_MediaProcessor(){}
    virtual ~CTC_MediaProcessor(){}
public:
    //取得播放模式
    virtual int  GetPlayMode()=0;
    //显示窗口
    virtual int  SetVideoWindow(int x,int y,int width,int height)=0;
    //显示视频
    virtual int  VideoShow(void)=0;
    //隐藏视频
    virtual int  VideoHide(void)=0;
    /**
     * @brief    初始化视频参数
     * @param    pVideoPara - 视频相关参数，采用ES方式播放时，pid=0xffff;
     *                        没有video的情况下，vFmt=VFORMAT_UNKNOWN
     * @return    无
     **/
    virtual void InitVideo(PVIDEO_PARA_T pVideoPara) = 0;

    /**
     * @brief    初始化音频参数
     * @param    pAudioPara - 音频相关参数，采用ES方式播放时，pid=0xffff;
     *                         没有audio的情况下，aFmt=AFORMAT_UNKNOWN
     * @return    无
     **/
    virtual void InitAudio(PAUDIO_PARA_T pAudioPara) = 0;

    //开始播放
    virtual bool StartPlay()=0;
#if NewInterface
    //把ts流写入
    virtual int WriteData(unsigned char* pBuffer, unsigned int nSize) = 0;
#else
    /**
     * @brief    获取nSize大小的buffer
     * @param    type    - ts/video/audio
     *             pBuffer - buffer地址，如果有nSize大小的空间，则将置为对应的地址;如果不足nSize，置NULL
     *             nSize   - 需要的buffer大小
     * @return    0  - 成功
     *             -1 - 失败
     **/
    virtual int    GetWriteBuffer(IPTV_PLAYER_STREAMTYPE_e type,
                    unsigned char** pBuffer, unsigned int *nSize) = 0;

    /**
     * @brief    写入数据，和GetWriteBuffer对应使用
     * @param    type      - ts/video/audio
     *             pBuffer   - GetWriteBuffer得到的buffer地址
     *             nSize     - 写入的数据大小
     *             timestamp - ES Video/Audio对应的时间戳(90KHZ)
     * @return    0  - 成功
     *             -1 - 失败
     **/

    virtual int WriteData(IPTV_PLAYER_STREAMTYPE_e type, unsigned char* pBuffer,
                        unsigned int nSize, uint64_t timestamp) = 0;
#endif
    //暂停
    virtual bool Pause()=0;
    //继续播放
    virtual bool Resume()=0;
    //快进快退
    virtual bool Fast()=0;
    //停止快进快退
    virtual bool StopFast()=0;
    //停止
    virtual bool Stop()=0;
    //定位
    virtual bool Seek()=0;
    //设定音量
    virtual bool SetVolume(int volume)=0;
    //获取音量
    virtual int GetVolume()=0;
#if NewInterface
    //设定视频显示比例
    virtual bool SetRatio(int nRatio)=0;
#else
    /**
     * @brief    设置画面显示比例
     * @param    contentMode - 源比例/全屏，默认全屏
     * @return    无
     **/
    virtual void SetContentMode(IPTV_PLAYER_CONTENTMODE_e contentMode) = 0;
#endif
    /**
     * @brief    获取当前声道
     * @param    无
     * @return    1 - left channel
     *             2 - right channel
     *             3 - stereo
     **/
    virtual int GetAudioBalance() = 0;

    /**
     * @brief    设置播放声道
     * @param    nAudioBanlance - 1.left channel;2.right channle;3.stereo
     * @return    true  - 成功
     *             false - 失败
     **/
    virtual bool SetAudioBalance(int nAudioBalance) = 0;
    //获取视频分辩率
    virtual void GetVideoPixels(int& width, int& height)=0;
    virtual bool IsSoftFit()=0;
    virtual void SetEPGSize(int w, int h)=0;
    virtual void SetSurface(Surface *pSurface)=0;

#if NewInterface
    virtual void SwitchAudioTrack(int pid)=0;
#else
    virtual void SwitchAudioTrack(int pid, PAUDIO_PARA_T pAudioPara)=0;
#endif
    //选择字幕，pid:当前选择的字幕pid，如果有字幕，这个接口在StartPlay之前也会调用一次。
    virtual void SwitchSubtitle(int pid)=0;
    //音视频的相关设置
    virtual void SetProperty(int nType, int nSub, int nValue)=0;
    //获取当前播放时间(单位:毫秒)
    virtual long GetCurrentPlayTime()=0;
    //离开频道，在离开频道时调用该接口，如果不想在进入下一个频道再次Start，
    //可以在这里记录下，然后在StartPlay接口中继续播放就好
    virtual void leaveChannel()=0;
    virtual void playerback_register_evt_cb(IPTV_PLAYER_EVT_CB pfunc, void *handler) = 0;
    virtual int  GetAbendNum()=0;

#if !NewInterface
    virtual int GetBufferStatus(long *totalsize, long *datasize) = 0;

    /**
     * @brief    播放结束时是否保留最后一帧
     * @param    bHoldLastPic - true/false停止播放时是否保留最后一帧
     * @return    无
     **/
    virtual void SetStopMode(bool bHoldLastPic ) = 0;
#endif
};

CTC_MediaProcessor* GetMediaProcessor();
int GetMediaProcessorVersion();

#endif
