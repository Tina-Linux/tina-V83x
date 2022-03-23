#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "df"

static struct {
    bool report_to_percentage;
    bool report_to_absolute;
    struct {
        char *sellect_or_ignore;
        char *device;
        char *mountpoint;
        char *fstype;
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

    // report_to_absolute
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_REPORT_TO_ABSOLUTE);
    module.report_to_absolute = mjson_fetch_boolean(buf);

    // report_to_percentage
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_REPORT_TO_PERCENTAGE);
    module.report_to_percentage = mjson_fetch_boolean(buf);

    // match-sellect_or_ignore
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_SELLECT_OR_IGNORE);
    module.match.sellect_or_ignore = mjson_fetch_string(buf);

    // match-device
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_DF_MATCH_DEVICE);
    module.match.device = mjson_fetch_string(buf);

    // match-mountpoint
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_DF_MATCH_MOUNTPOINT);
    module.match.mountpoint = mjson_fetch_string(buf);

    // match-fstype
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_DF_MATCH_FSTYPE);
    module.match.fstype = mjson_fetch_string(buf);

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
    fprintf(fp, "\tValuesPercentage %s\n",
            module.report_to_percentage == true ? "true" : "false");
    fprintf(fp, "\tValuesAbsolute %s\n",
            module.report_to_absolute == true ? "true" : "false");
    if (module.match.sellect_or_ignore != NULL)
        fprintf(fp, "\tIgnoreSelected %s\n",
                strcmp(module.match.sellect_or_ignore, "sellect") != 0 ?
                "true" : "false");
    if (module.match.device != NULL &&
            strcmp(module.match.device, SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\tDevice %s\n", module.match.device);
    if (module.match.device != NULL &&
            strcmp(module.match.mountpoint, SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\tMountPoint %s\n", module.match.mountpoint);
    if (module.match.device != NULL &&
            strcmp(module.match.fstype, SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\tFSType %s\n", module.match.fstype);
    fprintf(fp, "</Plugin>\n");

    return 0;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
