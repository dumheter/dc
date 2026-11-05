#include <dc/dtest.hpp>
#include <dc/list.hpp>

using namespace dc;
using namespace dtest;

DTEST(canAdd) {
  List<int> v(TEST_ALLOCATOR);

  v.add(1337);

  ASSERT_EQ(v.getSize(), 1);
  ASSERT_TRUE(v.getCapacity() > 0);
  ASSERT_EQ(v[0], 1337);
}

DTEST(emptyDefaultListIsEmpty) {
  List<int> v(TEST_ALLOCATOR);
  ASSERT_EQ(v.getSize(), 0);
}

DTEST(growWhenOOM) {
  List<float> v(1, TEST_ALLOCATOR);

  v.add(10);
  ASSERT_EQ(v.getSize(), 1);

  v.add(20);
  ASSERT_EQ(v.getSize(), 2);
}

DTEST(removeByPosition) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int>> list(TEST_ALLOCATOR);

  list.add(10);
  list.add(20);
  list.add(30);

  const int destructsBefore = stats.destructs;
  list.removeAt(1);

  ASSERT_EQ(list.getSize(), 2);
  ASSERT_EQ(list[1], 30);
  // The removed element, and the moved-from slot of the one shifted down.
  ASSERT_EQ(stats.destructs, destructsBefore + 2);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(removeByIterator) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int*>> list(TEST_ALLOCATOR);

  int abc[3];
  abc[0] = 10;
  abc[1] = 20;
  abc[2] = 30;

  list.add(&abc[0]);
  list.add(&abc[1]);
  list.add(&abc[2]);

  const int destructsBefore = stats.destructs;
  list.remove(&list[1]);
  list.remove(&list[1]);
  list.remove(&list[0]);

  ASSERT_EQ(list.getSize(), 0);
  ASSERT_EQ(stats.copies, 0);
  // ASSERT_EQ(stats.moves, 0);
  // The 3 removed elements, and the moved-from slot of the 1 shifted down.
  ASSERT_EQ(stats.destructs, destructsBefore + 4);
}

DTEST(removeByReference) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int*>> list(TEST_ALLOCATOR);

  int abc[3];
  abc[0] = 10;
  abc[1] = 20;
  abc[2] = 30;

  list.add(&abc[0]);
  list.add(&abc[1]);
  list.add(&abc[2]);

  const int destructsBefore = stats.destructs;
  list.remove(&abc[1]);
  list.remove(&abc[0]);
  list.remove(&abc[2]);

  // double remove is noop
  const LifetimeTracker<int*> lookup(&abc[1]);
  list.remove(lookup);

  ASSERT_EQ(list.getSize(), 0);
  ASSERT_EQ(stats.copies, 0);
  // ASSERT_EQ(stats.moves, 0);
  // The 3 temporaries the lookups convert to, and the 3 removed elements.
  ASSERT_EQ(stats.destructs, destructsBefore + 6);
}

DTEST(removeIfHappyPath) {
  List<int> list(TEST_ALLOCATOR);

  list.add(1);
  list.add(2);
  list.add(1);
  list.add(4);

  list.add(2);
  list.add(3);
  list.add(2);
  list.add(1);

  list.removeIf([](const int& i) -> bool { return i == 2; });

  ASSERT_EQ(list.getSize(), 5);

  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 1);
  ASSERT_EQ(list[2], 4);
  ASSERT_EQ(list[3], 3);

  ASSERT_EQ(list[4], 1);
}

DTEST(removeIfRemovesNothing) {
  List<int> list(TEST_ALLOCATOR);

  list.add(1);
  list.add(2);
  list.add(1);
  list.add(4);

  list.add(2);
  list.add(3);
  list.add(2);
  list.add(1);

  list.removeIf([](const int& i) -> bool { return i == 1337; });

  ASSERT_EQ(list.getSize(), 8);

  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 2);
  ASSERT_EQ(list[2], 1);
  ASSERT_EQ(list[3], 4);

  ASSERT_EQ(list[4], 2);
  ASSERT_EQ(list[5], 3);
  ASSERT_EQ(list[6], 2);
  ASSERT_EQ(list[7], 1);
}

DTEST(removeIfWithLargeDataSet) {
  List<int> list(TEST_ALLOCATOR);

  for (int i = 0; i < 10000; ++i) {
    list.add(i % 10);
  }

  list.removeIf([](const int& i) -> bool { return i == 0; });

  ASSERT_EQ(list.getSize(), 9000);

  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 2);
  ASSERT_EQ(list[2], 3);
  ASSERT_EQ(list[3], 4);

  ASSERT_EQ(list[4], 5);
  ASSERT_EQ(list[5], 6);
  ASSERT_EQ(list[6], 7);
  ASSERT_EQ(list[7], 8);

  ASSERT_EQ(list[8], 9);
  ASSERT_EQ(list[9], 1);
  ASSERT_EQ(list[10], 2);
  ASSERT_EQ(list[11], 3);
}

