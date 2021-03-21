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
#include <dc/dlog.hpp>
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

  [[maybe_unused]] auto& a = internal::dlogStateInstance();
}

// ========================================================================== //
// State
// ========================================================================== //

class DlogState {
 public:
  using Queue = moodycamel::BlockingConcurrentQueue<LogPayload>;

  bool pushLog(LogPayload&& log) { return m_queue.enqueue(std::move(log)); }

  Queue& getQueue() { return m_queue; }

 private:
  Queue m_queue;
};

[[nodiscard]] DlogState& dlogStateInstance() {
  static DlogState dlogState;
  return dlogState;
}

[[nodiscard]] bool pushLog(LogPayload&& log) {
  DlogState& dlogState = dlogStateInstance();
  return dlogState.pushLog(std::move(log));
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

static void printLogPayload(const LogPayload& log) {
  if (log.level != Level::Raw)
    fmt::print("[{:dp}] [{:7}] [{:16}:{}] [{:10}] {}\n", log.timestamp,
               log.level, log.fileName, log.lineno, log.functionName, log.msg);
  else
    fmt::print("{}", log.msg);
}

static void DlogWorkerRun(DlogState::Queue& queue) {
  LogPayload log;
  for (;;) {
    queue.wait_dequeue(log);

    if (isShutdownLogPayload(log)) break;

    printLogPayload(log);
  }
  fmt::print("[{:dp}] ! dlog worker exit code detected, I die. !\n",
             makeTimestamp());
}

void DlogWorkerLaunch() {
  DlogState& dlogState = dlogStateInstance();
  std::thread t(DlogWorkerRun,
                dc::MutRef<DlogState::Queue>(dlogState.getQueue()));
  t.detach();
}

void DlogWorkerShutdown() {
  DlogState& dlogState = dlogStateInstance();
  dlogState.pushLog(makeShutdownLogPayload());
}

}  // namespace dc::internal
