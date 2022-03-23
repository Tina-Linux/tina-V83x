cmake_minimum_required(VERSION 3.0)
PROJECT(libtutuClear)

INCLUDE_DIRECTORIES(..)

LINK_DIRECTORIES(..)

#add library file
ADD_LIBRARY(tutuClearWrapper SHARED tutu_tool.c)

INSTALL(TARGETS tutuClearWrapper DESTINATION lib)