/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Copyright 2019 Allwinner Technology Co.,Ltd
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

#include "SampleApp/TconfigsMicrophoneWrapper.h"

#include <AVSCommon/Utils/Logger/Logger.h>
#include <tconfigs/audio/elements/common_sink.h>

namespace alexaClientSDK {
namespace sampleApp {

static const std::string TAG("TconfigsMicrophoneWrapper");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::unique_ptr<TconfigsMicrophoneWrapper> TconfigsMicrophoneWrapper::create(
        std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
        std::shared_ptr<TconfigsPlatformAdapter> adapter)
{
    if (!stream) {
        ACSDK_CRITICAL(LX("invalid parameter stream"));
        return nullptr;
    }
    if (!adapter) {
        ACSDK_CRITICAL(LX("invalid parameter adapter"));
        return nullptr;
    }

    std::unique_ptr<TconfigsMicrophoneWrapper> micWrapper(new TconfigsMicrophoneWrapper(stream));
    if (!micWrapper || !micWrapper->initialize(adapter)) {
        ACSDK_CRITICAL(LX("Failed to create TconfigsMicrophoneWrapper"));
        return nullptr;
    }
    return micWrapper;
}

TconfigsMicrophoneWrapper::TconfigsMicrophoneWrapper(
        std::shared_ptr<avsCommon::avs::AudioInputStream> stream)
    : m_audioInputStream(stream)
{
}

bool TconfigsMicrophoneWrapper::initialize(std::shared_ptr<TconfigsPlatformAdapter> adapter)
{
    m_writer = m_audioInputStream->createWriter(
            avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);
    if (!m_writer) {
        ACSDK_CRITICAL(LX("Failed to create stream writer"));
        return false;
    }

    m_pipeline = adapter->getRecordPipeline();
    if (!m_pipeline) {
        ACSDK_CRITICAL(LX("Invalid record pipeline"));
        return false;
    }

    m_pipeline->ConnectElementSignal<void*, const tconfigs::audio::BufferProperty*,
                                     tconfigs::audio::CommonSink::Result*>(
            "common_sink", "DataGot",
            [&](void* data, const tconfigs::audio::BufferProperty* property,
                tconfigs::audio::CommonSink::Result* result) {
        int effectiveBits, totalBits;
        tconfigs::audio::BufferProperty::FormatTypeToBits(
                property->format(), &effectiveBits, &totalBits);
        int bytes = totalBits / 8 * property->channels() * property->frames();
        size_t words = static_cast<size_t>(bytes) / m_writer->getWordSize();
        int ret = m_writer->write(data, words);
        if (ret <= 0) {
            ACSDK_CRITICAL(LX("Failed to write to stream").d("errorCode", ret));
            *result = tconfigs::audio::CommonSink::Result::kError;
            return;
        }
        *result = tconfigs::audio::CommonSink::Result::kNormal;
    });

    return true;
}

bool TconfigsMicrophoneWrapper::stopStreamingMicrophoneData()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pipeline->SetState(tconfigs::audio::State::kPaused);
}

bool TconfigsMicrophoneWrapper::startStreamingMicrophoneData()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pipeline->SetState(tconfigs::audio::State::kPlaying);
}

}  // namespace sampleApp
}  // namespace alexaClientSDK
