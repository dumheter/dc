#include <dc/debug_allocator.hpp>
#include <dc/dtest.hpp>

using namespace dc;

DTEST(debugAllocatorBasicAllocation) {
  DebugAllocator allocator;
  void* ptr = allocator.alloc(64);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_EQ(allocator.getAllocationCount(), 1u);

  allocator.free(ptr);
  ASSERT_EQ(allocator.getAllocationCount(), 0u);
  ASSERT_FALSE(allocator.hasLeaks());
}

DTEST(debugAllocatorMultipleAllocations) {
  DebugAllocator allocator;
  void* ptr1 = allocator.alloc(32);
  void* ptr2 = allocator.alloc(64);
  void* ptr3 = allocator.alloc(128);
  ASSERT_EQ(allocator.getAllocationCount(), 3u);

  allocator.free(ptr2);
  ASSERT_EQ(allocator.getAllocationCount(), 2u);

  allocator.free(ptr1);
  allocator.free(ptr3);
  ASSERT_FALSE(allocator.hasLeaks());
}

DTEST(debugAllocatorRealloc) {
  DebugAllocator allocator;
  void* ptr = allocator.alloc(32);
  ASSERT_EQ(allocator.getAllocationCount(), 1u);

  void* newPtr = allocator.realloc(ptr, 128);
  ASSERT_TRUE(newPtr != nullptr);
  ASSERT_EQ(allocator.getAllocationCount(), 1u);

  allocator.free(newPtr);
  ASSERT_FALSE(allocator.hasLeaks());
}

DTEST(debugAllocatorReallocFromNull) {
  DebugAllocator allocator;
  void* ptr = allocator.realloc(nullptr, 64);
  ASSERT_TRUE(ptr != nullptr);
  ASSERT_EQ(allocator.getAllocationCount(), 1u);

  allocator.free(ptr);
  ASSERT_FALSE(allocator.hasLeaks());
}

DTEST(debugAllocatorLeakDetection) {
  ASSERT_EXCEPTION({
    DebugAllocator allocator;
    void* ptr = allocator.alloc(64);
    DC_UNUSED(ptr);
  });
}

DTEST(debugAllocatorFreeNull) {
  DebugAllocator allocator;
  allocator.free(nullptr);
  ASSERT_EQ(allocator.getAllocationCount(), 0u);
}
