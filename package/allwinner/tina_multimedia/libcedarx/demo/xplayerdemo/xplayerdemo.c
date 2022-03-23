/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : xplayerdemo.c
 * Description : xplayerdemo in linux, H3-tv2next
 *               video write to DE, audio write with alsa
 * History :
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>

#include "iniparserapi.h"

#include "cdx_config.h"
#include <cdx_log.h>
#include "xplayer.h"
#include "CdxTypes.h"
#include "memoryAdapter.h"
#include "deinterlace.h"
//typedef unsigned long uintptr_t ;
extern LayerCtrl* LayerCreate();
extern SoundCtrl* TinaSoundDeviceInit();
//extern SoundCtrl* SoundDeviceCreate();

extern SubCtrl* SubtitleCreate();
extern Deinterlace* DeinterlaceCreate();
extern LayerCtrl* LayerCreate_DE();

static const int STATUS_STOPPED   = 0;
static const int STATUS_PREPARING = 1;
static const int STATUS_PREPARED  = 2;
static const int STATUS_PLAYING   = 3;
static const int STATUS_PAUSED    = 4;
static const int STATUS_SEEKING   = 5;

typedef struct DemoPlayerContext
{
    XPlayer*       mAwPlayer;
    int             mPreStatus;
    int             mStatus;
    int             mSeekable;
    int             mError;
    pthread_mutex_t mMutex;
    int             mVideoFrameNum;
}DemoPlayerContext;


//* define commands for user control.
typedef struct Command
{
    const char* strCommand;
    int         nCommandId;
    const char* strHelpMsg;
}Command;

#define COMMAND_HELP            0x1     //* show help message.
#define COMMAND_QUIT            0x2     //* quit this program.

#define COMMAND_SET_SOURCE      0x101   //* set url of media file.
#define COMMAND_PLAY            0x102   //* start playback.
#define COMMAND_PAUSE           0x103   //* pause the playback.
#define COMMAND_STOP            0x104   //* stop the playback.
#define COMMAND_SEEKTO          0x105   //* seek to posion, in unit of second.
#define COMMAND_SHOW_MEDIAINFO  0x106   //* show media information.
#define COMMAND_SHOW_DURATION   0x107   //* show media duration, in unit of second.
#define COMMAND_SHOW_POSITION   0x108   //* show current play position, in unit of second.
#define COMMAND_SWITCH_AUDIO    0x109   //* switch autio track.
#define COMMAND_SETSPEED        0x10a


static const Command commands[] =
{
    {"help",            COMMAND_HELP,               "show this help message."},
    {"quit",            COMMAND_QUIT,               "quit this program."},
    {"set url",         COMMAND_SET_SOURCE,
        "set url of the media, for example, set url: ~/testfile.mkv."},
    {"play",            COMMAND_PLAY,               "start playback."},
    {"pause",           COMMAND_PAUSE,              "pause the playback."},
    {"stop",            COMMAND_STOP,               "stop the playback."},
    {"set speed",       COMMAND_SETSPEED,      "stop the playback."},
    {"seek to",         COMMAND_SEEKTO,
        "seek to specific position to play, position is in unit of second, ex, seek to: 100."},
    {"show media info", COMMAND_SHOW_MEDIAINFO,  "show media information of the media file."},
    {"show duration",   COMMAND_SHOW_DURATION,   "show duration of the media file."},
    {"show position",   COMMAND_SHOW_POSITION,   "show current play position,in unit of second."},
    {"switch audio",    COMMAND_SWITCH_AUDIO,
        "switch audio to a track, for example, switch audio: 2, track is start counting from 0."},
    {NULL, 0, NULL}
};

static void showHelp(void)
{
    int     i;

    printf("\n");
    printf("******************************************************************************\n");
    printf("* This is a simple media player, when it is started, you can input commands to tell\n");
    printf("* what you want it to do.\n");
    printf("* Usage: \n");
    printf("*   # ./demoPlayer\n");
    printf("*   # set url: http://www.allwinner.com/ald/al3/testvideo1.mp4\n");
    printf("*   # show media info\n");
    printf("*   # play\n");
    printf("*   # pause\n");
    printf("*   # stop\n");
    printf("*\n");
    printf("* Command and it param is seperated by a colon, param is optional, as below:\n");
    printf("*     Command[: Param]\n");
    printf("*\n");
    printf("* here are the commands supported:\n");

    for(i=0; ; i++)
    {
        if(commands[i].strCommand == NULL)
            break;
        printf("*    %s:\n", commands[i].strCommand);
        printf("*\t\t%s\n",  commands[i].strHelpMsg);
    }
    printf("*\n");
    printf("********************************************************\n");
}

