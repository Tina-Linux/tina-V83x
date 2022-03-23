set(SampleApp_SOURCES)
list(APPEND SampleApp_SOURCES
    ConnectionObserver.cpp
    ConsolePrinter.cpp
    ConsoleReader.cpp
    GuiRenderer.cpp
    InteractionManager.cpp
    KeywordObserver.cpp
    UIManager.cpp
    UserInputManager.cpp
    SampleApplication.cpp
    SampleEqualizerModeController.cpp
    main.cpp)

if (PORTAUDIO)
    list(APPEND SampleApp_SOURCES PortAudioMicrophoneWrapper.cpp)
endif()

if (TCONFIGS)
    list(APPEND SampleApp_SOURCES
        TconfigsPlatformAdapter.cpp
        TconfigsMicrophoneWrapper.cpp
        TconfigsButtonInputManager.cpp)
endif()

IF (A4B)
    list(APPEND SampleApp_SOURCES RevokeAuthorizationObserver.cpp)
ENDIF()

if (PCC)
    list(APPEND SampleApp_SOURCES PhoneCaller.cpp)
endif()

IF (HAS_EXTERNAL_MEDIA_PLAYER_ADAPTERS)
    file(GLOB_RECURSE SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ExternalMediaAdapterRegistration/*.cpp)
    foreach(myfile ${SRC_FILE})
       list(APPEND SampleApp_SOURCES ${myfile})
    endforeach(myfile)
ENDIF()

add_executable(SampleApp ${SampleApp_SOURCES})

target_include_directories(SampleApp PUBLIC
    "${SampleApp_SOURCE_DIR}/include"
    "${AudioResources_SOURCE_DIR}/include"
    "${RegistrationManager_SOURCE_DIR}/include"
    "${ESP_SOURCE_DIR}/include"
    "${GST_INCLUDE_DIRS}"
    "${PORTAUDIO_INCLUDE_DIR}")

find_package(PkgConfig)
pkg_check_modules(AVS_CPP_SDK REQUIRED AlexaClientSDK>=1.10.0)

target_link_libraries(SampleApp
    "${AVS_CPP_SDK_LIBRARIES}"
)

if (TCONFIGS)
    target_link_libraries(SampleApp tconfigs)
endif()

if (PCC)
    target_link_libraries(SampleApp AVSCommon)
endif()

if (ANDROID)
    target_include_directories(SampleApp PUBLIC "${AndroidUtilities}/include")
    target_link_libraries(SampleApp AndroidUtilities)
    if (ANDROID_MEDIA_PLAYER)
        target_include_directories(SampleApp PUBLIC "${AndroidSLESMediaPlayer}/include")
        target_link_libraries(SampleApp AndroidSLESMediaPlayer)
    endif()
endif()

if (GSTREAMER_MEDIA_PLAYER)
    target_include_directories(SampleApp PUBLIC "${MediaPlayer_SOURCE_DIR}/include")
    target_link_libraries(SampleApp MediaPlayer)
endif()

if(KWD)
    target_link_libraries(SampleApp KeywordDetectorProvider)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(SampleApp
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreServices"
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(SampleApp
      rt m pthread asound)
endif()

install(TARGETS SampleApp DESTINATION bin)
