#include <dc/dlog.hpp>
#include <dc/dtest.hpp>
#include <dc/time.hpp>

DTEST(dlog_testing) {
  DASSERT_TRUE(true);

  DLOG_INIT();

  dc::internal::DlogWorkerLaunch();

  DLOG_VERBOSE("the logger is working 1");
  DLOG_INFO("the logger is working 2");
  DLOG_WARNING("the logger is working 3");
  DLOG_ERROR("the logger is working 4");
  DLOG_RAW("the logger is working 5\n");

  dc::sleepMs(1);

  dc::internal::DlogWorkerShutdown();

  dc::sleepMs(5);
}
