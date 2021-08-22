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

#include <dc/types.hpp>

namespace dc {

// TODO cgustafsson: allow to return a larger set of memory than requested
class IAllocator {
 public:
  virtual ~IAllocator() = default;

  static constexpr usize kMinimumAlignment = sizeof(void*);

  /// @param count Number of bytes to allocate. Note that size = count *
  /// sizeof(type)
  /// @return Pointer to the beginning of the newly allocated buffer.
  /// Or nullptr on failure.
  virtual void* alloc(usize count, usize align = kMinimumAlignment) = 0;

  /// @param data Pointer to memory to be reallocated. If a valid pointer is
  /// returned, then data is invalided.
  /// @return Pointer to the beginning of the newly allocated buffer.
  /// Or nullptr on failure.
  virtual void* realloc(void* data, usize count,
                        usize align = kMinimumAlignment) = 0;

  virtual void free(void* data) = 0;
};

IAllocator& getDefaultAllocator();

/// @return Old allocator
// IAllocator* setDefaultAllocator(IAllocator* newDefaultAllocator);

class GeneralAllocator final : public IAllocator {
 public:
  virtual void* alloc(usize count, usize align) override;

  virtual void* realloc(void* data, usize count, usize align) override;

  virtual void free(void* data) override;
};

}  // namespace dc
