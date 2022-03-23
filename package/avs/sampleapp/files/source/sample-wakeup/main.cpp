/*
 * main.cpp
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
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include <cstdlib>
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>
#include <set>

#include "SampleApp/AlsaMicrophoneWrapper.h"
#include "SampleApp/ConsolePrinter.h"

#ifdef KWD_KITTAI
#include <KittAi/KittAiKeyWordDetector.h>
#endif

#ifdef KWD_SENSORY
#include <Sensory/SensoryKeywordDetector.h>
#endif

#ifdef KWD_AMAZONLITE
#include <AmazonLite/PryonLiteKeywordDetector.h>
#endif

#include <TutuClearKeywordDetector/TutuClearKeywordDetector.h>

//#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AIP/AudioInputProcessor.h>

#include <utils/WavUtils.h>
#include <platformadapter/PlatformAdapter.h>

/// The sample rate of microphone audio data.
static const unsigned int SAMPLE_RATE_HZ = 16000;

/// The number of audio channels.
static const unsigned int NUM_CHANNELS = 1;

/// The size of each word within the stream.
static const size_t WORD_SIZE = 2;

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

static const size_t CHAR_BIT = 8;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

/// The size of the ring buffer.
static const size_t BUFFER_SIZE_IN_SAMPLES = (SAMPLE_RATE_HZ)*AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count();

/// Key for the root node value containing configuration values for SampleApp.
static const std::string SAMPLE_APP_CONFIG_KEY("sampleApp");

/// Key for the endpoint value under the @c SAMPLE_APP_CONFIG_KEY configuration node.
static const std::string ENDPOINT_KEY("endpoint");

/// Default AVS endpoint to connect to.
static const std::string DEFAULT_ENDPOINT("https://avs-alexa-na.amazon.com");

#ifdef KWD_KITTAI
/// The sensitivity of the Kitt.ai engine.
static const double KITT_AI_SENSITIVITY = 0.6;

/// The audio amplifier level of the Kitt.ai engine.
static const float KITT_AI_AUDIO_GAIN = 2.0;

/// Whether Kitt.ai should apply front end audio processing.
static const bool KITT_AI_APPLY_FRONT_END_PROCESSING = true;
#endif

/// A set of all log levels.
static const std::set<alexaClientSDK::avsCommon::utils::logger::Level> allLevels = {
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG8,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG7,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG6,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG5,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG4,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG3,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG2,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG1,
    alexaClientSDK::avsCommon::utils::logger::Level::DEBUG0,
    alexaClientSDK::avsCommon::utils::logger::Level::INFO,
    alexaClientSDK::avsCommon::utils::logger::Level::WARN,
    alexaClientSDK::avsCommon::utils::logger::Level::ERROR,
    alexaClientSDK::avsCommon::utils::logger::Level::CRITICAL,
    alexaClientSDK::avsCommon::utils::logger::Level::NONE};

static std::string getTimeStamp() {
    struct timeval tv;
    struct timezone tz;
    struct tm *p;

    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);

    char buf[200];
    snprintf(buf, 200, "%02d-%02d-%02d-%02d-%02d-%02d-%06ld", p->tm_year + 1900, \
                                                         1+p->tm_mon, \
                                                         p->tm_mday, \
                                                         p->tm_hour, \
                                                         p->tm_min, \
                                                         p->tm_sec, \
                                                         tv.tv_usec);
    return std::string(buf);
}

/**
 * Observes callbacks from keyword detections and notifies the DefaultClient that a wake word has occurred.
 */
class KeywordObserver : public alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface {
public:
    /**
     * Constructor.
     *
     * @param client The default SDK client.
     * @param audioProvider The audio provider from which to stream audio data from.
     */
    KeywordObserver(std::shared_ptr<AW::IShowManager> show,
                    std::shared_ptr<AW::DOAInfo> doa,
                    alexaClientSDK::avsCommon::utils::AudioFormat format,
                    std::string dir,
                    bool is_save = false){
        m_show = show;
        m_format = format;
        m_is_save_data = is_save;

        m_wakeup_data = new int16_t[m_cache_samples];
        m_doa = doa;
        m_dir = dir;
    }
    ~KeywordObserver(){
        delete[] m_wakeup_data;
    }

