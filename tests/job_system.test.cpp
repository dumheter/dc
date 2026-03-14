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

#include <atomic>
#include <dc/dtest.hpp>
#include <dc/job_system.hpp>
#include <dc/list.hpp>
#include <dc/spsc_ring.hpp>
#include <dc/time.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SpscRing tests
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingConstructor) {
  dc::SpscRing<s32> ring(4);
  ASSERT_TRUE(ring.isEmpty());
  ASSERT_FALSE(ring.isFull());
  ASSERT_EQ(ring.size(), 0u);
  ASSERT_EQ(ring.capacity(), 4u);
}

DTEST(spscRingRoundsUpCapacityToPowerOfTwo) {
  dc::SpscRing<s32> ring(5);
  ASSERT_EQ(ring.capacity(), 8u);
}

DTEST(spscRingAddAndRemove) {
  dc::SpscRing<s32> ring(4);

  s32 val = 42;
  ASSERT_TRUE(ring.add(dc::move(val)));
  ASSERT_EQ(ring.size(), 1u);

  const s32* out = ring.remove();
  ASSERT_NE(out, nullptr);
  ASSERT_EQ(*out, 42);
  ASSERT_TRUE(ring.isEmpty());
}

DTEST(spscRingFIFOOrder) {
  dc::SpscRing<s32> ring(8);

  for (s32 i = 0; i < 5; ++i) {
    s32 v = i;
    ASSERT_TRUE(ring.add(dc::move(v)));
  }

  for (s32 i = 0; i < 5; ++i) {
    const s32* out = ring.remove();
    ASSERT_NE(out, nullptr);
    ASSERT_EQ(*out, i);
  }
}

DTEST(spscRingReturnsFalseWhenFull) {
  dc::SpscRing<s32> ring(2);

  s32 a = 1, b = 2, c = 3;
  ASSERT_TRUE(ring.add(dc::move(a)));
  ASSERT_TRUE(ring.add(dc::move(b)));
  ASSERT_TRUE(ring.isFull());
  ASSERT_FALSE(ring.add(dc::move(c)));
}

DTEST(spscRingReturnsNullptrWhenEmpty) {
  dc::SpscRing<s32> ring(4);
  ASSERT_EQ(ring.remove(), nullptr);
}

