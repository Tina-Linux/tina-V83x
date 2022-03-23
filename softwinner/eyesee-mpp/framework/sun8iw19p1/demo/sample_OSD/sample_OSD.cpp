//#define LOG_NDEBUG 0
#define LOG_TAG "sample_OSD"
#include <utils/plat_log.h>

#include <sys/stat.h>
#include <cstdio>
#include <csignal>
#include <iostream>

#include <hwdisplay.h>
#include <confparser.h>
#include <mpi_sys.h>
#include <mpi_vo.h>
#include <BITMAP_S.h>

#include "sample_OSD_config.h"
#include "sample_OSD.h"

using namespace std;
using namespace EyeseeLinux;

static SampleOSDContext *pSampleOSDContext = NULL;

static void handle_exit(int signo)
{
    alogd("user want to exit!");
    if(pSampleOSDContext!=NULL)
    {
        cdx_sem_up(&pSampleOSDContext->mSemExit);
    }
}

bool EyeseeCameraListener::onInfo(int chnId, CameraMsgInfoType info, int extra, EyeseeCamera *pCamera)
{
    bool bHandleInfoFlag = true;
    switch(info)
    {
        case CAMERA_INFO_RENDERING_START:
        {
            if(chnId == mpContext->mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1])
            {
                cdx_sem_up(&mpContext->mSemRenderStart);
            }
            else
            {
                aloge("channel[%d] notify render start, but channel[%d] wait render start!", chnId, mpContext->mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
            }
            break;
        }
        default:
        {
            aloge("fatal error! unknown info[0x%x] from channel[%d]", info, chnId);
            bHandleInfoFlag = false;
            break;
        }
    }
    return bHandleInfoFlag;
}

void EyeseeCameraListener::onPictureTaken(int chnId, const void *data, int size, EyeseeCamera *pCamera)
{
    int ret = -1;
    alogd("channel %d picture data size %d", chnId, size);
    if(chnId != mpContext->mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0])
    {
        aloge("fatal error! channel[%d] is not match current channel[%d]", chnId, mpContext->mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
    }
    char picName[64];
    sprintf(picName, "pic[%02d].jpg", 0 /*mpContext->mPicNum++*/);
    std::string PicFullPath = mpContext->mConfigPara.mEncodeFolderPath + '/' + picName;
    FILE *fp = fopen(PicFullPath.c_str(), "wb");
    fwrite(data, 1, size, fp);
    fclose(fp);
}

EyeseeCameraListener::EyeseeCameraListener(SampleOSDContext *pContext)
    : mpContext(pContext)
{
}

void EyeseeRecorderListener::onError(EyeseeRecorder *pMr, int what, int extra)
{
    alogd("receive onError message![%d][%d]", what, extra);
    switch(what)
    {
        case EyeseeRecorder::MEDIA_ERROR_SERVER_DIED:
            break;
        default:
            break;
    }
}

void EyeseeRecorderListener::onInfo(EyeseeRecorder *pMr, int what, int extra)
{
    switch(what)
    {
        case EyeseeRecorder::MEDIA_RECORDER_INFO_NEED_SET_NEXT_FD:
        {
            aloge("fatal error! We don't test this function in this sample! why receive onInfo message: need_set_next_fd, muxer_id[%d]?", extra);
            break;
        }
        case EyeseeRecorder::MEDIA_RECORDER_INFO_RECORD_FILE_DONE:
        {
            alogd("receive onInfo message: record_file_done,  muxer_id[%d]", extra);
            break;
        }
        default:
            alogd("receive onInfo message! media_info_type[%d] extra[%d]", what, extra);
            break;
    }
}

void EyeseeRecorderListener::onData(EyeseeRecorder *pMr, int what, int extra)
{
    aloge("fatal error! Do not test CallbackOut!");
}

EyeseeRecorderListener::EyeseeRecorderListener(SampleOSDContext *pOwner)
    : mpOwner(pOwner)
{
}
SampleOSDContext::SampleOSDContext()
    :mCameraListener(this)
    , mRecorderListener(this)
{
    cdx_sem_init(&mSemExit, 0);
    cdx_sem_init(&mSemRenderStart, 0);
    mUILayer = HLAY(2, 0);
    mVoDev = -1;
    mVoLayer = -1;
    mpCamera = NULL;
    mpRecorder = NULL;
    mVippOverlayHandle = MM_INVALID_HANDLE;
    mVippCoverHandle = MM_INVALID_HANDLE;
    mVencOverlayHandle = MM_INVALID_HANDLE;
    mVencCoverHandle = MM_INVALID_HANDLE;
}

