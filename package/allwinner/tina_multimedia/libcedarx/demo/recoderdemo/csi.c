#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <time.h>
#include <linux/fb.h>
#include <pthread.h>
#include <semaphore.h>
#include "csi.h"
#include "disp.h"
#if MODULE_CSI_COMPILE_CFG == 1

enum v4l2_resolution{
	V4L2_RESOLUTION_UXGA	=0,
    V4L2_RESOLUTION_720P    =1,
    V4L2_RESOLUTION_XGA     =2,
    V4L2_RESOLUTION_SVGA    =3,
	V4L2_RESOLUTION_VGA     =4,
	V4L2_RESOLUTION_QVGA     =4,
};

int VFLip=1, HFLip=3;

/*********************************************************************************************************
*
*  Static Define
**********************************************************************************************************/
#define CSI_CLAR(x)			memset(&(x),  0, sizeof(x))

/*********************************************************************************************************
*
*  Static Variable
**********************************************************************************************************/
static INT8U CSI_LOG = 1;
#define CSI_DBG_LOG	 if(CSI_LOG)  printf

INT8U *csi_last_down_smpl_buf = NULL;
INT8U *csi_cur_down_smpl_buf = NULL;
INT8U csi_smpl_buf_compare_flag = 0;

/*********************************************************************************************************
*
*  Extern And Global Variable
**********************************************************************************************************/
INT32S csi_v4l2_fd;
CSI_BUF csi_buf;
pthread_mutex_t csi_lock;
INT32S csi_ctrl_set(INT32S fd,INT32U id,INT32S val)
{
	struct v4l2_control control;
	INT32S ret;
	control.id = id;
	control.value = val;

	ret = ioctl(fd, VIDIOC_S_CTRL, &control);
	return ret;
}

INT32S csi_get_fd()
{
	return csi_v4l2_fd;
}

