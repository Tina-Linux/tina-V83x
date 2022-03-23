cmake_minimum_required(VERSION 3.0)
PROJECT(libsensory)

option(ARCH "arch for lib, \"arm\" or \"aarch64\"" "arm")

INCLUDE_DIRECTORIES(chk)

AUX_SOURCE_DIRECTORY(chk DIR_SRCS)

if(${ARCH} STREQUAL "aarch64")
	LINK_DIRECTORIES(./lib/aarch64-linux-gnu/)
else()
	LINK_DIRECTORIES(./lib/arm-linux-gnueabihf/)
endif()

#add library file
ADD_LIBRARY(sensory SHARED "${DIR_SRCS}")

TARGET_LINK_LIBRARIES(sensory "-Wl,--whole-archive" snsr "-Wl,--no-whole-archive" asound)

INSTALL(TARGETS sensory DESTINATION lib)
INSTALL(FILES ./include/snsr.h DESTINATION include)
INSTALL(FILES ./models/thfft_alexa_a_enus_v3_1mb.snsr DESTINATION models)
INSTALL(FILES ./models/thfft_alexa_a_enus_v3_1mb_search_8.snsr DESTINATION models)
