cmake_minimum_required(VERSION 3.10)
project(dutil)
set(CMAKE_CXX_STANDARD 17)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCES
  source/dutil/file.cpp
  source/dutil/file.hpp
  source/dutil/mac.cpp
  source/dutil/mac.hpp
  source/dutil/misc.hpp
  source/dutil/platform.hpp
  source/dutil/queue.hpp
  source/dutil/stopwatch.cpp
  source/dutil/stopwatch.hpp
  source/dutil/types.hpp
  )

add_library(${PROJECT_NAME} STATIC ${COMMON_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
  source
  )
