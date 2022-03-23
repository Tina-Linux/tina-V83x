#include "control.h"
#include <alsa/asoundlib.h>
#include "common.h"

enum rw_flag {
    READ = 0,
    WRITE,
};

#if 0
/*
 * Add the "hw:" prefix for the sound card name.
 *  (e.g. "audiocodec" -> "hw:audiocodec")
 * Return the result string on success, otherwise NULL on error.
 */
static char *add_hw_prefix_for_card_name(const char *card)
{
    if (card == NULL) {
        aua_stderr("Failed to add \"hw:\" prefix: the card name is NULL\n");
        return NULL;
    }

    unsigned long add_len = strlen(card);
    char *str_out = (char *)malloc(add_len + 4);
    if (str_out == NULL) {
        aua_stderr("Failed to add \"hw:\" prefix for %s: no memory\n", card);
        return NULL;
    }
    strcpy(str_out, "hw:");
    strcat(str_out, card);
    return str_out;
}
#endif

/*
 * Return the value in string on success, otherwise NULL on error.
 * The returned string is dynamically allocated, needed to be deallocated by free()
 */
static char *get_value_from_ctl_elem(snd_ctl_t *ctl, snd_ctl_elem_info_t *info,
        snd_ctl_elem_value_t *value)
{
    int err;
    unsigned int index;
    unsigned int count;
    unsigned int item;
    snd_ctl_elem_type_t type;
    snd_aes_iec958_t iec958;
    char value_buf[AUA_STRING_MAX_LEN] = "";
    char value_result[AUA_STRING_MAX_LEN] = "";
    char *value_out = NULL;

    count = snd_ctl_elem_info_get_count(info);
    type = snd_ctl_elem_info_get_type(info);

    for (index = 0; index < count; ++index) {
        if (index > 0) {
            strcat(value_result, ",");
        }
        switch (type) {
        case SND_CTL_ELEM_TYPE_BOOLEAN:
            sprintf(value_buf, "%d",
                    snd_ctl_elem_value_get_boolean(value, index));
            strcat(value_result, value_buf);
            break;
        case SND_CTL_ELEM_TYPE_INTEGER:
            sprintf(value_buf, "%li",
                    snd_ctl_elem_value_get_integer(value, index));
            strcat(value_result, value_buf);
            break;
        case SND_CTL_ELEM_TYPE_INTEGER64:
            sprintf(value_buf, "%Li",
                    snd_ctl_elem_value_get_integer64(value, index));
            strcat(value_result, value_buf);
            break;
        case SND_CTL_ELEM_TYPE_ENUMERATED:
            item = snd_ctl_elem_value_get_enumerated(value, index);
            snd_ctl_elem_info_set_item(info, item);
            err = snd_ctl_elem_info(ctl, info);
            if (err < 0) {
                aua_stderr("Failed to get value: getting element info error\n",
                        snd_strerror(err));
                value_out = NULL;
                goto out;
            }
            strcat(value_result, snd_ctl_elem_info_get_item_name(info));
            break;
        case SND_CTL_ELEM_TYPE_BYTES:
            sprintf(value_buf, "0x%02x",
                    snd_ctl_elem_value_get_byte(value, index));
            strcat(value_result, value_buf);
            break;
        case SND_CTL_ELEM_TYPE_IEC958:
            snd_ctl_elem_value_get_iec958(value, &iec958);
            sprintf(value_buf,
                    "[AES0=0x%02x AES1=0x%02x AES2=0x%02x AES3=0x%02x]",
                    iec958.status[0], iec958.status[1],
                    iec958.status[2], iec958.status[3]);
            strcat(value_result, value_buf);
            break;
        default:
            aua_stderr("Failed to get value: unknown element type\n");
            value_out = NULL;
            goto out;
        }
    }

    value_out = strdup(value_result);

out:
    return value_out;
}

