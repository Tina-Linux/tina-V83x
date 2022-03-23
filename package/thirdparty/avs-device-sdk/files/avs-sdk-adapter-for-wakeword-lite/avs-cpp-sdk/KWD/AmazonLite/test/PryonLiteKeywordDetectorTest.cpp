/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <AVSCommon/AVS/AudioInputStream.h>
#include <AVSCommon/SDKInterfaces/KeyWordDetectorStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AVSCommon/Utils/SDS/SharedDataStream.h>

#include "AmazonLite/PryonLiteKeywordDetector.h"

namespace alexaClientSDK {
namespace kwd {
namespace test {

using namespace avsCommon;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;

/// The path to the inputs folder that should be passed in via command line
/// argument.
std::string inputsDirPath;

/// The keyword that PryonLite emits for the below sound files
static const std::string KEYWORD = "alexa";

/// The name of a test audio file.
static const std::string FOUR_ALEXAS_AUDIO_FILE = "/four_alexa.wav";

/// The name of a test audio file.
static const std::string ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE = "/alexa_stop_alexa_joke.wav";

/// Test audio file for 'Alexa tell me a joke'.
static const std::string ALEXA_JOKE_AUDIO_FILE = "/alexa_joke.wav";

/// Test audio file for 'Stop stop'.
static const std::string STOP_STOP_AUDIO_FILE = "/stop_stop.wav";

/// The number of samples per millisecond, assuming a sample rate of 16 kHz.
static const int SAMPLES_PER_MS = 16;

/// The margin in milliseconds for testing indices of keyword detections.
static const std::chrono::milliseconds MARGIN = std::chrono::milliseconds(250);

/// The margin in samples for testing indices of keyword detections.
static const AudioInputStream::Index MARGIN_IN_SAMPLES = MARGIN.count() * SAMPLES_PER_MS;

/// The number of "Alexa" keywords in the four_alexa.wav file.
static const size_t NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE = 4;

/// The approximate begin indices of the four "Alexa" keywords in the
/// four_alexa.wav file.
std::vector<AudioInputStream::Index> BEGIN_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE = {7520, 39680, 58880, 77120};

/// The approximate end indices of the four "Alexa" hotwords in the
/// four_alexa.wav file.
std::vector<AudioInputStream::Index> END_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE = {21440, 52800, 72480, 91552};

/// The number of "Alexa" keywords in the alexa_stop_alexa_joke.wav file.
static const size_t NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE = 2;

/// The approximate begin indices of the two "Alexa" keywords in the
/// alexa_stop_alexa_joke.wav file.
std::vector<AudioInputStream::Index> BEGIN_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE = {8000, 38240};

/// The approximate end indices of the two "Alexa" keywords in the
/// alexa_stop_alexa_joke.wav file.
std::vector<AudioInputStream::Index> END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE = {20960, 51312};

/// The compatible encoding for PryonLite.
static const avsCommon::utils::AudioFormat::Encoding COMPATIBLE_ENCODING =
    avsCommon::utils::AudioFormat::Encoding::LPCM;

/// The compatible endianness for PryonLite.
static const avsCommon::utils::AudioFormat::Endianness COMPATIBLE_ENDIANNESS =
    avsCommon::utils::AudioFormat::Endianness::LITTLE;

/// The compatible sample rate for PryonLite.
static const unsigned int COMPATIBLE_SAMPLE_RATE = 16000;

/// The compatible bits per sample for PryonLite.
static const unsigned int COMPATIBLE_SAMPLE_SIZE_IN_BITS = 16;

/// The compatible number of channels for PryonLite.
static const unsigned int COMPATIBLE_NUM_CHANNELS = 1;

/// Timeout for expected callbacks.
static const auto DEFAULT_TIMEOUT = std::chrono::milliseconds(4000);

/// Audio Stream buffer size used in this test
static const int AUDIO_STREAM_BUFFER_SIZE = 500000;

/// Data Stream Readers used for this test
static const size_t SDS_DATA_READERS = 1;

/// Data Stream word size used in this test
static const size_t SDS_DATA_WORD_SIZE = 2;

/// A test observer that mocks out the
/// KeyWordObserverInterface##onKeyWordDetected() call.
class TestKeyWordObserver : public KeyWordObserverInterface {
public:
    /// A struct used for bookkeeping of keyword detections.
    struct DetectionResult {
        AudioInputStream::Index beginIndex;
        AudioInputStream::Index endIndex;
        std::string keyword;
    };