DTEST(find) {
  List<int> v(TEST_ALLOCATOR);

  v.add(10);
  v.add(11);
  v.add(12);
  v.add(13);
  v.add(14);

  int* iter = v.find(12);
  ASSERT_TRUE(iter != v.end());
  ASSERT_TRUE(*iter == 12);

  v.remove(iter);

  iter = v.find(12);
  ASSERT_TRUE(iter == v.end());
}

DTEST(copyConstructor) {
  List<int> v(TEST_ALLOCATOR);

  v.add(10);
  v.add(20);
  v.add(30);

  List<int> copy(v);
  ASSERT_EQ(copy[0], 10);
  ASSERT_EQ(copy[1], 20);
  ASSERT_EQ(copy[2], 30);
  ASSERT_EQ(copy.getSize(), v.getSize());

  // Modify original, copy should be unchanged
  v[0] = 100;
  ASSERT_EQ(copy[0], 10);
  ASSERT_EQ(v[0], 100);
}

DTEST(copyAssignment) {
  List<int> v(TEST_ALLOCATOR);

  v.add(10);
  v.add(20);
  v.add(30);

  List<int> copy(TEST_ALLOCATOR);
  copy.add(99);

  copy = v;
  ASSERT_EQ(copy[0], 10);
  ASSERT_EQ(copy[1], 20);
  ASSERT_EQ(copy[2], 30);
  ASSERT_EQ(copy.getSize(), v.getSize());

  // Modify original, copy should be unchanged
  v[0] = 100;
  ASSERT_EQ(copy[0], 10);
  ASSERT_EQ(v[0], 100);
}

DTEST(selfCopyAssignment) {
  List<int> v(TEST_ALLOCATOR);
  v.add(10);
  v.add(20);

  List<int>& ref = v;
  v = ref;
  ASSERT_EQ(v[0], 10);
  ASSERT_EQ(v[1], 20);
  ASSERT_EQ(v.getSize(), 2);
}

DTEST(constIterator) {
  List<int> v(TEST_ALLOCATOR);

  v.add(1);
  v.add(2);
  v.add(4);

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 7);
}

DTEST(mutableIterator) {
  List<int> v(TEST_ALLOCATOR);

  v.add(1);
  v.add(2);
  v.add(4);

  for (int& i : v) i += 1;

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 10);
}

DTEST(addRangeForTrivialElementType) {
  List<char8> v(TEST_ALLOCATOR);

  const char8* str = " world";

  v.add('h');
  v.add('e');
  v.add('l');
  v.add('l');
  v.add('o');

  v.addRange(str, str + strlen(str) + 1);

  ASSERT_EQ(v.getSize(), strlen("hello world") + 1);  // +1 for null terminator
  ASSERT_EQ(v.getLast(), 0);
  ASSERT_TRUE(strcmp(v.begin(), "hello world") == 0);
}

DTEST(addRangeForNonTrivialElementType) {
  struct A {
    int a;
    bool operator==(int other) const { return a == other; }
  };

  List<A> l0(TEST_ALLOCATOR);
  l0.add(A{20});
  l0.add(A{21});
  l0.add(A{22});

  List<A> l1(TEST_ALLOCATOR);
  l1.add(A{18});
  l1.add(A{19});

  l1.addRange(l0.begin(), l0.end());

  ASSERT_EQ(l1.getSize(), 5);
  ASSERT_EQ(l1[0], 18);
  ASSERT_EQ(l1[1], 19);
  ASSERT_EQ(l1[2], 20);
  ASSERT_EQ(l1[3], 21);
  ASSERT_EQ(l1[4], 22);
}

///////////////////////////////////////////////////////////////////////////////
// No Allocation
//

DTEST(staticList) {
  struct NoAllocator final : public IAllocator {
    virtual void* alloc(usize, usize) override { return nullptr; }
    virtual void* realloc(void*, usize, usize) override { return nullptr; }
    virtual void free(void*) override {}
  } noAlloc;

  List<int, 4> list(noAlloc);

  list.add(1);
  list.add(2);
  list.add(4);
  list.add(8);

  ASSERT_EQ(4, list.getSize());
  ASSERT_EQ(8, list.getLast());

  // can't allocate, so this add will fail
  list.add(16);
  ASSERT_EQ(4, list.getSize());
  ASSERT_EQ(8, list.getLast());
}

