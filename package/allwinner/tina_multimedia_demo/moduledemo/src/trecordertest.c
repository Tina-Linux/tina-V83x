/*
 * Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : trecordertest.c
 * Description : trecorder functional test
 * History :
 *
 */
#include "trecordertest.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 400
#define FRONT_CAMERA 0
#define REAR_CAMERA 1

#define FRONT_CAPTURE_FRAMERATE 30
#define FRONT_CAPTURE_WIDTH 1920
#define FRONT_CAPTURE_HEIGHT 1080
#define FRONT_VIDEO_BITRATE 16*1000*1000
#define FRONT_VIDEO_FRAMERATE 30
#define FRONT_VIDEO_WIDTH 1920
#define FRONT_VIDEO_HEIGHT 1080
#define FRONT_MIC_SAMPLERATE 8000
#define FRONT_WM_POS_X 20
#define FRONT_WM_POS_Y 20

#define REAR_CAPTURE_FRAMERATE 25
#define REAR_CAPTURE_WIDTH 1280
#define REAR_CAPTURE_HEIGHT 720
#define REAR_VIDEO_BITRATE 8*1000*1000
#define REAR_VIDEO_FRAMERATE 25
#define REAR_VIDEO_WIDTH 1280
#define REAR_VIDEO_HEIGHT 720
#define REAR_MIC_SAMPLERATE 8000
#define REAR_WM_POS_X 20
#define REAR_WM_POS_Y 20

typedef struct Command
{
	const char* strCommand;
	int nCommandId;
	const char* strHelpMsg;
}Command;

#define COMMAND_HELP			0x1
#define COMMAND_QUIT			0x2
#define COMMAND_START_RECORD		0x3
#define COMMAND_STOP_RECORD		0x4
#define COMMAND_SWITCH_PREVIEW		0x5
#define COMMAND_DISABLE_PREVIEW		0x6
#define COMMAND_ENABLE_PREVIEW		0x7
#define COMMAND_SET_AUDIO_MUTE		0x8
#define COMMAND_SET_AUDIO_NORMAL	0x9
#define COMMAND_SET_LOOP_TIME		0xa
#define COMMAND_ENABLE_WATERMARK	0xb
#define COMMAND_DISABLE_WATERMARK	0xc
#define COMMAND_CAPTURE_PICTURE		0xd

static const Command commands[] =
{
	{"help",		COMMAND_HELP,			"show this help message."},
	{"quit",		COMMAND_QUIT,			"quit this program."},
	{"start",		COMMAND_START_RECORD,		"start record front and back media, 0--front, 1--back, 2--all."},
	{"stop",		COMMAND_STOP_RECORD,		"stop record front and back media, 0--front, 1--back, 2--all."},
	{"switch",	COMMAND_SWITCH_PREVIEW,		"switch the front and back camera preview"},
	{"notpreview",	COMMAND_DISABLE_PREVIEW,		"disable preview of front or back camera, 0--front, 1--back, 2--all."},
	{"setpreview",	COMMAND_ENABLE_PREVIEW,		"enable preview of front or back camera, 0--front, 1--back, 2--all."},
	{"setmute",	COMMAND_SET_AUDIO_MUTE,		"set front and back audio mute, 0--front, 1--back, 2--all."},
	{"notmute",	COMMAND_SET_AUDIO_NORMAL,	"set front and back audio normal, 0--front, 1--back, 2--all."},
	{"setmark",	COMMAND_ENABLE_WATERMARK,	 "enable front or back video water mark, 0--front, 1--back, 2--all."},
	{"notmark",	COMMAND_DISABLE_WATERMARK,	"disable front or back video water mark, 0--front, 1--back, 2--all."},
	{"capture",	COMMAND_CAPTURE_PICTURE,		"capture picture of front or back camera, 0--front, 1--back, 2--all."},
	{NULL, 0, NULL}
};

