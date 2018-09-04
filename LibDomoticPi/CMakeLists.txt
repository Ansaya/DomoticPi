project(libDomoticPi)
cmake_minimum_required(VERSION 3.7)

set(LIB_NAME "domoticPi")

file(GLOB_RECURSE LIB_SRCS "srcs/*.cpp")
file(GLOB_RECURSE LIB_HEADERS "include/*.h")

add_definitions("-DSPDLOG_FMT_PRINTF")
add_compile_options("-Wall" "-Wno-unknown-pragmas" "-fexceptions" "-std=c++17")

set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g2 -gdwarf-2" )
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3" )

include_directories("include")

link_libraries(wiringPi pthread mosquitto hap)

add_library(${LIB_NAME}_static STATIC ${LIB_SRCS})
set_target_properties(${LIB_NAME}_static PROPERTIES OUTPUT_NAME ${LIB_NAME})

add_library(${LIB_NAME} SHARED ${LIB_SRCS})

### Installation
install(TARGETS ${LIB_NAME}
        LIBRARY DESTINATION /usr/local/lib
)

install(TARGETS ${LIB_NAME}_static
        ARCHIVE DESTINATION /usr/local/lib
)

install(FILES ${LIB_HEADERS}
        DESTINATION /usr/local/include/${LIB_NAME}
)