///////////////////////////////////////////////////////////////////////////////
// Trivially Relocatable Optimization
//

DTEST(reserveNonTrivialStillMoves) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  List<LifetimeTracker<int>, 2> list(TEST_ALLOCATOR);
  list.add(1);
  list.add(2);

  const int movesBefore = stats.moves;
  list.reserve(100);

  // Should have moved 2 elements
  ASSERT_EQ(stats.moves, movesBefore + 2);
  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 2);
  ASSERT_EQ(list.getCapacity(), 100);
}

DTEST(reserveTrivialDoesNotMove) {
  // int is trivially relocatable, so reserve should use realloc/memcpy
  List<int, 2> list(TEST_ALLOCATOR);
  list.add(1);
  list.add(2);

  // Force reallocation by exceeding internal buffer
  list.reserve(100);

  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 2);
  ASSERT_EQ(list.getCapacity(), 100);

  // Add more to verify the data is correct after realloc
  list.add(3);
  list.add(4);
  ASSERT_EQ(list[2], 3);
  ASSERT_EQ(list[3], 4);
}

DTEST(reserveTrivialFromInternalBuffer) {
  // Test moving from internal buffer to heap for trivially relocatable type
  List<int, 4> list(TEST_ALLOCATOR);
  list.add(10);
  list.add(20);
  list.add(30);
  list.add(40);

  // Should use memcpy from internal buffer to newly allocated memory
  list.reserve(100);

  ASSERT_EQ(list.getSize(), 4);
  ASSERT_EQ(list[0], 10);
  ASSERT_EQ(list[1], 20);
  ASSERT_EQ(list[2], 30);
  ASSERT_EQ(list[3], 40);
}

DTEST(reserveTrivialReallocOnHeap) {
  // Test realloc path when already on heap
  List<int, 1> list(TEST_ALLOCATOR);
  list.add(1);

  // First reserve moves from internal buffer to heap
  list.reserve(10);
  ASSERT_EQ(list[0], 1);

  // Second reserve should use realloc (already on heap)
  list.reserve(100);
  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list.getCapacity(), 100);
}

DTEST(removeTrivialUsesMemmove) {
  List<int> list(TEST_ALLOCATOR);
  list.add(1);
  list.add(2);
  list.add(3);
  list.add(4);
  list.add(5);

  list.removeAt(1);  // Remove '2'

  ASSERT_EQ(list.getSize(), 4);
  ASSERT_EQ(list[0], 1);
  ASSERT_EQ(list[1], 3);
  ASSERT_EQ(list[2], 4);
  ASSERT_EQ(list[3], 5);

  list.removeAt(0);  // Remove '1'
  ASSERT_EQ(list.getSize(), 3);
  ASSERT_EQ(list[0], 3);
  ASSERT_EQ(list[1], 4);
  ASSERT_EQ(list[2], 5);

  list.removeAt(2);  // Remove '5' (last element)
  ASSERT_EQ(list.getSize(), 2);
  ASSERT_EQ(list[0], 3);
  ASSERT_EQ(list[1], 4);
}

DTEST(clearTrivialSkipsDestructors) {
  // For trivially relocatable types, clear should just reset m_end
  List<int> list(TEST_ALLOCATOR);
  list.add(1);
  list.add(2);
  list.add(3);

  list.clear();

  ASSERT_EQ(list.getSize(), 0);
  ASSERT_TRUE(list.isEmpty());

  // Can add again after clear
  list.add(10);
  ASSERT_EQ(list.getSize(), 1);
  ASSERT_EQ(list[0], 10);
}

DTEST(clearNonTrivialCallsDestructors) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  {
    List<LifetimeTracker<int>> list(TEST_ALLOCATOR);
    list.add(1);
    list.add(2);
    list.add(3);

    const int destructsBefore = stats.destructs;
    list.clear();

    // Should have called 3 destructors
    ASSERT_EQ(stats.destructs, destructsBefore + 3);
    ASSERT_EQ(list.getSize(), 0);
  }
}

DTEST(moveConstructTrivialUsesMemcpy) {
  // When moving from internal buffer, trivially relocatable types use memcpy
  List<int, 4> list1(TEST_ALLOCATOR);
  list1.add(100);
  list1.add(200);
  list1.add(300);

  List<int, 4> list2(dc::move(list1));

  ASSERT_EQ(list2.getSize(), 3);
  ASSERT_EQ(list2[0], 100);
  ASSERT_EQ(list2[1], 200);
  ASSERT_EQ(list2[2], 300);
}

