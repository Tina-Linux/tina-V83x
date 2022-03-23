#include <string.h>

#include "uci.h"
#include "volume_uci_config_api.h"

#include "cdx_log.h"

#define VOLUME_UCI_CONFIG_PATH "/etc/config/volume"

int volume_uci_set_config(const char *name, const char *value)
{
    logd("volume_uci_set_config %s: %s", name, value);

    struct uci_context *ctx = uci_alloc_context();
    if(!ctx) return -1;

    struct uci_package *pkg = NULL;

    if(UCI_OK != uci_load(ctx, VOLUME_UCI_CONFIG_PATH, &pkg))
    {
        if(pkg)
        {
            uci_unload(ctx, pkg);
        }
        uci_free_context(ctx);
        return -1;
    }

    struct uci_ptr ptr = {
        .package = "volume",
        .section = "global",
        .option = name,
        .value = value,
    };

    if(UCI_OK != uci_set(ctx, &ptr))
    {
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
        return -1;
    }

    if(UCI_OK != uci_commit(ctx, &ptr.p, true))
    {
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
        return -1;
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    logd("volume_uci_set_config done");

    return 0;
}

int volume_uci_get_config(const char *name, char *value, size_t value_max_len)
{
    //logv("volume_uci_get_config %s", name);

    struct uci_context *ctx = uci_alloc_context();
    if(!ctx) return -1;

    struct uci_package *pkg = NULL;

    if(UCI_OK != uci_load(ctx, VOLUME_UCI_CONFIG_PATH, &pkg))
    {
        if(pkg)
        {
            uci_unload(ctx, pkg);
        }
        uci_free_context(ctx);
        return -1;
    }

    struct uci_section *section = uci_lookup_section(ctx, pkg, "global");
    if(!section || strcmp(section->type, "setting") != 0)
    {
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
        return -1;
    }

    const char *content = uci_lookup_option_string(ctx, section, name);
    if(!content || value_max_len <= strlen(content))
    {
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
        return -1;
    }
    else
    {
        memcpy(value, content, strlen(content));
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    //logv("volume_uci_get_config done: %s", value);

    return 0;
}
