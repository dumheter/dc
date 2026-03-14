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

#include <atomic>
#include <condition_variable>
#include <dc/macros.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>
#include <memory>
#include <mutex>

namespace dc {

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobCounter
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Shared completion counter for a batch of jobs.
///
/// Each job in the batch decrements the counter when it finishes. When the
/// counter reaches zero the condition variable is notified, unblocking any
/// thread waiting in JobHandle::await().
///
/// Not intended to be used directly — obtain one via JobHandle.
struct JobCounter {
  explicit JobCounter(u32 count) : m_count(count) {}

  DC_DELETE_COPY(JobCounter);
  DC_DELETE_MOVE(JobCounter);

  /// Called by a worker thread when one job in the batch completes.
  /// Notifies waiters when the counter reaches zero.
  void decrement() {
    // Release so that job side-effects are visible to the awaiting thread.
    if (m_count.fetch_sub(1u, std::memory_order_release) == 1u) {
      std::scoped_lock lock(m_mutex);
      m_cv.notify_all();
    }
  }

  /// Block the calling thread until all jobs in the batch have completed.
  void wait() {
    std::unique_lock lock(m_mutex);
    // Acquire so we see all job side-effects written before decrement.
    m_cv.wait(lock,
              [this] { return m_count.load(std::memory_order_acquire) == 0u; });
  }

  [[nodiscard]] bool isDone() const {
    return m_count.load(std::memory_order_acquire) == 0u;
  }

 private:
  std::atomic<u32> m_count;
  std::mutex m_mutex;
  std::condition_variable m_cv;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobHandle
////////////////////////////////////////////////////////////////////////////////////////////////////

/// A lightweight handle representing a batch of jobs submitted to the
/// JobSystem.
///
/// Multiple handles may share the same underlying JobCounter (copying a handle
/// is cheap — it copies the shared_ptr). The batch is considered complete when
/// every job that was part of the batch has called its completion callback.
///
/// Usage:
/// @code
///   dc::List<dc::Job> jobs;
///   // ... fill jobs ...
///   dc::JobHandle handle = js.add(jobs);
///   handle.await();  // block until all jobs are done
/// @endcode
class JobHandle {
 public:
  /// Construct a handle that wraps an existing counter.
  explicit JobHandle(std::shared_ptr<JobCounter> counter)
      : m_counter(dc::move(counter)) {}

  DC_DEFAULT_COPY(JobHandle);
  DC_DEFAULT_MOVE(JobHandle);

  /// Block the calling thread until all jobs in the batch have completed.
  void await() const { m_counter->wait(); }

  /// Returns true if all jobs in the batch have completed.
  [[nodiscard]] bool isDone() const { return m_counter->isDone(); }

 private:
  std::shared_ptr<JobCounter> m_counter;
};

}  // namespace dc
