/*
 * Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : __trecordertest.c
 * Description : trecorder functional test
 * History :
 *
 */
#include "__trecordertest.h"

#define VIDEO_ENCODING_FORMAT "video_encoding_format"
#define AUDIO_ENCODING_FORMAT "audio_encoding_format"
#define MUXER_FORMAT "muxer_format"

#define RECORDER_MAX_TIME "recorder_max_timeS"
#define RECORDER_MAX_FILES "recorder_max_files"
#define RECORDER_FILE_PATH "recorder_file_path"

#define AUDIO_VALID_FORMAT "PCM/AAC/MP3/NULL"
#define VIDEO_VALID_FORMAT "H264/MJPEG/NULL"
#define MUXER_VALID_FORMAT "AAC/MP3/MP4/TS/NULL"

static char lastRecordedFile[64] = {0,};

static char *pAudioEncodeFormatInfo = NULL;
static char *pVideoEncodeFormatInfo = NULL;
static char *pMuxerFormatInfo = NULL;
static char *pRecorderFilePath = NULL;
static int RecorderMaxTimeS = 0;
static int RecorderMaxFiles = 0;
static char RecorderFilePath[64] = {0,};
static int audioFormatReal = 0;
static char audioFormatArray[5][8] = {0,};
static int videoFormatReal = 0;
static char videoFormatArray[3][8] = {0,};
static int muxerFormatReal = 0;
static char muxerFormatArray[5][8] = {0,};


static TaudioEncodeFormat ReturnAudioEncodeFormat(char *audioformat)
{
	TaudioEncodeFormat audiotype = T_AUDIO_END;

	if (!strcmp(audioformat, "PCM"))
		audiotype = T_AUDIO_PCM;
	else if (!strcmp(audioformat, "AAC"))
		audiotype = T_AUDIO_AAC;
	else if (!strcmp(audioformat, "MP3"))
		audiotype = T_AUDIO_MP3;

    return audiotype;
}

static TvideoEncodeFormat ReturnVideoEncodeFormat(char *videoformat)
{
	TvideoEncodeFormat videotype = T_VIDEO_END;

	if (!strcmp(videoformat, "H264"))
		videotype = T_VIDEO_H264;
	else if (!strcmp(videoformat, "MJPEG"))
		videotype = T_VIDEO_MJPEG;

	return videotype;
}

static char *ReturnAudioEncodeFormatName(TaudioEncodeFormat audioformat)
{
	switch (audioformat) {
	case T_AUDIO_PCM:
		return "PCM";
	case T_AUDIO_AAC:
		return "AAC";
	case T_AUDIO_MP3:
		return "MP3";
	default:
		return "NULL";
	}
}

static char *ReturnVideoEncodeFormatName(TvideoEncodeFormat videoformat)
{
	switch (videoformat) {
	case T_VIDEO_H264:
		return "H264";
	case T_VIDEO_MJPEG:
		return "MJPEG";
	default:
		return "NULL";
	}
}

static ToutputFormat ReturnMuxerFormat(char *muxerformat)
{
	ToutputFormat muxertype = T_OUTPUT_END;

	if (!strcmp(muxerformat, "AAC"))
		muxertype = T_OUTPUT_AAC;
	else if (!strcmp(muxerformat, "MP3"))
		muxertype = T_OUTPUT_MP3;
	else if (!strcmp(muxerformat, "MP4"))
		muxertype = T_OUTPUT_MOV;
	else if (!strcmp(muxerformat, "TS"))
		muxertype = T_OUTPUT_TS;

	return muxertype;
}

static char *ReturnMuxerFormatName(ToutputFormat muxerformat)
{
	switch (muxerformat) {
	case T_OUTPUT_AAC:
		return "aac";
	case T_OUTPUT_MP3:
		return "mp3";
	case T_OUTPUT_MOV:
		return "mp4";
	case T_OUTPUT_TS:
		return "ts";
	default:
		return "mp4";
	}
}

static int stringSqlitToArray(char *srcString, char dstArray[][8], int num)
{
	int count = 0;
	char *srctemp = srcString;
	char *sqps = NULL;

	if (!srcString || !dstArray || num <= 0)
		return -1;

	while (srctemp && count < num) {
		sqps = strchr(srctemp, '/');
		if (sqps) {
			strncpy(dstArray[count++], srctemp, sqps-srctemp);
			srctemp = sqps + 1;
		} else {
			strncpy(dstArray[count++], srctemp, strlen(srctemp));
			srctemp = sqps;
		}
	}

	return count;
}

