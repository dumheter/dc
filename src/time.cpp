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
#else
#include <time.h>

#include <chrono>
#endif

#if defined(DC_PLATFORM_LINUX)
#include <unistd.h>
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

  timeUs = static_cast<u64>(time.QuadPart * 1000'000 / freq.QuadPart);
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
    timeUs = time.tv_sec * 1000000 + time.tv_nsec / 1000;
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

  timeUs = static_cast<u64>(time.QuadPart * 1000'000 / freq.QuadPart);
#elif defined(DC_PLATFORM_LINUX)
  timespec time;
  std::atomic_signal_fence(std::memory_order_seq_cst);
  const auto res = clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  std::atomic_signal_fence(std::memory_order_seq_cst);
  if (res == 0)
    timeUs = time.tv_sec * 1000000 + time.tv_nsec / 1000;
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
  usleep(static_cast<unsigned int>(timeMs * 1000));
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
#else
  // TODO cgustafsson: make a native impl of timestamp on other platforms
  using namespace std::chrono;

  const time_point<system_clock> systemNow = system_clock::now();
  const time_t now = system_clock::to_time_t(systemNow);
  const tm* time = localtime(&now);

  out.year = time->tm_year + 1'900;
  out.month = static_cast<u8>(time->tm_mon);
  out.day = static_cast<u8>(time->tm_mday);
  out.hour = static_cast<u8>(time->tm_hour);
  out.minute = static_cast<u8>(time->tm_min);

  const auto tp = systemNow.time_since_epoch();
  const u32 s = duration_cast<seconds>(tp).count() % 60;
  const u32 ms = duration_cast<milliseconds>(tp).count() % 1000;
  const u32 us = duration_cast<microseconds>(tp).count() % 1000;
  out.second = s + (ms / 1'000.f) + (us / 1'000'000.f);
#endif

  return out;
}

}  // namespace dc
