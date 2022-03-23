/*
 * AlsaMicrophoneWrapper.cpp
 *
 * Copyright (c) 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "SampleApp/AlsaMicrophoneWrapper.h"
#include "SampleApp/ConsolePrinter.h"

namespace alexaClientSDK {
namespace sampleApp {

using avsCommon::avs::AudioInputStream;

static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK = 512;

std::shared_ptr<AlsaMicrophoneWrapper> AlsaMicrophoneWrapper::create(
        std::shared_ptr<avsCommon::avs::AudioInputStream> wakeword_stream,
        std::shared_ptr<avsCommon::avs::AudioInputStream> esp_stream,
        std::shared_ptr<AW::PlatformAdapter> platformadapter) {
    if (!wakeword_stream) {
        ConsolePrinter::simplePrint("Invalid stream passed to AlsaMicrophoneWrapper");
        return nullptr;
    }

    std::shared_ptr<AlsaMicrophoneWrapper> alsaMicrophoneWrapper(
        new AlsaMicrophoneWrapper(wakeword_stream, esp_stream, platformadapter)
    );
    if (!alsaMicrophoneWrapper->initialize()) {
        ConsolePrinter::simplePrint("Failed to initialize AlsaMicrophoneWrapper");
        return nullptr;
    }
    return alsaMicrophoneWrapper;
}

AlsaMicrophoneWrapper::AlsaMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> wakeword_stream,
                                             std::shared_ptr<avsCommon::avs::AudioInputStream> esp_stream,
                                             std::shared_ptr<AW::PlatformAdapter> platformadapter) :
        m_audioInputStream{wakeword_stream}, m_espInputStream{esp_stream}, m_platformadapter{platformadapter} {}

AlsaMicrophoneWrapper::~AlsaMicrophoneWrapper() {
    if(m_loop_back != nullptr)
        m_loop_back->release();
}

bool AlsaMicrophoneWrapper::initialize() {
    m_writer = m_audioInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        ConsolePrinter::simplePrint("Failed to create stream writer");
        return false;
    }

    if(m_espInputStream) {
        m_esp_writer = m_espInputStream->createWriter(AudioInputStream::Writer::Policy::NONBLOCKABLE);
        if (!m_esp_writer) {
            ConsolePrinter::simplePrint("Failed to create stream writer");
            return false;
        }
    }

    m_recorder = m_platformadapter->getRecorder();
    if(!m_recorder) return false;

    if(m_platformadapter->getVoiceDataLoopback() == true) {
        m_loop_back = std::unique_ptr<AW::AlsaUtils>(new AW::AlsaUtils(AW::AlsaUtils::Type::PLAYBACK));
        if(m_loop_back->init("hw:Loopback,0,0", 16000, 1, 16, 1024, 4) < 0) return false;
    }

    m_captureStatus = STOPPED;
    return true;
}

bool AlsaMicrophoneWrapper::startStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
    m_isStop = false;

    if(m_captureStatus == STOPPED)
        m_captureThread = std::thread(&AlsaMicrophoneWrapper::captureLoop, this);

    return true;
}

bool AlsaMicrophoneWrapper::stopStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
    ConsolePrinter::simplePrint("stopStreamingMicrophoneData start");
    m_isStop = true;

    if (m_captureThread.joinable()) {
        m_captureThread.join();
    }

    ConsolePrinter::simplePrint("stopStreamingMicrophoneData end");
    return true;
}

void AlsaMicrophoneWrapper::captureLoop() {
    ConsolePrinter::simplePrint("captureLoop start");
    m_captureStatus = STARTED;

    if(m_loop_back != nullptr) m_loop_back->start();

    m_recorder->start();
    std::shared_ptr<AW::ConvertorInterface> convertor = nullptr;

    while(!m_isStop) {
        char *data;
        int ret = m_recorder->fetch(convertor, 0);
        if(ret == -EPIPE) continue;
        else if(ret < 0){
            break;
        } else if(ret == 0) {
            break;
        }

        ret = convertor->getChannelData(0, &data);
        ret = m_writer->write(data, ret);
        if (ret <= 0) {
            ConsolePrinter::simplePrint("Failed to write to wakeword stream.");
            exit(-1);
        }

        if(m_esp_writer) {
            ret = convertor->getChannelData(2, &data);
            ret = m_esp_writer->write(data, ret);
            if (ret <= 0) {
                ConsolePrinter::simplePrint("Failed to write to esp stream.");
                exit(-1);
            }
        }

        if(m_loop_back != nullptr) {
            ret = convertor->getChannelData(1, &data);
            ret = m_loop_back->play(data, ret);
            if (ret < 0) {
                ConsolePrinter::simplePrint("Failed to write to loopback stream.");
                exit(-1);
            }
        }
    }

    m_recorder->stop();
    if(m_loop_back != nullptr) m_loop_back->stop();

    m_captureStatus = STOPPED;

    ConsolePrinter::simplePrint("captureLoop stop");
}

} // namespace sampleApp
} // namespace alexaClientSDK
