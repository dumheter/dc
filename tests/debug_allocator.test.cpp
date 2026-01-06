#include <dc/debug_allocator.hpp>
#include <dc/dtest.hpp>
#include "dc/allocator.hpp"
#include "dc/time.hpp"

#if defined(_MSC_VER)
#if !defined(MEAN_AND_LEAN)
#define MEAN_AND_LEAN
#endif
#if !defined(NO_MIN_MAX)
#define NO_MIN_MAX
#endif
#include <Windows.h>
#endif

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
  DC_UNUSED(dtestBodyState__you_must_have_an_assert);
  DC_TRY {
    DebugAllocator allocator;
    void* ptr = allocator.alloc(64);
    DC_UNUSED(ptr);
  }
  DC_EXCEPT(EXCEPTION_EXECUTE_HANDLER) {
    ASSERT_TRUE(true);
    return;
  }

#ifndef WIN32
  ASSERT_TRUE(true);
#else
  ASSERT_TRUE(false);
#endif
}

DTEST(debugAllocatorFreeNull) {
  DebugAllocator allocator;
  allocator.free(nullptr);
  ASSERT_EQ(allocator.getAllocationCount(), 0u);
}
