#include <dc/dtest.hpp>
#include <dc/time.hpp>

DTEST(getTimeUs) {
  const u64 beforeUs = dc::getTimeUs();
  dc::sleepMs(5);  // minimum 5 milliseconds, probably more.
  const u64 afterUs = dc::getTimeUs();
  const u64 net = afterUs - beforeUs;

  DASSERT_TRUE(net > 5'000);
}

DTEST(getTimeUsNoReorder) {
  const u64 beforeUs = dc::getTimeUsNoReorder();
  dc::sleepMs(5);  // minimum 5 milliseconds, probably more.
  const u64 afterUs = dc::getTimeUsNoReorder();
  const u64 net = afterUs - beforeUs;

  DASSERT_TRUE(net > 5'000);
}

DTEST(timestamp) {
  const auto a = dc::makeTimestamp();
  dc::sleepMs(5);
  const auto b = dc::makeTimestamp();
  DASSERT_TRUE(a.second < b.second);
}
