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
#include "recorder/provider/AudioCodecProvider.h"
#include "recorder/filter/TutuClearFilter.h"
#include "recorder/convertor/InterleavedConvertor.h"

static void usage(char *command)
{
    printf("Usage: %s FILE...\n", command);
}

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

using namespace AW;

int main(int argc, char *argv[])
{

    int option_index, c;
    int ret;

    int channels = -1;
    int sample_bits = -1;
    int rate = 16000;

    char *out_file = nullptr;

    WavUtils wav_utils;

    if(argc == 2){
        out_file = argv[1];
    }else{
        printf("missing record file!\n");
        return 1;
    }

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    struct json_object *ac108provider = AW::JsonUtils::json_object_new_object();
    if (!is_error(ac108provider)) {
        AW::JsonUtils::json_object_object_add(ac108provider, "mode", AW::JsonUtils::json_object_new_string("normal"));
        AW::JsonUtils::json_object_object_add(ac108provider, "name", AW::JsonUtils::json_object_new_string("ac108"));
        AW::JsonUtils::json_object_object_add(ac108provider, "device", AW::JsonUtils::json_object_new_string("hw:1,0"));
        AW::JsonUtils::json_object_object_add(ac108provider, "channels", AW::JsonUtils::json_object_new_int(8));
        AW::JsonUtils::json_object_object_add(ac108provider, "period-size", AW::JsonUtils::json_object_new_int(1024));
        AW::JsonUtils::json_object_object_add(ac108provider, "period", AW::JsonUtils::json_object_new_int(4));
        AW::JsonUtils::json_object_object_add(ac108provider, "sample-rate", AW::JsonUtils::json_object_new_int(16000));
        AW::JsonUtils::json_object_object_add(ac108provider, "sample-bits", AW::JsonUtils::json_object_new_int(24));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(ac108provider, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }
    struct json_object *audiocodecprovider = AW::JsonUtils::json_object_new_object();
    if (!is_error(audiocodecprovider)) {
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "name", AW::JsonUtils::json_object_new_string("audiocodec"));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "device", AW::JsonUtils::json_object_new_string("hw:0,0"));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "channels", AW::JsonUtils::json_object_new_int(2));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "period-size", AW::JsonUtils::json_object_new_int(1024));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "period", AW::JsonUtils::json_object_new_int(4));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "sample-rate", AW::JsonUtils::json_object_new_int(48000));
        AW::JsonUtils::json_object_object_add(audiocodecprovider, "sample-bits", AW::JsonUtils::json_object_new_int(16));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(audiocodecprovider, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }

    struct json_object *tutuclear = AW::JsonUtils::json_object_new_object();
    if (!is_error(tutuclear)) {
        AW::JsonUtils::json_object_object_add(tutuclear, "prm-file", AW::JsonUtils::json_object_new_string("/etc/avs/tutuClearA1_ns4wakeup_stereo.prm"));
        AW::JsonUtils::json_object_object_add(tutuclear, "filter-block", AW::JsonUtils::json_object_new_int(160));
        AW::JsonUtils::json_object_object_add(tutuclear, "input-sample-rate", AW::JsonUtils::json_object_new_int(16000));
        AW::JsonUtils::json_object_object_add(tutuclear, "input-sample-bits", AW::JsonUtils::json_object_new_int(32));
        AW::JsonUtils::json_object_object_add(tutuclear, "input-channles", AW::JsonUtils::json_object_new_int(5));
        AW::JsonUtils::json_object_object_add(tutuclear, "output-sample-rate", AW::JsonUtils::json_object_new_int(16000));
        AW::JsonUtils::json_object_object_add(tutuclear, "output-sample-bits", AW::JsonUtils::json_object_new_int(32));
        AW::JsonUtils::json_object_object_add(tutuclear, "output-channles", AW::JsonUtils::json_object_new_int(3));
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-0", AW::JsonUtils::json_object_new_string("ac108-1"));
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-1", AW::JsonUtils::json_object_new_string("ac108-5"));
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-2", AW::JsonUtils::json_object_new_string("ac108-7"));
        AW::JsonUtils::json_object_object_add(tutuclear, "reference-channel-0", AW::JsonUtils::json_object_new_string("audiocodec-0"));
        AW::JsonUtils::json_object_object_add(tutuclear, "reference-channel-1", AW::JsonUtils::json_object_new_string("audiocodec-1"));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(tutuclear, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }

    auto filter = TutuClearFilter::create();
    if(filter->init(tutuclear) > 0) {
        printf("filter init failed\n");
        exit(-1);
    }

    auto provider1 = AC108Provider::create();
    if(provider1->init(ac108provider) < 0) {
        printf("recorder init failed\n");
        exit(-1);
    }
    auto provider2 = AudioCodecProvider::create();
    if(provider2->init(audiocodecprovider) < 0) {
        printf("recorder init failed\n");
        exit(-1);
    }

    int recorder_sample_bits1 = provider1->getSampleBits();
    if(recorder_sample_bits1 == 24) recorder_sample_bits1 = 32;

    int recorder_sample_bits2 = provider2->getSampleBits();
    if(recorder_sample_bits2 == 24) recorder_sample_bits2 = 32;

    auto provider1_convertor = InterleavedConvertor::create(filter->getFilterBlock(),
                                                          provider1->getSampleRate(),
                                                          provider1->getChannels(),
                                                          recorder_sample_bits1,
                                                          filter->getInputSampleRate(),
                                                          filter->getInputSampleBits());
    auto provider2_convertor = InterleavedConvertor::create(filter->getFilterBlock(),
                                                          provider2->getSampleRate(),
                                                          provider2->getChannels(),
                                                          recorder_sample_bits2,
                                                          filter->getInputSampleRate(),
                                                          filter->getInputSampleBits());

    auto output_convertor = InterleavedConvertor::create(filter->getFilterBlock(),
                                                       filter->getOutputSampleRate(),
                                                       filter->getOutputChannels(),
                                                       filter->getOutputSampleBits(),
                                                       filter->getOutputSampleRate(),
                                                       filter->getOutputSampleBits()/2);

    if(filter->setProvider(provider1, provider1_convertor) != 0) return -1;
    if(filter->setProvider(provider2, provider2_convertor) != 0) return -1;

    wav_utils.create(out_file, "wb", filter->getOutputSampleBits()/2, filter->getOutputChannels(), filter->getOutputSampleRate());

    filter->start();

    while(!shound_stop) {
        char *data;
        int samples = filter->fetch(output_convertor, filter->getFilterBlock());
        output_convertor->getChannelData(0, &data);
        wav_utils.write(data, samples);
    }

    wav_utils.release();
    printf("%s %d\n", __func__,__LINE__);
    filter->stop();
    filter->release();

    printf("%s %d\n", __func__,__LINE__);
    AW::JsonUtils::json_object_put(ac108provider);
    AW::JsonUtils::json_object_put(audiocodecprovider);
    AW::JsonUtils::json_object_put(tutuclear);
    printf("%s %d\n", __func__,__LINE__);
}