DTEST(moveAssignTrivialUsesMemcpy) {
  List<int, 4> list1(TEST_ALLOCATOR);
  list1.add(10);
  list1.add(20);

  List<int, 4> list2(TEST_ALLOCATOR);
  list2.add(99);

  list2 = dc::move(list1);

  ASSERT_EQ(list2.getSize(), 2);
  ASSERT_EQ(list2[0], 10);
  ASSERT_EQ(list2[1], 20);
}

DTEST(customTriviallyRelocatableType) {
  // Test a custom type marked as trivially relocatable
  struct TrivialStruct {
    using IsTriviallyRelocatable = bool;
    int x;
    int y;

    bool operator==(int other) const { return x == other; }
  };

  static_assert(isTriviallyRelocatable<TrivialStruct>);

  List<TrivialStruct, 2> list(TEST_ALLOCATOR);
  list.add(TrivialStruct{1, 2});
  list.add(TrivialStruct{3, 4});

  // Force reallocation
  list.reserve(100);

  ASSERT_EQ(list[0].x, 1);
  ASSERT_EQ(list[0].y, 2);
  ASSERT_EQ(list[1].x, 3);
  ASSERT_EQ(list[1].y, 4);
}

DTEST(moveAssignmentDoesNotUseDestroyedObject) {
  List<int, 4> list1(TEST_ALLOCATOR);
  list1.add(10);
  list1.add(20);
  list1.add(30);

  List<int, 4> list2(TEST_ALLOCATOR);
  list2.add(99);
  list2.add(88);

  list2 = dc::move(list1);

  ASSERT_EQ(list2.getSize(), 3);
  ASSERT_EQ(list2[0], 10);
  ASSERT_EQ(list2[1], 20);
  ASSERT_EQ(list2[2], 30);
}

DTEST(moveAssignmentHeapToHeap) {
  List<int, 2> list1(TEST_ALLOCATOR);
  for (int i = 0; i < 100; ++i) list1.add(i);

  List<int, 2> list2(TEST_ALLOCATOR);
  for (int i = 0; i < 50; ++i) list2.add(i * 10);

  list2 = dc::move(list1);

  ASSERT_EQ(list2.getSize(), 100);
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(list2[static_cast<u64>(i)], i);
  }
}

DTEST(copyAssignmentDoesNotUseDestroyedObject) {
  List<int, 4> list1(TEST_ALLOCATOR);
  list1.add(10);
  list1.add(20);
  list1.add(30);

  List<int, 4> list2(TEST_ALLOCATOR);
  list2.add(99);
  list2.add(88);

  list2 = list1;

  ASSERT_EQ(list2.getSize(), 3);
  ASSERT_EQ(list2[0], 10);
  ASSERT_EQ(list2[1], 20);
  ASSERT_EQ(list2[2], 30);
}