static int readCommand(char* strCommandLine, int nMaxLineSize)
{
    int            nMaxFds;
    fd_set         readFdSet;
    int            result;
    char*          p;
    unsigned int   nReadBytes;

    printf("\ndemoPlayer# ");
    fflush(stdout);

    nMaxFds    = 0;
    FD_ZERO(&readFdSet);
    FD_SET(STDIN_FILENO, &readFdSet);

    result = select(nMaxFds+1, &readFdSet, NULL, NULL, NULL);
    if(result > 0)
    {
        if(FD_ISSET(STDIN_FILENO, &readFdSet))
        {
            nReadBytes = read(STDIN_FILENO, &strCommandLine[0], nMaxLineSize);
            if(nReadBytes > 0)
            {
                p = strCommandLine;
                while(*p != 0)
                {
                    if(*p == 0xa)
                    {
                        *p = 0;
                        break;
                    }
                    p++;
                }
            }

            return 0;
        }
    }

    return -1;
}

static void formatString(char* strIn)
{
    char* ptrIn;
    char* ptrOut;
    int   len;
    int   i;

    if(strIn == NULL || (len=strlen(strIn)) == 0)
        return;

    ptrIn  = strIn;
    ptrOut = strIn;
    i      = 0;
    while(*ptrIn != '\0')
    {
        //* skip the beginning space or multiple space between words.
        if(*ptrIn != ' ' || (i!=0 && *(ptrOut-1)!=' '))
        {
            *ptrOut++ = *ptrIn++;
            i++;
        }
        else
            ptrIn++;
    }

    //* skip the space at the tail.
    if(i==0 || *(ptrOut-1) != ' ')
        *ptrOut = '\0';
    else
        *(ptrOut-1) = '\0';

    return;
}


