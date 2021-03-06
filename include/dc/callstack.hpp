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

#include <dc/macros.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>

///////////////////////////////////////////////////////////////////////////////
// Quickstart
//

/// Code usage example
/*
```cpp
int main(int, char**) {
  const Result<Callstack, CallstackErr> result = buildCallstack();
  const String& callstack =
    result
    .match([](const Callstack& cs) { return cs.toString(); },
           [](const CallstackErr& err) { return err.toString(); });
  LOG_INFO("{}", callstack);

  return 0;
}
```
*/
/// On linux, you need link option '-rdynamic' for linker to export function
/// names to the dynamic symbol table.

///////////////////////////////////////////////////////////////////////////////

namespace dc {

class Callstack;
class CallstackErr;

Result<Callstack, CallstackErr> buildCallstack();

///////////////////////////////////////////////////////////////////////////////

class Callstack {
 public:
  Callstack(StringView callstack) : m_callstack(callstack) {}

  Callstack(Callstack&& other) noexcept
      : m_callstack(dc::move(other.m_callstack)) {}

  Callstack& operator=(Callstack&& other) {
    if (this != &other) m_callstack = dc::move(other.m_callstack);

    return *this;
  }

  // use .clone() to explicitly copy
  DC_DELETE_COPY(Callstack);

  const String& toString() const { return m_callstack; }
  String& toString() { return m_callstack; }

  Callstack clone() const { return Callstack(m_callstack); }

 private:
  Callstack(const String& callstack) : m_callstack(callstack.clone()) {}

 private:
  String m_callstack;
};

class CallstackErr {
 public:
  CallstackErr(u64 err, int line) : m_err(err), m_line(line) {}

  u64 getErrCode() const { return m_err; }

  String toString() const;

  int getLine() const { return m_line; }

 private:
  u64 m_err;
  int m_line;
};

}  // namespace dc
