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
 * The above copyright notice and this permission notice shall be included in all
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

#ifndef __STOPWATCH_HPP__
#define __STOPWATCH_HPP__

// ============================================================ //
// Headers
// ============================================================ //

#include <chrono>

using namespace std::chrono;

// ============================================================ //
// Class
// ============================================================ //

namespace dutil
{
  class Stopwatch
  {

  public:
    Stopwatch();

    void start();

    void stop();

    long s() const;

    long ms() const;

    long us() const;

    long ns() const;

    long now_s() const;

    long now_ms() const;

    long now_us() const;

    long now_ns() const;

  private:
    time_point<steady_clock> m_start;
    time_point<steady_clock> m_end;

  };
}

#endif//__STOPWATCH_HPP__
