#ifndef __ALSA_UCM_AW_H__
#define __ALSA_UCM_AW_H__

/***********************************************************************
 * Common APIs
 ***********************************************************************/
/**
 * Run the EnableSequence/DisableSequence of a use case
 * @param card_name:    sound card name
 * @param verb_name:    verb name
 * @param dev_name:     device name
 * @param mod_name:     modifier name
 * @return: 0 on success, otherwise on error
 */
int aua_use_case_enable(const char *card_name, const char *verb_name,
                        const char *dev_name, const char *mod_name);
int aua_use_case_disable(const char *card_name, const char *verb_name,
                         const char *dev_name, const char *mod_name);
/**
 * Get the "Value" defined in UCM configurations
 * @param value_name:       value name (e.g. PlaybackPCM, PlaybackVolume, CapturePCM, etc.)
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @return: the string of "Value" on success; NULL on errer
 *
 * Note:
 *      The returned string is dynamically allocated. It needs be deallocated
 *      with free().
 */
char *aua_value_get(const char *value_name, const char *card_name,
                    const char *verb_name, const char *dev_or_mod_name);

/**
 * Get the volume according to the ALSA control defined in "PlaybackVolume" /
 * "CaptureVolume" in UCM configurations
 * @param card_name:        (in) sound card name
 * @param verb_name:        (in) verb name
 * @param dev_or_mod_name:  (in) device or modifier name
 * @param volume:           (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_get(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int *volume);
int aua_capture_volume_get(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int *volume);
/**
 * Set the volume according to the ALSA control defined in "PlaybackVolume" /
 * "CaptureVolume" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @param volume:           the volume value to set
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_set(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int volume);
int aua_capture_volume_set(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int volume);
/**
 * Set the volume according to the ALSA control and its default value defined in
 * "PlaybackVolume"/"CaptureVolume" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_set_default(const char *card_name, const char *verb_name,
                                    const char *dev_or_mod_name);
int aua_capture_volume_set_default(const char *card_name, const char *verb_name,
                                   const char *dev_or_mod_name);
/**
 * Get the min/max volume according to "PlaybackVolumeMin"/"PlaybackVolumeMax"/
 * "CaptureVolumeMin"/"CaptureVolumeMax" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @param volume:           (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_playback_volume_min_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume);
int aua_playback_volume_max_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume);
int aua_capture_volume_min_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume);
int aua_capture_volume_max_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume);


/************************************************************************
 * Default use case APIs
 *
 *  The parameters are similar to that in common APIs, except that here the
 *  card_name, verb_name, dev_name, mod_name can be NULL, which will make it
 *  find the default names in UCM configurations
 ***********************************************************************/
/**
 * Run the EnableSequence/DisableSequence of the default play/record use case
 * @return: 0 on success, otherwise on error
 */
int aua_default_play_enable(const char *card_name, const char *verb_name,
                            const char *dev_name, const char *mod_name);
int aua_default_play_disable(const char *card_name, const char *verb_name,
                             const char *dev_name, const char *mod_name);
int aua_default_record_enable(const char *card_name, const char *verb_name,
                              const char *dev_name, const char *mod_name);
int aua_default_record_disable(const char *card_name, const char *verb_name,
                               const char *dev_name, const char *mod_name);
/**
 * Get the volume of the default play/record use case according to the ALSA
 * control defined in "PlaybackVolume"/"CaptureVolume" in UCM configurations
 * @param volume:   (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_default_play_volume_get(const char *card_name, const char *verb_name,
                                const char *dev_name, const char *mod_name,
                                int *volume);
int aua_default_record_volume_get(const char *card_name, const char *verb_name,
                                  const char *dev_name, const char *mod_name,
                                  int *volume);
/**
 * Set the volume of the default play/record use case according to the ALSA
 * control defined in "PlaybackVolume"/"CaptureVolume" in UCM configurations
 * @param volume:   the volume value to set
 * @return: 0 on success, otherwise on error
 */
int aua_default_play_volume_set(const char *card_name, const char *verb_name,
                                const char *dev_name, const char *mod_name,
                                int volume);
int aua_default_record_volume_set(const char *card_name, const char *verb_name,
                                  const char *dev_name, const char *mod_name,
                                  int volume);
/**
 * Set the volume of the default play/record use case according to the ALSA
 * control and its default value defined in "PlaybackVolume"/"CaptureVolume" in
 * UCM configurations
 * @return: 0 on success, otherwise on error
 */
int aua_default_play_volume_set_default(const char *card_name,
                                        const char *verb_name,
                                        const char *dev_name,
                                        const char *mod_name);
int aua_default_record_volume_set_default(const char *card_name,
                                          const char *verb_name,
                                          const char *dev_name,
                                          const char *mod_name);

/**
 * Get the min/max volume according to "PlaybackVolumeMin"/"PlaybackVolumeMax"/
 * "CaptureVolumeMin"/"CaptureVolumeMax" in UCM configurations
 * @param card_name:        sound card name
 * @param verb_name:        verb name
 * @param dev_or_mod_name:  device or modifier name
 * @param volume:           (out) the volume value got
 * @return: 0 on success, otherwise on error
 */
int aua_default_play_volume_min_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume);
int aua_default_play_volume_max_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume);
int aua_default_record_volume_min_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume);
int aua_default_record_volume_max_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume);

/**
 * Run the "SectionDefaults" in UCM configurations
 * NOTE:
 *      These functions are **DEPRECATED**. If the sequence of "SectionDefaults"
 *      in UCM configurations is too long, it will cause memory overflow at
 *      runtime.
 */
#define ENABLE_RESET_COMMAND 0
#if ENABLE_RESET_COMMAND
int aua_use_case_reset(const char *card_name);
int aua_default_play_reset(const char *card_name);
int aua_default_record_reset(const char *card_name);
#endif /* if ENABLE_RESET_COMMAND */

#endif /* ifndef __ALSA_UCM_AW_H__ */
