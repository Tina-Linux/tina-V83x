#include <string.h>
#include <alsa/asoundlib.h>
#include <alsa/use-case.h>
#include "common.h"
#include "alsa-ucm-aw.h"

enum status {
    ENABLE = 0,
    DISABLE,
};

static const char enadev_id[] = "_enadev";
static const char disdev_id[] = "_disdev";
static const char enamod_id[] = "_enamod";
static const char dismod_id[] = "_dismod";

static int use_case_config(snd_use_case_mgr_t *uc_mgr, const char *verb_name,
        const char *dev_name, enum status dev_ena_or_dis,
        const char *mod_name, enum status mod_ena_or_dis)
{
    int ret = 0;
    int errnum = 0;
    const char *dev_id = NULL;
    const char *mod_id = NULL;

    if (!verb_name) {
        aua_stderr("The verb name is NULL\n");
        ret = -EINVAL;
        goto out;
    }
    errnum = snd_use_case_set(uc_mgr, "_verb", verb_name);
    if (errnum < 0) {
        aua_stderr("Failed to set verb: %s (%s)\n", verb_name,
                snd_strerror(errnum));
        ret = errnum;
        goto out;
    }

    if (dev_name) {
        dev_id = (dev_ena_or_dis == ENABLE) ? enadev_id : disdev_id;
        errnum = snd_use_case_set(uc_mgr, dev_id, dev_name);
        if (errnum < 0) {
            aua_stderr("Failed to set %s: %s (%s)\n", dev_id, dev_name,
                    snd_strerror(errnum));
            ret = errnum;
            goto out;
        }
    }

    if (mod_name) {
        mod_id = (mod_ena_or_dis == ENABLE) ? enamod_id : dismod_id;
        errnum = snd_use_case_set(uc_mgr, mod_id, mod_name);
        if (errnum < 0) {
            aua_stderr("Failed to set %s: %s (%s)\n", mod_id, mod_name,
                    snd_strerror(errnum));
            ret = errnum;
            goto out;
        }
    }

    ret = 0;
out:
    return ret;
}

int aua_use_case_enable(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name)
{
    int ret = 0;
    int errnum = 0;
    snd_use_case_mgr_t *uc_mgr;

    errnum = uc_mgr_open(&uc_mgr, card_name);
    if (errnum != 0) {
        ret = errnum;
        goto out;
    }

    errnum = use_case_config(uc_mgr, verb_name,
                             dev_name, ENABLE,
                             mod_name, ENABLE);
    if (errnum != 0) {
        ret = errnum;
        goto close_uc_mgr;
    }

    ret = 0;

close_uc_mgr:
    uc_mgr_close(uc_mgr);
out:
    return ret;
}

int aua_use_case_disable(const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name)
{
    int ret = 0;
    int errnum = 0;
    snd_use_case_mgr_t *uc_mgr;

    errnum = uc_mgr_open(&uc_mgr, card_name);
    if (errnum != 0) {
        ret = errnum;
        goto out;
    }

    /*
     * To disable a ALSA UCM device or modifier, we need to enable it first.
     *
     * Because the underlying implementation snd_use_case_set will determine
     * whether the device or modifier is already enabled or not, whose
     * information is saved in the structure uc_mgr. If a device or modifier
     * is not enabled, we can't disable it. However, our uc_mgr is a newly
     * created variable, which saves nothing.
     *
     * Therefore, we enable the device or modifier additionally before disabling
     * it, to ensure the disabling operation is successful.
     */
    errnum = use_case_config(uc_mgr, verb_name,
                             dev_name, ENABLE,
                             mod_name, ENABLE);
    if (errnum != 0) {
        ret = errnum;
        goto close_uc_mgr;
    }
    errnum = use_case_config(uc_mgr, verb_name,
                             dev_name, DISABLE,
                             mod_name, DISABLE);
    if (errnum != 0) {
        ret = errnum;
        goto close_uc_mgr;
    }

    ret = 0;

close_uc_mgr:
    uc_mgr_close(uc_mgr);
out:
    return ret;
}

static int (*use_case_handle[])(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name) = {
    aua_use_case_enable,
    aua_use_case_disable
};

static int default_use_case_handle(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        enum behaviour play_or_record, enum status enable_or_disable)
{
    int ret = 0;
    int errnum = 0;
    struct use_case_names *names;

    errnum = default_names_init(
            card_name, verb_name, dev_name, mod_name, &names, play_or_record);
    if (errnum != 0) {
        ret = errnum;
        goto out;
    }

    errnum = use_case_handle[enable_or_disable](names->card, names->verb,
            names->dev, names->mod);
    if (errnum != 0) {
        ret = errnum;
        goto free_default_names;
    }

    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

int aua_default_play_enable(const char *card_name, const char *verb_name,
                            const char *dev_name, const char *mod_name)
{
    return default_use_case_handle(
            card_name, verb_name, dev_name, mod_name, PLAY, ENABLE);
}

int aua_default_play_disable(const char *card_name, const char *verb_name,
                             const char *dev_name, const char *mod_name)
{
    return default_use_case_handle(
            card_name, verb_name, dev_name, mod_name, PLAY, DISABLE);
}

int aua_default_record_enable(const char *card_name, const char *verb_name,
                              const char *dev_name, const char *mod_name)
{
    return default_use_case_handle(
            card_name, verb_name, dev_name, mod_name, RECORD, ENABLE);
}

int aua_default_record_disable(const char *card_name, const char *verb_name,
                               const char *dev_name, const char *mod_name)
{
    return default_use_case_handle(
            card_name, verb_name, dev_name, mod_name, RECORD, DISABLE);
}

#if ENABLE_RESET_COMMAND
int aua_use_case_reset(const char *card_name)
{
    int ret = 0;
    int errnum = 0;
    snd_use_case_mgr_t *uc_mgr;

    errnum = uc_mgr_open(&uc_mgr, card_name);
    if (errnum != 0) {
        ret = errnum;
        goto out;
    }

    errnum = snd_use_case_mgr_reset(uc_mgr);
    if (errnum < 0) {
        aua_stderr("Failed to reset sound card %s (%s)\n", card_name,
                snd_strerror(errnum));
        ret = errnum;
        goto close_uc_mgr;
    }

    ret = 0;

close_uc_mgr:
    uc_mgr_close(uc_mgr);
out:
    return ret;
}

static int default_use_case_reset(const char *card_name,
                                  enum behaviour play_or_record)
{
    int ret = 0;
    int errnum = 0;
    struct use_case_names *names;

    errnum = default_names_init(
            card_name, NULL, NULL, NULL, &names, play_or_record);
    if (errnum != 0) {
        ret = errnum;
        goto out;
    }

    errnum = aua_use_case_reset(names->card);
    if (errnum != 0) {
        ret = errnum;
        goto free_default_names;
    }

    ret = 0;

free_default_names:
    default_names_free(names);
out:
    return ret;
}

int aua_default_play_reset(const char *card_name)
{
    return default_use_case_reset(card_name, PLAY);
}

int aua_default_record_reset(const char *card_name)
{
    return default_use_case_reset(card_name, RECORD);
}
#endif /* #if ENABLE_RESET_COMMAND */
