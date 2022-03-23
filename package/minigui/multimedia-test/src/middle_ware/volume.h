#ifndef _VOLUME_LIB_H_
#define  _VOLUME_LIB_H_

int volume_set_volume(int volume);

int volume_get_volume(void);

int volume_set_mic_AMP_gain_value(int val);

int volume_set_mic_mixer_value(int val);

int volume_get_mic_AMP_gain_value(int val);

int volume_get_mic_mixer_value(int val);

int volume_init(void);

#endif