#if 0
/*********************************************************************************************************
*
*  Function Define
**********************************************************************************************************/
INT32S csi_ctrl_set(INT32S fd, CSI_CTRL_ID ctrlId, INT32S ctrlVal)
{
	INT32S ret = 0;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control;
	#define V4L2_CID_RESOLUTION						(V4L2_CID_BASE+19)
	#define V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE	(V4L2_CID_CAMERA_CLASS_BASE+20)
	#define V4L2_CID_FLIP							(V4L2_CID_BASE+31)
	#define SW_ROTATE								4
	#define NO_ROTATE								5
	#define V4L2_CID_LIGHT_FREQ						(V4L2_CID_BASE+24)

	if(ctrlId >= CSI_CTRL_ID_MAX){
		CSI_DBG_LOG("%s:param error\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	CSI_CLAR(queryctrl);
	CSI_CLAR(control);

	if(ctrlId == CSI_CTRL_ID_SET_RESOLUTION){

		queryctrl.id = V4L2_CID_RESOLUTION;
		control.id   = V4L2_CID_RESOLUTION;
		switch(ctrlVal)
		{
		    case SENSOR_RESOLUTION_1920X1080:   control.value = V4l2_API_RESOLUTION_720P;		break;
			case SENSOR_RESOLUTION_1280X720:	control.value = V4l2_API_RESOLUTION_720P;				break;
			case SENSOR_RESOLUTION_640X480:	    control.value = V4l2_API_RESOLUTION_VGA;			break;
			default:							control.value = queryctrl.default_value;
		}
	}else if(ctrlId == CSI_CTRL_ID_SET_HUE){
		queryctrl.id = V4L2_CID_HUE;
		control.id   = V4L2_CID_HUE;
		switch(ctrlVal)
		{
			case HUE_NONE:					control.value = V4l2_API_COLORFX_NONE;				break;
			case HUE_MONO:					control.value = V4l2_API_COLORFX_BW;					break;
			case HUE_NEGATIVE:					control.value = V4l2_API_COLORFX_NEGATIVE;			break;
			case HUE_RETRO:					control.value = V4l2_API_COLORFX_SEPIA;				break;
			case HUE_SKY_BLUE:				control.value = V4l2_API_COLORFX_SKY_BLUE;			break;
			case HUE_GRASS_GREEN:				control.value = V4l2_API_COLORFX_GRASS_GREEN;			break;
			default:							control.value = queryctrl.default_value;
		}
	}else if(ctrlId == CSI_CTRL_ID_SET_AUTO_AWB){
		queryctrl.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
		control.id   = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;

		switch(ctrlVal)
		{
			case AWB_AUTO:						control.value = V4l2_API_WHITE_BALANCE_AUTO;			break;
			case AWB_SUNNY:					control.value = V4l2_API_WHITE_BALANCE_CLOUDY;	        break;
			case AWB_OVERCAST:				control.value = AWB_OVERCAST;						    break; // T.B.D
			case AWB_DAYLIGHT:				control.value = V4l2_API_WHITE_BALANCE_DAYLIGHT;	    break;
			case AWB_MANUAL:					control.value = V4l2_API_WHITE_BALANCE_MANUAL;			break;
			case AWB_INCANDESCENCE:				control.value = V4l2_API_WHITE_BALANCE_INCANDESCENT;	break; // 炽热色
			case AWB_FLUORESCENT:				control.value = V4l2_API_WHITE_BALANCE_FLUORESCENT;		break; // 荧光色
			case AWB_TUNGSTEN:					control.value = V4l2_API_WHITE_BALANCE_TUNGSTEN;        break; // 钨色
			case AWB_HORIZON:				control.value = V4l2_API_WHITE_BALANCE_HORIZON;		break; // T.B.D
			case AWB_FLASH:					control.value = V4l2_API_WHITE_BALANCE_FLASH;		    break; // T.B.D
			case AWB_SHADE:					control.value = V4l2_API_WHITE_BALANCE_SHADE;			break; // T.B.D
			default:							control.value = queryctrl.default_value;
		}
	}
	else if(ctrlId == CSI_CTRL_ID_SET_AE){
		queryctrl.id = V4L2_CID_EXPOSURE;
		control.id   = V4L2_CID_EXPOSURE;

		if(ctrlVal >= -4 && ctrlVal <= 4){
				control.value = ctrlVal;
		}else{
				control.value = queryctrl.default_value;
		}
	}
	else if(ctrlId == CSI_CTRL_ID_SET_LIGHT_FREQ){
		queryctrl.id = V4L2_CID_LIGHT_FREQ;
		control.id   = V4L2_CID_LIGHT_FREQ;
		switch(ctrlVal)
		{
			case LIGHT_FREQ_MODE_50_HZ:			control.value = V4l2_API_LIGHT_FREQ_50Hz;				break;
			case LIGHT_FREQ_MODE_60_HZ:		control.value = V4l2_API_LIGHT_FREQ_60Hz;			break;
			default:							control.value = queryctrl.default_value;
		}

	}else if(ctrlId == CSI_CTRL_ID_SET_FLIP){
		queryctrl.id = V4L2_CID_FLIP;
		control.id   = V4L2_CID_FLIP;
		switch(ctrlVal)
		{
			case VFLIP_SET_ZERO:    control.value = SW_ROTATE; break;
			case VFLIP_SET_POS1:
					if(VFLip)	   VFLip = 0;
					else		   VFLip = 1;
					 control.value = VFLip;
					break;
			case HFLIP_SET_ZERO:	control.value = NO_ROTATE; break;
			case HFLIP_SET_POS1:
					if(HFLip == 2) HFLip = 3;
					else		   HFLip = 2;
					control.value = HFLip;
					break;
			default:				control.value = queryctrl.default_value;
		}
	}else if(ctrlId == CSI_CTRL_ID_SET_BRIGHTNESS){
		queryctrl.id = V4L2_CID_BRIGHTNESS;
		control.id   = V4L2_CID_BRIGHTNESS;

		if(ctrlVal >= -4 && ctrlVal <= 4){
			control.value = ctrlVal;
		}else{
			control.value = queryctrl.default_value;
		}
	}else if(ctrlId == CSI_CTRL_ID_SET_SATURATION){
		queryctrl.id = V4L2_CID_SATURATION;
		control.id   = V4L2_CID_SATURATION;

		if(ctrlVal >= -4 && ctrlVal <= 4){
			control.value = ctrlVal;
		}else{
			control.value = queryctrl.default_value;
		}
	}else if(ctrlId == CSI_CTRL_ID_SET_AG){
		queryctrl.id = V4L2_CID_AUTOGAIN;
		control.id   = V4L2_CID_AUTOGAIN;

		if(ctrlVal >= -4 && ctrlVal <= 4){
			control.value = ctrlVal;
		}else{
			control.value = queryctrl.default_value;
		}
	}

	ret = ioctl(fd, VIDIOC_S_CTRL, &control);
	if(ret != 0){
		CSI_DBG_LOG("%s:ioctl v4l2 set control.id = %d, control.value = %d error!!!\r\n", __func__, control.id, control.value);
		ret = -3;
		goto errHdl;
	}

	CSI_DBG_LOG("%s:control.id = %d, control.value = %d\r\n", __func__, control.id, control.value);
errHdl:
	return ret;
}
#endif

INT32S csi_input_set(INT32S fd, INT32U idx)
{
	INT32S ret = 0;
	struct v4l2_input input;

	CSI_CLAR(input);

	input.index = idx;
    ret = ioctl(fd, VIDIOC_S_INPUT, (void *)&input);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 input set error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

errHdl:
	return ret;
}
INT32S csi_steam_param_set(INT32S fd, INT16U frmRate)
{
	INT32S ret = 0;
	struct v4l2_streamparm streamparam;

	CSI_CLAR(streamparam);

	streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparam.parm.capture.timeperframe.numerator = 1;
    streamparam.parm.capture.timeperframe.denominator = frmRate;
    streamparam.parm.capture.capturemode = V4L2_MODE_VIDEO;
    ret = ioctl(fd, VIDIOC_S_PARM, (void *)&streamparam);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 streamparam set error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

errHdl:
	return ret;
}

INT32S csi_fmt_set(INT32S fd, INT16U width, INT16U height, INT32U fmt)
{
	INT32S ret = 0;
	struct v4l2_format format;
	INT16U sensorWidth = 0, sensorHeight = 0;
	if((width == 1920) && (height == 1080)){
		ret = csi_steam_param_set(csi_v4l2_fd, CSI_INPUT_1080P_FRM_RATE);
		if(ret != 0){
			CSI_DBG_LOG("%s:set csi 1080P frame rate error!\r\n", __func__);
			ret = -1;
			goto errHdl;
		}
	}else{
		ret = csi_steam_param_set(csi_v4l2_fd, CSI_INPUT_720P_FRM_RATE);
		if(ret != 0){
			CSI_DBG_LOG("%s:set csi 720P frame rate error!\r\n", __func__);
			ret = -2;
			goto errHdl;
		}
	}
	if((width == 1920) && (height == 1080)){
		sensorWidth = 1920;
		sensorHeight = 1080;
	}else{
		sensorWidth = width;
		sensorHeight = height;
	}

	CSI_CLAR(format);
	format.type			= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width	= sensorWidth;
	format.fmt.pix.height	= sensorHeight;
	format.fmt.pix.pixelformat	= fmt;
	format.fmt.pix.field	= V4L2_FIELD_NONE;// V4L2_FIELD_INTERLACED
	ret = ioctl(fd, VIDIOC_S_FMT, (void *)&format);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 format set error!\r\n", __func__);
		ret = -3;
		goto errHdl;
    }

	ret = ioctl(fd, VIDIOC_G_FMT, (void *)&format);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 format get error!\r\n", __func__);
		ret = -4;
		goto errHdl;
    }

	CSI_DBG_LOG("Senor Resolution = %dX%d\n",format.fmt.pix.width,format.fmt.pix.height);
errHdl:
	return ret;
}

