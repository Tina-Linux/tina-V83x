
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>

#include "cdx_config.h"
#include "log.h"
#include "awplayer.h"
#include "CdxTypes.h"
#include "memoryAdapter.h"

//typedef unsigned long uintptr_t ;

static const int STATUS_STOPPED   = 0;
static const int STATUS_PREPARING = 1;
static const int STATUS_PREPARED  = 2;
static const int STATUS_PLAYING   = 3;
static const int STATUS_PAUSED    = 4;
static const int STATUS_SEEKING   = 5;

#if 0
//-------------------------------------------------------------------
 /*
　　位图文件的组成
          结构名称 符 号
	位图文件头 (bitmap-file header) BITMAPFILEHEADER bmfh
	位图信息头 (bitmap-information header) BITMAPINFOHEADER bmih
	彩色表　(color table) RGBQUAD aColors[]
	图象数据阵列字节 BYTE aBitmapBits[]
  */
typedef struct bmp_header
{
	short twobyte           ;//两个字节，用来保证下面成员紧凑排列，这两个字符不能写到文件中
	     //14B
	char bfType[2]          ;//!文件的类型,该值必需是0x4D42，也就是字符'BM'
	unsigned int bfSize     ;//!说明文件的大小，用字节为单位
	unsigned int bfReserved1;//保留，必须设置为0
	unsigned int bfOffBits  ;//!说明从文件头开始到实际的图象数据之间的字节的偏移量，这里为14B+sizeof(BMPINFO)
}BMPHEADER;

typedef struct bmp_info
{
	     //40B
	 unsigned int biSize         ;//!BMPINFO结构所需要的字数
	 int biWidth                 ;//!图象的宽度，以象素为单位
	 int biHeight                ;//!图象的宽度，以象素为单位,如果该值是正数，说明图像是倒向的，如果该值是负数，则是正向的
	 unsigned short biPlanes     ;//!目标设备说明位面数，其值将总是被设为1
	 unsigned short biBitCount   ;//!比特数/象素，其值为1、4、8、16、24、或32
	 unsigned int biCompression  ;//说明图象数据压缩的类型
	 #define BI_RGB        0L    //没有压缩
	 #define BI_RLE8       1L    //每个象素8比特的RLE压缩编码，压缩格式由2字节组成（重复象素计数和颜色索引）；
	 #define BI_RLE4       2L    //每个象素4比特的RLE压缩编码，压缩格式由2字节组成
	 #define BI_BITFIELDS  3L    //每个象素的比特由指定的掩码决定。
	 unsigned int biSizeImage    ;//图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0
	 int biXPelsPerMeter         ;//水平分辨率，用象素/米表示
	 int biYPelsPerMeter         ;//垂直分辨率，用象素/米表示
	 unsigned int biClrUsed      ;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
	 unsigned int biClrImportant ;//对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。
}BMPINFO;

typedef struct tagRGBQUAD
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO
{
	BMPINFO    bmiHeader;
	//RGBQUAD    bmiColors[1];
	unsigned int rgb[3];
} BITMAPINFO;


static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)
{
    int size = 0;
    if (head && info)
    {
        size = w * h * 2;
        memset(head, 0, sizeof(* head));
        memset(info, 0, sizeof(* info));
         head->bfType[0] = 'B';
         head->bfType[1] = 'M';
         head->bfOffBits = 14 + sizeof(* info);
         head->bfSize = head->bfOffBits + size;
         head->bfSize = (head->bfSize + 3) & ~3;
         size = head->bfSize - head->bfOffBits;

         info->bmiHeader.biSize = sizeof(info->bmiHeader);
         info->bmiHeader.biWidth = w;
         info->bmiHeader.biHeight = -h;
         info->bmiHeader.biPlanes = 1;
         info->bmiHeader.biBitCount = 16;
         info->bmiHeader.biCompression = BI_BITFIELDS;
         info->bmiHeader.biSizeImage = size;

         info->rgb[0] = 0xF800;
         info->rgb[1] = 0x07E0;
         info->rgb[2] = 0x001F;

         logd("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
     }
     return size;
 }


static int save_bmp_rgb565(FILE* fp, int width, int height, unsigned char* pData)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BITMAPINFO info;
	size = get_rgb565_header(width, height, &head, &info);
	if(size > 0)
	{

        fwrite(head.bfType,1,2,fp);
        fwrite(&head.bfSize,1,4,fp);
        fwrite(&head.bfReserved1,1,4,fp);
        fwrite(&head.bfOffBits,1,4,fp);

		fwrite(&info,1,sizeof(info), fp);
		fwrite(pData,1,size, fp);
		success = 1;
	}
	logd("*****success=%d\n", success);
	return success;
}
#endif


