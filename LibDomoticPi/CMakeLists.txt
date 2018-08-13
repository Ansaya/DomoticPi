cmake_minimum_required(VERSION 3.7)

project(libDomoticPi)

set(LIB_NAME "domoticPi")

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -fexceptions -Wno-psabi -Wno-unknown-pragmas -DSPDLOG_FMT_PRINTF" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g2 -gdwarf-2" )
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3" )

file(GLOB LIB_SRCS "src/*.cpp")

file(GLOB LIB_HEADERS "include/*.h")

add_library(${LIB_NAME} SHARED
	${LIB_SRCS}
	${LIB_HEADERS}
)

target_link_libraries(${LIB_NAME} wiringPi pthread mosquitto hap)