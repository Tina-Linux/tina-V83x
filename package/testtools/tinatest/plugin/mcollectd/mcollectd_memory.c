#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "memory"

static struct {
    bool report_to_percentage;
    bool report_to_absolute;
}module;

int module_make_conf(const char *collectd_path, FILE *fp)
{
    int len = 10 * strlen(collectd_path), ret = -1;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed\n", MODULE_NAME);
        goto out;
    }

    //report_to_absolute
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_REPORT_TO_PERCENTAGE);
    module.report_to_percentage = mjson_fetch_boolean(buf);

    //report_to_percentage
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_REPORT_TO_ABSOLUTE);
    module.report_to_absolute = mjson_fetch_boolean(buf);

    fprintf(fp, "LoadPlugin %s\n", MODULE_NAME);
    fprintf(fp, "<Plugin %s>\n", MODULE_NAME);
    fprintf(fp, "\tValuesPercentage %s\n",
            module.report_to_percentage == true ? "true" : "false");
    fprintf(fp, "\tValuesAbsolute %s\n",
            module.report_to_absolute == true ? "true" : "false");
    fprintf(fp, "</Plugin>\n");

    ret = 0;
out:
    free(buf);
    return ret;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
