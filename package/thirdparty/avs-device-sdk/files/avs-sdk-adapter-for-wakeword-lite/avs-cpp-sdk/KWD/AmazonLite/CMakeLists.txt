cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(AMAZONLITE LANGUAGES CXX)

include(../../build/BuildDefaults.cmake)

add_subdirectory("src")
if(NOT AMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING)
	add_subdirectory("test")
endif()
