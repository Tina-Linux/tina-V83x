#ifndef __CAMERA_PARSE_CONFIG_H__
#define __CAMERA_PARSE_CONFIG_H__

#include <CameraInterface.h>

#define KEY_LENGTH	256
#define CAMERA_KEY_CONFIG_PATH	"/etc/camera.cfg"


#define kNUMBER_OF_CAMERA					"number_of_camera"

#define KCAMERA_ID							"camera_id"

/* camera config */
#define KCAMERA_REQBUF_COUNT				"camera_reqbuf_count"
#define KCAMERA_SIZE						"camera_size"
#define KCAMERA_FORMAT						"camera_format"
#define KCAMERA_FRAMERATE					"camera_framerate"

/* camera supported config */
#define KCAMERA_SUPPORTED_SIZE				"camera_supported_size"
#define KCAMERA_SUPPORTED_FORMAT			"camera_supported_format"
#define KCAMERA_SUPPORTED_FRAMERATE			"camera_supported_framerate"

int readKey(int cameraId, char *key, char *value);
int setKey(int cameraId, char *key, char *value);
int addKey(int cameraId, char *key, char *value);


#endif
