#include "player_int.h"
#include "BottomMenu.h"
extern BottomMenuDataType *bottom;
#define CEDARX_UNUSE(param) (void)param
#define ISNULL(x) if(!x){return -1;}
#if 0
typedef struct PLAYER_CONTEXT_T
{
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
}player_context_t;
#endif
static player_context_t player_context;
/* a callback for tplayer. */
static int CallbackForTPlayer(void* pUserData, int msg, int param0,
		void* param1) {
	player_context_t* pPlayer = (player_context_t*) pUserData;

	CEDARX_UNUSE(param1);
	switch (msg) {
	case TPLAYER_NOTIFY_PREPARED: {
		printf("TPLAYER_NOTIFY_PREPARED,has prepared.\n");
		pPlayer->mPreparedFlag = 1;
		sem_post(&pPlayer->mPreparedSem);
		break;
	}
	case TPLAYER_NOTIFY_PLAYBACK_COMPLETE: {
		printf("TPLAYER_NOTIFY_PLAYBACK_COMPLETE\n");
		player_context.mCompleteFlag = 1;
		SendMessage(bottom->BottomMenuHwnd, MSG_USRPLAYEXIT, 0, 0);

		SendMessage(bottom->startMenuHwnd, MSG_USRPLAYEXIT, 0, 0);
		/* BroadcastMessage(MSG_USRPLAYEXIT, 0, 0); */
		break;
	}
	case TPLAYER_NOTIFY_SEEK_COMPLETE: {
		printf("TPLAYER_NOTIFY_SEEK_COMPLETE>>>>info: seek ok.\n");
		break;
	}
	case TPLAYER_NOTIFY_MEDIA_ERROR: {
		switch (param0) {
		case TPLAYER_MEDIA_ERROR_UNKNOWN: {
			printf("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN\n");
			break;
		}
		case TPLAYER_MEDIA_ERROR_UNSUPPORTED: {
			printf("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED\n");
			break;
		}
		case TPLAYER_MEDIA_ERROR_IO: {
			printf("erro type:TPLAYER_MEDIA_ERROR_IO\n");
			break;
		}
		}
		printf("error: open media source fail.\n");
		break;
	}
	case TPLAYER_NOTIFY_NOT_SEEKABLE: {
		pPlayer->mSeekable = 0;
		printf("info: media source is unseekable.\n");
		break;
	}
	case TPLAYER_NOTIFY_BUFFER_START: {
		printf("have no enough data to play\n");
		break;
	}
	case TPLAYER_NOTIFY_BUFFER_END: {
		printf("have enough data to play again\n");
		break;
	}
	case TPLAYER_NOTIFY_VIDEO_FRAME: {
		/* printf("get the decoded video frame\n"); */
		break;
	}
	case TPLAYER_NOTIFY_AUDIO_FRAME: {
		/* printf("get the decoded audio frame\n"); */
		break;
	}
	case TPLAYER_NOTIFY_SUBTITLE_FRAME: {
		/* printf("get the decoded subtitle frame\n"); */
		break;
	}
	default: {
		printf("warning: unknown callback from Tinaplayer.\n");
		break;
	}
	}
	return 0;
}

int tplayer_init(TplayerVideoRotateType rotateDegree) {
	/* * create a player. */
	player_context.mTPlayer = TPlayerCreate(CEDARX_PLAYER);
	if (player_context.mTPlayer == NULL) {
		printf("can not create tplayer, quit.\n");
		return -1;
	}
	/* * set callback to player. */
	TPlayerSetNotifyCallback(player_context.mTPlayer, CallbackForTPlayer,
			(void*) &player_context);
	/* set player start status */
	player_context.mError = 0;
	player_context.mSeekable = 1;
	player_context.mPreparedFlag = 0;
	player_context.mLoopFlag = 0;
	player_context.mSetLoop = 0;
	player_context.mMediaInfo = NULL;
	player_context.mCompleteFlag = 0;
	sem_init(&player_context.mPreparedSem, 0, 0);
	TPlayerReset(player_context.mTPlayer);
	TPlayerSetDebugFlag(player_context.mTPlayer, 0);
	TPlayerSetRotate(player_context.mTPlayer, rotateDegree);
	return 0;
}

