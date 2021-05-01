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

#include <atomic>
#include <dc/assert.hpp>
#include <dc/platform.hpp>
#include <dc/time.hpp>

#if defined(DC_PLATFORM_WINDOWS)
#if !defined(VC_EXTRALEAN)
#define VC_EXTRALEAN
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <Windows.h>
#elif defined(DC_PLATFORM_LINUX)
#include <time.h>
#include <unistd.h>
#include <limits>
#include <dc/math.hpp>
#endif

namespace dc {

[[nodiscard]] u64 getTimeUs() {
  u64 timeUs;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time, freq;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;

  if (!QueryPerformanceFrequency(&freq)) {
    time.QuadPart = 0;
    freq.QuadPart = 1;
  }

  timeUs = static_cast<u64>(time.QuadPart * 1'000'000 / freq.QuadPart);
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    timeUs = static_cast<u64>(time.tv_sec * 1'000'000 + time.tv_nsec / 1'000);
  else
    timeUs = 0;
#else
  DC_ASSERT(false, "not implemented");
  timeUs = 0;
#endif
  return timeUs;
}

[[nodiscard]] u64 getTimeUsNoReorder() {
  u64 timeUs;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time, freq;
  std::atomic_signal_fence(std::memory_order_seq_cst);
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  if (!QueryPerformanceFrequency(&freq)) {
    time.QuadPart = 0;
    freq.QuadPart = 1;
  }
  std::atomic_signal_fence(std::memory_order_seq_cst);

  timeUs = static_cast<u64>(time.QuadPart * 1'000'000 / freq.QuadPart);
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  std::atomic_signal_fence(std::memory_order_seq_cst);
  const auto res = clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  std::atomic_signal_fence(std::memory_order_seq_cst);
  if (res == 0)
    timeUs = static_cast<u64>(time.tv_sec * 1'000'000 + time.tv_nsec / 1'000);
  else
    timeUs = 0;
#else
  DC_ASSERT(false, "not implemented");
  timeUs = 0;
#endif
  return timeUs;
}

void sleepMs(u32 timeMs) {
#if defined(DC_PLATFORM_WINDOWS)
  Sleep(static_cast<DWORD>(timeMs));
#elif defined(DC_PLATFORM_LINUX)
  const u32 safeTimeMs = clamp(timeMs, 0u, std::numeric_limits<u32>::max() / 1'000u);
  usleep(safeTimeMs * 1'000);
#else
  DC_ASSERT(false, "not implemented");
#endif
}

[[nodiscard]] Timestamp makeTimestamp() {
  Timestamp out;
#if defined(DC_PLATFORM_WINDOWS)
  FILETIME fileTime;
  GetSystemTimePreciseAsFileTime(&fileTime);

  SYSTEMTIME systemTime;
  const BOOL ok = FileTimeToSystemTime(&fileTime, &systemTime);

  if (!ok) {
    out.second = -1.f;
    return out;
  }

  out.year = systemTime.wYear;
  out.month = static_cast<u8>(systemTime.wMonth);
  out.day = static_cast<u8>(systemTime.wDay);
  out.hour = static_cast<u8>(systemTime.wHour);
  out.minute = static_cast<u8>(systemTime.wMinute);

  u64 ns = fileTime.dwLowDateTime;
  ns += (static_cast<u64>(fileTime.dwHighDateTime) << 32);
  out.second = systemTime.wSecond + ns % 10'000'000 / 10'000'000.f;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) != 0)
  {
    // make sure we are 0 if call fails
    time.tv_sec = 0;
    time.tv_nsec = 0;
  }

  const u64 ns = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  tm datetime;
  gmtime_r(&time.tv_sec, &datetime);

  out.year = static_cast<u32>(datetime.tm_year);
  out.month = static_cast<u8>(datetime.tm_mon);
  out.day = static_cast<u8>(datetime.tm_mday);
  out.hour = static_cast<u8>(datetime.tm_hour);
  out.minute = static_cast<u8>(datetime.tm_min);

  out.second = f32(ns) / 1'000'000'000.f;
#endif

  return out;
}

Stopwatch::Stopwatch() {
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER freq;
  if (!QueryPerformanceFrequency(&freq)) {
    freq.QuadPart = 1;  //< 1 on failure to guard against divde by zero.
  }
  m_freqCache = freq.QuadPart;
#endif
  start();
}

void Stopwatch::start() {
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  m_start = time.QuadPart;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    m_start = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    m_start = 0;
#else
  DC_ASSERT(false, "not implemented");
  m_start = 0;
#endif
}

void Stopwatch::stop() {
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  m_stop = time.QuadPart;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    m_stop = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    m_stop = 0;
#else
  DC_ASSERT(false, "not implemented");
  m_stop = 0;
#endif
}

u64 Stopwatch::ns() const {
#if defined(DC_PLATFORM_WINDOWS)
  return (m_stop - m_start) * 1'000'000'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  return (m_stop - m_start);
#else
  DC_ASSERT(false, "not implemented");
  return 0;
#endif
}

u64 Stopwatch::us() const {
#if defined(DC_PLATFORM_WINDOWS)
  return (m_stop - m_start) * 1'000'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  return (m_stop - m_start) / 1'000;
#else
  DC_ASSERT(false, "not implemented");
  return 0;
#endif
}

u64 Stopwatch::ms() const {
#if defined(DC_PLATFORM_WINDOWS)
  return (m_stop - m_start) * 1'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  return (m_stop - m_start) / 1'000'000;
#else
  DC_ASSERT(false, "not implemented");
  return 0;
#endif
}

u64 Stopwatch::s() const {
#if defined(DC_PLATFORM_WINDOWS)
  return (m_stop - m_start) / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  return (m_stop - m_start) / 1'000'000'000;
#else
  DC_ASSERT(false, "not implemented");
  return 0;
#endif
}

f64 Stopwatch::fs() const {
#if defined(DC_PLATFORM_WINDOWS)
  return (m_stop - m_start) / (1.0 * m_freqCache);
#elif defined(DC_PLATFORM_LINUX)
  return f64(m_stop - m_start) / 1'000'000'000.;
#else
  DC_ASSERT(false, "not implemented");
  return 0.;
#endif
}

u64 Stopwatch::nowNs() const {
  u64 now;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  now = time.QuadPart;
  return (now - m_start) * 1'000'000'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    now = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    now = 0;
  return (now - m_start);
#else
  DC_ASSERT(false, "not implemented");
  now = 0;
  return now;
#endif
}

u64 Stopwatch::nowUs() const {
  u64 now;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  now = time.QuadPart;
  return (now - m_start) * 1'000'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    now = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    now = 0;
  return (now - m_start) / 1'000;
#else
  DC_ASSERT(false, "not implemented");
  now = 0;
  return now;
#endif
}

u64 Stopwatch::nowMs() const {
  u64 now;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  now = time.QuadPart;
  return (now - m_start) * 1'000 / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    now = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    now = 0;
  return (now - m_start) / 1'000'000;
#else
  DC_ASSERT(false, "not implemented");
  now = 0;
  return now;
#endif
}

u64 Stopwatch::nowS() const {
  u64 now;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  now = time.QuadPart;
  return (now - m_start) / m_freqCache;
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    now = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    now = 0;
  return (now - m_start) / 1'000'000'000;
#else
  DC_ASSERT(false, "not implemented");
  now = 0;
  return now;
#endif
}

f64 Stopwatch::nowFs() const {
  u64 now;
#if defined(DC_PLATFORM_WINDOWS)
  LARGE_INTEGER time;
  if (!QueryPerformanceCounter(&time)) time.QuadPart = 0;
  now = time.QuadPart;
  return (now - m_start) / static_cast<f64>(m_freqCache);
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    now = static_cast<u64>(time.tv_sec * 1'000'000'000 + time.tv_nsec);
  else
    now = 0;
  return f64(now - m_start) / 1'000'000'000.;
#else
  DC_ASSERT(false, "not implemented");
  now = 0;
  return now;
#endif
}

}  // namespace dc
