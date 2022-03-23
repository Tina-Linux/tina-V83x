#include <string.h>
#include <alsa/asoundlib.h>
#include <alsa/use-case.h>
#include "common.h"
#include "control.h"
#include "value.h"
#include "alsa-ucm-aw.h"

struct control_volume {
    char *id;
    char *value;
};

enum volume_range {
    VOL_MIN,
    VOL_MAX
};

/* UCM value names */
static const char card_value_name[] = "Card";
static const char playback_volume_value_name[] = "PlaybackVolume";
static const char playback_volume_min_value_name[] = "PlaybackVolumeMin";
static const char playback_volume_max_value_name[] = "PlaybackVolumeMax";
static const char capture_volume_value_name[] = "CaptureVolume";
static const char capture_volume_min_value_name[] = "CaptureVolumeMin";
static const char capture_volume_max_value_name[] = "CaptureVolumeMax";

/**
 * The string of UCM volume has both the control identifier and the default
 * value, such as "name='Headphone Volume' 35". We need to separate them into
 * identifier ("name='Headphone Volume'") and value ("35").
 *
 * The returned structure and its members are dynamically allocated,
 * needed to be deallocated by using the function free_ctl_vol()
 */
static struct control_volume *get_ctl_vol_from_ucm_vol(const char *ucm_vol)
{
    const char *p;
    struct control_volume *ctl_vol = NULL;
    int single_quote_cnt = 0;
    int id_len = 0;     /* not inluding the size of rear '\0' */
    int value_len = 0;  /* not including the size of rear '\0' */

    if (ucm_vol == NULL) {
        ctl_vol = NULL;
        goto out;
    }
    for (p = ucm_vol; *p != '\0'; ++p) {
        if (*p == '\'') {
            ++single_quote_cnt;
            if (single_quote_cnt >= 2) {
                break;
            }
        }
    }

    ctl_vol = (struct control_volume *)malloc(sizeof(struct control_volume));
    if (ctl_vol == NULL) {
        goto out;
    }

    id_len = p - ucm_vol + 1;
    ctl_vol->id = (char *)malloc(id_len + 1);
    if (ctl_vol->id == NULL) {
        goto free_ctl_vol;
    }
    strncpy(ctl_vol->id, ucm_vol, id_len);
    ctl_vol->id[id_len] = '\0';

    do {
        ++p;
        if (*p != ' ') {
            break;
        }
    } while (*p != '\0');
    value_len = strlen(ucm_vol) - (p - ucm_vol);
    ctl_vol->value = (char *)malloc(value_len + 1);
    if (ctl_vol->value == NULL) {
        goto free_id;
    }
    strncpy(ctl_vol->value, p, value_len);
    ctl_vol->value[value_len] = '\0';

    goto out;

free_id:
    free(ctl_vol->id);
free_ctl_vol:
    free(ctl_vol);
    ctl_vol = NULL;
out:
    return ctl_vol;
}

static void free_ctl_vol(struct control_volume *ctl_vol)
{
    free(ctl_vol->id);
    free(ctl_vol->value);
    free(ctl_vol);
}

static int volume_get(const char *ucm_value_name, const char *card_name,
        const char *verb_name, const char *dev_or_mod_name, int *volume)
{
    int ret = 0;
    char *card = NULL;
    char *ucm_vol_str = NULL;
    struct control_volume *ctl_vol = NULL;
    char *ctl_value = NULL;

    card = aua_value_get(card_value_name, card_name, verb_name, dev_or_mod_name);
    if (card == NULL) {
        aua_stderr("Failed to get ALSA virtual card name of card: %s, "
                "verb: %s, device: %s\n", card_name, verb_name, dev_or_mod_name);
        ret = -1;
        goto out;
    }

    ucm_vol_str = aua_value_get(ucm_value_name, card_name, verb_name, dev_or_mod_name);
    if (ucm_vol_str == NULL) {
        aua_stderr("Failed to get the \"%s\" value of card: %s, verb: %s, "
                "device: %s\n", ucm_value_name, card_name, verb_name, dev_or_mod_name);
        ret = -1;
        goto free_card;
    }

    ctl_vol = get_ctl_vol_from_ucm_vol(ucm_vol_str);
    if (ctl_vol == NULL) {
        aua_stderr("Failed to get the control identifier and value from %s\n",
                ucm_vol_str);
        ret = -1;
        goto free_ucm_vol_str;
    }

    ctl_value = control_value_get(card, ctl_vol->id);
    if (ctl_value == NULL) {
        aua_stderr("Failed to get the value of \"%s\" in card %s\n",
                ctl_vol->id, card);
        ret = -1;
        goto free_ctl_vol;
    }

    *volume = atoi(ctl_value);

    free(ctl_value);
    ret = 0;

free_ctl_vol:
    free_ctl_vol(ctl_vol);
free_ucm_vol_str:
    free(ucm_vol_str);
free_card:
    free(card);
out:
    return ret;
}