INT32S csi_querycap_get(INT32S fd)
{
	INT32S ret = 0;
	struct v4l2_capability cap;

	CSI_CLAR(cap);
    ret = ioctl(fd, VIDIOC_QUERYCAP, (void *)&cap);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 querycap get error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

	CSI_DBG_LOG("Senor capabilities:V4L2_CAP_VIDEO_CAPTURE = %d\n",cap.capabilities); // 0x01
errHdl:
	return ret;
}

INT32S csi_querystd_get(INT32S fd)
{
	INT32S ret = 0;
	/*struct v4l2_std_id stdId;

    ret = ioctl(fd, VIDIOC_QUERYSTD, (void *)&stdId);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 querystd get error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

	switch(std){
	case V4L2_STD_NTSC:
		 CSI_DBG_LOG("current std: NTSC format\r\n");
		 break;
	case V4L2_STD_PAL:
		 CSI_DBG_LOG("current std: PAL format\r\n");
		 break;
	default:
		 break;
	}*/

//errHdl:
	return ret;
}

INT32S csi_down_smpl_buf_create(INT16U width, INT16U height)
{
	if((width == 0) ||
	   (height == 0)){
		CSI_DBG_LOG("%s:param error!\r\n", __func__);
		return -1;
	}

	csi_smpl_buf_compare_flag = 0;

	if(csi_cur_down_smpl_buf == NULL){
		csi_cur_down_smpl_buf = (INT8U *)malloc(width * height);
	}

	if(csi_last_down_smpl_buf == NULL){
		csi_last_down_smpl_buf = (INT8U *)malloc(width * height);
	}

	return 0;
}

