#include <iostream>
#include <stdio.h>
#include "convert.h"
#include "sunxi_cam.h"

using namespace std;
using namespace sunxi_cam;

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480

#define MAX_FILE_NUM 50

int main(int argc, char** argv)
{
    SunxiCam camera;
	ImageFrame image_frame;
	int count = 0;
	char filename[128] = {0};
	bool result = false;

	result = camera.OpenCamera();
	if(false == result){
		cout << "open camera error" << endl;
		return -1;
	}

	while(1){
		result = camera.GetImageFrame(&image_frame);
		if(false == result){
			//cout << "get image frame error" << endl;
			continue;
		}

		cout << "image_frame.image_id " << image_frame.image_id << " image_frame.lost_image_num " << image_frame.lost_image_num << endl;
		cout << "image_frame.exp_time " << image_frame.exp_time << " image_frame.image_timestamp " << image_frame.image_timestamp << endl;
		//printf(" image_frame.data %p\n", image_frame.data);

		count++;
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "/tmp/sunxicamera%d.bmp", count);

		YUVToBMP(filename, image_frame.data, NV21ToRGB24, VIDEO_WIDTH, VIDEO_HEIGHT);

		result = camera.ReturnImageFrame(&image_frame);
		if(false == result){
			cout << "return image frame error" << endl;
			break;
		}

		if(count >= MAX_FILE_NUM)
			break;
	}

	result = camera.CloseCamera();
	if(false == result){
		cout << "close camera error" << endl;
		return -1;
	}

	return 0;
}
