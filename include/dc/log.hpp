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

// ========================================================================== //
// Quick Start
// ========================================================================== //

// TODO cgustafsson: write

// ========================================================================== //
// Helper Macros
// ========================================================================== //

/// Log to the global logger.
#define LOG_VERBOSE(...)                                                      \
  do {                                                                        \
    dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                     \
                         dc::log::Level::Verbose, dc::log::getGlobalWorker(), \
                         __VA_ARGS__);                                        \
  } while (0)
#define LOG_INFO(...)                                                      \
  do {                                                                     \
    dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                  \
                         dc::log::Level::Info, dc::log::getGlobalWorker(), \
                         __VA_ARGS__);                                     \
  } while (0)
#define LOG_WARNING(...)                                                      \
  do {                                                                        \
    dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                     \
                         dc::log::Level::Warning, dc::log::getGlobalWorker(), \
                         __VA_ARGS__);                                        \
  } while (0)
#define LOG_ERROR(...)                                                      \
  do {                                                                      \
    dc::log::makePayload(DC_FILENAME, __func__, __LINE__,                   \
                         dc::log::Level::Error, dc::log::getGlobalWorker(), \
                         __VA_ARGS__);                                      \
  } while (0)

/// Log the raw string, without payload
#define LOG_RAW(...)                                                           \
  do {                                                                         \
    dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Raw, \
                         dc::log::getGlobalWorker(), __VA_ARGS__);             \
  } while (0)

// ========================================================================== //
// Log
// ========================================================================== //

namespace dc::log {

class Worker;
[[nodiscard]] Worker& getGlobalWorker();

/// Helper function that will call .start() on the worker, a global one is
/// created / used if none exist.
void init(Worker& worker = getGlobalWorker());

/// Helper function that will call .stop() on the worker, the global one is
/// used if none is provided.
/// @param timeoutUs Specify a maximum time in microseconds, to wait for it to
/// finish.
/// @return If the log worker finished all work and signal its death.
bool deinit(u64 timeoutUs = 1'000'000, Worker& worker = getGlobalWorker());

enum class Level {
  Verbose,
  Info,
  Warning,
  Error,
  Raw,
  None,
};

/// Helper function to set log level.
/// Example:
///   Setting Level::Info and verbose log will be ignored.
void setLevel(Level level, Worker& worker = getGlobalWorker());

struct [[nodiscard]] Settings {
  Level level = Level::Verbose;
};

// ========================================================================== //
// Worker
// ========================================================================== //

struct Payload;

class Worker {
 public:
  Worker();
  DC_DELETE_COPY(Worker);

  /// Start a worker thread which will start run() loop.
  void start();

  /// Exit the run() loop and shutdown the worker thread.
  bool stop(u64 timeoutUs = 1'000'000);

  /// Enqueue a log payload.
  [[nodiscard]] bool enqueue(Payload&& payload);

  /// Wait on worker dead signal, with timeout.
  /// Should only be done on one thread, since only one signal will be sent.
  [[nodiscard]] bool waitOnWorkerDeadTimeoutUs(u64 timeoutUs);

  Settings& getSettings() { return m_settings; }
  const Settings& getSettings() const { return m_settings; }

  /// Is the worker thread active?
  bool isWorking() const { return m_isWorking; }

 private:
  void run();

 private:
  bool m_isWorking = false;
  Settings m_settings;

  struct Data;
  std::unique_ptr<Data> m_data;
};

// ========================================================================== //
// Payload
// ========================================================================== //

struct [[nodiscard]] Payload {
  const char* fileName;
  const char* functionName;
  int lineno;
  Level level;
  Timestamp timestamp;
  std::string msg;
};

template <typename... Args>
inline void makePayload(const char* fileName, const char* functionName,
                        int lineno, Level level, Worker& worker,
                        Args&&... args) {
  Payload payload;
  payload.fileName = fileName;
  payload.functionName = functionName;
  payload.lineno = lineno;
  payload.level = level;
  payload.timestamp = makeTimestamp();
  payload.msg = fmt::format(std::forward<Args>(args)...);

  // Can fail if we cannot allocate memory.
  const bool res = worker.enqueue(std::move(payload));
  DC_ASSERT(res, "failed to allocate memory");
}

// ========================================================================== //
// Sinks
// ========================================================================== //

using Sink = std::function<void(const Payload&, Level)>;

struct ConsoleSink {
  void operator()(const Payload&, Level) const;
};

// ========================================================================== //
// Log Prefix Settings
// ========================================================================== //

#if !defined(DC_LOG_PREFIX_DATETIME)
/// Datetime 0 : no time nor date
/// Datetime 1 : date and time with microsecond precision
/// Datetime 2 : only time with microsecond precision
/// Datetime 3 : only time
#define DC_LOG_PREFIX_DATETIME 1
#endif
#if !defined(DC_LOG_PREFIX_LEVEL)
#define DC_LOG_PREFIX_LEVEL 1
#endif
#if !defined(DC_LOG_PREFIX_FILESTAMP)
#define DC_LOG_PREFIX_FILESTAMP 1
#endif
#if !defined(DC_LOG_PREFIX_FUNCTION)
#define DC_LOG_PREFIX_FUNCTION 1
#endif

// ========================================================================== //
// Bonus
// ========================================================================== //

/// Will fix the windows console, make it utf8 and enable colors. Noop on non
/// windows.
void windowsFixConsole();

}  // namespace dc::log

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
