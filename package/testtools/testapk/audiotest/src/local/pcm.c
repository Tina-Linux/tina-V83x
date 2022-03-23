#include "pcm.h"
#include "common.h"

static snd_pcm_format_t bits_to_pcm_format(
        const struct audiotest_wav_pcm_header *header)
{
    snd_pcm_format_t fmt = SND_PCM_FORMAT_UNKNOWN;
    int valid_bits;

    switch (header->bits_per_sample) {
    case 8:
        fmt = SND_PCM_FORMAT_U8;
        break;
    case 16:
        fmt = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        switch (header->block_align / header->n_channels) {
        case 3:
            fmt = SND_PCM_FORMAT_S24_3LE;
            break;
        case 4:
            fmt = SND_PCM_FORMAT_S24_LE;
            break;
        default:
            fmt = SND_PCM_FORMAT_UNKNOWN;
            break;
        }
        break;
    case 32:
        fmt = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        fmt = SND_PCM_FORMAT_UNKNOWN;
        break;
    }

    return fmt;
}

int audiotest_update_pcm_config_from_wav_header(
        const struct audiotest_wav_pcm_header *header,
        struct audiotest_pcm_config *config)
{
    int ret = 0;
    config->channels = header->n_channels;
    config->rate = header->sample_rate;
    config->bits = header->bits_per_sample;
    config->format = bits_to_pcm_format(header);
    if (config->format == SND_PCM_FORMAT_UNKNOWN) {
        audiotest_stderr("Not support WAVE files with sample %d bits\n",
                header->bits_per_sample);
        ret = -EINVAL;
        goto out;
    }
    config->frames_per_buffer = config->rate / 2;
    config->frames_per_period = config->frames_per_buffer / 4;
    ret = 0;
out:
    return ret;
}

/* return 0 on success, otherwise a negative number on error */
static int set_hw_params(
        snd_pcm_t *handle, struct audiotest_pcm_config *config)
{
    int ret = 0;
    int err;
    snd_pcm_hw_params_t *hw_params;

    snd_pcm_hw_params_alloca(&hw_params);
    err = snd_pcm_hw_params_any(handle, hw_params);
    if (err < 0) {
        audiotest_stderr("Broken configuration for PCM: %s (%s)\n",
                config->pcm_name, snd_strerror(err));
        ret = err;
        goto out;
    }

    err = snd_pcm_hw_params_set_access(
            handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        audiotest_stderr("Failed to set access type (%s)\n", snd_strerror(err));
        ret = err;
        goto out;
    }
    err = snd_pcm_hw_params_set_channels(handle, hw_params, config->channels);
    if (err < 0) {
        audiotest_stderr("Failed to set channels: %d (%s)\n", config->channels,
                snd_strerror(err));
        ret = err;
        goto out;
    }
    err = snd_pcm_hw_params_set_rate(handle, hw_params, config->rate, 0);
    if (err < 0) {
        audiotest_stderr("Failed to set rate: %d (%s)\n", config->rate,
                snd_strerror(err));
        ret = err;
        goto out;
    }
    err = snd_pcm_hw_params_set_format(handle, hw_params, config->format);
    if (err < 0) {
        audiotest_stderr("Failed to set format: %s (%s)\n",
                snd_pcm_format_name(config->format), snd_strerror(err));
        ret = err;
        goto out;
    }
    err = snd_pcm_hw_params_set_period_size_near(handle, hw_params,
            &(config->frames_per_period), 0);
    if (err < 0) {
        audiotest_stderr("Failed to set period size: %d (%s)\n",
                config->frames_per_period);
        ret = err;
        goto out;
    }
    err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params,
            &(config->frames_per_buffer));
    if (err < 0) {
        audiotest_stderr("Failed to set buffer size: %d (%s)\n",
                config->frames_per_buffer);
        ret = err;
        goto out;
    }

    err = snd_pcm_hw_params(handle, hw_params);
    if (err < 0) {
        audiotest_stderr("Failed to apply hardware parameters (%s)\n",
                snd_strerror(err));
        ret = err;
        goto out;
    }

    ret = 0;

out:
    return ret;
}

int audiotest_play_pcm_file_repeatedly(
        struct audiotest_pcm_config *config, int repeat_times, FILE *fp)
{
    int ret = 0;
    int err;

    if (repeat_times < 0) {
        audiotest_stderr("Parameter of repeated times is less than 0\n");
        ret = -EINVAL;
        goto out;
    }

    snd_pcm_t *handle;
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    err = snd_pcm_open(&handle, config->pcm_name, stream, 0);
    if (err < 0) {
        audiotest_stderr("Failed to open playback PCM (%s)\n",
                snd_strerror(err));
        ret = err;
        goto out;
    }

    err = set_hw_params(handle, config);
    if (err < 0) {
        ret = err;
        goto close_pcm;
    }

    unsigned int buf_size =
        config->bits / 8 * config->channels * config->frames_per_period;
    uint8_t *buf = (uint8_t *)malloc(buf_size);
    if (!buf) {
        audiotest_stderr("Failed to allocate %u bytes for buffer\n", buf_size);
        ret = -ENOMEM;
        goto close_pcm;
    }
    long offset = ftell(fp);
    size_t read_size;
    int repeat_cnt = 0;
    while (repeat_cnt < repeat_times) {
        read_size = fread(buf, sizeof(uint8_t), buf_size, fp);
        err = snd_pcm_writei(handle, buf, config->frames_per_period);
        if (err == -EPIPE) {
            err = snd_pcm_prepare(handle);
            if (err < 0) {
                audiotest_stderr("Failed to prepare PCM (%s)\n",
                        snd_strerror(err));
                ret = err;
                goto free_buf;
            }
        } else if (ret < 0) {
            audiotest_stderr("Error when playing in repeated times: %d\n",
                    repeat_cnt);
            ret = err;
            goto free_buf;
        }
        if (feof(fp)) {
            ++repeat_cnt;
            if (repeat_cnt >= repeat_times) {
                break;
            } else {
                fseek(fp, offset, SEEK_SET);
            }
        }
    }

    ret = 0;

free_buf:
    free(buf);
close_pcm:
    snd_pcm_close(handle);
out:
    return ret;
}
