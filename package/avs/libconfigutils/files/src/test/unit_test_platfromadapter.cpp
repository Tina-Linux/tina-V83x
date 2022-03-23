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
#include "platformadapter/PlatformAdapter.h"

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
    const char *file = nullptr;
    const char *out_file = nullptr;
    WavUtils wav_utils;
    WavUtils wav_utils1;
    WavUtils wav_utils2;
    if(argc == 3){
        file = argv[1];
    out_file = argv[2];
    }else{
        printf("missing config file!\n");
        file = "/etc/avs/config.json";
        out_file = "/tmp/a.wav";
        //return 1;
    }

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    auto platformadapter = PlatformAdapter::create(file);

    auto recorder = platformadapter->getRecorder();

    recorder->init();
    recorder->start();
    char file_name[100];
    snprintf(file_name, 100, "%s-%d", out_file, 0);
    wav_utils.create(file_name, "wb", 16, 1, 16000);
    snprintf(file_name, 100, "%s-%d", out_file, 1);
    wav_utils1.create(file_name, "wb", 16, 1, 16000);
    snprintf(file_name, 100, "%s-%d", out_file, 2);
    wav_utils2.create(file_name, "wb", 16, 1, 16000);

    while(!shound_stop) {
        char *data;
        int samples;
#if 0
        auto convertor = recorder->fetch(1000);
        if(convertor == nullptr) {
            printf("fetch error!\n");
            break;
        }
#else
        std::shared_ptr<ConvertorInterface> convertor = nullptr;
        samples = recorder->fetch(convertor, 1000);
        if(samples <= 0) {
            printf("fetch error\n");
            if(samples == -EPIPE) printf("EPIPE error\n");
        }
        if(convertor == nullptr)
            printf("convertor is nullptr\n");
#endif
        samples = convertor->getChannelData(0, &data);
        wav_utils.write(data, samples);
        samples = convertor->getChannelData(1, &data);
        wav_utils1.write(data, samples);
        samples = convertor->getChannelData(2, &data);
        wav_utils2.write(data, samples);
    }

    wav_utils.release();
    wav_utils1.release();
    wav_utils2.release();

    recorder->stop();
    recorder->release();
}
