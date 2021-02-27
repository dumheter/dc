cmake_minimum_required(VERSION 3.10)
project(dutil)
set(CMAKE_CXX_STANDARD 17)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCES
  include/dutil/file.cpp
  include/dutil/file.hpp
  include/dutil/mac.cpp
  include/dutil/mac.hpp
  include/dutil/misc.hpp
  include/dutil/platform.hpp
  include/dutil/queue.hpp
  include/dutil/stopwatch.cpp
  include/dutil/stopwatch.hpp
  include/dutil/types.hpp
  include/dutil/assert.hpp
  include/dutil/assert.cpp
  )

add_library(${PROJECT_NAME} STATIC ${COMMON_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
  include
  )

option(DUTIL_TEST "Add test compile target." off)

if (DUTIL_TEST)
  add_subdirectory(tests)
endif ()
