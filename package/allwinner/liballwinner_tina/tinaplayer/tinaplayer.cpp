#define LOG_TAG "TinaPlayer"
#include "tinaplayer.h"
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "awplayer.h"
#include "tinasoundcontrol.h"
#include "log.h"

#define SAVE_PCM_DATA 0
#define SAVE_YUV_DATA 0

#if SAVE_PCM_DATA
	FILE* savaPcmFd = NULL;
#endif

#if SAVE_YUV_DATA
	FILE* savaYuvFd = NULL;
#endif


namespace aw{

	static SoundCtrl* _SoundDeviceInit(void* pAudioSink){
		logd("_SoundDeviceInit()");
		SoundCtrl* soundCtrl = NULL;
		soundCtrl = TinaSoundDeviceInit(pAudioSink);
		if(soundCtrl == NULL){
			loge(" _SoundDeviceInit(),ERR:soundCtrl == NULL");
		}
		return soundCtrl;
	}

	static void _SoundDeviceRelease(SoundCtrl* s){
		logd(" _SoundDeviceRelease()");
		TinaSoundDeviceRelease(s);
	}

	static void _SoundDeviceSetFormat(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		logd(" _SoundDeviceSetFormat()");
		TinaSoundDeviceSetFormat(s, nSampleRate, nChannelNum);
	}

	static int _SoundDeviceStart(SoundCtrl* s){
		logd(" _SoundDeviceStart()");
		//system("amixer cset numid=27 0");
		return TinaSoundDeviceStart(s);
	}

	static int _SoundDeviceStop(SoundCtrl* s){
		logd(" _SoundDeviceStop()");
		return TinaSoundDeviceStop(s);
	}

	static int _SoundDevicePause(SoundCtrl* s){
		logd(" _SoundDevicePause()");
		return TinaSoundDevicePause(s);
	}

