#include "common.h"
#include <stdio.h>
#include <stdarg.h>

const char default_card_name[] = "audiocodec";
const char default_verb_play_name[] = "Play";
const char default_verb_record_name[] = "Record";

static const char list_dev_id_prefix[] = "_devices/";
static const char list_mod_id_prefix[] = "_modifiers/";

void aua_stderr(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "[ERROR] [alsa-ucm-aw] ");
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void aua_stdout(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stdout, "[alsa-ucm-aw] ");
    vfprintf(stdout, fmt, va);
    va_end(va);
}

int uc_mgr_open(snd_use_case_mgr_t **uc_mgr, const char *card_name)
{
    int ret = 0;
    int errnum = 0;
    if (!card_name) {
        aua_stderr("The sound card name is NULL\n");
        ret = -EINVAL;
        goto out;
    }
    errnum = snd_use_case_mgr_open(uc_mgr, card_name);
    if (errnum < 0) {
        aua_stderr("Failed to open sound card: %s (%s)\n", card_name,
                snd_strerror(errnum));
        ret = errnum;
        goto out;
    }
    ret = 0;
out:
    return ret;
}

int uc_mgr_close(snd_use_case_mgr_t *uc_mgr)
{
    return snd_use_case_mgr_close(uc_mgr);
}

static char *get_default_card_name(void)
{
    const char **list = NULL;
    int err = 0;
    int i, cnt;
    char *result = NULL;

    /*
     * Note:
     *  In the list got via snd_use_case_get_list, the odd members are the names
     *  of elements, and the even members are their corresponding comments.
     */

    /*
     * Get sound card list.
     * If a card whose name is the same as the default_card_name, use it.
     * Otherwise, use the name of first card.
     */
    err = snd_use_case_get_list(NULL, NULL, &list);
    if (err < 0) {
        aua_stderr("Failed to get sound card list (%s)\n",
                snd_strerror(err));
        result = NULL;
        goto out;
    } else if (err == 0) {
        aua_stderr("Sound card list is empty\n");
        result = NULL;
        goto out;
    }
    cnt = err / 2;
    for (i = 0; i < cnt; ++i) {
        if (0 == strcmp(list[i * 2], default_card_name)) {
            result = strdup(default_card_name);
            break;
        }
    }
    if (!result) {
        result = strdup(list[0]);
    }
    snd_use_case_free_list(list, err);

out:
    return result;
}

static char *get_default_verb_name(snd_use_case_mgr_t *uc_mgr,
                                   enum behaviour play_or_record)
{
    const char **list = NULL;
    int err = 0;
    int i, cnt;
    char *result = NULL;

    /*
     * Get verb list.
     * If a verb whose name is the same as the default_verb_name, use it.
     * Otherwise, use the name of first verb.
     */
    err = snd_use_case_get_list(uc_mgr, "_verbs", &list);
    if (err < 0) {
        aua_stderr("Failed to get verb list (%s)\n", snd_strerror(err));
        result = NULL;
        goto out;
    } else if (err == 0) {
        aua_stderr("Verb list is empty\n");
        result = NULL;
        goto out;
    }
    cnt = err / 2;
    const char *default_verb_name = (play_or_record == PLAY) ?
        default_verb_play_name : default_verb_record_name;
    for (i = 0; i < cnt; ++i) {
        if (0 == strcmp(list[i * 2], default_verb_name)) {
            result = strdup(default_verb_name);
            break;
        }
    }
    if (!result) {
        result = strdup(list[0]);
    }
    snd_use_case_free_list(list, err);

out:
    return result;
}

