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

#include <dc/assert.hpp>
#include <dc/job_system.hpp>
#include <dc/list.hpp>
#include <dc/traits.hpp>
#include <thread>

namespace dc {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Worker thread loop
////////////////////////////////////////////////////////////////////////////////////////////////////

static void workerLoop(Worker& worker) {
  while (true) {
    // Wait until there is work or a shutdown signal. The predicate is
    // evaluated while holding worker.mutex, which is the same mutex the
    // producer holds when it calls notify_one(). This eliminates the
    // lost-wakeup race: a notify cannot be missed between the predicate
    // check and the actual sleep.
    {
      std::unique_lock lock(worker.mutex);
      worker.cv.wait(lock, [&worker] {
        return !worker.ring.isEmpty() || worker.shutdown;
      });

      // Check exit condition while still holding the mutex and before
      // releasing it to drain. If shutdown was requested and the ring is
      // already empty, exit now.
      if (worker.shutdown && worker.ring.isEmpty()) break;
    }

    // Drain all available jobs. We do NOT hold the mutex during job
    // execution — the ring is SPSC so no lock is needed for ring access.
    while (Job* job = worker.ring.remove()) {
      job->run();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobSystem
////////////////////////////////////////////////////////////////////////////////////////////////////

JobSystem::JobSystem(u32 threadCount) : m_rng(std::random_device{}()) {
  if (threadCount == 0) {
    const u32 hwThreads = static_cast<u32>(std::thread::hardware_concurrency());
    threadCount = hwThreads > 0 ? hwThreads : 1;
  }

  m_workers.reserve(threadCount);
  for (u32 i = 0; i < threadCount; ++i) {
    auto worker = std::make_unique<Worker>();
    worker->thread = std::thread(workerLoop, std::ref(*worker));
    m_workers.push_back(dc::move(worker));
  }
}

JobSystem::~JobSystem() {
  // Signal all workers to shut down.
  for (auto& worker : m_workers) {
    {
      std::scoped_lock lock(worker->mutex);
      worker->shutdown = true;
    }
    worker->cv.notify_one();
  }

  // Join all worker threads.
  for (auto& worker : m_workers) {
    if (worker->thread.joinable()) {
      worker->thread.join();
    }
  }
}

void JobSystem::add(Job job) {
  std::scoped_lock lock(m_mutex);

  // First, try to drain any previously overflowed jobs.
  drainOverflow();

  // Try to assign the new job to a worker, starting from a random index.
  const u32 workerCount = static_cast<u32>(m_workers.size());
  const u32 startIndex = randomWorkerIndex();

  for (u32 i = 0; i < workerCount; ++i) {
    const u32 index = (startIndex + i) % workerCount;
    Worker& worker = *m_workers[index];

    if (worker.ring.add(dc::move(job))) {
      // Notify while holding the worker's mutex to prevent a lost-wakeup.
      // Without this, the worker could: check the predicate (empty=true),
      // release its mutex to sleep, and miss a notify that fired in between.
      // Holding worker.mutex here ensures the notify arrives either while
      // the worker is already in cv.wait(), or before it re-evaluates the
      // predicate — both are safe.
      std::scoped_lock workerLock(worker.mutex);
      worker.cv.notify_one();
      return;
    }
  }

  // All worker rings are full — push to the overflow ring.
  if (m_overflowRing.isFull() || m_overflowRing.data == nullptr) {
    const u32 newCapacity =
        m_overflowRing.capacity == 0 ? 64u : m_overflowRing.capacity * 2;
    [[maybe_unused]] const bool ok = m_overflowRing.reserve(newCapacity);
    DC_ASSERT(ok, "Failed to grow overflow ring");
  }

  [[maybe_unused]] const bool added = m_overflowRing.add(dc::move(job));
  DC_ASSERT(added, "Failed to add job to overflow ring after growing");
}

JobHandle JobSystem::add(dc::List<Job>& jobs) {
  const usize count = jobs.getSize();
  auto counter = std::make_shared<JobCounter>(static_cast<u32>(count));

  for (usize i = 0; i < count; ++i) {
    // Capture the job fn by value and the counter by shared_ptr copy so that
    // each lambda keeps the counter alive until it runs.
    Job wrapped{[fn = dc::move(jobs[i].fn), counter] {
      fn();
      counter->decrement();
    }};
    add(dc::move(wrapped));
  }

  return JobHandle{dc::move(counter)};
}

void JobSystem::drainOverflow() {
  // m_mutex must be held by the caller.
  while (!m_overflowRing.isEmpty()) {
    const u32 workerCount = static_cast<u32>(m_workers.size());
    const u32 startIndex = randomWorkerIndex();
    bool dispatched = false;

    for (u32 i = 0; i < workerCount; ++i) {
      const u32 index = (startIndex + i) % workerCount;
      Worker& worker = *m_workers[index];

      Job* overflowJob = m_overflowRing.remove();
      if (!overflowJob) break;

      if (worker.ring.add(dc::move(*overflowJob))) {
        {
          std::scoped_lock workerLock(worker.mutex);
          worker.cv.notify_one();
        }
        dispatched = true;
        break;
      } else {
        // Worker ring full — put the job back. Since we just removed it, the
        // overflow ring has space for it.
        [[maybe_unused]] const bool readded =
            m_overflowRing.add(dc::move(*overflowJob));
        DC_ASSERT(readded, "Failed to re-add job to overflow ring");
        break;
      }
    }

    if (!dispatched) break;
  }
}

u32 JobSystem::randomWorkerIndex() {
  // m_mutex must be held by the caller.
  std::uniform_int_distribution<u32> dist(
      0u, static_cast<u32>(m_workers.size()) - 1u);
  return dist(m_rng);
}

}  // namespace dc
