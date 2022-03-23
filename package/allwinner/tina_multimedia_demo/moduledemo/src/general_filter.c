#include "general_filter.h"
#include "trecordertest.h"

static int generalFilterAlloc(struct generalFilter *handle, int length)
{
	if(!handle || length <= 0){
		printf(RED" %s error size %d\n"NONE, __func__, length);
		return -1;
	}
        int i = 0;
	for(i = 0; i < GENERAL_FILTER_BUF_NUM; i++)
		alloc_VirPhyBuf(&(handle->dataArray[i].pVirBuf), &(handle->dataArray[i].pPhyBuf), length);
}

static int generalFilterFree(struct generalFilter *handle)
{
	if(!handle)
		return -1;
        int i = 0;
	for(i = 0; i < GENERAL_FILTER_BUF_NUM; i++)
		free_VirPhyBuf(handle->dataArray[i].pVirBuf);
}

static int generalFilterDequeue(struct generalFilter *handle, struct Filteraddr *databuf)
{
	int ret = -1;
	int i = 0;

	if(!handle || !databuf)
		return ret;
	for(i = 0; i < GENERAL_FILTER_BUF_NUM; i++){
		if(handle->dataArray[i].userFlag == 0)
			break;
	}

	if(i < GENERAL_FILTER_BUF_NUM){
		databuf->pVirBuf = handle->dataArray[i].pVirBuf;
		databuf->pPhyBuf = handle->dataArray[i].pPhyBuf;
		handle->dataArray[i].userFlag = 1;
		ret = 0;
	}

	return ret;
}

static int generalFilterEnqueue(struct generalFilter *handle, unsigned char *pVirBuf)
{
	int ret = -1;
	int i = 0;

	if(!handle || !pVirBuf)
		return ret;

	for(i = 0; i < GENERAL_FILTER_BUF_NUM; i++){
		if(handle->dataArray[i].pVirBuf == pVirBuf)
			break;
	}

	if(i < GENERAL_FILTER_BUF_NUM){
		handle->dataArray[i].userFlag = 0;
		ret = 0;
	}

	return ret;
}

static int generalFilterInit(struct generalFilter* handle)
{
    int ret;

    if(!handle)
        return -1;

    memset(&handle->generalFilterPort, 0x00, sizeof(struct moduleAttach));

    ModuleSetNotifyCallback(&handle->generalFilterPort,
                            NotifyCallbackToSink, &handle->generalFilterPort);

    handle->generalFilterPort.name = T_CUSTOM;

    handle->generalFilterPort.inputTyte = VIDEO_YUV;
    handle->generalFilterPort.outputTyte = VIDEO_YUV;

    return 0;
}

