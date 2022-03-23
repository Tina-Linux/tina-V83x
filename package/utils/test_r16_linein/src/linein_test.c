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

#define PRESS_DATA_NAME    "audiocodec linein Jack"
#define PRESS_SYSFS_PATH   "/sys/class/input"
#define MIXER_AUDIO_SET_LINEIN	"Audio linein in"

int fd;
struct input_event buff;
struct mixer *mixer;
struct mixer_ctl *set_linein;

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
			mixer_ctl_set_value(set_linein, 0, 1);
			printf("linein plug in\n");
		} else if (buff.code != 0 && buff.value == 64) {
			printf("linein signal\n");
		} else if (buff.code != 0 && buff.value == 128) {
			mixer_ctl_set_value(set_linein, 0, 0);
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
    char *path = "/sys/module/sun8iw5_sndcodec/parameters/AUX_JACK_DETECT";
	jack_buf[0] = 'N';

	fp = fopen(path, "rb");
	if (fp != NULL) {
		fread(jack_buf, 1, 1, fp);
	}

	if (jack_buf[0] == 'Y')
		mixer_ctl_set_value(set_linein, 0, 1);

	if (jack_buf[0] == 'N')
		mixer_ctl_set_value(set_linein, 0, 0);

	close(fd);
}

int main()
{
	mixer = mixer_open(0);
    if (!mixer) {
	    printf("Unable to open the mixer, aborting.");
	    return -1;
    }
    set_linein = mixer_get_ctl_by_name(mixer, MIXER_AUDIO_SET_LINEIN);
    if (!set_linein) {
	printf("Unable to get set_linein, aborting.");
	    return -1;
    }
    linein_jack_init();
	test_linein();
	mixer_ctl_set_value(set_linein, 0, 0);
	mixer_close(mixer);

	return 0;
}
