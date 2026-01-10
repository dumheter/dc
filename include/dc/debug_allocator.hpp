/**
 * MIT License
 *
 * Copyright (c) 2026 Christoffer Gustafsson
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

#include <dc/allocator.hpp>
#include <dc/callstack.hpp>
#include <dc/macros.hpp>
#include <dc/types.hpp>
#include <functional>
#include <unordered_map>

namespace dc {

class DebugAllocator final : public IAllocator {
 public:
  explicit DebugAllocator(IAllocator& backing = getDefaultAllocator());
  ~DebugAllocator();

  DC_DELETE_COPY(DebugAllocator);
  DC_DELETE_MOVE(DebugAllocator);

  void* alloc(usize count, usize align = kMinimumAlignment) override;
  void* realloc(void* data, usize count,
                usize align = kMinimumAlignment) override;
  void free(void* data) override;

  [[nodiscard]] usize getAllocationCount() const;

  [[nodiscard]] bool hasLeaks() const;

  void reportLeaks() const;

  void setSuppressAbortOnLeak(bool suppress);

  /// Set global flag to suppress abort on leak for all DebugAllocators.
  /// This is useful when running tests that should continue after a leak.
  static void setGlobalSuppressAbortOnLeak(bool suppress);

  /// Set a global callback that is invoked when any DebugAllocator detects
  /// leaks during destruction. The callback receives the number of leaks
  /// detected. This is useful for test frameworks to mark tests as failed.
  using LeakCallback = std::function<void(usize leakCount)>;
  static void setGlobalLeakCallback(LeakCallback callback);

 private:
  struct Record {
    CallstackAddresses callstack;
    usize size;
    usize alignment;
  };

  IAllocator& m_backing;
  std::unordered_map<void*, Record> m_allocations;
  bool m_suppressAbortOnLeak = false;
};

}  // namespace dc
