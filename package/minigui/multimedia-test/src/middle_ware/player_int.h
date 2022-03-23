#ifndef _PLAYER_INT_H_
#define _PLAYER_INT_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <tplayer.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define MSG_USRPLAYEXIT 103

typedef struct PLAYER_CONTEXT_T {
	TPlayer* mTPlayer;
	int mSeekable;
	int mError;
	int mVideoFrameNum;
	bool mPreparedFlag;
	bool mLoopFlag;
	bool mSetLoop;
	bool mCompleteFlag;
	char mUrl[512];
	MediaInfo* mMediaInfo;
	sem_t mPreparedSem;
} player_context_t;

int tplayer_init(TplayerVideoRotateType rotateDegree);
int tplayer_exit(void);
int tplayer_play_url(const char *parth);
int tplayer_play(void);
int tplayer_setvolume(int volume);
int tplayer_getvolume();
int tplayer_pause(void);
int tplayer_seekto(int nSeekTimeMs);
int tplayer_stop(void);
int tplayer_setlooping(bool bLoop);
int tplayer_loop(int bloop);
int tplayer_setscaledown(TplayerVideoScaleDownType nHorizonScaleDown,
		TplayerVideoScaleDownType nVerticalScaleDown);
int tplayer_setrotate(TplayerVideoRotateType rotateDegree);
MediaInfo* tplayer_getmediainfo(void);
int tplayer_getduration(int* msec);
int tplayer_getcurrentpos(int* msec);
int tplayer_getcompletestate(void);
int tplayer_setdisplayrect(int x, int y, unsigned int width,
		unsigned int height);

#endif
