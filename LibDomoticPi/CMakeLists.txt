cmake_minimum_required(VERSION 3.5)
project(libDomotic)

set(LIB_NAME "domotic")

#########################################################################################
#	Global compiler options
#########################################################################################

set(CMAKE_CXX_STANDARD 17)
add_compile_options("-fexceptions")
add_compile_options("-Wall")
add_compile_options("-Wno-unknown-pragmas")
add_compile_options("-Wno-psabi")

set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g2 -gdwarf-2" )
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer" )

#########################################################################################
#	Include directories
#########################################################################################

include_directories( "include" )

add_subdirectory(src)

add_subdirectory(test)

#########################################################################################
#
#########################################################################################