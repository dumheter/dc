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

// ========================================================================== //
// MACROS
// ========================================================================== //

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

#define DC_FILENAME dc::filenameFromPath(__FILE__)

#define DC_UNUSED(variable) (void)variable

#if defined(DC_STATIC_LIB)
#define DC_EXPORT
#else
#define DC_EXPORT __declspec(dllexport)
#endif

#define DC_FORCE_INLINE __forceinline

#define DC_NOT_IMPLEMENTED DC_ASSERT(false, "not implemented");
