/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <memory>

#include <AVSCommon/Utils/Logger/Logger.h>

#include "TutuClearKeywordDetector/TutuClearKeywordDetector.h"

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon::utils::logger;

/// String to identify log entries originating from this file.
static const std::string TAG("TutuClearKeywordDetector");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The number of hertz per kilohertz.
static const size_t HERTZ_PER_KILOHERTZ = 1000;

/// The timeout to use for read calls to the SharedDataStream.
const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

/// The Sensory compatible AVS sample rate of 16 kHz.
static const unsigned int SENSORY_COMPATIBLE_SAMPLE_RATE = 16000;

/// The Sensory compatible bits per sample of 16.
static const unsigned int SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS = 16;

/// The Sensory compatible number of channels, which is 1.
static const unsigned int SENSORY_COMPATIBLE_NUM_CHANNELS = 1;

/// The Sensory compatible audio encoding of LPCM.
static const avsCommon::utils::AudioFormat::Encoding SENSORY_COMPATIBLE_ENCODING =
    avsCommon::utils::AudioFormat::Encoding::LPCM;

/// The Sensory compatible endianness which is little endian.
static const avsCommon::utils::AudioFormat::Endianness SENSORY_COMPATIBLE_ENDIANNESS =
    avsCommon::utils::AudioFormat::Endianness::LITTLE;

/**
 * Checks to see if an @c avsCommon::utils::AudioFormat is compatible with Sensory.
 *
 * @param audioFormat The audio format to check.
 * @return @c true if the audio format is compatible with Sensory and @c false otherwise.
 */
static bool isAudioFormatCompatibleWithSensory(avsCommon::utils::AudioFormat audioFormat) {
    if (SENSORY_COMPATIBLE_ENCODING != audioFormat.encoding) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithSensoryFailed")
                        .d("reason", "incompatibleEncoding")
                        .d("sensoryEncoding", SENSORY_COMPATIBLE_ENCODING)
                        .d("encoding", audioFormat.encoding));
        return false;
    }
    if (SENSORY_COMPATIBLE_ENDIANNESS != audioFormat.endianness) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithSensoryFailed")
                        .d("reason", "incompatibleEndianess")
                        .d("sensoryEndianness", SENSORY_COMPATIBLE_ENDIANNESS)
                        .d("endianness", audioFormat.endianness));
        return false;
    }
    if (SENSORY_COMPATIBLE_SAMPLE_RATE != audioFormat.sampleRateHz) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithSensoryFailed")
                        .d("reason", "incompatibleSampleRate")
                        .d("sensorySampleRate", SENSORY_COMPATIBLE_SAMPLE_RATE)
                        .d("sampleRate", audioFormat.sampleRateHz));
        return false;
    }
    if (SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS != audioFormat.sampleSizeInBits) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithSensoryFailed")
                        .d("reason", "incompatibleSampleSizeInBits")
                        .d("sensorySampleSizeInBits", SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS)
                        .d("sampleSizeInBits", audioFormat.sampleSizeInBits));
        return false;
    }
    if (SENSORY_COMPATIBLE_NUM_CHANNELS != audioFormat.numChannels) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithSensoryFailed")
                        .d("reason", "incompatibleNumChannels")
                        .d("sensoryNumChannels", SENSORY_COMPATIBLE_NUM_CHANNELS)
                        .d("numChannels", audioFormat.numChannels));
        return false;
    }
    return true;
}

class FilterWakeWordCtx : public AW::FilterInterface::KeyWordObserver,
                          public std::enable_shared_from_this<FilterWakeWordCtx>
{
public:
    FilterWakeWordCtx(void *ctx):m_ctx{ctx}{};

private:
    void onFilterKeyWordDetected(std::string keyword, long beginIndex, long endIndex) {
        TutuClearKeywordDetector* engine = static_cast<TutuClearKeywordDetector*>(m_ctx);
        engine->notifyKeyWordObservers(
            engine->m_stream, keyword, beginIndex, endIndex);
    };

    void *m_ctx{nullptr};
};

std::unique_ptr<TutuClearKeywordDetector> TutuClearKeywordDetector::create(
    std::shared_ptr<AW::PlatformAdapter> platformadapter,
    std::shared_ptr<AudioInputStream> stream,
    avsCommon::utils::AudioFormat audioFormat,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>> keyWordObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>
        keyWordDetectorStateObservers) {
    if (!stream) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullStream"));
        return nullptr;
    }

    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between endianness and if it's not too much, do it.
    if (isByteswappingRequired(audioFormat)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "endianMismatch"));
        return nullptr;
    }

    if (!isAudioFormatCompatibleWithSensory(audioFormat)) {
        return nullptr;
    }
    std::unique_ptr<TutuClearKeywordDetector> detector(new TutuClearKeywordDetector(
        stream, keyWordObservers, keyWordDetectorStateObservers));
    if (!detector->init(platformadapter)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initDetectorFailed"));
        return nullptr;
    }
    return detector;
}

TutuClearKeywordDetector::~TutuClearKeywordDetector() {
    m_filter->removeKeyWordObserver(m_filter_wakeword_ctx);
}

TutuClearKeywordDetector::TutuClearKeywordDetector(
    std::shared_ptr<AudioInputStream> stream,
    std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
    std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>> keyWordDetectorStateObservers) :
        AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
        m_stream{stream} {
}

bool TutuClearKeywordDetector::init(std::shared_ptr<AW::PlatformAdapter> platformadapter) {

    auto recorder = platformadapter->getRecorder();
    if(recorder == nullptr) return false;

    m_filter = recorder->getFilter();
    if(m_filter == nullptr) return false;

    m_filter_wakeword_ctx = std::make_shared<FilterWakeWordCtx>(reinterpret_cast<void*>(this));
    m_filter->addKeyWordObserver(m_filter_wakeword_ctx);
    return true;
}

}  // namespace kwd
}  // namespace alexaClientSDK