    void onKeyWordDetected(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
        std::string keyword,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex,
        std::shared_ptr<const std::vector<char>> KWDMetadata) {
            std::cout<< "["
                     << getTimeStamp()
                     << "]"
                     << m_dir
                     << " keyword \""
                     << keyword
                     <<"\" detect! direction: "
                     << m_doa->get()
                     << ", count: "
                     << m_count++
                     << " begin: "
                     << beginIndex
                     << ", end: "
                     << endIndex ;

        m_show->enableShow(AW::Profile::WAKEUPTEST, AW::ProfileFlag::REPLACE);

        if (alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::INVALID_INDEX == beginIndex || !m_is_save_data){
            std::cout << std::endl;
            return;
        }

        auto reader = stream->createReader(alexaClientSDK::avsCommon::utils::sds::InProcessSDS::Reader::Policy::NONBLOCKING);
        if(reader->seek(beginIndex) == false)
            return;

        ssize_t nWords = endIndex - beginIndex;
        if(m_cache_samples < nWords){
            delete m_wakeup_data;
            m_wakeup_data = new int16_t[nWords];
            m_cache_samples = nWords;
        }

        reader->read((void*)m_wakeup_data, nWords);

        std::string file_path = m_dir + getTimeStamp() + ".wav";
        AW::WavUtils wav;
        wav.create(file_path, "wb", m_format.sampleSizeInBits, m_format.numChannels, m_format.sampleRateHz);
        wav.write((char*)m_wakeup_data, nWords);
        wav.release();

        std::cout << "\t, wakup up pcm files: " << file_path << std::endl;
    }
private:
    int m_count = 0;
    bool m_is_save_data = false;
    int16_t *m_wakeup_data = nullptr;
    ssize_t m_cache_samples = 16000*2;
    std::string m_dir;
    alexaClientSDK::avsCommon::utils::AudioFormat m_format;

    std::shared_ptr<AW::IShowManager> m_show;
    std::shared_ptr<AW::DOAInfo> m_doa;
};

/// Observes user input from the console and notifies the interaction manager of the user's intentions.
class ButtonInputManager : public AW::IButtonManager::Observer,
                           public std::enable_shared_from_this<ButtonInputManager>
{
public:
    /**
     * Create a ButtonInputManager.
     *
     * @param interactionManager An instance of the @c InteractionManager used to manage user input.
     * @return Returns a new @c ButtonInputManager, or @c nullptr if the operation failed.
     */
    static std::shared_ptr<ButtonInputManager> create(std::shared_ptr<AW::PlatformAdapter> platformadapter){
        auto button_manger = std::shared_ptr<ButtonInputManager>(new ButtonInputManager(platformadapter));
        platformadapter->getButtonManager()->addButtonObserver(button_manger);

        return button_manger;
    };

    /**
     * Processes user input forever. Returns upon a quit command.
     */
    int run(){ return 0; };
    void stop(){
        m_platformadapter->getButtonManager()->removeButtonObserver(shared_from_this());
    };
private:
    /**
     * Constructor.
     */
    ButtonInputManager(std::shared_ptr<AW::PlatformAdapter> platformadapter){
        m_platformadapter = platformadapter;
        m_platformadapter->getMuteManager()->privacyMute(false);
        m_status = Status::PRIVATE_UNMUTE;
    };

    void onVolumeUp(){};
    void onVolumeDown(){};
    void onMute(){
        if(m_status == Status::PRIVATE_UNMUTE){
            m_status = Status::PRIVATE_MUTE;
            m_platformadapter->getShowManager()->enableShow(AW::Profile::MUTE, AW::ProfileFlag::REPLACE);
            m_platformadapter->getMuteManager()->privacyMute(true);
        }else{
            m_status = Status::PRIVATE_UNMUTE;
            m_platformadapter->getShowManager()->enableShow(AW::Profile::UNMUTE, AW::ProfileFlag::REPLACE);
            m_platformadapter->getMuteManager()->privacyMute(false);
        }
    };
    void onAudioJackPlugIn(){
        m_platformadapter->getAudioJackManager()->doAudioJackPlugIn();
    };
    void onAudioJackPlugOut(){
        m_platformadapter->getAudioJackManager()->doAudioJackPlugOut();
    };

private:
    /// The main interaction manager that interfaces with the SDK.
    std::shared_ptr<AW::PlatformAdapter> m_platformadapter;

    enum class Status
    {
        PRIVATE_MUTE,
        PRIVATE_UNMUTE
    };
    Status m_status{Status::PRIVATE_UNMUTE};
};

