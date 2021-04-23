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
#include <functional>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Quick Start
//

// TODO cgustafsson: write

///////////////////////////////////////////////////////////////////////////////
// Helper Macros
//

/// Log to the global logger.
#define LOG_VERBOSE(...)                                                    \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                     \
                       dc::log::Level::Verbose, dc::log::getGlobalLogger(), \
                       __VA_ARGS__)
#define LOG_INFO(...)                                                         \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Info, \
                       dc::log::getGlobalLogger(), __VA_ARGS__)
#define LOG_WARNING(...)                                                    \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                     \
                       dc::log::Level::Warning, dc::log::getGlobalLogger(), \
                       __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Error, \
                       dc::log::getGlobalLogger(), __VA_ARGS__)

/// Log the raw string, without prefix
#define LOG_RAW(...)                                                         \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Raw, \
                       dc::log::getGlobalLogger(), __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////

/// Specify the logger
#define LLOG_VERBOSE(logger, ...)                       \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, \
                       dc::log::Level::Verbose, logger, __VA_ARGS__)
#define LLOG_INFO(logger, ...)                                                \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Info, \
                       logger, __VA_ARGS__)
#define LLOG_WARNING(logger, ...)                       \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, \
                       dc::log::Level::Warning, logger, __VA_ARGS__)
#define LLOG_ERROR(logger, ...)                                                \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Error, \
                       logger, __VA_ARGS__)

/// Log the raw string, without prefix
#define LLOG_RAW(logger, ...)                                                \
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Raw, \
                       logger, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Log
//

namespace dc::log {

class Logger;
[[nodiscard]] Logger& getGlobalLogger();

/// Helper function to start a logger, equivalent to:
///   `getGlobalLogger().start()`
void init(Logger& logger = getGlobalLogger());

/// Helper function to stop a logger, equivalent to:
///   `bool ok = getGlobalLogger().stop(1'000'000)`
/// @param timeoutUs Specify a maximum time, in microseconds, to wait for the
/// logger to finish.
/// @return If the logger finished all work and signal its death.
bool deinit(u64 timeoutUs = 1'000'000, Logger& logger = getGlobalLogger());

enum class Level : int {
  Verbose = 0,
  Info,
  Warning,
  Error,
  Raw,
  None,
};

/// Helper function to set log level.
/// Example:
///   Setting Level::Info and verbose log will be ignored.
void setLevel(Level level, Logger& logger = getGlobalLogger());

///////////////////////////////////////////////////////////////////////////////
// Sinks
//

struct Payload;

using Sink = std::function<void(const Payload&, Level)>;

struct ConsoleSink {
  void operator()(const Payload&, Level) const;
};

///////////////////////////////////////////////////////////////////////////////
// Logger
//

class Logger {
 public:
  // Initialize the logger, attach a ConsoleSink called "default".
  Logger(Sink sink = ConsoleSink(), const char* name = "default");

  ~Logger();

  DC_DELETE_COPY(Logger);

  /// Start a logger thread which will start run() loop.
  void start();

  /// Exit the run() loop and shutdown the logger thread.
  bool stop(u64 timeoutUs = 1'000'000);

  /// Enqueue a log payload.
  [[nodiscard]] bool enqueue(Payload&& payload);

  /// Wait on logger dead signal, with timeout.
  /// Should only be done on one thread, since only one signal will be sent.
  [[nodiscard]] bool waitOnLoggerDeadTimeoutUs(u64 timeoutUs);

  [[nodiscard]] Level getLevel() const { return m_level; }

  void setLevel(Level level) { m_level = level; }

  /// Is the logger thread active?
  bool isActive() const { return m_isActive; }

  /// Attach a log sink to the logger. Every log payload will be sent to the
  /// sink. Note: By default, a ConsoleSink is added at logger construction.
  /// @param sink
  /// @param name Name of the sink, used to detach sinks.
  void attachSink(Sink sink, const char* name);

  /// Detach a log sink by name.
  /// Note: The default ConsoleSink is called "default".
  void detachSink(const char* name);

 private:
  void run();

 private:
  bool m_isActive = false;
  Level m_level = Level::Verbose;

  struct Data;
  Data* m_data;
};

///////////////////////////////////////////////////////////////////////////////
// Payload
//

struct [[nodiscard]] Payload {
  const char* fileName;
  const char* functionName;
  int lineno;
  Level level;
  Timestamp timestamp;
  std::string msg;
};

template <typename... Args>
void makePayload(const char* fileName, const char* functionName, int lineno,
                 Level level, Logger& logger, Args&&... args) {
  Payload payload;
  payload.fileName = fileName;
  payload.functionName = functionName;
  payload.lineno = lineno;
  payload.level = level;
  payload.timestamp = makeTimestamp();
  payload.msg = fmt::format(std::forward<Args>(args)...);

  // Can fail if we cannot allocate memory.
  const bool res = logger.enqueue(std::move(payload));
  DC_ASSERT(res, "failed to allocate memory");
}

///////////////////////////////////////////////////////////////////////////////
// Bonus
//

/// Will fix the windows console, make it utf8 and enable colors. Noop on non
/// windows.
void windowsFixConsole();

}  // namespace dc::log

///////////////////////////////////////////////////////////////////////////////
// Log Prefix Settings
//

#if !defined(DC_LOG_PREFIX_DATETIME)
/// Ex [2021-04-02 14:10:7.710909]
/// Datetime 0 : no time nor date
/// Datetime 1 : date and time with microsecond precision
/// Datetime 2 : only time with microsecond precision
/// Datetime 3 : only time
#define DC_LOG_PREFIX_DATETIME 1
#endif
#if !defined(DC_LOG_PREFIX_LEVEL)
/// Ex [warning]
#define DC_LOG_PREFIX_LEVEL 1
#endif
#if !defined(DC_LOG_PREFIX_FILESTAMP)
/// Ex [log.test.cpp              :42 ]
#define DC_LOG_PREFIX_FILESTAMP 1
#endif
#if !defined(DC_LOG_PREFIX_FUNCTION)
/// Ex [TestBody      ]
#define DC_LOG_PREFIX_FUNCTION 1
#endif

///////////////////////////////////////////////////////////////////////////////
// Fmt specialization
//

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
  ///   'p': Turn on microsecond precision time print.
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
      return format_to(ctx.out(), "{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>9.6f}",
                       t.year, t.month + 1, t.day, t.hour, t.minute, t.second);
    else if (printDate && !highPrecisionTime)
      return format_to(ctx.out(), "{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>6.3f}",
                       t.year, t.month + 1, t.day, t.hour, t.minute, t.second);
    else if (!printDate && highPrecisionTime)
      return format_to(ctx.out(), "{:0>2}:{:0>2}:{:0>9.6f}", t.hour, t.minute,
                       t.second);
    else /* if (!printDate && !highPrecisionTime) */
      return format_to(ctx.out(), "{:0>2}:{:0>2}:{:0>6.3f}", t.hour, t.minute,
                       t.second);
  }
};