DTEST(copyAssignmentHeapToHeap) {
  List<int, 2> list1(TEST_ALLOCATOR);
  for (int i = 0; i < 100; ++i) list1.add(i);

  List<int, 2> list2(TEST_ALLOCATOR);
  for (int i = 0; i < 50; ++i) list2.add(i * 10);

  list2 = list1;

  ASSERT_EQ(list2.getSize(), 100);
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(list2[static_cast<u64>(i)], i);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Element lifetimes in the internal buffer
//

namespace {

/// Counts how many times each value is destroyed, in a destructor that writes
/// nothing to the object itself, so that the count holds in optimized builds
/// too. A moved-from probe holds no value and is not counted.
struct ValueProbe {
  static constexpr u64 kValues = 8;
  static inline s32 destroyed[kValues];
  static inline s32 defaultConstructed;

  static void reset() {
    for (s32& count : destroyed) count = 0;
    defaultConstructed = 0;
  }

  ValueProbe() { ++defaultConstructed; }
  explicit ValueProbe(u64 v) : value(v) {}
  ValueProbe(const ValueProbe& other) = default;
  ValueProbe& operator=(const ValueProbe& other) = default;
  ValueProbe(ValueProbe&& other) noexcept : value(other.value) {
    other.value = 0;
  }
  ValueProbe& operator=(ValueProbe&& other) noexcept {
    if (&other != this) {
      release();
      value = other.value;
      other.value = 0;
    }
    return *this;
  }
  ~ValueProbe() { release(); }

  void release() {
    if (value != 0) ++destroyed[value];
  }

  [[nodiscard]] bool operator==(const ValueProbe& other) const {
    return value == other.value;
  }

  u64 value = 0;
};

/// Long enough for the string to own heap memory rather than its small buffer.
constexpr const char8* kHeapString =
    "alsa_output.pci-0000_00_1f.3.analog-stereo";

}  // namespace

DTEST(internalBufferElementDestroyedOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 1> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
}

DTEST(internalBufferIsNotDefaultConstructed) {
  ValueProbe::reset();
  List<ValueProbe, 4> list(TEST_ALLOCATOR);
  ASSERT_EQ(ValueProbe::defaultConstructed, 0);
}

DTEST(singleHeapStringInInternalBuffer) {
  List<String, 1> list;
  list.add(String(kHeapString));
  ASSERT_EQ(list.getSize(), 1);
  ASSERT_TRUE(list[0] == kHeapString);
}

DTEST(listOfListsWithSingleHeapString) {
  List<List<String, 1>, 1> outer;
  List<String, 1> inner;
  inner.add(String(kHeapString));
  outer.add(dc::move(inner));
  ASSERT_TRUE(outer[0][0] == kHeapString);
}

DTEST(moveConstructFromInternalBufferDestroysEachValueOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 2> a(TEST_ALLOCATOR);
    a.add(ValueProbe(1));
    a.add(ValueProbe(2));
    const List<ValueProbe, 2> b(dc::move(a));
    ASSERT_EQ(b[0].value, 1);
    ASSERT_EQ(b[1].value, 2);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
  ASSERT_EQ(ValueProbe::destroyed[2], 1);
}

DTEST(moveAssignFromInternalBufferDestroysEachValueOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 2> a(TEST_ALLOCATOR);
    a.add(ValueProbe(1));
    List<ValueProbe, 2> b(TEST_ALLOCATOR);
    b.add(ValueProbe(2));
    b = dc::move(a);
    ASSERT_EQ(ValueProbe::destroyed[2], 1);
    ASSERT_EQ(b[0].value, 1);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
  ASSERT_EQ(ValueProbe::destroyed[2], 1);
}

DTEST(copyDestroysEachCopyOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 1> a(TEST_ALLOCATOR);
    a.add(ValueProbe(1));
    const List<ValueProbe, 1> b(a);
    ASSERT_EQ(b[0].value, 1);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 2);
}

DTEST(growingOutOfInternalBufferDestroysEachValueOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 1> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
    list.add(ValueProbe(2));
    list.add(ValueProbe(3));
    ASSERT_EQ(list.getSize(), 3);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
  ASSERT_EQ(ValueProbe::destroyed[2], 1);
  ASSERT_EQ(ValueProbe::destroyed[3], 1);
}

DTEST(clearDestroysInternalBufferElementOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 1> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
    list.clear();
    ASSERT_EQ(ValueProbe::destroyed[1], 1);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
}

DTEST(removeAtDestroysInternalBufferElementOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 2> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
    list.add(ValueProbe(2));
    list.removeAt(0);
    ASSERT_EQ(ValueProbe::destroyed[1], 1);
    ASSERT_EQ(list[0].value, 2);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
  ASSERT_EQ(ValueProbe::destroyed[2], 1);
}

DTEST(removeByValueDestroysInternalBufferElementOnce) {
  ValueProbe::reset();
  {
    List<ValueProbe, 2> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
    const ValueProbe lookup(2);
    list.add(ValueProbe(2));
    list.remove(lookup);
    ASSERT_EQ(list.getSize(), 1);
    ASSERT_EQ(ValueProbe::destroyed[2], 1);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
}

DTEST(removeIfKeepsLastElementWhenNothingMatches) {
  List<String> list;
  list.add(String(kHeapString));
  list.add(String(kHeapString));
  list.removeIf([](const String&) -> bool { return false; });
  ASSERT_EQ(list.getSize(), 2);
  ASSERT_TRUE(list[0] == kHeapString);
  ASSERT_TRUE(list[1] == kHeapString);
}

DTEST(removeIfDestroysOnlyRemovedElements) {
  ValueProbe::reset();
  {
    List<ValueProbe, 3> list(TEST_ALLOCATOR);
    list.add(ValueProbe(1));
    list.add(ValueProbe(2));
    list.add(ValueProbe(3));
    list.removeIf([](const ValueProbe& p) -> bool { return p.value == 2; });
    ASSERT_EQ(list.getSize(), 2);
    ASSERT_EQ(list[0].value, 1);
    ASSERT_EQ(list[1].value, 3);
    ASSERT_EQ(ValueProbe::destroyed[1], 0);
    ASSERT_EQ(ValueProbe::destroyed[2], 1);
    ASSERT_EQ(ValueProbe::destroyed[3], 0);
  }
  ASSERT_EQ(ValueProbe::destroyed[1], 1);
  ASSERT_EQ(ValueProbe::destroyed[2], 1);
  ASSERT_EQ(ValueProbe::destroyed[3], 1);
}