typedef struct DemoPlayerContext
{
    AwPlayer*       mAwPlayer;
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
    {"set url",         COMMAND_SET_SOURCE,         "set url of the media, for example, set url: ~/testfile.mkv."},
    {"play",            COMMAND_PLAY,               "start playback."},
    {"pause",           COMMAND_PAUSE,              "pause the playback."},
    {"stop",            COMMAND_STOP,               "stop the playback."},
    {"set speed",       COMMAND_SETSPEED,      "stop the playback."},
    {"seek to",         COMMAND_SEEKTO,
            "seek to specific position to play, position is in unit of second, for example, seek to: 100."},
    {"show media info", COMMAND_SHOW_MEDIAINFO,     "show media information of the media file."},
    {"show duration",   COMMAND_SHOW_DURATION,      "show duration of the media file."},
    {"show position",   COMMAND_SHOW_POSITION,      "show current play position, position is in unit of second."},
    {"switch audio",    COMMAND_SWITCH_AUDIO,
        "switch audio to a specific track, for example, switch audio: 2, track is start counting from 0."},
    {NULL, 0, NULL}
};

static void showHelp(void)
{
    int     i;

    printf("\n");
    printf("******************************************************************************************\n");
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
    printf("******************************************************************************************\n");
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
                *pParam = (int)strtol(strParam, (char**)NULL, 10);  //* audio stream index start counting from 0.
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
void CallbackForAwPlayer(void* pUserData, int msg, int param0, void* param1)
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
			pDemoPlayer->mError = 1;

			loge(" error : how to deal with it");
            break;
        }

        case NOTIFY_PREPARED:
        {
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mPreStatus = pDemoPlayer->mStatus;
            pDemoPlayer->mStatus = STATUS_PREPARED;
            printf("info: prepare ok.\n");

            MediaInfo* mi = pDemoPlayer->mAwPlayer->getMediaInfo();
            printf("album: %s , albumsz: %d \n", mi->album, mi->albumsz);
            printf("author: %s , authsz: %d \n", mi->author, mi->authorsz);
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
            pthread_mutex_lock(&pDemoPlayer->mMutex);
            pDemoPlayer->mStatus = pDemoPlayer->mPreStatus;
            printf("info: seek ok.\n");
            pthread_mutex_unlock(&pDemoPlayer->mMutex);
            break;
        }

        case NOTIFY_VIDEO_FRAME:
        {

		//if(pDemoPlayer->mVideoFrameNum == 0)
		{
			VideoPicData* videodata = (VideoPicData*)param1;
				if(videodata)
				{
					if(videodata->ePixelFormat == VIDEO_PIXEL_FORMAT_YUV_MB32_420)
					{
						char filename[1024];
					sprintf(filename, "/mnt/UDISK/mb32_%d.dat", pDemoPlayer->mVideoFrameNum);
					FILE* outFp = fopen(filename, "wb");
					if(outFp != NULL)
					    {
						int height64Align = (videodata->nHeight + 63)& ~63;

						fwrite(videodata->pData0, videodata->nWidth*videodata->nHeight, 1, outFp);
						fwrite(videodata->pData1, videodata->nWidth*height64Align/2, 1, outFp);
						fclose(outFp);
					    }
					}

			    }
		    }

		    pDemoPlayer->mVideoFrameNum ++;
		break;
        }

        case NOTIFY_AUDIO_FRAME:
        {

		{
			//AudioPcmData* pcmData = (AudioPcmData*)param1;

			/*
			FILE* outFp = fopen("/mnt/UDISK/mb32.dat", "wb");
			    if(outFp != NULL)
			    {
				save_bmp_rgb565(outFp, videodata->nWidth, videodata->nHeight, videodata->pData);
				fwrite(videodata->pData, videodata->nSize, 1, outFp);
				fclose(outFp);
			    }
			    */
		    }
		    usleep(33*1000);
		break;
        }

        case NOTIFY_VIDEO_PACKET:
        {
		DemuxData* videoData = (DemuxData*)param1;
			//logd("videoData pts: %lld", videoData->nPts);
			static int frame = 0;
			if(frame == 0)
			{
				FILE* outFp = fopen("/mnt/UDISK/video.jpg", "wb");
			if(videoData->nSize0)
			{
				fwrite(videoData->pData0, 1, videoData->nSize0, outFp);
			}
			if(videoData->nSize1)
			{
				fwrite(videoData->pData1, 1, videoData->nSize1, outFp);
			}
			fclose(outFp);
			frame ++;
		}
		break;
        }

        case NOTIFY_AUDIO_PACKET:
        {
		DemuxData* audioData = (DemuxData*)param1;
			//logd("audio pts: %lld", audioData->nPts);
			static int audioframe = 0;
			if(audioframe == 0)
			{
				FILE* outFp = fopen("/mnt/UDISK/audio.pcm", "wb");
			if(audioData->nSize0)
			{
				fwrite(audioData->pData0, 1, audioData->nSize0, outFp);
			}
			if(audioData->nSize1)
			{
				fwrite(audioData->pData1, 1, audioData->nSize1, outFp);
			}
			fclose(outFp);
			audioframe ++;
		}

        }

        default:
        {
            //printf("warning: unknown callback from AwPlayer.\n");
            break;
        }
    }

    return;
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
    printf("******************************************************************************************\n");
    printf("* This program implements a simple player, you can type commands to control the player.\n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("* Inplemented by Allwinner ALD-AL3 department.\n");
    printf("******************************************************************************************\n");

    //* create a player.
    memset(&demoPlayer, 0, sizeof(DemoPlayerContext));
    pthread_mutex_init(&demoPlayer.mMutex, NULL);
    demoPlayer.mAwPlayer = new AwPlayer();
    if(demoPlayer.mAwPlayer == NULL)
    {
        printf("can not create AwPlayer, quit.\n");
        exit(-1);
    }

    //* set callback to player.
    demoPlayer.mAwPlayer->setNotifyCallback(CallbackForAwPlayer, (void*)&demoPlayer);

    //* check if the player work.
    if(demoPlayer.mAwPlayer->initCheck() != 0)
    {
        printf("initCheck of the player fail, quit.\n");
        delete demoPlayer.mAwPlayer;
	    demoPlayer.mAwPlayer = NULL;
        exit(-1);
    }

    //* read, parse and process command from user.
    bQuit = 0;
    while(!bQuit)
    {
	if(demoPlayer.mError)
        {
		demoPlayer.mAwPlayer->reset();
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

                    if(demoPlayer.mStatus != STATUS_STOPPED)
                    {
                        printf("invalid command:\n");
                        printf("    play is not in stopped status.\n");
                        break;
                    }

                    demoPlayer.mSeekable = 1;   //* if the media source is not seekable, this flag will be
                                                //* clear at the NOTIFY_NOT_SEEKABLE callback.

                    //* set url to the AwPlayer.
                    if(demoPlayer.mAwPlayer->setDataSource((const char*)pUrl, NULL) != 0)
                    {
                        printf("error:\n");
                        printf("    AwPlayer::setDataSource() return fail.\n");
                        break;
                    }
                     printf("setDataSource end\n");

                    //* start preparing.
                    pthread_mutex_lock(&demoPlayer.mMutex);    //* lock to sync with the prepare finish notify.
                    if(demoPlayer.mAwPlayer->prepareAsync() != 0)
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
                    if(demoPlayer.mStatus != STATUS_PREPARED &&
                       demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        printf("invalid command:\n");
                        printf("    play eris neither in prepared status nor in seeking.\n");
                        break;
                    }

                    //* start the playback
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        if(demoPlayer.mAwPlayer->start() != 0)
                        {
                            printf("error:\n");
                            printf("    AwPlayer::start() return fail.\n");
                            break;
                        }
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                        demoPlayer.mStatus    = STATUS_PLAYING;
                        printf("playing.\n");
                    }
                    else
                    {
                        //* the player will keep the started status and start to play after seek finish.
                        demoPlayer.mAwPlayer->start();
                        demoPlayer.mPreStatus = STATUS_PLAYING; //* current status is seeking, will set
                                                                //* to mPreStatus when seek finish callback.
                    }
                    break;
                }

                case COMMAND_PAUSE:   //* pause the playback.
                {
                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        printf("invalid command:\n");
                        printf("    player is neither in playing status nor in seeking status.\n");
                        break;
                    }

                    //* pause the playback
                    if(demoPlayer.mStatus != STATUS_SEEKING)
                    {
                        if(demoPlayer.mAwPlayer->pause() != 0)
                        {
                            printf("error:\n");
                            printf("    AwPlayer::pause() return fail.\n");
                            break;
                        }
                        demoPlayer.mPreStatus = demoPlayer.mStatus;
                        demoPlayer.mStatus    = STATUS_PAUSED;
                        printf("paused.\n");
                    }
                    else
                    {
                        //* the player will keep the pauded status and pause the playback after seek finish.
                        demoPlayer.mAwPlayer->pause();
                        demoPlayer.mPreStatus = STATUS_PAUSED;  //* current status is seeking, will set
                                                                //* to mPreStatus when seek finish callback.
                    }
                    break;
                }

                case COMMAND_STOP:   //* stop the playback.
                {
                    if(demoPlayer.mAwPlayer->reset() != 0)
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

                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING &&
                       demoPlayer.mStatus != STATUS_PAUSED  &&
                       demoPlayer.mStatus != STATUS_PREPARED)
                    {
                        printf("invalid command:\n");
                        printf("    player is not in playing/seeking/paused/prepared status.\n");
                        break;
                    }

                    demoPlayer.mAwPlayer->getDuration(&nDuration);

                    if(demoPlayer.mAwPlayer->getDuration(&nDuration) != 0)
                    {
                        printf("getDuration fail, unable to seek!\n");
                        break;
                    }

                    if(nSeekTimeMs > nDuration)
                    {
                        printf("seek time out of range, media duration = %u seconds.\n", nDuration/1000);
                        break;
                    }

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    //* the player will keep the pauded status and pause the playback after seek finish.
                    pthread_mutex_lock(&demoPlayer.mMutex);    //* sync with the seek finish callback.
                    demoPlayer.mAwPlayer->seekTo(nSeekTimeMs);
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

                    if(demoPlayer.mStatus != STATUS_PLAYING &&
                       demoPlayer.mStatus != STATUS_SEEKING &&
                       demoPlayer.mStatus != STATUS_PAUSED  &&
                       demoPlayer.mStatus != STATUS_PREPARED)
                    {
                        printf("invalid command:\n");
                        printf("    player is not in playing/seeking/paused/prepared status.\n");
                        break;
                    }

                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }

                    demoPlayer.mAwPlayer->setSpeed(nSpeed);
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
                    if(demoPlayer.mAwPlayer->getDuration(&nDuration) == 0)
                        printf("media duration = %u seconds.\n", nDuration/1000);
                    else
                        printf("fail to get media duration.\n");
                    break;
                }

                case COMMAND_SHOW_POSITION:   //* show current play position, in unit of second.
                {
                    int nPosition = 0;
                    if(demoPlayer.mAwPlayer->getCurrentPosition(&nPosition) == 0)
                        printf("current position = %u seconds.\n", nPosition/1000);
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
	    delete demoPlayer.mAwPlayer;
	    demoPlayer.mAwPlayer = NULL;
	}

	printf("destroy AwPlayer 1.\n");
    pthread_mutex_destroy(&demoPlayer.mMutex);

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");

	return 0;
}
