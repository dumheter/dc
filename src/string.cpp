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
#include <limits>
#include <unordered_map>

namespace dc {

utf8::CodePoint Utf8Iterator::operator*() const {
  utf8::CodePoint cp;
  utf8::decode(m_string, static_cast<usize>(m_offset), cp);
  return cp;
}

bool Utf8Iterator::operator==(const Utf8Iterator& other) const {
  return (hasValidOffset() == other.hasValidOffset()) &&
         ((m_string + m_offset) == (other.m_string + other.m_offset));
}

bool Utf8Iterator::operator!=(const Utf8Iterator& other) const {
  return !(*this == other);
}

Utf8Iterator& Utf8Iterator::operator++() {
  utf8::CodePoint cp;
  const utf8::CodeSize codeSize =
      utf8::decode(m_string, static_cast<usize>(m_offset), cp);
  m_offset += static_cast<s64>(codeSize);

  return *this;
}

Utf8Iterator& Utf8Iterator::operator--() {
  /*
    To lookup the length backwards, we have to probe until we find a
    non-sequence byte, then we can read the size.

    Char. number range  |        UTF-8 octet sequence
    (hexadecimal)       |        (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

    source: RFC-3629
   */

  // find a valid utf8 character
  Option<utf8::CodeSize> size = None;
  for (s64 i = m_offset - 1; i >= 0 && size.isNone(); --i) {
    size = utf8::validate(m_string + i);
  }

  m_offset -= size.valueOr(1);  // move mimimum one byte

  return *this;
}

bool Utf8Iterator::hasValidOffset() const {
  return m_offset >= 0 && m_offset < static_cast<s64>(m_size);
}

///////////////////////////////////////////////////////////////////////////////

char8 StringView::operator[](u64 pos) const {
  DC_ASSERT(pos <= m_size, "Trying to read outside the StringView memory.");
  return m_string[pos];
}

u64 StringView::getLength() const {
  u64 length = 0;

  for (u64 i = 0; i < getSize(); ++length) {
    utf8::CodePoint cp;
    utf8::CodeSize cpSize = utf8::decode(c_str(), i, cp);
    i += cpSize;
  }

  return length;
}

Option<u64> StringView::find(StringView pattern) const {
  if (pattern.isEmpty() || isEmpty() || pattern.getSize() > getSize())
    return None;

  const u64 patternSize = pattern.getSize();
  const u64 textSize = getSize();

  constexpr u64 kNoOfChars = 256;
  s64 badChar[kNoOfChars];

  for (u64 i = 0; i < kNoOfChars; ++i) {
    badChar[i] = -1;
  }

  for (u64 i = 0; i < patternSize; ++i) {
    badChar[static_cast<u8>(pattern[i])] = static_cast<s64>(i);
  }

  u64 shift = 0;

  while (shift <= textSize - patternSize) {
    s64 j = static_cast<s64>(patternSize) - 1;

    while (j >= 0 && pattern[static_cast<u64>(j)] ==
                         m_string[shift + static_cast<u64>(j)]) {
      j--;
    }

    if (j < 0) {
      return Some(shift);
    }

    const s64 badCharIndex =
        badChar[static_cast<u8>(m_string[shift + static_cast<u64>(j)])];
    const s64 shiftAmount = dc::max(static_cast<s64>(1), j - badCharIndex);
    shift += static_cast<u64>(shiftAmount);
  }

  return None;
}

///////////////////////////////////////////////////////////////////////////////

String::String(IAllocator& allocator) : m_list(allocator) {}

String::String(const char8* str, IAllocator& allocator)
    : String(str, strlen(str), allocator) {}

String::String(const char8* str, u64 size, IAllocator& allocator)
    : m_list(allocator) {
  m_list.resize(size + 1);
  memcpy(m_list.begin(), str, size);
  m_list[size] = 0;  // support non-null terminated strings.
}

String::String(StringView view) : String(view.c_str(), view.getSize()) {}

String::String(String&& other) noexcept : m_list(dc::move(other.m_list)) {};

// TODO cgustafsson: does the forward here skip a move?
String::String(List<char8>&& list) noexcept
    : m_list(dc::forward<List<char8>&&>(list)) {
  if (!m_list.isEmpty()) *(m_list.end() - 1) = 0;
}

String& String::operator=(String&& other) noexcept {
  if (this != &other) {
    m_list = dc::move(other.m_list);
  }
  return *this;
}

void String::operator=(const char8* str) {
  const u64 size = strlen(str);

  m_list.resize(size + 1);
  memcpy(m_list.begin(), str, size + 1);
}

String String::clone() const {
  String out(m_list.clone());
  return out;
}

const char8* String::c_str() const { return m_list.begin(); }

const char8* String::getData() const { return m_list.begin(); }

char8* String::getData() { return m_list.begin(); }

char8 String::getDataAt(u64 pos) const { return m_list[pos]; }

void String::setDataAt(u64 pos, char8 data) { m_list[pos] = data; }

u64 String::getSize() const {
  const auto size = m_list.getSize();
  return (size > 0 ? size - 1 : 0);
}

u64 String::getLength() const {
  u64 length = 0;

  for (u64 i = 0; i < getSize(); ++length) {
    utf8::CodePoint cp;
    utf8::CodeSize cpSize = utf8::decode(c_str(), i, cp);
    i += cpSize;
  }

  return length;
}

u64 String::getCapacity() const {
  const auto capacity = m_list.getCapacity();
  return (capacity > 0 ? capacity - 1 : 0);
}

bool String::isEmpty() const { return getSize() == 0; }

bool String::operator==(const String& other) const {
  return getSize() == other.getSize() &&
         memcmp(getData(), other.getData(), getSize()) == 0;
}

bool String::operator!=(const String& other) const { return !(*this == other); }

bool String::operator==(const char8* other) const {
  const auto size = getSize();
  return size == strlen(other) && memcmp(getData(), other, size) == 0;
}

bool String::operator!=(const char8* other) const { return !(*this == other); }

void String::append(const char8* str, u64 size) {
  insert(str, size, getSize());
}

void String::insert(const char8* str, u64 size, u64 offset) {
  const u64 thisSize = getSize();
  DC_ASSERT(offset <= thisSize,
            "Cannot have an offset larger than the size of String.");
  if (offset + size > thisSize) {
    m_list.resize(size + offset + 1);
    if (m_list.getSize() >= size)
      *(m_list.end() - 1) =
          0;  // we can't assume str is null terminated when size is given.
  }

  if (m_list.getSize() >= size) {
    memcpy(m_list.begin() + offset, str, size);
  }
}

void String::insert(const char8* str, u64 offset) {
  const u64 size = strlen(str);
  insert(str, size, offset);
}

u64 String::resize(u64 size) {
  m_list.resize(size + 1);
  m_list[size] = 0;
  return getSize();
}

Option<u64> String::find(StringView pattern) const {
  StringView thisString(c_str(), getSize());
  return thisString.find(pattern);
}

bool String::endsWith(char8 c) const {
  return isEmpty() ? false : c == *(end() - 1);
}

void String::operator+=(const String& other) {
  append(other.getData(), other.getSize());
}

void String::operator+=(u8 data) { append((char8*)&data, 1); }

void String::operator+=(const char8* str) { append(str, strlen(str)); }

bool operator==(const char8* a, const dc::String& b) { return b.operator==(a); }

}  // namespace dc
