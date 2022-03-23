#ifndef __VOL_CONTROL_H__
#define __VOL_CONTROL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#define SPEAKER_VOL_CTL (51)    /*Master Playback Volume*/
//#define VOL_CFG_FILE		"/data/cfg/volume_config"
//add by 926 @20190605
#define SPEAKER_VOL_CTL (26)    /*Master Playback Volume*/
#define VOL_CFG_FILE	"/mnt/UDISK/cfg/volume_config"

//Init by midea_initVolume and get info from config.json
struct m_volume_curve{
    int curve[128]; //Support less than 128 degrees volume curve
    int vol_degrees;    //Should be less than 128.
    int default_volume;
    int min_volume;
    int max_volume;
};

enum{
   SPEAKER_VOLUME_ONE_LEVEL,
   SPEAKER_VOLUME_TWO_LEVEL,
   SPEAKER_VOLUME_THREE_LEVEL,
   SPEAKER_VOLUME_FOUR_LEVEL,
   SPEAKER_VOLUME_FIVE_LEVEL
};

#define SPEAKER_VOLUME_DEFAULT_LEVEL SPEAKER_VOLUME_FOUR_LEVEL
#define SPEAKER_VOLUME_MAX_LEVEL SPEAKER_VOLUME_FIVE_LEVEL
#define SPEAKER_VOLUME_MIN_LEVEL SPEAKER_VOLUME_ONE_LEVEL

//Mute state is only saved in user struct item "is_mute"
int midea_setMicMute(void *user,bool mute);     //Success return 0 ; failed return -1.
int midea_getMicMute(void *user);   //Success return mute state ; failed return -1.

//Recommend
int midea_setSpeakerMute(bool mute);    //Success return 0 ; failed return -1.
int midea_getSpeakerlevel(void);    //Return level.
int midea_setSpeakerLevel(bool isadd, int value);   //Success return mute state ; failed return -1.

//Not recommend
int midea_setSpeakerVol(bool isadd, int value);     //Success return 0 ; failed return -1.
int midea_getSpeakerVol(void);  //Success return volume value ; failed return -1.
int midea_toSpeakerVol(int value);  //Mspeech internal interface, do not call in demo.
int midea_get_silent(void *user);
int midea_set_silent(void *user, bool silent);

//NOTE: Already init in maspeech, do not call again in demo.
int midea_initVolume(const char *config);
int midea_getVolumeCurve(struct m_volume_curve *vc);
int midea_releaseVolume(void);

#endif
