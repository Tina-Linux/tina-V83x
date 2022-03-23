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
#include "recorder/filter/TutuClearFilter.h"
#include "recorder/convertor/InterleavedConvertor.h"

static void usage(char *command)
{
    printf("Usage: %s FILE...\n", command);
}

using namespace AW;

int main(int argc, char *argv[])
{

    int option_index, c;
    int ret;

    int channels = -1;
    int sample_bits = -1;
    int rate = 16000;

    char *in_file1 = nullptr;
    char *in_file2 = nullptr;
    char *out_file = nullptr;

    WavUtils wav_utils;

    if(argc == 4){
        in_file1 = argv[1];
        in_file2 = argv[2];
        out_file = argv[3];
    }else{
        printf("missing record file!\n");
        return 1;
    }

    struct json_object *fileprovider1 = AW::JsonUtils::json_object_new_object();
    if (!is_error(fileprovider1)) {
        AW::JsonUtils::json_object_object_add(fileprovider1, "format", AW::JsonUtils::json_object_new_string("wav"));
        AW::JsonUtils::json_object_object_add(fileprovider1, "name", AW::JsonUtils::json_object_new_string("file1"));
        AW::JsonUtils::json_object_object_add(fileprovider1, "path", AW::JsonUtils::json_object_new_string(in_file1));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(fileprovider1, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }
    struct json_object *fileprovider2 = AW::JsonUtils::json_object_new_object();
    if (!is_error(fileprovider2)) {
        AW::JsonUtils::json_object_object_add(fileprovider2, "format", AW::JsonUtils::json_object_new_string("wav"));
        AW::JsonUtils::json_object_object_add(fileprovider2, "name", AW::JsonUtils::json_object_new_string("file2"));
        AW::JsonUtils::json_object_object_add(fileprovider2, "path", AW::JsonUtils::json_object_new_string(in_file2));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(fileprovider2, JSON_C_TO_STRING_PRETTY);
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
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-0", AW::JsonUtils::json_object_new_string("file1-1"));
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-1", AW::JsonUtils::json_object_new_string("file1-5"));
        AW::JsonUtils::json_object_object_add(tutuclear, "signal-channel-2", AW::JsonUtils::json_object_new_string("file1-7"));
        AW::JsonUtils::json_object_object_add(tutuclear, "reference-channel-0", AW::JsonUtils::json_object_new_string("file2-0"));
        AW::JsonUtils::json_object_object_add(tutuclear, "reference-channel-1", AW::JsonUtils::json_object_new_string("file2-1"));

        const char *str = AW::JsonUtils::json_object_to_json_string_ext(tutuclear, JSON_C_TO_STRING_PRETTY);
        printf("str %s\n", str);
    }

    auto filter = TutuClearFilter::create();
    if(filter->init(tutuclear) > 0) {
        printf("filter init failed\n");
        exit(-1);
    }

    auto provider1 = FileProvider::create();
    if(provider1->init(fileprovider1) < 0) {
        printf("recorder init failed\n");
        exit(-1);
    }
    auto provider2 = FileProvider::create();
    if(provider2->init(fileprovider2) < 0) {
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

    while(1) {
        char *data;
        int samples = filter->fetch(output_convertor, filter->getFilterBlock());
        if(samples <= 0) break;
        output_convertor->getChannelData(0, &data);
        wav_utils.write(data, samples);
    }

    wav_utils.release();

    filter->stop();
    filter->release();

    AW::JsonUtils::json_object_put(fileprovider1);
    AW::JsonUtils::json_object_put(fileprovider2);
    AW::JsonUtils::json_object_put(tutuclear);
}
