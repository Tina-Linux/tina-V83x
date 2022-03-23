/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : awLogRecorder.h
* Description : log recoder for cmcc
* History :
*   Author  : AL3
*   Date    : 2015/05/05
*   Comment : first virsion
*
*/

#ifndef AW_LOG_RECORDER_H
#define AW_LOG_RECORDER_H

typedef void* AwLogRecorder;

AwLogRecorder* AwLogRecorderCreate(void);

void AwLogRecorderDestroy(AwLogRecorder* l);

//int AwLogRecorderRecord(AwLogRecorder* l, char* url, unsigned int nPlayTimeMs, int status);
int AwLogRecorderRecord(AwLogRecorder* l, char* cmccLog);

#endif
