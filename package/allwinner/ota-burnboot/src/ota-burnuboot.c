#include <stdio.h>
#include <unistd.h>
#include "OTA_BurnBoot.h"

void show_help()
{
    printf("Usage: ota-burnuboot <uboot-image>\n");
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

    if (OTA_burnuboot(argv[0]) < 0) {
        fprintf(stderr, "Burn Uboot Failed\n");
        return -1;
    } else {
        fprintf(stdout, "Burn Uboot Success\n");
        return 0;
    }
}
