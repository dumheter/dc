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

#ifndef DUTIL_STOPWATCH_HPP_
#define DUTIL_STOPWATCH_HPP_

#include <chrono>
#include "types.hpp"
/**
 * Simply including functional increases compile time considerably.
 * Define the flag to dissable the function TimedCheck and to not
 * include functional.
 */
#ifndef DUTIL_DISSABLE_FUNCTIONAL
#include <functional>
#endif

namespace dutil {

using namespace std::chrono;
using clock_type = high_resolution_clock;

/**
 * Will run your @fn continuously, until it returns true, or
 * the stopwatch shows more than @timeout_ms.
 *
 * @param fn The function that will be called continuously.
 * @param timeout_ms How long it will attempt to get a true from @fn.
 * @retval true fn did return true within the time limit.
 * @retval false fn did not return true within the time limit.
 */
#ifndef DUTIL_DISSABLE_FUNCTIONAL
bool TimedCheck(std::function<bool()> fn, s64 timeout_ms);
#endif

class Stopwatch {
 public:
  Stopwatch();

  /**
   * Start tracking time.
   */
  void Start();

  /**
   * Stop tracking time.
   */
  void Stop();

  /**
   * Get time elapsed from start to stop.
   */
  s64 s() const;
  s64 ms() const;
  s64 us() const;
  s64 ns() const;

  double fs() const;
  double fms() const;
  double fus() const;
  double fns() const;

  /**
   * Get elapsed time since start.
   */
  s64 now_s() const;
  s64 now_ms() const;
  s64 now_us() const;
  s64 now_ns() const;

  double fnow_s() const;
  double fnow_ms() const;
  double fnow_us() const;
  double fnow_ns() const;

 private:
  time_point<clock_type> start_;
  time_point<clock_type> end_;
};

}  // namespace dutil

#endif  // DUTIL_STOPWATCH_HPP_