static int volume_set(const char *ucm_value_name, const char *card_name,
        const char *verb_name, const char *dev_or_mod_name, int volume,
        int is_set_default_value)
{
    int ret = 0;
    int err = 0;
    char *card = NULL;
    char *ucm_vol_str = NULL;
    struct control_volume *ctl_vol = NULL;
    char ctl_vol_value[AUA_STRING_MAX_LEN] = "";

    card = aua_value_get(card_value_name, card_name, verb_name, dev_or_mod_name);
    if (card == NULL) {
        aua_stderr("Failed to get ALSA virtual card name of card: %s, "
                "verb: %s, device: %s\n", card_name, verb_name, dev_or_mod_name);
        ret = -1;
        goto out;
    }

    ucm_vol_str = aua_value_get(ucm_value_name, card_name, verb_name, dev_or_mod_name);
    if (ucm_vol_str == NULL) {
        aua_stderr("Failed to get the \"%s\" value of card: %s, verb: %s, "
                "device: %s\n", ucm_value_name, card_name, verb_name, dev_or_mod_name);
        ret = -1;
        goto free_card;
    }

    ctl_vol = get_ctl_vol_from_ucm_vol(ucm_vol_str);
    if (ctl_vol == NULL) {
        aua_stderr("Failed to get the control identifier and value from %s\n",
                ucm_vol_str);
        ret = -1;
        goto free_ucm_vol_str;
    }

    if (is_set_default_value) {
        sprintf(ctl_vol_value, "%s", ctl_vol->value);
    } else {
        sprintf(ctl_vol_value, "%d", volume);
    }
    err = control_value_set(card, ctl_vol->id, ctl_vol_value);
    if (err != 0) {
        aua_stderr("Failed to set value of \"%s\" to \"%s\" in card %s\n",
                ctl_vol->id, ctl_vol_value, card);
        ret = err;
        goto free_ctl_vol;
    }

    ret = 0;

free_ctl_vol:
    free_ctl_vol(ctl_vol);
free_ucm_vol_str:
    free(ucm_vol_str);
free_card:
    free(card);
out:
    return ret;
}


/*************************************************************
 * Common get & set volume
 *************************************************************/
