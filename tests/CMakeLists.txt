cmake_minimum_required (VERSION 3.10)
project(dc_test CXX)
set(CMAKE_CXX_STANDARD 17)

#//////////////////////////////////////////////////////////////////////////////
#// Override compile flags

if (WIN32)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// List source files

set(TEST_SOURCES
  main.test.cpp
  result.result.test.cpp
  result.option.test.cpp
  track_lifetime.test.cpp
  log.test.cpp
  time.test.cpp
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
  check_cxx_compiler_flag(-Wzero-as-null-pointer-constant HAS_NULLPTR_WARNING)
  if (HAS_NULLPTR_WARNING)
    set(PEDANTIC_COMPILE_FLAGS ${PEDANTIC_COMPILE_FLAGS}
        -Wzero-as-null-pointer-constant)
  endif ()
  set(WERROR_FLAG -Werror)
endif ()

if (MSVC)
  set(PEDANTIC_COMPILE_FLAGS /W4)
  set(WERROR_FLAG /WX)
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// Declare libraries / executables

target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_DATE=0)
target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_LEVEL=0)
target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_FUNCTION=0)

add_executable (${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} dc dtest)

if (DC_PEDANTIC)
  target_compile_options(${PROJECT_NAME} PRIVATE ${PEDANTIC_COMPILE_FLAGS})
endif ()

if (DC_WERROR)
  target_compile_options(${PROJECT_NAME} PRIVATE ${WERROR_FLAG})
endif ()