INT32S csi_down_smpl_buf_destroy(void)
{
	if(csi_last_down_smpl_buf){
		free(csi_last_down_smpl_buf);
		csi_last_down_smpl_buf = NULL;
	}

	if(csi_cur_down_smpl_buf){
		free(csi_cur_down_smpl_buf);
		csi_cur_down_smpl_buf = NULL;
	}

	csi_smpl_buf_compare_flag = 0;

	return 0;
}

INT32S csi_down_sampling(INT8U *src, INT8U *dst, INT16U width, INT16U height)
{
	INT32S nPos, ret = 0;
	INT16U i = 0, j = 0;
	INT8U *p_dst = NULL;

	if((src == NULL) ||
	   (dst == NULL) ||
	   (width == 0) ||
	   (height == 0)){
		CSI_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	p_dst = dst;
	for(i = 0; i < height; i += 4){
		nPos = i * width;
		for(j = 0; j < width; j += 4){
			*p_dst++ = src[nPos + j];
		}
	}

errHdl:
	return ret;
}

INT8U csi_is_move(INT8U *img1, INT8U *img2, INT16U width, INT16U height)
{
	INT32U sum = 0;
	INT16U	i = 0, j = 0;
	INT8U *p_img1 = NULL, *p_img2 = NULL;
	INT8U diff = 0;

	if((img1 == NULL) ||
	   (img2 == NULL) ||
	   (width == 0) ||
	   (height == 0)){
		CSI_DBG_LOG("%s:param error!\r\n", __func__);
		return 0;
	}

	p_img1 = img1;
	p_img2 = img2;
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			diff = abs(*p_img1++ - *p_img2++);
			sum += diff > 20 ? 1 : 0;
		}
	}

	return sum;
}

