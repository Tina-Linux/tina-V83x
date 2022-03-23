#ifndef _NP_UCI_CONFIG_H_
#define _NP_UCI_CONFIG_H_

#include "uci.h"

#define NATIVE_POWER_CONFIG_PATH "/etc/config/nativepower"
typedef struct {
	struct uci_context *context;
	struct uci_package *package;
} NP_UCI;

NP_UCI *np_uci_open(const char *file);
void np_uci_close(NP_UCI * uci);
int np_uci_read_config(NP_UCI * uci, const char *conf_section, const char *conf_name, char *conf_buf, size_t len);
int np_uci_write_config(NP_UCI * uci, const char *conf_section, const char *conf_name, const char *conf_buff);

#endif