SampleOSDContext::~SampleOSDContext()
{
    cdx_sem_deinit(&mSemExit);
    cdx_sem_deinit(&mSemRenderStart);
    if(mpCamera!=NULL)
    {
        aloge("fatal error! EyeseeCamera is not destruct!");
    }
}

status_t SampleOSDContext::ParseCmdLine(int argc, char *argv[])
{
    alogd("this program path:[%s], arg number is [%d]", argv[0], argc);
    status_t ret = NO_ERROR;
    int i=1;
    while(i < argc)
    {
        if(!strcmp(argv[i], "-path"))
        {
            if(++i >= argc)
            {
                std::string errorString;
                errorString = "fatal error! use -h to learn how to set parameter!!!";
                cout<<errorString<<endl;
                ret = -1;
                break;
            }
            mCmdLinePara.mConfigFilePath = argv[i];
        }
        else if(!strcmp(argv[i], "-h"))
        {
            std::string helpString;
            helpString += "CmdLine param example:\n";
            helpString += "\t run -path /home/sample_OSD.conf\n";
            cout<<helpString<<endl;
            ret = 1;
            break;
        }
        else
        {
            std::string ignoreString;
            ignoreString += "ignore invalid CmdLine param:[";
            ignoreString += argv[i];
            ignoreString += ", type -h to get how to set parameter!";
            cout<<ignoreString<<endl;
        }
        i++;
    }
    return ret;
}

