/*************************************************************************
	> File Name: display.c
	> Author:
	> Mail:
	> Created Time: 2017年09月21日 星期四 15时15分51秒
 ************************************************************************/

#include "display.h"

typedef struct Command
{
    const char* strCommand;
    int         nCommandId;
    const char* strHelpMsg;

}Command;

#define COMMAND_HELP                0x1
#define COMMAND_QUIT                0x2
#define COMMAND_SWITCH_FRONTBACK    0x3
#define COMMAND_START_RECORD        0x4
#define COMMAND_STOP_RECORD         0x5
#define COMMAND_CAPTURE_PICTURE     0x6
#define COMMAND_DISABLE_PREVIEW     0x7
#define COMMAND_ENABLE_PREVIEW      0x8
#define COMMAND_SET_AUDIO_MUTE      0x9

static const Command commands[] =
{
        {"help",            COMMAND_HELP,               "show this help message."},
        {"quit",            COMMAND_QUIT,               "quit this program."},
        {"switch",         COMMAND_SWITCH_FRONTBACK,
                "switch the front and back camera preview"},
        {"start",            COMMAND_START_RECORD,               "start record two way video."},
        {"stop",           COMMAND_STOP_RECORD,              "stop record two way audio."},
        {"capture",      COMMAND_CAPTURE_PICTURE,
                "capture picture of front or back camera.capture 0(capture front),capture 1:capture back."},
        {"disable",   COMMAND_DISABLE_PREVIEW,
                "disable preview of front or back camera.disable 0:disable front,disable 1:disable back."},

        {"enable",   COMMAND_ENABLE_PREVIEW,
                "enable preview of front or back camera.enable 0:enable front,enable 1:enable back."},
        {"setmute",   COMMAND_SET_AUDIO_MUTE,
                "set the recorder0 mute or not,set mute:1 means mute,set mute:0 means not mute"},
        {NULL, 0, NULL}

};

static int readCommand(char *strCommandLine, int nMaxLineSize)
{
    int nMaxFds;
    fd_set readFdSet;
    int result;
    int nReadBytes;

    printf("display# ");
    fflush(stdout);

    nMaxFds = 0;
    FD_ZERO(&readFdSet);
    FD_SET(STDIN_FILENO, &readFdSet);

    result = select(nMaxFds+1, &readFdSet, NULL, NULL, NULL);
    if(result > 0)
    {
        if(FD_ISSET(STDIN_FILENO, &readFdSet))
        {
            nReadBytes = read(STDIN_FILENO, &strCommandLine[0], nMaxLineSize);
			if(strCommandLine[0] == '\n')
				return -1;

            if(nReadBytes > 0)
                return 0;
        }
    }

    return -1;
}

static void formatString(char *strIn)
{
    char *ptrIn;

    if(strIn == NULL || strlen(strIn) == 0)
        return;

    ptrIn = strIn;
    while(*ptrIn != '\0')
    {
        if(*ptrIn == ' ' || *ptrIn == '\r' || *ptrIn == '\n')
            *ptrIn = '\0';
        else
            ptrIn++;
    }

    return;
}

static int parseCommand(char *strCommandLine)
{
    char *strCommand = strCommandLine;
    int i;
    int nCommandId = -1;

    if(strCommand == NULL || strlen(strCommand) == 0)
        return -1;

    formatString(strCommand);
    //CAMPLAY_DBG_LOG(" parsecommand = %s\n",strCommand);
    for(i=0; commands[i].strCommand != NULL; i++)
    {
        //CAMPLAY_DBG_LOG(" commands[%d].strCommand = %s command = %s\n",i,commands[i].strCommand,strCommand);
        if(strcmp(commands[i].strCommand, strCommand) == 0)
        {
            //CAMPLAY_DBG_LOG(" match command success\n");
            nCommandId = commands[i].nCommandId;
            break;
        }
    }

    if(commands[i].strCommand == NULL)
        return -1;

    return nCommandId;
}

static void showHelp(void)
{
    int     i;

    printf("\n");
    printf("******************************************************************************\n");
    printf("* This is camera play, when it is started, you can input commands to tell\n");
    printf("* what you want it to do.\n");
    printf("* Usage: \n");
    printf("*   # switch: switch the back and front preview\n");
    printf("*   # stop:stop record of front and back camera\n");
    printf("*   # start:start record of the front and back camera\n");
    printf("*   # capture:capture picture of the front or back camera\n");
    printf("*   # disable:disable layer of front or back camera\n");
    printf("*   # enable:enable layer of front or back camera\n");
    printf("*\n");
    printf("* Command and it param is seperated by a space, param is optional, as below:\n");
    printf("*     Command [Param]\n");
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
    printf("******************************************************************************\n");
}



int main(int argc,char *argv[])
{
	int ret = 0;
    char command[32];
    int commandId;
    cameraplay *camdisp;

    camdisp = malloc(sizeof(cameraplay));
	camdisp->quitFlag = 0;
	camdisp->saveframe_Flag = 0;

    cameraInit(&camdisp->capture);
    camera_dispInit(camdisp);

    /* start capture */
    camdisp->capture.tid = 0;
    cameraStartCapture(&camdisp->capture);
    ret = pthread_create(&camdisp->capture.tid, NULL, CameraThread, (void *)camdisp);
    CAMPLAY_DBG_LOG(" Create camera capture thread ret：%d\n",ret);

    /* start display */
    camdisp->display.tid = 0;
    ret = pthread_create(&camdisp->display.tid, NULL, DisplayThread, (void *)camdisp);
    CAMPLAY_DBG_LOG(" Create camera display thread ret：%d\n",ret);

    /* clean command*/
    memset(command, 0, sizeof(command));
    while(!camdisp->quitFlag)
    {
        if(readCommand(command,sizeof(command)) == 0)
        {
            /* parse command */
            commandId = parseCommand(command);

            /* process command */
            switch(commandId)
            {
                case COMMAND_HELP:
                {
                    showHelp();
                    break;
                }
                case COMMAND_QUIT:
                {
                    camdisp->quitFlag = 1;
					CAMPLAY_DBG_LOG(" quit command\n");
                    break;
                }
                case COMMAND_CAPTURE_PICTURE:
                {
                    camdisp->saveframe_Flag = 1;
					CAMPLAY_DBG_LOG(" capture command\n");
                    break;
                }
                default:
                {
                    CAMPLAY_DBG_LOG(" Invalid command %d\n",commandId);
                    break;
                }
            }
        }

        /* clean command*/
        memset(command, 0, sizeof(command));
    }

    pthread_join(camdisp->capture.tid, NULL);
    pthread_join(camdisp->display.tid, NULL);

    CAMPLAY_DBG_LOG(" finally completed\n");

    free(camdisp);

    return 0;
}
