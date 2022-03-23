#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int audio_mixer_get(const char* shell,const char* name)
{
    int bytes;
    char buf[10];

    char cmd[500];
    sprintf(cmd,shell,name);
    FILE   *stream;
    printf("%s\n",cmd);
    stream = popen( cmd, "r" );
    if(!stream)
		return -1;
    bytes = fread( buf, sizeof(char), sizeof(buf), stream);
    pclose(stream);
    if(bytes > 0){
            return atoi(buf);
    }else {
            printf("%s --> failed\n",cmd);
            return -1;
    }
}


/*0~63*/
int volume_set_volume(int volume)
{
	char *name = "head phone volume";
    char cmd[100];
    sprintf(cmd,"amixer cset name='%s' %d",name,volume);
	system(cmd);
	return SM_OK;
}

int volume_get_volume(void)
{
	char *name = "head phone volume";
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return audio_mixer_get(shell,name);
}
/*0~7 */
int volume_set_mic_AMP_gain_value(int val)
{
	char *name = "MIC1 boost AMP gain control";
    char cmd[100];
    sprintf(cmd,"amixer cset name='%s' %d",name,val);
	system(cmd);
	return  SM_OK;
}
/*0~7*/
int volume_set_mic_mixer_value(int val)
{
	char *name = "MIC1_G boost stage output mixer control";
    char cmd[100];
    sprintf(cmd,"amixer cset name='%s' %d",name,val);
	system(cmd);
	return  SM_OK;
}

int volume_get_mic_AMP_gain_value(int val)
{
	char *name = "MIC1 boost AMP gain control";
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return audio_mixer_get(shell,name);
}

int volume_get_mic_mixer_value(int val)
{
	char *name = "MIC1 boost AMP gain control";
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return audio_mixer_get(shell,name);
}

int volume_init(void)
{
	char cmd[100];
	char *name = "Speaker Function";
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"amixer cset name='%s' 2",name);
	system(cmd);
	return  SM_OK;
}