static char *get_default_dev_or_mod_name(const char *list_id_prefix,
        const char *verb_name, snd_use_case_mgr_t *uc_mgr)
{
    const char **list = NULL;
    int err = 0;
    char *result = NULL;

    char *list_id
        = (char *)malloc(strlen(list_id_prefix) + strlen(verb_name) + 1);
    if (list_id == NULL) {
        result = NULL;
        goto out;
    }
    list_id[0] = '\0';
    strcat(list_id, list_dev_id_prefix);
    strcat(list_id, verb_name);
    err = snd_use_case_get_list(uc_mgr, list_id, &list);
    free(list_id);
    if (err <= 0) {
        result = NULL;
        goto out;
    }
    result = strdup(list[0]);
    snd_use_case_free_list(list, err);
out:
    return result;
}

int default_names_init(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        struct use_case_names **names, enum behaviour play_or_record)
{
    int ret = 0;
    int errnum = 0;
    int i, cnt;
    const char **list = NULL;

    struct use_case_names *n
        = (struct use_case_names *)malloc(sizeof(struct use_case_names));
    if (n == NULL) {
        aua_stderr("Failed to allocate memory for default names\n");
        ret = -ENOMEM;
        goto error_out;
    }
    n->card = NULL;
    n->verb = NULL;
    n->dev = NULL;
    n->mod = NULL;

    n->card = card_name ? strdup(card_name) : get_default_card_name();
    if (n->card == NULL) {
        ret = -1;
        goto free_use_case_names;
    }

    snd_use_case_mgr_t *uc_mgr;
    errnum = uc_mgr_open(&uc_mgr, n->card);
    if (errnum != 0) {
        ret = errnum;
        goto free_card_name;
    }

    n->verb = verb_name ?
        strdup(verb_name) : get_default_verb_name(uc_mgr, play_or_record);
    if (n->verb == NULL) {
        ret = -1;
        goto close_uc_mgr;
    }

    if (dev_name == NULL && mod_name == NULL) {
        n->dev = get_default_dev_or_mod_name(list_dev_id_prefix, n->verb, uc_mgr);
        if (n->dev == NULL) {
            n->mod = get_default_dev_or_mod_name(list_mod_id_prefix, n->verb, uc_mgr);
            if (n->mod == NULL) {
                ret = -1;
                goto free_verb_name;
            }
        }
    } else {
        n->dev = dev_name ? strdup(dev_name) : NULL;
        n->mod = mod_name ? strdup(mod_name) : NULL;
        if (n->dev == NULL && n->mod == NULL) {
            ret = -1;
            goto free_verb_name;
        }
    }

    uc_mgr_close(uc_mgr);
    *names = n;
    return 0;

free_verb_name:
    free(n->verb);
close_uc_mgr:
    uc_mgr_close(uc_mgr);
free_card_name:
    free(n->card);
free_use_case_names:
    free(n);
    n = NULL;
error_out:
    *names = n;
    return ret;
}

void default_names_free(struct use_case_names *names)
{
    if (names) {
        free(names->card);
        free(names->verb);
        free(names->dev);
        free(names->mod);
        free(names);
    }
}

