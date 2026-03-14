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
#include <dc/assert.hpp>
#include <dc/macros.hpp>
#include <dc/math.hpp>
#include <dc/traits.hpp>
#include <dc/types.hpp>

namespace dc {

/// Single-producer / single-consumer lock-free ring buffer.
///
/// Thread safety contract:
///   - Only ONE producer thread may call add().
///   - Only ONE consumer thread may call remove().
///   - Both threads may call size(), isEmpty(), isFull() at any time.
///
/// The capacity must be a power of 2 and is fixed at construction.
/// add() returns false if the ring is full — it never grows.
template <typename T>
class SpscRing {
 public:
  /// Construct with a fixed power-of-2 capacity.
  /// @param capacity Must be > 0. Will be rounded up to the next power of 2.
  explicit SpscRing(u32 capacity) {
    capacity = roundUpToPowerOf2(capacity);
    DC_ASSERT(capacity > 0, "SpscRing capacity must be > 0");
    m_data = new T[capacity];
    m_capacity = capacity;
  }

  ~SpscRing() { delete[] m_data; }

  DC_DELETE_COPY(SpscRing);
  DC_DELETE_MOVE(SpscRing);

  /// Add an element. Called only by the producer thread.
  /// @return false if the ring is full.
  bool add(T&& elem) {
    const u32 write = m_write.load(std::memory_order_relaxed);
    const u32 read = m_read.load(std::memory_order_acquire);

    if (write - read == m_capacity) return false;  // full

    m_data[mask(write)] = dc::move(elem);
    m_write.store(write + 1, std::memory_order_release);
    return true;
  }

  /// Remove the front element. Called only by the consumer thread.
  ///
  /// The returned pointer is valid until the *next* call to remove().
  /// The element is copied into an internal single-slot buffer so that the
  /// ring slot is freed to the producer immediately, yet the caller can safely
  /// read through the returned pointer until the next remove() call.
  ///
  /// @return Pointer to the front element, or nullptr if the ring is empty.
  T* remove() {
    const u32 read = m_read.load(std::memory_order_relaxed);
    const u32 write = m_write.load(std::memory_order_acquire);

    if (read == write) return nullptr;  // empty

    // Copy the value out of the shared ring slot into the consumer-private
    // buffer before advancing m_read.  The producer may not touch the slot
    // until m_read is stored, and we store it only after the copy, so there
    // is no race on the read side.  The caller reads from m_lastRemoved (not
    // from the ring slot), so even if the producer immediately refills the
    // slot after our store, the returned pointer remains valid.
    m_lastRemoved = m_data[mask(read)];
    m_read.store(read + 1, std::memory_order_release);
    return &m_lastRemoved;
  }

  /// Number of elements currently in the ring.
  /// May be called from any thread (approximate when called cross-thread).
  u32 size() const {
    const u32 write = m_write.load(std::memory_order_acquire);
    const u32 read = m_read.load(std::memory_order_acquire);
    return write - read;
  }

  bool isEmpty() const { return size() == 0; }

  bool isFull() const { return size() == m_capacity; }

  u32 capacity() const { return m_capacity; }

 private:
  u32 mask(u32 index) const { return index & (m_capacity - 1); }

  T* m_data = nullptr;
  u32 m_capacity = 0;

  // Pad to separate cache lines to avoid false sharing between producer and
  // consumer.
  alignas(64) std::atomic<u32> m_write{0};

  // Consumer-owned cache line.
  // m_lastRemoved is a private copy of the most recently removed element.
  // remove() copies into this buffer before advancing m_read so the returned
  // pointer stays valid while the caller inspects it, even after the producer
  // refills the original ring slot.
  alignas(64) std::atomic<u32> m_read{0};
  T m_lastRemoved{};
};

}  // namespace dc
