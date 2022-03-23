#ifndef __ALSA_UCM_AW_VALUE_H__
#define __ALSA_UCM_AW_VALUE_H__

char *aua_value_get(const char *value_name, const char *card_name,
                    const char *verb_name, const char *dev_or_mod_name);

int value_string_to_integer(const char *ucm_value_name, const char *card_name,
        const char *verb_name, const char *dev_or_mod_name, int *result);

#endif /* ifndef __ALSA_UCM_AW_VALUE_H__ */
