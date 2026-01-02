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

#include <dc/assert.hpp>
#include <dc/fmt.hpp>
#include <dc/macros.hpp>
#include <dc/string.hpp>
#include <dc/time.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>
#include <functional>
#include <limits>

///////////////////////////////////////////////////////////////////////////////
// Quick Start
//

/// -----------
/// Basic Usage:
///
/// ```cpp
/// #include <dc/log.hpp>
///
/// int main(int, char**) {
///   dc::log::init();
///
///   LOG_INFO("Hello from {}!", "DC");
///
///   const bool logDeinitOk = dc::log::deinit();
///   return !logDeinitOk;
/// }
/// ```
///
/// -------------------
/// Change default sink:
///
/// ```cpp
/// #include <dc/log.hpp>
///
/// int main(int, char**) {
///   dc::log::init();
///   dc::log::getGlobalLogger()
///     .detachLogger("default")
///     .attachLogger(ColoredConsoleLogger, "colored logger");
///
///   LOG_INFO("Hello from {}!", "DC");
///
///   const bool logDeinitOk = dc::log::deinit();
///   return !logDeinitOk;
/// }
/// ```

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
// Log Prefix Settings
//

#if !defined(DC_LOG_PREFIX_DATETIME)
/// Ex [2021-04-02 14:10:7.710909]
///     ^          ^       ^
///     date       time    precision
///
/// Datetime 0 : no time nor date
/// Datetime 1 : date and time with microsecond precision
/// Datetime 2 : date and time with millisecond precision
/// Datetime 3 : only time with microsecond precision
/// Datetime 4 : only time with millisecond precision
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
// Sinks
//

struct Payload;

using Sink = std::function<void(const Payload&, Level)>;

struct ConsoleSink {
  void operator()(const Payload&, Level) const;
};

struct ColoredConsoleSink {
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

  /// Try to find out how many payloads are in queue, may not be accurate.
  [[nodiscard]] usize approxPayloadsInQueue() const;

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
  Logger& attachSink(Sink sink, const char* name);

  /// Detach a log sink by name.
  /// Note: The default ConsoleSink is called "default".
  Logger& detachSink(const char* name);

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
  const char* fileName = nullptr;
  const char* functionName = nullptr;
  int lineno = -1;
  Level level = Level::None;
  Timestamp timestamp;
  String msg;
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
  {
    auto res = formatTo(payload.msg, dc::forward<Args>(args)...);
    if (res.isErr()) {
      // TODO cgustafsson: dont allocate memory here
      String str;
      [[maybe_unused]] auto _ =
          formatTo(str, "Failed to format, with error [{}].",
                   toString(res.errValue(), payload.msg.toView()));
      DC_ASSERT(false, str.c_str());
    }
  }

  // Can fail if we cannot allocate memory.
  const bool res = logger.enqueue(dc::move(payload));
  DC_ASSERT(res, "failed to allocate memory");
}

///////////////////////////////////////////////////////////////////////////////
// Bonus
//

/// Will fix the windows console, make it utf8 and enable colors. Noop on non
/// windows.
void windowsFixConsole();

/// Note, it's common to "theme" the terminal, changing the presented color of
/// these following colors. Thus, the color "kYellow" might appear as some
/// other color, for some users.
using ColorType = int;
enum class Color : ColorType {
  Gray = 90,
  BrightRed = 91,
  BrightGreen = 92,
  BrightYellow = 93,
  BrightBlue = 94,
  Magenta = 95,
  Teal = 96,
  White = 97,

  Black = 30,
  Red = 31,
  Green = 32,
  Yellow = 33,
  DarkBlue = 34,
  Purple = 35,
  Blue = 36,
  BrightGray = 37,
};

template <usize kStrLen>
class Paint {
  static_assert(kStrLen > 8, "Must be large enough to hold the coloring.");

 public:
  Paint(const char* str, Color color) {
    DC_ASSERT(strlen(str) < kStrLen, "Trying to paint a too large string.");
    const int res = snprintf(m_str, kStrLen, "\033[%dm%s\033[0m",
                             static_cast<ColorType>(color), str);
    DC_ASSERT(res < static_cast<int>(clamp(
                        kStrLen, static_cast<usize>(0),
                        static_cast<usize>(std::numeric_limits<int>::max()))),
              "Too small buffer.");
    DC_ASSERT(res >= 0, "Encoding error from snprintf.");
    m_currentStrLen = res >= 0 ? static_cast<usize>(res) : 0;
  }
  const char* c_str() const { return m_str; }
  usize size() const { return m_currentStrLen; }
  DC_DELETE_COPY(Paint);

 private:
  usize m_currentStrLen = 0;
  char m_str[kStrLen];
};

}  // namespace dc::log

///////////////////////////////////////////////////////////////////////////////
// Fmt specialization
//

template <>
struct std::formatter<dc::log::Level> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(dc::log::Level level, std::format_context& ctx) const {
    std::string_view str;
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
      case dc::log::Level::Raw:
        str = "raw";
        break;
      case dc::log::Level::None:
        str = "none";
        break;
      default:
        str = "unknown";
    }
    return std::format_to(ctx.out(), "{}", str);
  }
};

template <>
struct std::formatter<dc::Timestamp> {
  bool printDate = false;
  bool highPrecisionTime = false;

  constexpr auto parse(std::format_parse_context& ctx) {
    auto it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
      if (*it == 'd')
        printDate = true;
      else if (*it == 'p')
        highPrecisionTime = true;
      ++it;
    }
    return it;
  }

  auto format(const dc::Timestamp& t, std::format_context& ctx) const {
    // Legacy default: "{<02}:{<02}:{<02.3}"
    // Note: cast u8 to int to avoid std::format treating them as char
    return std::format_to(ctx.out(), "{:0>2}:{:0>2}:{:0>6.3f}",
                          static_cast<int>(t.hour), static_cast<int>(t.minute),
                          t.second);
  }
};

template <u64 kStrLen>
struct std::formatter<dc::log::Paint<kStrLen>> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const dc::log::Paint<kStrLen>& paint,
              std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}",
                          std::string_view(paint.c_str(), paint.size()));
  }
};
