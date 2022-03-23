CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

#project name
PROJECT(sensory-test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

if(${ARCH} STREQUAL "aarch64")
	LINK_DIRECTORIES(../wrapper/lib/aarch64-linux-gnu/)
else()
	LINK_DIRECTORIES(../wrapper/lib/arm-linux-gnueabihf/)
endif()

include_directories("../wrapper/include/")

ADD_EXECUTABLE(sensory-test main.cpp)
LINK_DIRECTORIES(../wrapper)

TARGET_LINK_LIBRARIES(sensory-test sensory)

INSTALL(TARGETS sensory-test DESTINATION bin)
