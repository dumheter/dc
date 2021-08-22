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
  Utf8Iterator(const char* string, u64 size, s64 offset)
      : m_string(string), m_size(size), m_offset(offset) {}

  /// @return The code point at the current offset.
  [[nodiscard]] utf8::CodePoint operator*();

  //[[nodiscard]] utf8::CodePoint operator->() const;

  [[nodiscard]] bool operator==(const Utf8Iterator& other) const;

  [[nodiscard]] bool operator!=(const Utf8Iterator& other) const;

  /// prefix
  Utf8Iterator& operator++();

  /// prefix
  Utf8Iterator& operator--();

  [[nodiscard]] bool hasValidOffset() const;

 private:
  const char* m_string = nullptr;
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

  StringView(const char* string) : StringView(string, strlen(string)) {}

  StringView(const char* string, u64 size) : m_string(string), m_size(size) {}

  [[nodiscard]] constexpr const char* c_str() const { return m_string; }

  [[nodiscard]] constexpr const u8* getData() const {
    return (const u8*)m_string;
  }

  [[nodiscard]] constexpr u64 getSize() const { return m_size; }

  [[nodiscard]] u64 getLength() const;

  [[nodiscard]] constexpr bool isEmpty() const { return m_size == 0; }

  [[nodiscard]] char operator[](u64 pos) const;

  // [[nodiscard]] constexpr const char* begin() const { return m_string; }
  // [[nodiscard]] constexpr const char* end() const { return m_string + m_size;
  // }
  [[nodiscard]] Utf8Iterator begin() const {
    return Utf8Iterator(m_string, m_size, 0);
  }
  [[nodiscard]] Utf8Iterator end() const {
    return Utf8Iterator(m_string, m_size, m_size);
  }

  [[nodiscard]] Option<u64> find(StringView pattern) const;

 private:
  const char* m_string = nullptr;
  u64 m_size = 0;
};

/// String
///
/// Has small string optimization. It may start as small string, but if it needs
/// to allocate, it will never be small again.
class [[nodiscard]] String {
 public:
  class [[nodiscard]] Iterator {
   public:
    Iterator(u8* data, u64 pos) : m_data(data), m_pos(pos) {}

    [[nodiscard]] u8& operator*() { return m_data[m_pos]; }
    [[nodiscard]] const u8& operator*() const { return m_data[m_pos]; }

    [[nodiscard]] u8* operator->() { return &m_data[m_pos]; }
    [[nodiscard]] const u8* operator->() const { return &m_data[m_pos]; }

    [[nodiscard]] constexpr bool operator==(const Iterator& other) const {
      return m_data == other.m_data && m_pos == other.m_pos;
    }
    [[nodiscard]] constexpr bool operator!=(const Iterator& other) const {
      return m_data != other.m_data || m_pos != other.m_pos;
    }

    constexpr void operator++() { ++m_pos; }

   private:
    u8* m_data;
    u64 m_pos;
  };

  class [[nodiscard]] CIterator {
   public:
    CIterator(const u8* data, u64 pos) : m_data(data), m_pos(pos) {}

    [[nodiscard]] const u8& operator*() const { return m_data[m_pos]; }

    [[nodiscard]] const u8* operator->() const { return &m_data[m_pos]; }

    [[nodiscard]] constexpr bool operator==(const CIterator& other) const {
      return m_data == other.m_data && m_pos == other.m_pos;
    }
    [[nodiscard]] constexpr bool operator!=(const CIterator& other) const {
      return m_data != other.m_data || m_pos != other.m_pos;
    }

    constexpr void operator++() { ++m_pos; }

   private:
    const u8* m_data;
    u64 m_pos;
  };

 public:
  String(IAllocator& = getDefaultAllocator());
  String(const char* str, IAllocator& = getDefaultAllocator());
  String(const char* str, u64 size, IAllocator& = getDefaultAllocator());
  String(StringView view);
  String(String&& other) noexcept;

  ~String();

  String& operator=(String&& other) noexcept;
  void operator=(const char* str);

