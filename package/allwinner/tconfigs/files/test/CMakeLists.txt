cmake_minimum_required(VERSION 3.0)

project(tconfigs-unit-test)

set(GTEST_MIN_VERSION "1.8.0")

find_package(GTest ${GTEST_MIN_VERSION} REQUIRED)

if (NOT GTEST_FOUND)
    message(FATAL_ERROR "GTest not Found")
endif (NOT GTEST_FOUND)

include_directories(${GTEST_INCLUDE_DIRS})

function(Unit_test arg)
    add_executable(unit_test_${arg} unit_test_${arg}.cpp)
    target_link_libraries(unit_test_${arg} tconfigs dl pthread ${GTEST_BOTH_LIBRARIES})
    install(TARGETS unit_test_${arg} DESTINATION bin)
#    if (EXISTS unit_test_${arg}.json)
#        add_custom_command(
#            TARGET unit_test_${arg} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy
#                    ${CMAKE_SOURCE_DIR}/unit_test_${arg}.json
#                    ${CMAKE_CURRENT_BINARY_DIR}/etc/tconfigs/unit_test_${arg}.json)
#    endif()
    if (EXISTS "${PROJECT_SOURCE_DIR}/config/unit_test_${arg}.json")
        install(FILES ${PROJECT_SOURCE_DIR}/config/unit_test_${arg}.json
            DESTINATION etc/tconfigs/test)
    endif()
endfunction()

Unit_test(json_utils)
#Unit_test(pipeline)
#Unit_test(file_src_file_sink)
#Unit_test(alsa_src_alsa_sink)
#Unit_test(file_src_converter_file_sink)
Unit_test(signal_slot)
Unit_test(any)
Unit_test(event_loop)
Unit_test(input_utils)
Unit_test(input_event_watcher)
Unit_test(key_handler)
Unit_test(amixer_utils)
Unit_test(volume_manager)
#Unit_test(wav_utils)
Unit_test(bus)
