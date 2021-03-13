cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

include_directories(.)

set(TEST_SOURCES
  main.test.cpp
  result.test.cpp
  result.option.test.cpp
  dtest.cpp
  track_lifetime.test.cpp
  )

add_executable (dutil_test ${TEST_SOURCES})

target_link_libraries(dutil_test dutil)
