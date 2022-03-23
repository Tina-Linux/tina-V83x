/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED,
 * OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdalign.h>
#include <unordered_map>

#include <AVSCommon/Utils/Logger/Logger.h>

#include "AmazonLite/PryonLiteKeywordDetector.h"

#ifndef KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING

/// The keyword model data that has been compiled in externally defined in the
/// model.cpp file.
extern unsigned char prlBinaryModelData[];

/// The total size of the model binary in bytes externally defined in the
/// model.cpp file.
extern int prlBinaryModelLen;

#endif

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon;
using namespace avsCommon::sdkInterfaces;

/// String to identify log entries originating from this file.
static const std::string TAG("PryonLiteKeywordDetector");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

#ifdef KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING

/**
 * Loads the given model file into memory.
 *
 * @param modelMem[out] The object into which the model will be loaded into.
 * @param modelFilePath The path to the model file to load into memory.
 * @param return @c true if the model was successfully loaded into memory and @c
 * false otherwise.
 */
static bool loadModelIntoMemory(std::vector<char>* modelMem, const std::string& modelFilePath) {
    if (!modelMem) {
        ACSDK_ERROR(LX("loadModelIntoMemoryFailed").d("reason", "nullInMemoryModel"));
        return false;
    }
    std::ifstream inFile(modelFilePath, std::ios::binary | std::ios::ate);
    if (!inFile.good()) {
        ACSDK_ERROR(LX("loadModelIntoMemoryFailed")
                        .d("reason", "failedToCreateFileStreamWithGivenFilePath")
                        .d("filePath", modelFilePath));
    }
    auto size = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    modelMem->resize(size);
    if (!inFile.read(modelMem->data(), modelMem->size())) {
        ACSDK_ERROR(LX("loadModelIntoMemoryFailed").d("reason", "failedToReadFromFile"));
        return false;
    }
    return true;
}

#endif

/// The timeout to use for read calls to the SharedDataStream.
static const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

// Map of Decoder instance to detector objects. This should be locked with
// g_lock before access.
static std::unordered_map<PryonLiteDecoderHandle, PryonLiteKeywordDetector*> g_decoderMap;

/// Lock to surround initialization of the detector and g_decoderMap.
static std::mutex g_lock;

/// Maximum metadata size is 2KB
static const int32_t METADATA_MAX_SIZE = 2 * 1024;

/// The Pryon Lite compatible sample rate of 16 kHz.
static const unsigned int PRYON_LITE_COMPATIBLE_SAMPLE_RATE_HZ = 16000;

/// The Pryon Lite compatible sample size in bits of 16.
static const unsigned int PRYON_LITE_COMPATIBLE_SAMPLE_SIZE_IN_BITS = 16;

/// The Pryon Lite compatible number of channels of 1.
static const unsigned int PRYON_LITE_COMPATIBLE_NUM_CHANNELS = 1;

/// The Pryon Lite compatible audio encoding of LPCM.
static const avsCommon::utils::AudioFormat::Encoding PRYON_LITE_COMPATIBLE_ENCODING =
    avsCommon::utils::AudioFormat::Encoding::LPCM;

/// The Pryon Lite compatible endianness which is little endian.
static const avsCommon::utils::AudioFormat::Endianness PRYON_LITE_COMPATIBLE_ENDIANNESS =
    avsCommon::utils::AudioFormat::Endianness::LITTLE;

