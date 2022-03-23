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

#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSMICROPHONEWRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSMICROPHONEWRAPPER_H_

#include <AVSCommon/AVS/AudioInputStream.h>
#include <Audio/MicrophoneInterface.h>

#include "SampleApp/TconfigsPlatformAdapter.h"

namespace alexaClientSDK {
namespace sampleApp {

/// This acts as a wrapper around Tconfigs.
class TconfigsMicrophoneWrapper
    : public applicationUtilities::resources::audio::MicrophoneInterface {
public:
    /**
     * Creates a @c TconfigsMicrophoneWrapper.
     *
     * @param stream The shared data stream to write to.
     * @param adapter The adapter for different platform configurations.
     * @return A unique_ptr to a @c TconfigsMicrophoneWrapper if creation
     *      was successful and @c nullptr otherwise.
     */
    static std::unique_ptr<TconfigsMicrophoneWrapper> create(
            std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
            std::shared_ptr<TconfigsPlatformAdapter> adapter);

    /**
     * Stops streaming from the microphone.
     *
     * @return Whether the stop was successful.
     */
    bool stopStreamingMicrophoneData() override;

    /**
     * Starts streaming from the microphone.
     *
     * @return Whether the start was successful.
     */
    bool startStreamingMicrophoneData() override;

    /**
     * Destructor.
     */
    ~TconfigsMicrophoneWrapper() = default;

private:
    /**
     * Constructor.
     *
     * @param stream The shared data stream to write to.
     */
    TconfigsMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> stream);
    TconfigsMicrophoneWrapper() = delete;

    /// Initializes TconfigsMicrophoneWrapper
    bool initialize(std::shared_ptr<TconfigsPlatformAdapter> adapter);

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_audioInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_writer = nullptr;

    /// The Tconfigs record pipeline
    std::shared_ptr<tconfigs::audio::Pipeline> m_pipeline = nullptr;

    /**
     * A lock to seralize access to startStreamingMicrophoneData() and
     * stopStreamingMicrophoneData() between different threads.
     */
    std::mutex m_mutex;
};

}  // namespace sampleApp
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSMICROPHONEWRAPPER_H_
