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

#include <fmt/format.h>

#include <dc/allocator.hpp>
#include <dc/list.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>
#include <dc/utf.hpp>

namespace dc {

///////////////////////////////////////////////////////////////////////////////
// Iterator
//

/// Iterator to a utf-8 encoded string.
class [[nodiscard]] Utf8Iterator {
 public:
  Utf8Iterator(const char8* string, u64 size, s64 offset)
      : m_string(string), m_size(size), m_offset(offset) {}

  /// @return The code point at the current offset.
  [[nodiscard]] utf8::CodePoint operator*();

  [[nodiscard]] bool operator==(const Utf8Iterator& other) const;

  [[nodiscard]] bool operator!=(const Utf8Iterator& other) const;

  /// prefix
  Utf8Iterator& operator++();

  /// prefix
  Utf8Iterator& operator--();

  [[nodiscard]] bool hasValidOffset() const;

  [[nodiscard]] const char8* beginChar8() const { return m_string + m_offset; }

  [[nodiscard]] const char8* endChar8() const { return m_string + m_size; }

 private:
  const char8* m_string = nullptr;
  u64 m_size = 0;
  s64 m_offset = 0;
};

///////////////////////////////////////////////////////////////////////////////
// String View
//

// constexpr u64 kStringViewDynamic = 0;

// template <u64 kSize = kStringViewDynamic>
class [[nodiscard]] StringView {
 public:
  // TODO cgustafsson: How to allow this constexpr size constructor?
  // constexpr StringView(const char (&string)[kSize])
  //    : m_string(string), m_size(kSize - 1) {}
  StringView() = default;

  StringView(const char8* string) : StringView(string, strlen(string)) {}

  StringView(const char8* string, u64 size) : m_string(string), m_size(size) {}

  [[nodiscard]] constexpr const char8* c_str() const { return m_string; }

  [[nodiscard]] constexpr const u8* getData() const {
    return (const u8*)m_string;
  }

  [[nodiscard]] constexpr u64 getSize() const { return m_size; }

  [[nodiscard]] u64 getLength() const;

  [[nodiscard]] constexpr bool isEmpty() const { return m_size == 0; }

  [[nodiscard]] char8 operator[](u64 pos) const;

  // [[nodiscard]] constexpr const char8* begin() const { return m_string; }
  // [[nodiscard]] constexpr const char8* end() const { return m_string +
  // m_size;
  // }
  [[nodiscard]] Utf8Iterator begin() const {
    return Utf8Iterator(m_string, m_size, 0);
  }
  [[nodiscard]] Utf8Iterator end() const {
    return Utf8Iterator(m_string, m_size, m_size);
  }

  [[nodiscard]] const char8* beginChar8() const { return m_string; }
  [[nodiscard]] const char8* endChar8() const { return m_string + m_size; }

  [[nodiscard]] Option<u64> find(StringView pattern) const;

 private:
  const char8* m_string = nullptr;
  u64 m_size = 0;
};

/// String
///
/// Utilizes List for small string optimization.
class [[nodiscard]] String {
 public:
  String(IAllocator& = getDefaultAllocator());
  String(const char8* str, IAllocator& = getDefaultAllocator());
  String(const char8* str, u64 size, IAllocator& = getDefaultAllocator());
  String(StringView view);
  String(String&& other) noexcept;

  /// Construct by taking ownership of an already existing list.
  /// Will null terminate the list
  String(List<char8>&& list) noexcept;

  String& operator=(String&& other) noexcept;

  /// Try to avoid this function, has to measure the length of the c string.
  // TODO cgustafsson: refer user to something else
  void operator=(const char8* str);

  // use clone() instead
  String(const String& other) = delete;
  String& operator=(const String& other) = delete;

  /// Make a copy of this string instance.
  String clone() const;

  StringView toView() const { return StringView(c_str(), getSize()); }

  // TODO cgustafsson:
  // String substr() const;

  // TODO cgustafsson: should this be deleted, or kept for legacy reason?
  [[nodiscard]] const char8* c_str() const;

  // TODO cgustafsson: getData, getRaw, getCStr, CStr, c_str, getString, get
  /// Access raw data of the string. Note that utf-8 encodes its characters
  /// across multiple char's.
  [[nodiscard]] const char8* getData() const;
  [[nodiscard]] char8* getData();

  // TODO cgustafsson: Should this be in terms of raw char's, or in unicode
  // cp's?
  // [[nodiscard]] char8& operator[](const u64 pos) { return m_list[pos]; }
  // [[nodiscard]] const char8& operator[](const u64 pos) const {
  //   return m_list[pos];
  // }

  /// Raw data access. Users responsibility to not disturb utf-8 validity.
  char8 getDataAt(u64 pos) const;
  void setDataAt(u64 pos, char8 data);

  /// @return Number of bytes used by the string, excluding the null terminator.
  [[nodiscard]] u64 getSize() const;

  /// @return Number of unicode code points used by the string, excluding null
  /// terminator.
  [[nodiscard]] u64 getLength() const;

  /// @return Number of bytes that is reserved for, and usable by, the string.
  /// Excluding the null terminator.
  [[nodiscard]] u64 getCapacity() const;

  /// @return Is the string empty, excluding the null terminator.
  [[nodiscard]] bool isEmpty() const;

  [[nodiscard]] bool operator==(const String& other) const;
  [[nodiscard]] bool operator!=(const String& other) const;

  [[nodiscard]] bool operator==(const char8* other) const;
  [[nodiscard]] bool operator!=(const char8* other) const;

  void operator+=(const String& other);
  void operator+=(u8 data);
  void operator+=(const char8* str);

  [[nodiscard]] Utf8Iterator begin() {
    return Utf8Iterator(c_str(), getSize(), 0);
  }

  [[nodiscard]] Utf8Iterator end() {
    return Utf8Iterator(c_str(), getSize(), getSize());
  }

  /// Append to the back of the string.
  void append(const char8* str, u64 size);

  /// Insert into the string, may allocate if needed. Will overwrite if not
  /// at end of string.
  /// @param size Byte size of str, without the null termination.
  void insert(const char8* str, u64 size, u64 offset);
  void insert(const char8* str, u64 offset);

  /// Resize the internal buffer. Will also put a null terminator after size.
  /// @param size The new size. The internal buffer size will be size + 1, to
  /// also contain the null terminator.
  /// @return The new size.
  u64 resize(u64 size);

  /// @retval Some with position if found
  /// @retval None if not found
  Option<u64> find(StringView pattern) const;

  List<char8>& getInternalList() { return m_list; }
  const List<char8>& getInternalList() const { return m_list; }

 private:
  List<char8> m_list;
};

bool operator==(const char8* a, const dc::String& b);

}  // namespace dc

///////////////////////////////////////////////////////////////////////////////
// Fmt
//

namespace dc {

String vformat(fmt::string_view formatStr, fmt::format_args args);

template <typename... Args>
inline String format(fmt::string_view formatStr, Args&&... args) {
  return dc::vformat(formatStr, fmt::make_format_args(args...));
}

}  // namespace dc

template <>
struct fmt::formatter<dc::String> : formatter<string_view> {
  template <typename FormatContext>
  auto format(const dc::String& string, FormatContext& ctx) {
    string_view str(string.c_str(), string.getSize());
    return formatter<string_view>::format(str, ctx);
  }
};
