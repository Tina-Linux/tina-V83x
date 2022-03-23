/*
 * Copyright (C) 2018 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <CameraInterface.h>

#include "CameraLog.h"
#include "CameraParseConfig.h"
#include "CameraV4l2.h"

static int CameraEnumFrameInfo(void *hdl)
{
	Camera *handle = (Camera *) hdl;

	V4L2EnumFmt(handle);
	V4L2TryFmt(handle);
	V4L2EnumFrameSizes(handle);
	/* most driver not do it */
	V4L2EnumFrameIntervals(handle);

	return 0;
}

int CameraOpen(void *hdl, int index)
{
	Camera *handle = (Camera *) hdl;
	char dev_name[30];

	/* check camera state */
	if (handle->state != CAMERA_STATE_NONE) {
		CamErr("[%s] camera[%d] state err\n", __func__, index);
		return -1;
	}
	/* open video node according to the index */
	handle->index = index;
	snprintf(dev_name, sizeof(dev_name), "/dev/video%d", index);
	handle->fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (handle->fd < 0) {
		CamErr("[%s] open camera[%d] error\n", __func__, handle->index);
		return -1;
	} else
	    CamDebug("[%s] open %s ok\n", __func__, dev_name);

	/* query device capabilityies */
	V4L2QueryCap(handle);
	/* set current video input */
	V4L2SetInput(handle);
	/* enum frame info */
	CameraEnumFrameInfo(handle);
	/* update support config */
	CameraUpdateSupportConfig(handle);

	/* state change from NONE to CREATED */
	handle->state = CAMERA_STATE_CREATED;
	return 0;
}

int CameraClose(void *hdl)
{
	Camera *handle = (Camera *) hdl;

	/* check camera state */
	if (handle->state != CAMERA_STATE_CREATED) {
		CamErr("[%s] camera[%d] state err\n", __func__, handle->index);
		return -1;
	}

	//CameraCleanFrameInfo();

	close(handle->fd);
	/* state change from CREATED to NONE */
	handle->state = CAMERA_STATE_NONE;
	return 0;
}

int CameraInit(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	int ret;

	if (CameraGetConfig(handle) < 0)
		CamErr("[%s] camera[%d] can not get config from camera.cfg\n", \
				__func__, handle->index);

	V4L2SetParm(handle);
	V4L2SetFormat(handle);
	V4L2RequestQueue(handle);
	/* state change from CREATED to PREPARED */
	handle->state = CAMERA_STATE_PREPARED;
	return 0;
}

int CameraDeInit(void *hdl)
{
	Camera *handle = (Camera *) hdl;
	int ret;

	V4L2ReleaseQueue(handle);
	V4L2GetFormat(handle);
	V4L2GetParm(handle);

	if (CameraSetConfig(handle) < 0)
		CamErr("[%s] camera[%d] can not set config to camera.cfg\n", \
				__func__, handle->index);

	/* state change from PREPARED to CREATED */
	handle->state = CAMERA_STATE_CREATED;
	return 0;
}

int CameraStreamOn(void *hdl)
{
	Camera *handle = (Camera *) hdl;

	V4L2StreamOn(handle);
	/* state change from PREPARED to CAPTURING */
	handle->state = CAMERA_STATE_CAPTURING;
	return 0;
}

int CameraStreamOff(void *hdl)
{
	Camera *handle = (Camera *) hdl;

	V4L2StreamOff(handle);
	/* state change from CAPTURING to PREPARED */
	handle->state = CAMERA_STATE_PREPARED;
	return 0;
}

int CameraDequeue(void *hdl, CameraBuffer *buf)
{
	Camera *handle = (Camera *) hdl;
	fd_set fds;
	struct timeval tv;
	int ret;

	FD_ZERO(&fds);
	FD_SET(handle->fd, &fds);
	tv.tv_sec = MAX_SELECT_TIME;
	tv.tv_usec = 0;
	/* ensure data are finished or timeout */
	ret = select(handle->fd + 1, &fds, NULL, NULL, &tv);

	V4L2DQBUF(handle, buf);

	return 0;

}


int CameraEnqueue(void *hdl, CameraBuffer *buf)
{
	Camera *handle = (Camera *) hdl;

	V4L2QBUF(handle, buf);

	return 0;

}

Camera *CreateCameraDevice()
{
	Camera *camdev = (Camera *)malloc(sizeof(Camera));
	if (camdev == NULL)
		return NULL;

	memset(camdev, 0, sizeof(Camera));
	camdev->open		= CameraOpen;
	camdev->close		= CameraClose;
	camdev->init		= CameraInit;
	camdev->deinit		= CameraDeInit;
	camdev->streamon	= CameraStreamOn;
	camdev->streamoff	= CameraStreamOff;
	camdev->dequeue		= CameraDequeue;
	camdev->enqueue		= CameraEnqueue;

	return camdev;
}
