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

#ifndef ALEXA_CLIENT_SDK_KWD_SENSORY_INCLUDE_TUTUCLEAR_TUTUCLEARKEYWORDDETECTOR_H_
#define ALEXA_CLIENT_SDK_KWD_SENSORY_INCLUDE_TUTUCLEAR_TUTUCLEARKEYWORDDETECTOR_H_

#include <atomic>
#include <string>
#include <thread>
#include <unordered_set>

#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/SDKInterfaces/KeyWordDetectorStateObserverInterface.h>

#include "KWD/AbstractKeywordDetector.h"

#include <platformadapter/PlatformAdapter.h>

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;

class FilterWakeWordCtx;
class TutuClearKeywordDetector : public AbstractKeywordDetector
{
public:
    /**
     * Creates a @c TutuClearKeywordDetector.
     *
     * have a sample rate of 16 kHz. Additionally, the data should be in little endian format.
     * @param audioFormat The format of the audio data located within the stream.
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param keyWordDetectorStateObservers The observers to notify of state changes in the engine.
     * @return A new @c TutuClearKeywordDetector, or @c nullptr if the operation failed.
     */
    static std::unique_ptr<TutuClearKeywordDetector> create(
        std::shared_ptr<AW::PlatformAdapter> platformadapter,
        std::shared_ptr<AudioInputStream> stream,
        avsCommon::utils::AudioFormat audioFormat,
        std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
        std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>> keyWordDetectorStateObservers);

    /**
     * Destructor.
     */
    ~TutuClearKeywordDetector() override;

private:
    friend class FilterWakeWordCtx;
    /**
     * Constructor.
     *
     * @param stream The stream of audio data. This should be formatted in LPCM encoded with 16 bits per sample and
     * have a sample rate of 16 kHz. Additionally, the data should be in little endian format.
     * @param audioFormat The format of the audio data located within the stream.
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param keyWordDetectorStateObservers The observers to notify of state changes in the engine.
     */
    TutuClearKeywordDetector(
        std::shared_ptr<AudioInputStream> stream,
        std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
        std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>> keyWordDetectorStateObservers);

    bool init(std::shared_ptr<AW::PlatformAdapter> platformadapter);

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_stream{nullptr};

    std::shared_ptr<AW::FilterInterface> m_filter{nullptr};
    std::shared_ptr<FilterWakeWordCtx> m_filter_wakeword_ctx{nullptr};
};

}  // namespace kwd
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_KWD_SENSORY_INCLUDE_TUTUCLEAR_TUTUCLEARKEYWORDDETECTOR_H_