static int readCommand(char* strCommandLine, int nMaxLineSize)
{
	int nMaxFds;
	fd_set readFdSet;
	int result;
	unsigned int nReadBytes;
	char* p;

	printf("\nRecorderCmd# ");
	fflush(stdout);

	nMaxFds = 0;
	FD_ZERO(&readFdSet);
	FD_SET(STDIN_FILENO, &readFdSet);

	result = select(nMaxFds+1, &readFdSet, NULL, NULL, NULL);
	if (result > 0) {
		if (FD_ISSET(STDIN_FILENO, &readFdSet)) {
			nReadBytes = read(STDIN_FILENO, &strCommandLine[0], nMaxLineSize);
			if (nReadBytes > 0) {
				p = strCommandLine;
				while (*p != 0) {
					if (*p == 0xa) {
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
	int len;
	int i;

	if (strIn == NULL || (len=strlen(strIn)) == 0)
		return;

	ptrIn = strIn;
	ptrOut = strIn;
	i = 0;
	while (*ptrIn != '\0') {
		//* skip the beginning space or multiple space between words.
		if (*ptrIn != ' ' || (i!=0 && *(ptrOut-1)!=' ')) {
			*ptrOut++ = *ptrIn++;
			i++;
		}
		else
			ptrIn++;
	}

	//* skip the space at the tail.
	if (i==0 || *(ptrOut-1) != ' ')
		*ptrOut = '\0';
	else
		*(ptrOut-1) = '\0';

	return;
}

static int checkCommandParam(char* strParam, int* pParam)
{
	if (strParam != NULL) {
		*pParam = (int)atoi(strParam);
		if (errno == EINVAL || errno == ERANGE) {
			printf("command paramerter is invalid\n");
			return -1;
		}
		if(*pParam != 0 && *pParam != 1 && *pParam != 2) {
			printf("command paramerter is not right\n");
			return -1;
		}
	} else {
			printf("no command parameter\n");
			return -1;
	}

	return 0;
}

//* return command id,
static int parseCommandLine(char* strCommandLine, int* pParam)
{
	char* strCommand;
	char* strParam;
	int   i;
	int   nCommandId;
	char  colon = ' ';
	int ret;

	if (strCommandLine == NULL || strlen(strCommandLine) == 0)
		return -1;

	strCommand = strCommandLine;
	strParam = strchr(strCommandLine, colon);
	if (strParam != NULL) {
		*strParam = '\0';
		strParam++;
	}
	formatString(strCommand);
	formatString(strParam);

	nCommandId = -1;
	for (i=0; commands[i].strCommand != NULL; i++) {
		if(strcmp(commands[i].strCommand, strCommand) == 0) {
			nCommandId = commands[i].nCommandId;
			break;
		}
	}
	if (commands[i].strCommand == NULL)
		return -1;

	switch (nCommandId) {
        case COMMAND_SWITCH_PREVIEW:
		case COMMAND_START_RECORD:
		case COMMAND_STOP_RECORD:
		case COMMAND_DISABLE_PREVIEW:
		case COMMAND_ENABLE_PREVIEW:
		case COMMAND_SET_AUDIO_MUTE:
		case COMMAND_SET_AUDIO_NORMAL:
		case COMMAND_ENABLE_WATERMARK:
		case COMMAND_DISABLE_WATERMARK:
		case COMMAND_CAPTURE_PICTURE:
			ret = checkCommandParam(strParam, pParam);
			break;
        case COMMAND_HELP:
		case COMMAND_QUIT:
		default:
			break;
	}

	if (ret < 0)
		nCommandId = -1;

	return nCommandId;
}

static void showHelp(void)
{
	int i;

	printf("\n");
	printf("******************************************************************************\n");
	printf("* This is Tina recorder, when it is started, you can input commands to tell\n");
	printf("* what you want it to do.\n");
	printf("* Command and it param is seperated by a space, param is optional, as below:\n");
	printf("* Command [Param]\n");
	printf("*\n");
	printf("* here are the commands supported:\n");

	for(i=0; ; i++)
	{
		if(commands[i].strCommand == NULL)
			break;
		printf("*%s:\n", commands[i].strCommand);
		printf("*\t\t%s\n",  commands[i].strHelpMsg);
	}
	printf("*\n");
	printf("********************************************************\n");

	return;
}

void showHelpDemo()
{
	printf("****************************************************************************\n");
	printf("* moduledemo generalsrc 0: src module test\n");
	printf("* moduledemo generalsink 0: sink module test\n");
	printf("* moduledemo generalfilter 0: filter module test\n");
	printf("***************************************************************************\n");
}

void getRecorderStatus(RecorderStatusContext *RecorderStatus, char s[5][10])
{
	if (RecorderStatus->PreviewEnable)
		strcpy(s[0], "enable");
	else
		strcpy(s[0], "disable");
	if (RecorderStatus->PreviewRatioEnable)
		strcpy(s[1], "window");
	else
		strcpy(s[1], "full");
	if (RecorderStatus->AudioMuteEnable)
		strcpy(s[2], "mute");
	else
		strcpy(s[2], "normal");
	if (RecorderStatus->VideoMarkEnable)
		strcpy(s[3], "enable");
	else
		strcpy(s[3], "disable");
	if (RecorderStatus->RecordingEnable)
		strcpy(s[4], "start");
	else
		strcpy(s[4], "stop");

	return;
}
void showRecorderStatus(RecorderStatusContext *RecorderStatus, int number)
{
	char a[5][10] = {"      ", "    ", "    ", "      ", "     "};
	char b[5][10] = {"      ", "    ", "    ", "      ", "     "};

	if (number == 0)
		getRecorderStatus(&RecorderStatus[number], a);
	else if (number == 1)
		getRecorderStatus(&RecorderStatus[number], b);
	else if (number == 2) {
		getRecorderStatus(&RecorderStatus[0], a);
		getRecorderStatus(&RecorderStatus[1], b);
	}
	printf("---------------------------------------------------------------------------------------------\n");
	printf(L_RED"              | Preview Status | Preview Size | Audio Status | Water Mark | Recorder Status |\n"NONE);
	printf("---------------------------------------------------------------------------------------------\n");
	printf("      front   |     %-7s    |    %-6s    |    %-6s    |   %-7s  |      %-5s      |\n", a[0], a[1], a[2], a[3], a[4]);
	printf("---------------------------------------------------------------------------------------------\n");
	printf("      rear    |     %-7s    |    %-6s    |    %-6s    |   %-7s  |      %-5s      |\n", b[0], b[1], b[2], b[3], b[4]);
	printf("---------------------------------------------------------------------------------------------\n");

	return;
}

int CallbackFromTRecorder(void* pUserData, int msg, void* param)
{
    static int count;
    char num[][6] = {"front", "rear"};
	RecoderTestContext *trTestContext = (RecoderTestContext *)pUserData;

	if (!trTestContext) {
		printf("trTestContext is null\n");
		return -1;
	}

	switch(msg) {
		case T_RECORD_ONE_FILE_COMPLETE:
		{
            count++;
            printf(" file count %d\n", count);

			trTestContext->mRecorderFileCount++;
            if(trTestContext->mRecorderFileCount > 40)
                trTestContext->mRecorderFileCount = 0;

			memset(trTestContext->mRecorderPath,0,sizeof(trTestContext->mRecorderPath));

			snprintf(trTestContext->mRecorderPath, sizeof(trTestContext->mRecorderPath),
			            "/mnt/SDCARD/AW_%s_video%d.mp4", num[trTestContext->mRecorderId],
			            trTestContext->mRecorderFileCount);

			if (trTestContext->mTrecorder) {
				printf("change the output path to %s\n", trTestContext->mRecorderPath);
				TRchangeOutputPath(trTestContext->mTrecorder, trTestContext->mRecorderPath);
			}
			break;
		}
        case T_RECORD_ONE_AAC_FILE_COMPLETE:
		{
            count++;
            printf(" file count %d\n", count);

			trTestContext->mRecorderFileCount++;
            if(trTestContext->mRecorderFileCount > 40)
                trTestContext->mRecorderFileCount = 0;

			memset(trTestContext->mRecorderPath,0,sizeof(trTestContext->mRecorderPath));

			snprintf(trTestContext->mRecorderPath, sizeof(trTestContext->mRecorderPath),
			            "/mnt/SDCARD/AW_%s_audio%d.aac", num[trTestContext->mRecorderId],
			            trTestContext->mRecorderFileCount);

			if (trTestContext->mTrecorder) {
				printf("change the output path to %s\n", trTestContext->mRecorderPath);
				TRchangeOutputPath(trTestContext->mTrecorder, trTestContext->mRecorderPath);
			}
			break;
		}
		default:
		{
			printf("warning: unknown callback from trecorder\n");
			break;
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	int number, i;
	RecoderTestContext recorderTestContext[2];
	RecorderStatusContext RecorderStatus[2];
	char strCommandLine[256];
	int  nCommandParam;
	int  nCommandId;
	int  bQuit = 0;
	TCaptureConfig captureConfig;
	char jpegPath[128];
	int frontjpgNumber = 0;
	int rearjpgNumber = 0;
	int switchTimes = 0;
	TdispRect rect;

	printf("****************************************************************************\n");
	printf("* This program shows how to test trecorder\n");
	printf("****************************************************************************\n");

	if ((argc == 1)||((argc == 2)&&(strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0))) {
		showHelpDemo();
		return -1;
	}

	if((argc == 3)&&(strcmp(argv[1],"generalsrc")==0)){
        number = atoi(argv[2]);
        generalSrcTest(recorderTestContext, RecorderStatus, number);
    }else if((argc == 3)&&(strcmp(argv[1],"generalsink")==0)){
        number = atoi(argv[2]);
        generalSinkTest(recorderTestContext, RecorderStatus, number);
    }else if((argc == 3)&&(strcmp(argv[1],"generalfilter")==0)){
        number = atoi(argv[2]);
        generalFilterTest(recorderTestContext, RecorderStatus, number);
    }else{
		showHelpDemo();
		return -1;
	}

	while (!bQuit) {
		showRecorderStatus(RecorderStatus, number);
		if (readCommand(strCommandLine, sizeof(strCommandLine)) == 0) {
			//* parse command
			nCommandParam = 0;
			nCommandId = parseCommandLine(strCommandLine, &nCommandParam);
			//* process command.
			switch (nCommandId) {
				case COMMAND_HELP:
					showHelp();
					break;
				case COMMAND_QUIT:
                    if((argc == 3)&&(strcmp(argv[1],"generalsrc")==0))
                        generalSrcQuit(recorderTestContext, RecorderStatus, number);
                    else if((argc == 3)&&(strcmp(argv[1],"generalfilter")==0))
                        generalFilterQuit(recorderTestContext, RecorderStatus, number);
                    else if((argc == 3)&&(strcmp(argv[1],"generalsink")==0))
                        generalSinkQuit(recorderTestContext, RecorderStatus, number);

					bQuit = 1;
					break;
				case COMMAND_SWITCH_PREVIEW:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

                    if(nCommandParam == 0){
                        TRstop(recorderTestContext[1].mTrecorder, T_PREVIEW);
                        TRstart(recorderTestContext[0].mTrecorder, T_PREVIEW);
                        RecorderStatus[0].PreviewEnable= 1;
                        RecorderStatus[1].PreviewEnable= 0;
                    }else{
                        TRstop(recorderTestContext[0].mTrecorder, T_PREVIEW);
                        TRstart(recorderTestContext[1].mTrecorder, T_PREVIEW);
                        RecorderStatus[0].PreviewEnable= 0;
                        RecorderStatus[1].PreviewEnable= 1;
                    }
					break;
				case COMMAND_DISABLE_PREVIEW:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

                    if (nCommandParam < 2) {
						TRstop(recorderTestContext[nCommandParam].mTrecorder, T_PREVIEW);
						RecorderStatus[nCommandParam].PreviewEnable = 0;
					} else if (nCommandParam == 2) {
						TRstop(recorderTestContext[0].mTrecorder, T_PREVIEW);
						RecorderStatus[0].PreviewEnable = 0;
						TRstop(recorderTestContext[1].mTrecorder, T_PREVIEW);
						RecorderStatus[1].PreviewEnable = 0;
					}
					break;
				case COMMAND_ENABLE_PREVIEW:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

                    if (nCommandParam < 2) {
                        TRstart(recorderTestContext[nCommandParam].mTrecorder,T_PREVIEW);
                        RecorderStatus[nCommandParam].PreviewEnable = 1;
                    } else if  (nCommandParam == 2) {
                        TRstart(recorderTestContext[0].mTrecorder,T_PREVIEW);
                        RecorderStatus[0].PreviewEnable = 1;
                        TRstart(recorderTestContext[1].mTrecorder,T_PREVIEW);
                        RecorderStatus[1].PreviewEnable = 1;
                    }
					break;
				case COMMAND_SET_AUDIO_MUTE:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

					if (nCommandParam < 2) {
						TRsetAudioMute(recorderTestContext[0].mTrecorder, 1);
						RecorderStatus[nCommandParam].AudioMuteEnable = 1;
					} else {
						TRsetAudioMute(recorderTestContext[0].mTrecorder, 1);/*  only one mic */
						RecorderStatus[0].AudioMuteEnable = 1;
						RecorderStatus[1].AudioMuteEnable = 1;
					}
					break;
				case COMMAND_SET_AUDIO_NORMAL:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

					if (nCommandParam < 2) {
						TRsetAudioMute(recorderTestContext[nCommandParam].mTrecorder, 0);
						RecorderStatus[nCommandParam].AudioMuteEnable = 0;
					} else {
						TRsetAudioMute(recorderTestContext[0].mTrecorder, 0);/*  only one mic */
						RecorderStatus[0].AudioMuteEnable = 0;
						RecorderStatus[1].AudioMuteEnable = 0;
					}
					break;
				case COMMAND_START_RECORD:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

					if (nCommandParam < 2) {
						TRstart(recorderTestContext[nCommandParam].mTrecorder,T_RECORD);
						RecorderStatus[nCommandParam].RecordingEnable = 1;
					} else if (nCommandParam == 2) {
						for (i = 0; i < 2; i++) {
							TRstart(recorderTestContext[i].mTrecorder,T_RECORD);
							RecorderStatus[i].RecordingEnable = 1;
						}
					}
					break;
				case COMMAND_STOP_RECORD:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

					if (nCommandParam < 2) {
						TRstop(recorderTestContext[nCommandParam].mTrecorder,T_RECORD);
						RecorderStatus[nCommandParam].RecordingEnable = 0;
					} else if (nCommandParam == 2) {
						for(i = 0; i < 2; i++) {
							TRstop(recorderTestContext[i].mTrecorder,T_RECORD);
							RecorderStatus[i].RecordingEnable = 0;
						}
					}
					break;
				case COMMAND_ENABLE_WATERMARK:
					if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

                    if(nCommandParam < 2){
						TRsetCameraEnableWM(recorderTestContext[nCommandParam].mTrecorder,
									FRONT_WM_POS_X, FRONT_WM_POS_Y, 1);
						RecorderStatus[nCommandParam].VideoMarkEnable = 1;
                    }else if(nCommandParam == 2){
                        for(i = 0; i < number; i++){
							TRsetCameraEnableWM(recorderTestContext[i].mTrecorder,
										FRONT_WM_POS_X, FRONT_WM_POS_Y, 1);
							RecorderStatus[i].VideoMarkEnable = 1;
                        }
					}
					break;
				case COMMAND_DISABLE_WATERMARK:
                    if (nCommandParam > number || (number < 2 && nCommandParam != number)) {
						printf("invalid command.\n");
						break;
					}

                    if(nCommandParam < 2){
						TRsetCameraEnableWM(recorderTestContext[nCommandParam].mTrecorder,
									FRONT_WM_POS_X, FRONT_WM_POS_Y, 0);
						RecorderStatus[nCommandParam].VideoMarkEnable = 0;
                    }else if(nCommandParam == 2){
                        for(i = 0; i < number; i++){
							TRsetCameraEnableWM(recorderTestContext[i].mTrecorder,
										FRONT_WM_POS_X, FRONT_WM_POS_Y, 0);
							RecorderStatus[i].VideoMarkEnable = 0;
                        }
					}
					break;
				case COMMAND_CAPTURE_PICTURE:
                    if(nCommandParam > number || (number < 2 && nCommandParam != number)
                                                || nCommandParam >= 2){
						printf("invalid command %d.\n", nCommandParam);
						break;
					}
					captureConfig.captureFormat = T_CAPTURE_JPG;

                    memset(captureConfig.capturePath, 0, sizeof(captureConfig.capturePath));
					if (nCommandParam == 0) {
                        sprintf(captureConfig.capturePath, "/mnt/SDCARD/jpeg_front%d.jpg", frontjpgNumber);
						captureConfig.captureWidth = 1920;
						captureConfig.captureHeight = 1080;

                        frontjpgNumber++;
					} else {
					    sprintf(captureConfig.capturePath, "/mnt/SDCARD/jpeg_rear%d.jpg", rearjpgNumber);
						captureConfig.captureWidth = 1280;
						captureConfig.captureHeight = 720;

                        rearjpgNumber++;
					}

                    printf("jpegPath = %s\n",captureConfig.capturePath);
					TRCaptureCurrent(recorderTestContext[nCommandParam].mTrecorder,&captureConfig);
					break;
				default:
					if (strlen(strCommandLine) > 0)
						printf("invalid command.\n");
					break;
			}
		}
	}

	if (number == 0 || number == 1) {
		TRstop(recorderTestContext[number].mTrecorder,T_ALL);
		TRrelease(recorderTestContext[number].mTrecorder);
	} else if (number == 2) {
		for (i = 0; i < 2; i++) {
			TRstop(recorderTestContext[i].mTrecorder,T_ALL);
			TRrelease(recorderTestContext[i].mTrecorder);
		}
	}

END_PROGRAM:
	printf("\n");
	printf("*************************************************************************\n");
	printf("* Quit the program, goodbye!\n");
	printf("********************************************************************\n");
	printf("\n");

	return 0;
}
