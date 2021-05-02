#include <dc/assert.hpp>
#include <dc/dtest.hpp>
#include <dc/log.hpp>

struct [[nodiscard]] BufferSink {
  BufferSink(std::string& buf) : m_buf(buf) {}
  void operator()(const dc::log::Payload& payload, dc::log::Level) {
    m_buf += payload.msg;
  }
  std::string& m_buf;
};

[[nodiscard]] static bool drainLogger(dc::log::Logger& logger,
                                      int timeoutMs = 1'000) {
  int c = 0;
  while (logger.approxPayloadsInQueue()) {
    dc::sleepMs(1);
    if (c++ > timeoutMs) break;
  }
  dc::sleepMs(1);  //< give the logger extra time to stabilize
  return c <= timeoutMs;
}

DTEST(assertFalseWillLogCallstack) {
  dc::log::Logger& logger = dc::log::getGlobalLogger();
  ASSERT_TRUE(drainLogger(logger));

  std::string buf;
  logger.attachSink(BufferSink(buf), "bufferSink");

  DC_ASSERT(false, "test assert :)");
  ASSERT_TRUE(drainLogger(logger));

  logger.detachSink("bufferSink");
  ASSERT_TRUE(drainLogger(logger));

  ASSERT_TRUE(buf.find("dc::details::dcDoAssert") != std::string::npos ||
              buf.find(" [?:?]") != std::string::npos);
}