int aua_playback_volume_get(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int *volume)
{
    return volume_get(playback_volume_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

int aua_playback_volume_set(const char *card_name, const char *verb_name,
                            const char *dev_or_mod_name, int volume)
{
    return volume_set(playback_volume_value_name,
            card_name, verb_name, dev_or_mod_name, volume, 0);
}

int aua_capture_volume_get(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int *volume)
{
    return volume_get(capture_volume_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

int aua_capture_volume_set(const char *card_name, const char *verb_name,
                           const char *dev_or_mod_name, int volume)
{
    return volume_set(capture_volume_value_name,
            card_name, verb_name, dev_or_mod_name, volume, 0);
}


/*************************************************************
 * Default use case get & set volume
 *************************************************************/
static int (*use_case_volume_get[])(const char *card_name, const char *verb_name,
        const char *dev_or_mod_name, int *volume) = {
    aua_playback_volume_get,
    aua_capture_volume_get
};

static int (*use_case_volume_set[])(const char *card_name, const char *verb_name,
        const char *dev_or_mod_name, int volume) = {
    aua_playback_volume_set,
    aua_capture_volume_set
};

static int default_use_case_volume_get(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        enum behaviour play_or_record, int *volume)
{
    int ret = 0;
    int err = 0;
    struct use_case_names *names;
    int v = 0;

    err = default_names_init(
            card_name, verb_name, dev_name, mod_name, &names, play_or_record);
    if (err != 0) {
        ret = err;
        goto out;
    }

    err = use_case_volume_get[play_or_record](names->card, names->verb,
            (names->dev ? names->dev : names->mod), &v);
    if (err != 0) {
        ret = err;
        goto free_default_names;
    }

    *volume = v;
    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

static int default_use_case_volume_set(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        enum behaviour play_or_record, int volume)
{
    int ret = 0;
    int err = 0;
    struct use_case_names *names;

    err = default_names_init(
            card_name, verb_name, dev_name, mod_name, &names, play_or_record);
    if (err != 0) {
        ret = err;
        goto out;
    }

    err = use_case_volume_set[play_or_record](names->card, names->verb,
            (names->dev ? names->dev : names->mod), volume);
    if (err != 0) {
        ret = err;
        goto free_default_names;
    }

    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

int aua_default_play_volume_get(const char *card_name, const char *verb_name,
                                const char *dev_name, const char *mod_name,
                                int *volume)
{
    return default_use_case_volume_get(
            card_name, verb_name, dev_name, mod_name, PLAY, volume);
}

int aua_default_play_volume_set(const char *card_name, const char *verb_name,
                                const char *dev_name, const char *mod_name,
                                int volume)
{
    return default_use_case_volume_set(
            card_name, verb_name, dev_name, mod_name, PLAY, volume);
}

int aua_default_record_volume_get(const char *card_name, const char *verb_name,
                                  const char *dev_name, const char *mod_name,
                                  int *volume)
{
    return default_use_case_volume_get(
            card_name, verb_name, dev_name, mod_name, RECORD, volume);
}

int aua_default_record_volume_set(const char *card_name, const char *verb_name,
                                  const char *dev_name, const char *mod_name,
                                  int volume)
{
    return default_use_case_volume_set(
            card_name, verb_name, dev_name, mod_name, RECORD, volume);
}

/*************************************************************
 * Common set volume to default
 *************************************************************/
int aua_playback_volume_set_default(const char *card_name, const char *verb_name,
                                    const char *dev_or_mod_name)
{
    return volume_set(playback_volume_value_name,
            card_name, verb_name, dev_or_mod_name, 0, 1);
}

int aua_capture_volume_set_default(const char *card_name, const char *verb_name,
                                   const char *dev_or_mod_name)
{
    return volume_set(capture_volume_value_name,
            card_name, verb_name, dev_or_mod_name, 0, 1);
}


/*************************************************************
 * Default use case set volume to default
 *************************************************************/
static int (*use_case_volume_set_default[])(const char *card_name,
        const char *verb_name, const char *dev_or_mod_name) = {
    aua_playback_volume_set_default,
    aua_capture_volume_set_default
};

static int default_use_case_volume_set_default(const char *card_name,
        const char *verb_name, const char *dev_name, const char *mod_name,
        enum behaviour play_or_record)
{
    int ret = 0;
    int err = 0;
    struct use_case_names *names;

    err = default_names_init(
            card_name, verb_name, dev_name, mod_name, &names, play_or_record);
    if (err != 0) {
        ret = err;
        goto out;
    }

    err = use_case_volume_set_default[play_or_record](
            names->card, names->verb, (names->dev ? names->dev : names->mod));
    if (err != 0) {
        ret = err;
        goto free_default_names;
    }

    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

int aua_default_play_volume_set_default(const char *card_name,
                                        const char *verb_name,
                                        const char *dev_name,
                                        const char *mod_name)
{
    return default_use_case_volume_set_default(
            card_name, verb_name, dev_name, mod_name, PLAY);
}

int aua_default_record_volume_set_default(const char *card_name,
                                          const char *verb_name,
                                          const char *dev_name,
                                          const char *mod_name)
{
    return default_use_case_volume_set_default(
            card_name, verb_name, dev_name, mod_name, RECORD);
}


/*************************************************************
 * Common get min/max volume
 *************************************************************/
int aua_playback_volume_min_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume)
{
    return value_string_to_integer(playback_volume_min_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

int aua_playback_volume_max_get(const char *card_name, const char *verb_name,
                                const char *dev_or_mod_name, int *volume)
{
    return value_string_to_integer(playback_volume_max_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

int aua_capture_volume_min_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume)
{
    return value_string_to_integer(capture_volume_min_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

int aua_capture_volume_max_get(const char *card_name, const char *verb_name,
                               const char *dev_or_mod_name, int *volume)
{
    return value_string_to_integer(capture_volume_max_value_name,
            card_name, verb_name, dev_or_mod_name, volume);
}

/*************************************************************
 * Default use case get min/max volume
 *************************************************************/
static int (*use_case_volume_range_get[][2])(const char *card_name,
        const char *verb_name, const char *dev_or_mod_name, int *volume) = {
    {aua_playback_volume_min_get, aua_playback_volume_max_get},
    {aua_capture_volume_min_get, aua_capture_volume_max_get},
};

static int default_use_case_volume_range_get(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        enum behaviour play_or_record, enum volume_range min_or_max,
        int *volume)
{
    int ret = 0;
    int err = 0;
    struct use_case_names *names;
    int v = 0;

    err = default_names_init(
            card_name, verb_name, dev_name, mod_name, &names, play_or_record);
    if (err != 0) {
        ret = err;
        goto out;
    }

    err = use_case_volume_range_get[play_or_record][min_or_max](
            names->card, names->verb, (names->dev ? names->dev : names->mod), &v);
    if (err != 0) {
        ret = err;
        goto free_default_names;
    }

    *volume = v;
    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

int aua_default_play_volume_min_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume)
{
    return default_use_case_volume_range_get(
            card_name, verb_name, dev_name, mod_name, PLAY, VOL_MIN, volume);
}

int aua_default_play_volume_max_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume)
{
    return default_use_case_volume_range_get(
            card_name, verb_name, dev_name, mod_name, PLAY, VOL_MAX, volume);
}

int aua_default_record_volume_min_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume)
{
    return default_use_case_volume_range_get(
            card_name, verb_name, dev_name, mod_name, RECORD, VOL_MIN, volume);
}

int aua_default_record_volume_max_get(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name, int *volume)
{
    return default_use_case_volume_range_get(
            card_name, verb_name, dev_name, mod_name, RECORD, VOL_MAX, volume);
}
