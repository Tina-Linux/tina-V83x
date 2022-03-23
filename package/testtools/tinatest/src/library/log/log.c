#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "fileops.h"

FILE *tty;
void init_log(void) {
    char link[100];
    if (get_fstd(STDOUT_FILENO, link, 100) < 0)
        goto def;

    if (strncmp(link, "/dev/null", sizeof("/dev/null")))
        goto def;

    if (get_tty(link, 100) < 0)
        goto def;

    if (access(link, F_OK))
        goto def;

    tty = fopen(link, "w");
    if (tty == NULL)
        goto def;

    return;
def:
    tty = stderr;
    return;
}
