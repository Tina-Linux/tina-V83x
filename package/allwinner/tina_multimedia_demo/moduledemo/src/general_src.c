#include "general_src.h"
#include "trecordertest.h"

#define YUV_PATH "/tmp/source_NV21_1.yuv"
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))

static long long GetNowMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int generalSrcAlloc(struct generalSrc *handle, int length)
{
	if(!handle || length <= 0){
		printf(RED" generalSrcAlloc error size %d\n"NONE, length);
		return -1;
	}
        int i = 0;
	for(i = 0; i < GENERRAL_SRC_BUF; i++)
		alloc_VirPhyBuf(&(handle->dataArray[i].pVirBuf), &(handle->dataArray[i].pPhyBuf), length);

    return 0;
}

static int generalSrcFree(struct generalSrc *handle)
{
	if(!handle)
		return -1;
        int i = 0;
	for(i = 0; i < GENERRAL_SRC_BUF; i++)
		free_VirPhyBuf(handle->dataArray[i].pVirBuf);
}

static int generalSrcDequeue(struct generalSrc *handle, struct Srcaddr *databuf)
{
	int ret = -1;
	int i = 0;

	if(!handle || !databuf)
		return ret;

	for(i = 0; i < GENERRAL_SRC_BUF; i++){
		if(handle->dataArray[i].userFlag == 0)
			break;
	}

	if(i < GENERRAL_SRC_BUF){
		databuf->pVirBuf = handle->dataArray[i].pVirBuf;
		databuf->pPhyBuf = handle->dataArray[i].pPhyBuf;
		handle->dataArray[i].userFlag = 1;
		ret = 0;
	}

	return ret;
}

static int generalSrcEnqueue(struct generalSrc *handle, unsigned char *pVirBuf)
{
	int ret = -1;
	int i = 0;

	if(!handle || !pVirBuf)
		return ret;

	for(i = 0; i < GENERRAL_SRC_BUF; i++){
		if(handle->dataArray[i].pVirBuf == pVirBuf)
			break;
	}

	if(i < GENERRAL_SRC_BUF){
		handle->dataArray[i].userFlag = 0;
		ret = 0;
	}

	return ret;
}

static int readYUVData(struct generalSrc *handle, char *path)
{
    FILE *fp = NULL;
    int ret = 0;

    if(!handle || !path)
        return -1;

    fp = fopen(path, "rb");
    if(!fp){
        printf(RED"read YUV file error:%s\n"NONE, path);
        return -1;
    }

    fseek(fp,0,SEEK_END);
    handle->dataLength = ftell(fp);
    rewind(fp);

    if((handle->dataLength == 640*480*3/2)
                || (handle->dataLength == 640*480*3/2 + 4096)
                || (handle->dataLength == ALIGN_4K(640*480*3/2))
                || (handle->dataLength == ALIGN_4K(ALIGN_16B(640)*ALIGN_16B(480)*3/2))
                || (handle->dataLength == ALIGN_16B(640)*ALIGN_16B(480)*3/2)){
        handle->photoWidth = 640;
        handle->photoHeight = 480;
    }else if((handle->dataLength == 1280*720*3/2)
                || (handle->dataLength == 1280*720*3/2 + 4096)
                || (handle->dataLength == ALIGN_4K(1280*720*3/2))
                || (handle->dataLength == ALIGN_4K(ALIGN_16B(1280)*ALIGN_16B(720)*3/2))
                || (handle->dataLength == ALIGN_16B(1280)*ALIGN_16B(720)*3/2)){
        handle->photoWidth = 1280;
        handle->photoHeight = 720;
    }else if((handle->dataLength == 1920*1080*3/2)
                || (handle->dataLength == 1920*1080*3/2 + 4096)
                || (handle->dataLength == ALIGN_4K(1920*1080*3/2))
                || (handle->dataLength == ALIGN_4K(ALIGN_16B(1920)*ALIGN_16B(1080)*3/2))
                || (handle->dataLength == ALIGN_16B(1920)*ALIGN_16B(1080)*3/2)){
        handle->photoWidth = 1920;
        handle->photoHeight = 1080;
    }else{
        printf(RED" unknown size:%d\n"NONE, handle->dataLength);
        fclose(fp);
        return -1;
    }

    handle->pYUV = (unsigned char *)malloc(sizeof(unsigned char)*(handle->dataLength));

    if(!handle->pYUV){
        printf(RED"malloc memory fail\n"NONE);
        fclose(fp);
        return -1;
    }

    ret = fread (handle->pYUV, 1, handle->dataLength, fp);
    if (ret != handle->dataLength){
        printf(RED"read %s fail\n"NONE, path);
        fclose(fp);
        free(handle->pYUV);
        handle->pYUV = NULL;
        return -1;
    }

    if(fp)
        fclose(fp);

    return 0;
}

static int generalSrcInit(struct generalSrc* handle)
{
    int ret;
    int i = 0;

    if(!handle)
        return -1;

    memset(&handle->generalSrcPort, 0x00, sizeof(struct moduleAttach));

    ModuleSetNotifyCallback(&handle->generalSrcPort,
                            NotifyCallbackToSink, &handle->generalSrcPort);

    handle->generalSrcPort.name = T_CUSTOM;

    handle->generalSrcPort.outputTyte = VIDEO_YUV;

    /* module init */
    /* SRC module output read the image data */
    ret = readYUVData(handle, YUV_PATH);
    if(ret < 0){
        printf(RED" read yuv data error\n"NONE);
        return -1;
    }

    /* malloc mem */
    ret = generalSrcAlloc(handle, handle->dataLength);
    if(ret < 0){
        printf(RED" general src alloc error\n"NONE);
        free(handle->pYUV);
        return -1;
    }
    for(i = 0; i < GENERRAL_SRC_BUF; i++)
		generalSrcEnqueue(handle, handle->dataArray[i].pVirBuf);

    return 0;
}

