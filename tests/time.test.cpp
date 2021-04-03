#include <dc/dtest.hpp>
#include <dc/time.hpp>

struct [[nodiscard]] PayloadLarge {
  const char* fileName;
  const char* functionName;
  int lineno;
  dc::log::Level level;
  dc::TimestampLarge timestamp;
  std::string msg;
};

DTEST(getTimeUs) {
  const u64 beforeUs = dc::getTimeUs();
  dc::sleepMs(5);  // minimum 5 milliseconds, probably more.
  const u64 afterUs = dc::getTimeUs();
  const u64 net = afterUs - beforeUs;

  LOG_INFO("timestamp: {}, timestampLarge: {}, Payload: {}, PayloadLarge: {}",
           sizeof(dc::Timestamp), sizeof(dc::TimestampLarge),
           sizeof(dc::log::Payload), sizeof(PayloadLarge));

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