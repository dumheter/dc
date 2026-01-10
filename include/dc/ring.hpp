/**
 * MIT License
 *
 * Copyright (c) 2026 Christoffer Gustafsson
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

#include <dc/allocator.hpp>
#include <dc/math.hpp>
#include <dc/types.hpp>

#include "dc/assert.hpp"
#include "dc/traits.hpp"

namespace dc {

/// Design from https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
template <typename T>
struct Ring {
  Ring(IAllocator& alloc = getDefaultAllocator()) : allocator(alloc) {}

  bool add(T&& elem) {
    if (isFull()) return false;

    data[mask(write++)] = dc::move(elem);
    return true;
  }

  T* add() {
    if (isFull()) return nullptr;

    return &data[mask(write++)];
  }

  T* remove() {
    if (isEmpty()) return nullptr;

    return &data[mask(read++)];
  }

  u32 size() const { return write - read; }

  bool isEmpty() const { return read == write; }

  bool isFull() const { return size() == capacity; }

  T* begin() { return &data[read]; }

  T* end() { return &data[mask(write)]; }

  bool reserve(u32 newCapacity) {
    newCapacity = roundUpToPowerOf2(newCapacity);
    DC_ASSERT(newCapacity < 0x80000000, "Must be less than 31 bits");
    if (capacity == 0) {
      data = static_cast<T*>(allocator.alloc(sizeof(T) * newCapacity));
      capacity = newCapacity;
    } else if (capacity < newCapacity) {
      T* newData = static_cast<T*>(allocator.alloc(sizeof(T) * newCapacity));

      u32 newWrite = 0;
      for (T& elem : *this) {
        newData[newWrite++] = dc::move(elem);
      }

      data = newData;
      capacity = newCapacity;
      write = newWrite;
    }

    return data != nullptr;
  }

  u32 mask(u32 index) const { return index & (capacity - 1); }

  IAllocator& allocator;
  T* data = nullptr;
  u32 capacity = 0;
  u32 read = 0;
  u32 write = 0;
};

}  // namespace dc
