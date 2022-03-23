cmake_minimum_required(VERSION 3.0)
#project name
PROJECT(tinyxml)

if(TIXML_USE_STL)
    message("Build with Tixml stl")
    add_definitions(-DTIXML_USE_STL)
endif()

if(TIDEBUG)
    message("Build with DEBUG mode")
    add_definitions(-DDEBUG)
endif()


#add library file
ADD_LIBRARY(tinyxml SHARED tinyxml.cpp tinystr.cpp tinyxmlparser.cpp tinyxmlerror.cpp)

ADD_EXECUTABLE(xmltest xmltest.cpp)
TARGET_LINK_LIBRARIES(xmltest tinyxml)
INSTALL(TARGETS tinyxml DESTINATION lib)
INSTALL(TARGETS xmltest DESTINATION bin)
INSTALL(FILES tinyxml.h DESTINATION include)
INSTALL(FILES tinystr.h DESTINATION include)
