#include <dc/dtest.hpp>
#include <dc/log.hpp>
#include <dc/time.hpp>
#include <thread>

struct [[nodiscard]] BufferSink {
  BufferSink(std::string& buf) : m_buf(buf) {}
  void operator()(const dc::log::Payload& payload, dc::log::Level) {
    m_buf += payload.msg;
  }
  std::string& m_buf;
};

DTEST(isLoggingCorrectly) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "test sink");
  logger.start();

  LLOG_INFO(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_INFO(logger, " awesome!");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, "you are awesome!");
}

DTEST(levelVerbose) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::Verbose);
  logger.start();

  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, " :)");
  LLOG_RAW(logger, " rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, "you are awesome! :) rawr");
}

DTEST(levelInfo) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::Info);
  logger.start();

  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, " :)");
  LLOG_RAW(logger, " rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, " are awesome! :) rawr");
}

DTEST(levelWarning) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::Warning);
  logger.start();

  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, " :)");
  LLOG_RAW(logger, " rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, " awesome! :) rawr");
}

DTEST(levelError) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::Error);
  logger.start();

  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, " :)");
  LLOG_RAW(logger, " rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, " :) rawr");
}

DTEST(levelRaw) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::Raw);
  logger.start();

  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, " :)");
  LLOG_RAW(logger, " rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_EQ(buf, " rawr");
}

DTEST(levelNone) {
  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "buf sink");
  logger.setLevel(dc::log::Level::None);
  logger.start();
  LLOG_VERBOSE(logger, "you");
  LLOG_INFO(logger, " are");
  LLOG_WARNING(logger, " awesome!");
  LLOG_ERROR(logger, ":)");
  LLOG_RAW(logger, "rawr");

  const bool stopOk = logger.stop(100'000);
  ASSERT_TRUE(stopOk);
  ASSERT_TRUE(buf.empty());
}

DTEST(multithreadedStressTest) {
  constexpr usize kIterCount = 1000;

  struct Data {
    std::vector<int> verbose;
    std::vector<int> info;
    std::vector<int> warning;
    std::vector<int> error;
    std::vector<int> raw;
    std::vector<int> none;
    bool success = true;
  };

  struct CountSink {
    CountSink(Data& data, usize size) : data(data) {
      data.verbose.resize(size);
      data.info.resize(size);
      data.warning.resize(size);
      data.error.resize(size);
      data.raw.resize(size);
      data.none.resize(size);
    }

    void operator()(const dc::log::Payload& payload, dc::log::Level) {
      switch (payload.level) {
        case dc::log::Level::Verbose:
          ++data.verbose[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        case dc::log::Level::Info:
          ++data.info[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        case dc::log::Level::Warning:
          ++data.warning[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        case dc::log::Level::Error:
          ++data.error[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        case dc::log::Level::Raw:
          ++data.raw[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        case dc::log::Level::None:
          ++data.none[dc::clamp(
              static_cast<usize>(std::atoi(payload.msg.c_str())), 0ul,
              kIterCount)];
          break;
        default:
          data.success = false;
      }
    }

    Data& data;
  };

  Data data;
  dc::log::Logger logger(CountSink(data, kIterCount), "test sink");
  logger.start();

  constexpr int kThreadCount = 8;
  std::thread threads[kThreadCount];

  for (int t = 0; t < kThreadCount; t++) {
    threads[t] = std::thread([&logger = logger, k = kIterCount]() {
      for (usize i = 0; i < k; ++i) {
        LLOG_VERBOSE(logger, "{}", i);
        LLOG_INFO(logger, "{}", i);
        LLOG_WARNING(logger, "{}", i);
        LLOG_ERROR(logger, "{}", i);
        LLOG_RAW(logger, "{}", i);
      }
    });
  }

  for (int t = 0; t < kThreadCount; t++) {
    threads[t].join();
  }

  const bool stopOk = logger.stop(5'000'000);
  const auto allElementsAreK = [k = kThreadCount](const std::vector<int>& v) {
    bool correct = true;
    for (const auto e : v)
      if (e != k) correct = false;
    return correct;
  };

  ASSERT_TRUE(stopOk);
  ASSERT_TRUE(data.success);
  ASSERT_TRUE(allElementsAreK(data.verbose));
  ASSERT_TRUE(allElementsAreK(data.info));
  ASSERT_TRUE(allElementsAreK(data.warning));
  ASSERT_TRUE(allElementsAreK(data.error));
  ASSERT_TRUE(allElementsAreK(data.raw));
}

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

DTEST(canAttachSinkToGlobalLoggerAndDetach) {
  dc::log::Logger& logger = dc::log::getGlobalLogger();
  ASSERT_TRUE(drainLogger(logger));

  std::string buf;
  logger.attachSink(BufferSink(buf), "bufferSink");

  LOG_INFO("you");
  LOG_INFO(" are");
  LOG_INFO(" awesome!");
  ASSERT_TRUE(drainLogger(logger));

  ASSERT_EQ(buf, "you are awesome!");
  logger.detachSink("bufferSink");

  LOG_INFO("this is not added to bufferSink");
  ASSERT_TRUE(drainLogger(logger));

  ASSERT_EQ(buf, "you are awesome!");
}