static bool isAudioFormatCompatibleWithPryonLite(avsCommon::utils::AudioFormat audioFormat) {
    if (audioFormat.numChannels != PRYON_LITE_COMPATIBLE_NUM_CHANNELS) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithPryonLiteFailed")
                        .d("reason",
                           "Audio data number of channels does not meet PryonLite "
                           "requirements of " +
                               std::to_string(PRYON_LITE_COMPATIBLE_NUM_CHANNELS)));
        return false;
    }
    if (audioFormat.sampleRateHz != PRYON_LITE_COMPATIBLE_SAMPLE_RATE_HZ) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithPryonLiteFailed")
                        .d("reason",
                           "Audio data sample rate does not meet PryonLite requirements "
                           "of " +
                               std::to_string(PRYON_LITE_COMPATIBLE_SAMPLE_RATE_HZ)));
        return false;
    }
    if (audioFormat.sampleSizeInBits != PRYON_LITE_COMPATIBLE_SAMPLE_SIZE_IN_BITS) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithPryonLiteFailed")
                        .d("reason",
                           "Audio data bits per sample does not meet PryonLite "
                           "requirements of " +
                               std::to_string(PRYON_LITE_COMPATIBLE_SAMPLE_SIZE_IN_BITS)));
        return false;
    }
    if (audioFormat.endianness != PRYON_LITE_COMPATIBLE_ENDIANNESS) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithPryonLiteFailed")
                        .d("reason", "Audio data fed to PryonLite must be little endian"));
        return false;
    }
    if (audioFormat.encoding != PRYON_LITE_COMPATIBLE_ENCODING) {
        ACSDK_ERROR(LX("isAudioFormatCompatibleWithPryonLiteFailed")
                        .d("reason", "Audio data fed to PryonLite must be LPCM encoded"));
        return false;
    }
    return true;
}

std::unique_ptr<PryonLiteKeywordDetector> PryonLiteKeywordDetector::create(
    const std::shared_ptr<avsCommon::avs::AudioInputStream>& stream,
    avsCommon::utils::AudioFormat audioFormat,
    const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>>& keyWordObservers,
    const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>&
        KeyWordDetectorStateObservers,
    const std::string& pathToModelFile,
    int32_t detectThreshold,
    PryonLiteVad_Callback voiceActivityDetectionCallback) {
    std::lock_guard<std::mutex> lock{g_lock};
    if (!stream) {
        ACSDK_ERROR(LX("createFailed").d("reason", "Key word detector must be initialized with a valid stream"));
        return nullptr;
    }
    if (!isAudioFormatCompatibleWithPryonLite(audioFormat)) {
        return nullptr;
    }
    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between
    // endianness and if it's not too much, do it.
    if (isByteswappingRequired(audioFormat)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "Audio data endianness must match system endianness"));
        return nullptr;
    }
    if (detectThreshold < 1 || detectThreshold > 1000) {
        ACSDK_ERROR(LX("createFailed").d("reason", "detection threshold must be between 1 and 1000"));
        return nullptr;
    }
    std::unique_ptr<PryonLiteKeywordDetector> detector(
        new PryonLiteKeywordDetector(stream, audioFormat, keyWordObservers, KeyWordDetectorStateObservers));
    if (!detector->init(pathToModelFile, detectThreshold, voiceActivityDetectionCallback)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "Unable to initialize Pryon detector"));
        return nullptr;
    }
    return detector;
}

PryonLiteKeywordDetector::~PryonLiteKeywordDetector() {
    ACSDK_DEBUG9(LX("destructor"));
    m_isShuttingDown = true;
    if (m_detectionThread.joinable()) {
        m_detectionThread.join();
    }
    if (m_streamReader) {
        m_streamReader.reset();
    }
    {
        std::lock_guard<std::mutex> lock{g_lock};
        g_decoderMap.erase(m_decoder);
    }
    PryonLiteDecoder_Destroy(&m_decoder);
}

