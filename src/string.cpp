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
#include <dc/math.hpp>
#include <dc/string.hpp>

namespace dc {

String::String(IAllocator& allocator)
    : m_allocator(&allocator), m_smallString() {
  setState(State::SmallString);
  m_smallString.clear();
}

String::String(const char* str, IAllocator& allocator)
    : String(str, strlen(str), allocator) {}

String::String(const char* str, usize size, IAllocator& allocator)
    : m_allocator(&allocator) {
  if (size >= SmallString::kSize) {
    m_bigString = BigString();
    m_bigString.string = static_cast<u8*>(m_allocator->alloc(size + 1));
    memcpy(m_bigString.string, str, size);
    m_bigString.string[size] = 0;
    m_bigString.size = size;
    m_bigString.capacity = size;
    setState(State::BigString);
  } else {
    m_smallString = SmallString();
    memcpy(&m_smallString.string[0], str, size);
    m_smallString.setSize(size);
    m_smallString.string[size] = 0;
    setState(State::SmallString);
  }
}

String::String(String&& other) noexcept { take(std::move(other)); }

String::~String() { destroyAndClear(); }

String& String::operator=(String&& other) noexcept {
  if (this != &other) {
    destroyAndClear();

    take(std::move(other));
  }

  return *this;
}

void String::operator=(const char* str) {
  const usize size = strlen(str);

  if (getState() == State::BigString) {
    if (size < m_bigString.capacity) {
      // big -> big (fits in capacity)
      memcpy(m_bigString.string, str, size);
      m_bigString.string[size] = 0;
      m_bigString.size = size;
    } else {
      // big -> big (need realloc)
      m_bigString.string =
          static_cast<u8*>(m_allocator->realloc(m_bigString.string, size + 1));
      m_bigString.string[size] = 0;
      m_bigString.size = size;
      m_bigString.capacity = size;
    }
  } else {
    if (size >= SmallString::kSize) {
      // small -> big
      setState(State::BigString);
      m_bigString = BigString();
      m_bigString.string = static_cast<u8*>(m_allocator->alloc(size + 1));
      m_bigString.string[size] = 0;
      m_bigString.size = size;
      m_bigString.capacity = size;
    } else {
      // small -> small
      m_smallString = SmallString();
      memcpy(&m_smallString.string[0], str, size);
      m_smallString.setSize(size);
      m_smallString.string[size] = 0;
    }
  }
}

usize String::getSize() const {
  return getState() == State::BigString ? m_bigString.size
                                        : m_smallString.getSize();
}

bool String::isEmpty() const {
  return getState() == State::BigString ? m_bigString.size == 0
                                        : m_smallString.isEmpty();
}

const char* String::c_str() const {
  const u8* data = getState() == State::BigString ? m_bigString.string
                                                  : &m_smallString.string[0];
  return reinterpret_cast<const char*>(data);
}

void String::SmallString::setSize(usize newSize) {
  DC_ASSERT(newSize <= kSize, "tried to set a size larger than the capacity");
  string[kSize - 1] = static_cast<u8>(kSize - newSize);
}

void String::SmallString::clear() {
  string[0] = 0;
  setSize(kSize);
}

String::State String::getState() const {
  const auto ptr = reinterpret_cast<uintptr_t>(m_allocator);
  auto state = static_cast<State>(ptr & kFlagStateMask);
  return state;
}

void String::setState(State state) {
  auto ptr = reinterpret_cast<uintptr_t>(m_allocator);
  const bool boolState = static_cast<bool>(state);
  ptr = setBit(ptr, kFlagStateBit, boolState);
  m_allocator = (IAllocator*)ptr;
}

IAllocator& String::getAllocator() {
  const auto ptr = reinterpret_cast<uintptr_t>(m_allocator);
  auto out = reinterpret_cast<IAllocator*>(ptr - (ptr & kFlagStateMask));
  return *out;
}

void String::destroyAndClear() {
  if (getState() == State::BigString) {
    m_allocator->free(m_bigString.string);
    // do cleanup for easier debugging
    m_bigString.size = 0;
    m_bigString.capacity = 0;
  } else {
    // do cleanup for easier debugging
    m_smallString.clear();
  }
}

void String::take(String&& other) {
  m_allocator = other.m_allocator;

  if (other.getState() == State::BigString) {
    m_bigString = other.m_bigString;
    other.m_bigString.string = nullptr;
    other.m_bigString.size = 0;
    other.m_bigString.capacity = 0;
  } else {
    m_smallString = other.m_smallString;
    m_smallString.clear();
  }
}

}  // namespace dc
