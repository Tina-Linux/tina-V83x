cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(avs-fetch-device-sn LANGUAGES CXX)

add_executable(avs-fetch-device-sn main.cpp)

target_link_libraries(avs-fetch-device-sn json-c)

install(TARGETS avs-fetch-device-sn DESTINATION bin)
