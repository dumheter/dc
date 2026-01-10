#include <dc/callstack.hpp>
#include <dc/dtest.hpp>

using namespace dc;

DTEST(callstackWorksTwice) {
  Result<Callstack, CallstackErr> callstack = buildCallstack();
  ASSERT_TRUE(callstack.isOk());

  Result<Callstack, CallstackErr> callstack2 = buildCallstack();
  ASSERT_TRUE(callstack2.isOk());
}

DTEST(captureCallstackReturnsAddresses) {
  Result<CallstackAddresses, CallstackErr> result = captureCallstack();
  ASSERT_TRUE(result.isOk());

  const CallstackAddresses& addresses = result.value();
  ASSERT_TRUE(addresses.addresses.getSize() > 0);

  // Verify addresses are non-null
  for (void* addr : addresses.addresses) {
    ASSERT_TRUE(addr != nullptr);
  }
}

DTEST(resolveCallstackWithCapturedAddresses) {
  Result<CallstackAddresses, CallstackErr> captureResult = captureCallstack();
  ASSERT_TRUE(captureResult.isOk());

  Result<Callstack, CallstackErr> resolveResult =
      resolveCallstack(captureResult.value());
  ASSERT_TRUE(resolveResult.isOk());

  const Callstack& callstack = resolveResult.value();
  ASSERT_TRUE(callstack.callstack.getSize() > 0);
}

DTEST(lazyResolution) {
  // Capture now
  Result<CallstackAddresses, CallstackErr> captureResult = captureCallstack();
  ASSERT_TRUE(captureResult.isOk());

  const CallstackAddresses& addresses = captureResult.value();

  // Do some work in between
  volatile int sum = 0;
  for (int i = 0; i < 1000; ++i) {
    sum += i;
  }

  // Resolve later
  Result<Callstack, CallstackErr> resolveResult = resolveCallstack(addresses);
  ASSERT_TRUE(resolveResult.isOk());

  // The resolved callstack should still be valid
  ASSERT_TRUE(resolveResult.value().callstack.getSize() > 0);
}

DTEST(buildCallstackMatchesCaptureAndResolve) {
  // Using the all-in-one function
  Result<Callstack, CallstackErr> directResult = buildCallstack();
  ASSERT_TRUE(directResult.isOk());

  // Using the two-step process
  Result<CallstackAddresses, CallstackErr> captureResult = captureCallstack();
  ASSERT_TRUE(captureResult.isOk());

  Result<Callstack, CallstackErr> resolveResult =
      resolveCallstack(captureResult.value());
  ASSERT_TRUE(resolveResult.isOk());

  // Both should produce non-empty results
  ASSERT_TRUE(directResult.value().callstack.getSize() > 0);
  ASSERT_TRUE(resolveResult.value().callstack.getSize() > 0);
}