static void *generalFilterThread(void *param)
{
	struct generalFilter *hdl = (struct generalFilter*)param;
	struct modulePacket *InPacket = NULL;
    struct modulePacket *OutPacket = NULL;
    struct MediaPacket *inputbuf = NULL;
    struct MediaPacket *outputbuf = NULL;
	int buf_index;
	int data_len;
    int bytes_used;
    int64_t nPts;
	unsigned int width,height;
	struct Filteraddr databuf;
	int ret;

	int index = 0;
	char path[64];
        int i = 0;

WaitData:
	/* get the buf size and allocate memory */
	while(module_InputQueueEmpty(&hdl->generalFilterPort))
		usleep(1*1000);

	InPacket = (struct modulePacket *)module_pop(&hdl->generalFilterPort);
	if(!InPacket)
		goto WaitData;

	inputbuf = (struct MediaPacket *)InPacket->buf;

	bytes_used = inputbuf->bytes_used;

	packetDestroy(InPacket);

	generalFilterAlloc(hdl, bytes_used);
	for(i = 0; i < GENERAL_FILTER_BUF_NUM; i++)
		generalFilterEnqueue(hdl, hdl->dataArray[i].pVirBuf);

	while(hdl->enable) {
		/* wait data in queue and pop it */
        module_waitReceiveSem(&hdl->generalFilterPort);

		InPacket = (struct modulePacket *)module_pop(&hdl->generalFilterPort);
		if(!InPacket)
			continue;

		inputbuf = (struct MediaPacket *)InPacket->buf;

		if(bytes_used != inputbuf->bytes_used){
			printf(RED"the size of the buffer has changed,inputbuf->bytes_used %d bytes_used %d\n"NONE,
																	inputbuf->bytes_used, bytes_used);
			packetDestroy(InPacket);
			continue;
		}
		buf_index = inputbuf->buf_index;
		data_len = inputbuf->data_len;
		bytes_used = inputbuf->bytes_used;
		nPts = inputbuf->nPts;
		width = inputbuf->width;
		height = inputbuf->height;

		ret = generalFilterDequeue(hdl, &databuf);
		if(ret < 0){
			printf(RED" generalFilter no free buf\n"NONE);
			packetDestroy(InPacket);
			continue;
		}

        /* handle data and output packet */
		memcpy(databuf.pVirBuf, (unsigned char*)inputbuf->Vir_Y_addr, bytes_used);
		memFlushCache(databuf.pVirBuf, bytes_used);
		packetDestroy(InPacket);

#if 0
		index++;
		if((index < 100) && (index%10 == 0)){
			memset(path, 0, sizeof(path));
			sprintf(path, "/tmp/bmp_generalFilter%d.bmp", index);
			YUVToBMP(path, (char *)databuf.pVirBuf, NV21ToRGB24, width, height);
		}
#endif

		InPacket = NULL;

        /* set packet */
		OutPacket = packetCreate(sizeof(struct MediaPacket));

        outputbuf = (struct MediaPacket *)OutPacket->buf;
		outputbuf->buf_index = buf_index;
		outputbuf->Vir_Y_addr = databuf.pVirBuf;
		outputbuf->Vir_C_addr = databuf.pVirBuf + width*height;
		outputbuf->Phy_Y_addr = databuf.pPhyBuf;
		outputbuf->Phy_C_addr = databuf.pPhyBuf + width*height;
		outputbuf->width = width;
		outputbuf->height = height;
		outputbuf->nPts = nPts;
		outputbuf->data_len = data_len;
		outputbuf->bytes_used = bytes_used;

		OutPacket->OnlyMemFlag = 0;
		/* set packet notify callback */
		OutPacket->mode.notify.notifySrcHdl = &hdl->generalFilterPort;
		OutPacket->mode.notify.notifySrcFunc = module_postReturnSem;
		OutPacket->packet_type = VIDEO_YUV;

		/* push packet into queue */
        ret = module_push(&hdl->generalFilterPort, OutPacket);
		for(i = 0; i < ret; i++)
		module_waitReturnSem(&hdl->generalFilterPort);

		generalFilterEnqueue(hdl, databuf.pVirBuf);

		free(OutPacket->buf);
		free(OutPacket);
		OutPacket = NULL;
	}

	modulePort_SetEnable(&hdl->generalFilterPort, 0);
	while(!module_InputQueueEmpty(&hdl->generalFilterPort)) {
		InPacket = (struct modulePacket *)module_pop(&hdl->generalFilterPort);
		if(!InPacket)
			continue;

		packetDestroy(InPacket);
	}

	/* free buf */
	generalFilterFree(hdl);

	return NULL;
}

