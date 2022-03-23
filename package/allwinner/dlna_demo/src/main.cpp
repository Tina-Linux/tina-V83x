#define TAG "main"
#include <tina_log.h>

#include <stdio.h>
#include <unistd.h>
#include <string>

#include   <stdio.h>
#include   <sys/ioctl.h>
#include   <sys/socket.h>
#include   <netinet/in.h>
#include   <net/if.h>
#include   <string.h>
#include <alsa/asoundlib.h>
#include <allwinner/tinaplayer.h>

#include <dlna/DLNAService.h>

using namespace softwinner;
using namespace std;
using namespace aw;
//* define commands for user control.
typedef struct Command
{
    const char* strCommand;
    int         nCommandId;
    const char* strHelpMsg;
}Command;

#define COMMAND_HELP            0x01     //* show help message.
#define COMMAND_QUIT            0x02     //* quit this program.

#define COMMAND_START			0x101   //* start the dmr.
#define COMMAND_STOP			0x102   //* stop the dmr.

static const Command commands[] =
{
    {"help",            COMMAND_HELP,               "show this help message."},
    {"quit",            COMMAND_QUIT,               "quit this program."},
    {"start",           COMMAND_START,			"start the dmr."},
    {"stop",            COMMAND_STOP,               "stop the dmr."},
    {NULL, 0, NULL}
};


int getHwAddr(const char* name,char* hwaddr)
{
    struct ifreq ifreq;
    int sock;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return 2;
    }
    strcpy(ifreq.ifr_name,name);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        return 3;
    }
    sprintf(hwaddr,"%02x:%02x:%02x:%02x:%02x:%02x\n ",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

    return   0;
}
class myDLNAPlayer : public EventListener{
public:
    myDLNAPlayer(DLNADeviceInfo info){
        //am = new Apa3165();
        printf("myDLNAPlayer()\n");
        mDlna = new DLNAService();
		mTinaPlayer = new TinaPlayer();
        //start
        //int curvolume = am->getVolume();
        int curvolume = 40;
        mDlna->updateVolume(curvolume);
        mDlna->setDeviceInfo(info);
        mDlna->setEventListener(this);
		mDlna->setPlayer((void*)mTinaPlayer);
        mDlna->startDMR();
    };
    ~myDLNAPlayer(){
		printf("~myDLNAPlayer()\n");
		if(mDlna != NULL){
			mDlna->stopDMR();
	        //delete am;
	        delete mDlna;
			mDlna = NULL;
		}
		if(mTinaPlayer != NULL){
			delete mTinaPlayer;
			mTinaPlayer = NULL;
		}
    };

	int stopDMR(){
		if(mDlna != NULL){
			int ret = mDlna->stopDMR();
			return ret;
		}else{
			TLOGE("mDlna is null");
			return -1;
		}
	};
    /* Override */
    void onPlay(string url, DLNAMediaInfo info){
        //不能阻塞
        TLOGD("onPlay url: %s",url.data());
        TLOGD("onPlay url: %s",info.Title.data());
    };
    /* Override */
    void onSetVolume(int volume){
        //不能阻塞
        //以0~100量化，0最小，100最大
        //am->setVolume(volume);
        //如果设置成功
        //dlna->updateVolume(volume);
        TLOGD("onSetVolume : %d",volume);

    };
    /* Override */
    void onPause(){
        //不能阻塞
        TLOGD("onPause***** ");
    }
    /* Override */
    void onStop(){
        //不能阻塞
        TLOGD("onStop***** ");
    }

private:
     DLNAService *mDlna;
	 TinaPlayer* mTinaPlayer;
     //AudioManager* am;
};

