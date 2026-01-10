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

#include <dc/dtest.hpp>
#include <dc/ring.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: Constructor
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringConstructor) {
  dc::Ring<s32> ring;
  ASSERT_EQ(ring.capacity, 0u);
  ASSERT_EQ(ring.read, 0u);
  ASSERT_EQ(ring.write, 0u);
  ASSERT_EQ(ring.data, nullptr);
  ASSERT_TRUE(ring.isEmpty());
  ASSERT_EQ(ring.size(), 0u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: reserve
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringReserve) {
  dc::Ring<s32> ring;
  const bool success = ring.reserve(4);
  ASSERT_TRUE(success);
  ASSERT_EQ(ring.capacity, 4u);
  ASSERT_NE(ring.data, nullptr);
  ASSERT_TRUE(ring.isEmpty());
}

DTEST(ringReserveRoundsUpToPowerOfTwo) {
  dc::Ring<s32> ring;
  ring.reserve(5);
  ASSERT_EQ(ring.capacity, 8u);

  dc::Ring<s32> ring2;
  ring2.reserve(7);
  ASSERT_EQ(ring2.capacity, 8u);

  dc::Ring<s32> ring3;
  ring3.reserve(16);
  ASSERT_EQ(ring3.capacity, 16u);
}

DTEST(ringReserveDoesNotShrink) {
  dc::Ring<s32> ring;
  ring.reserve(8);
  ASSERT_EQ(ring.capacity, 8u);

  ring.reserve(4);
  ASSERT_EQ(ring.capacity, 8u);
}

DTEST(ringReservePreservesData) {
  dc::Ring<s32> ring;
  ring.reserve(4);
  s32 v1 = 10;
  s32 v2 = 20;
  s32 v3 = 30;
  ring.add(dc::move(v1));
  ring.add(dc::move(v2));
  ring.add(dc::move(v3));
  ASSERT_EQ(ring.size(), 3u);

  ring.reserve(8);
  ASSERT_EQ(ring.capacity, 8u);
  ASSERT_EQ(ring.size(), 3u);

  const s32* first = ring.remove();
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(*first, 10);

  const s32* second = ring.remove();
  ASSERT_NE(second, nullptr);
  ASSERT_EQ(*second, 20);

  const s32* third = ring.remove();
  ASSERT_NE(third, nullptr);
  ASSERT_EQ(*third, 30);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: add (with value)
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringAddValue) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32 value = 42;
  const bool success = ring.add(dc::move(value));
  ASSERT_TRUE(success);
  ASSERT_EQ(ring.size(), 1u);
  ASSERT_FALSE(ring.isEmpty());
}

DTEST(ringAddValueFull) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32 v1 = 1;
  s32 v2 = 2;
  s32 v3 = 3;
  s32 v4 = 4;
  ASSERT_TRUE(ring.add(dc::move(v1)));
  ASSERT_TRUE(ring.add(dc::move(v2)));
  ASSERT_TRUE(ring.add(dc::move(v3)));
  ASSERT_TRUE(ring.add(dc::move(v4)));
  ASSERT_TRUE(ring.isFull());

  s32 v5 = 5;
  const bool failedAdd = ring.add(dc::move(v5));
  ASSERT_FALSE(failedAdd);
  ASSERT_EQ(ring.size(), 4u);
}

DTEST(ringAddMultipleValues) {
  dc::Ring<s32> ring;
  ring.reserve(8);

  for (s32 i = 0; i < 5; ++i) {
    s32 value = i * 10;
    ASSERT_TRUE(ring.add(dc::move(value)));
  }

  ASSERT_EQ(ring.size(), 5u);
  ASSERT_FALSE(ring.isFull());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: add (returns pointer)
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringAddPointer) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32* ptr = ring.add();
  ASSERT_NE(ptr, nullptr);
  *ptr = 100;

  ASSERT_EQ(ring.size(), 1u);
}

DTEST(ringAddPointerFull) {
  dc::Ring<s32> ring;
  ring.reserve(2);

  s32* ptr1 = ring.add();
  ASSERT_NE(ptr1, nullptr);
  *ptr1 = 10;

  s32* ptr2 = ring.add();
  ASSERT_NE(ptr2, nullptr);
  *ptr2 = 20;

  ASSERT_TRUE(ring.isFull());

  s32* ptr3 = ring.add();
  ASSERT_EQ(ptr3, nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: remove
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringRemove) {
  dc::Ring<s32> ring;
  ring.reserve(4);
  s32 v = 42;
  ring.add(dc::move(v));

  const s32* value = ring.remove();
  ASSERT_NE(value, nullptr);
  ASSERT_EQ(*value, 42);
  ASSERT_TRUE(ring.isEmpty());
}

DTEST(ringRemoveEmpty) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  const s32* value = ring.remove();
  ASSERT_EQ(value, nullptr);
}

