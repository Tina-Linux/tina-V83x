#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static unsigned int vol;

struct amixer_control {
    char *name;
    unsigned int value;
};

struct amixer_control controls[] =
{
#ifdef CONFIG_R40
    /*r40*/
    {"Left Output Mixer DACL Switch", 1},
    {"Right Output Mixer DACR Switch", 1},
    {"HPL Mux", 1},
    {"HPR Mux", 1},
    {"Headphone Switch", 1}
#endif

#ifdef CONFIG_R16
    /*r16*/
    {"AIF1IN0L Mux", 0},
    {"AIF1IN0R Mux", 0},
    {"DACL Mixer AIF1DA0L Switch", 1},
    {"DACR Mixer AIF1DA0R Switch", 1},
    {"Left Output Mixer DACL Switch", 1},
    {"Right Output Mixer DACR Switch", 1},
    {"HP_L Mux", 1},
    {"HP_R Mux", 1},
    {"Headphone Switch", 1}
#endif

#ifdef CONFIG_R6
    /*r6*/
    {"head phone power", 1},
    {"ADC MIC Boost AMP en", 1},
    {"ADC mixer mute for mic", 1},
#endif

#ifdef CONFIG_R11
    /*r11*/
    {"Speaker Function", 1},
    {"Audio speaker out", 1}
#endif

#ifdef CONFIG_R7
    /*r7*/
    {"Speaker Function", 1},
    {"Audio speaker out", 1}
#endif
};

static int audio_mixer_cset(char *mixer_name,int value)
{
    char cmd[128] = {0};
    sprintf(cmd,"amixer cset name='%s' %d",mixer_name,value);
    system(cmd);
    return TINAL_OK;
}

int audio_mixer_get(const char* shell,const char* name)
{
    int bytes;
    char buf[10] = {0};
    char cmd[500] = {0};
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

/*0~31*/
int volume_set_volume(int volume)
{
#ifdef CONFIG_R11
    char *name = "Master Playback Volume";/*r11*/
#endif

#ifdef CONFIG_R7
    char *name = "Master Playback Volume";/*r7*/
#endif

#ifdef CONFIG_R6
    char *name = "head phone volume"; /*r6*/
#endif

#ifdef CONFIG_R40
    char *name = "Headphone volume"; /*r40*/
#endif

#ifdef CONFIG_R16
    char *name = "headphone volume"; /*r16*/
#endif
    audio_mixer_cset(name,volume);
	vol = volume;
	return TINAL_OK;
}

int volume_get_volume(void)
{
	return vol;
}
/*0~7 */
int volume_set_mic_AMP_gain_value(int val)
{
#ifdef CONFIG_R11
    char *name = "MIC1 boost AMP gain control";/*r11*/
#endif

#ifdef CONFIG_R7
    char *name = "MIC1 boost AMP gain control";/*r7*/
#endif

#ifdef CONFIG_R16
    char *name = "MIC1 boost amplifier gain";/*r16*/
#endif

#ifdef CONFIG_R40
    char *name = "MIC1 boost volume";/*r40*/
#endif

#ifdef CONFIG_R6
    char *name = "MICIN GAIN control";/*r6*/
#endif

    audio_mixer_cset(name,val);
	return TINAL_OK;
}
/*0~7*/
int volume_set_mic_mixer_value(int val)
{
	char *name = "MIC1_G boost stage output mixer control";
    char cmd[100] = {0};

	audio_mixer_cset(name,val);
	return  TINAL_OK;
}

int volume_get_mic_AMP_gain_value(int val)
{
#ifdef CONFIG_R11
    char *name = "MIC1 boost AMP gain control";/*r11*/
#endif

#ifdef CONFIG_R7
    char *name = "MIC1 boost AMP gain control";/*r7*/
#endif

#ifdef CONFIG_R16
    char *name = "MIC1 boost amplifier gain";/*r16*/
#endif

#ifdef CONFIG_R40
    char *name = "MIC1 boost volume";/*r40*/
#endif

#ifdef CONFIG_R6
    char *name = "MICIN GAIN control";/*r6*/
#endif

    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return audio_mixer_get(shell,name);
}

int volume_get_mic_mixer_value(int val)
{
	char *name = "MIC1_G boost stage output mixer control";
    const char* shell = "volume=`amixer cget name='%s' | grep ': values='`;volume=${volume#*=};echo $volume";
    return audio_mixer_get(shell,name);
}

int volume_init(void)
{
    int i = 0;
    int num = sizeof(controls)/sizeof(controls[0]);
    for(i = 0;i < num;i++)
    {
        audio_mixer_cset(controls[i].name,controls[i].value);
    }

    return  TINAL_OK;
}
