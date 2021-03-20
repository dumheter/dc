cmake_minimum_required(VERSION 3.10)
project(fmt CXX)
set(CMAKE_CXX_STANDARD 17)

option(FMT_STATIC_LIB "Static library?" ON)

set(FMT_SOURCES
  include/fmt/args.h
  include/fmt/chrono.h
  include/fmt/color.h
  include/fmt/compile.h
  include/fmt/core.h
  include/fmt/format.h
  include/fmt/format-inl.h
  include/fmt/locale.h
  include/fmt/os.h
  include/fmt/ostream.h
  include/fmt/posix.h
  include/fmt/printf.h
  include/fmt/ranges.h
  src/format.cc
  src/os.cc
  )

if (FMT_STATIC_LIB)
  add_library(${PROJECT_NAME} STATIC ${FMT_SOURCES})
else ()
  add_library(${PROJECT_NAME} SHARED ${FMT_SOURCES})
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}>)
