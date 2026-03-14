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

#include <condition_variable>
#include <dc/job/job.hpp>
#include <dc/macros.hpp>
#include <dc/spsc_ring.hpp>
#include <dc/types.hpp>
#include <mutex>
#include <thread>

namespace dc {

/// Per-worker state. Each worker owns a SpscRing of jobs and a thread that
/// drains it. The JobSystem is the sole producer; the worker thread is the
/// sole consumer.
struct Worker {
  static constexpr u32 kRingCapacity = 1024;

  Worker() : ring(kRingCapacity) {}

  DC_DELETE_COPY(Worker);
  DC_DELETE_MOVE(Worker);

  /// The job queue. Written by the JobSystem (producer), read by the worker
  /// thread (consumer). Lock-free SPSC — no mutex needed for ring access.
  SpscRing<Job> ring;

  /// The worker thread. Joins on JobSystem destruction.
  std::thread thread;

  /// Guards m_shutdown and is used with cv to signal the worker.
  std::mutex mutex;

  /// Worker waits on this until work is available or shutdown is requested.
  std::condition_variable cv;

  /// Set to true by the JobSystem before join. Written under mutex.
  bool shutdown = false;
};

}  // namespace dc