  // use clone() instead
  String(const String& other);
  String& operator=(const String& other) = delete;

  /// Make a copy of this string instance.
  String clone() const;

  StringView toView() const { return StringView(c_str(), getSize()); }

  // TODO cgustafsson:
  // String substr() const;

  [[nodiscard]] const char* c_str() const;
  [[nodiscard]] const u8* getData() const;
  [[nodiscard]] u8* getData();

  [[nodiscard]] u8& operator[](const u64 pos) { return getData()[pos]; }
  [[nodiscard]] const u8& operator[](const u64 pos) const {
    return getData()[pos];
  }

  [[nodiscard]] u64 getSize() const;
  [[nodiscard]] u64 getLength() const;
  [[nodiscard]] u64 getCapacity() const;

  [[nodiscard]] bool isEmpty() const;

  [[nodiscard]] bool operator==(const String& other) const;
  [[nodiscard]] bool operator!=(const String& other) const;

  [[nodiscard]] bool operator==(const char* other) const;
  [[nodiscard]] bool operator!=(const char* other) const;

  void operator+=(const String& other);
  void operator+=(u8 codePoint);
  void operator+=(const char* str);

  [[nodiscard]] Iterator begin() { return Iterator(getData(), 0); }

  [[nodiscard]] Iterator end() { return Iterator(getData(), getSize()); }

  [[nodiscard]] CIterator cbegin() const { return CIterator(getData(), 0); }
  [[nodiscard]] CIterator cend() const {
    return CIterator(getData(), getSize());
  }

  /// Append to the back of the string.
  void append(const u8* str, u64 size);

  /// Insert into the string, may allocate if needed. Will overwrite if not
  /// at end of string.
  /// @param size Byte size of str, without the null termination.
  void insert(const u8* str, u64 size, u64 offset);
  void insert(const char* str, u64 offset);

  /// Resize the internal buffer.
  /// @param size
  /// @return
  u64 resize(u64 size);

  /// @retval Some with position if found
  /// @retval None if not found
  Option<u64> find(StringView pattern) const;

 private:
  struct [[nodiscard]] BigString {
    u8* string;
    u64 size;
    u64 capacity;
  };
  static_assert(isPod<BigString>);

  struct [[nodiscard]] SmallString {
    static constexpr u64 kSize =
        sizeof(BigString) - 1;  //< last byte reserved for null terminator
    u8 string[sizeof(BigString)];

    [[nodiscard]] constexpr bool isEmpty() const { return getSize() == 0; }
    [[nodiscard]] constexpr u64 getSize() const {
      return kSize - string[kSize];
    }
    [[nodiscard]] constexpr u64 getCapacity() const { return kSize; }
    void setSize(u64 newSize);
    void clear();
  };
  static_assert(isPod<SmallString>);

  enum class State : uintptr_t {
    BigString = 0,
    SmallString = 1,
  };
  static constexpr uintptr_t kFlagStateBit = 0;
  static constexpr uintptr_t kFlagStateMask = 1;

 private:
  [[nodiscard]] State getState() const;
  void setState(State state);

  /// Set the size in the correct string state.
  /// This is the raw internal set size method.
  void setSize(u64 size);

  [[nodiscard]] IAllocator& getAllocator() const;

  /// Clear our data, if we have allocated, free.
  void destroyAndClear();

  /// Take ownership of the other's data.
  void take(String&& other);

  /// When an allocation fails, set our string to a memory alloc failed state.
  void onAllocFailed();

 private:
  IAllocator* m_allocator;  //< used to store flags, do not access directly.

  union {
    BigString m_bigString;
    SmallString m_smallString;
  };
};

bool operator==(const char* a, const dc::String& b);

}  // namespace dc

///////////////////////////////////////////////////////////////////////////////
// Fmt
//

namespace dc {

String vformat(fmt::string_view formatStr, fmt::format_args args);

template <typename... Args>
inline String format(fmt::string_view formatStr, Args... args) {
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
