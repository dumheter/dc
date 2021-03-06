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

#include <dc/platform.hpp>
#include <dc/types.hpp>

namespace dc {

/// Get a timestamp from a high resolution clock.
/// Note: Use getTimeUsNoReordering when benchmarking things, to ensure no
/// reordering.
[[nodiscard]] u64 getTimeUs();

/// Get a timestamp from a high resolution clock. In addition, it has memory
/// barrieers to protect from reordering.
[[nodiscard]] u64 getTimeUsNoReorder();

/// Yield execution back to the operating system.
/// @param timeMs Hint to the os at the time until execution should be given
/// back.
///               The os may take less or more time.
void sleepMs(u32 timeMs);

/// fmt formatting code exists in dlog.hpp
struct [[nodiscard]] Timestamp {
  u32 year;    // [0, ...]
  u8 month;    // [1, 12]
  u8 day;      // [1, 31]
  u8 hour;     // [0, 23]
  u8 minute;   // [0, 59]
  f32 second;  // underlying precision is ~100 nanosecond ticks.
};

[[nodiscard]] Timestamp makeTimestamp();

class [[nodiscard]] Stopwatch {
 public:
  /// Will call start().
  Stopwatch();

  /// Start the stopwatch.
  /// Note: Called by constructor.
  void start();

  /// Stop the stopwatch.
  void stop();

  /// Get time from start to stop.
  [[nodiscard]] u64 ns() const;
  [[nodiscard]] u64 us() const;
  [[nodiscard]] u64 ms() const;
  [[nodiscard]] u64 s() const;

  /// Get time from start to stop. With full precision in floating point.
  [[nodiscard]] f64 fs() const;

  /// Get time from start to now.
  [[nodiscard]] u64 nowNs() const;
  [[nodiscard]] u64 nowUs() const;
  [[nodiscard]] u64 nowMs() const;
  [[nodiscard]] u64 nowS() const;

  /// Get time from start to now. With full precision in floating point.
  [[nodiscard]] f64 nowFs() const;

 private:
  u64 m_start;
  u64 m_stop;
#if defined(DC_PLATFORM_WINDOWS)
  u64 m_freqCache;
#endif
};

}  // namespace dc
