#ifndef __ALSA_UCM_AW_CONTROL_H__
#define __ALSA_UCM_AW_CONTROL_H__

/**
 * control_value_get() - Get the value of an ALSA control
 * @param card:     (in) ALSA virtual card name (e.g. "hw:audiocodec")
 * @param ctl_id:   (in) ALSA control identifier (e.g. "numid=xxx", "name='xxx'", etc.)
 * @return: the control value on success, otherwise NULL on error
 *          (The returned string is dynamically allocated, needed to be
 *          deallocated by free())
 */
char *control_value_get(const char *card, const char *ctl_id);

/**
 * control_value_set() - Set the value of an ALSA control
 * @param card:         (in) ALSA virtual card name (e.g. "hw:audiocodec")
 * @param ctl_id:       (in) ALSA control identifier (e.g. "numid=xxx", "name='xxx'", etc.)
 * @param ctl_value:    (in) The control value
 * @return: 0 on success, otherwise on error
 */
int control_value_set(const char *card, const char *ctl_id, const char *ctl_value);

#endif /* ifndef __ALSA_UCM_AW_CONTROL_H__ */
