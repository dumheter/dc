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

#include <dc/core.hpp>
#include <dc/log.hpp>
#include <dc/platform.hpp>
#include <dc/traits.hpp>
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

namespace dc::internal {

static inline void fixConsole() {
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

void init() {
  fixConsole();

  [[maybe_unused]] auto& a = internal::logStateInstance();
}

// ========================================================================== //
// State
// ========================================================================== //

class LogState {
 public:
  using Queue = moodycamel::BlockingConcurrentQueue<LogPayload>;

  [[nodiscard]] bool pushLog(LogPayload&& log) {
    return m_queue.enqueue(std::move(log));
  }

  [[nodiscard]] Queue& getQueue() { return m_queue; }

  /// Wait on worker dead signal, with timeout.
  /// Should only be done on one thread, since only one signal will be sent.
  [[nodiscard]] bool waitOnWorkerDeadTimeoutUs(u64 timeoutUs) {
    return m_workerDeadSem.wait(timeoutUs);
  }

  /// Worker call when it dies.
  void workerDied() { m_workerDeadSem.signal(); }

 private:
  Queue m_queue;
  moodycamel::LightweightSemaphore m_workerDeadSem;
};

[[nodiscard]] LogState& logStateInstance() {
  static LogState logState;
  return logState;
}

[[nodiscard]] bool pushLog(LogPayload&& log) {
  LogState& logState = logStateInstance();
  return logState.pushLog(std::move(log));
}

inline static LogPayload makeShutdownLogPayload() {
  LogPayload log;
  log.fileName = nullptr;
  log.functionName = nullptr;
  log.lineno = -418;
  log.timestamp = {};
  log.level = Level::Info;
  return log;
}

inline static bool isShutdownLogPayload(const LogPayload& log) {
  return log.lineno == -418 && log.level == Level::Info &&
         log.fileName == nullptr && log.functionName == nullptr;
}

// ========================================================================== //
// Worker
// ========================================================================== //

#if defined(DC_LOG_DEBUG)
static void printLogPayload(const LogPayload& log, size_t qSize) {
  const Timestamp now = makeTimestamp();
#else
static void printLogPayload(const LogPayload& log) {
#endif

  if (log.level != Level::Raw) {
#if defined(DC_LOG_DEBUG)
    const float diff = now.second > log.timestamp.second
                           ? now.second - log.timestamp.second
                           : now.second + 60.f - log.timestamp.second;
    fmt::print(
        "[{:dp}] #\033[93m{:.6f} q{}\033[0m# [{:7}] [{:16}:{}] [{:10}] {}\n",
        log.timestamp, diff, qSize, log.level, log.fileName, log.lineno,
        log.functionName, log.msg);
#else
    fmt::print("[{:dp}] [{:7}] [{:16}:{}] [{:10}] {}\n", log.timestamp,
               log.level, log.fileName, log.lineno, log.functionName, log.msg);
#endif
  } else
    fmt::print("{}", log.msg);
}

static void logWorkerRun(LogState& state) {
  LogState::Queue& queue = state.getQueue();
  LogPayload log;

  for (;;) {
    queue.wait_dequeue(log);

    if (isShutdownLogPayload(log)) break;

#if defined(DC_LOG_DEBUG)
    printLogPayload(log, queue.size_approx());
#else
    printLogPayload(log);
#endif
  }

#if defined(DC_LOG_DEBUG)
  fmt::print(
      "#\033[93m[{:dp}] ! log worker exit code detected, I die. !\033[0m#\n",
      makeTimestamp());
#endif
  state.workerDied();
}

void logWorkerLaunch() {
  LogState& logState = logStateInstance();
  std::thread t(logWorkerRun, dc::MutRef<LogState>(logState));
  t.detach();
}

bool logWorkerShutdown(u64 timeoutUs) {
#if defined(DC_LOG_DEBUG)
  const auto before = dc::getTimeUsNoReorder();
#endif
  LogState& logState = logStateInstance();

  bool ok;
  for (int i = 0; i < 5; ++i) {
    ok = logState.pushLog(makeShutdownLogPayload());
    if (ok) break;
  }

  bool didDieOk = false;
  if (ok) didDieOk = logState.waitOnWorkerDeadTimeoutUs(timeoutUs);

#if defined(DC_LOG_DEBUG)
  const auto diff = dc::getTimeUsNoReorder() - before;
  fmt::print("#\033[93mShutdown in {:.6f} seconds.\033[0m#\n",
             diff / 1'000'000.f);
#endif

  return didDieOk;
}

}  // namespace dc::internal
