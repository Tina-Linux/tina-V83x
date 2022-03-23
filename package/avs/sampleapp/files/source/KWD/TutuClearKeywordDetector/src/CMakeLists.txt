add_definitions("-DACSDK_LOG_MODULE=tutuClearKeywordDetector")
add_library(TUTUCLEAR SHARED
    TutuClearKeywordDetector.cpp)

target_include_directories(TUTUCLEAR  PUBLIC
    "../include")

target_link_libraries(TUTUCLEAR KWD AVSCommon)

INSTALL(TARGETS TUTUCLEAR DESTINATION lib)