/*
 * Copyright (C) 2014 ALLWINNERTECH
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
#include <uci.h>

#include "bluetooth_params.h"

static struct uci_context *uci_ctx;
static struct uci_package *uci_bluetooth;

static struct uci_package *config_init_package(const char *config)
{
	struct uci_context *ctx;
	struct uci_package *p = NULL;

	ctx = uci_alloc_context();
    uci_ctx = ctx;

	ctx->flags &= ~UCI_FLAG_STRICT;

	if(uci_load(ctx, config, &p))
		return NULL;

	return p;
}

static void config_parse_params(struct uci_section *s, struct bluetooth_params *bt_params)
{
	const char *device = NULL;

	device = uci_lookup_option_string(uci_ctx, s, "device");
	if (device && bt_params) {
	    strncpy(bt_params->device, device, DEVICE_NAME_LEN);
		bt_params->device[DEVICE_NAME_LEN] = '\0';
	}
}

static void config_init_params(struct bluetooth_params *bt_params)
{
	struct uci_element *e;

	uci_foreach_element(&uci_bluetooth->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "params"))
			config_parse_params(s, bt_params);
	}
}

void get_params_from_config(const char *config, struct bluetooth_params *bt_params)
{
    uci_bluetooth =  config_init_package(config);
	config_init_params(bt_params);
}
