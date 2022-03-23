#include "general_sink.h"
#include "trecordertest.h"

#define SINK_AUDIO_INPUT (0)
#define SINK_VIDEO_INPUT (1)


static int generalSinkInit(struct generalSink* handle)
{
    int ret;

    if(!handle)
        return -1;

    memset(&handle->generalSinkPort, 0x00, sizeof(struct moduleAttach));

    ModuleSetNotifyCallback(&handle->generalSinkPort,
                            NotifyCallbackToSink, &handle->generalSinkPort);

    handle->generalSinkPort.name = T_CUSTOM;

#if SINK_VIDEO_INPUT
    handle->generalSinkPort.inputTyte |= (VIDEO_YUV | SCALE_YUV | VIDEO_ENC);
#endif

#if SINK_AUDIO_INPUT
    handle->generalSinkPort.inputTyte |= (AUDIO_PCM | AUDIO_ENC);
#endif

    return 0;
}

static void *generalSinkThread(void *param)
{
	struct generalSink *hdl = (struct generalSink*)param;
    struct modulePacket *InPacket = NULL;
    struct MediaPacket *videoInputbuf = NULL;
    struct AudioPacket *audioInputbuf = NULL;
    FILE *audioFp = NULL;
    char file_path[64];
    int index = 0;
    int ret = 0;

    /* determine the type of input */
    if(hdl->generalSinkPort.src_name & T_AUDIO){
        printf(" src module is audio\n");

        memset(file_path, 0, sizeof(file_path));
        sprintf(file_path, "/tmp/TRaudio_test.pcm");

        /* create audio test file */
        audioFp = fopen(file_path, "wb+");
    }else if(hdl->generalSinkPort.src_name & T_CAMERA){
        printf(" src module is camera\n");
    }

	while(hdl->enable){
        /* wait data in queue and pop it */
        module_waitReceiveSem(&hdl->generalSinkPort);

        /* pop packet from queue */
        InPacket = (struct modulePacket *)module_pop(&hdl->generalSinkPort);
        if(!InPacket || !InPacket->buf)
            continue;

        /* packet may be two types */
        if (InPacket->packet_type & AUDIO_PCM){
            audioInputbuf = (struct AudioPacket *)InPacket->buf;

            fwrite(audioInputbuf->pData, audioInputbuf->nLen, 1, audioFp);
        }else if(InPacket->packet_type & VIDEO_YUV) {
            index++;

            videoInputbuf = (struct MediaPacket *)InPacket->buf;

            memset(file_path, 0, sizeof(file_path));
            sprintf(file_path, "/tmp/TRvideo_test%d.bmp", index);

            YUVToBMP(file_path, videoInputbuf->Vir_Y_addr, NV21ToRGB24,
                                            videoInputbuf->width, videoInputbuf->height);
        }else{
            printf(RED" generalSink pop packet type is not right,InPacket %p type =0x%x\n"NONE,
                                            InPacket, InPacket->packet_type);
        }

        /* destroy packet when not use packet again */
        packetDestroy(InPacket);
        InPacket = NULL;
	}

    if(hdl->generalSinkPort.src_name & T_AUDIO)
        fclose(audioFp);

    printf(" general sink module thread exit\n");

	return NULL;
}

int generalSinkTest(void *recorderTestContext, void *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	TmoduleName camera,audio;
    RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
    struct generalSink *generalSinkHdl = NULL;

	i = number;
	/* Create TRecorder handle */
	memset(&Context[i], 0, sizeof(RecoderTestContext));
	Context[i].mTrecorder = CreateTRecorder();
	if (Context[i].mTrecorder == NULL) {
		printf(RED"CreateTRecorder[%d] err\n"NONE, i);
		return -1;
	}
	/* Reset Trecorder*/
	Context[i].mRecorderId = i;
	TRreset(Context[i].mTrecorder);

#if SINK_VIDEO_INPUT
    TRsetCamera(Context[i].mTrecorder, (TcameraIndex)i);
    Status[i].VideoMarkEnable = 0;
#endif

#if SINK_AUDIO_INPUT
    TRsetAudioSrc(Context[i].mTrecorder, (TmicIndex)i);
    Status[i].AudioMuteEnable = 1;
#endif

    generalSinkHdl = &Context[i].generalSinkHdl;

    generalSinkInit(generalSinkHdl);

	/* Custom module connection */
    camera = T_CAMERA;
    audio = T_AUDIO;
#if SINK_VIDEO_INPUT
    ret = TRmoduleLink(Context[i].mTrecorder, &camera,
		        (TmoduleName *)&(generalSinkHdl->generalSinkPort.name), NULL);
#endif

#if SINK_AUDIO_INPUT
    ret = TRmoduleLink(Context[i].mTrecorder, &audio,
		        (TmoduleName *)&(generalSinkHdl->generalSinkPort.name), NULL);
#endif

    if(ret < 0){
        printf(RED"TRmoduleLink error\n"NONE);
        return -1;
    }

	ret = TRprepare(Context[i].mTrecorder);
    if(ret < 0){
        printf(RED"trecorder %d prepare error\n"NONE, i);
        return -1;
    }

    generalSinkHdl->enable = 1;
    modulePort_SetEnable(&generalSinkHdl->generalSinkPort, 1);
	pthread_create(&generalSinkHdl->generalSinkId, NULL,
                            generalSinkThread, generalSinkHdl);

    /* print module connection information */
    {
		int index;
	    struct outputSrc *outputinfo;
	    printf("trecorder module link info:\n");
	    index = 0;
	    outputinfo = generalSinkHdl->generalSinkPort.output;

        printf("generalSinkPort enable %u name 0x%x, supported output type 0x%x, self queue %p, ouput info:\n",
                    generalSinkHdl->generalSinkPort.moduleEnable, generalSinkHdl->generalSinkPort.name,
                    generalSinkHdl->generalSinkPort.outputTyte, generalSinkHdl->generalSinkPort.sinkQueue);
        while(outputinfo){
            printf("  %d. enable %u supported input type 0x%x, queue addr %p\n",
                index, *(outputinfo->moduleEnable), *(outputinfo->sinkInputType), outputinfo->srcQueue);
            index++;
            outputinfo = outputinfo->next;
        }
	}

	ret = TRstart(Context[i].mTrecorder,T_PREVIEW);
    if(ret < 0){
        printf(RED"trecorder %d start error\n"NONE, i);
        return -1;
    }
    Status[i].PreviewEnable= 1;
    Status[i].RecordingEnable = 0;

    return 0;
}


void generalSinkQuit(void *recorderTestContext, void *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	TmoduleName camera,audio;
    RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
    struct generalSink *generalSinkHdl = NULL;

	i = number;

    generalSinkHdl = &Context[i].generalSinkHdl;

    generalSinkHdl->enable = 0;
    modulePort_SetEnable(&generalSinkHdl->generalSinkPort, 0);

    module_postReceiveSem(&generalSinkHdl->generalSinkPort);
    pthread_join(generalSinkHdl->generalSinkId, NULL);

	/* Custom module connection */
    camera = T_CAMERA;
    audio = T_AUDIO;
#if SINK_VIDEO_INPUT
    TRmoduleUnlink(Context[i].mTrecorder, &camera,
		        (TmoduleName *)&(generalSinkHdl->generalSinkPort.name), NULL);
#endif

#if SINK_AUDIO_INPUT
    TRmoduleUnlink(Context[i].mTrecorder, &audio,
		        (TmoduleName *)&(generalSinkHdl->generalSinkPort.name), NULL);
#endif

}
