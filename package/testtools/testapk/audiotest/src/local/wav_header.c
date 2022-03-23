#include "wav_header.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define PCM_FORMAT_CODE 0x0001

static const char riff_id[4] = {'R', 'I', 'F', 'F'};
static const char wave_id[4] = {'W', 'A', 'V', 'E'};
static const char fmt_id[4] = {'f', 'm', 't', ' '};
static const char data_id[4] = {'d', 'a', 't', 'a'};

int audiotest_wav_pcm_header_read(struct audiotest_wav_pcm_header *header, FILE *fp)
{
    uint8_t header_buf[AUDIOTEST_WAV_PCM_HEADER_SIZE];
    uint8_t *p = NULL;
    int i;

    if (0 != fseek(fp, 0, SEEK_SET)) {
        fprintf(stderr, "Fail to locate to the beginning"
                "of the PCM wave file\n");
        return -1;
    }

    if (AUDIOTEST_WAV_PCM_HEADER_SIZE
            != fread(header_buf, 1, AUDIOTEST_WAV_PCM_HEADER_SIZE, fp)) {
        fprintf(stderr, "Fail to read header of the PCM wave file\n");
        return -1;
    }

    p = header_buf;
    for (i = 0; i < (int)sizeof(header->riff_id); ++i)
        header->riff_id[i] = *(p + i);
    p += sizeof(header->riff_id);

    header->riff_size = little_endian_u8_to_u32(p);
    p += sizeof(header->riff_size);

    for (i = 0; i < (int)sizeof(header->riff_format); ++i)
        header->riff_format[i] = *(p + i);
    p += sizeof(header->riff_format);

    for (i = 0; i < (int)sizeof(header->fmt_id); ++i)
        header->fmt_id[i] = *(p + i);
    p += sizeof(header->fmt_id);

    header->fmt_size = little_endian_u8_to_u32(p);
    p += sizeof(header->fmt_size);

    header->audio_format = little_endian_u8_to_u16(p);
    p += sizeof(header->audio_format);

    header->n_channels = little_endian_u8_to_u16(p);
    p += sizeof(header->n_channels);

    header->sample_rate = little_endian_u8_to_u32(p);
    p += sizeof(header->sample_rate);

    header->byte_rate = little_endian_u8_to_u32(p);
    p += sizeof(header->byte_rate);

    header->block_align = little_endian_u8_to_u16(p);
    p += sizeof(header->block_align);

    header->bits_per_sample = little_endian_u8_to_u16(p);
    p += sizeof(header->bits_per_sample);

    for (i = 0; i < (int)sizeof(header->data_id); ++i)
        header->data_id[i] = *(p + i);
    p += sizeof(header->data_id);

    header->data_size = little_endian_u8_to_u32(p);

    if (memcmp(header->riff_id, riff_id, 4)
            || memcmp(header->riff_format, wave_id, 4)
            || memcmp(header->fmt_id, fmt_id, 4)
            || memcmp(header->data_id, data_id, 4)) {
        fprintf(stderr, "Error: This is not a PCM wave file\n");
        return -2;
    }

    return 0;
}

int audiotest_wav_pcm_header_write(uint16_t n_channels, uint32_t sample_rate,
                         uint16_t bits_per_sample, uint16_t block_align,
                         uint32_t frames, FILE *fp)
{
    uint8_t header_buf[AUDIOTEST_WAV_PCM_HEADER_SIZE];
    uint8_t *p = header_buf;
    int size;

    uint16_t audio_format = PCM_FORMAT_CODE;
    uint32_t byte_rate = (uint32_t)(block_align) * sample_rate;
    uint32_t fmt_size = 16;
    uint32_t data_size = (uint32_t)(block_align) * frames;
    uint32_t riff_size = data_size + 36;

    size = (int)sizeof(((struct audiotest_wav_pcm_header *)0)->riff_id);
    memcpy(p, riff_id, size);
    p += size;

    u32_to_little_endian_u8(riff_size, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->riff_size);

    size = (int)sizeof(((struct audiotest_wav_pcm_header *)0)->riff_format);
    memcpy(p, wave_id, size);
    p += size;

    size = (int)sizeof(((struct audiotest_wav_pcm_header *)0)->fmt_id);
    memcpy(p, fmt_id, size);
    p += size;

    u32_to_little_endian_u8(fmt_size, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->fmt_size);

    u16_to_little_endian_u8(audio_format, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->audio_format);

    u16_to_little_endian_u8(n_channels, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->n_channels);

    u32_to_little_endian_u8(sample_rate, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->sample_rate);

    u32_to_little_endian_u8(byte_rate, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->byte_rate);

    u16_to_little_endian_u8(block_align, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->block_align);

    u16_to_little_endian_u8(bits_per_sample, p);
    p += sizeof(((struct audiotest_wav_pcm_header *)0)->bits_per_sample);

    size = sizeof(((struct audiotest_wav_pcm_header *)0)->data_id);
    memcpy(p, data_id, size);
    p += size;

    u32_to_little_endian_u8(data_size, p);

    if (0 != fseek(fp, 0 , SEEK_SET)) {
        fprintf(stderr, "Fail to locate to the beginning"
                "of the PCM wave file\n");
        return -1;
    }

    if (AUDIOTEST_WAV_PCM_HEADER_SIZE
            != fwrite(header_buf, 1, AUDIOTEST_WAV_PCM_HEADER_SIZE, fp)) {
        fprintf(stderr, "Fail to write header of the PCM wave file\n");
        return -1;
    }

    return 0;
}

void audiotest_wav_pcm_header_print(
        const struct audiotest_wav_pcm_header *header)
{
    int i;
    printf("RIFF chunk ID: ");
    for (i = 0; i < (int)sizeof(header->riff_id); ++i)
        printf("%c", header->riff_id[i]);
    printf("\n");

    printf("RIFF chunk size: %u bytes\n", header->riff_size);

    printf("RIFF format: ");
    for (i = 0; i < (int)sizeof(header->riff_format); ++i) {
        printf("%c", header->riff_format[i]);
    }
    printf("\n");

    printf("fmt chunk ID: ");
    for (i = 0; i < (int)sizeof(header->fmt_id); ++i) {
        printf("%c", header->fmt_id[i]);
    }
    printf("\n");

    printf("audio format: ");
    switch (header->audio_format) {
    case PCM_FORMAT_CODE:
        printf("PCM");
        break;
    default:
        printf("non-PCM");
        break;
    }
    printf("\n");

    printf("number of channels: %d\n", header->n_channels);

    printf("sample rate: %u\n", header->sample_rate);

    printf("byte rate: %u\n", header->byte_rate);

    printf("block align: %d\n", header->block_align);

    printf("bits per sample: %d\n", header->bits_per_sample);

    printf("data chunk ID: ");
    for (i = 0; i < (int)sizeof(header->data_id); ++i) {
        printf("%c", header->data_id[i]);
    }
    printf("\n");

    printf("data size: %u bytes\n", header->data_size);
}

