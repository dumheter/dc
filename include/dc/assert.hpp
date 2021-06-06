/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
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

#pragma once

/// Assert the condition, with msg.
/// If condition check fails,
///   * log the assert via dc::log
///   * show message box if 'DC_ASSERT_DIALOG' is defined
/// @param condition Condition to assert.
/// @param msg Message to be included in assert notifications.
#define DC_ASSERT(condition, msg) \
  dc::details::dcAssert(!!(condition), msg, __FILE__, __func__, __LINE__)

/// Assert the condition, with msg.
/// If condition check fails,
///   * call debugbreak
///   * log the assert via dc::log
///   * show message box if 'DC_ASSERT_DIALOG' is defined
///   * pump the log queue
///   * exit the program with code '1'.
/// @param condition Condition to assert.
/// @param msg Message to be included in assert notifications.
#define DC_FATAL_ASSERT(condition, msg) \
  dc::details::dcFatalAssert(!!(condition), msg, __FILE__, __func__, __LINE__)

///////////////////////////////////////////////////////////////////////////////

namespace dc::details {

/// Call by using the macro DC_ASSERT
void dcAssert(bool condition, const char* msg, const char* file,
              const char* func, int line);

void dcFatalAssert(bool condition, const char* msg, const char* file,
                   const char* func, int line);

void debugBreak();

}  // namespace dc::details