void parseArgv(char *argument, char *value)
{
	if (!argument || !value)
		return;

	if (!strcmp(argument, VIDEO_ENCODING_FORMAT))
		pVideoEncodeFormatInfo = value;
	else if (!strcmp(argument, AUDIO_ENCODING_FORMAT))
		pAudioEncodeFormatInfo = value;
	else if (!strcmp(argument, MUXER_FORMAT))
		pMuxerFormatInfo = value;
	else if (!strcmp(argument, RECORDER_MAX_TIME))
		sscanf(value, "%d", &RecorderMaxTimeS);
	else if (!strcmp(argument, RECORDER_MAX_FILES))
		sscanf(value, "%d", &RecorderMaxFiles);
	else if (!strcmp(argument, RECORDER_FILE_PATH))
		pRecorderFilePath = value;
}

int CallbackFromTRecorder(void* pUserData, int msg, void* param)
{
	char num[][6] = {"FRONT", "REAR"};
	RecoderTestContext *trTestContext = (RecoderTestContext *)pUserData;

	if (!trTestContext) {
		ERROR("trTestContext is null\n");
		return -1;
	}

	switch(msg) {
	case T_RECORD_ONE_FILE_COMPLETE:
	{
		trTestContext->mRecorderFileCount++;
		/* Record only one file */
		if (trTestContext->mRecorderFileCount > RecorderMaxFiles)
			sem_post(&trTestContext->CompSem);

		memset(trTestContext->mRecorderPath, 0, sizeof(trTestContext->mRecorderPath));
		if (trTestContext->outputFormat != T_OUTPUT_MP3) {
			snprintf(trTestContext->mRecorderPath, sizeof(trTestContext->mRecorderPath),
				"%s/%s_%s_%s_%d.%s", RecorderFilePath, num[trTestContext->mRecorderId],
				ReturnVideoEncodeFormatName(trTestContext->videoEncodeFormat),
				ReturnAudioEncodeFormatName(trTestContext->audioEncodeFormat),
				trTestContext->mRecorderFileCount,
				ReturnMuxerFormatName(trTestContext->outputFormat));
		} else {
			snprintf(trTestContext->mRecorderPath, sizeof(trTestContext->mRecorderPath),
				"%s/%s_%s_%d.mp3", RecorderFilePath, num[trTestContext->mRecorderId],
				ReturnAudioEncodeFormatName(trTestContext->audioEncodeFormat),
				trTestContext->mRecorderFileCount);

		}
		if (trTestContext->mTrecorder) {
			DEBUG("change the output path to %s\n", trTestContext->mRecorderPath);
			TRchangeOutputPath(trTestContext->mTrecorder, trTestContext->mRecorderPath);
		}
		break;
	}
	case T_RECORD_ONE_AAC_FILE_COMPLETE:
	{
		trTestContext->mRecorderFileCount++;
		/* Record only one file */
		if (trTestContext->mRecorderFileCount > RecorderMaxFiles)
			sem_post(&trTestContext->CompSem);

		memset(trTestContext->mRecorderPath, 0, sizeof(trTestContext->mRecorderPath));
		snprintf(trTestContext->mRecorderPath, sizeof(trTestContext->mRecorderPath),
			"%s/%s_%s_%d.aac", RecorderFilePath, num[trTestContext->mRecorderId],
			ReturnAudioEncodeFormatName(trTestContext->audioEncodeFormat),
			trTestContext->mRecorderFileCount);
		if (trTestContext->mTrecorder) {
			DEBUG("change the output path to %s\n", trTestContext->mRecorderPath);
			TRchangeOutputPath(trTestContext->mTrecorder, trTestContext->mRecorderPath);
		}
		break;
	}
	default:
	{
		ERROR("warning: unknown callback from trecorder\n");
		break;
	}
	}

	/* Save the last recorded file name and delete it when finish recording */
	memset(lastRecordedFile, 0, sizeof(lastRecordedFile));
	strncpy(lastRecordedFile, trTestContext->mRecorderPath, sizeof(lastRecordedFile));

	return 0;
}

