/**
 * MIT License
 *
 * Copyright (c) 2018 Christoffer Gustafsson
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

#ifndef STOPWATCH_HPP__
#define STOPWATCH_HPP__

// ============================================================ //
// Headers
// ============================================================ //

#include <chrono>

using namespace std::chrono;

// ============================================================ //
// Class
// ============================================================ //

namespace dutil {
using clock_type = high_resolution_clock;

class Stopwatch {
 public:
  Stopwatch();

  void start();

  void stop();

  /**
   * Get time elapsed from start to stop.
   */
  long s() const;
  long ms() const;
  long us() const;
  long ns() const;

  double fs() const;
  double fms() const;
  double fus() const;
  double fns() const;

  /**
   * Get elapsed time since start.
   */
  long now_s() const;
  long now_ms() const;
  long now_us() const;
  long now_ns() const;

  double fnow_s() const;
  double fnow_ms() const;
  double fnow_us() const;
  double fnow_ns() const;

 private:
  time_point<clock_type> m_start;
  time_point<clock_type> m_end;
};
}  // namespace dutil

#endif  // STOPWATCH_HPP__
