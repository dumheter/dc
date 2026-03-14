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
#include <dc/spsc_ring.hpp>
#include <dc/string.hpp>
#include <thread>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingConstructor) {
  dc::SpscRing<s32> ring(4);
  ASSERT_TRUE(ring.isEmpty());
  ASSERT_FALSE(ring.isFull());
  ASSERT_EQ(ring.size(), 0u);
  ASSERT_EQ(ring.capacity(), 4u);
}

DTEST(spscRingRoundsUpCapacityToPowerOfTwo) {
  dc::SpscRing<s32> ring5(5);
  ASSERT_EQ(ring5.capacity(), 8u);

  dc::SpscRing<s32> ring7(7);
  ASSERT_EQ(ring7.capacity(), 8u);

  dc::SpscRing<s32> ring16(16);
  ASSERT_EQ(ring16.capacity(), 16u);

  dc::SpscRing<s32> ring1(1);
  ASSERT_EQ(ring1.capacity(), 1u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// add / remove
////////////////////////////////////////////////////////////////////////////////////////////////////

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

DTEST(spscRingReturnsFalseWhenFull) {
  dc::SpscRing<s32> ring(2);

  s32 a = 1, b = 2, c = 3;
  ASSERT_TRUE(ring.add(dc::move(a)));
  ASSERT_TRUE(ring.add(dc::move(b)));
  ASSERT_TRUE(ring.isFull());
  ASSERT_FALSE(ring.add(dc::move(c)));
  ASSERT_EQ(ring.size(), 2u);
}

DTEST(spscRingReturnsNullptrWhenEmpty) {
  dc::SpscRing<s32> ring(4);
  ASSERT_EQ(ring.remove(), nullptr);
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
  ASSERT_TRUE(ring.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// size / isEmpty / isFull state transitions
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingSizeTracking) {
  dc::SpscRing<s32> ring(8);

  ASSERT_EQ(ring.size(), 0u);

  s32 v1 = 1;
  ring.add(dc::move(v1));
  ASSERT_EQ(ring.size(), 1u);

  s32 v2 = 2;
  s32 v3 = 3;
  ring.add(dc::move(v2));
  ring.add(dc::move(v3));
  ASSERT_EQ(ring.size(), 3u);

  ring.remove();
  ASSERT_EQ(ring.size(), 2u);

  ring.remove();
  ring.remove();
  ASSERT_EQ(ring.size(), 0u);
}

DTEST(spscRingIsEmpty) {
  dc::SpscRing<s32> ring(4);

  ASSERT_TRUE(ring.isEmpty());

  s32 v = 10;
  ring.add(dc::move(v));
  ASSERT_FALSE(ring.isEmpty());

  ring.remove();
  ASSERT_TRUE(ring.isEmpty());
}

DTEST(spscRingIsFull) {
  dc::SpscRing<s32> ring(2);

  ASSERT_FALSE(ring.isFull());

  s32 v1 = 1;
  ring.add(dc::move(v1));
  ASSERT_FALSE(ring.isFull());

  s32 v2 = 2;
  ring.add(dc::move(v2));
  ASSERT_TRUE(ring.isFull());

  ring.remove();
  ASSERT_FALSE(ring.isFull());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Wraparound
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingWraparound) {
  dc::SpscRing<s32> ring(4);

  // Fill and drain twice to exercise index wraparound.
  for (s32 round = 0; round < 2; ++round) {
    for (s32 i = 0; i < 4; ++i) {
      s32 v = i + round * 4;
      ASSERT_TRUE(ring.add(dc::move(v)));
    }
    ASSERT_TRUE(ring.isFull());
    for (s32 i = 0; i < 4; ++i) {
      const s32* out = ring.remove();
      ASSERT_NE(out, nullptr);
      ASSERT_EQ(*out, i + round * 4);
    }
    ASSERT_TRUE(ring.isEmpty());
  }
}

DTEST(spscRingContinuousAddRemove) {
  dc::SpscRing<s32> ring(4);

  for (s32 i = 0; i < 100; ++i) {
    s32 v = i;
    ASSERT_TRUE(ring.add(dc::move(v)));
    const s32* out = ring.remove();
    ASSERT_NE(out, nullptr);
    ASSERT_EQ(*out, i);
  }

  ASSERT_TRUE(ring.isEmpty());
}

DTEST(spscRingPartialFillWraparound) {
  // Add 3, remove 3, add 4 (crossing boundary), verify FIFO order.
  dc::SpscRing<s32> ring(4);

  for (s32 i = 0; i < 3; ++i) {
    s32 v = i;
    ring.add(dc::move(v));
  }
  for (s32 i = 0; i < 3; ++i) {
    ring.remove();
  }

  // Now write/read indices are at 3; next adds will wrap around.
  for (s32 i = 10; i < 14; ++i) {
    s32 v = i;
    ASSERT_TRUE(ring.add(dc::move(v)));
  }
  ASSERT_TRUE(ring.isFull());

  for (s32 i = 10; i < 14; ++i) {
    const s32* out = ring.remove();
    ASSERT_NE(out, nullptr);
    ASSERT_EQ(*out, i);
  }
  ASSERT_TRUE(ring.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Non-trivial element type
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingStringElementType) {
  dc::SpscRing<dc::String> ring(4);

  dc::String s1("hello");
  dc::String s2("world");
  ring.add(dc::move(s1));
  ring.add(dc::move(s2));

  ASSERT_EQ(ring.size(), 2u);

  const dc::String* first = ring.remove();
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(first->toView(), "hello");

  const dc::String* second = ring.remove();
  ASSERT_NE(second, nullptr);
  ASSERT_EQ(second->toView(), "world");

  ASSERT_TRUE(ring.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread safety: single-producer / single-consumer
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(spscRingThreadedProducerConsumer) {
  constexpr s32 kItemCount = 1024;
  dc::SpscRing<s32> ring(static_cast<u32>(kItemCount));
  std::atomic<s32> consumed{0};

  std::thread consumer([&ring, &consumed] {
    s32 expected = 0;
    while (expected < kItemCount) {
      const s32* item = ring.remove();
      if (item) {
        // Items must arrive in order.
        if (*item != expected) return;
        ++expected;
        consumed.fetch_add(1, std::memory_order_release);
      }
    }
  });

  std::thread producer([&ring] {
    for (s32 i = 0; i < kItemCount; ++i) {
      s32 v = i;
      // Spin until add succeeds (ring may be full if consumer is slow).
      while (!ring.add(dc::move(v))) {
      }
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(consumed.load(std::memory_order_acquire), kItemCount);
}

DTEST(spscRingThreadedSmallRingHighContention) {
  // Use a tiny ring (4 slots) to force many full/empty transitions.
  constexpr s32 kItemCount = 512;
  dc::SpscRing<s32> ring(4);
  std::atomic<s32> consumed{0};

  std::thread consumer([&ring, &consumed] {
    s32 expected = 0;
    while (expected < kItemCount) {
      const s32* item = ring.remove();
      if (item) {
        if (*item != expected) return;
        ++expected;
        consumed.fetch_add(1, std::memory_order_release);
      }
    }
  });

  std::thread producer([&ring] {
    for (s32 i = 0; i < kItemCount; ++i) {
      s32 v = i;
      while (!ring.add(dc::move(v))) {
      }
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(consumed.load(std::memory_order_acquire), kItemCount);
}
