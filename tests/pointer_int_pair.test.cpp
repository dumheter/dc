#include <dc/allocator.hpp>
#include <dc/dtest.hpp>
#include <dc/pointer_int_pair.hpp>

using namespace dc;

DTEST(typicalSetAndGet) {
  PointerIntPair<int*, int> pair;
  int* ptr = static_cast<int*>(getDefaultAllocator().alloc(sizeof(int)));
  *ptr = 1337;
  int value = 3;

  pair.setPointer(ptr);
  pair.setInt(value);

  int* pairPtr = pair.getPointer();
  int pairInt = pair.getInt();
  ASSERT_EQ(pairPtr, ptr);
  ASSERT_EQ(pairInt, 3);

  getDefaultAllocator().free(ptr);
}
