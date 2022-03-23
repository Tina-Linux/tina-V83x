#include <stdio.h>
#include <sysexits.h>
#include <getopt.h>
#include <sys/time.h>
#include "audiotest.h"

static void print_help_msg()
{
    printf("USAGE:\n");
    printf("\taudiotest_wav_play_time [OPTIONS] <wav_file>\n");
    printf("DESCRIPTION:\n");
    printf("\ttime how long it takes to play a PCM wave file in several times\n");
    printf("OPTIONS:\n");
    printf("\t-h,--help             : show help message\n");
    printf("\t-D,--device NAME      : select PCM by name"
            "(default is \"default\")\n");
    printf("\t-t,--times COUNT      : times the PCM wave file will be played "
            "(default is 1)\n");
    printf("\t-U,--microsecond      : print result in microsecond "
            "(otherwise in second)\n");
    printf("EXIT CODE:\n");
    printf("\t0         : success\n");
    printf("\t66        : cannot open input file\n");
    printf("\totherwise : error\n");
    printf("RESULT:\n");
    printf("\ttime (in second or microsecond) used in playing the PCM wave "
            "file will be output to stdout\n");
}

int main(int argc, char *argv[])
{
    int ret;
    int opt;
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"device", required_argument, NULL, 'D'},
        {"times", required_argument, NULL, 't'},
        {"microsecond", required_argument, NULL, 'U'},
    };
    struct audiotest_pcm_config config = {
        .pcm_name = "default",
        .channels = 2,
        .rate = 48000,
        .bits = 16,
        .format = SND_PCM_FORMAT_S16_LE,
        .frames_per_period = 1024,
        .frames_per_buffer = 4096,
    };
    int repeat_times = 1;
    int is_usec = 0;

    if (argc <= 1) {
        print_help_msg();
        ret = EXIT_FAILURE;
        goto out;
    }

    while ((opt = getopt_long(argc, argv, "hD:t:U", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'h':
            print_help_msg();
            ret = EXIT_SUCCESS;
            goto out;
        case 'D':
            config.pcm_name = optarg;
            break;
        case 't':
            repeat_times = atoi(optarg);
            break;
        case 'U':
            is_usec = 1;
            break;
        default:
            fprintf(stderr, "Invalid option: -%c\n", (char)opt);
            print_help_msg();
            ret = EXIT_FAILURE;
            goto out;
        }
    }

    if (optind < argc - 1) {
        fprintf(stderr, "Lack of the WAVE file\n");
        ret = EX_NOINPUT;
        goto out;
    }
    char *wav_file = argv[optind];
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

    if (0 != audiotest_update_pcm_config_from_wav_header(&header, &config)) {
        ret = EXIT_FAILURE;
        goto close_file;
    }

    struct timeval time_start;
    struct timeval time_end;
    gettimeofday(&time_start, NULL);
    if (0 != audiotest_play_pcm_file_repeatedly(&config, repeat_times, fp)) {
        ret = EXIT_FAILURE;
        goto close_file;
    }
    gettimeofday(&time_end, NULL);
    long time_used = 0;
    if (is_usec) {
        time_used = 1000000 * (time_end.tv_sec - time_start.tv_sec)
            + time_end.tv_usec - time_start.tv_usec;
    } else {
        time_used = time_end.tv_sec - time_start.tv_sec;
    }

    fprintf(stdout, "%ld\n", time_used);

    ret = EXIT_SUCCESS;

close_file:
    fclose(fp);
out:
    return ret;
}
