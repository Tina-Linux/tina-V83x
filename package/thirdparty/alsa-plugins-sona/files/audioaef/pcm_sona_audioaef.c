#include "config.h"
#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>

#include "audioaef_ext.h"

typedef struct {
    char* config_file;
    int max_frames;
    int runtime_config;
    int save_runtime_config;
} sona_audioaef_params_t;

typedef struct {
    snd_pcm_extplug_t ext;
    sona_audioaef_params_t params;
    void *instance;
    void *rpc_handle;
} snd_pcm_sona_audioaef_t;

static int save_config_callback(char *file, void *private_data)
{
    snd_pcm_sona_audioaef_t *audioaef = (snd_pcm_sona_audioaef_t *)private_data;
    audioaef_save_config(audioaef->instance, audioaef->params.config_file);
    return 0;
}

static inline void *area_addr(const snd_pcm_channel_area_t *area,
        snd_pcm_uframes_t offset)
{
    unsigned int bitofs = area->first + area->step * offset;
    return (char *) area->addr + bitofs / 8;
}

static snd_pcm_sframes_t sona_audioaef_transfer(snd_pcm_extplug_t *ext,
        const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset,
        const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset,
        snd_pcm_uframes_t size)
{
    snd_pcm_sona_audioaef_t *audioaef = (snd_pcm_sona_audioaef_t *)ext->private_data;

    char *src = (char *)area_addr(src_areas, src_offset);
    char *dst = (char *)area_addr(dst_areas, dst_offset);

    int in_channels = ext->channels;
    int out_channls = ext->slave_channels;
    int in_bits = snd_pcm_format_physical_width(ext->format);

    int residue_bytes = in_bits / 8 * in_channels * size;
    int process_bytes_max = in_bits / 8 * audioaef->params.max_frames;
    int process_bytes = 0;

    while (residue_bytes > 0) {
        process_bytes = process_bytes_max < residue_bytes ?
            process_bytes_max : residue_bytes;
        int ret = audioaef_process(audioaef->instance,
                src, process_bytes, in_channels, in_bits, dst, out_channls);
        if (ret < 0) {
            SNDERR("Error in audioaef_process. Maybe max_frames not large enough? "
                    "(max_frames: %d, process_bytes: %d)",
                    audioaef->params.max_frames, process_bytes);
            return ret;
        }
        src += process_bytes;
        dst += process_bytes;
        residue_bytes -= process_bytes;
    }

    return size;
}

static int sona_audioaef_hw_params(snd_pcm_extplug_t *ext,
        snd_pcm_hw_params_t *params)
{
    snd_pcm_sona_audioaef_t *audioaef = (snd_pcm_sona_audioaef_t *)ext->private_data;

    if (audioaef->instance) {
        return 0;
    }

    if (audioaef->params.max_frames == 0) {
        snd_pcm_uframes_t period_size = 0;
        if (0 != snd_pcm_hw_params_get_period_size(params, &period_size, 0)) {
            SNDERR("Fail to get period size");
        }
        audioaef->params.max_frames = (int)period_size;
    }

    audioaef->instance = audioaef_create(ext->rate, ext->rate,
            audioaef->params.config_file,
            snd_pcm_format_physical_width(ext->slave_format),
            audioaef->params.max_frames);
    if (!audioaef->instance) {
        SNDERR("Fail to create audioaef instance");
        return -1;
    }

    if (audioaef->params.runtime_config) {
        audioaef->rpc_handle = audioRPC_init(audioaef->instance);
        if (!audioaef->rpc_handle) {
            SNDERR("Fail to init audioaef RPC handle");
            return -1;
        }
        if (audioaef->params.save_runtime_config) {
            audioaef_set_saveFileCb(audioaef->instance,
                    save_config_callback, (void *)audioaef);
        }
    }

    return 0;
}

static int sona_audioaef_hw_free(snd_pcm_extplug_t *ext)
{
    snd_pcm_sona_audioaef_t *audioaef = (snd_pcm_sona_audioaef_t *)ext->private_data;

    if (!audioaef) {
        return 0;
    }
    if (audioaef->rpc_handle) {
        audioRPC_close(audioaef->rpc_handle);
        audioaef->rpc_handle = NULL;
    }
    if (audioaef->instance) {
        audioaef_destroy(audioaef->instance);
        audioaef->instance = NULL;
    }
    return 0;
}

static int sona_audioaef_close(snd_pcm_extplug_t *ext)
{
    snd_pcm_sona_audioaef_t *audioaef = (snd_pcm_sona_audioaef_t *)ext->private_data;

    if (audioaef->rpc_handle) {
        audioRPC_close(audioaef->rpc_handle);
        audioaef->rpc_handle = NULL;
    }
    if (audioaef->instance) {
        audioaef_destroy(audioaef->instance);
        audioaef->instance = NULL;
    }
    if (audioaef->params.config_file) {
        free(audioaef->params.config_file);
        audioaef->params.config_file = NULL;
    }
    free(audioaef);
    return 0;
}

