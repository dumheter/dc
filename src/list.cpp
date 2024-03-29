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

#include <cstring>
#include <dc/list.hpp>

namespace dc::detail {

void* BufferAwareAllocator::alloc(usize count, usize align) {
  m_pair.setInt(kHaveAllocated);
  return m_pair.getPointer()->alloc(count, align);
}

void* BufferAwareAllocator::realloc(void* data, usize count, usize align) {
  // TODO cgustafsson: only if trivially relocatable
  if (m_pair.getInt() == kHaveAllocated) {
    return m_pair.getPointer()->realloc(data, count, align);
  }

  m_pair.setInt(kHaveAllocated);
  void* newData = m_pair.getPointer()->alloc(count, align);
  // TODO / BUG: this user level realloc doesnt work since count is not the 
  // old valid count, its the new count and cannot be used here.
  //memcpy(newData, data, count);
  return newData;
}

void BufferAwareAllocator::free(void* data) {
  if (m_pair.getInt() == kHaveAllocated) m_pair.getPointer()->free(data);
}

}  // namespace dc::detail
