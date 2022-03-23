cmake_minimum_required(VERSION 3.0)
#project name

function(Foo arg)
	ADD_EXECUTABLE(${arg} unit_${arg}.cpp)
	TARGET_LINK_LIBRARIES(${arg} ConfigUtils gtest gmock)
	INSTALL(TARGETS ${arg} DESTINATION bin)
endfunction()

PROJECT(unit-test)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
#head file path
INCLUDE_DIRECTORIES(../include)

LINK_DIRECTORIES(.)

#ADD_EXECUTABLE(test_main main.cpp)
#INSTALL(TARGETS test_main DESTINATION bin)

#Foo(test_eventwatch)
#Foo(test_executor)
#Foo(test_element)
#Foo(test_inputeventwatch)
#Foo(test_keyeventwatch)

#Foo(test_i2c_adapter)
#Foo(test_led)
#Foo(test_ledgroup)
#Foo(test_ledpad)
Foo(test_show)
Foo(test_show_manager)

Foo(test_ac108provider)
Foo(test_audiodataconvertor)
Foo(test_configoverlay)
Foo(test_tutuclearfilter_ac108provider)
Foo(test_tutuclearfilter_fileprovider)
Foo(test_audiodataconvertor_fileprovider)
Foo(test_tutuclearfilter_double_fileprovider)
Foo(test_tutuclearfilter_ac108_audiocodec_provider)
Foo(test_platfromadapter)
