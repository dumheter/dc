/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
n *
 * The above copyright notice and this permission notice shall be included in
all
 * copies or substantial portions of the Software.
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

/**
 * clamp a value on the range [min-max].
 *
 * Examples
 *   clamp(1, 1, 10)   -> 1
 *   clamp(10, 1, 10)  -> 10
 *   clamp(0, 1, 10)   -> 1
 *   clamp(99, 1, 10)  -> 10
 *   clamp(5, 1, 10)   -> 5
 */
template <typename T>
inline T clamp(const T val, const T min, const T max) {
  if (val >= min && val <= max)
    return val;
  else if (val < min)
    return min;
  else /*if (val > max)*/
    return max;
}

/**
 * Is the value on or inside the range [min-max]?
 *
 * Example:
 *   inside(5, 0, 10) -> true
 *   inside(11, 0, 10) -> false
 */
template <typename T>
inline bool inside(const T val, const T min, const T max) {
  return val >= min && val <= max;
}

/**
 * Map val from a range [from_min, from_max], to another range
 * [to_min, to_max].
 *
 * @pre val must be in range [from_min, from_max].
 *
 * Example:
 *   map(5, 0, 10, 0, 100)  -> 50
 *   map(10, 0, 10, 0, 100) -> 100
 *   map(0, 0, 10, 0, 100)  -> 0
 */
template <typename T>
inline T map(T val, T from_min, T from_max, T to_min, T to_max) {
  return (val - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
}

// ========================================================================== //

// FNV1a c++11 constexpr compile time hash functions, 32 and 64 bit
// str should be a null terminated string literal, value should be left out
// e.g hash_32_fnv1a_const("example")
// code license: public domain or equivalent
// post: https://notes.underscorediscovery.com/constexpr-fnv1a/

namespace details::hash {

constexpr u32 val32 = 0x811c9dc5;
constexpr u32 prime32 = 0x1000193;
constexpr u64 val64 = 0xcbf29ce484222325;
constexpr u64 prime64 = 0x100000001b3;

}  // namespace details::hash

inline constexpr u32 hash32fnv1a(
    const char* const str, const u32 value = details::hash::val32) noexcept {
  return (str[0] == '\0') ? value
                          : hash32fnv1a(&str[1], (value ^ u32(str[0])) *
                                                     details::hash::prime32);
}

inline constexpr u64 hash64fnv1a(
    const char* const str, const u64 value = details::hash::val64) noexcept {
  return (str[0] == '\0') ? value
                          : hash64fnv1a(&str[1], (value ^ u64(str[0])) *
                                                     details::hash::prime64);
}

}  // namespace dc
