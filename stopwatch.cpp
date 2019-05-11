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

#include "stopwatch.hpp"

namespace dutil {

Stopwatch::Stopwatch() : start_{}, end_{} {}

void Stopwatch::Start() { start_ = clock_type::now(); }

void Stopwatch::Stop() { end_ = clock_type::now(); }

s64 Stopwatch::s() const {
  return duration_cast<seconds>(end_ - start_).count();
}

s64 Stopwatch::ms() const {
  return duration_cast<milliseconds>(end_ - start_).count();
}

s64 Stopwatch::us() const {
  return duration_cast<microseconds>(end_ - start_).count();
}

s64 Stopwatch::ns() const {
  return duration_cast<nanoseconds>(end_ - start_).count();
}

double Stopwatch::fs() const { return duration<double>{end_ - start_}.count(); }

double Stopwatch::fms() const {
  return duration<double, std::milli>{end_ - start_}.count();
}

double Stopwatch::fus() const {
  return duration<double, std::micro>{end_ - start_}.count();
}

double Stopwatch::fns() const {
  return duration<double, std::nano>{end_ - start_}.count();
}

s64 Stopwatch::now_s() const {
  return duration_cast<seconds>(clock_type::now() - start_).count();
}

s64 Stopwatch::now_ms() const {
  return duration_cast<milliseconds>(clock_type::now() - start_).count();
}

s64 Stopwatch::now_us() const {
  return duration_cast<microseconds>(clock_type::now() - start_).count();
}

s64 Stopwatch::now_ns() const {
  return duration_cast<nanoseconds>(clock_type::now() - start_).count();
}

double Stopwatch::fnow_s() const {
  return duration<double>{clock_type::now() - start_}.count();
}

double Stopwatch::fnow_ms() const {
  return duration<double, std::milli>{clock_type::now() - start_}.count();
}

double Stopwatch::fnow_us() const {
  return duration<double, std::micro>{clock_type::now() - start_}.count();
}

double Stopwatch::fnow_ns() const {
  return duration<double, std::nano>{clock_type::now() - start_}.count();
}

}  // namespace dutil
