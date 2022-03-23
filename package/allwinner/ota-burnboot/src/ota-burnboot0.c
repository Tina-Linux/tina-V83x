#include <stdio.h>
#include <unistd.h>
#include "OTA_BurnBoot.h"

void show_help()
{
    printf("Usage: ota-burnboot0 <boot0-image>\n");
}

int main(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h':
            show_help();
            return 0;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1 || access(argv[0], R_OK)) {
        show_help();
        return 1;
    }

    if (OTA_burnboot0(argv[0]) < 0) {
        fprintf(stderr, "Burn Boot0 Failed\n");
        return -1;
    } else {
        fprintf(stdout, "Burn Boot0 Success\n");
        return 0;
    }
}
