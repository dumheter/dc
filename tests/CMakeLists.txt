cmake_minimum_required (VERSION 3.10)
project(dc_test CXX)

#//////////////////////////////////////////////////////////////////////////////
#// List source files

set(TEST_SOURCES
  assert.test.cpp
  callstack.test.cpp
  debug_allocator.test.cpp
  file.test.cpp
  fmt.test.cpp
  list.test.cpp
  log.test.cpp
  main.test.cpp
  math.test.cpp
  pointer_int_pair.test.cpp
  result.intrusive_option.test.cpp
  result.option.test.cpp
  result.result.test.cpp
  ring.test.cpp
  string.test.cpp
  time.test.cpp
  track_lifetime.test.cpp
  traits.test.cpp
  utf.test.cpp
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
  set(PEDANTIC_COMPILE_FLAGS /W4)
  set(WERROR_FLAG /WX)
endif ()

#//////////////////////////////////////////////////////////////////////////////
#// Declare libraries / executables

target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_DATETIME=2)
target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_LEVEL=0)
target_compile_definitions(dc PRIVATE -DDC_LOG_PREFIX_FUNCTION=0)

add_executable (${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} dc dtest)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if (DC_PEDANTIC)
  target_compile_options(${PROJECT_NAME} PRIVATE ${PEDANTIC_COMPILE_FLAGS})
endif ()

if (DC_WERROR)
  target_compile_options(${PROJECT_NAME} PRIVATE ${WERROR_FLAG})
endif ()