static int startTest(RecoderTestContext *recorderTestContext,
				int videoIndex,
				TvideoEncodeFormat videoEncodeFormat,
				TaudioEncodeFormat audioEncodeFormat,
				ToutputFormat outputFormat)
{
	int ret = 0;
	char device[16] = {0,};

	/* Check the device exists */
	sprintf(device, "/dev/video%d", videoIndex);
	if (access(device, F_OK) != 0) {
		ERROR("%s : No such file or directory\n", device);
		return -1;
	}

	/* Create TRecorder handle */
	memset(recorderTestContext, 0, sizeof(RecoderTestContext));
	sem_init(&recorderTestContext->CompSem, 0, 0);
	recorderTestContext->mTrecorder = CreateTRecorder();
	if (!recorderTestContext->mTrecorder) {
		ERROR("CreateTRecorder error\n");
		return -1;
	}
	/* Reset Trecorder*/
	recorderTestContext->mRecorderId = videoIndex;
	recorderTestContext->videoEncodeFormat = videoEncodeFormat;
	recorderTestContext->audioEncodeFormat = audioEncodeFormat;
	recorderTestContext->outputFormat = outputFormat;
	TRreset(recorderTestContext->mTrecorder);

	if (videoEncodeFormat != T_VIDEO_END)
		TRsetCamera(recorderTestContext->mTrecorder, videoIndex);

	/* audio recording only supports audio 0 */
	if (audioEncodeFormat != T_AUDIO_END)
		TRsetAudioSrc(recorderTestContext->mTrecorder, 0);

	/* the currently set path is invalid */
	/* recording file path in callback function settings */
	TRsetOutput(recorderTestContext->mTrecorder, "/tmp");
	TRsetMaxRecordTimeMs(recorderTestContext->mTrecorder, RecorderMaxTimeS*1000);
	TRsetRecorderCallback(recorderTestContext->mTrecorder,
				CallbackFromTRecorder, (void*)recorderTestContext);

	if (videoEncodeFormat != T_VIDEO_END)
		TRsetVideoEncoderFormat(recorderTestContext->mTrecorder,
							videoEncodeFormat);
	/* audio recording only supports audio 0 */
	if (audioEncodeFormat != T_AUDIO_END)
		TRsetAudioEncoderFormat(recorderTestContext->mTrecorder,
							audioEncodeFormat);
	TRsetOutputFormat(recorderTestContext->mTrecorder, outputFormat);

	ret = TRprepare(recorderTestContext->mTrecorder);
	if (ret < 0) {
		ERROR("trecorder %d prepare error\n", videoIndex);
		return -1;
	}
	ret = TRstart(recorderTestContext->mTrecorder, T_RECORD);
	if (ret < 0) {
		ERROR("trecorder %d start error\n", videoIndex);
		return -1;
	}

	return 0;
}

static int stopTest(RecoderTestContext *recorderTestContext)
{
	if (!recorderTestContext)
		return -1;

	TRstop(recorderTestContext->mTrecorder, T_ALL);
	TRrelease(recorderTestContext->mTrecorder);

	sem_destroy(&recorderTestContext->CompSem);

	return 0;
}