INT32S csi_buf_enqueue(CSI_BUF *csiBufAddr)
{
	INT32S ret = 0;
	struct v4l2_buffer v4l2Buf;

	CSI_CLAR(v4l2Buf);

	//CSI_DBG_LOG("%s:index = %d, addr = 0x%x\r\n", __func__, csiBufAddr->m_v4l2BufIdx, csiBufAddr->m_v4l2Buf[csiBufAddr->m_v4l2BufIdx].m_addr);
	v4l2Buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2Buf.memory	= V4L2_MEMORY_MMAP;
	v4l2Buf.index	= csiBufAddr->m_v4l2BufIdx;
	ret = ioctl(csi_v4l2_fd, VIDIOC_QBUF, (void *)&v4l2Buf);
	if(ret != 0)
    {

		//CSI_DBG_LOG("%s:ioctl v4l2 buf enqueue error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

errHdl:
	return ret;
}

INT32S csi_buf_dequeue(CSI_BUF *csiBufAddr, INT64U *curV4l2BufMsec, INT64U *diffV4l2BufMsec)
{
	INT32S ret = 0;
	struct v4l2_buffer v4l2Buf;
	fd_set fdSet;
	struct timeval tv;
	static INT32U lastFrmMsec = 0;
	INT32U curFrmMsec = 0, diffFrmMsec = 1;
	//bug14:WJQ:2016-7-5 start
	INT16U sensorWidth = 0, sensorHeight = 0;
	//bug14:WJQ:2016-7-5 end
	INT8S waterMaskStr[64];

#if 0
	DATE_TIME dateTime;
	POINT tmpP;
	DATE_TIME_WATER_MARK dateTimeWaterMarkDisp = DATE_TIME_WATER_MARK_MAX;
#endif
	if( (csiBufAddr == NULL) ||
		(curV4l2BufMsec == NULL) ||
		(diffV4l2BufMsec == NULL)){
		CSI_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	FD_ZERO(&fdSet);
	FD_SET(csi_v4l2_fd, &fdSet);

	tv.tv_sec = CSI_SELECT_FRM_TIMEOUT;
	tv.tv_usec = 0;
	ret = select(csi_v4l2_fd + 1, &fdSet, NULL, NULL, &tv);
	if(ret == -1){
		CSI_DBG_LOG("select csi input frm error\r\n");
		ret = -2;
		goto errHdl;
	}else if(ret == 0){
		CSI_DBG_LOG("select csi input frm timeout\r\n");
		ret = -3;
		goto errHdl;
	}

	CSI_CLAR(v4l2Buf);
	v4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2Buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(csi_v4l2_fd, VIDIOC_DQBUF, (void *)&v4l2Buf);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 buf dequeue error!\r\n", __func__);
		ret = -4;
		goto errHdl;
    }

	pthread_mutex_lock(&csi_lock);
	if(v4l2Buf.index >= CSI_INPUT_FRM_BUF_MAX){
		CSI_DBG_LOG("%s:ioctl v4l2 buf dequeue index error!\r\n", __func__);
		pthread_mutex_unlock(&csi_lock);
		ret = -5;
		goto errHdl;
	}

	//CSI_DBG_LOG("%s:index = %d, addr = 0x%x\r\n", __func__, v4l2Buf.index, v4l2Buf.m.offset);
	csiBufAddr->m_v4l2BufIdx = v4l2Buf.index;
	csiBufAddr->m_v4l2Buf[csiBufAddr->m_v4l2BufIdx].m_addr = v4l2Buf.m.offset;
	csi_buf.m_v4l2BufIdx = v4l2Buf.index;
	csiBufAddr->m_buf_start = csi_buf.m_v4l2Buf[v4l2Buf.index].m_addr;

#if 0
	// add water mask
	sys_param_date_time_water_mark_get(&dateTimeWaterMarkDisp);
	if(dateTimeWaterMarkDisp == DATE_TIME_WATER_MARK_YES){
		sys_param_date_time_get(&dateTime);
		sys_param_sensor_resoulution_get((INT16U*)&sensorWidth,(INT16U*)&sensorHeight);
		//bug14:WJQ:2016-7-5 start
		snprintf(waterMaskStr, sizeof(waterMaskStr), "%04d-%02d-%02d %02d:%02d:%02d", dateTime.m_date.m_year, dateTime.m_date.m_month, dateTime.m_date.m_day,
																					  dateTime.m_time.m_hour, dateTime.m_time.m_minute, dateTime.m_time.m_sec);
		tmpP.x = GUI_WATER_MASK_POS_X_PIXEL;
		if((sensorWidth == 640) && (sensorHeight == 480)){
			tmpP.y = GUI_WATER_MASK_VGA_POS_Y_PIXEL;
		}else{
			tmpP.y = GUI_WATER_MASK_720P_POS_Y_PIXEL;
		}

		if(csi_buf.m_v4l2Buf[csi_buf.m_v4l2BufIdx].m_addr == NULL){
			printf("%s:water mark->invalid csi frame address!\r\n", __func__);
		}else{
			//gui_yuv420p_disp_watermask(&tmpP, waterMaskStr, (INT8U *)(csi_buf.m_v4l2Buf[csi_buf.m_v4l2BufIdx].m_addr));
			gui_yuv420spmb32_disp_watermask(&tmpP, waterMaskStr, (INT8U *)(csi_buf.m_v4l2Buf[csi_buf.m_v4l2BufIdx].m_addr));
		}
		//bug14:WJQ:2016-7-5 end
	}
#endif
	// gettimeofday(&(v4l2Buf.timestamp), NULL);
	curFrmMsec = v4l2Buf.timestamp.tv_sec * 1000 + v4l2Buf.timestamp.tv_usec / 1000;
	if(lastFrmMsec != 0){
		diffFrmMsec = curFrmMsec - lastFrmMsec;
	}
	lastFrmMsec = curFrmMsec;

	CSI_DBG_LOG("%s:curFrmMsec = %ums, diffFrmMsec = %ums\r\n", __func__, curFrmMsec, diffFrmMsec);

	*curV4l2BufMsec  = curFrmMsec;
	*diffV4l2BufMsec = diffFrmMsec;
	pthread_mutex_unlock(&csi_lock);
errHdl:
	return ret;
}

INT32S csi_buf_request(INT32S fd, INT8U frmNum)
{
	INT32S ret = 0;
	INT8U i = 0;
	struct v4l2_requestbuffers requestbuffers;
	struct v4l2_buffer v4l2Buf;
	//CSI_BUF csiV4l2Buf;

	CSI_CLAR(requestbuffers);
	requestbuffers.count	= frmNum;
	requestbuffers.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	requestbuffers.memory	= V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_REQBUFS, (void *)&requestbuffers);
	if(ret != 0 || requestbuffers.count != CSI_INPUT_FRM_BUF_MAX)
    {
		CSI_DBG_LOG("%s:ioctl v4l2 buf request error!, ret = %d, count = %d\r\n", __func__, ret, requestbuffers.count);
		ret = -1;
		goto errHdl;
    }

	CSI_DBG_LOG("%s:count = %d\r\n", __func__, requestbuffers.count);

	pthread_mutex_lock(&csi_lock);
	for(i = 0; i < requestbuffers.count; i++){
		CSI_CLAR(v4l2Buf);
		v4l2Buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2Buf.memory		= V4L2_MEMORY_MMAP;
		v4l2Buf.index		= i;
		ret = ioctl(fd, VIDIOC_QUERYBUF, (void *)&v4l2Buf);
		if(ret != 0)
	    {
			CSI_DBG_LOG("%s:ioctl v4l2 buf query error, ret = %d!\r\n", __func__, ret);
			pthread_mutex_unlock(&csi_lock);
			ret = -2;
			goto errHdl;
	    }

		csi_buf.m_v4l2BufIdx = i;
		csi_buf.m_v4l2BufLen = v4l2Buf.length;
		csi_buf.m_v4l2Buf[i].m_addr = (INT32U)mmap(NULL, /* start anywhere */
										     v4l2Buf.length,
											 PROT_READ | PROT_WRITE, /* required */
											 MAP_SHARED, /* recommended */
											 fd,
											 v4l2Buf.m.offset);
		csi_buf.m_v4l2Buf[i].m_ref = 0;
		if((void *)(csi_buf.m_v4l2Buf[i].m_addr) == MAP_FAILED)
		{
			CSI_DBG_LOG("%s:mmap kernel space alloc buf to user space error!\r\n", __func__);
			pthread_mutex_unlock(&csi_lock);
			ret = -3;
			goto errHdl;
		}

		/*CSI_DBG_LOG("%s:m_v4l2BufIdx = %d, m_v4l2BufLen =%d, m_v4l2Buf[%d].m_addr = 0x%x\r\n", __func__,
						    csi_buf.m_v4l2BufIdx, csi_buf.m_v4l2BufLen, i, csi_buf.m_v4l2Buf[i].m_addr);*/
	}
	pthread_mutex_unlock(&csi_lock);
errHdl:
	return ret;
}

