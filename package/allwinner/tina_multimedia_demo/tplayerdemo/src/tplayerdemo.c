#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>


#include <allwinner/tplayer.h>
//#include <power_manager_client.h>

#define TOTAL_VIDEO_AUDIO_NUM 100
#define MAX_FILE_NAME_LEN 256
#define FILE_TYPE_NUM 21
#define FILE_TYPE_LEN 10
typedef struct DemoPlayerContext
{
    TPlayer*        mTPlayer;
    int                 mSeekable;
    int                 mError;
    int                 mVideoFrameNum;
    bool              mPreparedFlag;
    bool              mLoopFlag;
    bool              mSetLoop;
    char              mUrl[512];
    MediaInfo*    mMediaInfo;
    char              mVideoAudioList[TOTAL_VIDEO_AUDIO_NUM][MAX_FILE_NAME_LEN];
    int                 mCurPlayIndex;
    int                 mRealFileNum;
    sem_t             mPreparedSem;
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
#define COMMAND_PREPARE		0x102   //* prepare the media file.
#define COMMAND_PLAY            0x103   //* start playback.
#define COMMAND_PAUSE           0x104   //* pause the playback.
#define COMMAND_STOP            0x105   //* stop the playback.
#define COMMAND_SEEKTO          0x106   //* seek to posion, in unit of second.
#define COMMAND_RESET           0x107   //* reset the player
#define COMMAND_SHOW_MEDIAINFO  0x108   //* show media information.
#define COMMAND_SHOW_DURATION   0x109   //* show media duration, in unit of second.
#define COMMAND_SHOW_POSITION   0x110   //* show current play position, in unit of second.
#define COMMAND_SWITCH_AUDIO    0x111   //* switch autio track.
#define COMMAND_PLAY_URL	0x112   //set url and prepare and play
#define COMMAND_SET_VOLUME	0x113   //set the software volume
#define COMMAND_GET_VOLUME	0x114   //get the software volume
#define COMMAND_SET_LOOP	0x115   //set loop play flag,1 means loop play,0 means not loop play
#define COMMAND_SET_SCALEDOWN	0x116   //set video scale down ratio,valid value is:2,4,8 .  2 means 1/2 scaledown,4 means 1/4 scaledown,8 means 1/8 scaledown
#define COMMAND_FAST_FORWARD	0x117   //fast forward,valid value is:2,4,8,16, 2 means 2 times fast forward,4 means 4 times fast forward,8 means 8 times fast forward,16 means 16 times fast forward
#define COMMAND_FAST_BACKWARD	0x118   //fast backward,valid value is:2,4,8,16,2 means 2 times fast backward,4 means 4 times fast backward,8 means 8 times fast backward,16 means 16 times fast backward
#define COMMAND_SET_SRC_RECT	0x119  //set display source crop rect
#define COMMAND_SET_OUTPUT_RECT	0x120  //set display output display rect
#define COMMAND_GET_DISP_FRAMERATE   0x121   //* show the real display framerate


#define CEDARX_UNUSE(param) (void)param

static const Command commands[] =
{
    {"help",            COMMAND_HELP,               "show this help message."},
    {"quit",            COMMAND_QUIT,               "quit this program."},
    {"set url",         COMMAND_SET_SOURCE,         "set url of the media, for example, set url: ~/testfile.mkv."},
    {"prepare",         COMMAND_PREPARE,		"prepare the media."},
    {"play",            COMMAND_PLAY,               "start playback."},
    {"pause",           COMMAND_PAUSE,              "pause the playback."},
    {"stop",            COMMAND_STOP,               "stop the playback."},
    {"seek to",         COMMAND_SEEKTO,
            "seek to specific position to play, position is in unit of second, for example, seek to: 100."},
    {"reset",		COMMAND_RESET,               "reset the player."},
    {"show media info", COMMAND_SHOW_MEDIAINFO,     "show media information of the media file."},
    {"show duration",   COMMAND_SHOW_DURATION,      "show duration of the media file."},
    {"show position",   COMMAND_SHOW_POSITION,      "show current play position, position is in unit of second."},
    {"switch audio",    COMMAND_SWITCH_AUDIO,
        "switch audio to a specific track, for example, switch audio: 2, track is start counting from 0."},
    {"play url",         COMMAND_PLAY_URL,         "set url and prepare and play url,for example:play url:/mnt/UDISK/test.mp3"},
    {"set volume",         COMMAND_SET_VOLUME,         "set the software volume,the range is 0-40,for example:set volume:30"},
    {"get volume",         COMMAND_GET_VOLUME,         "get the software volume"},
    {"set loop",         COMMAND_SET_LOOP,         "set the loop play flag,1 means loop play,0 means not loop play"},
    {"set scaledown",         COMMAND_SET_SCALEDOWN,         "set video scale down ratio,valid value is:2,4,8 .  2 means 1/2 scaledown,4 means 1/4 scaledown,8 means 1/8 scaledown"},
    {"fast forward",         COMMAND_FAST_FORWARD,         "fast forward,valid value is:2,4,8,16, 2 means 2 times fast forward,4 means 4 times fast forward,8 means 8 times fast forward,16 means 16 times fast forward"},
    {"fast backward",         COMMAND_FAST_BACKWARD,         "fast backward,valid value is:2,4,8,16,2 means 2 times fast backward,4 means 4 times fast backward,8 means 8 times fast backward,16 means 16 times fast backward"},
	{"set src_rect",		  COMMAND_SET_SRC_RECT,		"set display source crop rect"},
	{"set dst_rect",		  COMMAND_SET_OUTPUT_RECT,	"set display output rect"},
	{"get display framerate",   COMMAND_GET_DISP_FRAMERATE,      "show the real display framerate."},
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
    printf("*   # ./tplayerdemo\n");
    printf("*   # set url:/mnt/UDISK/test.mp3\n");
    printf("*   # prepare\n");
    printf("*   # show media info\n");
    printf("*   # play\n");
    printf("*   # pause\n");
    printf("*   # stop\n");
    printf("*   # reset\n");
    printf("*   # seek to:100\n");
    printf("*   # play url:/mnt/UDISK/test.mp3\n");
    printf("*   # set volume:30\n");
    printf("*   # get volume\n");
    printf("*   # set loop:1 \n");
    printf("*   # set scaledown:2 \n");
    printf("*   # fast forward:2 \n");
    printf("*   # fast backward:2 \n");
    printf("*   # switch audio:1 \n");
    printf("*   #get display framerate \n");
    printf("*\n");
    printf("* Command and it's param is seperated by a colon, param is optional, as below:\n");
    printf("*     Command[: Param]\n");
    printf("*     #notice:when can play a list use the following command,for example the video or audio file puts in /mnt/UDISK/ directory \n");
    printf("*     #tplayerdemo /mnt/UDISK/ \n");
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
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fflush(stdout);

    nMaxFds    = 0;
    FD_ZERO(&readFdSet);
    FD_SET(STDIN_FILENO, &readFdSet);

    result = select(nMaxFds+1, &readFdSet, NULL, NULL, &tv);
    if(result > 0)
    {
        printf("\ntplayerdemo# ");
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
static int parseCommandLine(char* strCommandLine, unsigned long* pParam)
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

	case COMMAND_PLAY_URL:
            if(strParam != NULL && strlen(strParam) > 0)
                *pParam = (uintptr_t)strParam;        //* pointer to the url.
            else
            {
                printf("no url to play.\n");
                nCommandId = -1;
            }
            break;

	case COMMAND_SET_VOLUME:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);  //* seek time in unit of second.
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("volume value is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("the volume value is not specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SET_LOOP:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("loop value is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("the loop value is not specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_SET_SCALEDOWN:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("scaledown value is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("the scaledown value is not specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_FAST_FORWARD:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("play fast value is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("play fast value is not specified.\n");
                nCommandId = -1;
            }
            break;

        case COMMAND_FAST_BACKWARD:
            if(strParam != NULL)
            {
                *pParam = (int)strtol(strParam, (char**)NULL, 10);
                if(errno == EINVAL || errno == ERANGE)
                {
                    printf("play slow value is not valid.\n");
                    nCommandId = -1;
                }
            }
            else
            {
                printf("play slow value is not specified.\n");
                nCommandId = -1;
            }
            break;

        default:
            break;
    }

    return nCommandId;
}

static int semTimedWait(sem_t* sem, int64_t time_ms)
{
    int err;

    if(time_ms == -1)
    {
        err = sem_wait(sem);
    }
    else
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += time_ms % 1000 * 1000 * 1000;
        ts.tv_sec += time_ms / 1000 + ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec = ts.tv_nsec % (1000*1000*1000);

        err = sem_timedwait(sem, &ts);
    }

    return err;
}


//* a callback for tplayer.
int CallbackForTPlayer(void* pUserData, int msg, int param0, void* param1)
{
    DemoPlayerContext* pDemoPlayer = (DemoPlayerContext*)pUserData;

    CEDARX_UNUSE(param1);
    switch(msg)
    {
        case TPLAYER_NOTIFY_PREPARED:
        {
            printf("TPLAYER_NOTIFY_PREPARED,has prepared.\n");
            sem_post(&pDemoPlayer->mPreparedSem);
            pDemoPlayer->mPreparedFlag = 1;
            break;
        }

        case TPLAYER_NOTIFY_PLAYBACK_COMPLETE:
        {
            printf("TPLAYER_NOTIFY_PLAYBACK_COMPLETE\n");
            if(pDemoPlayer->mSetLoop == 1){
                pDemoPlayer->mLoopFlag = 1;
            }else{
                pDemoPlayer->mLoopFlag = 0;
            }
            //PowerManagerReleaseWakeLock("tplayerdemo");
            break;
        }

        case TPLAYER_NOTIFY_SEEK_COMPLETE:
        {
            printf("TPLAYER_NOTIFY_SEEK_COMPLETE>>>>info: seek ok.\n");
            break;
        }

        case TPLAYER_NOTIFY_MEDIA_ERROR:
        {
            switch (param0)
            {
                case TPLAYER_MEDIA_ERROR_UNKNOWN:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN\n");
                    break;
                }
                case TPLAYER_MEDIA_ERROR_UNSUPPORTED:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED\n");
                    break;
                }
                case TPLAYER_MEDIA_ERROR_IO:
                {
                    printf("erro type:TPLAYER_MEDIA_ERROR_IO\n");
                    break;
                }
            }
            printf("TPLAYER_NOTIFY_MEDIA_ERROR\n");
            pDemoPlayer->mError = 1;
            if(pDemoPlayer->mPreparedFlag == 0){
                printf("recive err when preparing\n");
                sem_post(&pDemoPlayer->mPreparedSem);
            }
            if(pDemoPlayer->mSetLoop == 1){
                pDemoPlayer->mLoopFlag = 1;
            }else{
                pDemoPlayer->mLoopFlag = 0;
            }
            printf("error: open media source fail.\n");
            break;
        }

        case TPLAYER_NOTIFY_NOT_SEEKABLE:
        {
            pDemoPlayer->mSeekable = 0;
            printf("info: media source is unseekable.\n");
            break;
        }

	case TPLAYER_NOTIFY_BUFFER_START:
        {
            printf("have no enough data to play\n");
            break;
        }

	case TPLAYER_NOTIFY_BUFFER_END:
        {
            printf("have enough data to play again\n");
            break;
        }

        case TPLAYER_NOTIFY_VIDEO_FRAME:
        {
            //printf("get the decoded video frame\n");
            break;
        }

        case TPLAYER_NOTIFY_AUDIO_FRAME:
        {
            //printf("get the decoded audio frame\n");
            break;
        }

        case TPLAYER_NOTIFY_SUBTITLE_FRAME:
        {
            //printf("get the decoded subtitle frame\n");
            break;
        }
        case TPLAYER_NOTYFY_DECODED_VIDEO_SIZE:
        {
            int w, h;
            w   = ((int*)param1)[0];   //real decoded video width
            h  = ((int*)param1)[1];   //real decoded video height
            printf("*****tplayerdemo:video decoded width = %d,height = %d",w,h);
            //int divider = 1;
            //if(w>400){
            //    divider = w/400+1;
            //}
            //w = w/divider;
            //h = h/divider;
            printf("real set to display rect:w = %d,h = %d\n",w,h);
            //TPlayerSetSrcRect(pDemoPlayer->mTPlayer, 0, 0, w, h);
        }

        default:
        {
            printf("warning: unknown callback from Tinaplayer.\n");
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
    unsigned long  nCommandParam;
    int  bQuit;
    char strCommandLine[1024];
    CEDARX_UNUSE(argc);
    CEDARX_UNUSE(argv);
    memset(&demoPlayer, 0, sizeof(DemoPlayerContext));
    demoPlayer.mError = 0;
    demoPlayer.mSeekable = 1;
    demoPlayer.mPreparedFlag = 0;
    demoPlayer.mLoopFlag = 0;
    demoPlayer.mSetLoop = 0;
    demoPlayer.mMediaInfo = NULL;
    int waitErr = 0;
    int isDir = 0;

    printf("\n");
    printf("******************************************************************************************\n");
    printf("* This program implements a simple player, you can type commands to control the player.\n");
    printf("* To show what commands supported, type 'help'.\n");
    printf("******************************************************************************************\n");

    if(argc == 2){
        printf("argc = %d,argv[0] = %s,argv[1] = %s\n",argc,argv[0],argv[1]);
        char fileType[FILE_TYPE_NUM][FILE_TYPE_LEN] = {".avi",".mkv",".flv",".ts",".mp4",".ts",".webm",".asf",".mpg",".mov",".vob",
                                                                                           ".mp1",".mp2",".mp3",".ogg",".flac",".ape",".wav",".m4a",".amr",".aac"};
        char* lastStrPos;
        if((lastStrPos = strrchr(argv[1],'.')) != NULL){
            int i = 0;
            printf("may be is one file:cut down suffix is:%s\n",lastStrPos);
            int findMatchFileFlag = 0;
            for(i = 0;i < FILE_TYPE_NUM;i++){
                if(!strncasecmp(lastStrPos,&(fileType[i]),strlen(&(fileType[i])))){
                    printf("find the matched type:%s\n",&(fileType[i]));
                    findMatchFileFlag = 1;
                    break;
                }
            }
            if(findMatchFileFlag == 0){
                printf("can not play this file:%s\n",argv[1]);
                exit(-1);
            }
            demoPlayer.mLoopFlag = 1;
            demoPlayer.mSetLoop = 1;
        }else{
            DIR *dir = opendir(argv[1]);
            if (dir == NULL)
            {
                printf("opendir %s fail\n",argv[1]);
                return -1;
            }
            isDir = 1;
            struct dirent *entry;
            int count= 0;
            while ((entry = readdir(dir)) != NULL)
            {
                printf("file record length = %d,type = %d, name = %s\n",entry->d_reclen,entry->d_type,entry->d_name);
                if(entry->d_type == 8){
                    char* strpos;
                    if((strpos = strrchr(entry->d_name,'.')) != NULL){
                        int i = 0;
                        printf("cut down suffix is:%s\n",strpos);
                        for(i = 0;i < FILE_TYPE_NUM;i++){
                            if(!strncasecmp(strpos,&(fileType[i]),strlen(&(fileType[i])))){
                                printf("find the matched type:%s\n",&(fileType[i]));
                                break;
                            }
                        }

                        if(i < FILE_TYPE_NUM){
                            if(count < TOTAL_VIDEO_AUDIO_NUM){
                                strncpy(&demoPlayer.mVideoAudioList[count],entry->d_name,strlen(entry->d_name));
                                printf("video file name = %s\n",&demoPlayer.mVideoAudioList[count]);
                                count++;
                            }else{
                                count++;
                                printf("warning:the video file in /mnt/UDISK/ is %d,which is larger than %d,we only support %d\n",count,TOTAL_VIDEO_AUDIO_NUM,TOTAL_VIDEO_AUDIO_NUM);
                            }
                        }
                    }
                }
            }
            closedir(dir);
            demoPlayer.mLoopFlag = 1;
            demoPlayer.mSetLoop = 1;
            if(count >  TOTAL_VIDEO_AUDIO_NUM){
                demoPlayer.mRealFileNum = TOTAL_VIDEO_AUDIO_NUM;
            }else{
                demoPlayer.mRealFileNum = count;
            }
            if(demoPlayer.mRealFileNum == 0){
                printf("there are no video or audio files in %s,exit(-1)\n",argv[1]);
                exit(-1);
            }
        }
    }

    //* create a player.
    demoPlayer.mTPlayer= TPlayerCreate(CEDARX_PLAYER);
    if(demoPlayer.mTPlayer == NULL)
    {
        printf("can not create tplayer, quit.\n");
        exit(-1);
    }
    //* set callback to player.
    TPlayerSetNotifyCallback(demoPlayer.mTPlayer,CallbackForTPlayer, (void*)&demoPlayer);
    if(((access("/dev/zero",F_OK)) < 0)||((access("/dev/fb0",F_OK)) < 0)){
        printf("/dev/zero OR /dev/fb0 is not exit\n");
    }else{
        system("dd if=/dev/zero of=/dev/fb0");//clean the framebuffer
    }
    sem_init(&demoPlayer.mPreparedSem, 0, 0);
    //* read, parse and process command from user.
    bQuit = 0;

    while(!bQuit)
    {
        //for test loop play which use reset for each play
        //printf("demoPlayer.mLoopFlag = %d",demoPlayer.mLoopFlag);
	if(demoPlayer.mLoopFlag){
            demoPlayer.mLoopFlag = 0;
            //char* pUrl = "https://192.168.0.125/hls/h264/playlist_10.m3u8";
            printf("TPlayerReset begin\n");
            if(TPlayerReset(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerReset return fail.\n");
            }else{
		printf("reset the player ok.\n");
		if(demoPlayer.mError == 1){
			demoPlayer.mError = 0;
		}
		//PowerManagerReleaseWakeLock("tplayerdemo");
            }
            demoPlayer.mSeekable = 1;   //* if the media source is not seekable, this flag will be
                                                        //* clear at the TINA_NOTIFY_NOT_SEEKABLE callback.

             if(argc == 2){
                if(isDir){
                    if(demoPlayer.mCurPlayIndex == demoPlayer.mRealFileNum){
                    demoPlayer.mCurPlayIndex = 0;
                    }
                    strcpy(demoPlayer.mUrl,argv[1]);
                    strcat(demoPlayer.mUrl,&demoPlayer.mVideoAudioList[demoPlayer.mCurPlayIndex]);
                    printf("demoPlayer.mUrl = %s\n",demoPlayer.mUrl);
                    demoPlayer.mCurPlayIndex++;
                }else{
                     strcpy(demoPlayer.mUrl,argv[1]);
                     printf("demoPlayer.mUrl = %s\n",demoPlayer.mUrl);
                }
                if(((access("/dev/zero",F_OK)) < 0)||((access("/dev/fb0",F_OK)) < 0)){
                    printf("/dev/zero OR /dev/fb0 is not exit\n");
                }else{
                    system("dd if=/dev/zero of=/dev/fb0");//clean the framebuffer
                }
             }
            //* set url to the tinaplayer.
            if(TPlayerSetDataSource(demoPlayer.mTPlayer,(const char*)demoPlayer.mUrl,NULL) != 0)
            {
			printf("TPlayerSetDataSource() return fail.\n");
            }else{
			printf("TPlayerSetDataSource() end\n");
            }
            demoPlayer.mPreparedFlag = 0;
            if(TPlayerPrepareAsync(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerPrepareAsync() return fail.\n");
            }else{
		        printf("preparing...\n");
            }
            waitErr = semTimedWait(&demoPlayer.mPreparedSem,300*1000);
            if(waitErr == -1){
                printf("prepare fail,has wait 300s\n");
                break;
            }else if(demoPlayer.mError == 1){
                printf("prepare fail\n");
                break;
            }
            printf("prepare ok\n");
            printf("TPlayerSetHoldLastPicture()\n");
            TPlayerSetHoldLastPicture(demoPlayer.mTPlayer,1);
            printf("start play\n");
            //TPlayerSetLooping(demoPlayer.mTPlayer,1);//let the player into looping mode
            //* start the playback
            if(TPlayerStart(demoPlayer.mTPlayer) != 0)
            {
                printf("TPlayerStart() return fail.\n");
            }else{
		printf("started.\n");
		//PowerManagerAcquireWakeLock("tplayerdemo");
            }
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
                    printf("COMMAND_QUIT\n");
                    bQuit = 1;
                    break;
                }

                case COMMAND_SET_SOURCE :   //* set url of media file.
                {
                    char* pUrl;
                    pUrl = (char*)(uintptr_t)nCommandParam;
                    memset(demoPlayer.mUrl,0,512);
                    strcpy(demoPlayer.mUrl,pUrl);
                    printf("demoPlayer.mUrl = %s\n",demoPlayer.mUrl);
                    if(demoPlayer.mError == 1) //pre status is error,reset the player first
                    {
			printf("pre status is error,reset the tina player first.\n");
			TPlayerReset(demoPlayer.mTPlayer);
			demoPlayer.mError = 0;
                    }

                    demoPlayer.mSeekable = 1;   //* if the media source is not seekable, this flag will be
                                                                //* clear at the TINA_NOTIFY_NOT_SEEKABLE callback.
                    //* set url to the tinaplayer.
                    if(TPlayerSetDataSource(demoPlayer.mTPlayer,(const char*)demoPlayer.mUrl,NULL) != 0)
                    {
                        printf("TPlayerSetDataSource() return fail.\n");
                        break;
                    }else{
                        printf("setDataSource end\n");
                    }
                    break;
                }

                case COMMAND_PREPARE:
                {
                    demoPlayer.mPreparedFlag = 0;
                    if(TPlayerPrepareAsync(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerPrepareAsync() return fail.\n");
                    }else{
			printf("prepare\n");
                    }
                    waitErr = semTimedWait(&demoPlayer.mPreparedSem,300*1000);
                    if(waitErr == -1){
                        printf("prepare fail,has wait 300s\n");
                        break;
                    }else if(demoPlayer.mError == 1){
                        printf("prepare fail\n");
                        break;
                    }
                    printf("prepared ok\n");
                    break;
                }

                case COMMAND_PLAY:   //* start playback.
                {
                    printf("TPlayerSetHoldLastPicture()\n");
                    TPlayerSetHoldLastPicture(demoPlayer.mTPlayer,1);
                    if(TPlayerStart(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerStart() return fail.\n");
                        break;
                    }else{
			printf("started.\n");
			//PowerManagerAcquireWakeLock("tplayerdemo");
                    }
                    break;
                }

                case COMMAND_PAUSE:   //* pause the playback.
                {
		if(TPlayerPause(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerPause() return fail.\n");
                        break;
                    }else{
			printf("paused.\n");
			//PowerManagerReleaseWakeLock("tplayerdemo");
		}
                    break;
                }

                case COMMAND_STOP:   //* stop the playback.
                {
                    if(TPlayerStop(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerStop() return fail.\n");
                        break;
                    }else{
			//PowerManagerReleaseWakeLock("tplayerdemo");
		}
                    break;
                }

                case COMMAND_SEEKTO:   //* seek to posion, in unit of second.
                {
                    int nSeekTimeMs;
                    int nDuration;
                    nSeekTimeMs = nCommandParam*1000;
                    int ret = TPlayerGetDuration(demoPlayer.mTPlayer,&nDuration);
                    printf("nSeekTimeMs = %d , nDuration = %d\n",nSeekTimeMs,nDuration);
                    if(ret != 0)
                    {
		        printf("getDuration fail, unable to seek!\n");
                        break;
                    }

                    if(nSeekTimeMs > nDuration){
                        printf("seek time out of range, media duration = %d seconds.\n", nDuration/1000);
                        break;
                    }
                    if(demoPlayer.mSeekable == 0)
                    {
                        printf("media source is unseekable.\n");
                        break;
                    }
                    if(TPlayerSeekTo(demoPlayer.mTPlayer,nSeekTimeMs) != 0){
                        printf("TPlayerSeekTo() return fail,nSeekTimeMs= %d\n",nSeekTimeMs);
                        break;
                    }else{
			printf("is seeking.\n");
		    }
                    break;
                }

                case COMMAND_RESET:   //* reset the player
                {
                    if(TPlayerReset(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerReset() return fail.\n");
                        break;
                    }else{
			printf("reset the player ok.\n");
			//PowerManagerReleaseWakeLock("tinaplayerdemo");
		    }
                    break;
                }

                case COMMAND_SHOW_MEDIAINFO:   //* show media information.
                {
                    printf("show media information:\n");
                    MediaInfo* mi = NULL;
                    demoPlayer.mMediaInfo = TPlayerGetMediaInfo(demoPlayer.mTPlayer);
                    if(demoPlayer.mMediaInfo != NULL){
                        mi = demoPlayer.mMediaInfo;
                        printf("file size = %lld KB\n",mi->nFileSize/1024);
                        printf("duration = %lld ms\n",mi->nDurationMs);
                        printf("bitrate = %d Kbps\n",mi->nBitrate/1024);
                        printf("container type = %d\n",mi->eContainerType);
#ifndef ONLY_ENABLE_AUDIO
                        printf("video stream num = %d\n",mi->nVideoStreamNum);
#endif
                        printf("audio stream num = %d\n",mi->nAudioStreamNum);
#ifndef ONLY_ENABLE_AUDIO
                        printf("subtitle stream num = %d\n",mi->nSubtitleStreamNum);
                        if(mi->pVideoStreamInfo != NULL){
                            printf("video codec tpye = %d\n",mi->pVideoStreamInfo->eCodecFormat);
                            printf("video width = %d\n",mi->pVideoStreamInfo->nWidth);
                            printf("video height = %d\n",mi->pVideoStreamInfo->nHeight);
                            printf("video framerate = %d\n",mi->pVideoStreamInfo->nFrameRate);
                            printf("video frameduration = %d\n",mi->pVideoStreamInfo->nFrameDuration);
                        }
#endif
                        if(mi->pAudioStreamInfo != NULL){
                            printf("audio codec tpye = %d\n",mi->pAudioStreamInfo->eCodecFormat);
                            printf("audio channel num = %d\n",mi->pAudioStreamInfo->nChannelNum);
                            printf("audio BitsPerSample = %d\n",mi->pAudioStreamInfo->nBitsPerSample);
                            printf("audio sample rate  = %d\n",mi->pAudioStreamInfo->nSampleRate);
                            printf("audio bitrate = %d Kbps\n",mi->pAudioStreamInfo->nAvgBitrate/1024);
                        }

                    }
                    break;
                }

                case COMMAND_SHOW_DURATION:   //* show media duration, in unit of second.
                {
                    int nDuration = 0;
                    if(TPlayerGetDuration(demoPlayer.mTPlayer,&nDuration) == 0)
                        printf("media duration = %d seconds.\n", nDuration/1000);
                    else
                        printf("fail to get media duration.\n");
                    break;
                }

                case COMMAND_SHOW_POSITION:   //* show current play position, in unit of second.
                {
                    int nPosition = 0;
                    if(TPlayerGetCurrentPosition(demoPlayer.mTPlayer,&nPosition) == 0)
                        printf("current position = %d seconds.\n", nPosition/1000);
                    else
                        printf("fail to get pisition.\n");
                    break;
                }

                case COMMAND_GET_DISP_FRAMERATE:   //* show media duration, in unit of second.
                {
                    float realDispFramerate = 0.0;
                    int getFramerateRet = TPlayerGetVideoDispFramerate(demoPlayer.mTPlayer,&realDispFramerate);
                    if(getFramerateRet == -1)
                        printf("err:we should get the real display framerate after play\n");
                    else
                        printf("real display framerate : %f\n",realDispFramerate);
                    break;
                }

                case COMMAND_SWITCH_AUDIO:   //* switch autio track.
                {
                    int audioStreamIndex;
                    audioStreamIndex = (int)nCommandParam;
                    printf("switch audio to the %dth track.\n", audioStreamIndex);
                    int ret = TPlayerSwitchAudio(demoPlayer.mTPlayer,audioStreamIndex);
                    if(ret != 0){
                        printf("switch audio err\n");
                    }
                    break;
                }

                case COMMAND_PLAY_URL:   //* set url of media file.
                {
                    char* pUrl;
                    pUrl = (char*)(uintptr_t)nCommandParam;
                    memset(demoPlayer.mUrl,0,512);
                    strcpy(demoPlayer.mUrl,pUrl);
                    printf("demoPlayer.mUrl = %s",demoPlayer.mUrl);
                    if(TPlayerReset(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerReset() return fail.\n");
                        break;
                    }else{
                        printf("reset the player ok.\n");
                        if(demoPlayer.mError == 1){
			    demoPlayer.mError = 0;
                        }
                        //PowerManagerReleaseWakeLock("tplayerdemo");
		    }
                    demoPlayer.mSeekable = 1;   //* if the media source is not seekable, this flag will be
                                                                //* clear at the TINA_NOTIFY_NOT_SEEKABLE callback.
                    //* set url to the tinaplayer.
                    if(TPlayerSetDataSource(demoPlayer.mTPlayer,(const char*)demoPlayer.mUrl,NULL)!= 0)
                    {
                        printf("TPlayerSetDataSource return fail.\n");
                        break;
                    }else{
                        printf("setDataSource end\n");
                    }

		    demoPlayer.mPreparedFlag = 0;
                    if(TPlayerPrepareAsync(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerPrepareAsync() return fail.\n");
                    }else{
			printf("preparing...\n");
                    }
                    waitErr = semTimedWait(&demoPlayer.mPreparedSem,300*1000);
                    if(waitErr == -1){
                        printf("prepare fail,has wait 300s\n");
                        break;
                    }else if(demoPlayer.mError == 1){
                        printf("prepare fail\n");
                        break;
                    }
                    printf("prepared ok\n");
                    printf("TPlayerSetHoldLastPicture()\n");
                    TPlayerSetHoldLastPicture(demoPlayer.mTPlayer,1);
                    printf("start play \n");
                    //TPlayerSetLooping(demoPlayer.mTPlayer,1);//let the player into looping mode
                    //* start the playback
                    if(TPlayerStart(demoPlayer.mTPlayer) != 0)
                    {
                        printf("TPlayerStart() return fail.\n");
                        break;
                    }else{
                        printf("started.\n");
                        //PowerManagerAcquireWakeLock("tplayerdemo");
                    }
                    break;
                }

                case COMMAND_SET_VOLUME:   //* seek to posion, in unit of second.
                {
                    int volume = (int)nCommandParam;
                    printf("tplayerdemo setVolume:volume = %d\n",volume);
                    int ret = TPlayerSetVolume(demoPlayer.mTPlayer,volume);
                    if(ret == -1){
                        printf("tplayerdemo set volume err\n");
                    }else{
                        printf("tplayerdemo set volume ok\n");
                    }
                    break;
                }

                case COMMAND_GET_VOLUME:   //* seek to posion, in unit of second.
                {
                    int curVolume = TPlayerGetVolume(demoPlayer.mTPlayer);
                    printf("cur volume = %d\n",curVolume);
                    break;
                }

                case COMMAND_SET_LOOP:   //* set loop flag
                {
                    printf("tplayerdemo set loop flag:flag = %d\n",(int)nCommandParam);
                    if(nCommandParam == 1){
                        demoPlayer.mSetLoop = 1;
                    }else if(nCommandParam == 0){
                        demoPlayer.mSetLoop = 0;
                    }else{
                        printf("the set loop value is wrong\n");
                    }
                    break;
                }

                case COMMAND_SET_SCALEDOWN:   //set video scaledown
                {
                    int scaleDown = (int)nCommandParam;
                    printf("tplayerdemo set scaledown value = %d\n",scaleDown);
                    switch (scaleDown)
                    {
                        case 2:
                        {
                            printf("scale down 1/2\n");
                            int ret = TPlayerSetScaleDownRatio(demoPlayer.mTPlayer,TPLAYER_VIDEO_SCALE_DOWN_2,TPLAYER_VIDEO_SCALE_DOWN_2);
                            if(ret != 0){
                                printf("set scale down err\n");
                            }
                            break;
                        }

                        case 4:
                        {
                            printf("scale down 1/4\n");
                            int ret = TPlayerSetScaleDownRatio(demoPlayer.mTPlayer,TPLAYER_VIDEO_SCALE_DOWN_4,TPLAYER_VIDEO_SCALE_DOWN_4);
                            if(ret != 0){
                                printf("set scale down err\n");
                            }
                            break;
                        }

                        case 8:
                        {
                            printf("scale down 1/8\n");
                            int ret = TPlayerSetScaleDownRatio(demoPlayer.mTPlayer,TPLAYER_VIDEO_SCALE_DOWN_8,TPLAYER_VIDEO_SCALE_DOWN_8);
                            if(ret != 0){
                                printf("set scale down err\n");
                            }
                            break;
                        }
                        default:
                        {
                            printf("scaledown value is wrong\n");
                            break;
                        }
                    }
                    break;
                }

                case COMMAND_FAST_FORWARD:   //fast forward
                {
                    int fastForwardTimes = (int)nCommandParam;
                    printf("tplayerdemo fast forward times = %d\n",fastForwardTimes);

                    switch (fastForwardTimes)
                    {
                        case 2:
                        {
                            printf("fast forward 2 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_FORWARD_2);
                            if(ret != 0){
                                printf("fast forward err\n");
                            }
                            break;
                        }

                        case 4:
                        {
                            printf("fast forward 4 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_FORWARD_4);
                            if(ret != 0){
                                printf("fast forward err\n");
                            }
                            break;
                        }

                        case 8:
                        {
                            printf("fast forward 8 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_FORWARD_8);
                            if(ret != 0){
                                printf("fast forward err\n");
                            }
                            break;
                        }

                        case 16:
                        {
                            printf("fast forward 16 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_FORWARD_16);
                            if(ret != 0){
                                printf("fast forward err\n");
                            }
                            break;
                        }

                        default:
                        {
                            printf("the value of fast forward times is wrong,can not fast forward. value = %d\n",fastForwardTimes);
                            printf("we set it to normal play\n");
                            TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_1);
                            break;
                        }
                    }
                    break;
                }

                case COMMAND_FAST_BACKWARD:   //fast backward
                {
                    int fastBackwardTimes = (int)nCommandParam;
                    printf("tplayerdemo fast backward times = %d\n",fastBackwardTimes);

                    switch (fastBackwardTimes)
                    {
                        case 2:
                        {
                            printf("fast backward 2 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_BACKWARD_2);
                            if(ret != 0){
                                printf("fast backward err\n");
                            }
                            break;
                        }

                        case 4:
                        {
                            printf("fast backward 4 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_BACKWARD_4);
                            if(ret != 0){
                                printf("fast backward err\n");
                            }
                            break;
                        }

                        case 8:
                        {
                            printf("fast backward 8 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_BACKWARD_8);
                            if(ret != 0){
                                printf("fast backward err\n");
                            }
                            break;
                        }

                        case 16:
                        {
                            printf("fast backward 16 times\n");
                            int ret = TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_FAST_BACKWARD_16);
                            if(ret != 0){
                                printf("fast backward err\n");
                            }
                            break;
                        }

                        default:
                        {
                            printf("the value of fast backward times is wrong,can not fast backward. value = %d\n",fastBackwardTimes);
                            printf("we set it to normal play\n");
                            TPlayerSetSpeed(demoPlayer.mTPlayer,PLAY_SPEED_1);
                            break;
                        }
                    }
                    break;
                }
				case COMMAND_SET_SRC_RECT:
				{
					TPlayerSetSrcRect(demoPlayer.mTPlayer, 0, 0, 720, 480);
					break;
				}
				case COMMAND_SET_OUTPUT_RECT:
				{
					TPlayerSetDisplayRect(demoPlayer.mTPlayer, 0, 0, 400, 400);
					break;
				}
            }
        }
    }

    if(demoPlayer.mTPlayer != NULL)
    {
        TPlayerDestroy(demoPlayer.mTPlayer);
        demoPlayer.mTPlayer = NULL;
        printf("TPlayerDestroy() successfully\n");
    }

    printf("destroy tplayer \n");

    sem_destroy(&demoPlayer.mPreparedSem);

    //PowerManagerReleaseWakeLock("tplayerdemo");
    printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");

    return 0;
}
