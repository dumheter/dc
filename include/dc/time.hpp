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

#include <dc/core.hpp>

namespace dc {

/// Get a timestamp from a high resolution clock.
/// Note: Do not use for any serious benchmark, is not protected from
/// reordering.
u64 getTimeUs();

/// Get a timestamp from a high resolution clock. In addition, it has memory
/// barrieers to protect from reordering.
// TODO cgustafsson: needs testing
u64 getTimeUsNoReorder();

void sleepMs(u32 timeMs);

/// fmt formatting code exists in dlog.hpp
struct [[nodiscard]] Timestamp {
  u32 year;      //< [0, 4096]
  u8 month;      //< [0, 16), starts at 1
  u8 day;        //< [0, 32), starts at 1
  u8 hour;       //< [0, 32)
  u8 minute;     //< [0, 64)
  float second;  //< underlying precision is 100 nanoseconds.
};

[[nodiscard]] Timestamp makeTimestamp();

}  // namespace dc
