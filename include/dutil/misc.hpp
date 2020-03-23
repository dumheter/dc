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

#ifndef DUTIL_MISC_HPP_
#define DUTIL_MISC_HPP_

namespace dutil {

/**
 * Clamp a value on the range [min-max].
 *
 * Examples
 *   Clamp(1, 1, 10)   -> 1
 *   Clamp(10, 1, 10)  -> 10
 *   Clamp(0, 1, 10)   -> 1
 *   Clamp(99, 1, 10)  -> 10
 *   Clamp(5, 1, 10)   -> 5
 */
template <typename T>
inline T Clamp(const T val, const T min, const T max) {
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
 *   Inside(5, 0, 10) -> true
 *   Inside(11, 0, 10) -> false
 */
template <typename T>
inline bool Inside(const T val, const T min, const T max) {
  return val >= min && val <= max;
}

/**
 * Map val from a range [from_min, from_max], to another range
 * [to_min, to_max].
 *
 * @pre val must be in range [from_min, from_max].
 *
 * Example:
 *   Map(5, 0, 10, 0, 100)  -> 50
 *   Map(10, 0, 10, 0, 100) -> 100
 *   Map(0, 0, 10, 0, 100)  -> 0
 */
template <typename T>
inline T Map(T val, T from_min, T from_max, T to_min, T to_max) {
  return (val - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
}

}  // namespace dutil

#endif  // DUTIL_MISC_HPP_
