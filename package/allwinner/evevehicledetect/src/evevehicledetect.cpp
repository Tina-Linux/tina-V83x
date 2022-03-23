#ifdef __cplusplus
extern "C"{
#endif
#include "aw_ai_eve_event_interface.h"

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <CameraInterface.h>
#include "AWIspApi.h"
AWIspApi *ispPort;
AWIspApi *ispPort1;

#include "videoOutPort.h"
#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

#define WIDTH 1280
#define HEIGHT 1080

#define VIDEOWIDTH 1280
#define VIDEOHEIGHT 1080

#define SFWIDTH 1280
#define SFHEIGHT 1080

#define TEST_COST 1
#define SAVE_RESULT 1

#ifdef TEST_COST

const int framesize = VIDEOWIDTH * VIDEOHEIGHT * 3 / 2;


dispOutPort *DisPort;
dispOutPort *DisPort1;

dispOutPort * dispInit(void)
{
	dispOutPort *disPort;

	VoutRect mPreviewRect;
	videoParam param;

	mPreviewRect.x = 0;
	mPreviewRect.y = 0;
	mPreviewRect.width = 1280;
	mPreviewRect.height = 360;

	disPort = CreateVideoOutport(0);

	disPort->init(disPort, 1, ROTATION_ANGLE_0, &mPreviewRect);

	/* set Route */
	disPort->setRoute(disPort, VIDEO_SRC_FROM_CAM);

	/* disable display */
	disPort->setEnable(disPort,1);

	/* set rotate angel */
	disPort->setRotateAngel(disPort, ROTATION_ANGLE_0);

	/* set Rect */
	disPort->setRect(disPort,&mPreviewRect);

	param.srcInfo.w = WIDTH;
	param.srcInfo.h = HEIGHT;
	disPort->allocateVideoMem(disPort, &param);

	disPort->SetZorder(disPort, VIDEO_ZORDER_TOP);

	return disPort;
}

dispOutPort * dispInit1(void)
{
	dispOutPort *disPort;

	VoutRect mPreviewRect;
	videoParam param;

	mPreviewRect.x = 0;
	mPreviewRect.y = 360;
	mPreviewRect.width = 1280;
	mPreviewRect.height = 360;

	disPort = CreateVideoOutport(0);

	disPort->init(disPort, 1, ROTATION_ANGLE_0, &mPreviewRect);

	/* set Route */
	disPort->setRoute(disPort, VIDEO_SRC_FROM_CAM);

	/* disable display */
	disPort->setEnable(disPort,1);

	/* set rotate angel */
	disPort->setRotateAngel(disPort, ROTATION_ANGLE_0);

	/* set Rect */
	disPort->setRect(disPort,&mPreviewRect);

	param.srcInfo.w = WIDTH;
	param.srcInfo.h = HEIGHT;
	disPort->allocateVideoMem(disPort, &param);

	disPort->SetZorder(disPort, VIDEO_ZORDER_TOP);

	return disPort;
}

int display(dispOutPort *disPort, unsigned char *start, int size)
{
	videoParam paramDisp;
	int ret;
	/* set rotate angel */
	disPort->setRotateAngel(disPort, ROTATION_ANGLE_0);

	paramDisp.isPhy = 0;
	paramDisp.srcInfo.w = WIDTH;
	paramDisp.srcInfo.h = HEIGHT;
	paramDisp.srcInfo.crop_x = 0;
	paramDisp.srcInfo.crop_y = 0;
	paramDisp.srcInfo.crop_w = WIDTH;
	paramDisp.srcInfo.crop_h = HEIGHT;
	paramDisp.srcInfo.format = VIDEO_PIXEL_FORMAT_NV12;
	ret = disPort->writeData(disPort, start, size, &paramDisp);

	return ret;
}

int dispClose(dispOutPort *disPort)
{
	disPort->enable = 0;
	disPort->freeVideoMem(disPort);
	disPort->deinit(disPort);
	DestroyVideoOutport(disPort);

	return 0;
}

const static Scalar colors[] =
{
	Scalar(255,0,0),
	Scalar(255,128,0),
	Scalar(255,255,0),
	Scalar(0,255,0),
	Scalar(0,128,255),
	Scalar(0,255,255),
	Scalar(0,0,255),
	Scalar(255,0,255)
};

static int YuvToMat(Mat &img, unsigned char *pYUV, int width, int height)
{
	if(!pYUV || (width <= 0) || (height <= 0)){
		fprintf(stderr, "YuvToMat input error\n");
		return -1;
	}

	img.create(height*3/2, width, CV_8UC1);
	img.data = pYUV;
}

int save_frame_to_file(void *str,void *start,int length)
{
	FILE *fp = NULL;

	fp = fopen((const char*)str, "wb+");
	if(!fp) {
		printf(" Open %s error\n", (char *)str);
		return -1;
	}

	if (fwrite(start, length, 1, fp)) {
		fclose(fp);
		return 0;
	} else {
		printf(" Write file fail (%s)\n",strerror(errno));
		fclose(fp);
		return -1;
	}

	return 0;
}
/**
 * ��������ʱ���ļ������õ�ʱ����
 * @param struct timeval* resule ���ؼ���������ʱ��
 * @param struct timeval* x ��Ҫ������ǰһ��ʱ��
 * @param struct timeval* y ��Ҫ�����ĺ�һ��ʱ��
 * return -1 failure ,0 success
 **/
static int timeval_subtract(struct timeval *result, struct timeval *x,
														struct timeval *y) {
	int nsec;

	if (x->tv_sec > y->tv_sec)
		return -1;

	if ((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))
		return -1;

	result->tv_sec = (y->tv_sec - x->tv_sec);
	result->tv_usec = (y->tv_usec - x->tv_usec);

	if (result->tv_usec < 0) {
		result->tv_sec--;
		result->tv_usec += 1000000;
	}

	return 0;
}
#endif

static AW_S32 write_blob(AW_BLOB_S *blob, FILE *fs) {
	fwrite(&blob->s16X, sizeof(AW_S16), 1, fs);
	fwrite(&blob->s16Y, sizeof(AW_S16), 1, fs);
	fwrite(&blob->s16Width, sizeof(AW_S16), 1, fs);
	fwrite(&blob->s16Height, sizeof(AW_S16), 1, fs);
	return 0;
}

AW_S32 eve_kernel_dump_result_to_file(AW_BLOB_S *resPtr, AW_U32 num, FILE *fs) {
	AW_U32 i = 0;
	fwrite(&num, sizeof(AW_U32), 1, fs);
	for (i = 0; i < num; i++, resPtr++) {
		write_blob(resPtr, fs);
	}
	return 0;
}

void *dma_pUsr = NULL;

static void dmaCallBackFunc(void *pUsr) {
	printf("[Dma Callback]: Run!\n");
	return;
}

static const char *awKeyNew = "1111111111111111";


void eve_init_ctrl(AW_AI_EVE_CTRL_S *sEVECtrl)
{
	// initilize
	sEVECtrl->addrInputType = AW_AI_EVE_ADDR_INPUT_VIR;
	sEVECtrl->scale_factor = 1;
	sEVECtrl->mScanStageNo = 10;
	sEVECtrl->yStep = 3;
	sEVECtrl->xStep0 = 1;
	sEVECtrl->xStep1 = 4;
	sEVECtrl->mRltNum = AW_AI_EVE_MAX_RESULT_NUM;
	sEVECtrl->mMidRltNum = 0;
	sEVECtrl->mMidRltStageNo = 50;
	sEVECtrl->mRltNum = AW_AI_EVE_MAX_RESULT_NUM;
	sEVECtrl->rltType = AW_AI_EVE_RLT_OUTPUT_DETAIL;

	sEVECtrl->mDmaOut.s16Width = 640;
	sEVECtrl->mDmaOut.s16Height = 540;
	sEVECtrl->mPyramidLowestLayel.s16Width = 640;
	sEVECtrl->mPyramidLowestLayel.s16Height = 540;
	sEVECtrl->dmaSrcSize.s16Width = 1280;
	sEVECtrl->dmaSrcSize.s16Height = 1080;
	sEVECtrl->dmaDesSize.s16Width = 640;
	sEVECtrl->dmaDesSize.s16Height = 540;
	sEVECtrl->dmaRoi.s16X = 0;
	sEVECtrl->dmaRoi.s16Y = 0;
	sEVECtrl->dmaRoi.s16Width = 1280;
	sEVECtrl->dmaRoi.s16Height = 1080;

	sEVECtrl->classifierNum = 1;
	sEVECtrl->classifierPath[0].path = (AW_S8 *)"/etc/eve/frontface.ld";
	sEVECtrl->classifierPath[0].key = (AW_U8 *)awKeyNew;
	sEVECtrl->classifierPath[0].type = AW_CLASSIFIY_TYPE_HAAR;
	/*
	sEVECtrl->classifierPath[1].path = (AW_S8*)"/etc/eve/car.ld";
	sEVECtrl->classifierPath[1].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[1].type = AW_CLASSIFIY_TYPE_LBP;
	sEVECtrl->classifierPath[2].path = (AW_S8*)"/etc/eve/fullprofrightface.ld";
	sEVECtrl->classifierPath[2].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[3].path = (AW_S8*)"/etc/eve/halfdownface.ld";
	sEVECtrl->classifierPath[3].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[4].path = (AW_S8*)"/etc/eve/smallface.ld";
	sEVECtrl->classifierPath[4].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[5].path = (AW_S8*)"/etc/eve/rotleftface.ld";
	sEVECtrl->classifierPath[5].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[6].path = (AW_S8*)"/etc/eve/rotrightface.ld";
	sEVECtrl->classifierPath[6].key  = (AW_U8*)awKeyNew;
	sEVECtrl->classifierPath[7].path =
	(AW_S8*)"./classifier/smallface.ld"; sEVECtrl->classifierPath[7].key  =
	(AW_U8*)awKeyNew;
	*/

	sEVECtrl->dmaCallBackFunc = &dmaCallBackFunc;
	sEVECtrl->dma_pUsr = dma_pUsr;
}

void eve_init_facedet_param(AW_AI_EVE_EVENT_FACEDET_PARAM_S *facedetparam)
{
	facedetparam->sRoiSet.s32RoiNum = 1;
	facedetparam->sRoiSet.sID[0] = 1;
	facedetparam->sRoiSet.sRoi[0].s16Left = 0;
	facedetparam->sRoiSet.sRoi[0].s16Top = 0;
	facedetparam->sRoiSet.sRoi[0].s16Right = VIDEOWIDTH;
	facedetparam->sRoiSet.sRoi[0].s16Bottom = VIDEOHEIGHT;
	facedetparam->s32ClassifyFlag = 0;
	facedetparam->s32MinFaceSize = 20;
	facedetparam->s32OverLapCoeff = 20;
	facedetparam->s32MaxFaceNum = 128;
	facedetparam->s32MergeThreshold = 3;
}

int main(int argc, char *argv[]) {
	#ifdef TEST_COST
		struct timeval starttime, endtime, difftime;
	#endif
	int frameNum = 0;

	AW_S32 status;
	AW_S8 cVersion[128];
	AW_AI_EVE_CTRL_S sEVECtrl;
	AW_AI_EVE_EVENT_FACEDET_PARAM_S facedetparam;
	AW_IMAGE_S image;
	AW_AI_EVE_EVENT_RESULT_S faceres;
	AW_BLOB_SET_S blobsetres;
	AW_S32 i;

	Camera *camera;
	Camera *camera1;
	CameraBuffer buf;
	CameraBuffer buf1;
	char sourcePath[32];
	int ispid;
	int ispid1;

	Mat yuvImg;
	Mat rgbImg;

	printf("start app\n");
	/* check eve version */
	status = AW_AI_EVE_Event_GetAlgoVersion(cVersion);
	if (AW_STATUS_ERROR == status) {
		printf("AW_AI_EVE_Event_GetAlgoVersion failure!\n");
		return -1;
	} else {
		printf("AW_AI_EVE_Event_GetAlgoVersion version is %s!\n", cVersion);
	}
	/* AW_AI_EVE_Event_Init */
	eve_init_ctrl(&sEVECtrl);
	AW_HANDLE hEveEvent = AW_AI_EVE_Event_Init(&sEVECtrl);
	if (AW_NULL == hEveEvent) {
		printf("AW_AI_EVE_Event_Init failure!\n");
		return -1;
	} else {
		printf("AW_AI_EVE_Event_Init finish!\n");
	}
	/* AW_AI_EVE_Event_SetEveDMAExecuteMode */
	AW_AI_EVE_Event_SetEveDMAExecuteMode(hEveEvent, AW_AI_EVE_DMA_EXECUTE_SYNC);
	/* set face event parameter */
	eve_init_facedet_param(&facedetparam);
	status = AW_AI_EVE_Event_SetEventParam(hEveEvent, AW_AI_EVE_EVENT_FACEDETECT,
										 (AW_PVOID)&facedetparam);
	if (AW_STATUS_ERROR == status) {
		printf("AW_AI_EVE_Event_SetEventParam failure!, errorcode = %d\n",
					 AW_AI_EVE_Event_GetLastError(hEveEvent));
		return -1;
	}
	/* init aw eve image info  */
	image.mWidth = VIDEOWIDTH;
	image.mHeight = VIDEOHEIGHT;
	image.mPixelFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	image.mStride[0] = VIDEOWIDTH;
	image.mStride[1] = VIDEOWIDTH;
	image.mStride[2] = 0;

	/* create aw camera device and streamon */
	camera = CreateCameraDevice();
	camera->open(camera, 0);
	camera->init(camera);
	camera->streamon(camera);

	camera1 = CreateCameraDevice();
	camera1->open(camera1, 1);
	camera1->init(camera1);
	camera1->streamon(camera1);
	/* enable aw isp */
	ispPort = CreateAWIspApi();
	ispPort1 = CreateAWIspApi();
	ispid = ispPort->ispGetIspId(0);
	ispid1 = ispPort->ispGetIspId(1);
	ispPort->ispStart(ispid);
	ispPort1->ispStart(ispid1);
	/* init aw display */
	DisPort = dispInit();
	DisPort1 = dispInit1();
	/* create yuvImg(opencv mat) to process */
	yuvImg.create(VIDEOHEIGHT*3/2, VIDEOWIDTH, CV_8UC1);
	/* start capture and facedetct and display */
	while (1) {
		camera->dequeue(camera, &buf);
		camera1->dequeue(camera1, &buf1);
		/* check camera capture yuv data */
/*
		printf("-------------length = %d\n", buf.length[0]);
		sprintf(sourcePath,"/tmp/test%d.yuv", frameNum);
		save_frame_to_file(sourcePath, buf.viraddr[0], buf.length[0]);
*/
		image.mpVirAddr[0] = (AW_PVOID)buf.viraddr[0];
		image.mPhyAddr[0] = (AW_U32)buf.phyaddr[0];
		AW_AI_EVE_Event_SetEveSourceAddress(hEveEvent, (void *)image.mpVirAddr[0]);
#ifdef TEST_COST
		gettimeofday(&starttime, 0);
#endif
		status = AW_AI_EVE_Event_ProcessImage(hEveEvent, &image, &faceres);
		if (AW_STATUS_ERROR == status) {
			printf("AW_AI_EVE_Event_Process failure!, errorcode = %d\n",
					 AW_AI_EVE_Event_GetLastError(hEveEvent));
			break;
		}
#ifdef TEST_COST
		gettimeofday(&endtime, 0);
		timeval_subtract(&difftime, &starttime, &endtime);
		printf("-----AW_AI_EVE_Event_Process cost time = %f ms , frames:%d\n",
				difftime.tv_usec / 1000.f, frameNum);
#endif

		memcpy(yuvImg.data, image.mpVirAddr[0], framesize);
#ifdef SAVE_RESULT
		blobsetres.s32Num = faceres.sTarget.s32TargetNum;
		for (i = 0; i < blobsetres.s32Num; i++) {
			blobsetres.blob[i].s16X = faceres.sTarget.astTargets[i].stRect.s16Left;
			blobsetres.blob[i].s16Y = faceres.sTarget.astTargets[i].stRect.s16Top;
			blobsetres.blob[i].s16Width = \
								faceres.sTarget.astTargets[i].stRect.s16Right -
								faceres.sTarget.astTargets[i].stRect.s16Left + 1;
			blobsetres.blob[i].s16Height = \
								faceres.sTarget.astTargets[i].stRect.s16Bottom -
								faceres.sTarget.astTargets[i].stRect.s16Top + 1;
			/* rect */
			Scalar color = colors[i%8];
			rectangle(yuvImg, Rect(blobsetres.blob[i].s16X, blobsetres.blob[i].s16Y,
					blobsetres.blob[i].s16Width, blobsetres.blob[i].s16Height),
					color, 1, 4, 0);
		}
#endif
		/* use libuapi to show yuv image */
		display(DisPort, (unsigned char *)yuvImg.data, framesize);
		display(DisPort1, (unsigned char *)buf1.viraddr[0], framesize);
		/* use opencv convert and qt to show rgb image */
/*
		cvtColor(yuvImg, rgbImg, CV_YUV2BGR_NV12);
#ifdef TEST_COST
		gettimeofday(&starttime, 0);
#endif
		imshow("test_show", rgbImg);
		if (waitKey(1) >= 0)
			break;
#ifdef TEST_COST
		gettimeofday(&endtime, 0);
		timeval_subtract(&difftime, &starttime, &endtime);
		printf("-----imshow time = %f ms , frames:%d\n",
					 difftime.tv_usec / 1000.f, frameNum);
#endif
*/
		camera->enqueue(camera, &buf);
		camera1->enqueue(camera1, &buf1);

		frameNum++;
	}

	printf("------------now end app\n");
	/* aw camera streamoff and deinit and close */
	camera->streamoff(camera);
	camera->deinit(camera);
	camera->close(camera);

	camera1->streamoff(camera1);
	camera1->deinit(camera1);
	camera1->close(camera1);

	/* aw isp stop and destroy */
	ispPort->ispStop(ispid);
	DestroyAWIspApi(ispPort);

	ispPort1->ispStop(ispid1);
	DestroyAWIspApi(ispPort1);
/* close aw display */
	dispClose(DisPort);
	dispClose(DisPort1);
	/* aw eve uninit */
	status = AW_AI_EVE_Event_UnInit(hEveEvent);
	if (AW_STATUS_ERROR == status) {
		printf("AW_AI_EVE_Event_UnInit failure!, errorcode = %d\n",
					 AW_AI_EVE_Event_GetLastError(hEveEvent));
		return -1;
	}
	return 0;
}
