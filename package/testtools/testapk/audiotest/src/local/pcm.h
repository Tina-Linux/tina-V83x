#ifndef __AUDIOTEST_PCM_H__
#define __AUDIOTEST_PCM_H__

#include <alsa/asoundlib.h>
#include "wav_header.h"

struct audiotest_pcm_config {
    const char *pcm_name;
    unsigned int channels;
    unsigned int rate;
    unsigned int bits;
    snd_pcm_format_t format;
    snd_pcm_uframes_t frames_per_period;
    snd_pcm_uframes_t frames_per_buffer;
};

/**
 * audiotest_update_pcm_config_from_wav_header()
 * @param header:   (in) PCM wave header
 * @param config:   (out) A pointer to the structure audiotest_pcm_config
 *
 * @return: 0 on success, otherwise on error
 */
int audiotest_update_pcm_config_from_wav_header(
        const struct audiotest_wav_pcm_header *header,
        struct audiotest_pcm_config *config);

/**
 * audiotest_play_pcm_file_repeatedly() - play a PCM file repeatedly
 * @param config:       (in) A pointer to the structure audiotest_pcm_config
 * @param repeat_times: Repeated times that the PCM file is played.
 * @param fp:           (in&out) A file pointer to the PCM file.
 *                          "fp" will update to the end of PCM file after
 *                          running this function.
 *
 * The parameter of repeated times which is less than 0 is invalid.
 *
 * @return: 0 on success, nonzero on error
 */
int audiotest_play_pcm_file_repeatedly(
        struct audiotest_pcm_config *config, int repeat_times, FILE *fp);

#endif /* ifndef __AUDIOTEST_PCM_H__ */