    /// Implementation of the KeyWordObserverInterface##onKeyWordDetected() call.
    void onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index beginIndex = UNSPECIFIED_INDEX,
        AudioInputStream::Index endIndex = UNSPECIFIED_INDEX,
        std::shared_ptr<const std::vector<char>> KWDMetadata = nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
        if (keyword != "alexa") {
            return;
        }
        m_detectionResults.push_back({beginIndex, endIndex, keyword});
        m_detectionOccurred.notify_one();
    };

    /**
     * Waits for the KeyWordObserverInterface##onKeyWordDetected() call N times.
     *
     * @param numDetectionsExpected The number of detections expected.
     * @param timeout The amount of time to wait for the calls.
     * @return The detection results that actually occurred.
     */
    std::vector<DetectionResult> waitForNDetections(
        unsigned int numDetectionsExpected,
        std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_detectionOccurred.wait_for(lock, timeout, [this, numDetectionsExpected]() {
            return m_detectionResults.size() == numDetectionsExpected;
        });
        return m_detectionResults;
    }

private:
    /// The detection results that have occurred.
    std::vector<DetectionResult> m_detectionResults;

    /// A lock to guard against new detections.
    std::mutex m_mutex;

    /// A condition variable to wait for detection calls.
    std::condition_variable m_detectionOccurred;
};

/// A test observer that mocks out the
/// KeyWordObserverInterface##onKeyWordDetected() call. This observer waits for
/// a stop keyword
class TestStopKeyWordObserver : public KeyWordObserverInterface {
public:
    /**
     * Constructor.
     */
    TestStopKeyWordObserver() : m_alexaKeyWordDetected{false}, m_stopKeyWordDetected{false}, m_keyWordsInOrder(false) {
    }

    /// Implementation of the KeyWordObserverInterface##onKeyWordDetected() call.
    void onKeyWordDetected(
        std::shared_ptr<AudioInputStream> stream,
        std::string keyword,
        AudioInputStream::Index beginIndex = UNSPECIFIED_INDEX,
        AudioInputStream::Index endIndex = UNSPECIFIED_INDEX,
        std::shared_ptr<const std::vector<char>> KWDMetadata = nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if ("ALEXA" == keyword) {
            m_alexaKeyWordDetected = true;
        } else if ("STOP" == keyword) {
            m_stopKeyWordDetected = true;
            if (m_alexaKeyWordDetected == true) {
                m_keyWordsInOrder = true;
            }
            m_detectionOccurred.notify_one();
        }
    };

    /* Waits for the 'Stop' keyword detection or will time out after
     * DEFAULT_TIMEOUT if not detected
     * @return true, if the detection is finished, else, no detection happened
     */
    bool waitKeyWordDetection() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_detectionOccurred.wait_for(lock, DEFAULT_TIMEOUT, [this]() { return m_stopKeyWordDetected; });
    }

    /**
     * Identifies if the 'Stop' keyword has been detected.
     *
     * @return true, if 'Stop has been detected; false otherwise.
     */
    bool isStopDetected() {
        return m_stopKeyWordDetected;
    }

    /**
     * Identifies if 'Alexa' and Stop' keywords has been detected, and KWD
     * notifies the callback in order
     *
     * @return true, if the notification is in order; false otherwise.
     */
    bool isInOrder() {
        return m_keyWordsInOrder;
    }

private:
    /// Indicates if "Alexa" keyword has been detected
    bool m_alexaKeyWordDetected;

    /// Indicates if "Stop" keyword has been detected
    bool m_stopKeyWordDetected;

    /// Indicates if "Alexa" and "Stop" keywords have been detected in order
    bool m_keyWordsInOrder;

    /// A lock to guard against new detections.
    std::mutex m_mutex;

