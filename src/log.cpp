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

#include <moodycamel/blockingconcurrentqueue.h>
#include <moodycamel/lightweightsemaphore.h>

#include <dc/core.hpp>
#include <dc/log.hpp>
#include <dc/math.hpp>
#include <dc/platform.hpp>
#include <dc/time.hpp>
#include <dc/traits.hpp>
#include <functional>
#include <thread>

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
#endif

namespace dc::log {

[[nodiscard]] Logger& getGlobalLogger() {
  static Logger logger;
  return logger;
}

void init(Logger& logger) { logger.start(); }

bool deinit(u64 timeoutUs, Logger& logger) { return logger.stop(timeoutUs); }

void setLevel(Level level, Logger& logger) { logger.setLevel(level); }

// ========================================================================== //
// Logger
// ========================================================================== //

struct TaggedSink {
  Sink sink;
  u32 tag;
};

struct Logger::Data {
  using Queue = moodycamel::BlockingConcurrentQueue<Payload>;
  moodycamel::LightweightSemaphore loggerDeadSem;
  Queue queue;
  std::vector<TaggedSink> sinks;
};

Logger::Logger(Sink sink, const char* name) : m_data(new Data) {
  m_data->sinks.push_back({std::move(sink), dc::hash32fnv1a(name)});
}

Logger::~Logger() { delete m_data; }

inline static Payload makeShutdownPayload() {
  Payload payload;
  payload.fileName = nullptr;
  payload.functionName = nullptr;
  payload.lineno = -418;
  payload.timestamp = {};
  payload.level = Level::None;
  return payload;
}

inline static bool isShutdownPayload(const Payload& payload) {
  return payload.lineno == -418 && payload.level == Level::None &&
         payload.fileName == nullptr && payload.functionName == nullptr;
}

void Logger::start() {
  std::thread t([this] { run(); });
  t.detach();
}

bool Logger::stop(u64 timeoutUs) {
#if defined(DC_LOG_DEBUG)
  const auto before = dc::getTimeUsNoReorder();
#endif

  bool ok;
  for (int i = 0; i < 5; ++i) {
    ok = enqueue(makeShutdownPayload());
    if (ok) break;
  }

  bool didDieOk = false;
  if (ok) didDieOk = waitOnLoggerDeadTimeoutUs(timeoutUs);

#if defined(DC_LOG_DEBUG)
  const auto diff = dc::getTimeUsNoReorder() - before;
  fmt::print("\033[93m#{:dp}# logger shutdown time: {:.6f}s.\033[0m\n",
             makeTimestamp(), diff / 1'000'000.f);
#endif

  return didDieOk;
}

[[nodiscard]] bool Logger::enqueue(Payload&& payload) {
  return m_data->queue.enqueue(std::move(payload));
}

[[nodiscard]] usize Logger::approxPayloadsInQueue() const {
  return m_data->queue.size_approx();
}

[[nodiscard]] bool Logger::waitOnLoggerDeadTimeoutUs(u64 timeoutUs) {
  return m_data->loggerDeadSem.wait(timeoutUs);
}

void Logger::run() {
  m_isActive = true;
  Payload payload;

  for (;;) {
    m_data->queue.wait_dequeue(payload);
    if (isShutdownPayload(payload)) break;
    for (auto& taggedSink : m_data->sinks)
      if (payload.level >= m_level) taggedSink.sink(payload, m_level);
  }

  // shutdown payload recieved - drain the log backlog
  const u64 now = dc::getTimeUs();
  usize payloadsDrained = 0;
  while (m_data->queue.wait_dequeue_timed(payload, 0)) {
    ++payloadsDrained;
    if (isShutdownPayload(payload))
      continue;  //< protect from multiple shutdowns
    for (auto& taggedSink : m_data->sinks) taggedSink.sink(payload, m_level);
  }

#if defined(DC_LOG_DEBUG)
  const u64 drained = dc::getTimeUs();
  fmt::print(
      "\033[93m#{:dp}# logger exit code detected, dying. drainTime: {:.6f}s, "
      "drainedPayloads: {}\033[0m\n",
      makeTimestamp(), (drained - now) / 1'000'000.0, payloadsDrained);
#endif
  m_data->loggerDeadSem.signal();
  m_isActive = false;
}

void Logger::attachSink(Sink sink, const char* name) {
  m_data->sinks.push_back({std::move(sink), dc::hash32fnv1a(name)});
}

void Logger::detachSink(const char* name) {
  const u32 tag = dc::hash32fnv1a(name);
  m_data->sinks.erase(std::remove_if(m_data->sinks.begin(), m_data->sinks.end(),
                                     [tag](const TaggedSink& taggedSink) {
                                       return taggedSink.tag == tag;
                                     }),
                      m_data->sinks.end());
}

// ========================================================================== //
// Sinks
// ========================================================================== //

void ConsoleSink::operator()(const Payload& payload, Level level) const {
  if (payload.level >= level) {
    if (payload.level != Level::Raw) {
      fmt::print(
#if DC_LOG_PREFIX_DATETIME == 1
          "[{:dp}] "
#elif DC_LOG_PREFIX_DATETIME == 2
          "[{:p}] "
#elif DC_LOG_PREFIX_DATETIME == 3
          "[{}] "
#endif
#if DC_LOG_PREFIX_LEVEL == 1
          "[{:7}] "
#endif
#if DC_LOG_PREFIX_FILESTAMP == 1
          "[{:<26}] "
#endif
#if DC_LOG_PREFIX_FUNCTION == 1
          "[{:10}] "
#endif
          "{}\n",
#if DC_LOG_PREFIX_DATETIME > 0
          payload.timestamp,
#endif
#if DC_LOG_PREFIX_LEVEL == 1
          payload.level,
#endif
#if DC_LOG_PREFIX_FILESTAMP == 1
          fmt::format("{}:{}", payload.fileName, payload.lineno),
#endif
#if DC_LOG_PREFIX_FUNCTION == 1
          payload.functionName,
#endif
          payload.msg);

    } else if (payload.level == Level::Raw)
      fmt::print("{}", payload.msg);
  }
}

// ========================================================================== //
// Bonus
// ========================================================================== //

void windowsFixConsole() {
#if defined(DC_PLATFORM_WINDOWS)
  // Set console encoding
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  // Enable virtual terminal processing
  const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(out, &mode);
  SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

}  // namespace dc::log
