/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions set forth in the accompanying
 * LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS, WITH RESPECT TO THESE MATERIALS,
 * ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef ALEXA_CLIENT_SDK_KWD_AMAZONLITE_INCLUDE_AMAZONLITE_PRYONLITEKEYWORDDETECTOR_H_
#define ALEXA_CLIENT_SDK_KWD_AMAZONLITE_INCLUDE_AMAZONLITE_PRYONLITEKEYWORDDETECTOR_H_

#include <memory>
#include <thread>
#include <unordered_set>

#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/SDKInterfaces/KeyWordDetectorStateObserverInterface.h>
#include <AVSCommon/Utils/SDS/Reader.h>

#include "KWD/AbstractKeywordDetector.h"
#include "pryon_lite.h"

namespace alexaClientSDK {
namespace kwd {

/**
 * A class that makes calls to the underlying PryonLite wake word engine and notifies observers of keyword events being
 * triggered.
 */
class PryonLiteKeywordDetector : public AbstractKeywordDetector {
public:
    /**
     * Creates an @c PryonLiteKeywordDetector.
     *
     * @param stream The stream of audio data. This audio stream should be PCM encoded, have 16 bits per sample, have a
     * sample rate of 16 kHz, and be in little endian format.
     * @param audioFormat The format of the audio data located within the stream.
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param KeyWordDetectorStateObservers The observers to notify of state changes in the engine.
     * @param pathToModelFile The path to the desired model file to use for detections. This parameter is used if the
     * macro KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING is defined.
     * @param detectThreshold The threshold to use for detections. This must be a value from 1-1000. A value of 1
     * will result in the most detections and a value of 1000 will result in the fewest detections.
     * @param voiceActivityDetectionCallback An optional callback to be notified of voice activity detection state
     * changes. The definition of the required function signature may be found in pryon_lite.h.
     * @return A new @c PryonLiteKeywordDetector, or @c nullptr if the operation failed.
     */
    static std::unique_ptr<PryonLiteKeywordDetector> create(
        const std::shared_ptr<avsCommon::avs::AudioInputStream>& stream,
        avsCommon::utils::AudioFormat audioFormat,
        const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>>& keyWordObservers,
        const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>&
            KeyWordDetectorStateObservers,
        const std::string& pathToModelFile = "",
        int32_t detectThreshold = 500,
        PryonLiteVad_Callback voiceActivityDetectionCallback = nullptr);

    /**
     * Destructor. This joins on the internal detection loop thread and resets the PryonLite engine.
     */
    ~PryonLiteKeywordDetector() override;

private:
    /*
     * The callback that PryonLite will use to notify this class of wake word detections.
     *
     * @param handle A handle to the PryonLite decoder instance.
     * @param result PryonLite result containing the indices in which a detection occurred and
     * PryonLite metadata to be sent to AVS.
     * @note The maximum size for the PryonLite metadata is 2K, otherwise it won't be sent.
     */
    static void resultCallback(PryonLiteDecoderHandle handle, const PryonLiteResult* result);

    /**
     * Constructor.
     *
     * @param stream The stream from which to detect keywords.
     * @param audioFormat The format of the audio data located within the stream.
     * @param keyWordObservers The observers to notify of keyword detections.
     * @param KeyWordDetectorStateObservers The observers to notify of state changes in the engine.
     */
    PryonLiteKeywordDetector(
        const std::shared_ptr<avsCommon::avs::AudioInputStream>& stream,
        avsCommon::utils::AudioFormat audioFormat,
        const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>>& keyWordObservers,
        const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>&
            KeyWordDetectorStateObservers);

    /**
     * Initializes the PryonLite engine and creates a new decoder that that the client will be primarily interacting
     * with. This function should only be called once with each new @c PryonWakeWordDetector.
     *
     * @param pathToModelFile The path to the desired model file to use for detections. This parameter is used if the
     * macro KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING is defined.
     * @param detectThreshold Integer from 1-1000. Default is 500. The higher the threshold, the fewer detections.
     * @return @c true if a new decoder was created successfully and @c false if there were any errors with
     * initialization.
     */
    bool init(
        const std::string& pathToModelFile,
        int32_t detectThreshold,
        PryonLiteVad_Callback voiceActivityDetectionCallback);

    /// The main function that reads data and feeds it into the engine.
    void detectionLoop();

    /// Indicates whether the internal main loop should keep running.
    std::atomic<bool> m_isShuttingDown;

    /// The number of samples to feed to the PryonLite engine per iteration. This is supplied by PryonLite.
    int32_t m_pryonLiteSamplesPerPush;

    /// The stream from which keywords will be detected.
    const std::shared_ptr<avsCommon::avs::AudioInputStream> m_stream;

    /// The reader that will be used to read audio data from the stream.
    std::shared_ptr<avsCommon::avs::AudioInputStream::Reader> m_streamReader;

    /// The PryonLite decoder.
    PryonLiteDecoderHandle m_decoder;

    /// The memory allocated to hold m_decoder.
    std::vector<char> m_decoderMem;

#ifdef KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING

    /// The memory allocated to hold the PryonLite model data.
    std::vector<char> m_modelMem;

#endif

    /// Internal thread that reads audio from the buffer and feeds it to PryonLite.
    std::thread m_detectionThread;
};

}  // namespace kwd
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_KWD_AMAZONLITE_INCLUDE_AMAZONLITE_PRYONLITEKEYWORDDETECTOR_H_