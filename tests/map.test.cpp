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
#include <dc/map.hpp>
#include <dc/string.hpp>

using namespace dc;
using namespace dtest;

// ========================================================================== //
// Basic Operations
// ========================================================================== //

DTEST(mapBasicInsertAndGet) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  u64* val = map.insert(42);
  ASSERT_TRUE(val != nullptr);
  *val = 100;

  auto* entry = map.tryGet(42);
  ASSERT_TRUE(entry != nullptr);
  ASSERT_EQ(entry->key, 42);
  ASSERT_EQ(entry->value, 100);
  ASSERT_EQ(map.getSize(), 1);
}

DTEST(mapInsertMultiple) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  for (u64 i = 0; i < 10; ++i) {
    u64* val = map.insert(i);
    ASSERT_TRUE(val != nullptr);
    *val = i * 10;
  }

  ASSERT_EQ(map.getSize(), 10);

  for (u64 i = 0; i < 10; ++i) {
    auto* entry = map.tryGet(i);
    ASSERT_TRUE(entry != nullptr);
    ASSERT_EQ(entry->value, i * 10);
  }
}

DTEST(mapGetNonExistent) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  auto* entry = map.tryGet(999);
  ASSERT_TRUE(entry == nullptr);
}

// ========================================================================== //
// Operator[] Access
// ========================================================================== //

DTEST(mapOperatorBracket) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[1] = 10;
  map[2] = 20;
  map[3] = 30;

  ASSERT_EQ(map[1], 10);
  ASSERT_EQ(map[2], 20);
  ASSERT_EQ(map[3], 30);
  ASSERT_EQ(map.getSize(), 3);
}

DTEST(mapOperatorBracketAutoInsert) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  // Access non-existent key should auto-insert with default value
  u64& val = map[42];
  val = 100;

  ASSERT_EQ(map[42], 100);
  ASSERT_EQ(map.getSize(), 1);
}

// ========================================================================== //
// Remove Operations
// ========================================================================== //

DTEST(mapRemove) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[1] = 10;
  map[2] = 20;
  map[3] = 30;

  ASSERT_TRUE(map.remove(2));
  ASSERT_EQ(map.getSize(), 2);

  ASSERT_TRUE(map.tryGet(2) == nullptr);
  ASSERT_EQ(map[1], 10);
  ASSERT_EQ(map[3], 30);
}

DTEST(mapRemoveNonExistent) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[1] = 10;

  ASSERT_FALSE(map.remove(999));
  ASSERT_EQ(map.getSize(), 1);
}

DTEST(mapRemoveWithValue) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[42] = 100;

  u64 removedValue = 0;
  ASSERT_TRUE(map.remove(42, &removedValue));
  ASSERT_EQ(removedValue, 100);
  ASSERT_EQ(map.getSize(), 0);
}

// ========================================================================== //
// Iteration
// ========================================================================== //

DTEST(mapIteration) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[1] = 10;
  map[2] = 20;
  map[3] = 30;

  u64 sum = 0;
  u64 count = 0;
  for (const auto& entry : map) {
    sum += entry.value;
    ++count;
  }

  ASSERT_EQ(count, 3);
  ASSERT_EQ(sum, 60);
}

DTEST(mapEmptyIteration) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  u64 count = 0;
  for (const auto& entry : map) {
    DC_UNUSED(entry);
    ++count;
  }

  ASSERT_EQ(count, 0);
}

// ========================================================================== //
// Collision Handling & Resizing
// ========================================================================== //

DTEST(mapCollisionHandling) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  // Insert many entries to trigger collisions and resizing
  for (u64 i = 0; i < 1000; ++i) {
    map[i] = i * 2;
  }

  ASSERT_EQ(map.getSize(), 1000);

  for (u64 i = 0; i < 1000; ++i) {
    auto* entry = map.tryGet(i);
    ASSERT_TRUE(entry != nullptr);
    ASSERT_EQ(entry->value, i * 2);
  }
}

DTEST(mapResize) {
  Map<u64, u64> map(4, 0.75f, TEST_ALLOCATOR);

  ASSERT_EQ(map.getCapacity(), 4);

  // Insert enough elements to trigger resize
  for (u64 i = 0; i < 10; ++i) {
    map[i] = i;
  }

  ASSERT_TRUE(map.getCapacity() > 4);
  ASSERT_EQ(map.getSize(), 10);
}

DTEST(mapReserve) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  const u64 initialCapacity = map.getCapacity();
  map.reserve(100);

  ASSERT_TRUE(map.getCapacity() >= 100);
  ASSERT_TRUE(map.getCapacity() > initialCapacity);
}

// ========================================================================== //
// Clear
// ========================================================================== //