INT32S csi_buf_release(void)
{
	INT32S ret = 0;
	INT32U i = 0;

	pthread_mutex_lock(&csi_lock);
	for(i = 0; i < CSI_INPUT_FRM_BUF_MAX; i++) {
		/*
		memset((void *)csi_buf.m_v4l2Buf[i].m_addr, 0x00, (csi_buf.m_v4l2BufLen * 2) / 3); // y
		memset((void *)(csi_buf.m_v4l2Buf[i].m_addr + (csi_buf.m_v4l2BufLen * 2) / 3), 0x80, (csi_buf.m_v4l2BufLen) / 6); // cb
		memset((void *)(csi_buf.m_v4l2Buf[i].m_addr + (csi_buf.m_v4l2BufLen * 5) / 6), 0x80, (csi_buf.m_v4l2BufLen) / 6); // cr
		*/
		ret = munmap((void *)csi_buf.m_v4l2Buf[i].m_addr, csi_buf.m_v4l2BufLen);
		if(ret != 0){
			CSI_DBG_LOG("munmap csi frm buf error\r\n");
			pthread_mutex_unlock(&csi_lock);
			ret = -1;
			goto errHdl;
		}
	}

	pthread_mutex_unlock(&csi_lock);
errHdl:
	return ret;
}

INT32S csi_stream_on(INT32S fd)
{
	INT32S ret = 0;
	INT8U i = 0;
	enum v4l2_buf_type type;
	CSI_BUF csiBufAddr;

	for(i = 0; i < CSI_INPUT_FRM_BUF_MAX; i++){
		csiBufAddr.m_v4l2BufIdx = i;
		ret = csi_buf_enqueue(&csiBufAddr);
		if(ret != 0)
	    {
			CSI_DBG_LOG("%s:ioctl v4l2 buf enqueue error!\r\n", __func__);
			ret = -1;
			goto errHdl;
	    }
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, (void *)&type);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl video stream on error!\r\n", __func__);
		ret = -2;
		goto errHdl;
    }