DTEST(spscRingWraparound) {
  dc::SpscRing<s32> ring(4);

  // Fill and drain twice to exercise wraparound.
  for (s32 round = 0; round < 2; ++round) {
    for (s32 i = 0; i < 4; ++i) {
      s32 v = i + round * 4;
      ASSERT_TRUE(ring.add(dc::move(v)));
    }
    for (s32 i = 0; i < 4; ++i) {
      const s32* out = ring.remove();
      ASSERT_NE(out, nullptr);
      ASSERT_EQ(*out, i + round * 4);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobSystem tests
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Helper: spin-wait for an atomic counter to reach an expected value.
static bool waitForCount(const std::atomic<s32>& counter, s32 expected,
                         int timeoutMs = 2000) {
  for (int i = 0; i < timeoutMs; ++i) {
    if (counter.load(std::memory_order_acquire) == expected) return true;
    dc::sleepMs(1);
  }
  return false;
}

DTEST(jobSystemConstructDefault) {
  // Should not crash; uses hardware_concurrency() workers.
  dc::JobSystem js;
  ASSERT_TRUE(js.workerCount() > 0u);
}

DTEST(jobSystemConstructCustomThreadCount) {
  dc::JobSystem js(2);
  ASSERT_EQ(js.workerCount(), 2u);
}

DTEST(jobSystemAddSingleJob) {
  dc::JobSystem js(2);
  std::atomic<s32> counter{0};

  js.add(
      dc::Job{[&counter] { counter.fetch_add(1, std::memory_order_release); }});

  ASSERT_TRUE(waitForCount(counter, 1));
}

DTEST(jobSystemAddManyJobs) {
  dc::JobSystem js(4);
  constexpr s32 kJobCount = 100;
  std::atomic<s32> counter{0};

  for (s32 i = 0; i < kJobCount; ++i) {
    js.add(dc::Job{
        [&counter] { counter.fetch_add(1, std::memory_order_release); }});
  }

  ASSERT_TRUE(waitForCount(counter, kJobCount));
}

DTEST(jobSystemAddFromMultipleThreads) {
  dc::JobSystem js(4);
  constexpr s32 kThreads = 4;
  constexpr s32 kJobsPerThread = 25;
  constexpr s32 kTotal = kThreads * kJobsPerThread;
  std::atomic<s32> counter{0};

  std::thread producers[kThreads];
  for (s32 t = 0; t < kThreads; ++t) {
    producers[t] = std::thread([&js, &counter] {
      for (s32 i = 0; i < kJobsPerThread; ++i) {
        js.add(dc::Job{
            [&counter] { counter.fetch_add(1, std::memory_order_release); }});
      }
    });
  }

  for (s32 t = 0; t < kThreads; ++t) {
    producers[t].join();
  }

  ASSERT_TRUE(waitForCount(counter, kTotal));
}

DTEST(jobSystemShutdownDrainsWorkerRings) {
  // Jobs dispatched just before destruction should still complete because the
  // worker loop drains its ring before honoring shutdown.
  std::atomic<s32> counter{0};
  constexpr s32 kJobCount = 20;

  {
    dc::JobSystem js(2);
    for (s32 i = 0; i < kJobCount; ++i) {
      js.add(dc::Job{
          [&counter] { counter.fetch_add(1, std::memory_order_release); }});
    }
    // Destructor joins workers here — workers will finish their rings first.
  }

  ASSERT_EQ(counter.load(std::memory_order_acquire), kJobCount);
}

DTEST(jobSystemOverflowRingHandlesFullWorkerRings) {
  // Use a single worker with 1 worker thread and submit more jobs than fit in
  // one ring (Worker::kRingCapacity = 1024). The overflow ring should handle
  // the excess.
  constexpr s32 kJobCount = 2048;
  std::atomic<s32> counter{0};

  {
    dc::JobSystem js(1);
    for (s32 i = 0; i < kJobCount; ++i) {
      js.add(dc::Job{
          [&counter] { counter.fetch_add(1, std::memory_order_release); }});
    }
    // Wait for jobs to complete before destruction.
    ASSERT_TRUE(waitForCount(counter, kJobCount, 5000));
  }

  ASSERT_EQ(counter.load(std::memory_order_acquire), kJobCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobHandle / batch add tests
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(jobHandleAwaitSmallBatch) {
  // Submit a small batch via the List overload and block until all finish.
  dc::JobSystem js(4);
  constexpr s32 kJobCount = 16;
  std::atomic<s32> counter{0};

  dc::List<dc::Job> jobs;
  for (s32 i = 0; i < kJobCount; ++i) {
    jobs.add(dc::Job{
        [&counter] { counter.fetch_add(1, std::memory_order_release); }});
  }

  dc::JobHandle handle = js.add(jobs);
  handle.await();

  ASSERT_EQ(counter.load(std::memory_order_acquire), kJobCount);
}

DTEST(jobHandleAwaitLargeBatch) {
  // 500 jobs — the core use-case described in the feature request.
  dc::JobSystem js(4);
  constexpr s32 kJobCount = 500;
  std::atomic<s32> counter{0};

  dc::List<dc::Job> jobs;
  for (s32 i = 0; i < kJobCount; ++i) {
    jobs.add(dc::Job{
        [&counter] { counter.fetch_add(1, std::memory_order_release); }});
  }

  dc::JobHandle handle = js.add(jobs);
  handle.await();

  ASSERT_EQ(counter.load(std::memory_order_acquire), kJobCount);
}

DTEST(jobHandleIsDoneAfterAwait) {
  dc::JobSystem js(2);
  std::atomic<s32> counter{0};

  dc::List<dc::Job> jobs;
  jobs.add(
      dc::Job{[&counter] { counter.fetch_add(1, std::memory_order_release); }});

  dc::JobHandle handle = js.add(jobs);
  handle.await();

  ASSERT_TRUE(handle.isDone());
  ASSERT_EQ(counter.load(std::memory_order_acquire), 1);
}

DTEST(jobHandleCopySharesCompletion) {
  // Two handles sharing the same counter — both become done at the same time.
  dc::JobSystem js(2);
  std::atomic<s32> counter{0};

  dc::List<dc::Job> jobs;
  jobs.add(
      dc::Job{[&counter] { counter.fetch_add(1, std::memory_order_release); }});

  dc::JobHandle h1 = js.add(jobs);
  dc::JobHandle h2 = h1;  // copy — shared counter

  h1.await();

  ASSERT_TRUE(h2.isDone());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distribution tests
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(jobBatchSpreadAcrossWorkers) {
  // Verify that a large batch of jobs is spread somewhat evenly across all
  // workers. Each job records the thread ID it ran on. After all jobs
  // complete we check that every worker thread handled at least
  // (1 / workerCount) * 0.25 of the total jobs — a generous lower bound that
  // still catches a scheduler that routes everything to a single worker.
  constexpr u32 kWorkerCount = 4;
  constexpr s32 kJobCount = 400;  // 100 per worker on average

  dc::JobSystem js(kWorkerCount);

  std::mutex mapMutex;
  std::unordered_map<std::thread::id, s32> jobsPerThread;

  dc::List<dc::Job> jobs;
  for (s32 i = 0; i < kJobCount; ++i) {
    jobs.add(dc::Job{[&mapMutex, &jobsPerThread] {
      const auto id = std::this_thread::get_id();
      std::scoped_lock lock(mapMutex);
      jobsPerThread[id]++;
    }});
  }

  dc::JobHandle handle = js.add(jobs);
  handle.await();

  // Every worker thread should have received work.
  ASSERT_EQ(static_cast<u32>(jobsPerThread.size()), kWorkerCount);

  // No single worker should have received more than 75% of all jobs. With a
  // uniform random assignment the expected max is ~25%+noise, so 75% is a
  // very conservative ceiling that only triggers on a broken scheduler.
  const s32 maxAllowed = static_cast<s32>(kJobCount * 3 / 4);
  for (const auto& [id, count] : jobsPerThread) {
    ASSERT_TRUE(count <= maxAllowed);
  }

  // Each worker should have received at least 5% of all jobs. This rules out
  // a scheduler that starves some workers entirely while still being lenient
  // enough not to flake under heavy system load.
  const s32 minRequired = static_cast<s32>(kJobCount * 5 / 100);
  for (const auto& [id, count] : jobsPerThread) {
    ASSERT_TRUE(count >= minRequired);
  }
}