int generalFilterTest(void *recorderTestContext, void *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
	struct generalFilter *generalFilterHdl = NULL;
    char num[][6] = {"front", "rear"};
    char outputchar[64];
	TmoduleName camera,audio,display,encoder,muexr,decoder;

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

	TRsetCamera(Context[i].mTrecorder, (TcameraIndex)i);
    Status[i].VideoMarkEnable = 0;
	TRsetAudioSrc(Context[i].mTrecorder, (TmicIndex)i);
    Status[i].AudioMuteEnable = 1;
	TRsetPreview(Context[i].mTrecorder,T_DISP_LAYER0);

    memset(outputchar, 0, sizeof(outputchar));
    sprintf(outputchar, "%s%s%s", "/mnt/SDCARD/AW_", num[i], "_video0.mp4");
    TRsetOutput(Context[i].mTrecorder,outputchar);
	TRsetMaxRecordTimeMs(Context[i].mTrecorder, 60*1000);
    TRsetRecorderCallback(Context[i].mTrecorder,CallbackFromTRecorder,(void*)&Context[i]);

	generalFilterHdl = &Context[i].generalFilterHdl;

    generalFilterInit(generalFilterHdl);

	/* Custom module connection */
    audio = T_AUDIO;
	camera = T_CAMERA;
	display = T_SCREEN;
	encoder = T_ENCODER;
	muexr = T_MUXER;
    decoder = T_DECODER;

    ret = TRmoduleLink(Context[i].mTrecorder, &audio, &encoder, &muexr, NULL);
    if(ret < 0){
        printf(RED"TRmoduleLink error\n"NONE);
        return -1;
    }
    ret = TRmoduleLink(Context[i].mTrecorder, &camera, &decoder,
		        (TmoduleName *)&(generalFilterHdl->generalFilterPort.name), &display, NULL);
    if(ret < 0){
        printf(RED"TRmoduleLink error\n"NONE);
        return -1;
    }
    ret = TRmoduleLink(Context[i].mTrecorder, &camera, &decoder,
		        (TmoduleName *)&(generalFilterHdl->generalFilterPort.name), &encoder, &muexr, NULL);
    if(ret < 0){
        printf(RED"TRmoduleLink error\n"NONE);
        return -1;
    }

	ret = TRprepare(Context[i].mTrecorder);
    if(ret < 0){
        printf(RED"trecorder %d prepare error\n"NONE, i);
        return -1;
    }

    generalFilterHdl->enable = 1;
    modulePort_SetEnable(&generalFilterHdl->generalFilterPort, 1);
	pthread_create(&generalFilterHdl->generalFilterId, NULL,
                            generalFilterThread, generalFilterHdl);

	/* print module connection information */
	{
		int index;
	    struct outputSrc *outputinfo;
	    printf("trecorder module link info:\n");
	    index = 0;
	    outputinfo = generalFilterHdl->generalFilterPort.output;

        printf("generalFilter enable %u name 0x%x, supported output type 0x%x, self queue %p, ouput info:\n",
                    generalFilterHdl->generalFilterPort.moduleEnable, generalFilterHdl->generalFilterPort.name,
                    generalFilterHdl->generalFilterPort.outputTyte, generalFilterHdl->generalFilterPort.sinkQueue);
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

void generalFilterQuit(void *recorderTestContext, void *RecorderStatus, int number)
{
	RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
	struct generalFilter *generalFilterHdl = NULL;
	TmoduleName camera,audio,display,encoder,muexr,decoder;

	generalFilterHdl = &Context[number].generalFilterHdl;

    generalFilterHdl->enable = 0;
    modulePort_SetEnable(&generalFilterHdl->generalFilterPort, 0);

    module_postReceiveSem(&generalFilterHdl->generalFilterPort);
    pthread_join(generalFilterHdl->generalFilterId, NULL);

	/* Custom module connection */
    audio = T_AUDIO;
	camera = T_CAMERA;
	display = T_SCREEN;
	encoder = T_ENCODER;
	muexr = T_MUXER;
    decoder = T_DECODER;

    TRmoduleUnlink(Context[number].mTrecorder, &audio, &encoder, &muexr, NULL);
    TRmoduleUnlink(Context[number].mTrecorder, &camera, &decoder,
		        (TmoduleName *)&(generalFilterHdl->generalFilterPort.name), &display, NULL);
    TRmoduleUnlink(Context[number].mTrecorder, &camera, &decoder,
		        (TmoduleName *)&(generalFilterHdl->generalFilterPort.name), &encoder, &muexr, NULL);
}
