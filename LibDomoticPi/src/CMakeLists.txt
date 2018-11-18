
find_package(RapidJSON REQUIRED)
find_package(spdlog REQUIRED)

add_library(libDomotic SHARED)

target_compile_definitions(libDomotic "-DSPDLOG_FMT_PRINTF")

target_include_directories(libDomotic PRIVATE RapidJSON spdlog)

target_link_libraries(libDomotic PRIVATE RapidJSON spdlog)

add_subdirectory( IComm )
add_subdirectory( IModule )

target_sources(libDomotic PRIVATE $<TARGET_OBJECTS:IModule>)