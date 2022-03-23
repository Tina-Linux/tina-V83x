#ifndef __ALSA_UCM_AW_COMMON_H__
#define __ALSA_UCM_AW_COMMON_H__

#include <alsa/asoundlib.h>
#include <alsa/use-case.h>

#define AUA_STRING_MAX_LEN 256

enum behaviour {
    NONE = -1,
    PLAY = 0,
    RECORD
};

struct use_case_names {
    char *card;
    char *verb;
    char *dev;
    char *mod;
};

extern const char default_card_name[];
extern const char default_verb_play_name[];
extern const char default_verb_record_name[];

void aua_stderr(const char *fmt, ...);
void aua_stdout(const char *fmt, ...);

int uc_mgr_open(snd_use_case_mgr_t **uc_mgr, const char *card_name);
int uc_mgr_close(snd_use_case_mgr_t *uc_mgr);

int default_names_init(
        const char *card_name, const char *verb_name,
        const char *dev_name, const char *mod_name,
        struct use_case_names **names, enum behaviour play_or_record);
void default_names_free(struct use_case_names *names);

int print_all_list(void);

#endif /* ifndef __ALSA_UCM_AW_COMMON_H__ */
