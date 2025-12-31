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

#include <dc/assert.hpp>
#include <dc/math.hpp>
#include <dc/types.hpp>

namespace dc {

template <typename PointerT, typename IntT = u32>
class PointerIntPair {
 public:
  PointerIntPair() = default;

  PointerIntPair(PointerT ptr) { setPointer(ptr); }

  PointerT getPointer() const {
    return reinterpret_cast<PointerT>(m_value & kPointerMask);
  }

  IntT getInt() const { return static_cast<IntT>(m_value & kIntMask); }

  void setPointer(PointerT ptr) {
    m_value = setBits(m_value, sizeof(PointerT) * 8 - kFreeBits, 0,
                      reinterpret_cast<uintptr_t>(ptr));
  }

  void setInt(IntT value) {
    DC_ASSERT(value < static_cast<IntT>(kFreeBitsValue),
              "Trying to assign a too large int.");
    m_value = setBits(m_value, kFreeBits, 0, static_cast<uintptr_t>(value));
  }

 private:
  static constexpr int kFreeBits =
      static_cast<int>(dc::log2(alignof(PointerT)));
  static constexpr int kFreeBitsValue = alignof(PointerT);
  static constexpr uintptr_t kIntMask =
      static_cast<uintptr_t>(kFreeBitsValue - 1);
  static constexpr uintptr_t kPointerMask = ~kIntMask;

 private:
  uintptr_t m_value = 0;
};

}  // namespace dc
