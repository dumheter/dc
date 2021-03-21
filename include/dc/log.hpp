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
// API
// ========================================================================== //

/// Use these macros to dispatch log payloads to the log worker.
#define DC_VERBOSE(...)                                                   \
  do {                                                                    \
    dc::log::internal::makePayload(DC_FILENAME, __func__, __LINE__,       \
                                   dc::log::Level::Verbose, __VA_ARGS__); \
  } while (0)
#define DC_INFO(...)                                                   \
  do {                                                                 \
    dc::log::internal::makePayload(DC_FILENAME, __func__, __LINE__,    \
                                   dc::log::Level::Info, __VA_ARGS__); \
  } while (0)
#define DC_WARNING(...)                                                   \
  do {                                                                    \
    dc::log::internal::makePayload(DC_FILENAME, __func__, __LINE__,       \
                                   dc::log::Level::Warning, __VA_ARGS__); \
  } while (0)
#define DC_ERROR(...)                                                   \
  do {                                                                  \
    dc::log::internal::makePayload(DC_FILENAME, __func__, __LINE__,     \
                                   dc::log::Level::Error, __VA_ARGS__); \
  } while (0)

/// Log the raw string, without payload
#define DC_RAW(...)                                                   \
  do {                                                                \
    dc::log::internal::makePayload(DC_FILENAME, __func__, __LINE__,   \
                                   dc::log::Level::Raw, __VA_ARGS__); \
  } while (0)

namespace dc::log {

/// Start the log worker. Log will not do anything until this is called.
void start();

/// Stop the log worker. It's called safely because not calling it is not safe.
/// There could be logs lost by not calling it.
/// @param timeoutUs Specify a maximum time in microseconds, to wait for it to
/// finish.
/// @return If the log worker finished all work before dying.
bool stopSafely(u64 timeoutUs = 1'000'000);

enum class Level {
  Verbose,
  Info,
  Warning,
  Error,
  Raw,
  None,
};

void setLevel(Level level);

}  // namespace dc::log

// ========================================================================== //
// Internal
// ========================================================================== //

namespace dc::log::internal {

struct [[nodiscard]] Payload {
  const char* fileName;
  const char* functionName;
  int lineno;
  Level level;
  Timestamp timestamp;
  std::string msg;
};

struct [[nodiscard]] Settings {
  Level level;
};

[[nodiscard]] bool push(Payload&& payload);

template <typename... Args>
inline void makePayload(const char* fileName, const char* functionName,
                        int lineno, Level level, Args&&... args) {
  Payload payload;
  payload.fileName = fileName;
  payload.functionName = functionName;
  payload.lineno = lineno;
  payload.level = level;
  payload.timestamp = makeTimestamp();
  payload.msg = fmt::format(std::forward<Args>(args)...);

  // Can fail if we cannot allocate memory (if needed).
  const bool res = internal::push(std::move(payload));
  DC_ASSERT(res, "failed to allocate memory");
}

}  // namespace dc::log::internal

// ========================================================================== //
// Fmt specialization
// ========================================================================== //

template <>
struct fmt::formatter<dc::log::Level> : formatter<string_view> {
  template <typename FormatContext>
  auto format(dc::log::Level level, FormatContext& ctx) {
    string_view str;
    switch (level) {
      case dc::log::Level::Verbose:
        str = "verbose";
        break;
      case dc::log::Level::Info:
        str = "info";
        break;
      case dc::log::Level::Warning:
        str = "warning";
        break;
      case dc::log::Level::Error:
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
