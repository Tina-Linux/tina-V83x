#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "disk"

static struct {
    struct {
        char *sellect_or_ignore;
        char *disk_regular_expression;
    }match;
}module;

static int module_scan_json(const char *collectd_path)
{
    int len = 10 * strlen(collectd_path), ret = -1;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed\n", MODULE_NAME);
        goto out;
    }

    // match-sellect_or_ignore
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_SELLECT_OR_IGNORE);
    module.match.sellect_or_ignore = mjson_fetch_string(buf);

    // match-disk_regular_expression
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_DISK_MATCH_DISK);
    module.match.disk_regular_expression = mjson_fetch_string(buf);

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
    if (module.match.sellect_or_ignore != NULL)
        fprintf(fp, "\tIgnoreSelected %s\n",
                strcmp(module.match.sellect_or_ignore, "sellect") != 0 ?
                "true" : "false");
    if (module.match.disk_regular_expression != NULL &&
            strcmp(module.match.disk_regular_expression,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\tDisk \"/%s/\"\n", module.match.disk_regular_expression);
    fprintf(fp, "</Plugin>\n");

    return 0;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
