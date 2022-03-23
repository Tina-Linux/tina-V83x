#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcollectd.h"
#include <mjson.h>

#define MODULE_NAME "filecount"

static struct {
    char *directory;
    bool include_hidden;
    bool include_subdir;
    struct {
        char *name;
        char *size;
        char *mtime;
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

    // directory
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_DIRECTORY);
    module.directory = mjson_fetch_string(buf);

    // include_hidden
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_INCLUDE_HIDDEN);
    module.include_hidden = mjson_fetch_boolean(buf);

    // include_subdir
    snprintf(buf, len, "%s/%s/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_INCLUDE_SUBDIR);
    module.include_subdir = mjson_fetch_boolean(buf);

    // match-name
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_NAME);
    module.match.name = mjson_fetch_string(buf);

    // match-size
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_SIZE);
    module.match.size = mjson_fetch_string(buf);

    // match-mtime
    snprintf(buf, len, "%s/%s/match/%s", collectd_path, MODULE_NAME,
            SYSKEY_GLOBAL_INFO_COLLECTD_FILECOUNT_MATCH_MTIME);
    module.match.mtime = mjson_fetch_string(buf);

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
    if (module.directory != NULL)
        fprintf(fp, "\t<Directory \"%s\">\n", module.directory);
    fprintf(fp, "\t\tIncludeHidden %s\n",
            module.include_hidden == true ? "true" : "false");
    fprintf(fp, "\t\tRecursive %s\n",
            module.include_subdir == true ? "true" : "false");
    if (module.match.name != NULL && strcmp(module.match.name,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tName \"%s\"\n", module.match.name);
    if (module.match.size != NULL && strcmp(module.match.size,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tSize %s\n", module.match.size);
    if (module.match.mtime != NULL && strcmp(module.match.mtime,
                SYSKEY_GLOBAL_INFO_COLLECTD_MATCH_ALL))
        fprintf(fp, "\t\tMTime %s\n", module.match.mtime);
    if (module.directory != NULL)
        fprintf(fp, "\t</Directory>\n");
    fprintf(fp, "</Plugin>\n");

    return 0;
}

void module_init(void)
{
    mcollectd_register(module_make_conf);
}