int tplayer_loop(int bloop) {
	player_context.mSetLoop = bloop;
	return 0;
}
int tplayer_exit(void) {
	if (!player_context.mTPlayer) {
		printf("player not init.\n");
		return -1;
	}
	printf("join in\n");
	TPlayerReset(player_context.mTPlayer);
	printf("player reset\n");
	TPlayerDestroy(player_context.mTPlayer);
	printf("player destroy\n");
	player_context.mTPlayer = NULL;
	sem_destroy(&player_context.mPreparedSem);
	return 0;
}

int tplayer_play_url(const char *parth) {
	ISNULL(player_context.mTPlayer);
	TPlayerReset(player_context.mTPlayer);
	if (TPlayerSetDataSource(player_context.mTPlayer, parth, NULL) != 0) {
		printf("TPlayerSetDataSource() return fail.\n");
		return -1;
	} else {
		printf("setDataSource end\n");
	}
	player_context.mPreparedFlag = 0;
	if (TPlayerPrepareAsync(player_context.mTPlayer) != 0) {
		printf("TPlayerPrepareAsync() return fail.\n");
		return -1;
	} else {
		printf("prepare\n");
	}
	sem_wait(&player_context.mPreparedSem);
	printf("prepared ok\n");
	return 0;
}

int tplayer_play(void) {
	ISNULL(player_context.mTPlayer);
	if (!player_context.mPreparedFlag) {
		printf("not prepared!\n");
		return -1;
	}
	if (TPlayerIsPlaying(player_context.mTPlayer)) {
		printf("already palying!\n");
		return -1;
	}
	player_context.mCompleteFlag = 0;
	return TPlayerStart(player_context.mTPlayer);
}
int tplayer_setvolume(int volume) {
	TPlayerSetVolume(player_context.mTPlayer, volume);
	return 0;
}
int tplayer_getvolume() {
	return TPlayerGetVolume(player_context.mTPlayer);
}
int tplayer_pause(void) {
	ISNULL(player_context.mTPlayer);
	if (!TPlayerIsPlaying(player_context.mTPlayer)) {
		printf("not playing!\n");
		return -1;
	}
	return TPlayerPause(player_context.mTPlayer);
}

int tplayer_seekto(int nSeekTimeMs) {
	ISNULL(player_context.mTPlayer);
	if (!player_context.mPreparedFlag) {
		printf("not prepared!\n");
		return -1;
	}

	/*
	 if(TPlayerIsPlaying(player_context.mTPlayer)){
	 printf("seekto can not at palying state!\n");
	 return -1;
	 }
	 */
	return TPlayerSeekTo(player_context.mTPlayer, nSeekTimeMs);
}

int tplayer_stop(void) {
	ISNULL(player_context.mTPlayer);
	if (!player_context.mPreparedFlag) {
		printf("not prepared!\n");
		return -1;
	}
	return TPlayerStop(player_context.mTPlayer);
}

int tplayer_setlooping( bool bLoop) {
	ISNULL(player_context.mTPlayer);
	return TPlayerSetLooping(player_context.mTPlayer, bLoop);
}

int tplayer_setscaledown(TplayerVideoScaleDownType nHorizonScaleDown,
		TplayerVideoScaleDownType nVerticalScaleDown) {
	ISNULL(player_context.mTPlayer);
	return TPlayerSetScaleDownRatio(player_context.mTPlayer, nHorizonScaleDown,
			nVerticalScaleDown);
}

int tplayer_setdisplayrect(int x, int y, unsigned int width,
		unsigned int height) {
	ISNULL(player_context.mTPlayer);
	TPlayerSetDisplayRect(player_context.mTPlayer, x, y, width, height);

	return 0;
}

int tplayer_setrotate(TplayerVideoRotateType rotateDegree) {
	ISNULL(player_context.mTPlayer);
	return TPlayerSetRotate(player_context.mTPlayer, rotateDegree);
}

MediaInfo * tplayer_getmediainfo(void) {
	return TPlayerGetMediaInfo(player_context.mTPlayer);
}

int tplayer_getduration(int* msec) {
	ISNULL(player_context.mTPlayer);
	return TPlayerGetDuration(player_context.mTPlayer, msec);
}

int tplayer_getcurrentpos(int* msec) {
	ISNULL(player_context.mTPlayer);
	return TPlayerGetCurrentPosition(player_context.mTPlayer, msec);
}

int tplayer_getcompletestate(void) {
	return player_context.mCompleteFlag;
}