int main(int argc, char** argv)
{
	int i = 0;
	int ret = 0;
	static int errorNum = 0;
	RecoderTestContext recorderTestContext;
	int muxerIndex, videoIndex, audioIndex;

	DEBUG("****************************************************************************\n");
	DEBUG("* This program test trecorder function\n");
	DEBUG("****************************************************************************\n");

	/* parse the config paramter */
	if (argc >= 2) {
		for(i = 1; i < (int)argc; i += 2)
			parseArgv(argv[i], argv[i + 1]);
	}

	/* get audio format,check validity */
	audioFormatReal = stringSqlitToArray(pAudioEncodeFormatInfo,
				audioFormatArray,
				sizeof(audioFormatArray)/sizeof(audioFormatArray[0]));
	for (i = 0; i < audioFormatReal; i++) {
		if (!strstr(AUDIO_VALID_FORMAT, audioFormatArray[i])) {
			ERROR(" Invalid audio encoding format:%s\n", audioFormatArray[i]);
			strcpy(audioFormatArray[i], "NULL");
			audioFormatReal = i;
			break;
		}
	}

	videoFormatReal = stringSqlitToArray(pVideoEncodeFormatInfo,
				videoFormatArray,
				sizeof(videoFormatArray)/sizeof(videoFormatArray[0]));
	for (i = 0; i < videoFormatReal; i++) {
		if (!strstr(VIDEO_VALID_FORMAT, videoFormatArray[i])) {
			ERROR(" Invalid video encoding format:%s\n", videoFormatArray[i]);
			strcpy(videoFormatArray[i], "NULL");
			videoFormatReal = i;
			break;
		}
	}
	muxerFormatReal = stringSqlitToArray(pMuxerFormatInfo,
				muxerFormatArray,
				sizeof(muxerFormatArray)/sizeof(muxerFormatArray[0]));
	for (i = 0; i < muxerFormatReal; i++) {
		if (!strstr(MUXER_VALID_FORMAT, muxerFormatArray[i])) {
			ERROR(" Invalid muxer format:%s\n", muxerFormatArray[i]);
			strcpy(muxerFormatArray[i], "NULL");
			muxerFormatReal = i;
			break;
		}
	}

	memset(RecorderFilePath, 0, sizeof(RecorderFilePath));
	strncpy(RecorderFilePath, pRecorderFilePath, sizeof(RecorderFilePath));
	if (access(RecorderFilePath, F_OK) != 0) {
		ERROR("%s : No such file or directory,set to /tmp\n", RecorderFilePath);
		memset(RecorderFilePath, 0, sizeof(RecorderFilePath));
		strncpy(RecorderFilePath, "/tmp", sizeof(RecorderFilePath));
	}

	DEBUG("-------------------- testtrecorder test info --------------------\n");
	DEBUG(" recording file save path: %s\n", RecorderFilePath);
	DEBUG(" file recording time: %d s\n", RecorderMaxTimeS);
	DEBUG(" file recording files number: %d\n", RecorderMaxFiles);
	DEBUG(" muxer format: ");
	for(i = 0; i < muxerFormatReal; i++)
		printf("%s ", muxerFormatArray[i]);
	printf("\n");
	DEBUG(" audio format: ");
	for(i = 0; i < audioFormatReal; i++)
		printf("%s ", audioFormatArray[i]);
	printf("\n");
	DEBUG(" video format: ");
	for(i = 0; i < videoFormatReal; i++)
		printf("%s ", videoFormatArray[i]);
	printf("\n");
	DEBUG("------------------------------ end ------------------------------\n");

	/* arrays instead of strings is better */
	for (muxerIndex = 0; muxerIndex < muxerFormatReal; muxerIndex++) {
		if (strstr("NULL", muxerFormatArray[muxerIndex]))
			continue;
		for (videoIndex = 0; videoIndex < videoFormatReal; videoIndex++) {
			if (strstr("AAC/MP3", muxerFormatArray[muxerIndex])
					&& !strstr("NULL", videoFormatArray[videoIndex]))
				continue;
			if (strstr("TS", muxerFormatArray[muxerIndex])
					&& !strstr("H264", videoFormatArray[videoIndex]))
				continue;
			if (strstr("MP4", muxerFormatArray[muxerIndex])
					&& strstr("NULL", videoFormatArray[videoIndex]))
				continue;
			for (audioIndex = 0; audioIndex < audioFormatReal; audioIndex++) {
				if (strstr("AAC", muxerFormatArray[muxerIndex])
						&& !strstr("AAC", audioFormatArray[audioIndex]))
					continue;
				if (strstr("MP3", muxerFormatArray[muxerIndex])
						&& !strstr("MP3", audioFormatArray[audioIndex]))
					continue;
				if (strstr("TS", muxerFormatArray[muxerIndex])
						&& !strstr("AAC/NULL", audioFormatArray[audioIndex]))
					continue;

				DEBUG(" start test trecorder\n");
				DEBUG("    muxer format %s\n", muxerFormatArray[muxerIndex]);
				DEBUG("    video format %s\n", videoFormatArray[videoIndex]);
				DEBUG("    audio format %s\n", audioFormatArray[audioIndex]);

				/* test */
				ret = startTest(&recorderTestContext, 0,
						ReturnVideoEncodeFormat(videoFormatArray[videoIndex]),
						ReturnAudioEncodeFormat(audioFormatArray[audioIndex]),
						ReturnMuxerFormat(muxerFormatArray[muxerIndex]));
				if (ret < 0) {
					ERROR(" startTEST error\n");
					ERROR("    muxer format %s\n", muxerFormatArray[muxerIndex]);
					ERROR("    video format %s\n", videoFormatArray[videoIndex]);
					ERROR("    audio format %s\n", audioFormatArray[audioIndex]);

					errorNum--;
					stopTest(&recorderTestContext);
					continue;
				}

				/* waiting for recording to complete */
				sem_wait(&recorderTestContext.CompSem);
				/* Wait for the callback function to complete, preventing errors */
				usleep(500*1000);

				stopTest(&recorderTestContext);

				if (access(lastRecordedFile, F_OK) == 0)
					remove(lastRecordedFile);

				DEBUG(" trecorder test case ok\n");
			}
		}
	}

	return errorNum;
}