/**
 * control_config() - Read or write the value of an ALSA control
 * @param card:             (in) ALSA virtual card name (e.g. "hw:0", "hw:audiocodec", etc.)
 * @param ctl_id:           (in) ALSA control identifier (e.g. "numid=xxx", "name='xxx'", etc.)
 * @param ctl_value:        (in&out) The control value
 * @param read_or_write:    READ or WRITE
 * @return: 0 on success, otherwise on error
 */
static int control_config(const char *card, const char *ctl_id,
        char *ctl_value[], enum rw_flag read_or_write)
{
    int ret;
    int err;
    snd_ctl_t *ctl = NULL;
    snd_ctl_elem_id_t *id = NULL;
    snd_ctl_elem_info_t *info = NULL;
    snd_ctl_elem_value_t *value = NULL;

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&value);

    if (card == NULL) {
        aua_stderr("The card name is NULL\n");
        ret = -EINVAL;
        goto out;
    }
    if (ctl_id == NULL) {
        aua_stderr("The control identifier is NULL. Please specify it:\n"
                "\t[[iface=<iface>,][name='name',][index=<index>,]"
                "[device=<device>,][subdevice=<subdevice>]]|[numid=<numid>]\n");
        ret = -EINVAL;
        goto out;
    }
    if (read_or_write == WRITE && *ctl_value == NULL) {
        aua_stderr("The value of %s is NULL\n", ctl_id);
        ret = -EINVAL;
        goto out;
    }

    if (0 != snd_ctl_ascii_elem_id_parse(id, ctl_id)) {
        aua_stderr("Failed to parse the control identifier: %s\n", ctl_id);
        ret = -EINVAL;
        goto out;
    }

    err = snd_ctl_open(&ctl, card, 0);
    if (err < 0) {
        aua_stderr("Failed to open CTL of %s (%s)\n", card, snd_strerror(err));
        ret = err;
        goto out;
    }
    snd_ctl_elem_info_set_id(info, id);
    err = snd_ctl_elem_info(ctl, info);
    if (err < 0) {
        aua_stderr("Failed to get CTL information of %s (%s)\n", card,
                snd_strerror(err));
        ret = err;
        goto close_ctl;
    }
    snd_ctl_elem_value_set_id(value, id);
    err = snd_ctl_elem_read(ctl, value);
    if (err < 0) {
        aua_stderr("Failed to read the given element from %s (%s)\n",
                card, snd_strerror(err));
        ret = err;
        goto close_ctl;
    }

    switch (read_or_write) {
    case READ:
        *ctl_value = get_value_from_ctl_elem(ctl, info, value);
        if (*ctl_value == NULL) {
            ret = -1;
            goto close_ctl;
        }
        break;
    case WRITE:
        err = snd_ctl_ascii_value_parse(ctl, value, info, *ctl_value);
        if (err < 0) {
            aua_stderr("Failed to parse the value of %s in card %s (%s)\n",
                    ctl_id, card, snd_strerror(err));
            ret = err;
            goto close_ctl;
        }
        err = snd_ctl_elem_write(ctl, value);
        if (err < 0) {
            aua_stderr("Failed to write value '%s' to %s in card %s (%s)\n",
                    *ctl_value, ctl_id, card, snd_strerror(err));
            ret = err;
            goto close_ctl;
        }
        break;
    default:
        aua_stderr("Invalid operation\n");
        ret = -EINVAL;
        goto close_ctl;
    }

    ret = 0;

close_ctl:
    snd_ctl_close(ctl);
out:
    return ret;
}

char *control_value_get(const char *card, const char *ctl_id)
{
    char *value = NULL;
    int err = control_config(card, ctl_id, &value, READ);
    if (err != 0) {
        value = NULL;
    }
    return value;
}

int control_value_set(const char *card, const char *ctl_id, const char *ctl_value)
{
    return control_config(card, ctl_id, (char **)&ctl_value, WRITE);
}
