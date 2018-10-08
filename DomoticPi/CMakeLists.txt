project(DomoticPi)
cmake_minimum_required(VERSION 3.7)

add_definitions("-DSPDLOG_FMT_PRINTF")
add_compile_options("-Wall" "-Wno-unknown-pragmas" "-Wno-psabi" "-fexceptions" "-std=c++17")

set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g2 -gdwarf-2" )
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -fomit-frame-pointer" )

include_directories("include")

link_libraries(wiringPi domoticPi hap pthread mosquitto systemd)

add_executable(domoticPiService main.cpp)
set_target_properties(domoticPiService PROPERTIES OUTPUT_NAME domoticPi)

install(TARGETS domoticPiService
	DESTINATION /usr/local/etc/domoticPi
	RENAME domoticPi
)

install(FILES domoticpi.service DESTINATION /etc/systemd/system)