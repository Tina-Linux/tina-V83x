/*
 * AlsaMicrophoneWrapper.h
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

#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_MICROPHONE_WRAPPER_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_MICROPHONE_WRAPPER_H_

#include <mutex>
#include <thread>

#include <AVSCommon/AVS/AudioInputStream.h>

#include "SampleApp/MicrophoneWrapperInterface.h"

#include <platformadapter/PlatformAdapter.h>
#include <recorder/RecorderInterface.h>
#include <utils/AlsaUtils.h>

namespace alexaClientSDK {
namespace sampleApp {

/// This acts as a wrapper around PortAudio, a cross-platform open-source audio I/O library.
class AlsaMicrophoneWrapper : public MicrophoneWrapperInterface {
public:
    /**
     * Creates a @c AlsaMicrophoneWrapper.
     *
     * @param stream The shared data stream to write to.
     * @return A unique_ptr to a @c AlsaMicrophoneWrapper if creation was successful and @c nullptr otherwise.
     */
    static std::shared_ptr<AlsaMicrophoneWrapper> create(
            std::shared_ptr<avsCommon::avs::AudioInputStream> wakeword_stream,
            std::shared_ptr<avsCommon::avs::AudioInputStream> esp_stream,
            std::shared_ptr<AW::PlatformAdapter> platformadapter);

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
    ~AlsaMicrophoneWrapper();

private:
    static const bool STARTED = true;
    static const bool STOPPED = false;
    /**
     * Constructor.
     *
     * @param stream The shared data stream to write to.
     */

    AlsaMicrophoneWrapper(std::shared_ptr<avsCommon::avs::AudioInputStream> wakeword_stream,
                          std::shared_ptr<avsCommon::avs::AudioInputStream> esp_stream,
                          std::shared_ptr<AW::PlatformAdapter> platformadapter);


    /// Initializes PortAudio
    bool initialize();

    void captureLoop();

    /// The stream of audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_audioInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_writer{nullptr};

    /// The stream of esp audio data.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_espInputStream;

    /// The writer that will be used to writer audio data into the sds.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Writer> m_esp_writer{nullptr};


    /// The AlsaRecorder snd handler
    std::shared_ptr<AW::PlatformAdapter> m_platformadapter;
    std::shared_ptr<AW::RecorderInterface> m_recorder;

    /**
     * A lock to seralize access to startStreamingMicrophoneData() and stopStreamingMicrophoneData() between different
     * threads.
     */
    std::mutex m_mutex;

    bool m_isStop{false};
    bool m_captureStatus{STOPPED};

    std::thread m_captureThread;

    std::unique_ptr<AW::AlsaUtils> m_loop_back{nullptr};
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_ALSA_MICROPHONE_WRAPPER_H_