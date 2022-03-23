#include <gtest/gtest.h>

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <endian.h>
#include <signal.h>
#include <getopt.h>

#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include "utils/JsonUtils.h"
#include "utils/WavUtils.h"
#include "recorder/provider/AC108Provider.h"


static bool shound_stop = false;

static void when_signal(int sig)
{
    switch(sig){
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        {
        printf("signal coming, stop the capture\n");
            shound_stop = true;
            break;
        }
        case SIGPIPE:
        {
            //When the client is closed after start scaning and parsing,
            //this signal will come, ignore it!
            printf("do nothings for PIPE signal\n");
            break;
        }
    }
}

static void usage(char *command)
{
    printf("Usage: %s [OPTION]... [FILE]...\n"
"\n"
"-h, --help              help\n"
"-D, --device=NAME       select PCM by name\n"
"-c, --channels=#        channels\n"
"-f, --format=FORMAT     sample format (case insensitive)\n"
"-r, --rate=#            sample rate\n"
        , command);
}

using namespace AW;

int main(int argc, char *argv[])
{

    int option_index, c;

    char *card = nullptr;
    int channels = -1;
    int sample_bits = -1;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    int rate = 16000;

    char *file = nullptr;

    char ac108_mode[100];
    bzero(ac108_mode, 100);

    std::shared_ptr<ProviderInterface> recorder;
    WavUtils wav_utils;

    const char short_options[] = "hD:c:f:r:en";
    const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"device", 1, 0, 'D'},
        {"channels", 1, 0, 'c'},
        {"format", 1, 0, 'f'},
        {"rate", 1, 0, 'r'},
        {"encode", 0, 0, 'e'},
        {"normal", 0, 0, 'n'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'D':
            card = optarg;
            break;
        case 'c':
            channels = atoi(optarg);
            break;
        case 'f':
            format = snd_pcm_format_value(optarg);
            break;
        case 'r':
            rate = strtol(optarg, NULL, 0);
            break;
        case 'e':
            strncpy(ac108_mode, "encode", 6);
            break;
        case 'n':
            strncpy(ac108_mode, "normal", 6);
            break;
        default:
            printf("Try `%s --help' for more information.\n", argv[0]);
            return 1;
        }
    }

    if(argc == optind+1)
        file = argv[optind];
    else{
        printf("missing record file!\n");
        return 1;
    }


    if (channels < 1 || channels > 256) {
        printf("value %i for channels is invalid\n", channels);
        return 1;
    }

    if (format == SND_PCM_FORMAT_UNKNOWN) {
        printf("wrong extended format '%s'\n", optarg);
        return 1;
    }else if(format == SND_PCM_FORMAT_S8) sample_bits = 8;
    else if(format == SND_PCM_FORMAT_S16_LE) sample_bits = 16;
    else if(format == SND_PCM_FORMAT_S24_LE) sample_bits = 24;
    else if(format == SND_PCM_FORMAT_S32_LE) sample_bits = 32;

    if (rate < 300) rate *= 1000;
    if (rate < 2000 || rate > 192000) {
        printf("bad speed value %i\n", rate);
        return 1;
    }

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    struct json_object *ac108 = AW::JsonUtils::json_object_new_object();
    if (!is_error(ac108)) {
        AW::JsonUtils::json_object_object_add(ac108, "mode", AW::JsonUtils::json_object_new_string(ac108_mode));
        AW::JsonUtils::json_object_object_add(ac108, "name", AW::JsonUtils::json_object_new_string("ac108"));
        AW::JsonUtils::json_object_object_add(ac108, "device", AW::JsonUtils::json_object_new_string(card));
        AW::JsonUtils::json_object_object_add(ac108, "channels", AW::JsonUtils::json_object_new_int(channels));
        AW::JsonUtils::json_object_object_add(ac108, "period-size", AW::JsonUtils::json_object_new_int(1024));
        AW::JsonUtils::json_object_object_add(ac108, "period", AW::JsonUtils::json_object_new_int(4));
        AW::JsonUtils::json_object_object_add(ac108, "sample-rate", AW::JsonUtils::json_object_new_int(rate));
        AW::JsonUtils::json_object_object_add(ac108, "sample-bits", AW::JsonUtils::json_object_new_int(sample_bits));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(ac108, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }

    recorder = AC108Provider::create();

    recorder->init(ac108);

    int out_sample_bits = recorder->getSampleBits();
    if(out_sample_bits == 24) out_sample_bits = 32;

    wav_utils.create(file, "wb", out_sample_bits, recorder->getChannels(), recorder->getSampleRate());

    recorder->start();

    while(!shound_stop) {
        char *data;
        if(recorder->getFetchMode() == ProviderInterface::FetchMode::FETCH_GET_MEMORY) {
            int samples = recorder->fetch(&data, 160);
            wav_utils.write(data, samples);
        }
    }
    wav_utils.release();

    recorder->stop();
    recorder->release();

    AW::JsonUtils::json_object_put(ac108);
}
