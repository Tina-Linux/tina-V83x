#include <stdio.h>
#include <sysexits.h>
#include <sys/time.h>
#include "audiotest.h"

static void print_help_msg()
{
    printf("USAGE:\n");
    printf("\taudiotest_wav_header <wav_file>\n");
    printf("DESCRIPTION:\n");
    printf("\tparse and print WAVE file header\n");
    printf("EXIT CODE:\n");
    printf("\t0         : success\n");
    printf("\t66        : cannot open input file\n");
    printf("\totherwise : error\n");
}

int main(int argc, char *argv[])
{
    int ret;
    if (argc <= 1) {
        print_help_msg();
        ret = EXIT_FAILURE;
        goto out;
    }

    char *wav_file = argv[1];
    FILE *fp = fopen(wav_file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open \"%s\"\n", wav_file);
        ret = EX_NOINPUT;
        goto out;
    }

    struct audiotest_wav_pcm_header header;
    if (0 != audiotest_wav_pcm_header_read(&header, fp)) {
        ret = EXIT_FAILURE;
        goto close_file;
    }
    audiotest_wav_pcm_header_print(&header);

    ret = EXIT_SUCCESS;

close_file:
    fclose(fp);
out:
    return ret;
}
