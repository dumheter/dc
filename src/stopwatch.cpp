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

#include <dc/stopwatch.hpp>

namespace dc {

Stopwatch::Stopwatch() : m_start{}, m_stop{} { start(); }

void Stopwatch::start() { m_start = clock_type::now(); }

void Stopwatch::stop() { m_stop = clock_type::now(); }

void Stopwatch::resume() { m_start = clock_type::now() - (m_stop - m_start); }

s64 Stopwatch::s() const {
  return duration_cast<seconds>(m_stop - m_start).count();
}

s64 Stopwatch::ms() const {
  return duration_cast<milliseconds>(m_stop - m_start).count();
}

s64 Stopwatch::us() const {
  return duration_cast<microseconds>(m_stop - m_start).count();
}

s64 Stopwatch::ns() const {
  return duration_cast<nanoseconds>(m_stop - m_start).count();
}

double Stopwatch::fs() const {
  return duration<double>{m_stop - m_start}.count();
}

double Stopwatch::fms() const {
  return duration<double, std::milli>{m_stop - m_start}.count();
}

double Stopwatch::fus() const {
  return duration<double, std::micro>{m_stop - m_start}.count();
}

double Stopwatch::fns() const {
  return duration<double, std::nano>{m_stop - m_start}.count();
}

s64 Stopwatch::nowS() const {
  return duration_cast<seconds>(clock_type::now() - m_start).count();
}

s64 Stopwatch::nowMs() const {
  return duration_cast<milliseconds>(clock_type::now() - m_start).count();
}

s64 Stopwatch::nowUs() const {
  return duration_cast<microseconds>(clock_type::now() - m_start).count();
}

s64 Stopwatch::nowNs() const {
  return duration_cast<nanoseconds>(clock_type::now() - m_start).count();
}

double Stopwatch::fnowS() const {
  return duration<double>{clock_type::now() - m_start}.count();
}

double Stopwatch::fnowMs() const {
  return duration<double, std::milli>{clock_type::now() - m_start}.count();
}

double Stopwatch::fnowUs() const {
  return duration<double, std::micro>{clock_type::now() - m_start}.count();
}

double Stopwatch::fnowNs() const {
  return duration<double, std::nano>{clock_type::now() - m_start}.count();
}

}  // namespace dc
