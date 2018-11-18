 find_package(Catch2 REQUIRED)

 add_executable(libDomoticTests test.cpp)
 target_link_libraries(libDomoticTests Catch2::Catch2)

 include(CTest)
 include(Catch)
 catch_discover_tests(foo)