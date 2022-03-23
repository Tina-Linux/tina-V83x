#include <alsa/asoundlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "utils/AmixerUtils.h"

namespace AW {

int AmixerUtils::cset(const char *card, const char *iface, const char *name, const char *value)
{
    return amixer_opreation(card, iface, name, value, 0, nullptr);
}

int AmixerUtils::cset(const char *card, const char *iface, const char *name, int value)
{
    char set[10];
    snprintf(set, 10, "%d", value);
    return amixer_opreation(card, iface, name, set, 0, nullptr);
}

int AmixerUtils::cset(const char *card, const char *iface, const char *name, bool value)
{
    if(value)
        return amixer_opreation(card, iface, name, "on", 0, nullptr);

    return amixer_opreation(card, iface, name, "off", 0, nullptr);
}

int AmixerUtils::cget(const char *card, const char *iface, const char *name, std::string &value)
{
    return -1;
}

int AmixerUtils::cget(const char *card, const char *iface, const char *name, int &value)
{
    return -1;
}

int AmixerUtils::cget(const char *card, const char *iface, const char *name, bool &value)
{
    return -1;
}

int AmixerUtils::amixer_opreation(const char *card,
                                  const char *iface,
                                  const char *name,
                                  const char *value,
                                  int roflag,
                                  int *get)
{
    printf("%s iface: %s, name: %s value: %s\n",__func__, iface, name, value);
    int err;
    snd_ctl_t *handle = NULL;

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_value_t *control;

    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);

    char cmd[250];
    snprintf(cmd, 250, "iface=%s,name=%s", iface, name);
    if (snd_ctl_ascii_elem_id_parse(id, cmd)) {
        printf("Wrong control identifier: %s\n", cmd);
        return -EINVAL;
    }

    if (handle == NULL &&
        (err = snd_ctl_open(&handle, card, 0)) < 0) {
        printf("Control %s open error: %s\n", card, snd_strerror(err));
        return err;
    }

    snd_ctl_elem_info_set_id(info, id);

    if ((err = snd_ctl_elem_info(handle, info)) < 0) {
        printf("Cannot find the given element from control %s\n", card);
        snd_ctl_close(handle);
        handle = NULL;
        return err;
    }

    snd_ctl_elem_info_get_id(info, id);     /* FIXME: Remove it when hctl find works ok !!! */
    snd_ctl_elem_value_set_id(control, id);
    if ((err = snd_ctl_elem_read(handle, control)) < 0) {
        printf("Cannot read the given element from control %s\n", card);
        snd_ctl_close(handle);
        handle = NULL;
        return err;
    }

    if (!roflag) {
        //set
        err = snd_ctl_ascii_value_parse(handle, control, info, value);
        if (err < 0) {
            printf("Control %s parse error: %s\n", card, snd_strerror(err));
            snd_ctl_close(handle);
            handle = NULL;
            return err;
        }

        if ((err = snd_ctl_elem_write(handle, control)) < 0) {
            printf("Control %s element %s %s write error: %s\n", card, name, value, snd_strerror(err));
            snd_ctl_close(handle);
            handle = NULL;
            return err;
        }
    }else{
        //get
        unsigned int count;
        snd_ctl_elem_type_t type;
        count = snd_ctl_elem_info_get_count(info);
        type  = snd_ctl_elem_info_get_type(info);
        if(count > 1) {
            printf("TODO: snd_ctl_elem_info_get_count = %u\n", count);
        }else{

            switch(type){
                case SND_CTL_ELEM_TYPE_INTEGER:{
                    *get = snd_ctl_elem_value_get_integer(control, 0);
                    break;
                }
                case SND_CTL_ELEM_TYPE_BOOLEAN:{
                    *get = snd_ctl_elem_value_get_boolean(control, 0);
                    break;
                }
                default:
                ;
            }
        }
    }

    snd_ctl_close(handle);
    handle = NULL;
    return 0;
}

}
