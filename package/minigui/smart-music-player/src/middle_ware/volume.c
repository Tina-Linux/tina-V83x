#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static unsigned int vol;

int audio_mixer_get(const char* shell, const char* name) {
	int bytes;
	char buf[10] = { 0 };
	char cmd[500] = { 0 };
	sprintf(cmd, shell, name);
	FILE *stream;
	printf("%s\n", cmd);
	stream = popen(cmd, "r");
	if (!stream)
		return -1;
	bytes = fread(buf, sizeof(char), sizeof(buf), stream);
	pclose(stream);
	if (bytes > 0) {
		return atoi(buf);
	} else {
		printf("%s --> failed\n", cmd);
		return -1;
	}
}

/*0~31*/
int volume_set_volume(int volume) {
	char *name = "Master Playback Volume";
	char cmd[128] = { 0 };
	sprintf(cmd, "amixer cset name='%s' %d", name, volume);
	system(cmd);
	vol = volume;
	return SM_OK;
}

int volume_get_volume(void) {
	return vol;
}
/*0~7 */
int volume_set_mic_AMP_gain_value(int val) {
	char *name = "MIC1 boost AMP gain control";
	char cmd[100] = { 0 };
	sprintf(cmd, "amixer cset name='%s' %d", name, val);
	system(cmd);
	return SM_OK;
}
/*0~7*/
int volume_set_mic_mixer_value(int val) {
	char *name = "MIC1_G boost stage output mixer control";
	char cmd[100] = { 0 };
	sprintf(cmd, "amixer cset name='%s' %d", name, val);
	system(cmd);
	return SM_OK;
}

int volume_get_mic_AMP_gain_value(int val) {
	char *name = "MIC1 boost AMP gain control";
	const char* shell =
			"volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
	return audio_mixer_get(shell, name);
}

int volume_get_mic_mixer_value(int val) {
	char *name = "MIC1_G boost stage output mixer control";
	const char* shell =
			"volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
	return audio_mixer_get(shell, name);
}

int volume_init(void) {
	char cmd[100] = { 0 };
	char *name = "Speaker Function";
	char *name1 = "Audio speaker out";
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "amixer cset name='%s' 1", name);
	system(cmd);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "amixer cset name='%s' 1", name1);
	system(cmd);
	return SM_OK;
}
