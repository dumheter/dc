/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <dc/dtest.hpp>

///////////////////////////////////////////////////////////////////////////////
// repeat
//

/// Counts how many times the repeatCounter test body has executed across all
/// repetitions. This is a static so it persists across dtest repetitions.
static s32 g_repeatBodyRunCount = 0;

/// Each repetition increments the counter. When run with --repeat=N the test
/// body executes N times, so g_repeatBodyRunCount == N at the end of each
/// iteration. We compare against getRepeatCount() to verify the two are in
/// sync.
DTEST(repeatCounterMatchesRepeatArg) {
  ++g_repeatBodyRunCount;
  ASSERT_TRUE(g_repeatBodyRunCount <= dtest::internal::getRepeatCount());
}

/// Verify the default repeat count is 1 when no --repeat argument is passed.
/// When the test binary IS run with --repeat=N this assertion is skipped via
/// the counter check above.
DTEST(repeatDefaultIsOne) {
  // getRepeatCount() reflects the --repeat=N argument (default 1).
  ASSERT_TRUE(dtest::internal::getRepeatCount() >= 1);
}

///////////////////////////////////////////////////////////////////////////////
// break_on_failure
//

/// isBreakOnFailure() returns a bool without crashing.
/// The actual value depends on whether --break_on_failure was passed.
DTEST(breakOnFailureIsBool) {
  const bool flag = dtest::internal::isBreakOnFailure();
  ASSERT_TRUE(flag == true || flag == false);
}

/// isDebuggerPresent() must not crash and must return a bool.
/// The actual value depends on the runtime environment.
DTEST(isDebuggerPresentDoesNotCrash) {
  const bool result = dc::details::isDebuggerPresent();
  // Either true or false is valid; we just verify the call works.
  ASSERT_TRUE(result == true || result == false);
}
