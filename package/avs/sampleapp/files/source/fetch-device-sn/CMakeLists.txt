remove_definitions(-DENABLE_ESP)

add_executable(fetch-device-sn main.cpp)

target_link_libraries(fetch-device-sn json-c)

install(TARGETS fetch-device-sn DESTINATION bin)