    /// A condition variable to wait for detection calls.
    std::condition_variable m_detectionOccurred;
};

/// A test observer that mocks out the
/// KeyWordDetectorStateObserverInterface##onStateChanged() call.
class TestStateObserver : public KeyWordDetectorStateObserverInterface {
public:
    /**
     * Constructor.
     */
    TestStateObserver() :
            m_state(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::STREAM_CLOSED),
            m_stateChangeOccurred{false} {
    }

    /// Implementation of the
    /// KeyWordDetectorStateObserverInterface##onStateChanged() call.
    void onStateChanged(KeyWordDetectorStateObserverInterface::KeyWordDetectorState keyWordDetectorState) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_state = keyWordDetectorState;
        m_stateChangeOccurred = true;
        m_stateChanged.notify_one();
    }

    /**
     * Waits for the KeyWordDetectorStateObserverInterface##onStateChanged() call.
     *
     * @param timeout The amount of time to wait for the call.
     * @param stateChanged An output parameter that notifies the caller whether a
     * call occurred.
     * @return Returns the state of the observer.
     */
    KeyWordDetectorStateObserverInterface::KeyWordDetectorState waitForStateChange(
        std::chrono::milliseconds timeout,
        bool* stateChanged) {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool success = m_stateChanged.wait_for(lock, timeout, [this]() { return m_stateChangeOccurred; });

        if (!success) {
            *stateChanged = false;
        } else {
            m_stateChangeOccurred = false;
            *stateChanged = true;
        }
        return m_state;
    }

private:
    /// The state of the observer.
    KeyWordDetectorStateObserverInterface::KeyWordDetectorState m_state;

    /// A boolean flag so that we can re-use the observer even after a callback
    /// has occurred.
    bool m_stateChangeOccurred;

    /// A lock to guard against state changes.
    std::mutex m_mutex;

    /// A condition variable to wait for state changes.
    std::condition_variable m_stateChanged;
};

class PryonLiteKeywordTest : public ::testing::Test {
protected:
    /**
     * Reads audio from a WAV file.
     *
     * @param fileName The path of the file to read from.
     * @param [out] errorOccurred Lets users know if any errors occurred while
     * parsing the file.
     * @return A vector of int16_t containing the raw audio data of the WAV file
     * without the RIFF header.
     */
    std::vector<int16_t> readAudioFromFile(const std::string& fileName, bool* errorOccurred) {
        const int RIFF_HEADER_SIZE = 44;

        std::ifstream inputFile(fileName.c_str(), std::ifstream::binary);
        if (!inputFile.good()) {
            std::cout << "Couldn't open audio file!" << std::endl;
            if (errorOccurred) {
                *errorOccurred = true;
            }
            return {};
        }
        inputFile.seekg(0, std::ios::end);
        int fileLengthInBytes = inputFile.tellg();
        if (fileLengthInBytes <= RIFF_HEADER_SIZE) {
            std::cout << "File should be larger than 44 bytes, which is the size of "
                         "the RIFF header"
                      << std::endl;
            if (errorOccurred) {
                *errorOccurred = true;
            }
            return {};
        }

        inputFile.seekg(RIFF_HEADER_SIZE, std::ios::beg);

        int numSamples = (fileLengthInBytes - RIFF_HEADER_SIZE) / 2;

        std::vector<int16_t> retVal(numSamples, 0);

        inputFile.read((char*)&retVal[0], numSamples * 2);

        if (inputFile.gcount() != numSamples * 2) {
            std::cout << "Error reading audio file" << std::endl;
            if (errorOccurred) {
                *errorOccurred = true;
            }
            return {};
        }

        inputFile.close();
        if (errorOccurred) {
            *errorOccurred = false;
        }
        return retVal;
    }

