#ifndef __TR_COMMON__
#define __TR_COMMON__

#include <stdio.h>
#include <sys/time.h>
#include "Video_input_port.h"
#include "vencoder.h"
#include "awencoder.h"

long long GetNowMs();
long long GetNowUs();
int GetCaptureFormat(int format);
char *get_format_name(unsigned int format);
int GetAudioTRFormat(char *name);
int GetVideoTRFormat(char *name);
char *ReturnVideoTRFormatText(int format);
int GetJpegSourceFormat(int TRformat);
int GetDecoderVideoSourceFormat(int format);
int GetDisplayFormat(int format);
int GetEncoderVideoSourceFormat(int format);
int GetEncoderAudioSourceFormat(int format);
int GetEncoderAudioType(char *typename);
int GetEncoderVideoType(char *typename);
int ReturnEncoderVideoType(int format);
int ReturnMediaAudioType(int type);
int ReturnMediaVideoType(int type);
int ReturnMuxerOuputType(char *typename);
void TRdebugEncoderAudioInfo(void *info);
void TRdebugEncoderVideoInfo(void *info);
void TRdebugMuxerMediaInfo(void *info);

#endif