status_t SampleOSDContext::loadConfig()
{
    int ret;
    char *ptr;
    std::string& ConfigFilePath = mCmdLinePara.mConfigFilePath;
    if(ConfigFilePath.empty())
    {
        alogd("user not set config file. use default test parameter!");
        mConfigPara.mCaptureWidth = 1920;
        mConfigPara.mCaptureHeight = 1080;
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YUV_AW_AFBC;
        mConfigPara.mPreviewWidth = 1920;
        mConfigPara.mPreviewHeight = 1080;
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        mConfigPara.mDigitalZoom = 0;
        mConfigPara.mFrameRate = 30;
        mConfigPara.mDisplayFrameRate = 0;
        mConfigPara.mDisplayWidth = 640; 
        mConfigPara.mDisplayHeight = 480;
        mConfigPara.mPreviewRotation = 0;

        mConfigPara.mVippOverlayX = 0;
        mConfigPara.mVippOverlayY = 0;
        mConfigPara.mVippOverlayW = 100;
        mConfigPara.mVippOverlayH = 100;

        mConfigPara.mVippCoverX = 200;
        mConfigPara.mVippCoverY = 0;
        mConfigPara.mVippCoverW = 100;
        mConfigPara.mVippCoverH = 100;
        mConfigPara.mVippCoverColor = 0x0000ff;

        mConfigPara.mVencOverlayX = 0;
        mConfigPara.mVencOverlayY = 300;
        mConfigPara.mVencOverlayW = 100;
        mConfigPara.mVencOverlayH = 100;

        mConfigPara.mVencCoverX = 200;
        mConfigPara.mVencCoverY = 300;
        mConfigPara.mVencCoverW = 100;
        mConfigPara.mVencCoverH = 100;
        mConfigPara.mVencCoverColor = 0x00ff00;
        
        mConfigPara.mEncodeType = PT_H265;
        mConfigPara.mAudioEncodeType = PT_MAX;
        mConfigPara.mEncodeWidth = 1920;
        mConfigPara.mEncodeHeight = 1080;
        mConfigPara.mEncodeBitrate = 6*1024*1024;
        mConfigPara.mEncodeFolderPath = "/home/sample_OSD_Files";
        mConfigPara.mFilePath = "file_osd.mp4";
        mConfigPara.mTestDuration = 20;
        return SUCCESS;
    }
    CONFPARSER_S stConfParser;
    ret = createConfParser(ConfigFilePath.c_str(), &stConfParser);
    if(ret < 0)
    {
        aloge("load conf fail");
        return UNKNOWN_ERROR;
    }
    mConfigPara.mCaptureWidth = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_CAPTURE_WIDTH, 0);
    mConfigPara.mCaptureHeight = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_CAPTURE_HEIGHT, 0);
    char *pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_CAPTURE_FORMAT, NULL); 
    if(!strcmp(pStrPixelFormat, "yu12"))
    {
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YUV_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "yv12"))
    {
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YVU_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv21"))
    {
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv12"))
    {
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "afbc"))
    {
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YUV_AW_AFBC;
    }
    else
    {
        aloge("fatal error! conf file pic_format is [%s]?", pStrPixelFormat);
        mConfigPara.mCaptureFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    mConfigPara.mPreviewWidth = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_PREVIEW_WIDTH, 0);
    mConfigPara.mPreviewHeight = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_PREVIEW_HEIGHT, 0);
    pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_PREVIEW_FORMAT, NULL);
    if(!strcmp(pStrPixelFormat, "yu12"))
    {
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YUV_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "yv12"))
    {
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YVU_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv21"))
    {
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv12"))
    {
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "afbc"))
    {
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YUV_AW_AFBC;
    }
    else
    {
        aloge("fatal error! conf file pic_format is [%s]?", pStrPixelFormat);
        mConfigPara.mPreviewFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    mConfigPara.mDigitalZoom = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_DIGITAL_ZOOM, 0);
    
    mConfigPara.mFrameRate = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_FRAME_RATE, 0); 
    mConfigPara.mDisplayFrameRate = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_DISPLAY_FRAME_RATE, 0); 
    mConfigPara.mDisplayWidth = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_DISPLAY_WIDTH, 0); 
    mConfigPara.mDisplayHeight = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_DISPLAY_HEIGHT, 0);
    mConfigPara.mPreviewRotation = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_PREVIEW_ROTATION, 0);

    mConfigPara.mVippOverlayX = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_OVERLAY_X, 0);
    mConfigPara.mVippOverlayY = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_OVERLAY_Y, 0);
    mConfigPara.mVippOverlayW = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_OVERLAY_W, 0);
    mConfigPara.mVippOverlayH = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_OVERLAY_H, 0);

    mConfigPara.mVippCoverX = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_COVER_X, 0);
    mConfigPara.mVippCoverY = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_COVER_Y, 0);
    mConfigPara.mVippCoverW = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_COVER_W, 0);
    mConfigPara.mVippCoverH = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_COVER_H, 0);
    mConfigPara.mVippCoverColor = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VIPP_COVER_COLOR, 0);

    mConfigPara.mVencOverlayX = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_OVERLAY_X, 0);
    mConfigPara.mVencOverlayY = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_OVERLAY_Y, 0);
    mConfigPara.mVencOverlayW = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_OVERLAY_W, 0);
    mConfigPara.mVencOverlayH = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_OVERLAY_H, 0);

    mConfigPara.mVencCoverX = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_COVER_X, 0);
    mConfigPara.mVencCoverY = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_COVER_Y, 0);
    mConfigPara.mVencCoverW = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_COVER_W, 0);
    mConfigPara.mVencCoverH = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_COVER_H, 0);
    mConfigPara.mVencCoverColor = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_VENC_COVER_COLOR, 0);

    char *pStrEncodeType = (char*)GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_ENCODE_TYPE, NULL); 
    if(!strcmp(pStrEncodeType, "h264"))
    {
        mConfigPara.mEncodeType = PT_H264;
    }
    else if(!strcmp(pStrEncodeType, "h265"))
    {
        mConfigPara.mEncodeType = PT_H265;
    }
    else if(!strcmp(pStrEncodeType, "mjpeg"))
    {
        mConfigPara.mEncodeType = PT_MJPEG;
    }
    else
    {
        aloge("fatal error! conf file encode_type is [%s]?", pStrEncodeType);
        mConfigPara.mEncodeType = PT_H264;
    }
    pStrEncodeType = (char*)GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_AUDIO_ENCODE_TYPE, NULL); 
    if(pStrEncodeType!=NULL)
    {
        if(!strcmp(pStrEncodeType, "aac"))
        {
            mConfigPara.mAudioEncodeType = PT_AAC;
        }
        else if(!strcmp(pStrEncodeType, "mp3"))
        {
            mConfigPara.mAudioEncodeType = PT_MP3;
        }
        else if(!strcmp(pStrEncodeType, ""))
        {
            alogd("user set no audio.");
            mConfigPara.mAudioEncodeType = PT_MAX;
        }
        else
        {
            aloge("fatal error! conf file audio encode type is [%s]?", pStrEncodeType);
            mConfigPara.mAudioEncodeType = PT_MAX;
        }
    }
    else
    {
        mConfigPara.mAudioEncodeType = PT_MAX;
    }
    mConfigPara.mEncodeWidth = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_ENCODE_WIDTH, 0);
    mConfigPara.mEncodeHeight = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_ENCODE_HEIGHT, 0);
    mConfigPara.mEncodeBitrate = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_ENCODE_BITRATE, 0)*1024*1024;
    mConfigPara.mEncodeFolderPath = GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_ENCODE_FOLDER, NULL);
    mConfigPara.mFilePath = GetConfParaString(&stConfParser, SAMPLE_OSD_KEY_FILE_PATH, NULL);

    mConfigPara.mTestDuration = GetConfParaInt(&stConfParser, SAMPLE_OSD_KEY_TEST_DURATION, 0);
    destroyConfParser(&stConfParser);
    return SUCCESS;
}

