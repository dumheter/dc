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
#include <utility>  // for std::forward
#include "types.hpp"

namespace dutil {

// ============================================================ //
// Time Related Functions
// ============================================================ //

/**
 * Calls the function @fn at @ticks_per_s times a second. It will catch up if
 * it falls behind. Internally it counts in nano seconds, combined with a
 * std::chrono::high_resolution_clock, so expect a resolution of that.
 *
 * Note: This function must be called continuously to allow it to update
 * its internal state.
 *
 * @param ticks_per_s How many times the @fn will be called each second.
 * @param fn The function to be called each tick.
 * @return If fn was called.
 *
 * Example:
 *   FixedTimeUpdate(30, [](){ std::cout << "tick!\n"; });
 *
 * Example, class method as callback function:
 *   const auto fn =
 *     std::bind(&Class::Method, &instance);
 *  FixedTimeUpdate(32, fn);
 *
 */
template <typename TFunction, typename... ARGS>
bool FixedTimeUpdate(const f64 ticks_per_s, TFunction&& fn, ARGS&& ... args);

// ============================================================ //

/**
 * Will run your @fn continuously, until it returns true, or
 * the stopwatch shows more than @timeout_ms.
 *
 * @tparam TFn Function with signature 'bool fn()'. Example of a class
 * method being called from a bound object:
 *    auto fn = std::bind(&ClassName::MethodName, &BoundObject);
 * @param fn The function that will be called continuously.
 * @param timeout_ms How long it will attempt to get a true from @fn.
 * @retval true fn did return true within the time limit.
 * @retval false fn did not return true within the time limit.
 */
template <typename TFn, typename ... ARGS>
bool TimedCheck(s64 timeout_ms, TFn&& fn, ARGS&& ... args);

// ============================================================ //
// Stopwatch
// ============================================================ //

using namespace std::chrono;
using clock_type = high_resolution_clock;

/**
 * Works like a physical stopwatch. Use Start() and Stop() to track time.
 * Then use the getters in the unit you want. You can also Resume() tracking
 * time after calling Stop().
 *
 * After calling Start(), you can use the now* getters to get the current
 * reading on the stopwatch.
 */
class Stopwatch {
 public:
  /**
   * Will call Start.
   */
  Stopwatch();

  /**
   * Start tracking time. A second call to Start will overwrite the current
   * start time point.
   */
  void Start();

  /**
   * Stop tracking time.
   */
  void Stop();

  /**
   * Resume tracking time.
   * @pre Must have called Stop() previously.
   */
  void Resume();

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
  time_point<clock_type> stop_;
};

// ============================================================ //
// Template definition
// ============================================================ //

template <typename TFunction, typename ... ARGS>
bool FixedTimeUpdate(const f64 ticks_per_s, TFunction&& fn, ARGS&& ... args) {
  static dutil::Stopwatch sw{};
  static f64 timer_ns{sw.fnow_ns()};
  const f64 ticks_per_ns = 1000.0 * 1000.0 * 1000.0 / ticks_per_s;
  if (sw.fnow_ns() - timer_ns > ticks_per_ns) {
    timer_ns += ticks_per_ns;
    fn(std::forward<ARGS>(args) ...);
    return true;
  }
  return false;
}

// ============================================================ //

template <typename TFn, typename... ARGS>
bool TimedCheck(s64 timeout_ms, TFn&& fn, ARGS&& ... args) {
  dutil::Stopwatch stopwatch{};
  stopwatch.Start();
  bool did_timeout = false;
  bool result = false;
  while (!result) {
    result = fn(std::forward<ARGS>(args)...);
    if (stopwatch.now_ms() > timeout_ms) {
      did_timeout = true;
      break;
    }
  }
  return !did_timeout;
}

}  // namespace dutil

#endif  // DUTIL_STOPWATCH_HPP_