int print_all_list(void)
{
    int ret = 0;
    int errnum = 0;
    const char **card_list = NULL;
    const char **verb_list = NULL;
    const char **dev_list = NULL;
    const char **mod_list = NULL;
    int cards = 0;
    int verbs = 0;
    int devs = 0;
    int mods = 0;
    snd_use_case_mgr_t *uc_mgr;
    int i, j, k;

    fprintf(stdout, "\n");
    errnum = snd_use_case_get_list(NULL, NULL, &card_list);
    if (errnum < 0) {
        aua_stderr("Failed to get sound card list (%s)\n", snd_strerror(errnum));
        ret = errnum;
        goto err_out;
    } else if (errnum == 0) {
        aua_stderr("Sound card list is empty\n");
        ret = -ENOENT;
        goto err_out;
    }
    cards = errnum / 2;

    for (i = 0; i < cards; ++i) {
        const char *card = card_list[i * 2];
        const char *card_comment = card_list[i * 2  + 1];
        fprintf(stdout, "[c] %s", card);
        if (card_comment) {
            fprintf(stdout, " (%s)\n", card_comment);
        } else {
            fprintf(stdout, "\n");
        }
        errnum = uc_mgr_open(&uc_mgr, card);
        if (errnum != 0) {
            ret = errnum;
            goto err_free_card_list;
        }
        errnum = snd_use_case_get_list(uc_mgr, "_verbs", &verb_list);
        if (errnum < 0) {
            aua_stderr("Failed to get verb list of %s (%s)\n", card, snd_strerror(errnum));
            ret = errnum;
            goto err_close_uc_mgr;
        } else if (errnum == 0) {
            goto close_uc_mgr;
        }
        verbs = errnum / 2;
        for (j = 0; j < verbs; ++j) {
            const char *verb = verb_list[j * 2];
            const char *verb_comment = verb_list[j * 2 + 1];
            fprintf(stdout, "\t[v] %s", verb);
            if (verb_comment) {
                fprintf(stdout, " (%s)\n", verb_comment);
            } else {
                fprintf(stdout, "\n");
            }
            int verb_len = strlen(verb);

            char *dev_id = (char *)malloc(strlen("_devices/") + verb_len + 1);
            if (dev_id == NULL) {
                aua_stderr("Failed to allocate memory for device list identifier\n");
                ret = -ENOMEM;
                goto err_free_verb_list;
            }
            sprintf(dev_id, "_devices/%s", verb);
            errnum = snd_use_case_get_list(uc_mgr, dev_id, &dev_list);
            if (errnum < 0) {
                aua_stderr("Failed to get device list of %s/%s (%s)\n", verb,
                        card, snd_strerror(errnum));
                ret = errnum;
                goto err_free_verb_list;
            } else if (errnum != 0) {
                devs = errnum / 2;
                for (k = 0; k < devs; ++k) {
                    const char *dev = dev_list[k * 2];
                    const char *dev_comment = dev_list[k * 2 + 1];
                    fprintf(stdout, "\t\t[d] %s", dev);
                    if (dev_comment) {
                        fprintf(stdout, " (%s)\n", dev_comment);
                    } else {
                        fprintf(stdout, "\n");
                    }
                }
            }
            snd_use_case_free_list(dev_list, devs);
            free(dev_id);

            char *mod_id = (char *)malloc(strlen("_modifiers/") + verb_len + 1);
            if (mod_id == NULL) {
                aua_stderr("Failed to allocate memory for modifier list identifier\n");
                ret = -ENOMEM;
                goto err_free_dev_list;
            }
            sprintf(mod_id, "_modifiers/%s", verb);
            errnum = snd_use_case_get_list(uc_mgr, mod_id, &mod_list);
            if (errnum < 0) {
                aua_stderr("Failed to get modifier list of %s/%s (%s)\n", verb,
                        card, snd_strerror(errnum));
                ret = errnum;
                goto err_free_dev_list;
            } else if (errnum != 0) {
                mods = errnum / 2;
                for (k = 0; k < mods; ++k) {
                    const char *mod = mod_list[k * 2];
                    const char *mod_comment = mod_list[k * 2 + 1];
                    fprintf(stdout, "\t\t[m] %s", mod);
                    if (mod_comment) {
                        fprintf(stdout, " (%s)\n", mod_comment);
                    } else {
                        fprintf(stdout, "\n");
                    }
                }
            }
            snd_use_case_free_list(mod_list, mods);
            free(mod_id);
        }
        snd_use_case_free_list(verb_list, verbs);
close_uc_mgr:
        uc_mgr_close(uc_mgr);
    }
    snd_use_case_free_list(card_list, cards);
    fprintf(stdout, "\n");
    return 0;

err_free_dev_list:
    snd_use_case_free_list(dev_list, devs);
err_free_verb_list:
    snd_use_case_free_list(verb_list, verbs);
err_close_uc_mgr:
    uc_mgr_close(uc_mgr);
err_free_card_list:
    snd_use_case_free_list(card_list, cards);
err_out:
    return ret;
}
