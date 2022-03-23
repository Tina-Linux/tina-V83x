#ifndef __AUDIOTEST_WAV_HEADER_H__
#define __AUDIOTEST_WAV_HEADER_H__

#include <stdint.h>
#include <stdio.h>

/* size of the header of a PCM wave file */
#define AUDIOTEST_WAV_PCM_HEADER_SIZE 44

/* header of a PCM wave file */
struct audiotest_wav_pcm_header {
    char riff_id[4];
    uint32_t riff_size;
    char riff_format[4];
    char fmt_id[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t n_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_id[4];
    uint32_t data_size;
};


/**
 * audiotest_wav_pcm_header_read() - read header of a PCM wave file
 * @param header:   (out) A pointer to the structure of PCM wave header.
 * @param fp:       (in&out) A FILE pointer to the the wave file (not
 *                      necessarily need to be the begining of the wave file).
 *                      "fp" will update to the position after the end of PCM
 *                      wave header after running this function.
 * @return: 0 on success, nonzero on error
 */
int audiotest_wav_pcm_header_read(struct audiotest_wav_pcm_header *header,
                                  FILE *fp);

/**
 * audiotest_wav_pcm_header_write() - write header to a PCM wave file
 * @param n_channels:   Number of channels of the PCM wave file
 * @param sample_rate:  Sampling rate of the PCM wave file (blocks per second)
 * @param bits_per_sample:  Number of bits that one sample has
 * @param block_align:  Numeber of bytes for one sample including all channels
 * @param frames:   Number of frames that the PCM wave file has
 *                      (size of 1 frame = bits_per_sample / 8 * n_channels)
 * @param fp:       (in&out) A FILE pointer to the wave file (not
 *                      necessarily need to be the begining of the wave file).
 *                      "fp" will update to the position after the end of PCM
 *                      wave header after running this function.
 *
 * This function will overwrite several bytes of data (determined by the size
 * of PCM wave header) in the beginning of the PCM wave file.
 *
 * @return: 0 on success, nonzero on error
 */
int audiotest_wav_pcm_header_write(uint16_t n_channels, uint32_t sample_rate,
                                   uint16_t bits_per_sample, uint16_t block_align,
                                   uint32_t frames, FILE *fp);

/**
 * audiotest_wav_pcm_header_print() - print the header of the PCM wave file
 * @param header:    (in) A pointer to the structure of PCM wave header.
 *
 * @return: void
 */
void audiotest_wav_pcm_header_print(
        const struct audiotest_wav_pcm_header *header);

#endif /* ifndef __AUDIOTEST_WAV_HEADER_H__ */