int record_stereo_test(const char *filename, int sample_rate)
{
    int i;
    int err;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    int dtime = 1;
    int r;

    FILE *fp = NULL;
    int loop;
    char buf[4096];

    fprintf(stderr, "sample_rate is %d\n", sample_rate);

    //if((err = snd_pcm_open(&capture_handle,  "default", SND_PCM_STREAM_CAPTURE, 0)) < 0)
    if((err = snd_pcm_open(&capture_handle,  "hw:1,0", SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        fprintf(stderr, "cannot open audio device (%s)\n",  snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, sample_rate, 0)) < 0)
    {
        fprintf(stderr , "cannot set sample rate (%s)\n", snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 2)) <0)
    {
        fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
        goto fail1;
    }

    if((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
    {
        fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
        goto fail1;
    }

    snd_pcm_hw_params_free(hw_params);

    fprintf(stderr, "open file : %s\n",filename);
    dtime = 10;

    fp = fopen(filename, "wb+");
    if (fp == NULL) {
        fprintf(stderr, "open test pcm file err\n");
        goto fail1;
    }
    loop = dtime*sample_rate/1024;

    for( ;loop > 0;loop-- )
    {
        r = snd_pcm_readi(capture_handle, buf, 1024);
        if(r>0)
        {
            err = fwrite(buf, 1, r*4, fp);

            if(err < 0)
            {
                fprintf(stderr, "write to audio interface failed (%s)\n",snd_strerror(err));
                return err;
            }
        }
    }
    fprintf(stderr, "close file\n");
    fclose(fp);

    fprintf(stderr, "close dev\n");
    snd_pcm_close(capture_handle);
    fprintf(stderr, "ok\n");

    return 0;

fail1:
    fprintf(stderr, "close file\n");
    if(fp){
        fclose(fp);
        fp = NULL;
    }
    fprintf(stderr, "close dev\n");
    snd_pcm_close(capture_handle);
    fprintf(stderr, "Fail\n");

    return -1;
}

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

    printf("\nmydlnademo:readCommand() \n");
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

    return nCommandId;
}


int main(){

    char hwaddr[18];
    memset(hwaddr,0,sizeof(hwaddr));
    getHwAddr("wlan0",hwaddr);

	int  nCommandId = -1;
	int  nCommandParam = 0;
	int  bQuit = 0;
    char strCommandLine[1024];
    DLNADeviceInfo info;
    info.name = string("Tina-DLNA");

    info.uuid = string("gredjlgjrwgljdgpejfgepotjwgr");

    info.device_id = string("dlna_test11");
    info.manufacture_id = string("grwgwr");
    info.pre_shared_key = string("0x3e,0x0b,0x32,0x1f,0xa8,0xc9,0xa1,0xd8,0x5a,0xef,0x10,0xc4,0xda,0x3d,0x3c,0x34,0x00");
    info.qplay_ver = 0;

	myDLNAPlayer* dlnaPlayer = NULL;

	while(!bQuit){
		//* read command from stdin.
        if(readCommand(strCommandLine, sizeof(strCommandLine)) == 0){
			nCommandId = parseCommandLine(strCommandLine, &nCommandParam);
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

				case COMMAND_START:
                {
					printf("COMMAND_START\n");
					if(dlnaPlayer == NULL){
						printf("new myDLNAPlayer()");
						dlnaPlayer = new myDLNAPlayer(info);
					}
				//sleep(90);
                    break;
                }

                case COMMAND_STOP:
                {
					printf("COMMAND_STOP\n");
					if(dlnaPlayer != NULL){
						printf("stop myDLNAPlayer() and delete ");
						dlnaPlayer->stopDMR();
						delete dlnaPlayer;
						dlnaPlayer = NULL;
					}
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
	if(dlnaPlayer != NULL){
		printf("delete dlnaPlayer\n");
		delete dlnaPlayer;
		dlnaPlayer = NULL;
	}
	printf("\n");
    printf("******************************************************************************************\n");
    printf("* Quit the program, goodbye!\n");
    printf("******************************************************************************************\n");
    printf("\n");
	return 0;
    //for(int i = 0; i < 100; i++)
    //{
    //    myDLNAPlayer listener(info);
    //    sleep(90);
    //}

    //record_stereo_test("/mnt/UDISK/test.pcm",16000);
    //while(1);
}