static void when_signal(int sig)
{
    switch(sig){
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        {
            printf("signal coming, stop the capture\n");
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

/**
 * This serves as the starting point for the application. This code instantiates the @c UserInputManager and processes
 * user input until the @c run() function returns.
 *
 * @param argc The number of elements in the @c argv array.
 * @param argv An array of @argc elements, containing the program name and all command-line arguments.
 * @return @c EXIT_FAILURE if the program failed to initialize correctly, else @c EXIT_SUCCESS.
 */
int main(int argc, char** argv) {

    signal(SIGHUP,when_signal);
    signal(SIGQUIT,when_signal);
    signal(SIGINT,when_signal);
    signal(SIGPIPE,when_signal);

    std::string pathToConfig;
    std::string pathToInputFolder;
    std::string logLevel;

    if (argc < 2) {
        alexaClientSDK::sampleApp::ConsolePrinter::simplePrint(
            "USAGE: " + std::string(argv[0]) +
            " <path_to_AlexaClientSDKConfig.json> <path_to_inputs_folder> [log_level]");
        return EXIT_FAILURE;
    } else {
        pathToInputFolder = std::string(argv[1]);
        if (3 == argc) {
            logLevel = std::string(argv[2]);
        }
    }

    /*
     * Creating the buffer (Shared Data Stream) that will hold user audio data. This is the main input into the SDK.
     */
    size_t bufferSize = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize(
        BUFFER_SIZE_IN_SAMPLES, WORD_SIZE, MAX_READERS);
    auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>(bufferSize);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> sharedDataStream =
        alexaClientSDK::avsCommon::avs::AudioInputStream::create(buffer, WORD_SIZE, MAX_READERS);

    if (!sharedDataStream) {
        alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create shared data stream!");
        return false;
    }

    alexaClientSDK::avsCommon::utils::AudioFormat compatibleAudioFormat;
    compatibleAudioFormat.sampleRateHz = SAMPLE_RATE_HZ;
    compatibleAudioFormat.sampleSizeInBits = WORD_SIZE * CHAR_BIT;
    compatibleAudioFormat.numChannels = NUM_CHANNELS;
    compatibleAudioFormat.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
    compatibleAudioFormat.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;

    auto platform = AW::PlatformAdapter::create(pathToInputFolder.data());
    if(platform == nullptr) {
        alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to get " + pathToInputFolder + " for configure!");
        return false;
    }

    auto buttonmanager = ButtonInputManager::create(platform);
    if(buttonmanager == nullptr) {
        alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create buttonmanager!");
        return false;
    }
    buttonmanager->run();

    bool is_save_wakeup = false;
    std::string wakeup_data_dir = "";

    std::shared_ptr<alexaClientSDK::sampleApp::MicrophoneWrapperInterface> micWrapper =
        alexaClientSDK::sampleApp::AlsaMicrophoneWrapper::create(sharedDataStream, nullptr, platform);
    if (!micWrapper) {
        alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create AlsaMicrophoneWrapper!");
        return false;
    }

    //std::unique_ptr<alexaClientSDK::kwd::AbstractKeywordDetector> m_keywordDetector{nullptr};
    //const char *detector = platform->getDetectorType();
#if defined(KWD_SENSORY)
    // This observer is notified any time a keyword is detected and notifies the DefaultClient to start recognizing.
    auto sensory_keywordObserver = std::make_shared<KeywordObserver>(platform->getShowManager(),
                                                             platform->getRecorder()->getFilter()->getDOAInfo(),
                                                             compatibleAudioFormat,
                                                             "sensory",
                                                             is_save_wakeup);
    //if(strcmp(detector, "sensory") == 0) {
        auto sensory_keywordDetector = alexaClientSDK::kwd::SensoryKeywordDetector::create(
            sharedDataStream,
            compatibleAudioFormat,
            {sensory_keywordObserver},
            std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>(),
            platform->getSensoryModel());
        if (!sensory_keywordDetector) {
            alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create SensoryKeyWordDetector!");
            return false;
        }
    //}
#endif
#if defined(KWD_AMAZONLITE)
    auto amazonlite_keywordObserver = std::make_shared<KeywordObserver>(platform->getShowManager(),
                                                             platform->getRecorder()->getFilter()->getDOAInfo(),
                                                             compatibleAudioFormat,
                                                             "amazonlite",
                                                             is_save_wakeup);
    //if(strcmp(detector, "amazon-lite") == 0) {
        auto amazonlite_keywordDetector = alexaClientSDK::kwd::PryonLiteKeywordDetector::create(
            sharedDataStream,
            compatibleAudioFormat,
            {amazonlite_keywordObserver},
            std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>(),
            platform->getAmazonliteModel(),
            platform->getAmazonliteDetectThreshold());
        if (!amazonlite_keywordDetector) {
            alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create PryonLiteKeywordDetector!");
            return false;
        }
    //}
#endif
#if 0
    auto tutudetect_keywordObserver = std::make_shared<KeywordObserver>(platform->getShowManager(),
                                                             platform->getRecorder()->getFilter()->getDOAInfo(),
                                                             compatibleAudioFormat,
                                                             "tutudetect",
                                                             is_save_wakeup);
    //if(strcmp(detector, "tutudetect") == 0) {
        auto tutudetect_keywordDetector = alexaClientSDK::kwd::TutuClearKeywordDetector::create(
            platform,
            sharedDataStream,
            compatibleAudioFormat,
            {tutudetect_keywordObserver},
            std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>());
        if (!tutudetect_keywordDetector) {
            alexaClientSDK::sampleApp::ConsolePrinter::simplePrint("Failed to create TutuClearKeywordDetector!");
            return false;
        }
    //}
#endif

    micWrapper->startStreamingMicrophoneData();

    sleep(100000000);

    micWrapper->stopStreamingMicrophoneData();
    buttonmanager->stop();

    return EXIT_SUCCESS;
}
