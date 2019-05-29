cmake_minimum_required(VERSION 3.10)
project(dutil)
set(CMAKE_CXX_STANDARD 17)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

set(COMMON_SOURCE
  source/file.cpp
  source/mac.cpp
  source/stopwatch.cpp
  )

add_library(${PROJECT_NAME} STATIC ${COMMON_SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC
  source
  )
