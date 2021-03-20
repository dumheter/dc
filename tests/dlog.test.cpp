#include <dc/dlog.hpp>
#include <dc/dtest.hpp>
#include <dc/time.hpp>

DTEST_VIP;

DTEST(dlog_testing) {
	DASSERT_TRUE(true);

	DLOG_INIT();

	dc::internal::DlogWorkerLaunch();

	DLOG_INFO("the logger is working");

	dc::sleepMs(1);

	dc::internal::DlogWorkerShutdown();

	dc::sleepMs(1);
}