errHdl:
	return ret;
}

INT32S csi_stream_off(INT32S fd)
{
	INT32S ret = 0;
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMOFF, (void *)&type);
	if(ret != 0)
    {
		CSI_DBG_LOG("%s:ioctl video stream off error!\r\n", __func__);
		ret = -1;
		goto errHdl;
    }

errHdl:
	return ret;
}

#if 0
INT32S csi_default_param_set(void)
{
	INT32S ret = 0;

	// resolution
	csi_stream_off(csi_v4l2_fd);
	ret = csi_fmt_set(csi_v4l2_fd, 1280, 720, CSI_INPUT_FMT);
	if(ret != 0){
		CSI_DBG_LOG("%s:resolution set failed! ret = %d\n", __func__, ret);
		ret = -1;
		goto errHdl;
	}
	sys_param_sensor_resoulution_set(SENSOR_RESOLUTION_1280X720);
	csi_stream_on(csi_v4l2_fd);

	// AWB
	sys_param_sensor_awb_set(AWB_AUTO);
	ret = csi_ctrl_set(csi_v4l2_fd, CSI_CTRL_ID_SET_AUTO_AWB, AWB_AUTO);
	if(ret != 0){
		CSI_DBG_LOG("%s:awb set failed! ret = %d\n", __func__, ret);
		ret = -2;
		goto errHdl;
	}

	// AE
	sys_param_sensor_ae_set(AE_0);
	ret = csi_ctrl_set(csi_v4l2_fd, CSI_CTRL_ID_SET_AE, AE_0);
	if(ret != 0){
		CSI_DBG_LOG("%s:ae set failed! ret = %d\n", __func__, ret);
		ret = -3;
		goto errHdl;
    }

	// LIGHT FREQ
	sys_param_light_freq_mode_set(LIGHT_FREQ_MODE_50_HZ);
	ret = csi_ctrl_set(csi_v4l2_fd, CSI_CTRL_ID_SET_LIGHT_FREQ, LIGHT_FREQ_MODE_50_HZ);
	if(ret != 0){
		CSI_DBG_LOG("%s:band set failed! ret = %d\n", __func__, ret);
		ret = -4;
		goto errHdl;
	}

	// FLIP
	sys_param_image_flip_mode_set(IMAGE_FLIP_MODE_OFF);
	ret = csi_ctrl_set(csi_v4l2_fd, CSI_CTRL_ID_SET_FLIP, IMAGE_FLIP_MODE_OFF);
	if(ret != 0){
		CSI_DBG_LOG("%s:band set failed! ret = %d\n", __func__, ret);
		ret = -4;
		goto errHdl;
	}

errHdl:
	return ret;
}
#endif

