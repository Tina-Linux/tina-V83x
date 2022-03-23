#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>
#include "include/tinyalsa/asoundlib.h"

#define PRESS_DATA_NAME    "sndac100 linein Jack"
#define PRESS_SYSFS_PATH   "/sys/class/input"

int fd;
struct input_event buff;

#define SPEAKER_VOLUME					"speaker volume"
#define RIGHT_OUTPUT_MIXER_LINEINR_SWITCH	"Right Output Mixer LINEINR Switch"
#define LEFT_OUTPUT_MIXER_LINEINL_SWITCH	"Left Output Mixer LINEINL Switch"
#define SPK_L_MUX						"SPK_L Mux"
#define	SPK_R_MUX						"SPK_R Mux"
#define EXTERNAL_SPEAKER_SWITCH			"External Speaker Switch"

int linein_route_en(int enable)
{
	struct mixer *mixer;
	/*SPK OUT*/
	struct mixer_ctl *speaker_volume;
	struct mixer_ctl *right_output_mixer_lineinr_switch;
	struct mixer_ctl *left_output_mixer_lineinl_switch;
	struct mixer_ctl *spk_l_mux;
	struct mixer_ctl *spk_r_mux;
	struct mixer_ctl *external_speaker_switch;

	mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return EXIT_FAILURE;
    }

    speaker_volume = mixer_get_ctl_by_name(mixer, SPEAKER_VOLUME);
    if (!speaker_volume) {
		printf("Unable to get speaker_volume, aborting.");
	    return -1;
    }

    right_output_mixer_lineinr_switch = mixer_get_ctl_by_name(mixer, RIGHT_OUTPUT_MIXER_LINEINR_SWITCH);
    if (!right_output_mixer_lineinr_switch) {
		printf("Unable to get right_output_mixer_lineinr_switch, aborting.");
	    return -1;
    }
    left_output_mixer_lineinl_switch = mixer_get_ctl_by_name(mixer, LEFT_OUTPUT_MIXER_LINEINL_SWITCH);
    if (!left_output_mixer_lineinl_switch) {
		printf("Unable to get left_output_mixer_lineinl_switch, aborting.");
	    return -1;
    }
    spk_l_mux = mixer_get_ctl_by_name(mixer, SPK_L_MUX);
    if (!spk_l_mux) {
		printf("Unable to get spk_l_mux, aborting.");
	    return -1;
    }
    spk_r_mux = mixer_get_ctl_by_name(mixer, SPK_R_MUX);
    if (!spk_r_mux) {
		printf("Unable to get spk_r_mux, aborting.");
	    return -1;
    }
    external_speaker_switch = mixer_get_ctl_by_name(mixer, EXTERNAL_SPEAKER_SWITCH);
    if (!external_speaker_switch) {
		printf("Unable to get external_speaker_switch, aborting.");
	    return -1;
    }
	if (enable) {
		mixer_ctl_set_value(speaker_volume, 0, 59);
		mixer_ctl_set_value(right_output_mixer_lineinr_switch, 0, 1);
		mixer_ctl_set_value(left_output_mixer_lineinl_switch, 0, 1);
		mixer_ctl_set_enum_by_string(spk_l_mux, "MIXEL Switch");
		mixer_ctl_set_enum_by_string(spk_r_mux, "MIXER Switch");
		mixer_ctl_set_value(external_speaker_switch, 0, 1);
	} else {
		mixer_ctl_set_value(right_output_mixer_lineinr_switch, 0, 0);
		mixer_ctl_set_value(left_output_mixer_lineinl_switch, 0, 0);
		mixer_ctl_set_value(external_speaker_switch, 0, 0);
	}
	mixer_close(mixer);
	return 0;
}

int linein_get_class_path(char *class_path)
{
	char dirname[] = PRESS_SYSFS_PATH;
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
		    continue;
		}

		sprintf(class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
		    continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
		    close(fd);
		    continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, PRESS_DATA_NAME) == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);
	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}
}

int test_linein()
{
	int len = 0;
	char class_path[100];

	memset(class_path,0,sizeof(class_path));
	linein_get_class_path(class_path);
	len = strlen(class_path);
	printf("index: %c\n",class_path[len - 1]);

	sprintf(class_path, "/dev/input/event%c", class_path[len - 1]);
	printf("path: %s\n",class_path);

	fd = open(class_path, O_RDONLY); //may be the powerlinein is /dev/input/event1
	if (fd < 0) {
		perror("can not open device usblineinboard!");
		exit(1);
	}
	printf("--fd:%d--\n",fd);
	read(fd,&buff,sizeof(struct input_event));
//	printf("%s,l:%d,d buff.code:%d, buff.value:%d\n", __FUNCTION__, __LINE__, buff.code, buff.value);
	while(1)
	{
		while(read(fd,&buff,sizeof(struct input_event))==0)
		{
			;
		}

		printf("%s,l:%d,d buff.code:%d, buff.value:%d\n", __FUNCTION__, __LINE__, buff.code, buff.value);
		if(buff.code != 0 && buff.value == 32) {
			linein_route_en(1);
			printf("linein plug in\n");
		} else if (buff.code != 0 && buff.value == 64) {
			printf("linein signal\n");
		} else if (buff.code != 0 && buff.value == 128) {
			linein_route_en(0);
			printf("linein plug out\n");
		}
	}
	close(fd);

	return 0;
}

void linein_jack_init()
{
    int fd = NULL;
    FILE *fp;
    char jack_buf[16];
    char *path = "/sys/module/ac100_dapm/parameters/AUX_JACK_DETECT";
	jack_buf[0] = 'N';

	fp = fopen(path, "rb");
	if (fp != NULL) {
		fread(jack_buf, 1, 1, fp);
	}

	if (jack_buf[0] == 'Y')
		linein_route_en(1);

	if (jack_buf[0] == 'N')
		linein_route_en(0);

	close(fd);
}

int main()
{
    linein_jack_init();
	test_linein();
	linein_route_en(0);

	return 0;
}
