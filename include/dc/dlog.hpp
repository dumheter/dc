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

#include <fmt/format.h>

#include <dc/assert.hpp>
#include <dc/core.hpp>
#include <dc/time.hpp>
#include <string>
#include <utility>

// ========================================================================== //
// Macros
// ========================================================================== //

/// Init dlog library, should do first thing before calling anything else.
#define DLOG_INIT()       \
  do {                    \
    dc::internal::init(); \
  } while (0)

/// Use these macros to dispatch log payloads to the log worker.
#define DLOG_VERBOSE(...)                                                    \
  do {                                                                       \
    dc::internal::makeLogPayload(DC_FILENAME, __func__, __LINE__,            \
                                 dc::internal::Level::Verbose, __VA_ARGS__); \
  } while (0)
#define DLOG_INFO(...)                                                    \
  do {                                                                    \
    dc::internal::makeLogPayload(DC_FILENAME, __func__, __LINE__,         \
                                 dc::internal::Level::Info, __VA_ARGS__); \
  } while (0)
#define DLOG_WARNING(...)                                                    \
  do {                                                                       \
    dc::internal::makeLogPayload(DC_FILENAME, __func__, __LINE__,            \
                                 dc::internal::Level::Warning, __VA_ARGS__); \
  } while (0)
#define DLOG_ERROR(...)                                                    \
  do {                                                                     \
    dc::internal::makeLogPayload(DC_FILENAME, __func__, __LINE__,          \
                                 dc::internal::Level::Error, __VA_ARGS__); \
  } while (0)

/// Log the raw string, without log payload
#define DLOG_RAW(...)                                                    \
  do {                                                                   \
    dc::internal::makeLogPayload(DC_FILENAME, __func__, __LINE__,        \
                                 dc::internal::Level::Raw, __VA_ARGS__); \
  } while (0)

// ========================================================================== //
// Internal
// ========================================================================== //

namespace dc::internal {

enum class Level {
  Verbose,
  Info,
  Warning,
  Error,
  Raw,
};

struct [[nodiscard]] LogPayload {
  const char* fileName;
  const char* functionName;
  int lineno;
  Level level;
  Timestamp timestamp;
  std::string msg;
};

void init();

class DlogState;

[[nodiscard]] DlogState& dlogStateInstance();

[[nodiscard]] bool pushLog(LogPayload&& log);

// TODO cgustafsson: make a shutdown where we can wait (with timeout) for the
// worker to finish logging.

void DlogWorkerLaunch();

void DlogWorkerShutdown();

template <typename... Args>
inline void makeLogPayload(const char* fileName, const char* functionName,
                           int lineno, Level level, Args&&... args) {
  LogPayload log;
  log.fileName = fileName;
  log.functionName = functionName;
  log.lineno = lineno;
  log.level = level;
  log.timestamp = makeTimestamp();
  log.msg = fmt::format(std::forward<Args>(args)...);

  // Can fail if we cannot allocate memory (if needed).
  const bool res = internal::pushLog(std::move(log));
  DC_ASSERT(res, "failed to allocate memory");
}

}  // namespace dc::internal

// ========================================================================== //
// Fmt specialization
// ========================================================================== //

template <>
struct fmt::formatter<dc::internal::Level> : formatter<string_view> {
  template <typename FormatContext>
  auto format(dc::internal::Level level, FormatContext& ctx) {
    string_view str;
    switch (level) {
      case dc::internal::Level::Verbose:
        str = "verbose";
        break;
      case dc::internal::Level::Info:
        str = "info";
        break;
      case dc::internal::Level::Warning:
        str = "warning";
        break;
      case dc::internal::Level::Error:
        str = "error";
        break;
      default:
        str = "unknown";
    }
    return formatter<string_view>::format(str, ctx);
  }
};

template <>
struct fmt::formatter<dc::Timestamp> {
  bool printDate = false;
  bool highPrecisionTime = false;

  /// Formatting options:
  ///   'd': Turn on date print.
  ///   'p': Turn on time print.
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    auto end = ctx.end();

    for (;;) {
      if (it != end) {
        if (*it == 'd')
          printDate = true;
        else if (*it == 'p')
          highPrecisionTime = true;
        else if (*it == '}')
          break;
        else
          throw format_error("invalid format");
      } else
        break;
      ++it;
    }

    return it;
  }

  template <typename FormatContext>
  auto format(const dc::Timestamp& t, FormatContext& ctx) {
    if (printDate && highPrecisionTime)
      return format_to(ctx.out(), "{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:.6f}",
                       2000 + t.yearFrom2000, t.month + 1, t.day, t.hour,
                       t.minute, t.second);
    else if (printDate && !highPrecisionTime)
      return format_to(ctx.out(), "{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:.0f}",
                       2000 + t.yearFrom2000, t.month + 1, t.day, t.hour,
                       t.minute, t.second);
    else if (!printDate && highPrecisionTime)
      return format_to(ctx.out(), "{:0>2}:{:0>2}:{:.6f}", t.hour, t.minute,
                       t.second);
    else /* if (!printDate && !highPrecisionTime) */
      return format_to(ctx.out(), "{:0>2}:{:0>2}:{:.0f}", t.hour, t.minute,
                       t.second);
  }
};
