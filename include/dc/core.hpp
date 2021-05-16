/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <dc/assert.hpp>
#include <dc/types.hpp>

namespace dc {

/// Returns pointer to past the last slash in the path string.
constexpr const char* filenameFromPath(const char* path) {
  const char* filename = path;

  while (*path) {
#if !defined(WIN32)
    if (*path++ == '/')
#else
    if (*path++ == '\\')
#endif
      filename = path;
  }

  return filename;
}

}  // namespace dc

///////////////////////////////////////////////////////////////////////////////
// Misc
//

/// Strips the path of '__FILE__', leaving only the filename plus extension.
///
/// Example:
///   'd:/dev/dc/include/dc/core.hpp' -> 'core.hpp'
#define DC_FILENAME dc::filenameFromPath(__FILE__)

#define DC_UNUSED(variable) (void)variable

#define DC_NOT_IMPLEMENTED DC_ASSERT(false, "not implemented");

#if defined(_MSC_VER)
#define DC_COMPILER_MSVC
#elif defined(__clang__)
#define DC_COMPILER_CLANG
#elif defined(__GNUC__)
#define DC_COMPILER_GCC
#endif

#if defined(DC_COMPILER_MSVC)
#define DC_DISABLE_OPTIMIZATIONS __pragma("optimize(\"\", off)")
#elif defined(DC_COMPILER_CLANG)
#define DC_DISABLE_OPTIMIZATIONS _Pragma("clang optimize off")
#elif defined(DC_COMPILER_GCC)
#define DC_DISABLE_OPTIMIZATIONS _Pragma("GCC optimize off")
#else
#define DC_DISABLE_OPTIMIZATIONS
#endif

#if defined(DC_COMPILER_MSVC)
#define DC_NOINLINE __declspec(noinline)
#elif defined(DC_COMPILER_CLANG)
#define DC_NOINLINE __attribute__((noinline))
#elif defined(DC_COMPILER_GCC)
#define DC_NOINLINE __attribute__((noinline))
#else
#error "DC_NOINLINE not defined for your compiler"
#endif

#if defined(DC_COMPILER_MSVC)
#define DC_INLINE __forceinline
#elif defined(DC_COMPILER_CLANG)
#define DC_INLINE __attribute__((always_inline))
#elif defined(DC_COMPILER_GCC)
#define DC_INLINE __attribute__((always_inline))
#else
#error "DC_INLINE not defined for your compiler"
#endif

///////////////////////////////////////////////////////////////////////////////
// ctor & dtor
//

#define DC_DELETE_COPY(ClassName)             \
  ClassName(const ClassName& other) = delete; \
  ClassName& operator=(const ClassName& other) = delete

#define DC_DEFAULT_COPY(ClassName)             \
  ClassName(const ClassName& other) = default; \
  ClassName& operator=(const ClassName& other) = default

#define DC_DELETE_MOVE(ClassName)        \
  ClassName(ClassName&& other) = delete; \
  ClassName& operator=(ClassName&& other) = delete

#define DC_DEFAULT_MOVE(ClassName)        \
  ClassName(ClassName&& other) = default; \
  ClassName& operator=(ClassName&& other) = default
