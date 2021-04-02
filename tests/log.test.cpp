#include <dc/dtest.hpp>
#include <dc/log.hpp>
#include <dc/time.hpp>
#include <thread>

DTEST(is_logging_correctly) {
  struct BufferSink {
    BufferSink(std::string& buf) : m_buf(buf) {}

    void operator()(const dc::log::Payload& payload, dc::log::Level) {
      m_buf += payload.msg;
    }

    std::string& m_buf;
  };

  std::string buf;
  dc::log::Logger logger(BufferSink(buf), "test sink");
  dc::log::init(logger);

  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Verbose,
                       logger, "you");
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Verbose,
                       logger, " are");
  dc::log::makePayload(DC_FILENAME, __func__, __LINE__, dc::log::Level::Verbose,
                       logger, " awesome!");

  const bool loggerStopOk = dc::log::deinit(100'000, logger);
  DASSERT_TRUE(loggerStopOk);
  DASSERT_EQ(buf, "you are awesome!");
}

DTEST(multithreaded_stress_test) {
  // constexpr int threadCount = 5;
  // std::thread threads[threadCount];
  // for (int t = 0; t < threadCount; t++) {
  //   threads[t] = std::thread([t]() {
  //     for (int i = 0; i < 100; ++i) {
  //       DC_VERBOSE("verbose log, iter {}, t {}", i, t);
  //       DC_INFO("informative log, iter {}, t {}", i, t);
  //       DC_WARNING("uh ohe, tier {}, t {}.", i, t);
  //       DC_ERROR("gg, iter {}, t {}.", i, t);
  //       DC_RAW("raw log call, iter {}, t {}.\n", i, t);
  //     }
  //   });
  // }

  // for (int t = 0; t < threadCount; t++) {
  //   threads[t].join();
  // }

  DASSERT_TRUE(false);
}
