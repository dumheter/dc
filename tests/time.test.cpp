#include <atomic>
#include <dc/dtest.hpp>
#include <dc/time.hpp>

DTEST(getTimeUs) {
  const u64 beforeUs = dc::getTimeUs();
  std::atomic_signal_fence(
      std::memory_order_seq_cst);  //< prevent beforeUs from being reordered
  dc::sleepMs(1);
  const u64 afterUs = dc::getTimeUs();
  const u64 net = afterUs - beforeUs;

  ASSERT_TRUE(net > 0);
}

DTEST(getTimeUsNoReorder) {
  const u64 beforeUs = dc::getTimeUsNoReorder();
  std::atomic_signal_fence(
      std::memory_order_seq_cst);  //< prevent beforeUs from being reordered
  dc::sleepMs(1);
  const u64 afterUs = dc::getTimeUsNoReorder();
  const u64 net = afterUs - beforeUs;

  ASSERT_TRUE(net > 0);
}

DTEST(timestamp) {
  const auto a = dc::makeTimestamp();
  std::atomic_signal_fence(
      std::memory_order_seq_cst);  //< prevent beforeUs from being reordered
  dc::sleepMs(1);
  const auto b = dc::makeTimestamp();
  ASSERT_TRUE(a.second < b.second);
}

DTEST(stopwatch) {
  dc::Stopwatch s;  //< construction also starts the stopwatch
  std::atomic_signal_fence(
      std::memory_order_seq_cst);  //< prevent beforeUs from being reordered
  dc::sleepMs(1);
  const u64 nowUs = s.nowUs();
  s.stop();
  ASSERT_TRUE(nowUs > 0);
  ASSERT_TRUE(s.us() > 0);
}
