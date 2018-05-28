cmake_minimum_required(VERSION 3.7)

project(libDomoticPi)

set(LIB_NAME "domoticPi")

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexceptions -Wno-psabi -Wno-unknown-pragmas -DSPDLOG_FMT_PRINTF" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fPIC -Wall -g2 -gdwarf-2" )
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fPIC" )

file(GLOB LIB_SRCS "*.cpp")

file(GLOB LIB_HEADERS "*.h")

add_library(${LIB_NAME} SHARED
	${LIB_SRCS}
	${LIB_HEADERS}
)

target_link_libraries(${LIB_NAME} wiringPi pthread)