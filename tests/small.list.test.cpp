#include <dc/dtest.hpp>
#include <dc/list.hpp>

using namespace dc;

DTEST(canAddToSmallList) {
  SmallList<int> v;

  v.add(1337);
  v.add(1338);

  ASSERT_EQ(v.getSize(), 2);
  ASSERT_TRUE(v.getCapacity() > 0);
  ASSERT_EQ(v[0], 1337);
  ASSERT_EQ(v[1], 1338);
}

DTEST(emptyDefaultListIsEmpty) {
  SmallList<int> v;
  ASSERT_EQ(v.getSize(), 0);
}

DTEST(growWhenOOM) {
  SmallList<float> v;

  v.add(10);
  ASSERT_EQ(v.getSize(), 1);
  const u64 capacityBefore = v.getCapacity();

  v.add(20);
  v.add(20);
  v.add(20);
  v.add(20);

  v.add(20);
  v.add(20);
  v.add(20);
  v.add(20);

  v.add(20);
  v.add(20);
  v.add(20);
  v.add(20);

  v.add(20);
  v.add(20);
  ASSERT_EQ(v.getSize(), 15);
  ASSERT_TRUE(v.getCapacity() > capacityBefore);
}

DTEST(remove) {
  SmallList<int> v;

  v.add(10);
  v.add(20);
  v.add(30);

  v.remove(1);
  ASSERT_EQ(v.getSize(), 2);
  ASSERT_EQ(v[1], 30);
}

// DTEST(clone) {
// 	SmallList<int> v;

// 	v.add(10);
// 	v.add(20);
// 	v.add(30);

// 	SmallList<int> clone = v.clone();
// 	ASSERT_EQ(clone[0], 10);
// 	ASSERT_EQ(clone[1], 20);
// 	ASSERT_EQ(clone[2], 30);
// 	ASSERT_EQ(clone.getSize(), v.getSize());
// }

DTEST(constIterator) {
  SmallList<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  int sum = 0;
  for (const int& i : v) {
    sum += i;
  }

  ASSERT_EQ(sum, 7);
}

DTEST(mutableIterator) {
  SmallList<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  for (int& i : v) i += 1;

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 10);
}
