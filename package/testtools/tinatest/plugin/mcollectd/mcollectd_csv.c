#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "csv"

static struct {
    char *outdir;
}module;

int module_make_conf(const char *collectd_path, FILE *fp)
{
    int len = 10 * strlen(collectd_path), ret = -1;
    char *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed\n", MODULE_NAME);
        goto out;
    }

    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_OUTDIR);
    module.outdir = mjson_fetch_string(buf);

    if (module.outdir == NULL)
        module.outdir = DEFAULT_GLOBAL_OUTPATH;
    if (access(module.outdir, F_OK) == 0)
        remove(module.outdir); //incase it's a file
    mkdir(module.outdir, 0776);

    fprintf(fp, "LoadPlugin %s\n", MODULE_NAME);
    fprintf(fp, "<Plugin %s>\n", MODULE_NAME);
    fprintf(fp, "\tDataDir \"%s\"\n", module.outdir);
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
