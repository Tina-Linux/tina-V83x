CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

#project name
PROJECT(tutuclear-test)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

LINK_DIRECTORIES(../lib/ ../)
INCLUDE_DIRECTORIES(..)

ADD_EXECUTABLE(tutuclear-test main.cpp)
TARGET_LINK_LIBRARIES(tutuclear-test sensory tutuClear tutuClearWrapper)

INSTALL(TARGETS tutuclear-test DESTINATION bin)
