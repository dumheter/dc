cmake_minimum_required(VERSION 3.10)
project(dc)
set(CMAKE_CXX_STANDARD 17)

#//////////////////////////////////////////////////////////////////////////////
#// Options

option(DC_STATIC_LIB "Should dc be compiled to a static library?" ON)
option(DC_BUILD_TESTS "Should the test build be generated?" ON) # todo change default to off
option(DC_ADD_FMT "DC brings in fmt, if you are providing it yourself, turn this off." ON)
option(DC_ENABLE_LIB_DTEST "DTest is a testing library. Turn this off to disable any code related to dtest." ON)

#//////////////////////////////////////////////////////////////////////////////
#// Override compile flags

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// List source files

set(DC_SOURCES
  include/dc/assert.hpp
  include/dc/file.hpp
  include/dc/mac.hpp
  include/dc/misc.hpp
  include/dc/platform.hpp
  include/dc/queue.hpp
  include/dc/result.hpp
  include/dc/stopwatch.hpp
  include/dc/traits.hpp
  include/dc/types.hpp
  src/assert.cpp
  src/file.cpp
  src/mac.cpp
  src/stopwatch.cpp
  )

set(DTEST_SOURCES
  include/dc/dtest.hpp
  src/dtest.cpp
  )

#//////////////////////////////////////////////////////////////////////////////
#// Declare libraries / executables

if (DC_STATIC_LIB)
  add_library(${PROJECT_NAME} STATIC ${DC_SOURCES})
else ()
  add_library(${PROJECT_NAME} ${DC_SOURCES})
endif ()

target_include_directories(${PROJECT_NAME} INTERFACE include PRIVATE include)

if (DC_ENABLE_LIB_DTEST)
  add_library(dtest STATIC ${DTEST_SOURCES})
  target_include_directories(dtest INTERFACE include PRIVATE include)
endif ()

if (DC_BUILD_TESTS)
  if (DC_ENABLE_LIB_DTEST)
    add_subdirectory(tests)
  else ()
    message(FATAL_ERROR "Unable to build tests when the testing library 'dtest' is disabled")
  endif ()
endif ()
