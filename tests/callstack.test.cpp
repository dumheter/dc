#include <dc/callstack.hpp>
#include <dc/dtest.hpp>

DTEST(callstackWorksTwice) {
  dc::Result<dc::Callstack, dc::CallstackErr> callstack = dc::buildCallstack();
  DASSERT_TRUE(callstack.isOk());

  dc::Result<dc::Callstack, dc::CallstackErr> callstack2 = dc::buildCallstack();
  DASSERT_TRUE(callstack2.isOk());
}