DTEST(ringRemoveMultiple) {
  dc::Ring<s32> ring;
  ring.reserve(8);

  s32 v1 = 10;
  s32 v2 = 20;
  s32 v3 = 30;
  ring.add(dc::move(v1));
  ring.add(dc::move(v2));
  ring.add(dc::move(v3));

  const s32* first = ring.remove();
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(*first, 10);

  const s32* second = ring.remove();
  ASSERT_NE(second, nullptr);
  ASSERT_EQ(*second, 20);

  const s32* third = ring.remove();
  ASSERT_NE(third, nullptr);
  ASSERT_EQ(*third, 30);

  ASSERT_TRUE(ring.isEmpty());
}

DTEST(ringRemoveFIFO) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  for (s32 i = 0; i < 3; ++i) {
    s32 value = i;
    ring.add(dc::move(value));
  }

  for (s32 i = 0; i < 3; ++i) {
    const s32* value = ring.remove();
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(*value, i);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: size
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringSize) {
  dc::Ring<s32> ring;
  ring.reserve(8);

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: isEmpty
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringIsEmpty) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  ASSERT_TRUE(ring.isEmpty());

  s32 v = 42;
  ring.add(dc::move(v));
  ASSERT_FALSE(ring.isEmpty());

  ring.remove();
  ASSERT_TRUE(ring.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: isFull
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringIsFull) {
  dc::Ring<s32> ring;
  ring.reserve(2);

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
// Test: begin and end (iteration)
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringBeginEnd) {
  dc::Ring<s32> ring;
  ring.reserve(8);

  s32 v1 = 10;
  s32 v2 = 20;
  s32 v3 = 30;
  ring.add(dc::move(v1));
  ring.add(dc::move(v2));
  ring.add(dc::move(v3));

  s32 sum = 0;
  s32 count = 0;
  for (s32 elem : ring) {
    sum += elem;
    ++count;
  }

  ASSERT_EQ(sum, 60);
  ASSERT_EQ(count, 3);
}

DTEST(ringIterationEmpty) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32 count = 0;
  for (s32 _ : ring) {
    ++count;
  }

  ASSERT_EQ(count, 0);
}

DTEST(ringIterationModify) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32 v1 = 1;
  s32 v2 = 2;
  s32 v3 = 3;
  ring.add(dc::move(v1));
  ring.add(dc::move(v2));
  ring.add(dc::move(v3));

  for (s32& elem : ring) {
    elem *= 10;
  }

  const s32* first = ring.remove();
  ASSERT_NE(first, nullptr);
  ASSERT_EQ(*first, 10);

  const s32* second = ring.remove();
  ASSERT_NE(second, nullptr);
  ASSERT_EQ(*second, 20);

  const s32* third = ring.remove();
  ASSERT_NE(third, nullptr);
  ASSERT_EQ(*third, 30);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test: Wraparound behavior
////////////////////////////////////////////////////////////////////////////////////////////////////

DTEST(ringWraparound) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  s32 v1 = 1;
  s32 v2 = 2;
  ring.add(dc::move(v1));
  ring.add(dc::move(v2));
  ring.remove();
  ring.remove();

  s32 v3 = 3;
  s32 v4 = 4;
  s32 v5 = 5;
  s32 v6 = 6;
  ring.add(dc::move(v3));
  ring.add(dc::move(v4));
  ring.add(dc::move(v5));
  ring.add(dc::move(v6));

  ASSERT_TRUE(ring.isFull());
  ASSERT_EQ(ring.size(), 4u);

  const s32* first = ring.remove();
  ASSERT_EQ(*first, 3);
}

DTEST(ringContinuousAddRemove) {
  dc::Ring<s32> ring;
  ring.reserve(4);

  for (s32 i = 0; i < 100; ++i) {
    s32 value = i;
    ring.add(dc::move(value));
    const s32* removed = ring.remove();
    ASSERT_NE(removed, nullptr);
    ASSERT_EQ(*removed, i);
  }

  ASSERT_TRUE(ring.isEmpty());
}

DTEST(forRangeIteratorWraparound) {
  dc::Ring<s32> ring;
  ring.reserve(2);

  ring.add(1);
  LOG_INFO("size: {}", ring.size());
  ring.remove();
  LOG_INFO("size: {}", ring.size());

  ring.add(2);
  LOG_INFO("size: {}", ring.size());
  ring.add(3);

  LOG_INFO("size: {}", ring.size());

  dc::List<s32> list;
  for (const s32 elem : ring) {
	list.add(elem);
	LOG_INFO("elem: {}", elem);
  }

  ASSERT_EQ(list.getSize(), 2);
  ASSERT_EQ(list[0], 2);
  ASSERT_EQ(list[1], 3);
}
  
