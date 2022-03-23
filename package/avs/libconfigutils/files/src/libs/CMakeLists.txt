cmake_minimum_required(VERSION 3.0)

#project name
PROJECT(ConfigUtils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

#head file path
INCLUDE_DIRECTORIES(
    ../include
)

#source directories
#AUX_SOURCE_DIRECTORY(. DIR_SRCS)
AUX_SOURCE_DIRECTORY(./threading THREADING_SRCS)
AUX_SOURCE_DIRECTORY(./watcher WATCHER_SRCS)
AUX_SOURCE_DIRECTORY(./i2c I2C_SRCS)
AUX_SOURCE_DIRECTORY(./leds LEDS_SRCS)
AUX_SOURCE_DIRECTORY(./ledshow SHOW_SRCS)
AUX_SOURCE_DIRECTORY(./utils UTILS_SRCS)

#add library file
ADD_LIBRARY(ConfigUtils SHARED
			"${DIR_SRCS}"
			"${THREADING_SRCS}"
			"${WATCHER_SRCS}"
			"${I2C_SRCS}"
			"${LEDS_SRCS}"
			"${SHOW_SRCS}"
			"${UTILS_SRCS}"
			recorder/provider/AudioCodecProvider.cpp
			recorder/provider/AC108Provider.cpp
			recorder/provider/FileProvider.cpp
			recorder/convertor/AudioDataConvertor.cpp
			recorder/convertor/InterleavedConvertor.cpp
			recorder/convertor/NonInterleavedConvertor.cpp
			recorder/PlatformRecorder.cpp
			configs/ConfigOverlay.cpp
			recorder/filter/TutuClearFilter.cpp
			platformadapter/PlatformAdapter.cpp
			gpio/GpioHandler.cpp
			amixer/AmixerHandler.cpp
			mute/MuteManager.cpp
			audiojack/AudioJackManager.cpp
			button/ButtonManager.cpp
)

TARGET_LINK_LIBRARIES(ConfigUtils ev ugpio asound resamplerate tutuClear tutuClearWrapper)

INSTALL(TARGETS ConfigUtils DESTINATION lib)
INSTALL(DIRECTORY ../include DESTINATION .)
