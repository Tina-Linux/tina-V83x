remove_definitions(-DENABLE_ESP)

add_executable(sample-wakeup main.cpp)

# AVS C++ SDK support
pkg_check_modules(AVS_CPP_SDK REQUIRED AlexaClientSDK=1.7.1)
#message(AUTHOR_WARNING "AVS_CPP_SDK_LIBRARIES: ${AVS_CPP_SDK_LIBRARIES}")

target_include_directories(sample-wakeup  PUBLIC
    ../sampleapp/include)

target_link_libraries(sample-wakeup "${AVS_CPP_SDK_LIBRARIES}" tutuClear)

target_link_libraries(sample-wakeup sampleapp)

install(TARGETS sample-wakeup DESTINATION bin)