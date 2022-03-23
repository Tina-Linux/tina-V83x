/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : demoplayer.cpp
 * Description : demoplayer
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

#include "cdx_config.h"
#include "cdx_log.h"
#include "CTC_wrapper.h"

static const int STATUS_STOPPED   = 0;
static const int STATUS_PREPARING = 1;
static const int STATUS_PREPARED  = 2;
static const int STATUS_PLAYING   = 3;
static const int STATUS_PAUSED    = 4;
static const int STATUS_SEEKING   = 5;

typedef struct DemoPlayerContext
{
    CTC_Wrapper*       mCTC_Wrapper;
    int             mPreStatus;
    int             mStatus;
    int             mSeekable;
    pthread_mutex_t mMutex;
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
#define COMMAND_RESUME            0x10a   //* switch autio track.
#define COMMAND_HIDE            0x10b   //* switch autio track.
#define COMMAND_SETPOSITION     0x200   //* set position of video
#define COMMAND_SETRADIO        0x201   //* set radio of video
#define COMMAND_SHOW            0x202   //* show video
#define COMMAND_SETVOLUME       0x203   //* show video
#define COMMAND_GETVOLUME       0x204   //* show video
#define COMMAND_FAST            0x205   //* show video
#define COMMAND_STOPFAST        0x206   //* show video
#define COMMAND_SEEKTEST        0x207   //* test seek
#define COMMAND_FASTTEST        0x208

static const Command commands[] =
{
    {"help",            COMMAND_HELP,               "show this help message."},
    {"quit",            COMMAND_QUIT,               "quit this program."},
    {"set url",         COMMAND_SET_SOURCE,
        "set url of the media,for example, set url: ~/testfile.mkv."},
    {"play",            COMMAND_PLAY,               "start playback."},
    {"pause",           COMMAND_PAUSE,              "pause the playback."},
    {"resume",            COMMAND_RESUME,             "resume the playback after pause."},
    {"hide",            COMMAND_HIDE,                 "hide the video."},
    {"stop",            COMMAND_STOP,               "stop the playback."},
    {"seek to",         COMMAND_SEEKTO,            "seek to specific position \
                to play, position is in unit of second, for example, seek to: 100."},
    {"show media info", COMMAND_SHOW_MEDIAINFO,     "show media information of the media file."},
    {"show duration",   COMMAND_SHOW_DURATION,      "show duration of the media file."},
    {"show position",   COMMAND_SHOW_POSITION,      "show current play position(us)"},
    {"switch audio",    COMMAND_SWITCH_AUDIO,    "switch audio to a specific track"},
    {"set position",    COMMAND_SETPOSITION,      "set position of video"},
    {"set radio",       COMMAND_SETRADIO,           "set radio of video"},
    {"show",            COMMAND_SHOW,               "show video"},
    {"set volume",        COMMAND_SETVOLUME,            "set volume:n, n=0~100"},
    {"get volume",        COMMAND_GETVOLUME,            "get volume:n, n=0~100"},
    {"fast",            COMMAND_FAST,                "fast playback"},
    {"stopfast",        COMMAND_STOPFAST,            "stop fast playback"},
    {"seektest",        COMMAND_SEEKTEST,            "seek test playback"},
    {"fasttest",        COMMAND_FASTTEST,            "fast test playback"},
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
    printf("************************************************\n");
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
                *pParam = (int)strParam;        //* pointer to the url.
            else
            {
                printf("no url specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SEEKTO:
            if(strParam != NULL)
            {
                char *end;
                *pParam = (int)strtol(strParam, &end, 10);

                if (!strcmp(end, strParam) || (*end != '\0' && *end != ' '))
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
                char *end;
                *pParam = (int)strtol(strParam, &end, 10);

                if (!strcmp(end, strParam) || (*end != '\0' && *end != ' '))
                {
                    printf("switch audio is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("no audio stream index is specified.\n");
                nCommandId = -1;
            }
            break;
        case COMMAND_SETPOSITION:
            if(strParam != NULL)
            {
                char *end;
                *pParam = (int)strtol(strParam, &end, 10);

                if (!strcmp(end, strParam) || (*end != '\0' && *end != ' '))
                {
                    printf("param is not valid.\n");
                    nCommandId = -1;
                }
            }else{
                printf("no position is specified.\n");
                nCommandId = -1;
            }
            break;
        case COMMAND_SETVOLUME:
            if(strParam != NULL)
            {
                char *end;
                *pParam = (int)strtol(strParam, &end, 10);

                if (!strcmp(end, strParam) || (*end != '\0' && *end != ' '))
                {
                    printf("param is not valid.\n");
                    nCommandId = -1;
                }
            }else{
                printf("no volume is specified.\n");
                nCommandId = -1;
            }
            break;
        case COMMAND_SETRADIO:
            if(strParam != NULL)
            {
                char *end;
                *pParam = (int)strtol(strParam, &end, 10);

                if (!strcmp(end, strParam) || (*end != '\0' && *end != ' '))
                {
                    printf("radio param is not valid.\n");
                    nCommandId = -1;
                }
            }else{
                printf("no radio is specified.\n");
                nCommandId = -1;
            }
            break;
        default:
            break;
    }

    return nCommandId;
}

//* a callback for awplayer.
void CallbackForCTC_Wrapper(void* pUserData, int msg, int param0, void* param1)
{
    DemoPlayerContext* pDemoPlayer = (DemoPlayerContext*)pUserData;

    CEDARX_UNUSE(param1);

    switch(msg)
    {
        case NOTIFY_NOT_SEEKABLE:
        {
            pDemoPlayer->mSeekable = 0;
            printf("info: media source is unseekable.\n");
            break;
        }

        case NOTIFY_ERROR:
        {
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mStatus = STATUS_STOPPED;
            pDemoPlayer->mPreStatus = STATUS_STOPPED;
            printf("error: open media source fail.\n");
            pthread_mutex_unlock(&pDemoPlayer->mMutex);
            break;
        }

        case NOTIFY_PREPARED:
        {
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mPreStatus = pDemoPlayer->mStatus;
            pDemoPlayer->mStatus = STATUS_PREPARED;
            printf("info: prepare ok.\n");
            pthread_mutex_unlock(&pDemoPlayer->mMutex);
            break;
        }

        case NOTIFY_BUFFERRING_UPDATE:
        {
            int nBufferedFilePos;
            int nBufferFullness;

            nBufferedFilePos = param0 & 0x0000ffff;
            nBufferFullness  = (param0>>16) & 0x0000ffff;
            printf("info: buffer %d percent of the media file, buffer fullness = %d percent.\n",
                nBufferedFilePos, nBufferFullness);

            break;
        }

        case NOTIFY_PLAYBACK_COMPLETE:
        {
            //* stop the player.
            //* TODO
            break;
        }

        case NOTIFY_RENDERING_START:
        {
            printf("info: start to show pictures.\n");
            break;
        }

        case NOTIFY_SEEK_COMPLETE:
        {
            logd("+++++++++++ NOTIFY_SEEK_COMPLETE");
            //pDemoPlayer->mStatus = pDemoPlayer->mPreStatus;
            printf("info: seek ok(%d).\n", pDemoPlayer->mStatus);

            break;
        }

        default:
        {
            printf("warning: unknown callback from CTC_Wrapper.\n");
            break;
        }
    }

    return;
}

//* the main method.
int main()
{
    DemoPlayerContext demoPlayer;
    int  nCommandId;
    int  nCommandParam;
    int  bQuit;
    char strCommandLine[1024];

    printf("\n");
    printf("**************************************************************\n");
    printf("* This program implements a simple player, you can type commands \
                to control the player.\n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("**************************************************************\n");

    //* create a player.
    memset(&demoPlayer, 0, sizeof(DemoPlayerContext));
    pthread_mutex_init(&demoPlayer.mMutex, NULL);
    demoPlayer.mCTC_Wrapper = new CTC_Wrapper();
    if(demoPlayer.mCTC_Wrapper == NULL)
    {
        printf("can not create CTC_Wrapper, quit.\n");
        exit(-1);
    }

    //* set callback to player.
    demoPlayer.mCTC_Wrapper->setNotifyCallback(CallbackForCTC_Wrapper, (void*)&demoPlayer);

    //* check if the player work.
    if(demoPlayer.mCTC_Wrapper->initCheck() != 0)
    {
        printf("initCheck of the player fail, quit.\n");
        delete demoPlayer.mCTC_Wrapper;
        demoPlayer.mCTC_Wrapper = NULL;
        exit(-1);
    }

    //* read, parse and process command from user.
    bQuit = 0;
    while(!bQuit)
    {
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
                    pUrl = (char*)nCommandParam;

                    if(demoPlayer.mStatus != STATUS_STOPPED)
                    {
                        printf("invalid command:\n");
                        printf("    play is not in stopped status.\n");
                        break;
                    }

                    demoPlayer.mSeekable = 1;

                    //* set url to the CTC_Wrapper.
                    if(demoPlayer.mCTC_Wrapper->setDataSource((const char*)pUrl, NULL) != 0)
                    {
                        printf("error:\n");
                        printf("    CTC_Wrapper::setDataSource() return fail.\n");
                        break;
                    }

                    //* start preparing.
                    pthread_mutex_lock(&demoPlayer.mMutex);
                    if(demoPlayer.mCTC_Wrapper->prepareAsync() != 0)
                    {
                        printf("error:\n");
                        printf("    CTC_Wrapper::prepareAsync() return fail.\n");
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
                    if(demoPlayer.mStatus != STATUS_PREPARED &&
                       demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        printf("invalid command:\n");
                        printf("    player is neither in prepared status nor in seeking.\n");
                        break;
                    }

                    {
                        int vWidth = 0, vHeight = 0;
                        demoPlayer.mCTC_Wrapper->getVideoPixel(vWidth, vHeight);
                        printf("video pixel is (%d, %d)\n", vWidth, vHeight);
                        demoPlayer.mCTC_Wrapper->setVideoWindow(0, 0, vWidth, vHeight);
                    }
                    //* start the playback
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        if(demoPlayer.mCTC_Wrapper->start() != 0)
                        {
                            printf("error:\n");
                            printf("    CTC_Wrapper::start() return fail.\n");
                            break;
                        }
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                        demoPlayer.mStatus    = STATUS_PLAYING;
                        printf("playing.\n");
                    }
                    else
                    {
                        //* the player will keep the started status
                        //* and start to play after seek finish.
                        demoPlayer.mCTC_Wrapper->start();
                        demoPlayer.mPreStatus = STATUS_PLAYING;

                    }
                    break;
                }

                case COMMAND_FAST:   //* start playback.
                {
                    //* the player will keep the started status
                    //* and start to play after seek finish.
                    demoPlayer.mCTC_Wrapper->fast();
                    demoPlayer.mPreStatus = STATUS_PLAYING;

                    break;
                }
                case COMMAND_STOPFAST:   //* stop the playback.
                {
                    if(demoPlayer.mCTC_Wrapper->stopFast() != 0)
                    {
                        printf("error:\n");
                        printf("    CTC_Wrapper::stop() return fail.\n");
                        break;
                    }
                    demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus    = STATUS_PLAYING;
                    printf("stopped.\n");
                    break;
                }

                case COMMAND_RESUME:   //* resume playback.
                {
                    if(demoPlayer.mStatus != STATUS_PAUSED)
                    {
                        printf("invalid command:\n");
                        printf("    player is not in paused status.(%d)\n", demoPlayer.mStatus);
                        break;
                    }

                    //* start the playback
                    if(demoPlayer.mCTC_Wrapper->resume() != 0)
                    {
                        printf("error:\n");
                        printf("    CTC_Wrapper::start() return fail.\n");
                        break;
                    }
                    demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus      = STATUS_PLAYING;
                    printf("playing.\n");
                    break;
                }

                case COMMAND_PAUSE:   //* pause the playback.
                {
                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        printf("invalid command:\n");
                        printf("player is neither in playing status nor in seeking status(%d).\n",
                                        demoPlayer.mStatus);
                        break;
                    }

                    //* pause the playback
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        if(demoPlayer.mCTC_Wrapper->pause() != 0)
                        {
                            printf("error:\n");
                            printf("    CTC_Wrapper::pause() return fail.\n");
                            break;
                        }
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                        demoPlayer.mStatus    = STATUS_PAUSED;
                        printf("paused.\n");
                    }
                    else
                    {
                        //* the player will keep the pauded status
                        //* and pause the playback after seek finish.
                        demoPlayer.mCTC_Wrapper->pause();
                        demoPlayer.mPreStatus = STATUS_PAUSED;

                    }
                    break;
                }

                case COMMAND_STOP:   //* stop the playback.
                {
                    if(demoPlayer.mCTC_Wrapper->stop() != 0)
                    {
                        printf("error:\n");
                        printf("    CTC_Wrapper::stop() return fail.\n");
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

                    printf("    CTC_Wrapper::seekto() %d\n", nSeekTimeMs);
                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING &&
                       demoPlayer.mStatus != STATUS_PAUSED  &&
                       demoPlayer.mStatus != STATUS_PREPARED)
                    {
                        printf("invalid command:\n");
                        printf("    player is not in playing/seeking/paused/prepared status.\n");
                        break;
                    }

                    if(demoPlayer.mCTC_Wrapper->getDuration(&nDuration) == 0)
                    {
                        if(nSeekTimeMs > nDuration)
                        {
                            printf("seek time out of range, media duration = %d seconds.\n",
                                        nDuration/1000);
                            break;
                        }
                    }

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    //* the player will keep the pauded status
                    //* and pause the playback after seek finish.
                    pthread_mutex_lock(&demoPlayer.mMutex);
                    demoPlayer.mCTC_Wrapper->seekTo(nSeekTimeMs);
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                    demoPlayer.mStatus = demoPlayer.mPreStatus;
                    pthread_mutex_unlock(&demoPlayer.mMutex);
                    break;
                }

                case COMMAND_SEEKTEST:   //* seek to posion, in unit of second.
                {
                    int nSeekTimeMs;
                    int nDuration;
                    int i=0;

                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING &&
                       demoPlayer.mStatus != STATUS_PAUSED  &&
                       demoPlayer.mStatus != STATUS_PREPARED)
                    {
                        printf("invalid command:\n");
                        printf("    player is not in playing/seeking/paused/prepared status.\n");
                        break;
                    }

                    if(demoPlayer.mCTC_Wrapper->getDuration(&nDuration) < 0)
                    {
                        printf("get duration failed ");
                        break;
                    }

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    for(i=0; i<100; i++)
                    {
                        nSeekTimeMs = random() % nDuration;
                        printf("seektest    CTC_Wrapper::seekto() %d\n", nSeekTimeMs);

                        //* the player will keep the pauded status
                        //* and pause the playback after seek finish.
                        pthread_mutex_lock(&demoPlayer.mMutex);
                        demoPlayer.mCTC_Wrapper->seekTo(nSeekTimeMs);
                        if(demoPlayer.mStatus != STATUS_SEEKING)
                            demoPlayer.mPreStatus = demoPlayer.mStatus;
                        demoPlayer.mStatus = demoPlayer.mPreStatus;
                        pthread_mutex_unlock(&demoPlayer.mMutex);

                        usleep(10*1000);
                    }

                    break;
                }

                case COMMAND_FASTTEST:   //* start playback.
                {
                    int i;
                    int nSeekTimeMs;
                    int nDuration;
                    if(demoPlayer.mCTC_Wrapper->getDuration(&nDuration) < 0)
                    {
                        printf("get duration failed ");
                        break;
                    }

                    for(i = 0; i<100; i++)
                    {
                        //* the player will keep the started status
                        //* and start to play after seek finish.
                        nSeekTimeMs = random() % nDuration;
                        printf("seektest    CTC_Wrapper::seekto() %d\n", nSeekTimeMs);

                        //* the player will keep the pauded status and
                        //* pause the playback after seek finish.
                        demoPlayer.mCTC_Wrapper->seekTo(nSeekTimeMs);

                        demoPlayer.mCTC_Wrapper->fast();
                        demoPlayer.mPreStatus = STATUS_PLAYING;

                        usleep(500*000);
                    }
                    break;
                }

                case COMMAND_SHOW_MEDIAINFO:   //* show media information.
                {
                    printf("show media information.\n");
                    CdxMediaInfoT *parserMediaInfo = demoPlayer.mCTC_Wrapper->GetMediaInfo();
                    ShowMediaInfo(parserMediaInfo);
                    break;
                }

                case COMMAND_SHOW_DURATION:   //* show media duration, in unit of second.
                {
                    int nDuration = 0;
                    if(demoPlayer.mCTC_Wrapper->getDuration(&nDuration) == 0)
                        printf("media duration = %d seconds.\n", nDuration/1000);
                    else
                        printf("fail to get media duration.\n");
                    break;
                }

                case COMMAND_SHOW_POSITION:   //* show current play position, in unit of second.
                {
                    int nPosition = 0;
                    if(demoPlayer.mCTC_Wrapper->getCurrentPosition(&nPosition) == 0)
                        printf("current position = %d seconds.\n", nPosition);
                    else
                        printf("fail to get pisition.\n");
                    break;
                }

                case COMMAND_SWITCH_AUDIO:   //* switch autio track.
                {
                    int nAudioStreamIndex;
                    nAudioStreamIndex = nCommandParam;
                    int ret = demoPlayer.mCTC_Wrapper->SwitchAudio(nAudioStreamIndex);
                    printf("switch audio to the %dth track, ret= %d.\n", nAudioStreamIndex, ret);
                    break;
                }
                case COMMAND_SETVOLUME:   //* set volume
                {
                    if(nCommandParam>=0 && nCommandParam<=100)
                    {
                        int ret = demoPlayer.mCTC_Wrapper->SetVolume(nCommandParam);
                        printf("SetVolume(%d), ret= %d.\n", nCommandParam, ret);
                    }
                    else
                    {
                        loge("invalid param, nCommandParam=%d", nCommandParam);
                    }
                    break;
                }

                case COMMAND_GETVOLUME:   //* get volume
                {
                    int ret = demoPlayer.mCTC_Wrapper->GetVolume();
                    printf("GetVolume(%d)\n", ret);
                    break;
                }

                case COMMAND_HIDE:   //* hide the video.
                {
                    if(demoPlayer.mCTC_Wrapper->hide() != 0)
                    {
                        printf("error:\n");
                        printf("    demoPlayer.mCTC_Wrapper->hide() return fail.\n");
                        break;
                    }
                    printf("hide.\n");
                    break;
                }
                case COMMAND_SETPOSITION:
                {
                    if(nCommandParam == 0){
                        //show video in a small window
                        demoPlayer.mCTC_Wrapper->setVideoWindow(20, 20, 400, 500);
                    }else{
                        int vWidth = 0, vHeight = 0;
                        demoPlayer.mCTC_Wrapper->getVideoPixel(vWidth, vHeight);
                        printf("video pixel is (%d, %d)\n", vWidth, vHeight);
                        demoPlayer.mCTC_Wrapper->setVideoWindow(0, 0, vWidth, vHeight);
                    }
                    break;
                }
                case COMMAND_SETRADIO:
                {
                    if(nCommandParam == 0 || nCommandParam == 1){
                        demoPlayer.mCTC_Wrapper->setVideoRadio(nCommandParam);
                    }
                    break;
                }
                case COMMAND_SHOW:
                {
                    demoPlayer.mCTC_Wrapper->show();
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
        else
        {
            printf("readCommand fail.\n");
            break;
        }
    }

    printf("destroy CTC_Wrapper.\n");

    if(demoPlayer.mCTC_Wrapper != NULL)
    {
        delete demoPlayer.mCTC_Wrapper;
        demoPlayer.mCTC_Wrapper = NULL;
    }

    printf("destroy CTC_Wrapper 1.\n");
    pthread_mutex_destroy(&demoPlayer.mMutex);

    printf("\n");
    printf("*************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("*************************************************************\n");
    printf("\n");

    return 0;
}