//* return command id,
static int parseCommandLine(char* strCommandLine, int* pParam)
{
    char* strCommand;
    char* strParam;
    int   i;
    int   nCommandId;
    char  colon = ':';

    if(strCommandLine == NULL || strlen(strCommandLine) == 0)
    {
        return -1;
    }

    strCommand = strCommandLine;
    strParam   = strchr(strCommandLine, colon);
    if(strParam != NULL)
    {
        *strParam = '\0';
        strParam++;
    }

    formatString(strCommand);
    formatString(strParam);

    nCommandId = -1;
    for(i=0; commands[i].strCommand != NULL; i++)
    {
        if(strcmp(commands[i].strCommand, strCommand) == 0)
        {
            nCommandId = commands[i].nCommandId;
            break;
        }
    }

    if(commands[i].strCommand == NULL)
        return -1;

    switch(nCommandId)
    {
        case COMMAND_SET_SOURCE:
            if(strParam != NULL && strlen(strParam) > 0)
                *pParam = (uintptr_t)strParam;        //* pointer to the url.
            else
            {
                printf("no url specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SEEKTO:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);  //* seek time in unit of second.
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("seek time is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("no seek time is specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SETSPEED:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);  //* seek speed.
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("seek time is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("no seek time is specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SWITCH_AUDIO:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("audio stream index is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("no audio stream index is specified.\n");
                nCommandId = -1;
            }
            break;

        default:
            break;
    }


    return nCommandId;
}


//* a callback for awplayer.
int CallbackForAwPlayer(void* pUserData, int msg, int ext1, void* param)
{
    DemoPlayerContext* pDemoPlayer = (DemoPlayerContext*)pUserData;

    switch(msg)
    {
        case AWPLAYER_MEDIA_INFO:
        {
            switch(ext1)
            {
                case AW_MEDIA_INFO_NOT_SEEKABLE:
                {
                    pDemoPlayer->mSeekable = 0;
                    printf("info: media source is unseekable.\n");
                    break;
                }
                case AW_MEDIA_INFO_RENDERING_START:
                {
                    printf("info: start to show pictures.\n");
                    break;
                }
            }
            break;
        }

        case AWPLAYER_MEDIA_ERROR:
        {
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mStatus = STATUS_STOPPED;
            pDemoPlayer->mPreStatus = STATUS_STOPPED;
            printf("error: open media source fail.\n");
            pthread_mutex_unlock(&pDemoPlayer->mMutex);
            pDemoPlayer->mError = 1;

            loge(" error : how to deal with it");
            break;
        }

        case AWPLAYER_MEDIA_PREPARED:
        {
            logd("info : preared");
            pDemoPlayer->mPreStatus = pDemoPlayer->mStatus;
            pDemoPlayer->mStatus = STATUS_PREPARED;
            printf("info: prepare ok.\n");
            break;
        }

        case AWPLAYER_MEDIA_BUFFERING_UPDATE:
        {
            int nTotalPercentage;
            int nBufferPercentage;
            int nLoadingPercentage;

            nTotalPercentage   = ((int*)param)[0];   //* read positon to total file size.
            nBufferPercentage  = ((int*)param)[1];   //* cache buffer fullness.
            nLoadingPercentage = ((int*)param)[2];   //* loading percentage to start play.

            int nBufferedFilePos;
            int nBufferFullness;

            break;
        }

        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
        {
            //* stop the player.
            //* TODO
            break;
        }

        case AWPLAYER_MEDIA_SEEK_COMPLETE:
        {
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mStatus = pDemoPlayer->mPreStatus;
            printf("info: seek ok.\n");
            pthread_mutex_unlock(&pDemoPlayer->mMutex);
            break;
        }
        case AWPLAYER_MEDIA_SET_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param)[0];   //* read positon to total file size.
            h  = ((int*)param)[1];   //* cache buffer fullness.
            printf("++++ video width: %d, height: %d \n", w, h);
            break;
        }

        default:
        {
            //printf("warning: unknown callback from AwPlayer.\n");
            break;
        }
    }

    return 0;
}


//* the main method.
int main(int argc, char** argv)
{
    DemoPlayerContext demoPlayer;
    int  nCommandId;
    int  nCommandParam;
    int  bQuit;
    char strCommandLine[1024];

    CEDARX_UNUSE(argc);
    CEDARX_UNUSE(argv);

    printf("\n");
    printf("*****************************************************************************\n");
    printf("* This program implements a simple player, \n");
    printf("* you can type commands to control the player. \n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("****************************************************************************\n");

    //* create a player.
    memset(&demoPlayer, 0, sizeof(DemoPlayerContext));
    pthread_mutex_init(&demoPlayer.mMutex, NULL);
    demoPlayer.mAwPlayer = XPlayerCreate();
    if(demoPlayer.mAwPlayer == NULL)
    {
        printf("can not create AwPlayer, quit.\n");
        exit(-1);
    }

    //* set callback to player.
    XPlayerSetNotifyCallback(demoPlayer.mAwPlayer, CallbackForAwPlayer, (void*)&demoPlayer);

    //* check if the player work.
    if(XPlayerInitCheck(demoPlayer.mAwPlayer) != 0)
    {
        printf("initCheck of the player fail, quit.\n");
        XPlayerDestroy(demoPlayer.mAwPlayer);
        demoPlayer.mAwPlayer = NULL;
        exit(-1);
    }

    //LayerCtrl* layer = LayerCreate();
    LayerCtrl* layer = LayerCreate_DE();
    SoundCtrl* sound = TinaSoundDeviceInit();
    //SoundCtrl* sound = SoundDeviceCreate();
    SubCtrl*   sub   = SubtitleCreate();
    Deinterlace* di = DeinterlaceCreate();

    XPlayerSetAudioSink(demoPlayer.mAwPlayer, (void*)sound);
    XPlayerSetVideoSurfaceTexture(demoPlayer.mAwPlayer, (void*)layer);
    XPlayerSetSubCtrl(demoPlayer.mAwPlayer, sub);
    XPlayerSetDeinterlace(demoPlayer.mAwPlayer, di);

    //* read, parse and process command from user.
    bQuit = 0;
    while(!bQuit)
    {
        if(demoPlayer.mError)
        {
            XPlayerReset(demoPlayer.mAwPlayer);
            demoPlayer.mError = 0;

            demoPlayer.mPreStatus = STATUS_PREPARED;
            demoPlayer.mStatus    = STATUS_STOPPED;
        }

        //* read command from stdin.
        if(readCommand(strCommandLine, sizeof(strCommandLine)) == 0)
        {
            //* parse command.
            nCommandParam = 0;
            nCommandId = parseCommandLine(strCommandLine, &nCommandParam);

            //* process command.
            switch(nCommandId)
            {
                case COMMAND_HELP:
                {
                    showHelp();
                    break;
                }

                case COMMAND_QUIT:
                {
                    bQuit = 1;
                    break;
                }

                case COMMAND_SET_SOURCE :   //* set url of media file.
                {
                    char* pUrl;
                    pUrl = (char*)(uintptr_t)nCommandParam;

                    demoPlayer.mSeekable = 1;

                    //* set url to the AwPlayer.
                    if(XPlayerSetDataSourceUrl(demoPlayer.mAwPlayer,
                                 (const char*)pUrl, NULL, NULL) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::setDataSource() return fail.\n");
                        break;
                    }
                     printf("setDataSource end\n");

                    //* start preparing.
                    pthread_mutex_lock(&demoPlayer.mMutex);
                    if(XPlayerPrepareAsync(demoPlayer.mAwPlayer) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::prepareAsync() return fail.\n");
                        pthread_mutex_unlock(&demoPlayer.mMutex);
                        break;
                    }


                    demoPlayer.mPreStatus = STATUS_STOPPED;
                    demoPlayer.mStatus    = STATUS_PREPARING;
                    printf("preparing...\n");
                    pthread_mutex_unlock(&demoPlayer.mMutex);

                    break;
                }

                case COMMAND_PLAY:   //* start playback.
                {
                    if(XPlayerStart(demoPlayer.mAwPlayer) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::start() return fail.\n");
                        break;
                    }
                    demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus    = STATUS_PLAYING;
                    printf("playing.\n");
                    break;
                }

                case COMMAND_PAUSE:   //* pause the playback.
                {
                    if(XPlayerPause(demoPlayer.mAwPlayer) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::pause() return fail.\n");
                        break;
                    }
                    demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus    = STATUS_PAUSED;
                    printf("paused.\n");

                    break;
                }

                case COMMAND_STOP:   //* stop the playback.
                {
                    if(XPlayerReset(demoPlayer.mAwPlayer) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::reset() return fail.\n");
                        break;
                    }
                    demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus    = STATUS_STOPPED;
                    printf("stopped.\n");
                    break;
                }

                case COMMAND_SEEKTO:   //* seek to posion, in unit of second.
                {
                    int nSeekTimeMs;
                    int nDuration;
                    nSeekTimeMs = nCommandParam*1000;

                    if(XPlayerGetDuration(demoPlayer.mAwPlayer, &nDuration) != 0)
                    {
                        printf("getDuration fail, unable to seek!\n");
                        break;
                    }

                    if(nSeekTimeMs > nDuration)
                    {
                        printf("seek time out of range, media duration = %d seconds.\n",
                                nDuration/1000);
                        break;
                    }

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    //* the player will keep the pauded status
                    //* and pause the playback after seek finish.
                    pthread_mutex_lock(&demoPlayer.mMutex);
                    XPlayerSeekTo(demoPlayer.mAwPlayer, nSeekTimeMs,0);
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus = STATUS_SEEKING;
                    pthread_mutex_unlock(&demoPlayer.mMutex);
                    break;
                }

                case COMMAND_SETSPEED:   //* set speed
                {
                    int nSpeed;
                    nSpeed = nCommandParam;

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    XPlayerSetSpeed(demoPlayer.mAwPlayer, nSpeed);
                    logd("===  set speed end");
                    break;
                }

                case COMMAND_SHOW_MEDIAINFO:   //* show media information.
                {
                    printf("show media information.\n");
                    break;
                }

                case COMMAND_SHOW_DURATION:   //* show media duration, in unit of second.
                {
                    int nDuration = 0;
                    if(XPlayerGetDuration(demoPlayer.mAwPlayer, &nDuration) == 0)
                        printf("media duration = %d seconds.\n", nDuration/1000);
                    else
                        printf("fail to get media duration.\n");
                    break;
                }

                case COMMAND_SHOW_POSITION:   //* show current play position, in unit of second.
                {
                    int nPosition = 0;
                    if(XPlayerGetCurrentPosition(demoPlayer.mAwPlayer, &nPosition) == 0)
                        printf("current position = %d seconds.\n", nPosition/1000);
                    else
                        printf("fail to get pisition.\n");
                    break;
                }

                case COMMAND_SWITCH_AUDIO:   //* switch autio track.
                {
                    int nAudioStreamIndex;
                    nAudioStreamIndex = nCommandParam;
                    printf("switch audio to the %dth track.\n", nAudioStreamIndex);
                    //* TODO
                    break;
                }

                default:
                {
                    if(strlen(strCommandLine) > 0)
                        printf("invalid command.\n");
                    break;
                }
            }
        }
    }

    printf("destroy AwPlayer.\n");

    if(demoPlayer.mAwPlayer != NULL)
    {
        XPlayerDestroy(demoPlayer.mAwPlayer);
        demoPlayer.mAwPlayer = NULL;
    }

    printf("destroy AwPlayer 1.\n");
    pthread_mutex_destroy(&demoPlayer.mMutex);

    printf("\n");
    printf("********************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("********************************************************************\n");
    printf("\n");

    return 0;
}
