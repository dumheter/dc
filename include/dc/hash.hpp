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

#include <dc/types.hpp>

namespace dc {

class String;
class StringView;

// ========================================================================== //
// Hash Utilities
// ========================================================================== //

/// FNV-1a hash for raw bytes
inline u64 hashBytes(const u8* data, usize size) {
  constexpr u64 kFnvOffset = 0xcbf29ce484222325;
  constexpr u64 kFnvPrime = 0x100000001b3;
  u64 hash = kFnvOffset;
  for (usize i = 0; i < size; ++i) {
    hash ^= static_cast<u64>(data[i]);
    hash *= kFnvPrime;
  }
  return hash;
}

// ========================================================================== //
// Hash Trait
// ========================================================================== //

/// Default hash functor - must be specialized for each key type
template <typename T>
struct Hash;

// -------------------------------------------------------------------------- //
// Integral Type Specializations
// -------------------------------------------------------------------------- //

template <>
struct Hash<u8> {
  u64 operator()(u8 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<u16> {
  u64 operator()(u16 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<u32> {
  u64 operator()(u32 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<u64> {
  u64 operator()(u64 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<s8> {
  u64 operator()(s8 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<s16> {
  u64 operator()(s16 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<s32> {
  u64 operator()(s32 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

template <>
struct Hash<s64> {
  u64 operator()(s64 key) const {
    return hashBytes(reinterpret_cast<const u8*>(&key), sizeof(key));
  }
};

// -------------------------------------------------------------------------- //
// String Type Specializations
// -------------------------------------------------------------------------- //

template <>
struct Hash<String> {
  u64 operator()(const String& key) const;
};

template <>
struct Hash<StringView> {
  u64 operator()(const StringView& key) const;
};

// ========================================================================== //
// Equal Trait
// ========================================================================== //

/// Default equality functor using operator==
template <typename T>
struct Equal {
  bool operator()(const T& a, const T& b) const { return a == b; }
};

// -------------------------------------------------------------------------- //
// String Type Specializations
// -------------------------------------------------------------------------- //

template <>
struct Equal<StringView> {
  bool operator()(const StringView& a, const StringView& b) const;
};

}  // namespace dc
