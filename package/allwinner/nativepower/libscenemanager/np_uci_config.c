/*
 * Copyright (C) 2016 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include <stdlib.h>

#include "np_uci_config.h"

NP_UCI *np_uci_open(const char *file)
{
	int ret = -1;
	NP_UCI *uci = (NP_UCI *) malloc(sizeof(NP_UCI));
	memset(uci, 0, sizeof(NP_UCI));

	uci->context = uci_alloc_context();	// 申请一个UCI上下文.

	ret = uci_load(uci->context, file, &uci->package);
	if (UCI_OK != ret) {
		np_uci_close(uci);
		return NULL;
	}

	return uci;
}

void np_uci_close(NP_UCI * uci)
{
	if (uci == NULL) {
		return;
	}

	if (uci->package != NULL) {
		uci_unload(uci->context, uci->package);
	}
	uci_free_context(uci->context);

	free(uci);
}

int np_uci_read_config(NP_UCI * uci, const char *conf_section, const char *conf_name, char *conf_buf, size_t len)
{
	int ret = -1;
	struct uci_section *section = NULL;
	const char *value = NULL;

	if (uci == NULL) {
		return -1;
	}

	section = uci_lookup_section(uci->context, uci->package, conf_section);
	if (section == NULL) {
		return -1;
	}

	value = uci_lookup_option_string(uci->context, section, conf_name);
	if (NULL != value && len > strlen(value)) {
		memcpy(conf_buf, value, strlen(value));
	}

	return 0;
}

int np_uci_write_config(NP_UCI * uci, const char *conf_section, const char *conf_name, const char *conf_buff)
{
	int ret = -1;
	struct uci_ptr ptr = {.package = "nativepower",.section = conf_section,.option = conf_name,.value = conf_buff, };

	if (uci == NULL) {
		return -1;
	}
	ret = uci_set(uci->context, &ptr);
	if (UCI_OK == ret) {
		ret = uci_commit(uci->context, &ptr.p, true);
	}

	return ret;
}