status_t SampleOSDContext::CreateFolder(const std::string& strFolderPath)
{
    if(strFolderPath.empty())
    {
        aloge("jpeg path is not set!");
        return UNKNOWN_ERROR;
    }
    const char* pFolderPath = strFolderPath.c_str();
    //check folder existence
    struct stat sb;
    if (stat(pFolderPath, &sb) == 0)
    {
        if(S_ISDIR(sb.st_mode))
        {
            return SUCCESS;
        }
        else
        {
            aloge("fatal error! [%s] is exist, but mode[0x%x] is not directory!", pFolderPath, sb.st_mode);
            return UNKNOWN_ERROR;
        }
    }
    //create folder if necessary
    int ret = mkdir(pFolderPath, S_IRWXU | S_IRWXG | S_IRWXO);
    if(!ret)
    {
        alogd("create folder[%s] success", pFolderPath);
        return SUCCESS;
    }
    else
    {
        aloge("fatal error! create folder[%s] failed!", pFolderPath);
        return UNKNOWN_ERROR;
    }
}

int main(int argc, char *argv[])
{
    int result = 0;
    cout<<"hello, sample_Camera_2VIPP!"<<endl;
    SampleOSDContext stContext;
    pSampleOSDContext = &stContext;
    //parse command line param
    if(stContext.ParseCmdLine(argc, argv) != NO_ERROR)
    {
        //aloge("fatal error! command line param is wrong, exit!");
        result = -1;
        return result;
    }
    //parse config file.
    if(stContext.loadConfig() != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        return result;
    }
    //check folder existence, create folder if necessary
    if(SUCCESS != stContext.CreateFolder(stContext.mConfigPara.mEncodeFolderPath))
    {
        return -1;
    }
    //register process function for SIGINT, to exit program.
    if (signal(SIGINT, handle_exit) == SIG_ERR)
        perror("can't catch SIGSEGV");
    //init mpp system
    memset(&stContext.mSysConf, 0, sizeof(MPP_SYS_CONF_S));
    stContext.mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&stContext.mSysConf);
    AW_MPI_SYS_Init();

    //config camera.
    {
        EyeseeCamera::clearCamerasConfiguration();
        int cameraId;
        CameraInfo cameraInfo;
        VI_CHN chn;
        ISPGeometry mISPGeometry;
        cameraId = 0;
        cameraInfo.facing = CAMERA_FACING_BACK;
        cameraInfo.orientation = 0;
        cameraInfo.canDisableShutterSound = true;
        cameraInfo.mCameraDeviceType = CameraInfo::CAMERA_CSI;
        cameraInfo.mMPPGeometry.mCSIChn = 1;
        mISPGeometry.mISPDev = 0;
        chn = 1;
        mISPGeometry.mScalerOutChns.push_back(chn);
        chn = 3;
        mISPGeometry.mScalerOutChns.push_back(chn);
        cameraInfo.mMPPGeometry.mISPGeometrys.push_back(mISPGeometry);
        EyeseeCamera::configCameraWithMPPModules(cameraId, &cameraInfo);
    }
    int cameraId;
    cameraId = 0;
    EyeseeCamera::getCameraInfo(cameraId, &stContext.mCameraInfo);
    stContext.mpCamera = EyeseeCamera::open(cameraId);
    stContext.mpCamera->setInfoCallback(&stContext.mCameraListener);
    stContext.mpCamera->prepareDevice(); 
    stContext.mpCamera->startDevice();

    CameraParameters cameraParam;
    stContext.mpCamera->openChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0], true);
    stContext.mpCamera->getParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0], cameraParam);
    SIZE_S captureSize={(unsigned int)stContext.mConfigPara.mCaptureWidth, (unsigned int)stContext.mConfigPara.mCaptureHeight};
    cameraParam.setVideoSize(captureSize);
    cameraParam.setPreviewFrameRate(stContext.mConfigPara.mFrameRate);
    //cameraParam.setDisplayFrameRate(stContext.mConfigPara.mDisplayFrameRate);
    cameraParam.setPreviewFormat(stContext.mConfigPara.mCaptureFormat);
    cameraParam.setVideoBufferNumber(6);
    //cameraParam.setPreviewRotation(stContext.mConfigPara.mPreviewRotation);
    stContext.mpCamera->setParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0], cameraParam);
    stContext.mpCamera->prepareChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);

    stContext.mpCamera->openChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], false);
    stContext.mpCamera->getParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], cameraParam);
    SIZE_S previewSize={(unsigned int)stContext.mConfigPara.mPreviewWidth, (unsigned int)stContext.mConfigPara.mPreviewHeight};
    cameraParam.setVideoSize(previewSize);
    cameraParam.setPreviewFrameRate(stContext.mConfigPara.mFrameRate);
    cameraParam.setDisplayFrameRate(stContext.mConfigPara.mDisplayFrameRate);
    cameraParam.setPreviewFormat(stContext.mConfigPara.mPreviewFormat);
    cameraParam.setVideoBufferNumber(6);
    cameraParam.setPreviewRotation(stContext.mConfigPara.mPreviewRotation);
    stContext.mpCamera->setParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], cameraParam);
    stContext.mpCamera->prepareChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
    
    stContext.mVoDev = 0;
    AW_MPI_VO_Enable(stContext.mVoDev);
    AW_MPI_VO_AddOutsideVideoLayer(stContext.mUILayer);
    AW_MPI_VO_CloseVideoLayer(stContext.mUILayer);//close ui layer.
    VO_LAYER hlay = 0;
    while(hlay < VO_MAX_LAYER_NUM)
    {
        if(SUCCESS == AW_MPI_VO_EnableVideoLayer(hlay))
        {
            break;
        }
        hlay++;
    }
    if(hlay >= VO_MAX_LAYER_NUM)
    {
        aloge("fatal error! enable video layer fail!");
    }
    AW_MPI_VO_GetVideoLayerAttr(hlay, &stContext.mLayerAttr);
    stContext.mLayerAttr.stDispRect.X = 0;
    stContext.mLayerAttr.stDispRect.Y = 0;
    stContext.mLayerAttr.stDispRect.Width = stContext.mConfigPara.mDisplayWidth;
    stContext.mLayerAttr.stDispRect.Height = stContext.mConfigPara.mDisplayHeight;
    AW_MPI_VO_SetVideoLayerAttr(hlay, &stContext.mLayerAttr);
    stContext.mVoLayer = hlay;
    //camera preview test
    alogd("prepare setPreviewDisplay(), hlay=%d", stContext.mVoLayer);
    stContext.mpCamera->setChannelDisplay(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], stContext.mVoLayer);

    stContext.mpCamera->startChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
    stContext.mpCamera->startChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
    int ret;
    ret = cdx_sem_down_timedwait(&stContext.mSemRenderStart, 5000);
    if(0 == ret)
    {
        alogd("app receive message that camera start render!");
    }
    else if(ETIMEDOUT == ret)
    {
        aloge("fatal error! wait render start timeout");
    }
    else
    {
        aloge("fatal error! other error[0x%x]", ret);
    }

    if(stContext.mConfigPara.mDigitalZoom > 0)
    {
        ret = cdx_sem_down_timedwait(&stContext.mSemExit, 5*1000);
        if(0 == ret)
        {
            alogd("user want to exit!");
            goto _exit0;
        }
        stContext.mpCamera->getParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], cameraParam);
        int oldZoom = cameraParam.getZoom();
        cameraParam.setZoom(stContext.mConfigPara.mDigitalZoom);
        stContext.mpCamera->setParameters(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], cameraParam);
        alogd("change digital zoom[%d]->[%d]", oldZoom, stContext.mConfigPara.mDigitalZoom);
    }
    //test overlay and cover
    if(stContext.mConfigPara.mVippOverlayW!=0 && stContext.mConfigPara.mVippOverlayH!=0)
    {
        RGN_ATTR_S stRegion;
        memset(&stRegion, 0, sizeof(RGN_ATTR_S));
        stRegion.enType = OVERLAY_RGN;
        stRegion.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;
        stRegion.unAttr.stOverlay.mSize = {(unsigned int)stContext.mConfigPara.mVippOverlayW, (unsigned int)stContext.mConfigPara.mVippOverlayH};
        stContext.mVippOverlayHandle = stContext.mpCamera->createRegion(&stRegion);
        BITMAP_S stBmp;
        memset(&stBmp, 0, sizeof(BITMAP_S));
        stBmp.mPixelFormat = stRegion.unAttr.stOverlay.mPixelFmt;
        stBmp.mWidth = stRegion.unAttr.stOverlay.mSize.Width;
        stBmp.mHeight = stRegion.unAttr.stOverlay.mSize.Height;
        int nSize = BITMAP_S_GetdataSize(&stBmp);
        stBmp.mpData = malloc(nSize);
        if(NULL == stBmp.mpData)
        {
            aloge("fatal error! malloc fail!");
        }
        memset(stBmp.mpData, 0xaa, nSize);
        stContext.mpCamera->setRegionBitmap(stContext.mVippOverlayHandle, &stBmp);
        free(stBmp.mpData);
        RGN_CHN_ATTR_S stRgnChnAttr = {0};
        stRgnChnAttr.bShow = TRUE;
        stRgnChnAttr.enType = stRegion.enType;
        stRgnChnAttr.unChnAttr.stOverlayChn.stPoint = {stContext.mConfigPara.mVippOverlayX, stContext.mConfigPara.mVippOverlayY};
        stRgnChnAttr.unChnAttr.stOverlayChn.mLayer = 0;
        stContext.mpCamera->attachRegionToChannel(stContext.mVippOverlayHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0], &stRgnChnAttr);
        stContext.mpCamera->attachRegionToChannel(stContext.mVippOverlayHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], &stRgnChnAttr);
    }
    if(stContext.mConfigPara.mVippCoverW!=0 && stContext.mConfigPara.mVippCoverH!=0)
    {
        RGN_ATTR_S stRegion;
        memset(&stRegion, 0, sizeof(RGN_ATTR_S));
        stRegion.enType = COVER_RGN;
        stContext.mVippCoverHandle = stContext.mpCamera->createRegion(&stRegion);
        RGN_CHN_ATTR_S stRgnChnAttr = {0};
        stRgnChnAttr.bShow = TRUE;
        stRgnChnAttr.enType = stRegion.enType;
        stRgnChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
        stRgnChnAttr.unChnAttr.stCoverChn.stRect = {stContext.mConfigPara.mVippCoverX, stContext.mConfigPara.mVippCoverY, (unsigned int)stContext.mConfigPara.mVippCoverW, (unsigned int)stContext.mConfigPara.mVippCoverH};
        stRgnChnAttr.unChnAttr.stCoverChn.mColor = stContext.mConfigPara.mVippCoverColor;
        stRgnChnAttr.unChnAttr.stCoverChn.mLayer = 0;
        stContext.mpCamera->attachRegionToChannel(stContext.mVippCoverHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0], &stRgnChnAttr);
        stContext.mpCamera->attachRegionToChannel(stContext.mVippCoverHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1], &stRgnChnAttr);
    }
    if(!stContext.mConfigPara.mFilePath.empty())
    {
        //record first file.
        stContext.mpRecorder = new EyeseeRecorder();
        stContext.mpRecorder->setOnInfoListener(&stContext.mRecorderListener);
        stContext.mpRecorder->setOnDataListener(&stContext.mRecorderListener);
        stContext.mpRecorder->setOnErrorListener(&stContext.mRecorderListener);
        //stContext.mpRecorder->setCamera(stContext.mpCamera);
        stContext.mpRecorder->setVideoSource(EyeseeRecorder::VideoSource::CAMERA);
        stContext.mpRecorder->setAudioSource(EyeseeRecorder::AudioSource::MIC);
        std::string strFilePath = stContext.mConfigPara.mEncodeFolderPath + '/' + stContext.mConfigPara.mFilePath;
        int nFallocateLen = 0;
        //nFallocateLen = 100*1024*1024;
        stContext.mpRecorder->addOutputFormatAndOutputSink(MEDIA_FILE_FORMAT_MP4, (char*)strFilePath.c_str(), nFallocateLen, false);
        alogd("setVideoFrameRate=[%d]", stContext.mConfigPara.mFrameRate);
        stContext.mpRecorder->setVideoFrameRate(stContext.mConfigPara.mFrameRate);
        alogd("setEncodeVideoSize=[%dx%d]", stContext.mConfigPara.mEncodeWidth, stContext.mConfigPara.mEncodeHeight);
        stContext.mpRecorder->setVideoSize(stContext.mConfigPara.mEncodeWidth, stContext.mConfigPara.mEncodeHeight);
        stContext.mpRecorder->setVideoEncoder(stContext.mConfigPara.mEncodeType);
        //alogd("enable long term ref");
        //stContext.mpRecorder->enableVideoEncodingLongTermRef(true);
        alogd("setIDRFrameInterval=[%d]", stContext.mConfigPara.mFrameRate*2);
        stContext.mpRecorder->setVideoEncodingIFramesNumberInterval(stContext.mConfigPara.mFrameRate*2);
        //alogd("setVirtualIFrameInterval=[%d]", stContext.mConfigPara.mFrameRate);
        //stContext.mpRecorder->setVirtualIFrameInterval(stContext.mConfigPara.mFrameRate);
        //VencSmartFun stVencSmartParam = {1, 1, 0, 2};
        //stContext.mpRecorder->setVideoEncodingSmartP(&stVencSmartParam);
        stContext.mpRecorder->setVideoEncodingRateControlMode(EyeseeRecorder::VideoRCMode_CBR);
        stContext.mpRecorder->setVideoEncodingBitRate(stContext.mConfigPara.mEncodeBitrate);
        alogd("setSourceChannel=[%d]", stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
        //stContext.mpRecorder->setSourceChannel(stContext.mCameraInfo.mMPPGeometry.mScalerOutChns[0]);
        stContext.mpRecorder->setCameraProxy(stContext.mpCamera->getRecordingProxy(), stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
        if(stContext.mConfigPara.mAudioEncodeType != PT_MAX)
        {
            stContext.mpRecorder->setAudioSamplingRate(8000);
            stContext.mpRecorder->setAudioChannels(1);
            stContext.mpRecorder->setAudioEncodingBitRate(12200);
            stContext.mpRecorder->setAudioEncoder(PT_AAC);
        }
        stContext.mpRecorder->setCaptureRate(0);
        alogd("prepare()!");
        stContext.mpRecorder->prepare();
        alogd("start()!");
        stContext.mpRecorder->start();
        alogd("will record [%d]s!", stContext.mConfigPara.mTestDuration);
        if(stContext.mConfigPara.mVencOverlayW!=0 && stContext.mConfigPara.mVencOverlayH!=0)
        {
            RGN_ATTR_S stRegion;
            memset(&stRegion, 0, sizeof(RGN_ATTR_S));
            stRegion.enType = OVERLAY_RGN;
            stRegion.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;
            stRegion.unAttr.stOverlay.mSize = {(unsigned int)stContext.mConfigPara.mVencOverlayW, (unsigned int)stContext.mConfigPara.mVencOverlayH};
            stContext.mVencOverlayHandle = stContext.mpRecorder->createRegion(&stRegion);
            BITMAP_S stBmp;
            memset(&stBmp, 0, sizeof(BITMAP_S));
            stBmp.mPixelFormat = stRegion.unAttr.stOverlay.mPixelFmt;
            stBmp.mWidth = stRegion.unAttr.stOverlay.mSize.Width;
            stBmp.mHeight = stRegion.unAttr.stOverlay.mSize.Height;
            int nSize = BITMAP_S_GetdataSize(&stBmp);
            stBmp.mpData = malloc(nSize);
            if(NULL == stBmp.mpData)
            {
                aloge("fatal error! malloc fail!");
            }
            memset(stBmp.mpData, 0xbb, nSize);
            stContext.mpRecorder->setRegionBitmap(stContext.mVencOverlayHandle, &stBmp);
            free(stBmp.mpData);
            RGN_CHN_ATTR_S stRgnChnAttr = {0};
            stRgnChnAttr.bShow = TRUE;
            stRgnChnAttr.enType = stRegion.enType;
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint = {stContext.mConfigPara.mVencOverlayX, stContext.mConfigPara.mVencOverlayY};
            stRgnChnAttr.unChnAttr.stOverlayChn.mLayer = 0;
            stContext.mpRecorder->attachRegionToVenc(stContext.mVencOverlayHandle, &stRgnChnAttr);
        }
        if(stContext.mConfigPara.mVencCoverW!=0 && stContext.mConfigPara.mVencCoverH!=0)
        {
            RGN_ATTR_S stRegion;
            memset(&stRegion, 0, sizeof(RGN_ATTR_S));
            stRegion.enType = COVER_RGN;
            stContext.mVencCoverHandle = stContext.mpRecorder->createRegion(&stRegion);
            RGN_CHN_ATTR_S stRgnChnAttr = {0};
            stRgnChnAttr.bShow = TRUE;
            stRgnChnAttr.enType = stRegion.enType;
            stRgnChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stRgnChnAttr.unChnAttr.stCoverChn.stRect = {stContext.mConfigPara.mVencCoverX, stContext.mConfigPara.mVencCoverY, (unsigned int)stContext.mConfigPara.mVencCoverW, (unsigned int)stContext.mConfigPara.mVencCoverH};
            stRgnChnAttr.unChnAttr.stCoverChn.mColor = stContext.mConfigPara.mVencCoverColor;
            stRgnChnAttr.unChnAttr.stCoverChn.mLayer = 0;
            stContext.mpRecorder->attachRegionToVenc(stContext.mVencCoverHandle, &stRgnChnAttr);
        }
    }

    if(stContext.mConfigPara.mTestDuration > 0)
    {
        cdx_sem_down_timedwait(&stContext.mSemExit, stContext.mConfigPara.mTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&stContext.mSemExit);
    }

    if(stContext.mpRecorder)
    {
        if(stContext.mVencOverlayHandle >= 0)
        {
            stContext.mpRecorder->detachRegionFromVenc(stContext.mVencOverlayHandle);
            stContext.mpRecorder->destroyRegion(stContext.mVencOverlayHandle);
            stContext.mVencOverlayHandle = MM_INVALID_HANDLE;
        }
        if(stContext.mVencCoverHandle >= 0)
        {
            stContext.mpRecorder->detachRegionFromVenc(stContext.mVencCoverHandle);
            stContext.mpRecorder->destroyRegion(stContext.mVencCoverHandle);
            stContext.mVencCoverHandle = MM_INVALID_HANDLE;
        }
        stContext.mpRecorder->stop();
        delete stContext.mpRecorder;
        stContext.mpRecorder = NULL;
    }

    //detach region from vipp and destroy
    if(stContext.mVippOverlayHandle >= 0)
    {
        stContext.mpCamera->detachRegionFromChannel(stContext.mVippOverlayHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
        stContext.mpCamera->detachRegionFromChannel(stContext.mVippOverlayHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
        stContext.mpCamera->destroyRegion(stContext.mVippOverlayHandle);
        stContext.mVippOverlayHandle = MM_INVALID_HANDLE;
    }
    if(stContext.mVippCoverHandle >= 0)
    {
        stContext.mpCamera->detachRegionFromChannel(stContext.mVippCoverHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
        stContext.mpCamera->detachRegionFromChannel(stContext.mVippCoverHandle, stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
        stContext.mpCamera->destroyRegion(stContext.mVippCoverHandle);
        stContext.mVippCoverHandle = MM_INVALID_HANDLE;
    }

_exit0:
    //close camera
    alogd("HerbCamera::release()");
    alogd("HerbCamera stopPreview()");
    stContext.mpCamera->stopChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
    stContext.mpCamera->releaseChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
    stContext.mpCamera->closeChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[0]);
    stContext.mpCamera->stopChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
    stContext.mpCamera->releaseChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
    stContext.mpCamera->closeChannel(stContext.mCameraInfo.mMPPGeometry.mISPGeometrys[0].mScalerOutChns[1]);
    stContext.mpCamera->stopDevice(); 
    stContext.mpCamera->releaseDevice(); 
    EyeseeCamera::close(stContext.mpCamera);
    stContext.mpCamera = NULL;
    //close vo
    AW_MPI_VO_DisableVideoLayer(stContext.mVoLayer);
    stContext.mVoLayer = -1;
    AW_MPI_VO_RemoveOutsideVideoLayer(stContext.mUILayer);
    AW_MPI_VO_Disable(stContext.mVoDev);
    stContext.mVoDev = -1;
    //exit mpp system
    AW_MPI_SYS_Exit(); 
    cout<<"bye, sample_Camera!"<<endl;
    return result;
}

