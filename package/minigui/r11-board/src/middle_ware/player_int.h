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
#include <allwinner/tplayer.h>

int tplayer_init(TplayerVideoRotateType rotateDegree);
int tplayer_exit(void);
int tplayer_play_url(const char *parth);
int tplayer_play(void);
int tplayer_pause(void);
int tplayer_seekto(int nSeekTimeMs);
int tplayer_stop(void);
int tplayer_setlooping( bool bLoop);
int tplayer_setscaledown(TplayerVideoScaleDownType nHorizonScaleDown, TplayerVideoScaleDownType nVerticalScaleDown);
int tplayer_setrotate(TplayerVideoRotateType rotateDegree);
MediaInfo* tplayer_getmediainfo(void);
int tplayer_getduration(int* msec);
int tplayer_getcurrentpos(int* msec);
int tplayer_getcompletestate(void);
int tplayer_setdisplayrect(int x, int y, unsigned int width, unsigned int height);

#endif
