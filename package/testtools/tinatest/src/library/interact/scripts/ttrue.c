#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "task.h"
#include "interact.h"

int main(int argc, char **argv)
{
    char ask[MAX_TEXT] = {0};
    int num = 1, len = 0;
    char *p = ask;
    while (num < argc && (MAX_TEXT - 1 - (p - ask)) >= (len = strlen(argv[num]))) {
        if (!strncmp(argv[num], "-n", 2)) {
            if (!strncmp(argv[num - 1], "-n", 2)) {
                *p++ = '\n';
            } else {
                *(p - 1) = '\n';
            }
        } else {
            strncpy(p, argv[num], len);
            p += len;
            *p++ = ' ';
        }
        num++;
    }
    ask[p - ask - 1] = '\0';

    if (ttrue(ask) == true) {
        printf("%s\n", STR_TRUE);
        return 0;
    } else {
        printf("%s\n", STR_FALSE);
        return 1;
    }
}