void PryonLiteKeywordDetector::resultCallback(PryonLiteDecoderHandle handle, const PryonLiteResult* result) {
    if (!result) {
        ACSDK_ERROR(LX("resultCallbackFailed").d("reason", "result is null"));
        return;
    }

    ACSDK_DEBUG9(LX("resultCallback").d("metadataSize", result->metadataBlob.blobSize));

    std::lock_guard<std::mutex> lock{g_lock};
    auto detectorFind = g_decoderMap.find(handle);
    if (detectorFind == g_decoderMap.end()) {
        ACSDK_ERROR(LX("Received wake word result for unknown decoder handle"));
        return;
    }
    auto detector = detectorFind->second;
    auto pryonMetadata = result->metadataBlob;

    std::shared_ptr<std::vector<char>> KWDMetadata;

    if (pryonMetadata.blobSize > METADATA_MAX_SIZE) {
        // We don't cancel a Recognize event due to an invalid metadata.
        ACSDK_ERROR(LX("Sending metadata failed")
                        .d("reason", "metadata too big")
                        .d("size", pryonMetadata.blobSize)
                        .d("max size", METADATA_MAX_SIZE));
    } else if (pryonMetadata.blobSize != 0 && pryonMetadata.blob != nullptr) {
        KWDMetadata =
            std::make_shared<std::vector<char>>(pryonMetadata.blob, pryonMetadata.blob + pryonMetadata.blobSize);
    }

    detector->notifyKeyWordObservers(
        detector->m_stream,
        result->keyword,
        detector->m_beginIndexOfStreamReader + detector->m_bufferOverrunJumpCounter + result->beginSampleIndex,
        detector->m_beginIndexOfStreamReader + detector->m_bufferOverrunJumpCounter + result->endSampleIndex,
        KWDMetadata);
}

PryonLiteKeywordDetector::PryonLiteKeywordDetector(
    const std::shared_ptr<avsCommon::avs::AudioInputStream>& stream,
    avsCommon::utils::AudioFormat audioFormat,
    const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>>& keyWordObservers,
    const std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>&
        KeyWordDetectorStateObservers) :
        AbstractKeywordDetector(keyWordObservers, KeyWordDetectorStateObservers),
        m_isShuttingDown{false},
        m_pryonLiteSamplesPerPush{0},
        m_stream{stream},
        m_beginIndexOfStreamReader{0},
        m_bufferOverrunJumpCounter{0},
        m_decoder{nullptr} {
}

bool PryonLiteKeywordDetector::init(
    const std::string& pathToModelFile,
    int32_t detectThreshold,
    PryonLiteVad_Callback voiceActivityDetectionCallback) {
    ACSDK_DEBUG9(LX("init"));

    m_streamReader = m_stream->createReader(avsCommon::avs::AudioInputStream::Reader::Policy::BLOCKING, true);
    if (!m_streamReader) {
        ACSDK_ERROR(LX("init").d("reason", "Unable to create stream reader"));
        return false;
    }

    PryonLiteDecoderConfig config = PryonLiteDecoderConfig_Default;
    config.detectThreshold = detectThreshold;
    if (voiceActivityDetectionCallback) {
        ACSDK_DEBUG9(LX("init").d("info", "userRequestedVoiceActivityDetectionEnabled"));
        config.useVad = true;
        config.vadCallback = voiceActivityDetectionCallback;
    }
#ifndef KWD_AMAZONLITE_DYNAMIC_MODEL_LOADING
    config.model = static_cast<void*>(prlBinaryModelData);
    config.sizeofModel = prlBinaryModelLen;
    if (!pathToModelFile.empty()) {
        ACSDK_WARN(LX("init").d(
            "warning",
            "model file path specified when built "
            "in static model compilation mode"));
    }
#else
    if (!loadModelIntoMemory(&m_modelMem, pathToModelFile)) {
        ACSDK_ERROR(LX("initFailed").d("reason", "FailedToLoadModelIntoMemory"));
        return false;
    }
    config.model = m_modelMem.data();
    config.sizeofModel = m_modelMem.size();
#endif
    config.resultCallback = resultCallback;
    PryonLiteSessionInfo sessionInfo;

    // Query for the size of instance memory required by the decoder
    PryonLiteModelAttributes modelAttributes;
    PryonLiteError error = PryonLite_GetModelAttributes(config.model, config.sizeofModel, &modelAttributes);
    if (error) {
        ACSDK_ERROR((LX("initFailed")).d("reason", "getModelAttributesFailed").d("errorCode", error));
        return false;
    }
    m_decoderMem.resize(modelAttributes.requiredDecoderMem);
    config.decoderMem = m_decoderMem.data();
    config.sizeofDecoderMem = modelAttributes.requiredDecoderMem;

    error = PryonLiteDecoder_Initialize(&config, &sessionInfo, &m_decoder);

    if (error) {
        ACSDK_ERROR(LX("initFailed").d("reason", "Unable to initialize PryonLite decoder").d("errorCode", error));
        return false;
    }

    ACSDK_INFO(LX("sessionInfo")
                   .d("engineVersion", sessionInfo.engineAttributes.engineVersion)
                   .d("modelVersion", sessionInfo.modelAttributes.modelVersion)
                   .d("requiredSamplesPerFramePerPush", sessionInfo.samplesPerFrame));

    m_pryonLiteSamplesPerPush = sessionInfo.samplesPerFrame;

    g_decoderMap.insert({m_decoder, this});

    m_isShuttingDown = false;
    m_detectionThread = std::thread{&PryonLiteKeywordDetector::detectionLoop, this};
    return true;
}

