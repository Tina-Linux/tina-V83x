cmake_minimum_required(VERSION 3.0)

project(tconfigs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_VERBOSE_MAKEFILE ON)

include(utils.cmake)

set(ALL_LIBRARY_HDRS_DIR "./include")

include_directories(
    ${ALL_LIBRARY_HDRS_DIR}
)

aux_source_directory(./src/log LOG_SRCS)
aux_source_directory(./src/common COMMON_SRCS)
aux_source_directory(./src/json JSON_SRCS)
aux_source_directory(./src/file FILE_SRCS)
aux_source_directory(./src/threading THREADING_SRCS)
aux_source_directory(./src/audio/utils AUDIO_UTILS_SRCS)
aux_source_directory(./src/audio/common AUDIO_COMMON_SRCS)
aux_source_directory(./src/audio/elements AUDIO_ELEMENTS_SRCS)
aux_source_directory(./src/audio/factory AUDIO_FACTORY_SRCS)
aux_source_directory(./src/audio/volume AUDIO_VOLUME_SRCS)
aux_source_directory(./src/event EVENT_SRCS)
aux_source_directory(./src/input INPUT_SRCS)

set(BASE_LIBRARIES asound ev)

if (NOT TCONFIGS_LOG_TYPE)
    set(TCONFIGS_LOG_TYPE "printf")
endif()

if (${TCONFIGS_LOG_TYPE} STREQUAL "syslog")
    add_definitions("-DTCONFIGS_LOG_SYSLOG")
else()
    add_definitions("-DTCONFIGS_LOG_PRINTF")
endif()

add_third_party_component(TCONFIGS_TUTUCLEAR
    TUTUCLEAR_SRCS "./src/audio/tutuclear"
    TUTUCLEAR_LIBS "tutuClear;tutu_tool")
#message(STATUS "TUTUCLEAR_SRCS = ${TUTUCLEAR_SRCS}")
#message(STATUS "TUTUCLEAR_LIBS = ${TUTUCLEAR_LIBS}")

add_library(${PROJECT_NAME} SHARED
    ${LOG_SRCS}
    ${COMMON_SRCS}
    ${JSON_SRCS}
    ${FILE_SRCS}
    ${THREADING_SRCS}
    ${AUDIO_UTILS_SRCS}
    ${AUDIO_COMMON_SRCS}
    ${AUDIO_ELEMENTS_SRCS}
    ${AUDIO_FACTORY_SRCS}
    ${AUDIO_VOLUME_SRCS}
    ${EVENT_SRCS}
    ${INPUT_SRCS}
# Third party srcs
    ${TUTUCLEAR_SRCS}
)

target_link_libraries(${PROJECT_NAME}
# Third party libraries
    ${TUTUCLEAR_LIBS}
# Base libraries
    ${BASE_LIBRARIES}
)

define_file_basename_for_sources(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY ${ALL_LIBRARY_HDRS_DIR} DESTINATION .)

if (BUILD_UNIT_TEST)
    add_subdirectory(test)
endif()