    /**
     * Checks to see that the expected keyword detection results are present.
     *
     * @param results A vector of @c detectionResult.
     * @param expectedBeginIndex The expected begin index of the keyword.
     * @param expectedEndIndex The expected end index of the keyword.
     * @param expectedKeyword The expected keyword.
     * @return @c true if the result is present within the margin and @c false
     * otherwise.
     */
    bool isResultPresent(
        std::vector<TestKeyWordObserver::DetectionResult>& results,
        AudioInputStream::Index expectedBeginIndex,
        AudioInputStream::Index expectedEndIndex,
        const std::string& expectedKeyword) {
        AudioInputStream::Index highBoundOfBeginIndex = expectedBeginIndex + MARGIN_IN_SAMPLES;
        AudioInputStream::Index lowBoundOfBeginIndex = expectedBeginIndex - MARGIN_IN_SAMPLES;
        AudioInputStream::Index highBoundOfEndIndex = expectedEndIndex + MARGIN_IN_SAMPLES;
        AudioInputStream::Index lowBoundOfEndIndex = expectedEndIndex - MARGIN_IN_SAMPLES;
        for (auto result : results) {
            if (result.endIndex <= highBoundOfEndIndex && result.endIndex >= lowBoundOfEndIndex &&
                result.beginIndex <= highBoundOfBeginIndex && result.beginIndex >= lowBoundOfBeginIndex &&
                expectedKeyword == result.keyword) {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<TestKeyWordObserver> keyWordObserver1;

    std::shared_ptr<TestKeyWordObserver> keyWordObserver2;

    std::shared_ptr<TestStopKeyWordObserver> stopKeyWordObserver;

    std::shared_ptr<TestStateObserver> stateObserver;

    AudioFormat compatibleAudioFormat;

    std::string modelFilePath;

    virtual void SetUp() {
        keyWordObserver1 = std::make_shared<TestKeyWordObserver>();
        keyWordObserver2 = std::make_shared<TestKeyWordObserver>();
        stopKeyWordObserver = std::make_shared<TestStopKeyWordObserver>();
        stateObserver = std::make_shared<TestStateObserver>();

        compatibleAudioFormat.sampleRateHz = COMPATIBLE_SAMPLE_RATE;
        compatibleAudioFormat.sampleSizeInBits = COMPATIBLE_SAMPLE_SIZE_IN_BITS;
        compatibleAudioFormat.numChannels = COMPATIBLE_NUM_CHANNELS;
        compatibleAudioFormat.endianness = COMPATIBLE_ENDIANNESS;
        compatibleAudioFormat.encoding = COMPATIBLE_ENCODING;
    }
};

/// Tests that we don't get back a valid detector if an invalid stream is passed
/// in.
TEST_F(PryonLiteKeywordTest, invalidStream) {
    auto detector =
        PryonLiteKeywordDetector::create(nullptr, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_FALSE(detector);
}

/// Tests that we don't get back a valid detector if an invalid endianness is
/// passed in.
TEST_F(PryonLiteKeywordTest, incompatibleEndianness) {
    auto rawBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto uniqueSds = avsCommon::avs::AudioInputStream::create(rawBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> sds = std::move(uniqueSds);

    compatibleAudioFormat.endianness = AudioFormat::Endianness::BIG;

    auto detector = PryonLiteKeywordDetector::create(sds, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_FALSE(detector);
}

/// Tests that we get back the expected number of keywords for the
/// four_alexa.wav file for one keyword observer.
TEST_F(PryonLiteKeywordTest, getExpectedNumberOfDetectionsInFourAlexasAudioFileForOneObserver) {
    auto fourAlexasBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto fourAlexasSds = avsCommon::avs::AudioInputStream::create(fourAlexasBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> fourAlexasAudioBuffer = std::move(fourAlexasSds);

    std::unique_ptr<AudioInputStream::Writer> fourAlexasAudioBufferWriter =
        fourAlexasAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + FOUR_ALEXAS_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    fourAlexasAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detector = PryonLiteKeywordDetector::create(
        fourAlexasAudioBuffer, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_TRUE(detector);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto detections =
        keyWordObserver1->waitForNDetections(END_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.size(), DEFAULT_TIMEOUT);
    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE);

    for (unsigned int i = 0; i < END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.size(); ++i) {
        ASSERT_TRUE(isResultPresent(
            detections,
            BEGIN_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            END_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            KEYWORD));
    }
}

/// Tests that we get back the expected number of keywords for the
/// four_alexa.wav file for two keyword observers.
TEST_F(PryonLiteKeywordTest, getExpectedNumberOfDetectionsInFourAlexasAudioFileForTwoObservers) {
    auto fourAlexasBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto fourAlexasSds = avsCommon::avs::AudioInputStream::create(fourAlexasBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> fourAlexasAudioBuffer = std::move(fourAlexasSds);

    std::unique_ptr<AudioInputStream::Writer> fourAlexasAudioBufferWriter =
        fourAlexasAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + FOUR_ALEXAS_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    fourAlexasAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detector = PryonLiteKeywordDetector::create(
        fourAlexasAudioBuffer, compatibleAudioFormat, {keyWordObserver1, keyWordObserver2}, {stateObserver});
    ASSERT_TRUE(detector);
    auto detections = keyWordObserver1->waitForNDetections(NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE, DEFAULT_TIMEOUT);
    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE);

    for (unsigned int i = 0; i < END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.size(); ++i) {
        ASSERT_TRUE(isResultPresent(
            detections,
            BEGIN_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            END_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            KEYWORD));
    }

    detections = keyWordObserver2->waitForNDetections(NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE, DEFAULT_TIMEOUT);
    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE);

    for (unsigned int i = 0; i < END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.size(); ++i) {
        ASSERT_TRUE(isResultPresent(
            detections,
            BEGIN_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            END_INDICES_OF_ALEXAS_IN_FOUR_ALEXAS_AUDIO_FILE.at(i),
            KEYWORD));
    }
}

/**
 * Tests that we get back the expected number of keywords for the
 * alexa_stop_alexa_joke.wav file for one keyword observer.
 */
TEST_F(PryonLiteKeywordTest, getExpectedNumberOfDetectionsInAlexaStopAlexaJokeAudioFileForOneObserver) {
    auto alexaStopAlexaJokeBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto alexaStopAlexaJokeSds = avsCommon::avs::AudioInputStream::create(alexaStopAlexaJokeBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> alexaStopAlexaJokeAudioBuffer = std::move(alexaStopAlexaJokeSds);

    std::unique_ptr<AudioInputStream::Writer> alexaStopAlexaJokeAudioBufferWriter =
        alexaStopAlexaJokeAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    alexaStopAlexaJokeAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detector = PryonLiteKeywordDetector::create(
        alexaStopAlexaJokeAudioBuffer, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_TRUE(detector);

    auto detections =
        keyWordObserver1->waitForNDetections(NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE, DEFAULT_TIMEOUT);

    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE);

    for (unsigned int i = 0; i < END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.size(); ++i) {
        ASSERT_TRUE(isResultPresent(
            detections,
            BEGIN_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.at(i),
            END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.at(i),
            KEYWORD));
    }
}

/// Tests that the detector state changes to ACTIVE when the detector is
/// initialized properly.
TEST_F(PryonLiteKeywordTest, getActiveState) {
    auto alexaStopAlexaJokeBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto alexaStopAlexaJokeSds = avsCommon::avs::AudioInputStream::create(alexaStopAlexaJokeBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> alexaStopAlexaJokeAudioBuffer = std::move(alexaStopAlexaJokeSds);

    std::unique_ptr<AudioInputStream::Writer> alexaStopAlexaJokeAudioBufferWriter =
        alexaStopAlexaJokeAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    alexaStopAlexaJokeAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detector = PryonLiteKeywordDetector::create(
        alexaStopAlexaJokeAudioBuffer, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_TRUE(detector);
    bool stateChanged = false;
    KeyWordDetectorStateObserverInterface::KeyWordDetectorState stateReceived =
        stateObserver->waitForStateChange(DEFAULT_TIMEOUT, &stateChanged);
    ASSERT_TRUE(stateChanged);
    ASSERT_EQ(stateReceived, KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
}

/**
 * Tests that the stream is closed and that the detector state changes to
 * STREAM_CLOSED when we close the only writer of the SDS passed in and all
 * keyword detections have occurred.
 */
TEST_F(PryonLiteKeywordTest, getStreamClosedState) {
    auto alexaStopAlexaJokeBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto alexaStopAlexaJokeSds = avsCommon::avs::AudioInputStream::create(alexaStopAlexaJokeBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> alexaStopAlexaJokeAudioBuffer = std::move(alexaStopAlexaJokeSds);

    std::unique_ptr<AudioInputStream::Writer> alexaStopAlexaJokeAudioBufferWriter =
        alexaStopAlexaJokeAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    alexaStopAlexaJokeAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detector = PryonLiteKeywordDetector::create(
        alexaStopAlexaJokeAudioBuffer, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_TRUE(detector);

    // so that when we close the writer, we know for sure that the reader will be
    // closed
    auto detections =
        keyWordObserver1->waitForNDetections(NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE, DEFAULT_TIMEOUT);
    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE);

    bool stateChanged = false;
    KeyWordDetectorStateObserverInterface::KeyWordDetectorState stateReceived =
        stateObserver->waitForStateChange(DEFAULT_TIMEOUT, &stateChanged);
    ASSERT_TRUE(stateChanged);
    ASSERT_EQ(stateReceived, KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);

    alexaStopAlexaJokeAudioBufferWriter->close();
    stateChanged = false;
    stateReceived = stateObserver->waitForStateChange(DEFAULT_TIMEOUT, &stateChanged);
    ASSERT_TRUE(stateChanged);
    ASSERT_EQ(stateReceived, KeyWordDetectorStateObserverInterface::KeyWordDetectorState::STREAM_CLOSED);
}

/**
 * Tests that we get back the expected number of keywords for the
 * alexa_stop_alexa_joke.wav file for one keyword observer even when SDS has
 * other data prior to the audio file in it. This tests that the reference point
 * that the Sensory wrapper uses is working as expected.
 */
TEST_F(PryonLiteKeywordTest, getExpectedNumberOfDetectionsInAlexaStopAlexaJokeAudioFileWithRandomDataAtBeginning) {
    auto alexaStopAlexaJokeBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(500000);
    auto alexaStopAlexaJokeSds = avsCommon::avs::AudioInputStream::create(alexaStopAlexaJokeBuffer, 2, 1);
    std::shared_ptr<AudioInputStream> alexaStopAlexaJokeAudioBuffer = std::move(alexaStopAlexaJokeSds);

    std::unique_ptr<AudioInputStream::Writer> alexaStopAlexaJokeAudioBufferWriter =
        alexaStopAlexaJokeAudioBuffer->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    std::vector<int16_t> randomData(5000, 0);
    alexaStopAlexaJokeAudioBufferWriter->write(randomData.data(), randomData.size());

    auto detector = PryonLiteKeywordDetector::create(
        alexaStopAlexaJokeAudioBuffer, compatibleAudioFormat, {keyWordObserver1}, {stateObserver});
    ASSERT_TRUE(detector);

    alexaStopAlexaJokeAudioBufferWriter->write(audioData.data(), audioData.size());

    auto detections =
        keyWordObserver1->waitForNDetections(NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE, DEFAULT_TIMEOUT);

    ASSERT_EQ(detections.size(), NUM_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE);

    for (unsigned int i = 0; i < END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.size(); ++i) {
        ASSERT_TRUE(isResultPresent(
            detections,
            BEGIN_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.at(i) + randomData.size(),
            END_INDICES_OF_ALEXAS_IN_ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE.at(i) + randomData.size(),
            KEYWORD));
    }
}

/**
 * Tests if 'Alexa Stop' can be detected and PryonLite notifies 'Alexa' and
 * 'Stop' in order
 */
TEST_F(PryonLiteKeywordTest, detectAlexaStop) {
    auto audioBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(AUDIO_STREAM_BUFFER_SIZE);
    auto audioSDS = avsCommon::avs::AudioInputStream::create(audioBuffer, SDS_DATA_WORD_SIZE, SDS_DATA_READERS);
    std::shared_ptr<AudioInputStream> audioStream = std::move(audioSDS);

    std::unique_ptr<AudioInputStream::Writer> audioBufferWriter =
        audioStream->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_STOP_ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    auto detector =
        PryonLiteKeywordDetector::create(audioStream, compatibleAudioFormat, {stopKeyWordObserver}, {stateObserver});
    ASSERT_TRUE(detector);

    audioBufferWriter->write(audioData.data(), audioData.size());

    ASSERT_TRUE(stopKeyWordObserver->waitKeyWordDetection());

    ASSERT_TRUE(stopKeyWordObserver->isStopDetected());
    ASSERT_TRUE(stopKeyWordObserver->isInOrder());
}

/**
 * Tests if 'Stop' keyword will not be detected if the utterance is 'Alexa tell
 * me a joke'
 */
TEST_F(PryonLiteKeywordTest, nonStopKeywordNotDetected) {
    auto audioBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(AUDIO_STREAM_BUFFER_SIZE);
    auto audioSDS = avsCommon::avs::AudioInputStream::create(audioBuffer, SDS_DATA_WORD_SIZE, SDS_DATA_READERS);
    std::shared_ptr<AudioInputStream> audioStream = std::move(audioSDS);

    std::unique_ptr<AudioInputStream::Writer> audioBufferWriter =
        audioStream->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + ALEXA_JOKE_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    auto detector =
        PryonLiteKeywordDetector::create(audioStream, compatibleAudioFormat, {stopKeyWordObserver}, {stateObserver});
    ASSERT_TRUE(detector);

    audioBufferWriter->write(audioData.data(), audioData.size());

    ASSERT_FALSE(stopKeyWordObserver->waitKeyWordDetection());

    ASSERT_FALSE(stopKeyWordObserver->isStopDetected());
}

/**
 * Tests if 'Stop' keyword will not be detected if the utterance is 'Stop stop'
 */
TEST_F(PryonLiteKeywordTest, stopNotDetectedWithoutAlexa) {
    auto audioBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(AUDIO_STREAM_BUFFER_SIZE);
    auto audioSDS = avsCommon::avs::AudioInputStream::create(audioBuffer, SDS_DATA_WORD_SIZE, SDS_DATA_READERS);
    std::shared_ptr<AudioInputStream> audioStream = std::move(audioSDS);

    std::unique_ptr<AudioInputStream::Writer> audioBufferWriter =
        audioStream->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + STOP_STOP_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    auto detector =
        PryonLiteKeywordDetector::create(audioStream, compatibleAudioFormat, {stopKeyWordObserver}, {stateObserver});
    ASSERT_TRUE(detector);

    audioBufferWriter->write(audioData.data(), audioData.size());

    ASSERT_FALSE(stopKeyWordObserver->waitKeyWordDetection());

    ASSERT_FALSE(stopKeyWordObserver->isStopDetected());
}

/**
 * Tests if 'Stop' keyword will not be detected if the utterance is 'Alexa
 * alexa'
 */
TEST_F(PryonLiteKeywordTest, stopNotDetectedWithoutStop) {
    auto audioBuffer = std::make_shared<avsCommon::avs::AudioInputStream::Buffer>(AUDIO_STREAM_BUFFER_SIZE);
    auto audioSDS = avsCommon::avs::AudioInputStream::create(audioBuffer, SDS_DATA_WORD_SIZE, SDS_DATA_READERS);
    std::shared_ptr<AudioInputStream> audioStream = std::move(audioSDS);

    std::unique_ptr<AudioInputStream::Writer> audioBufferWriter =
        audioStream->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);

    std::string audioFilePath = inputsDirPath + FOUR_ALEXAS_AUDIO_FILE;
    bool error;
    std::vector<int16_t> audioData = readAudioFromFile(audioFilePath, &error);
    ASSERT_FALSE(error);

    auto detector =
        PryonLiteKeywordDetector::create(audioStream, compatibleAudioFormat, {stopKeyWordObserver}, {stateObserver});
    ASSERT_TRUE(detector);

    audioBufferWriter->write(audioData.data(), audioData.size());

    ASSERT_FALSE(stopKeyWordObserver->waitKeyWordDetection());

    ASSERT_FALSE(stopKeyWordObserver->isStopDetected());
}

}  // namespace test
}  // namespace kwd
}  // namespace alexaClientSDK

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        std::cerr << "USAGE: " << std::string(argv[0]) << " <path_to_inputs_folder>" << std::endl;
        return 1;
    } else {
        alexaClientSDK::kwd::test::inputsDirPath = std::string(argv[1]);
        return RUN_ALL_TESTS();
    }
}