void PryonLiteKeywordDetector::detectionLoop() {
    // Initializes the m_beginIndexOfStreamReader field with the current index of the stream reader
    // so this detector will give the right index when notifying observers. See PryonLiteKeywordDetector::resultCallback
    m_beginIndexOfStreamReader = m_streamReader->tell();
    m_bufferOverrunJumpCounter = 0;
    notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
    std::vector<int16_t> audioDataToPush(m_pryonLiteSamplesPerPush);
    ACSDK_INFO(LX("detectionLoop").d("m_beginIndexOfStreamReader", m_beginIndexOfStreamReader));
    while (!m_isShuttingDown) {
        ssize_t wordsReadSoFar = 0;
        /*
         * PryonLite requires that m_pryonLiteSamplesPerPush samples are pushed per
         * call to PryonLiteDecoder_PushAudioSamples(). Anything less than that
         * causes issues with proper detection.
         */
        while (wordsReadSoFar < m_pryonLiteSamplesPerPush && !m_isShuttingDown) {
            bool didErrorOccur = false;
            ssize_t wordsReadThisIteration = 0;
            /*
             * We save the index before calling readFromStream so we can calculate the gap in case of buffer overrun.
             */
            auto indexBeforeRead = m_streamReader->tell();
            wordsReadThisIteration = readFromStream(
                m_streamReader,
                m_stream,
                audioDataToPush.data() + wordsReadSoFar,
                m_pryonLiteSamplesPerPush - wordsReadSoFar,
                TIMEOUT_FOR_READ_CALLS,
                &didErrorOccur);

            if (didErrorOccur) {
                break;
            } else if (wordsReadThisIteration < 0) {
                if (wordsReadThisIteration == avsCommon::avs::AudioInputStream::Reader::Error::OVERRUN) {
                    /*
                     * If a buffer overrun happened, the stream reader cursor jumped to the position before the writer.
                     * The wake word engine did not "see" such a data. So the wake word detection indexes that it will
                     * give us from now on will be wrong by the length of the jump. So we add the length of jump to the
                     * jump counter.
                     */
                    m_bufferOverrunJumpCounter += m_streamReader->tell() - indexBeforeRead;

                    ACSDK_INFO(LX("detectionLoop")
                            .d("m_bufferOverrunJumpCounter", m_bufferOverrunJumpCounter)
                            .m("bufferOverrun"));
                }
                ACSDK_ERROR(LX("detectionLoopFailed")
                                .d("reason", "invalidWordsRead")
                                .d("wordReadThisIterations", wordsReadThisIteration));
            } else if (wordsReadThisIteration > 0) {
                notifyKeyWordDetectorStateObservers(
                    KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
                wordsReadSoFar += wordsReadThisIteration;
            }
        }
        if (wordsReadSoFar == m_pryonLiteSamplesPerPush) {
            PryonLiteError error =
                PryonLiteDecoder_PushAudioSamples(m_decoder, audioDataToPush.data(), m_pryonLiteSamplesPerPush);
            if (error) {
                ACSDK_ERROR(LX("detectionLoop").d("reason", "Unable to push audio data").d("errorCode", error));
                notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ERROR);
                break;
            }
        } else {
            ACSDK_ERROR(
                LX("detectionLoopFailed").d("reason", "invalidWordsReadSoFar").d("wordsReadSoFar", wordsReadSoFar));
        }
    }
    m_streamReader->close();
}

}  // namespace kwd
}  // namespace alexaClientSDK