DTEST(mapClear) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[1] = 10;
  map[2] = 20;
  map[3] = 30;

  ASSERT_EQ(map.getSize(), 3);

  map.clear();

  ASSERT_EQ(map.getSize(), 0);
  ASSERT_TRUE(map.tryGet(1) == nullptr);
  ASSERT_TRUE(map.tryGet(2) == nullptr);
  ASSERT_TRUE(map.tryGet(3) == nullptr);
}

// ========================================================================== //
// Move Semantics
// ========================================================================== //

DTEST(mapMoveConstructor) {
  Map<u64, u64> map1(TEST_ALLOCATOR);
  map1[1] = 10;
  map1[2] = 20;

  Map<u64, u64> map2 = dc::move(map1);

  ASSERT_EQ(map2.getSize(), 2);
  ASSERT_EQ(map2[1], 10);
  ASSERT_EQ(map2[2], 20);
}

DTEST(mapMoveAssignment) {
  Map<u64, u64> map1(TEST_ALLOCATOR);
  map1[1] = 10;

  Map<u64, u64> map2(TEST_ALLOCATOR);
  map2[99] = 999;

  map2 = dc::move(map1);

  ASSERT_EQ(map2.getSize(), 1);
  ASSERT_EQ(map2[1], 10);
}

// ========================================================================== //
// Clone
// ========================================================================== //

DTEST(mapClone) {
  Map<u64, u64> map1(TEST_ALLOCATOR);
  map1[1] = 10;
  map1[2] = 20;
  map1[3] = 30;

  auto map2 = map1.clone();

  ASSERT_EQ(map2.getSize(), 3);
  ASSERT_EQ(map2[1], 10);
  ASSERT_EQ(map2[2], 20);
  ASSERT_EQ(map2[3], 30);

  // Verify they're independent
  map2[1] = 999;
  ASSERT_EQ(map1[1], 10);
  ASSERT_EQ(map2[1], 999);
}

// ========================================================================== //
// String Keys
// ========================================================================== //

DTEST(mapWithStringKeys) {
  Map<String, u64> map(TEST_ALLOCATOR);

  String key1("hello", TEST_ALLOCATOR);
  String key2("world", TEST_ALLOCATOR);
  String key1Lookup("hello", TEST_ALLOCATOR);
  String key2Lookup("world", TEST_ALLOCATOR);

  u64* val1 = map.insert(dc::move(key1));
  *val1 = 42;

  u64* val2 = map.insert(dc::move(key2));
  *val2 = 100;

  ASSERT_EQ(map.getSize(), 2);

  auto* entry1 = map.tryGet(key1Lookup);
  ASSERT_TRUE(entry1 != nullptr);
  ASSERT_EQ(entry1->value, 42);

  auto* entry2 = map.tryGet(key2Lookup);
  ASSERT_TRUE(entry2 != nullptr);
  ASSERT_EQ(entry2->value, 100);
}

DTEST(mapWithStringViewKeys) {
  Map<StringView, u64> map(TEST_ALLOCATOR);

  StringView key1("hello");
  StringView key2("world");

  u64* val1 = map.insert(key1);
  *val1 = 42;

  u64* val2 = map.insert(key2);
  *val2 = 100;

  ASSERT_EQ(map.getSize(), 2);

  auto* entry1 = map.tryGet(key1);
  ASSERT_TRUE(entry1 != nullptr);
  ASSERT_EQ(entry1->value, 42);
}

// ========================================================================== //
// Edge Cases
// ========================================================================== //

DTEST(mapEmptyState) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  ASSERT_EQ(map.getSize(), 0);
  ASSERT_TRUE(map.isEmpty());
  ASSERT_TRUE(map.getCapacity() > 0);
}

DTEST(mapSingleElement) {
  Map<u64, u64> map(TEST_ALLOCATOR);

  map[42] = 100;

  ASSERT_EQ(map.getSize(), 1);
  ASSERT_FALSE(map.isEmpty());

  ASSERT_TRUE(map.remove(42));
  ASSERT_TRUE(map.isEmpty());
}

DTEST(mapRobinHoodSwapping) {
  Map<u64, u64> map(8, 0.75f, TEST_ALLOCATOR);

  // Insert keys that will likely collide
  // This tests the robin hood swapping logic
  for (u64 i = 0; i < 20; ++i) {
    map[i * 8] = i;  // Keys that are multiples of capacity
  }

  // Verify all entries can still be found
  for (u64 i = 0; i < 20; ++i) {
    auto* entry = map.tryGet(i * 8);
    ASSERT_TRUE(entry != nullptr);
    ASSERT_EQ(entry->value, i);
  }
}