	static int _SoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize){
		//logd(" _SoundDeviceWrite(),nDataSize = %d",nDataSize);
		int ret = TinaSoundDeviceWrite(s, pData, nDataSize);
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				int write_ret = fwrite(pData, 1, nDataSize, savaPcmFd);
				//logd("PCM write_ret = %d",write_ret);
				if(write_ret <= 0){
					loge("_SoundDeviceWrite error,err str: %s",strerror(errno));
				}
			}
		#endif
		//logd(" _SoundDeviceWrite(),finish ");
		return ret;
	}

	static int _SoundDeviceReset(SoundCtrl* s){
		logd(" _SoundDeviceReset()");
		return TinaSoundDeviceReset(s);
	}

	static int _SoundDeviceGetCachedTime(SoundCtrl* s){
		//logd(" _SoundDeviceGetCachedTime()");
		return TinaSoundDeviceGetCachedTime(s);
	}


	static SoundCtrl* _SoundDeviceInit_raw(void* raw_data,void* hdeccomp,RawCallback callback){
		return TinaSoundDeviceInit_raw(raw_data,hdeccomp,callback);
	}

	static void _SoundDeviceRelease_raw(SoundCtrl* s){
		TinaSoundDeviceRelease_raw(s);
	}

	static void _SoundDeviceSetFormat_raw(SoundCtrl* s, unsigned int nSampleRate, unsigned int nChannelNum){
		TinaSoundDeviceSetFormat_raw(s, nSampleRate, nChannelNum);
	}

	static int _SoundDeviceStart_raw(SoundCtrl* s){
		return TinaSoundDeviceStart_raw(s);
	}

	static int _SoundDeviceStop_raw(SoundCtrl* s){
		return TinaSoundDeviceStop_raw(s);
	}

	static int _SoundDevicePause_raw(SoundCtrl* s){
		return TinaSoundDevicePause_raw(s);
	}

	static int _SoundDeviceWrite_raw(SoundCtrl* s, void* pData, int nDataSize){
		return TinaSoundDeviceWrite_raw(s, pData, nDataSize);
	}

	static int _SoundDeviceReset_raw(SoundCtrl* s){
		return TinaSoundDeviceReset_raw(s);
	}

	static int _SoundDeviceGetCachedTime_raw(SoundCtrl* s){
		return TinaSoundDeviceGetCachedTime_raw(s);
	}

	static int _SoundDeviceSetVolume(SoundCtrl* s, float volume){
		logd(" _SoundDeviceSetVolume()");
		return TinaSoundDeviceSetVolume(s, volume);
	}

	static int _SoundDeviceGetVolume(SoundCtrl* s, float *volume){
		logd(" _SoundDeviceGetVolume()");
		return TinaSoundDeviceGetVolume(s, volume);
	}

	static int _SoundDeviceSetCallback (SoundCtrl* s, SndCallback callback, void* pUserData){
		logd(" _SoundDeviceSetCallback()");
		return TinaSoundDeviceSetCallback(s, callback, pUserData);
	}

	static SoundControlOpsT gSoundControl;

	//* a callback for awplayer.
	void CallbackForAwPlayer(void* pUserData, int msg, int param0, void* param1)
	{
		int app_msg = 0;
		TinaPlayer* p = NULL;
		p = (TinaPlayer*)pUserData;
	    switch(msg)
	    {
	        case NOTIFY_NOT_SEEKABLE:
	        {
				logd(" ***NOTIFY_NOT_SEEKABLE***");
				app_msg = TINA_NOTIFY_NOT_SEEKABLE;
	            break;
	        }

	        case NOTIFY_ERROR:
	        {
				logd(" ****NOTIFY_ERROR***");
				app_msg = TINA_NOTIFY_ERROR;
	            break;
	        }

	        case NOTIFY_PREPARED:
	        {
				logd(" ***NOTIFY_PREPARED***");
				app_msg = TINA_NOTIFY_PREPARED;
	            break;
	        }

	        case NOTIFY_BUFFERRING_UPDATE:
	        {
	            //int nBufferedFilePos;
	            //int nBufferFullness;

	            //nBufferedFilePos = param0 & 0x0000ffff;
	            //nBufferFullness  = (param0>>16) & 0x0000ffff;
	            //TLOGD("info: buffer %d percent of the media file, buffer fullness = %d percent.\n",
	            //    nBufferedFilePos, nBufferFullness);
	            //TLOGD(" NOTIFY_BUFFERRING_UPDATE\n");
				//app_msg = TINA_NOTIFY_BUFFERRING_UPDATE;
	            break;
	        }

	        case NOTIFY_PLAYBACK_COMPLETE:
	        {
	            logd(" ****NOTIFY_PLAYBACK_COMPLETE****");
				if(p){
					if(p->mLoop == 0){
						app_msg = TINA_NOTIFY_PLAYBACK_COMPLETE;
					}
				}
	            break;
	        }

	        case NOTIFY_RENDERING_START:
	        {
	            logd(" NOTIFY_RENDERING_START");
				app_msg = TINA_NOTIFY_RENDERING_START;
	            break;
	        }

	        case NOTIFY_SEEK_COMPLETE:
	        {
	            logd(" NOTIFY_SEEK_COMPLETE****");
				app_msg = TINA_NOTIFY_SEEK_COMPLETE;
				if(param1 != NULL){
					logd(" seek to result = %d",((int*)param1)[0]);
					logd(" seek to %d ms",((int*)param1)[1]);
				}
	            break;
	        }

			case NOTIFY_BUFFER_START:
	        {
	            logd("NOTIFY_BUFFER_START:no enough data to play");
				app_msg = TINA_NOTIFY_BUFFER_START;
	            break;
	        }

			case NOTIFY_BUFFER_END:
	        {
	            logd(" NOTIFY_BUFFER_END:has enough data to play again");
				app_msg = TINA_NOTIFY_BUFFER_END;
	            break;
	        }

	        case NOTIFY_VIDEO_FRAME:
	        {
				#if SAVE_YUV_DATA
					VideoPicData* videodata = (VideoPicData*)param1;
					if(videodata){
						//logd("*****NOTIFY_VIDEO_FRAME****,videodata->nPts = %lld ms",videodata->nPts/1000);
						if(savaYuvFd!=NULL){
							if(p->mVideoFrameNum == 200){
								logd(" *****NOTIFY_VIDEO_FRAME****,videodata->ePixelFormat = %d,videodata->nWidth = %d,videodata->nHeight=%d",videodata->ePixelFormat,videodata->nWidth,videodata->nHeight);
								int write_ret0 = fwrite(videodata->pData0, 1, videodata->nWidth*videodata->nHeight, savaYuvFd);
								if(write_ret0 <= 0){
									loge("yuv write0 error,err str: %s",strerror(errno));
								}
								int write_ret1 = fwrite(videodata->pData1, 1, videodata->nWidth*videodata->nHeight/2, savaYuvFd);
								if(write_ret1 <= 0){
									loge("yuv write1 error,err str: %s",strerror(errno));
								}
								logd("only save 1 video frame\n");
								fclose(savaYuvFd);
								savaYuvFd = NULL;
							}
							p->mVideoFrameNum++;
							//if(p->mVideoFrameNum >= 100){
							//	logd("only save 100 video frame\n");
							//	fclose(savaYuvFd);
							//	savaYuvFd = NULL;
							//}
						}
					}
				#endif
				app_msg = TINA_NOTIFY_VIDEO_FRAME;
				break;
	        }

	        case NOTIFY_AUDIO_FRAME:
	        {
				#if SAVE_PCM_DATA
					AudioPcmData* pcmData = (AudioPcmData*)param1;
					if(pcmData){
						if(savaPcmFd!=NULL){
							//logd(" *****NOTIFY_AUDIO_FRAME#####,*pcmData->pData = %p,pcmData->nSize = %d",*(pcmData->pData),pcmData->nSize);
							int write_ret = fwrite(pcmData->pData, 1, pcmData->nSize, savaPcmFd);
							if(write_ret <= 0){
								loge("pcm write error,err str: %s",strerror(errno));
							}
							p->mAudioFrameNum++;
							if(p->mAudioFrameNum >= 500){
								logd("only save 500 audio frame\n");
								fclose(savaPcmFd);
								savaPcmFd = NULL;
							}
						}
					}
				#endif
				app_msg = TINA_NOTIFY_AUDIO_FRAME;
				break;
	        }

	        case NOTIFY_VIDEO_PACKET:
	        {
			//DemuxData* videoData = (DemuxData*)param1;
				//logd("videoData pts: %lld", videoData->nPts);
				//static int frame = 0;
				//if(frame == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/video.jpg", "wb");
		        //	if(videoData->nSize0)
		        //	{
		        //		fwrite(videoData->pData0, 1, videoData->nSize0, outFp);
		        //	}
		        //	if(videoData->nSize1)
		        //	{
		        //		fwrite(videoData->pData1, 1, videoData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	frame ++;
			//}
			//TLOGD(" NOTIFY_VIDEO_PACKET\n");
			//app_msg = TINA_NOTIFY_VIDEO_PACKET;
			break;
	        }

	        case NOTIFY_AUDIO_PACKET:
	        {
			//DemuxData* audioData = (DemuxData*)param1;
				//logd("audio pts: %lld", audioData->nPts);
				//static int audioframe = 0;
				//if(audioframe == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/audio.pcm", "wb");
		        //	if(audioData->nSize0)
		        //	{
		        //		fwrite(audioData->pData0, 1, audioData->nSize0, outFp);
		        //	}
		        //	if(audioData->nSize1)
		        //	{
		        //		fwrite(audioData->pData1, 1, audioData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	audioframe ++;
			//}
			//TLOGD(" NOTIFY_AUDIO_PACKET\n");
			//app_msg = TINA_NOTIFY_AUDIO_PACKET;
			break;

	        }

	        default:
	        {
	            logd(" warning: unknown callback from AwPlayer");
	            break;
	        }
	    }
		if(app_msg != 0){
			if(p){
				p->callbackToApp(app_msg, param0, param1);
			}
		}
	}
	TinaPlayer::TinaPlayer()
	{
		logd(" TinaPlayer() contructor begin");
		mLoop = 0;
		mNotifier = NULL;
		mUserData = NULL;
		mVideoFrameNum = 0;
		mAudioFrameNum = 0;
		mVolume = 20;
		mPlayer = (void*)new AwPlayer();
		initSoundControlOpsT();
		((AwPlayer*)mPlayer)->setControlOps(NULL, &gSoundControl);
		logd(" TinaPlayer() contructor finish");
	}

	TinaPlayer::~TinaPlayer()
	{
		logd(" ~TinaPlayer() contructor begin");
		if(((AwPlayer*)mPlayer) != NULL){
			delete ((AwPlayer*)mPlayer);
		mPlayer = NULL;
		}
		logd(" ~TinaPlayer() contructor finish");
	}

	void TinaPlayer::initSoundControlOpsT(){
		gSoundControl.SoundDeviceInit = _SoundDeviceInit;
		gSoundControl.SoundDeviceRelease = _SoundDeviceRelease;
		gSoundControl.SoundDeviceSetFormat = _SoundDeviceSetFormat;
		gSoundControl.SoundDeviceStart = _SoundDeviceStart;
		gSoundControl.SoundDeviceStop = _SoundDeviceStop;
		gSoundControl.SoundDevicePause = _SoundDevicePause;
		gSoundControl.SoundDeviceWrite = _SoundDeviceWrite;
		gSoundControl.SoundDeviceReset = _SoundDeviceReset;
		gSoundControl.SoundDeviceGetCachedTime = _SoundDeviceGetCachedTime;
		gSoundControl.SoundDeviceInit_raw = _SoundDeviceInit_raw;
		gSoundControl.SoundDeviceRelease_raw = _SoundDeviceRelease_raw;
		gSoundControl.SoundDeviceSetFormat_raw = _SoundDeviceSetFormat_raw;
		gSoundControl.SoundDeviceStart_raw = _SoundDeviceStart_raw;
		gSoundControl.SoundDeviceStop_raw = _SoundDeviceStop_raw;
		gSoundControl.SoundDevicePause_raw = _SoundDevicePause_raw;
		gSoundControl.SoundDeviceWrite_raw = _SoundDeviceWrite_raw;
		gSoundControl.SoundDeviceReset_raw = _SoundDeviceReset_raw;
		gSoundControl.SoundDeviceGetCachedTime_raw = _SoundDeviceGetCachedTime_raw;
		gSoundControl.SoundDeviceSetVolume = _SoundDeviceSetVolume;
		gSoundControl.SoundDeviceGetVolume = _SoundDeviceGetVolume;
		gSoundControl.SoundDeviceSetCallback = _SoundDeviceSetCallback;
	}

	int TinaPlayer::initCheck()
	{
		return ((AwPlayer*)mPlayer)->initCheck();
	}

	int TinaPlayer::setNotifyCallback(NotifyCallback notifier, void* pUserData)
	{
		mNotifier = notifier;
		mUserData = pUserData;
		return ((AwPlayer*)mPlayer)->setNotifyCallback(CallbackForAwPlayer, (void*)this);
	}

	int TinaPlayer::setDataSource(const char* pUrl, const map<string, string>* pHeaders)
	{
		return ((AwPlayer*)mPlayer)->setDataSource(pUrl, pHeaders);
	}


	int TinaPlayer::prepareAsync()
	{
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::prepareAsync() begin,tid = %d",tid);
		int ret = ((AwPlayer*)mPlayer)->prepareAsync();
		logd("TinaPlayer::prepareAsync() finish,tid = %d",tid);
		return ret;
	}

	int TinaPlayer::prepare()
	{
		return ((AwPlayer*)mPlayer)->prepare();
	}

	int TinaPlayer::start()
	{
		#if SAVE_PCM_DATA
			savaPcmFd = fopen("/tmp/save.pcm", "wb");
			if(savaPcmFd==NULL){
				loge("fopen save.pcm fail****");
				loge("err str: %s",strerror(errno));
			}else{
				fseek(savaPcmFd,0,SEEK_SET);
			}
		#endif
		#if SAVE_YUV_DATA
			savaYuvFd = fopen("/tmp/save.yuv", "wb");
			if(savaYuvFd==NULL){
				loge("fopen save.yuv fail****");
				loge("err str: %s",strerror(errno));
			}else{
				fseek(savaYuvFd,0,SEEK_SET);
			}
		#endif
		int ret = ((AwPlayer*)mPlayer)->start();
		return ret;
	}

	int TinaPlayer::stop()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		#if SAVE_YUV_DATA
			if(savaYuvFd!=NULL){
				fclose(savaYuvFd);
				savaYuvFd = NULL;
			}
		#endif
		return ((AwPlayer*)mPlayer)->stop();
	}

	int TinaPlayer::pause()
	{
		return ((AwPlayer*)mPlayer)->pause();
	}

	int TinaPlayer::seekTo(int msec)
	{
		return ((AwPlayer*)mPlayer)->seekTo(msec);
	}

	int TinaPlayer::reset()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		#if SAVE_YUV_DATA
			if(savaYuvFd!=NULL){
				fclose(savaYuvFd);
				savaYuvFd = NULL;
			}
		#endif
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::reset() begin,tid = %d",tid);
		struct timeval time1, time2, time3;
		memset(&time3, 0, sizeof(struct timeval));
		gettimeofday(&time1, NULL);
		int ret = ((AwPlayer*)mPlayer)->reset();
		gettimeofday(&time2, NULL);
		time3.tv_sec += (time2.tv_sec-time1.tv_sec);
		time3.tv_usec += (time2.tv_usec-time1.tv_usec);
		logd("TinaPlayer::reset() >>> time elapsed: %ld seconds  %ld useconds\n", time3.tv_sec, time3.tv_usec);
		logd("TinaPlayer::reset() finish,tid = %d",tid);
		return ret;
	}


	int TinaPlayer::isPlaying()
	{
		return ((AwPlayer*)mPlayer)->isPlaying();
	}


	int TinaPlayer::getCurrentPosition(int* msec)
	{
		return ((AwPlayer*)mPlayer)->getCurrentPosition(msec);
	}


	int TinaPlayer::getDuration(int *msec)
	{
		return ((AwPlayer*)mPlayer)->getDuration(msec);
	}


	int TinaPlayer::setLooping(int loop)
	{
		mLoop = loop;
		return ((AwPlayer*)mPlayer)->setLooping(loop);
	}

	/*now,this function do not do */
	int TinaPlayer::setVolume(int volume)
	{
		if(volume > 40){
			loge("the volume(%d) is larger than the largest volume(40),set it to 40",volume);
			volume = 40;
		}else if(volume < 0){
			loge("the volume(%d) is smaller than 0,set it to 0",volume);
			volume =0;
		}
		mVolume = volume;
		volume -= 20;
		return ((AwPlayer*)mPlayer)->setVolume(volume);
	}

	int TinaPlayer::getVolume()
	{
		return mVolume;
	}

	void TinaPlayer::callbackToApp(int msg, int param0, void* param1){
		if(mNotifier){
			mNotifier(mUserData,msg,param0,param1);
		}else{
			loge(" mNotifier is null ");
		}
	}

	int TinaPlayer::setVideoOutputScaleRatio(int horizonScaleDownRatio,int verticalScaleDownRatio){
		return ((AwPlayer*)mPlayer)->setVideoOutputScaleRatio(horizonScaleDownRatio,verticalScaleDownRatio);
	}

}
