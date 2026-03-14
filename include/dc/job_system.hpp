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

#include <dc/job/job.hpp>
#include <dc/job/job_handle.hpp>
#include <dc/job/worker.hpp>
#include <dc/list.hpp>
#include <dc/macros.hpp>
#include <dc/ring.hpp>
#include <dc/types.hpp>
#include <memory>
#include <mutex>
#include <random>
#include <vector>

namespace dc {

/// Global work/job system.
///
/// Create one instance per application. Thread-safe: jobs may be added
/// from any thread concurrently.
///
/// Workers each own a lock-free SpscRing<Job>. The JobSystem randomly assigns
/// incoming jobs to a worker and notifies it via a condition variable. If all
/// worker rings are full, jobs are placed in an overflow ring (protected by
/// m_mutex) which is drained on the next add call.
///
/// Lifecycle is RAII: the constructor starts worker threads and the destructor
/// joins them after signaling shutdown. Jobs already in a worker's ring when
/// shutdown begins will still be executed.
///
/// Usage:
/// @code
///   dc::JobSystem js;
///   js.add(dc::Job{[] { doWork(); }});
/// @endcode
class JobSystem {
 public:
  /// Start the job system.
  /// @param threadCount Number of worker threads. Pass 0 to use
  ///                    std::thread::hardware_concurrency().
  explicit JobSystem(u32 threadCount = 0);

  /// Signal all workers to stop and join their threads.
  /// Pending jobs in the overflow ring may not be executed.
  /// Jobs already added to worker rings will be completed.
  ~JobSystem();

  DC_DELETE_COPY(JobSystem);
  DC_DELETE_MOVE(JobSystem);

  /// Add a job to one of the workers. Thread-safe.
  ///
  /// Selects a worker at random and attempts to add the job to its ring.
  /// If the chosen worker's ring is full, tries remaining workers in order.
  /// If all rings are full, the job is queued in the overflow ring and will
  /// be added on the next call to add().
  void add(Job job);

  /// Add a batch of jobs and return a JobHandle that can be awaited.
  ///
  /// Each job in the list is wrapped so that it decrements a shared counter
  /// when it finishes. When all jobs have completed the counter reaches zero
  /// and any thread blocked in JobHandle::await() is unblocked.
  ///
  /// @param jobs List of jobs to schedule.
  /// @return A JobHandle whose await() blocks until all jobs finish.
  [[nodiscard]] JobHandle add(dc::List<Job>& jobs);

  /// Number of worker threads.
  [[nodiscard]] u32 workerCount() const {
    return static_cast<u32>(m_workers.size());
  }

 private:
  /// Try to drain the overflow ring into worker rings. Must be called with
  /// m_mutex held.
  void drainOverflow();

  /// Pick a random starting worker index. Must be called with m_mutex held.
  u32 randomWorkerIndex();

  std::mutex m_mutex;

  /// Fallback queue for jobs that could not be assigned to any worker ring.
  /// Guarded by m_mutex. Grows automatically when needed.
  dc::Ring<Job> m_overflowRing;

  /// Worker pool. Fixed size after construction. Workers are heap-allocated to
  /// avoid issues with non-movable mutexes/condition_variables in a vector.
  std::vector<std::unique_ptr<Worker>> m_workers;

  /// Random number generator for worker selection. Used under m_mutex.
  std::mt19937 m_rng;
};

}  // namespace dc
