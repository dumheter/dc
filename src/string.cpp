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
#include <dc/assert.hpp>
#include <dc/math.hpp>
#include <dc/string.hpp>
#include <dc/utf.hpp>

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
    if (m_bigString.string) {
      memcpy(m_bigString.string, str, size);
      m_bigString.string[size] = 0;
      m_bigString.size = size;
      m_bigString.capacity = size;
      setState(State::BigString);
    } else {
      onAllocFailed();
    }
  } else {
    m_smallString = SmallString();
    memcpy(&m_smallString.string[0], str, size);
    m_smallString.setSize(size);
    m_smallString.string[size] = 0;
    setState(State::SmallString);
  }
}

String::String(String&& other) noexcept { take(dc::move(other)); }

String::~String() { destroyAndClear(); }

String& String::operator=(String&& other) noexcept {
  if (this != &other) {
    destroyAndClear();

    take(dc::move(other));
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
      if (m_bigString.string) {
        m_bigString.string[size] = 0;
        m_bigString.size = size;
        m_bigString.capacity = size;
      } else {
        onAllocFailed();
      }
    }
  } else {
    if (size >= SmallString::kSize) {
      // small -> big
      setState(State::BigString);
      m_bigString = BigString();
      m_bigString.string = static_cast<u8*>(m_allocator->alloc(size + 1));
      if (m_bigString.string) {
        m_bigString.string[size] = 0;
        m_bigString.size = size;
        m_bigString.capacity = size;
      } else {
        onAllocFailed();
      }
    } else {
      // small -> small
      m_smallString = SmallString();
      memcpy(&m_smallString.string[0], str, size);
      m_smallString.setSize(size);
      m_smallString.string[size] = 0;
    }
  }
}

const char* String::c_str() const {
  const u8* data = getState() == State::BigString ? m_bigString.string
                                                  : &m_smallString.string[0];
  return reinterpret_cast<const char*>(data);
}

const u8* String::getData() const {
  const u8* data = getState() == State::BigString ? m_bigString.string
                                                  : &m_smallString.string[0];
  return data;
}

u8* String::getData() {
  u8* data = getState() == State::BigString ? m_bigString.string
                                            : &m_smallString.string[0];
  return data;
}

usize String::getSize() const {
  return getState() == State::BigString ? m_bigString.size
                                        : m_smallString.getSize();
}

usize String::getLength() const {
  const u8* data = getData();
  usize length = 0;

  for (usize i = 0; i < getSize(); ++length) {
    utf8::CodePoint cp;
    utf8::CodeSize cpSize = utf8::decode(data, i, cp);
    i += cpSize;
  }

  return length;
}

usize String::getCapacity() const {
  return getState() == State::BigString ? m_bigString.capacity
                                        : m_smallString.getCapacity();
}

bool String::isEmpty() const {
  return getState() == State::BigString ? m_bigString.size == 0
                                        : m_smallString.isEmpty();
}

bool String::operator==(const String& other) const {
  return getSize() == other.getSize() &&
         memcmp(getData(), other.getData(), getSize()) == 0;
}

bool String::operator!=(const String& other) const { return !(*this == other); }

bool String::operator==(const char* other) const {
  return getSize() == strlen(other) && memcmp(getData(), other, getSize()) == 0;
}

bool String::operator!=(const char* other) const { return !(*this == other); }

void String::insert(const u8* str, usize size, usize offset) {
  if (getCapacity() > size + offset) /* > includes the null terminator */ {
    memcpy(getData() + offset, str, size);

    const usize newSize = max(getSize(), size + offset);
    m_smallString.setSize(newSize);
    getData()[newSize] = 0;
  } else {
    const usize newSize = offset + size + 1 /* null termniator */;

    if (getState() == State::BigString) {
      // big -> big (realloc)
      m_bigString.string =
          static_cast<u8*>(m_allocator->realloc(m_bigString.string, newSize));
    } else {
      // small -> big (alloc)
      u8 temp[m_smallString.kSize];
      memcpy(temp, m_smallString.string, m_smallString.kSize);
      setState(State::BigString);
      m_bigString = BigString();
      m_bigString.string = static_cast<u8*>(m_allocator->alloc(newSize));
      memcpy(m_bigString.string, temp, m_smallString.kSize);
    }

    memcpy(m_bigString.string + offset, str, size);
    m_bigString.string[offset + size] = 0;
    m_bigString.size = offset + size;
    m_bigString.capacity = newSize;
  }
}

void String::insert(const char* str, usize offset) {
  const usize size = strlen(str);
  insert(reinterpret_cast<const u8*>(str), size, offset);
}

void String::append(const u8* str, usize len) { insert(str, len, getSize()); }

void String::operator+=(u8 codePoint) { append(&codePoint, 1); }

void String::operator+=(const char* str) {
  append(reinterpret_cast<const u8*>(str), strlen(str));
}

void String::SmallString::setSize(usize newSize) {
  DC_ASSERT(newSize <= kSize, "tried to set a size larger than the capacity");
  string[kSize - 1] = static_cast<u8>(kSize - newSize);
}

void String::SmallString::clear() {
  string[0] = 0;
  setSize(0);
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

bool operator==(const char* a, const dc::String& b) { return b.operator==(a); }

void String::onAllocFailed() {
  constexpr const char* const kAllocFailedString = "MEMORY ALLOC FAILED";
  constexpr usize kAllocFailedStringSize = 19;
  static_assert(kAllocFailedStringSize < SmallString::kSize);

  m_smallString = SmallString();
  memcpy(&m_smallString.string[0], kAllocFailedString, kAllocFailedStringSize);
  m_smallString.setSize(kAllocFailedStringSize);
  m_smallString.string[kAllocFailedStringSize] = 0;
  setState(State::SmallString);
}

}  // namespace dc
