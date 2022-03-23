#include "sunxi_cam.h"

#define SUNXICAM_ERROR(x,arg...) do{ \
								printf("[ERROR]" x,##arg); \
                            }while(0)

#define VIDEO_INDEX 0
#define VIDEO_FORMAT V4L2_PIX_FMT_NV21
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_FPS 10

namespace sunxi_cam {

SunxiCam::SunxiCam():camera(NULL),state(CAM_CLOSE)
{

}

SunxiCam::~SunxiCam()
{
	shutdown();
}

int SunxiCam::start(const int video_index, const int format,
					int fps, int width, int height)
{
	int ret = 0;

	camera = (camera_hal *)calloc(1, sizeof(camera_hal));
	if(!camera){
		SUNXICAM_ERROR("calloc camera_hal fail\n");
		return -1;
	}

	camera->video_index = video_index;
	camera->format = format;
	camera->fps = fps;
	camera->width = width;
	camera->height = height;

	ret = camerainit(camera);
	if(ret < 0){
		SUNXICAM_ERROR("camera init fail\n");
		free(camera);
		camera = NULL;
		state = CAM_CLOSE;
		return -1;
	}
	state = CAM_OPEN;

	ret = setformat(camera);
	if(ret < 0){
		SUNXICAM_ERROR("camera set format fail\n");
		releasecamera(camera);
		free(camera);
		camera = NULL;
		state = CAM_CLOSE;
		return -1;
	}

	ret = requestbuf(camera);
	if(ret < 0){
		SUNXICAM_ERROR("camera request buf fail\n");
		releasecamera(camera);
		free(camera);
		camera = NULL;
		state = CAM_CLOSE;
		return -1;
	}
	state = CAM_REQUESTBUF;

	ret = streamon(camera);
	if(ret < 0){
		SUNXICAM_ERROR("camera stream on fail\n");
		releasebuf(camera);
		releasecamera(camera);
		free(camera);
		camera = NULL;
		state = CAM_CLOSE;
		return -1;
	}
	state = CAM_STREAMON;

	return ret;
}

int SunxiCam::shutdown(void)
{
	int ret = 0;

	if(!camera)
		return 0;

	if(state == CAM_STREAMON){
		ret = streamoff(camera);
		if(ret < 0)
			SUNXICAM_ERROR("camera stream off fail\n");

		state = CAM_REQUESTBUF;
	}

	if(state == CAM_REQUESTBUF){
		ret = releasebuf(camera);
		if(ret < 0)
			SUNXICAM_ERROR("camera release buf fail\n");

		state = CAM_OPEN;
	}

	if(state == CAM_OPEN){
		ret = releasecamera(camera);
		if(ret < 0)
			SUNXICAM_ERROR("release camera fail\n");

		state = CAM_CLOSE;
	}

	state = CAM_CLOSE;
	free(camera);
	camera = NULL;

	return ret;
}

bool SunxiCam::OpenCamera(void)
{
	int ret = 0;

	if(state == CAM_STREAMON)
		return true;

	if(state != CAM_CLOSE)
		return false;

	ret = start(VIDEO_INDEX, VIDEO_FORMAT, VIDEO_FPS, VIDEO_WIDTH, VIDEO_HEIGHT);
	if(ret < 0){
		return false;
	}

	return true;
}

bool SunxiCam::CloseCamera(void)
{
	int ret = 0;

	if(state == CAM_CLOSE)
		return true;

	ret = shutdown();
	if(ret < 0){
		return false;
	}

	return true;
}

bool SunxiCam::ResetSync(void)
{
	if(state != CAM_STREAMON)
		return false;

	return (resetsync(camera) == 0);
}

bool SunxiCam::WaitingBuf(void)
{
	if(state != CAM_STREAMON)
		return false;

	return (waitingbuf(camera) == 0);
}

bool SunxiCam::GetImageFrame(ImageFrame * image_frame)
{
	int buf_index;

	if(state != CAM_STREAMON)
		return false;

	buf_index = dqbuf(camera);
	if(buf_index < 0){
		/* errno == EAGAIN */
		/* SUNXICAM_ERROR("camera dequeue fail\n"); */
		return false;
	}

	image_frame->image_id = camera->buffers[buf_index].image_id;
	image_frame->lost_image_num = camera->buffers[buf_index].lost_image_num;
	image_frame->exp_time = camera->buffers[buf_index].exp_time;
	image_frame->image_timestamp = camera->buffers[buf_index].image_timestamp;
	image_frame->data = (uint8_t*)camera->buffers[buf_index].start[0];

	return true;
}

bool SunxiCam::ReturnImageFrame(ImageFrame * image_frame)
{
	int ret = 0;
	int buf_index = -1;

	if(state != CAM_STREAMON)
		return false;

	for(buf_index = 0; buf_index < camera->buf_count; buf_index++){
		if(camera->buffers[buf_index].start[0] == image_frame->data)
			break;
	}

	if(buf_index >= camera->buf_count){
		SUNXICAM_ERROR("invalid buf index\n");
		return false;
	}

	buf_index = qbuf(camera, buf_index);
	if(buf_index < 0){
		SUNXICAM_ERROR("camera qbuf fail\n");
		return false;
	}

	return true;
}

}
