#ifndef _VOLUME_UCI_CONFIG_API_H_
#define _VOLUME_UCI_CONFIG_API_H_

int volume_uci_set_config(const char *name, const char *value);
int volume_uci_get_config(const char *name, char *value, size_t value_max_len);

#endif // _VOLUME_UCI_CONFIG_API_H_
