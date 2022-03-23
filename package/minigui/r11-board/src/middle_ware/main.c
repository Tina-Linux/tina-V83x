#if 0

#include "middle_ware.h"

#if 0 //≤•∑≈≤‚ ‘
int main(int argc, char *argv[])
{
	printf("middle_test_main\n");
	tplayer_init(0);
	if(argc < 2){
		printf("please set file parth\n");
		goto out;
	}
	if(access(argv[1],F_OK) != 0){
		printf("file is not exist\n");
		goto out;
	}

	if(tplayer_play_url(argv[1])){
		printf("play url failed!\n");
		goto out;
	}

	tplayer_play();

	while(!tplayer_getcompletestate())
		sleep(1);

out:
	tplayer_exit();
	printf("test bye bye!\n");
	return 0;
}
#endif

#if 0
int bat_lev;
	int charg;
	int val;
	disp_rectsz rect;
	bat_lev = power_get_battery_level();
	printf("%d\n", bat_lev);
	charg = power_is_charging();
	printf("%d\n", charg);

	volume_init();
	volume_set_volume(30);
	val = volume_get_volume();
	printf("val = %d\n", val);

	display_get_lcd_rect(&rect);
	printf("w = %d, h = %d\n", rect.width, rect.height);

	display_lcd_onoff(0);
	sleep(3);
	display_lcd_onoff(1);
#endif

#if 0
int main(int argc, char *argv[])
{
	create_media_list();
	disp_rect rect = {0, 0, 400, 320};
	media_play_init(rect, 0);
	show_current_file();
	while(1)
		sleep(1);

	return 0;
}
#endif

#if 1
int main(int argc, char *argv[])
{
	int ret = 0;
	ret = create_rec_list();
	if(ret < 0){
		printf("create list failed!\n");
		return -1;
	}
	rec_media_info_t info;
	info.pip_rect.x = 0;
	info.pip_rect.y = 0;
	info.pip_rect.width = 400;
	info.pip_rect.height = 320;
	info.show_rect.x = 0;
	info.show_rect.y = 0;
	info.show_rect.width = 1280;
	info.show_rect.height = 320;
	info.cam_quality_mode[0] = CAMERA_QUALITY_800;
	info.cycle_rec_time[0] = CYCLE_REC_TIME_1_MIM;
	info.mute_en[0] = 0;
	info.pre_mode[0] = PREVIEW_HOST;
	info.rec_quality_mode[0] = RECORD_QUALITY_1920_1080;
	info.source_size[0].width = 1920;
	info.source_size[0].height = 1080;
	info.source_frate[0] = 30;

	info.cam_quality_mode[1] = CAMERA_QUALITY_800;
	info.cycle_rec_time[1] = CYCLE_REC_TIME_1_MIM;
	info.mute_en[1] = 0;
	info.pre_mode[1] = PREVIEW_PIP;
	info.rec_quality_mode[1] = RECORD_QUALITY_1280_720;
	info.source_size[1].width = 1280;
	info.source_size[1].height = 720;
	info.source_frate[1] = 25;
	recorder_init_info(&info);

	recorder_init(0);

	recorder_init(1);

	recorder_start_preview(0);

	recorder_start_preview(1);

	char a;
	while((a = getchar()) != EOF){
		if(a == 'k'){
			recorder_start_recording(0);
		}else if(a == 't'){
			recorder_stop_recording(0);
		}else if(a == 'q'){
			break;
		}
	}

	printf("bye bye!\n");

	return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	disp_rect rect = {200, 0, 400, 320};
	char a;

	media_play_init(rect, 0);
	create_media_list();
	show_current_file();

	while(1){
		sleep(1);
		de_show();
		show_next_file();
		system("free");
	}
	printf("bye bye!\n");

	return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	printf("input a word  to enable usb storage\n");

	getchar();
	getchar();
	usb_storage_adcard_on();
	printf("already on, ready to off\n");
	getchar();
	getchar();
	usb_storage_adcard_off();
	printf("already off \n");
	getchar();
	getchar();
	printf("bye bye!\n");
	return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	int i;
	int val;
	for(i = 0; i < 10; i++){
		printf("rear_cam_det = %d\n", back_car_det());
		sleep(1);
	}
	while(1){
		val = power_key_det();
		printf("power_key_det = %d\n", val);
		if(val == 1)
			break;
		sleep(1);
	}

	return 0;
}
#endif

#endif
