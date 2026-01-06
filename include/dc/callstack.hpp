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

///////////////////////////////////////////////////////////////////////////////
// Quickstart
//

/// Code usage example - All-in-one function
/*
```cpp
int main(int, char**) {
  const Result<Callstack, CallstackErr> result
 * = buildCallstack();
  const String& callstack =
    result
 .match([](const
 * Callstack& cs) { return cs.callstack; },
           [](const CallstackErr&
 * err) { return err.toString(); });
  LOG_INFO("{}", callstack);

  return
 * 0;
}
```
*/

/// Code usage example - Lazy resolution (capture now, resolve later)
/*
```cpp
int main(int, char**) {
  // Fast: Capture callstack addresses
  const
 * Result<CallstackAddresses, CallstackErr> addresses = captureCallstack();
  

 * // ... do some work ...
  
  // Resolve addresses to human-readable format
 * when needed
  const Result<Callstack, CallstackErr> result =
 * resolveCallstack(addresses.value());
  const String& callstack =
    result

 * .match([](const Callstack& cs) { return cs.callstack; },
           [](const
 * CallstackErr& err) { return err.toString(); });
  LOG_INFO("{}",
 * callstack);

  return 0;
}
```
*/
/// On linux, you need link option '-rdynamic' for linker to export function
/// names to the dynamic symbol table.

///////////////////////////////////////////////////////////////////////////////

namespace dc {

struct Callstack;
struct CallstackAddresses;
struct CallstackErr;

Result<CallstackAddresses, CallstackErr> captureCallstack();
Result<Callstack, CallstackErr> resolveCallstack(
    const CallstackAddresses& addresses);
Result<Callstack, CallstackErr> buildCallstack();

///////////////////////////////////////////////////////////////////////////////

struct CallstackAddresses {
  List<void*> addresses;
};

struct Callstack {
  String callstack;
};

struct CallstackErr {
  enum class ErrType {
    Sys,
    Fmt,
  };

  CallstackErr(u64 errorCode, ErrType errorType, int lineNumber)
      : errCode(errorCode), errType(errorType), line(lineNumber) {}

  String toString() const;

  u64 errCode;
  ErrType errType;
  int line;
};

}  // namespace dc