INT32S csi_system_param_set(void)
{
	INT32S ret = 0;
	INT16U sensorWidth = 1280, sensorHeight = 720;

	#define V4L2_CID_LIGHT_FREQ						(V4L2_CID_BASE+24)
	#define V4L2_CID_FLIP							(V4L2_CID_BASE+31)
	#define SW_ROTATE								4
	#define NO_ROTATE								5
	#define V4L2_CID_RESOLUTION					(V4L2_CID_BASE+19)
//	sys_param_sensor_resoulution_get(&sensorWidth, &sensorHeight);
//	sys_param_sensor_awb_get(&tmpAwb);
//	sys_param_sensor_ae_get(&tmpAE) ;
//	sys_param_light_freq_mode_get(&tmpLightFreq);
//	sys_param_image_flip_mode_get(&tmpImgFlipMode);

	// Resolution Set
	csi_stream_off(csi_v4l2_fd);
	ret = csi_fmt_set(csi_v4l2_fd, sensorWidth, sensorHeight, CSI_INPUT_FMT);
	if(ret != 0){
		CSI_DBG_LOG("%s:resolution set failed! ret = %d\n", __func__, ret);
		ret = -1;
		goto errHdl;
	}

	csi_stream_on(csi_v4l2_fd);

	// AWB Set
	ret = csi_ctrl_set(csi_v4l2_fd, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, V4l2_API_WHITE_BALANCE_AUTO);
	if(ret != 0){
		CSI_DBG_LOG("%s:awb set failed! ret = %d\n", __func__, ret);
		ret = -2;
		goto errHdl;
	}

	// AE Set
	ret = csi_ctrl_set(csi_v4l2_fd, V4L2_CID_EXPOSURE, 0);
	if(ret != 0){
		ret = -3;
		goto errHdl;
    }

	// Light Freq Set
	ret = csi_ctrl_set(csi_v4l2_fd, V4L2_CID_LIGHT_FREQ, V4l2_API_LIGHT_FREQ_50Hz);
	if(ret != 0){
		CSI_DBG_LOG("%s:band set failed! ret = %d\n", __func__, ret);
		ret = -4;
		goto errHdl;
	}

	// Image Flip Set
	ret = csi_ctrl_set(csi_v4l2_fd, V4L2_CID_FLIP, NO_ROTATE);
	if(ret != 0){
		CSI_DBG_LOG("%s:flip set failed! ret = %d\n", __func__, ret);
		ret = -5;
		goto errHdl;
	}


	ret = csi_ctrl_set(csi_v4l2_fd, V4L2_CID_RESOLUTION, V4l2_API_RESOLUTION_720P);
	if(ret != 0){
		CSI_DBG_LOG("%s:resolution set failed! ret = %d\n", __func__, ret);
		ret = -5;
		goto errHdl;
	}
errHdl:
	return ret;
}

INT32S csi_open(INT8S *path)
{
	INT32S csiFd = 0;

	if(path == NULL){
		CSI_DBG_LOG("%s:param error!\r\n", __func__);
		csiFd = -1;
		goto errHdl;
	}

	csiFd = open(path, O_RDWR | O_NONBLOCK, 0);
	if(csiFd == -1){
		CSI_DBG_LOG("%s:open csi handle error!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return csiFd;
}

INT32S csi_close(INT32S csiFd)
{
	INT32S ret = 0;

	ret = close(csiFd);
	if(ret != 0){
		CSI_DBG_LOG("%s:close csi handle failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S csi_init(void)
{
	INT32S ret = 0;

	pthread_mutex_init(&csi_lock, NULL);

	csi_v4l2_fd = csi_open(CSI_DEV_NAME);
	if(csi_v4l2_fd == -1){
		CSI_DBG_LOG("%s:open csi handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = csi_input_set(csi_v4l2_fd, CSI_INPUT_ID_OV5640);
	if(ret != 0){
		CSI_DBG_LOG("%s:set csi input error!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

	ret = csi_fmt_set(csi_v4l2_fd, CSI_WIDTH_MAX, CSI_HEIGHT_MAX, CSI_INPUT_FMT);
	if(ret != 0){
		CSI_DBG_LOG("%s:set csi resolution and format error!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

	ret = csi_buf_request(csi_v4l2_fd, CSI_INPUT_FRM_BUF_MAX);
	if(ret != 0){
		CSI_DBG_LOG("%s:request csi buf error!\r\n", __func__);
		ret = -4;
	}
	/*ret = csi_system_param_set();
	if(ret != 0){
		CSI_DBG_LOG("%s:restore csi system param set error!\r\n", __func__);
		ret = -5;
	}*/
errHdl:
	return ret;
}

INT32S csi_uninit(void)
{
	INT32S ret = 0;

	ret = csi_buf_release();
	if(ret != 0){
		CSI_DBG_LOG("%s:free csi frame buffer failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = csi_close(csi_v4l2_fd);
	if(ret != 0){
		CSI_DBG_LOG("%s:close csi handle failed!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

	pthread_mutex_destroy(&csi_lock);

errHdl:
	return ret;
}

#endif // #if MODULE_CSI_COMPILE_CFG == 1
