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

#include <dc/callstack.hpp>
#include <dc/debug_allocator.hpp>
#include <dc/log.hpp>
#include <dc/macros.hpp>

#ifdef _WIN32
#if !defined(MEAN_AND_LEAN)
#define MEAN_AND_LEAN
#endif
#if !defined(NO_MIN_MAX)
#define NO_MIN_MAX
#endif
#include <Windows.h>
#else
#include <csignal>
#endif

namespace dc {

DebugAllocator::DebugAllocator(IAllocator& backing) : m_backing(backing) {}

DebugAllocator::~DebugAllocator() {
  if (hasLeaks()) {
    reportLeaks();
#ifdef _WIN32
    RaiseException(kDebugAllocatorLeakException, 0, 0, nullptr);
#else
    raise(kDebugAllocatorLeakSignal);
#endif
  }
}

void* DebugAllocator::alloc(usize count, usize align) {
  void* ptr = m_backing.alloc(count, align);
  if (ptr) {
    Record record;
    auto callstackResult = captureCallstack();
    if (callstackResult.isOk()) {
      record.callstack = dc::move(callstackResult.value());
    }
    record.size = count;
    record.alignment = align;
    m_allocations[ptr] = dc::move(record);
  }
  return ptr;
}

void* DebugAllocator::realloc(void* data, usize count, usize align) {
  if (data == nullptr) {
    return alloc(count, align);
  }

  auto it = m_allocations.find(data);
  Record oldRecord;
  if (it != m_allocations.end()) {
    oldRecord = dc::move(it->second);
    m_allocations.erase(it);
  }

  void* newPtr = m_backing.realloc(data, count, align);
  if (newPtr) {
    Record record;
    auto callstackResult = captureCallstack();
    if (callstackResult.isOk()) {
      record.callstack = dc::move(callstackResult.value());
    }
    record.size = count;
    record.alignment = align;
    m_allocations[newPtr] = dc::move(record);
  }
  return newPtr;
}

void DebugAllocator::free(void* data) {
  if (data) {
    m_allocations.erase(data);
    m_backing.free(data);
  }
}

usize DebugAllocator::getAllocationCount() const {
  return m_allocations.size();
}

bool DebugAllocator::hasLeaks() const { return !m_allocations.empty(); }

void DebugAllocator::reportLeaks() const {
  LOG_ERROR("DebugAllocator: {} memory leak(s) detected!",
            m_allocations.size());
  usize index = 0;
  for (const auto& [ptr, record] : m_allocations) {
    LOG_ERROR("Leak #{}: {} bytes at {} (alignment {})", index, record.size,
              ptr, record.alignment);
    auto resolvedCallstack = resolveCallstack(record.callstack);
    if (resolvedCallstack.isOk()) {
      LOG_ERROR("Allocation callstack:\n{}",
                resolvedCallstack.value().callstack);
    } else {
      LOG_ERROR("Failed to resolve callstack");
    }
    ++index;
  }
}

}  // namespace dc
