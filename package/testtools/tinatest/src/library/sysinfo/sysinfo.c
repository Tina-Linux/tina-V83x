#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SYSINFO "/usr/bin/sysinfo"
#define MAX_LEN 68

typedef enum {
    kernel_version = 0,
    boot_media,
    target,
}type;

static char *run(type t) {
    char *buf = NULL;
    FILE *fp = NULL;

    buf = calloc(1, MAX_LEN);
    if (buf == NULL)
        goto err;
    switch (t) {
        case kernel_version:
            fp = popen(SYSINFO " -k", "r");
            break;
        case boot_media:
            fp = popen(SYSINFO " -b", "r");
            break;
        case target:
            fp = popen(SYSINFO " -t", "r");
            break;
        default:
            goto err;
    }
    if (fp == NULL)
        goto err;

    if (fread(buf, 1, MAX_LEN, fp) <= 0)
        goto err;

    pclose(fp);
    return buf;
err:
    free(buf);
    pclose(fp);
    return NULL;
}

char *get_kernel_version()
{
    return run(kernel_version);
}

char *get_boot_media()
{
    return run(boot_media);
}

char *get_target()
{
    return run(target);
}
