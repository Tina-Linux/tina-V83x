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
#include "recorder/provider/FileProvider.h"
#include "recorder/convertor/InterleavedConvertor.h"

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
    int ret;

    int channels = -1;
    int sample_bits = -1;
    int rate = 16000;

    char *in_file = nullptr;
    char *out_file = nullptr;

    if(argc == 3){
        in_file = argv[1];
        out_file = argv[2];
    }else{
        printf("missing record file!\n");
        return 1;
    }

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    struct json_object *fileprovider = AW::JsonUtils::json_object_new_object();
    if (!is_error(fileprovider)) {
        AW::JsonUtils::json_object_object_add(fileprovider, "format", AW::JsonUtils::json_object_new_string("wav"));
        AW::JsonUtils::json_object_object_add(fileprovider, "name", AW::JsonUtils::json_object_new_string("file"));
        AW::JsonUtils::json_object_object_add(fileprovider, "path", AW::JsonUtils::json_object_new_string(in_file));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(fileprovider, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }

    auto recorder = FileProvider::create();

    recorder->init(fileprovider);

    int recorder_sample_bits = recorder->getSampleBits();
    if(recorder_sample_bits == 24) recorder_sample_bits = 32;

    WavUtils * wav_utils = new WavUtils[recorder->getChannels()];
    for(int channel = 0; channel < recorder->getChannels(); ++channel) {
        int len = strlen(out_file);
        char format[1024];
        char channel_file[1024];
        bzero(format, 1024);
        bzero(channel_file, 1024);
        memcpy(format, out_file, len);
        strcpy(format+len, ".%d");
        sprintf(channel_file, format, channel);
        wav_utils[channel].create(channel_file, "wb", recorder->getSampleBits(), 1, recorder->getSampleRate());
    }
    int block =160;
    auto convertor = InterleavedConvertor::create(block,
                                           recorder->getSampleRate(),
                                           recorder->getChannels(),
                                           recorder->getSampleBits(),
                                           recorder->getSampleRate(),
                                           recorder->getSampleBits());
    if(recorder->getChannels() == 8){
        convertor->setChannelMap(1,0);
        convertor->setChannelMap(5,1);
        convertor->setChannelMap(7,2);
        convertor->setChannelMap(2,3);
        convertor->setChannelMap(3,4);
    }
    recorder->start();

    while(!shound_stop) {
        char *data;
        int samples = recorder->fetch(convertor, block);
        if(samples <= 0) break;

        for(int channel = 0; channel < (recorder->getChannels() == 8 ? 5 : recorder->getChannels()); ++channel) {
            if(convertor->getChannelData(channel, &data) > 0)
                wav_utils[channel].write(data, samples);
        }
    }
    for(int channel = 0; channel < recorder->getChannels(); ++channel) {
        wav_utils[channel].release();
    }
    printf("%s %d\n", __func__,__LINE__);
    delete [] wav_utils;
    printf("%s %d\n", __func__,__LINE__);
    recorder->stop();
    recorder->release();
    printf("%s %d\n", __func__,__LINE__);
    AW::JsonUtils::json_object_put(fileprovider);
    printf("%s %d\n", __func__,__LINE__);
}
