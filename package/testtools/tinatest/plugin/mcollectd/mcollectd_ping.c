#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "ping"

static struct {
    char *host;
    char *send_interval_sec;
    char *timeout_sec;
    int max_ttl;
}module;

static int module_scan_json(const char *collectd_path)
{
    int len = 10 * strlen(collectd_path), ret = -1;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed\n", MODULE_NAME);
        goto out;
    }

    // host
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_PING_HOST);
    module.host = mjson_fetch_string(buf);

    // send_interval_sec
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_PING_SEND_INTERVAL_SEC);
    module.send_interval_sec = mjson_fetch_string(buf);

    // timeout_sec
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_PING_TIMEOUT_SEC);
    module.timeout_sec = mjson_fetch_string(buf);

    // max_ttl
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_PING_MAX_TTL);
    module.max_ttl = mjson_fetch_int(buf);

    ret = 0;
out:
    free(buf);
    return ret;
}

int module_make_conf(const char *collectd_path, FILE *fp)
{
    DEBUG(BASE, "make config for collectd_%s\n", MODULE_NAME);
    if (module_scan_json(collectd_path) < 0) {
        ERROR("%s: scan json failed\n", MODULE_NAME);
        return -1;
    }

    fprintf(fp, "LoadPlugin %s\n", MODULE_NAME);
    fprintf(fp, "<Plugin %s>\n", MODULE_NAME);
    if (module.host != NULL)
        fprintf(fp, "\tHost \"%s\"\n", module.host);
    if (module.send_interval_sec != NULL)
        fprintf(fp, "\tInterval %s\n", module.send_interval_sec);
    if (module.timeout_sec != NULL)
        fprintf(fp, "\tTimeout %s\n", module.send_interval_sec);
    if (module.max_ttl >= 0)
        fprintf(fp, "\tTTL %d\n", module.max_ttl);
    fprintf(fp, "</Plugin>\n");

    return 0;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
