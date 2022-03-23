#include <string.h>
#include <alsa/asoundlib.h>
#include <alsa/use-case.h>
#include "common.h"
#include "control.h"
#include "alsa-ucm-aw.h"

/*
 * Retrun the string after adding slash ("/") prefix on success,
 * otherwise NULL on error
 */
static char *add_slash_prefix(const char *str)
{
    unsigned long add_len = (str == NULL) ? 0 : strlen(str);
    char *str_out = (char *)malloc(add_len + 2);
    if (str_out == NULL) {
        aua_stderr("Failed to allocate memory to add slash prefix for string: "
                "%s\n", str);
        return NULL;
    }
    strcpy(str_out, "/");
    if (str != NULL) {
        strcat(str_out, str);
    }
    return str_out;
}

/*
 * Return the value identifier on success (value_name/dev_or_mod_name/verb_name),
 * otherwise NULL on error
 */
static char *get_value_id_from_names(const char *value_name,
        const char *verb_name, const char *dev_or_mod_name)
{
    char *value_id = NULL;
    char *verb_id = NULL;
    char *dev_or_mod_id = NULL;

    if (value_name == NULL) {
        aua_stderr("The value name is NULL\n");
        value_id = NULL;
        goto out;
    }

    verb_id = add_slash_prefix(verb_name);
    if (verb_id == NULL) {
        aua_stderr("Failed to add \"/\" prefix for the verb: %s\n", verb_name);
        value_id = NULL;
        goto out;
    }

    dev_or_mod_id = add_slash_prefix(dev_or_mod_name);
    if (dev_or_mod_id == NULL) {
        aua_stderr("Failed to add \"/\" prefix for the device or modifier: %s\n",
                dev_or_mod_name);
        value_id = NULL;
        goto free_verb_id;
    }

    value_id = (char *)malloc(strlen(value_name) + strlen(verb_id)
            + strlen(dev_or_mod_id) + 1);
    if (value_id == NULL) {
        aua_stderr("Failed to allocate memory for value identifier of %s\n",
                value_name);
        goto free_dev_or_mod_id;
    }
    strcpy(value_id, value_name);
    strcat(value_id, dev_or_mod_id);
    strcat(value_id, verb_id);

free_dev_or_mod_id:
    free(dev_or_mod_id);
free_verb_id:
    free(verb_id);
out:
    return value_id;
}

char *aua_value_get(const char *value_name, const char *card_name,
                    const char *verb_name, const char *dev_or_mod_name)
{
    int errnum = 0;
    snd_use_case_mgr_t *uc_mgr;
    char *value_id = NULL;
    char *value = NULL;

    errnum = uc_mgr_open(&uc_mgr, card_name);
    if (errnum != 0) {
        value = NULL;
        goto out;
    }

    value_id = get_value_id_from_names(value_name, verb_name, dev_or_mod_name);
    if (value_id == NULL) {
        value = NULL;
        goto close_uc_mgr;
    }

    errnum = snd_use_case_get(uc_mgr, value_id, &value);
    if (errnum < 0) {
        aua_stderr("Failed to get value from the identifier: %s (%s)\n",
                value_id, snd_strerror(errnum));
        value = NULL;
        goto free_value_id;
    }

free_value_id:
    free(value_id);
close_uc_mgr:
    uc_mgr_close(uc_mgr);
out:
    return value;
}

int value_string_to_integer(const char *ucm_value_name, const char *card_name,
        const char *verb_name, const char *dev_or_mod_name, int *result)
{
    char *ucm_value = aua_value_get(
            ucm_value_name, card_name, verb_name, dev_or_mod_name);
    if (ucm_value == NULL) {
        return -1;
    }
    *result = atoi(ucm_value);
    free(ucm_value);
    return 0;
}
