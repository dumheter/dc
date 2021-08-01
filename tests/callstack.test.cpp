#include <dc/callstack.hpp>
#include <dc/dtest.hpp>

using namespace dc;

DTEST(callstackWorksTwice) {
  Result<Callstack, CallstackErr> callstack = buildCallstack();
  ASSERT_TRUE(callstack.isOk());

  Result<Callstack, CallstackErr> callstack2 = buildCallstack();
  ASSERT_TRUE(callstack2.isOk());
}
