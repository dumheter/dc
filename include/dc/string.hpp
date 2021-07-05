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

#include <dc/allocator.hpp>
#include <dc/core.hpp>
#include <type_traits>

namespace dc {

/// String with (soon) UTF-8 support.
///
/// Has small string optimization. It may start as small string, but if it needs
/// to allocate, it will never be small again.
class [[nodiscard]] String {
 public:
  String(IAllocator& = getDefaultAllocator());
  String(const char* str, IAllocator& = getDefaultAllocator());
  String(const char* str, usize size, IAllocator& = getDefaultAllocator());
  String(String&& other) noexcept;

  ~String();

  [[nodiscard]] String& operator=(String&& other) noexcept;
  void operator=(const char* str);

  // use clone() instead
  String(const String& other) = delete;
  [[nodiscard]] String& operator=(const String& other) = delete;

  // TODO cgustafsson:
  // String substr() const;

  [[nodiscard]] const char* c_str() const;
  [[nodiscard]] const u8* data() const;
  [[nodiscard]] u8* data();

  [[nodiscard]] usize getSize() const;
  [[nodiscard]] bool isEmpty() const;

  [[nodiscard]] constexpr bool operator==(const String& other) const;
  [[nodiscard]] constexpr bool operator!=(const String& other) const;

  [[nodiscard]] constexpr bool operator==(const char* other) const;
  [[nodiscard]] constexpr bool operator!=(const char* other) const;

 private:
  struct [[nodiscard]] BigString {
    u8* string;
    usize size;
    usize capacity;
  };
  static_assert(std::is_pod_v<BigString>);

  struct [[nodiscard]] SmallString {
    static constexpr usize kSize =
        sizeof(BigString);  //< including null terminator
    u8 string[kSize];

    [[nodiscard]] constexpr bool isEmpty() const { return getSize() == 0; }
    [[nodiscard]] constexpr usize getSize() const {
      return kSize - string[kSize - 1];
    }
    [[nodiscard]] constexpr usize getCapacity() const { return kSize; }
    void setSize(usize newSize);
    void clear();
  };
  static_assert(std::is_pod_v<SmallString>);

  enum class State : uintptr_t {
    BigString = 0,
    SmallString = 1,
  };
  static constexpr uintptr_t kFlagStateBit = 0;
  static constexpr uintptr_t kFlagStateMask = 1;

 private:
  [[nodiscard]] State getState() const;
  void setState(State state);

  [[nodiscard]] IAllocator& getAllocator();

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

}  // namespace dc
