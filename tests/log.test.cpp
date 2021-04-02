#include <dc/dtest.hpp>
#include <dc/log.hpp>
#include <dc/time.hpp>
#include <thread>

DTEST(dlog_testing) {
  // TODO cgustafsson: make log have changeable log state's
  // TODO cgustafsson: have dtest use dc:log
  // TODO cgustafsson: create a new log state, for this function, swap
  // the dtest/global log state for a local one, and use it to test, then swap
  // back.
  // dc::log::init();

  // dc::log::State* dtestLogState = dc::log::newState();
  // dc::log::swapStateInstance(dtestLogState);

  // dc::log::setLevel(dc::log::Level::None);

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

  // const bool ok = dc::log::deinit(5'000'000);
  // DASSERT_TRUE(ok);

  // dc::log::swapStateInstance(dtestLogState);

  DASSERT_TRUE(true);
}
