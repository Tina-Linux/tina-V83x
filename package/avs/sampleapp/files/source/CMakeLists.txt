cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(SampleApp LANGUAGES CXX)

include(build/BuildDefaults.cmake)
include(build/cmake/PrepareInstall.cmake)

add_subdirectory("fetch-device-sn")
add_subdirectory("KWD/TutuClearKeywordDetector")
add_subdirectory("sample-wakeup")
add_subdirectory("sampleapp/src")
