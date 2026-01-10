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

#pragma once

#include <dc/list.hpp>
#include <dc/macros.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>

namespace dc {

struct Callstack;
struct CallstackAddresses;
struct CallstackErr;

/// Fast: Captures the current callstack addresses.
/// Returns a Result containing CallstackAddresses or a CallstackErr.
/// Use this when you want to capture the callstack now and resolve it later.
/// On Linux, you need link option '-rdynamic' for the linker to export function
/// names.
Result<CallstackAddresses, CallstackErr> captureCallstack();

/// Resolves previously captured callstack addresses to a human-readable format.
/// @param addresses The addresses captured by captureCallstack().
/// Returns a Result containing a Callstack with the resolved string or a
/// CallstackErr. Use this to resolve addresses that were captured earlier,
/// allowing you to do other work in between capture and resolution.
Result<Callstack, CallstackErr> resolveCallstack(
    const CallstackAddresses& addresses);

/// All-in-one function that captures and resolves the current callstack.
/// Returns a Result containing a Callstack with the resolved string or a
/// CallstackErr. Use this when you want to capture and resolve in a single
/// call. On Linux, you need link option '-rdynamic' for the linker to export
/// function names.
Result<Callstack, CallstackErr> buildCallstack();

///////////////////////////////////////////////////////////////////////////////

struct CallstackAddresses {
  List<void*> addresses;
};

struct Callstack {
  String callstack;
};

/// Represents an error that occurred during callstack capture or resolution.
/// Can be converted to a string via toString() for error reporting.
struct CallstackErr {
  enum class ErrType {
    Sys,  ///< System-level error during capture/resolve
    Fmt,  ///< Formatting error during string generation
  };

  CallstackErr(u64 errorCode, ErrType errorType, int lineNumber)
      : errCode(errorCode), errType(errorType), line(lineNumber) {}

  /// Converts the error to a human-readable string.
  /// @return A String describing the error.
  String toString() const;

  u64 errCode;
  ErrType errType;
  int line;
};

}  // namespace dc
