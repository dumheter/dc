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

// ========================================================================== //
// Implementation
// ========================================================================== //

namespace dc::log {

namespace internal {
class State;
static void fixConsole();
void workerLaunch();
[[nodiscard]] bool workerShutdown(u64 timeoutUs);
[[nodiscard]] State& getStateInstance();
static void setLevel(Level level);
}  // namespace internal

void start() {
  internal::fixConsole();
  [[maybe_unused]] auto& a = internal::getStateInstance();
  internal::workerLaunch();
}

bool stopSafely(u64 timeoutUs) { return internal::workerShutdown(timeoutUs); }

void setLevel(Level level) { internal::setLevel(level); }

}  // namespace dc::log

// ========================================================================== //
// Internal
// ========================================================================== //

namespace dc::log::internal {

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

// ========================================================================== //
// State
// ========================================================================== //

class State {
 public:
  using Queue = moodycamel::BlockingConcurrentQueue<Payload>;

  [[nodiscard]] bool push(Payload&& payload) {
    return m_queue.enqueue(std::move(payload));
  }

  [[nodiscard]] Queue& getQueue() { return m_queue; }

  /// Wait on worker dead signal, with timeout.
  /// Should only be done on one thread, since only one signal will be sent.
  [[nodiscard]] bool waitOnWorkerDeadTimeoutUs(u64 timeoutUs) {
    return m_workerDeadSem.wait(timeoutUs);
  }

  /// Worker call when it dies.
  void workerDied() { m_workerDeadSem.signal(); }

  Settings& getSettings() { return m_settings; }
  const Settings& getSettings() const { return m_settings; }

 private:
  Queue m_queue;
  moodycamel::LightweightSemaphore m_workerDeadSem;
  Settings m_settings;
};

[[nodiscard]] State& getStateInstance() {
  static State state;
  return state;
}

[[nodiscard]] bool push(Payload&& payload) {
  State& state = getStateInstance();
  return state.push(std::move(payload));
}

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

inline static void setLevel(Level level) {
  State& state = internal::getStateInstance();
  state.getSettings().level = level;
}

// ========================================================================== //
// Worker
// ========================================================================== //

#if defined(DC_LOG_DEBUG)
static void printPayload(const Payload& payload, Level level, size_t qSize) {
  const Timestamp now = makeTimestamp();
#else
static void printPayload(const Payload& payload, Level level) {
#endif

  if (payload.level >= level) {
    if (payload.level != Level::Raw) {
#if defined(DC_LOG_DEBUG)
      const float diff = now.second > payload.timestamp.second
                             ? now.second - payload.timestamp.second
                             : now.second + 60.f - payload.timestamp.second;
      fmt::print(
          "[{:dp}] #\033[93m{:.6f} q{}\033[0m# [{:7}] [{:16}:{}] [{:10}] {}\n",
          payload.timestamp, diff, qSize, payload.level, payload.fileName,
          payload.lineno, payload.functionName, payload.msg);
#else
      fmt::print("[{:dp}] [{:7}] [{:16}:{}] [{:10}] {}\n", payload.timestamp,
                 payload.level, payload.fileName, payload.lineno,
                 payload.functionName, payload.msg);
#endif
    } else if (payload.level == Level::Raw)
      fmt::print("{}", payload.msg);
  }
}

static void workerRun(State& state) {
  State::Queue& queue = state.getQueue();
  Payload payload;

  for (;;) {
    queue.wait_dequeue(payload);

    if (isShutdownPayload(payload)) break;

#if defined(DC_LOG_DEBUG)
    printPayload(payload, state.getSettings().level, queue.size_approx());
#else
    printPayload(payload, state.getSettings().level);
#endif
  }

#if defined(DC_LOG_DEBUG)
  fmt::print(
      "#\033[93m[{:dp}] ! log worker exit code detected, I die. !\033[0m#\n",
      makeTimestamp());
#endif
  state.workerDied();
}

inline void workerLaunch() {
  State& state = getStateInstance();
  std::thread t(workerRun, dc::MutRef<State>(state));
  t.detach();
}

inline bool workerShutdown(u64 timeoutUs) {
#if defined(DC_LOG_DEBUG)
  const auto before = dc::getTimeUsNoReorder();
#endif
  State& state = getStateInstance();

  bool ok;
  for (int i = 0; i < 5; ++i) {
    ok = state.push(makeShutdownPayload());
    if (ok) break;
  }

  bool didDieOk = false;
  if (ok) didDieOk = state.waitOnWorkerDeadTimeoutUs(timeoutUs);

#if defined(DC_LOG_DEBUG)
  const auto diff = dc::getTimeUsNoReorder() - before;
  fmt::print("#\033[93mShutdown in {:.6f} seconds.\033[0m#\n",
             diff / 1'000'000.f);
#endif

  return didDieOk;
}

}  // namespace dc::log::internal