/*
 * make sure that there is a photo in the corresponding path
 * YUV format:NV21
 */
static void *generalSrcThread(void *param)
{
	struct generalSrc *hdl = (struct generalSrc*)param;
    struct modulePacket *OutPacket = NULL;
    struct MediaPacket *outputbuf = NULL;
    struct Srcaddr databuf;
    int bufIndex = 0;
    int ret = 0;
    int i = 0;

    bufIndex = 0;
	while(hdl->enable){
        ret = generalSrcDequeue(hdl, &databuf);
		if(ret < 0){
			printf(RED" generalSrc no free buf\n"NONE);
			continue;
		}

        memcpy(databuf.pVirBuf, hdl->pYUV, hdl->dataLength);

        /* set packet */
		OutPacket = packetCreate(sizeof(struct MediaPacket));

        outputbuf = (struct MediaPacket *)OutPacket->buf;

		outputbuf->buf_index = bufIndex;
		outputbuf->Vir_Y_addr = databuf.pVirBuf;
		outputbuf->Vir_C_addr = databuf.pVirBuf + hdl->photoWidth*hdl->photoHeight;
		outputbuf->Phy_Y_addr = databuf.pPhyBuf;
		outputbuf->Phy_C_addr = databuf.pPhyBuf + hdl->photoWidth*hdl->photoHeight;
		outputbuf->width = hdl->photoWidth;
		outputbuf->height = hdl->photoHeight;
		outputbuf->nPts = GetNowMs();
		outputbuf->data_len = hdl->dataLength;
		outputbuf->bytes_used = hdl->dataLength;

		OutPacket->OnlyMemFlag = 0;
		/* set packet notify callback */
		OutPacket->mode.notify.notifySrcHdl = &hdl->generalSrcPort;
		OutPacket->mode.notify.notifySrcFunc = module_postReturnSem;
		OutPacket->packet_type = VIDEO_YUV;

		/* push packet into queue */
        ret = module_push(&hdl->generalSrcPort, OutPacket);
		for(i = 0; i < ret; i++)
		module_waitReturnSem(&hdl->generalSrcPort);

		generalSrcEnqueue(hdl, databuf.pVirBuf);

		free(OutPacket->buf);
		free(OutPacket);
		OutPacket = NULL;

        bufIndex++;
	}

	/* free buf */
	generalSrcFree(hdl);

    free(hdl->pYUV);

	return NULL;
}

int generalSrcTest(void *recorderTestContext, void *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	TmoduleName display;
    RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
    struct generalSrc *generalSrcHdl = NULL;

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

	TRsetPreview(Context[i].mTrecorder,T_DISP_LAYER0);

    generalSrcHdl = &Context[i].generalSrcHdl;
    ret = generalSrcInit(generalSrcHdl);
    if(ret < 0){
        printf(RED"general src module init error\n"NONE);
        return -1;
    }

	/* Custom module connection */
	display = T_SCREEN;
    ret = TRmoduleLink(Context[i].mTrecorder,
		        (TmoduleName *)&(generalSrcHdl->generalSrcPort.name), &display, NULL);
    if(ret < 0){
        printf(RED"TRmoduleLink error\n"NONE);
        return -1;
    }

	ret = TRprepare(Context[i].mTrecorder);
    if(ret < 0){
        printf(RED"trecorder %d prepare error\n"NONE, i);
        return -1;
    }

    generalSrcHdl->enable = 1;
    modulePort_SetEnable(&generalSrcHdl->generalSrcPort, 1);
	pthread_create(&generalSrcHdl->generalSrcId, NULL,
                            generalSrcThread, generalSrcHdl);

    /* print module connection information */
    {
		int index;
	    struct outputSrc *outputinfo;
	    printf("trecorder module link info:\n");
	    index = 0;
	    outputinfo = generalSrcHdl->generalSrcPort.output;

        printf("generalSrcPort enable %u name 0x%x, supported output type 0x%x, self queue %p, ouput info:\n",
                    generalSrcHdl->generalSrcPort.moduleEnable, generalSrcHdl->generalSrcPort.name,
                    generalSrcHdl->generalSrcPort.outputTyte, generalSrcHdl->generalSrcPort.sinkQueue);
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

void generalSrcQuit(void *recorderTestContext, void *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	TmoduleName display;
    RecoderTestContext *Context = (RecoderTestContext *)recorderTestContext;
    RecorderStatusContext *Status = (RecorderStatusContext *)RecorderStatus;
    struct generalSrc *generalSrcHdl = NULL;

	i = number;

    generalSrcHdl = &Context[i].generalSrcHdl;

    generalSrcHdl->enable = 0;
    pthread_join(generalSrcHdl->generalSrcId, NULL);

	/* Custom module connection */
	display = T_SCREEN;
    TRmoduleUnlink(Context[i].mTrecorder,
		        (TmoduleName *)&(generalSrcHdl->generalSrcPort.name), &display, NULL);
}
