set(SampleApp_SOURCES)
list(APPEND SampleApp_SOURCES
    ConnectionObserver.cpp
    ConsolePrinter.cpp
    GuiRenderer.cpp
    InteractionManager.cpp
    KeywordObserver.cpp
    PortAudioMicrophoneWrapper.cpp
    UIManager.cpp
    UserInputManager.cpp
    SampleApplication.cpp
    AlsaMicrophoneWrapper.cpp
	LedRingManager.cpp
    ButtonInputManager.cpp)

add_library(sampleapp SHARED ${SampleApp_SOURCES})
add_executable(SampleApp ${SampleApp_SOURCES} main.cpp)

find_package(PkgConfig)
# AVS C++ SDK support
pkg_check_modules(AVS_CPP_SDK REQUIRED AlexaClientSDK=1.7.1)

include_directories(
    "${PORTAUDIO_INCLUDE_DIR}"
    "${GST_INCLUDE_DIRS}"
    "../include"
    ../../KWD/TutuClearKeywordDetector/include
)

target_link_libraries(sampleapp
    "${AVS_CPP_SDK_LIBRARIES}"
    "${PORTAUDIO_LIB_PATH}"
    TUTUCLEAR
    ConfigUtils
)

target_link_libraries(SampleApp sampleapp)

install(TARGETS sampleapp DESTINATION lib)
install(TARGETS SampleApp DESTINATION bin)
INSTALL(DIRECTORY ../include DESTINATION .)
