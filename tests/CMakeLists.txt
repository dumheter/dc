cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

///////////////////////////////////////////////////////////////////////////////
// Override compile flags

if (WIN32)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

///////////////////////////////////////////////////////////////////////////////
// List source files

set(TEST_SOURCES
  main.test.cpp
  result.result.test.cpp
  result.option.test.cpp
  dtest.cpp
  track_lifetime.test.cpp
  )

///////////////////////////////////////////////////////////////////////////////
// Declare libraries / executables

add_executable (dc_test ${TEST_SOURCES})
target_link_libraries(dc_test dc)
