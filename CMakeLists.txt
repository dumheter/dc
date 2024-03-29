cmake_minimum_required(VERSION 3.10)
project(dc CXX)
set(CMAKE_CXX_STANDARD 17)

#//////////////////////////////////////////////////////////////////////////////
#// Options

option(DC_BUILD_TESTS "Should the test build be generated?" OFF)
option(DC_PEDANTIC "Enable extra compiler checks." ON)
option(DC_WERROR "Compiler warning is error." ON)
option(DC_ENABLE_LIB_DTEST "DTest is a testing library. Turn this off to disable any code related to dtest." OFF)
option(DC_LOG_DEBUG "Enable extra debugging options for dc::log." OFF)
option(DC_COPY_COMPILE_COMMANDS "Copy compile_commands file to top level." OFF)

#//////////////////////////////////////////////////////////////////////////////
#// Moodycamel

add_subdirectory(thirdparty/moodycamel)
include_directories(${PROJECT_NAME} thirdparty/moodycamel/include)

#//////////////////////////////////////////////////////////////////////////////
#// List source files

set(DC_SOURCES
  include/dc/allocator.hpp
  include/dc/assert.hpp
  include/dc/callstack.hpp
  include/dc/list.hpp
  include/dc/log.hpp
  include/dc/file.hpp
  include/dc/mac.hpp
  include/dc/macros.hpp
  include/dc/math.hpp
  include/dc/platform.hpp
  include/dc/result.hpp
  include/dc/string.hpp
  include/dc/time.hpp
  include/dc/traits.hpp
  include/dc/types.hpp
  include/dc/utf.hpp
  include/dc/list.hpp
  src/allocator.cpp
  src/assert.cpp
  src/callstack.cpp
  src/fmt.cpp
  src/log.cpp
  src/file.cpp
  src/mac.cpp
  src/string.cpp
  src/time.cpp
  src/utf.cpp
  src/list.cpp
  )

set(DTEST_SOURCES
  include/dc/dtest.hpp
  src/dtest.cpp
  )


#//////////////////////////////////////////////////////////////////////////////
#// Compiler flags

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(PEDANTIC_COMPILE_FLAGS -pedantic-errors -Wall -Wextra -pedantic
      -Wold-style-cast -Wundef
      -Wredundant-decls -Wwrite-strings -Wpointer-arith
      -Wcast-qual -Wformat=2 -Wmissing-include-dirs
      -Wcast-align
      -Wctor-dtor-privacy -Wdisabled-optimization
      -Winvalid-pch -Woverloaded-virtual
      -Wconversion -Wswitch-enum -Wundef
      -Wno-ctor-dtor-privacy -Wno-format-nonliteral)
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
      set(PEDANTIC_COMPILE_FLAGS ${PEDANTIC_COMPILE_FLAGS} -Wnoexcept
         -Wno-dangling-else -Wno-unused-local-typedefs)
  endif ()
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
      set(PEDANTIC_COMPILE_FLAGS ${PEDANTIC_COMPILE_FLAGS} -Wdouble-promotion
          -Wtrampolines -Wzero-as-null-pointer-constant -Wuseless-cast
          -Wvector-operation-performance -Wsized-deallocation -Wshadow)
  endif ()
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
      set(PEDANTIC_COMPILE_FLAGS ${PEDANTIC_COMPILE_FLAGS} -Wshift-overflow=2
          -Wnull-dereference -Wduplicated-cond)
  endif ()
  set(WERROR_FLAG -Werror)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(PEDANTIC_COMPILE_FLAGS -Wall -Wextra -pedantic -Wconversion -Wundef
      -Wdeprecated -Wweak-vtables -Wshadow)
  if (HAS_NULLPTR_WARNING)
    set(PEDANTIC_COMPILE_FLAGS ${PEDANTIC_COMPILE_FLAGS}
        -Wzero-as-null-pointer-constant)
  endif ()
  set(WERROR_FLAG -Werror)
endif ()

if (MSVC)
  set(PEDANTIC_COMPILE_FLAGS /W3)
  set(WERROR_FLAG /WX)
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// Libraries / Executables

add_library(${PROJECT_NAME} STATIC ${DC_SOURCES})

if (DC_ENABLE_LIB_DTEST OR DC_BUILD_TESTS)
  add_library(dtest STATIC ${DTEST_SOURCES})
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// Target options

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}>)

if (WIN32)
  target_link_libraries(${PROJECT_NAME} )
else ()
  target_link_libraries(${PROJECT_NAME} pthread dl)

  # TODO: bad idea? Forcing extra linker stuff in the exe for better callstack?
  target_link_options(${PROJECT_NAME} PUBLIC -rdynamic)
endif ()

if (DC_BUILD_TESTS)
    target_include_directories(dtest INTERFACE include PRIVATE include)
    add_subdirectory(tests)
endif ()

if (DC_PEDANTIC)
  target_compile_options(${PROJECT_NAME} PRIVATE ${PEDANTIC_COMPILE_FLAGS})
endif ()

if (DC_WERROR)
  target_compile_options(${PROJECT_NAME} PRIVATE ${WERROR_FLAG})
endif ()

if (DC_LOG_DEBUG)
  target_compile_options(${PROJECT_NAME} PRIVATE -DDC_LOG_DEBUG)
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// Copy compile_commands.json to project root
if (DC_COPY_COMPILE_COMMANDS)
  if (WIN32)
    add_custom_target(
      copy-compile-commands ALL
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_CURRENT_LIST_DIR}
      )
  endif ()
endif ()
