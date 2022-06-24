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
  utf8::decode(m_string, m_offset, cp);
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
  const utf8::CodeSize codeSize = utf8::decode(m_string, m_offset, cp);
  m_offset += codeSize;

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

std::unordered_map<utf8::CodePoint, u64> calcCharSkip(StringView string,
                                                      StringView pattern) {
  // calculate the step for each character in the search string.
  //
  // example
  // here is a simple example
  //
  // example
  //       ^   e is 0th letter from the back, and it exists
  //           in the search string, it has a skip value of 0.
  //
  // example
  //      ^    l is the 1th letter from the back, and it exists
  //           in the search string, it has a skip value of 1.
  //
  // example
  //  ^       x is the 5th letter from the back, and it exists
  //          in the search string, it has a skip value of 5.
  //
  //
  // The big gain comes from the symbols that are not in the pattern.
  // For example, if we read 't', we can skip forward the entire
  // length of the pattern.
  std::unordered_map<utf8::CodePoint, u64> charSkip;

  for (utf8::CodePoint thisCp : string) {
    Option<u64> posOfLastMatch = None;
    u64 i = 0;
    for (utf8::CodePoint otherCp : pattern) {
      if (thisCp == otherCp) posOfLastMatch = Some(i);
      ++i;
    }
    if (posOfLastMatch.isSome())
      charSkip[thisCp] = pattern.getLength() - posOfLastMatch.value();
  }

  return charSkip;
}

// Boyer-Moore string search
Option<u64> StringView::find(StringView pattern) const {
  if (pattern.isEmpty() || isEmpty() || pattern.getSize() > getSize())
    return None;

  // skip map
  const std::unordered_map<utf8::CodePoint, u64> charSkip =
      calcCharSkip(*this, pattern);

  // for the length of the text
  for (u64 i = pattern.getSize() - 1; i < getSize();) {
    // skip the length of our pattern
    // const u64 patternLength = pattern.getLength();

    // example - 7
    // getIterAt(7 - 1)

    // const Utf8Iterator iter = getIterAt(patternLength - 1);

    // while (iter)

    // compare against current cp
    // if ()

    // utf8::CodePoint thisCp;
    // utf8::CodePoint otherCp;
    // TODO cgustafsson: move forward in the string, in terms of code points
    // if (m_string[lastCharPos] != pattern[lastCharPos])
    // {
    // 	// move forward by precalcualted step
    // 	i += charSkip[]
    // }
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

String::String(String&& other) noexcept : m_list(dc::move(other.m_list)){};

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