static const snd_pcm_extplug_callback_t sona_audioaef_callback = {
    .transfer = sona_audioaef_transfer,
    .hw_params = sona_audioaef_hw_params,
    .hw_free = sona_audioaef_hw_free,
    .close = sona_audioaef_close,
};

static int get_int_parm(snd_config_t *n, const char *id, const char *str,
        int *value_ret)
{
    long value;
    int err;

    if (strcmp(id, str) != 0) {
        return 1;
    }
    err = snd_config_get_integer(n, &value);
    if (err < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return err;
    }
    *value_ret = value;
    return 0;
}

static int get_string_param(snd_config_t *n, const char *id, const char *str,
        const char **value_ret)
{
    const char *value = NULL;
    int err;

    if (strcmp(id, str) != 0) {
        return 1;
    }
    err = snd_config_get_string(n, &value);
    if (err < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return err;
    }
    *value_ret = value;
    return 0;
}

static int get_bool_parm(snd_config_t *n, const char *id, const char *str,
        int *value_ret)
{
    int value;
    if (strcmp(id, str) != 0) {
        return 1;
    }

    value = snd_config_get_bool(n);
    if (value < 0) {
        SNDERR("Invalid value for %s parameter", id);
        return value;
    }
    *value_ret = value;
    return 0;
}

SND_PCM_PLUGIN_DEFINE_FUNC(sona_audioaef)
{
    snd_config_iterator_t i, next;
    snd_config_t *slave_conf = NULL;
    snd_pcm_sona_audioaef_t *audioaef = NULL;
    sona_audioaef_params_t params = {
        .config_file = NULL,
        .max_frames = 0,
        .runtime_config = 0,
        .save_runtime_config = 0,
    };

    int err = 0;
    const char *config_file = NULL;

    snd_config_for_each(i, next, conf) {
        snd_config_t *n = snd_config_iterator_entry(i);
        const char *id;
        if (snd_config_get_id(n, &id) < 0) {
            continue;
        }
        if (strcmp(id, "comment") == 0
                || strcmp(id, "type") == 0
                || strcmp(id, "hint") == 0) {
            continue;
        }
        if (strcmp(id, "slave") == 0) {
            slave_conf = n;
            continue;
        }
        err = get_string_param(n, id, "config_file", &config_file);
        if (err <= 0) {
            goto ok;
        }
        err = get_int_parm(n, id, "max_frames", &params.max_frames);
        if (err <= 0) {
            goto ok;
        }
        err = get_bool_parm(n, id, "runtime_config", &params.runtime_config);
        if (err <= 0) {
            goto ok;
        }
        err = get_bool_parm(n, id, "save_runtime_config", &params.save_runtime_config);
        if (err <= 0) {
            goto ok;
        }
        SNDERR("Unknown field %s", id);
        err = -EINVAL;
    ok:
        if (err < 0) {
            goto error;
        }
    }

    if (!slave_conf) {
        SNDERR("No slave defined for sona_audioaef");
        err = -EINVAL;
        goto error;
    }

    if (!config_file) {
        config_file = "/etc/sona_audioaef.conf";
    }
    params.config_file = (char *)malloc(strlen(config_file) + 1);
    if (!params.config_file) {
        err = -ENOMEM;
        goto error;
    }
    strcpy(params.config_file, config_file);

    audioaef = calloc(1, sizeof(*audioaef));
    if (!audioaef) {
        err = -ENOMEM;
        goto error;
    }

    audioaef->ext.version = SND_PCM_EXTPLUG_VERSION;
    audioaef->ext.name = "Sona Audio Effect Plugin";
    audioaef->ext.callback = &sona_audioaef_callback;
    audioaef->ext.private_data = audioaef;
    audioaef->params = params;
    audioaef->instance = NULL;
    audioaef->rpc_handle = NULL;

    err = snd_pcm_extplug_create(&audioaef->ext, name, root, slave_conf, stream, mode);
    if (err < 0) {
        goto error;
    }

    snd_pcm_extplug_set_param(&audioaef->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 1);
    snd_pcm_extplug_set_slave_param(&audioaef->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 1);
    snd_pcm_extplug_set_param(&audioaef->ext, SND_PCM_EXTPLUG_HW_FORMAT,
            SND_PCM_FORMAT_S16_LE);
    snd_pcm_extplug_set_slave_param(&audioaef->ext, SND_PCM_EXTPLUG_HW_FORMAT,
            SND_PCM_FORMAT_S16_LE);

    *pcmp = audioaef->ext.pcm;
    return 0;

error:
    if (params.config_file) {
        free(params.config_file);
    }
    if (audioaef) {
        free(audioaef);
    }
    return err;
}

SND_PCM_PLUGIN_SYMBOL(sona_audioaef)
